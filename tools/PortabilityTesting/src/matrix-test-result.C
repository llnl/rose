static const char *gPurpose = "update database with test result";
static const char *gDescription =
    "Reads a file containing test results in the form of @v{key}=@v{value} lines and uses them to update "
    "the specified database. The list of valid keys can be obtained by running \"rose-matrix-query-table "
    "list\".  White space is allowed on either side of the equal sign, and white space is stripped from "
    "both the @v{key} and @v{value}. Blank lines and lines whose first non-space character is \"#\" are "
    "ignored.";

#include <rose.h>
#include "matrixTools.h"

// boost process.hpp first appeared in boost-1.64.0
#include <boost/version.hpp>
#if BOOST_VERSION >= 106400

#include <Rose/CommandLine.h>
#include <rose_getline.h>
#include <boost/algorithm/string/trim.hpp>
// Boost.Process v2 became the default in boost-1.86.0 and <boost/process.hpp> no longer provides
// the v1 API (child, ipstream, std_out) used below; v1 is still available, header-only, under
// <boost/process/v1/>.
#if BOOST_VERSION >= 108600
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/pipe.hpp>
namespace bp = boost::process::v1;
#else
#include <boost/process.hpp>
namespace bp = boost::process;
#endif

using namespace Rose;
using namespace Sawyer::Message::Common;
namespace DB = Sawyer::Database;

struct Settings {
    std::string databaseUri;                            // e.g., postgresql://user:password@host/database
    bool dryRun = false;
};

using KeyValuePairs = Sawyer::Container::Map<std::string /*colname*/, std::string /*value*/>;
using DependencyNames = Sawyer::Container::Map<std::string /*key*/, std::string /*colname*/>;

static Sawyer::Message::Facility mlog;

static boost::filesystem::path
parseCommandLine(int argc, char *argv[], Settings &settings) {
    using namespace Sawyer::CommandLine;
    Parser parser = Rose::CommandLine::createEmptyParser(gPurpose, gDescription);
    parser.errorStream(mlog[FATAL]);
    parser.doc("Synopsis", "@prop{programName} [@v{switches}] @v{file}");
    parser.doc("Output", "Emits the new test ID to standard output on success.");

    SwitchGroup sg("Tool-specific switches");
    insertDatabaseSwitch(sg, settings.databaseUri);

    sg.insert(Switch("dry-run", 'n')
              .intrinsicValue(true, settings.dryRun)
              .doc("Do everything but update the database and print a test ID."));

    const std::vector<std::string> args = parser
                                          .with(Rose::CommandLine::genericSwitches())
                                          .with(sg)
                                          .parse(argc, argv)
                                          .apply()
                                          .unreachedArgs();

    if (args.size() != 1) {
        mlog[FATAL] <<"incorrect usage; see --help\n";
        exit(1);
    }
    return args[0];
}

static DependencyNames
loadDependencyNames(DB::Connection db) {
    DependencyNames retval;
    auto stmt = db.stmt("select distinct name from dependencies");
    for (auto row: stmt) {
        const std::string key = row.get<std::string>(0).get();
        retval.insert(key, "rmc_"+key);
    }
    return retval;
}

static void
extraDependencies(DependencyNames &depnames /*in,out*/) {
    depnames.insert("duration", "duration");
    depnames.insert("noutput", "noutput");
    depnames.insert("nwarnings", "nwarnings");
    depnames.insert("os", "os");
    depnames.insert("rose", "rose");
    depnames.insert("rose_date", "rose_date");
    depnames.insert("status", "status");
    depnames.insert("tester", "tester");
}

static KeyValuePairs
parseFile(const boost::filesystem::path &fileName, const DependencyNames &depnames) {
    KeyValuePairs retval;

    std::ifstream in(fileName.string().c_str());
    if (!in.good()) {
        mlog[FATAL] <<"cannot open " <<fileName <<"\n";
        exit(1);
    }

    for (size_t lineNumber = 1; in; ++lineNumber) {
        std::string line = rose_getline(in);
        boost::trim(line);
        if (line.empty() || '#' == line[0])
            continue;

        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            mlog[FATAL] <<StringUtility::cEscape(fileName.string()) <<":" <<lineNumber <<": "
                        <<"not a key=value pair: \"" <<StringUtility::cEscape(line) <<"\"\n";
            exit(1);
        }
        const std::string key = boost::trim_copy(line.substr(0, eq));
        std::string val = boost::trim_copy(line.substr(eq+1));
        if (key.empty() || val.empty()) {
            mlog[FATAL] <<StringUtility::cEscape(fileName.string()) <<":" <<lineNumber <<": "
                        <<"key and value must be non-empty: \"" <<StringUtility::cEscape(line) <<"\"\n";
            exit(1);
        }
        if (!depnames.exists(key)) {
            mlog[FATAL] <<StringUtility::cEscape(fileName.string()) <<":" <<lineNumber <<": "
                        <<"key \"" <<StringUtility::cEscape(key) <<"\" is not recognized\n";
            exit(1);
        }
        if (retval.exists(depnames[key])) {
            mlog[WARN] <<StringUtility::cEscape(fileName.string()) <<":" <<lineNumber <<": "
                       <<"key \"" <<StringUtility::cEscape(key) <<"\" specified more than once\n";
            exit(1);
        }
        if (val.size() >= 2 && '\'' == val[0] && '\'' == val[val.size()-1])
            val = val.substr(1, val.size()-2);

        retval.insert(depnames[key], val);
    }
    return retval;
}

void
check_exists(const KeyValuePairs &kvpairs, const std::string &key) {
    if (!kvpairs.exists(key)) {
        mlog[FATAL] <<"key is required: \"" <<StringUtility::cEscape(key) <<"\"\n";
        exit(1);
    }
}

static std::string
getUserName() {
    if (const char *s = getenv("LOGNAME"))
        return s;

    bp::ipstream pipe_stream;
    bp::child c("whoami", bp::std_out > pipe_stream);
    std::string str;
    if (pipe_stream && std::getline(pipe_stream, str) && !str.empty())
        return boost::trim_copy(str);

    return "unknown";
}

static int
getUserId(DB::Connection db) {
    const std::string userName = getUserName();
    auto sql = db.stmt("select uid from users where name = ?name").bind("name", userName);
    for (auto row: sql) {
        auto userId = row.get<int>(0);
        return *userId;
    }
    mlog[FATAL] <<"no such user: \"" <<StringUtility::cEscape(userName) <<"\"\n";
    exit(1);
}

static std::string
getTester() {
    return getUserName() + " running " + Sawyer::CommandLine::Parser().programName();
}

int
main(int argc, char *argv[]) {
    ROSE_INITIALIZE;
    Diagnostics::initAndRegister(&mlog, "tool");
    Settings settings;
    boost::filesystem::path fileName = parseCommandLine(argc, argv, settings);
    DB::Connection db = connectToDatabase(settings.databaseUri, mlog);

    // Query the database to find valid keys
    DependencyNames dependencyNames = loadDependencyNames(db);
    extraDependencies(dependencyNames);

    // Parse and validate the key/val file
    KeyValuePairs kvpairs = parseFile(fileName, dependencyNames);

    // Some keys are required
    check_exists(kvpairs, "rose");
    check_exists(kvpairs, "status");
    check_exists(kvpairs, "tester");
    check_exists(kvpairs, "os");

    // Some keys can be set automatically
    kvpairs.insertMaybe("reporting_user", StringUtility::numberToString(getUserId(db)));
    kvpairs.insertMaybe("reporting_time", StringUtility::numberToString(time(NULL)));
    kvpairs.insertMaybe("tester", getTester());

    // Debugging
    if (mlog[TRACE]) {
        for (const KeyValuePairs::Node &node: kvpairs.nodes())
            mlog[TRACE] <<node.key() <<"=" <<node.value() <<"\n";
    }

    // Generate SQL to insert this information.
    auto stmt = db.stmt("insert into test_results"
                        " (" + StringUtility::join(", ", kvpairs.keys()) +")"
                        " values (?" + StringUtility::join(", ?", kvpairs.keys()) + ")"
                        " returning id");
    for (const auto &node: kvpairs.nodes())
        stmt.bind(node.key(), node.value());

    if (!settings.dryRun) {
        int testId = stmt.get<int>().get();
        mlog[INFO] <<"inserted test record #" <<testId <<"\n";
        std::cout <<testId <<"\n";
    }
}

#else

#include <iostream>

int main(int argc, char *argv[]) {
    std::cerr <<argv[0] <<": not available in this configuration of ROSE (requires boost-1.64.0 or later)\n";
    return 1;
}

#endif
