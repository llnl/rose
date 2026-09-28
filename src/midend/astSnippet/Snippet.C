#include "sage3basic.h"                                 // every librose .C file must start with this

#include "Snippet.h"
#include "AstTraversal.h"
#include "LinearCongruentialGenerator.h"
#include "rose_getline.h"

#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

namespace Rose {

std::ostream& operator<<(std::ostream &o, const SnippetInsertion &inserted) {
    o <<"(inserted=(" <<inserted.inserted->class_name() <<"*)" <<inserted.inserted
      <<", original=(" <<inserted.original->class_name() <<"*)" <<inserted.original
      <<", insertedBefore=(" <<inserted.insertedBefore->class_name() <<"*)" <<inserted.insertedBefore
      <<")";
    return o;
}

/*******************************************************************************************************************************
 *                                      Snippet AST Traversals
 *******************************************************************************************************************************/

void SnippetAstTraversal::traverse(SgNode *ast) {
    struct T1: AstPrePostProcessing {
        SnippetAstTraversal &self;
        T1(SnippetAstTraversal &self): self(self) {}
        virtual void preOrderVisit(SgNode *node) override {
            self(node, preorder);
            if (SgExpression *expr = isSgExpression(node)) {
                if (expr->attributeExists("body")) {
                    AstSgNodeAttribute *attr = dynamic_cast<AstSgNodeAttribute*>(expr->getAttribute("body"));
                    if (SgClassDeclaration *anonDecl = isSgClassDeclaration(attr ? attr->getNode() : NULL))
                        T1(self).traverse(anonDecl);    // new copy avoids iterator problems in AstProcessing.h
                }
            }
        }
        virtual void postOrderVisit(SgNode *node) override {
            self(node, postorder);
        }
    };
    T1(*this).traverse(ast);
}

    

/*******************************************************************************************************************************
 *                                      SnippetFile
 *******************************************************************************************************************************/

SnippetFile::Registry SnippetFile::registry;
std::vector<std::string> SnippetFile::varNameList;

// Class method
SnippetFilePtr
SnippetFile::instance(const std::string &filename, SgSourceFile *snippetAst/*=NULL*/)
{
    SnippetFilePtr retval = lookup(filename);
    if (retval!=NULL)
        return retval;

    if (!snippetAst) {
        snippetAst = parse(filename);
        assert(snippetAst!=NULL);
    }

    retval = registry[filename] = SnippetFilePtr(new SnippetFile(filename, snippetAst));
    retval->findSnippetFunctions();
    
    return retval;
}

// Class method
SnippetFilePtr
SnippetFile::lookup(const std::string &fileName)
{
    struct stat sb;
    if (-1 == stat(fileName.c_str(), &sb))
        throw std::runtime_error(std::string("Rose::Snippet: ") + strerror(errno) + ": " + fileName);
    return registry.get_value_or(fileName, SnippetFilePtr());
}

#ifdef ROSE_BUILD_JAVA_LANGUAGE_SUPPORT
// Return the first non-empty statement from the specified source code, without the trailing semicolon.  This returns the
// non-comment material from the beginning of the supplied string up to but not including the first semicolon that is not part
// of a comment or string literal.  Comments are C++/Java style comments. Strings are delimited by single and double quotes
// (any number of characters in either) and support backslash as an escape mechanism.  The source string is assumed to start
// outside a comment or string.
static std::string extractFirstStatement(const std::string &source)
{
    std::string firstStmt;
    std::string inComment;                              // comment state: "", "/", "//", "/*", or "/**"
    char inString='\0';                                 // string state: '\0', '"' or '\''
    bool escaped=false;                                 // true if previous character was a backslash not in a comment
    BOOST_FOREACH (char ch, source) {
        if (inComment=="" && !inString && '/'==ch) {
            assert(!escaped);
            inComment = "/";
        } else if (inComment=="/") {
            assert(!escaped);
            assert(!inString);
            if ('/'==ch || '*'==ch) {
                inComment += ch;
            } else {
                firstStmt += inComment + ch;
                inComment = "";
                if ('\''==ch || '"'==ch)
                    inString = ch;
                escaped = '\\'==ch;
            }
        } else if (inComment=="//" && '\n'==ch) {
            assert(!escaped);
            assert(!inString);
            inComment = "";
        } else if (inComment=="/*" && '*'==ch) {
            assert(!escaped);
            assert(!inString);
            inComment = "/**";
        } else if (inComment=="/**") {
            assert(!escaped);
            assert(!inString);
            if ('/'==ch) {
                inComment = "";
            } else {
                inComment = "/*";
            }
        } else if (inComment=="//" || inComment=="/*") {
            assert(!escaped);
            assert(!inString);
        } else if (inString) {
            assert(inComment=="");
            firstStmt += ch;
            if (!escaped && ch==inString)
                inString = '\0';
            escaped = '\\'==ch;
        } else if (';'==ch) {
            boost::trim(firstStmt);
            if (!firstStmt.empty())
                break;
        } else {
            firstStmt += ch;
        }
    }
    return boost::trim_copy(firstStmt);
}

// Look at source code (before it's parsed) to try to figure out what package it belongs to.  The "package" statement must be
// the first statement in the file and there can be only one, so it's fairly easy to find.
static std::string getJavaPackageFromSourceCode(const std::string &source)
{
    std::string firstStmt = extractFirstStatement(source);
    if (boost::starts_with(firstStmt, "package")) {
        std::string pkgName = boost::trim_copy(firstStmt.substr(7));
        return pkgName;
    }

    return "";
}

// Return the class name from a file name.  If the file is "a/b/c.java" then the class is "c"
static std::string getJavaClassNameFromFileName(const std::string fileName)
{
    std::string notDir;                                 // part of the name after the final slash
    size_t slashIdx = fileName.rfind('/');
    if (slashIdx != std::string::npos) {
        notDir = fileName.substr(slashIdx+1);
    } else {
        notDir = fileName;
    }

    return boost::erase_last_copy(notDir, ".java");
}
#endif

#ifdef _MSC_VER
#define UNUSED_VAR
#else
#define UNUSED_VAR __attribute__((unused))
#endif

#ifdef ROSE_BUILD_JAVA_LANGUAGE_SUPPORT
// Parse java file based on addJavaSource.cpp test case
static SgFile* parseJavaFile(const std::string &fileName)
{
    // Read in the file as a string.  Easiest way is to use the MemoryMap facilities.
    std::string sourceCode;
    {
        std::ostringstream ss;
        std::ifstream file(fileName.c_str());
        ss <<file.rdbuf();
        sourceCode = ss.str();
    }

    // Get the package name etc. from the file contents.  We must know these before we can parse the file.
    std::string pkgName = getJavaPackageFromSourceCode(sourceCode);
    std::string pkgDirectory = boost::replace_all_copy(pkgName, ".", "/");
    std::string className = getJavaClassNameFromFileName(fileName);
    std::string qualifiedClassName = pkgName.empty() ? className : pkgName + "." + className;

#if 0 /*DEBUGGING [Robb P. Matzke 2014-03-31]*/
    std::cerr <<"ROBB: Java file name is \"" <<fileName <<"\"\n"
              <<"      Java package directory is \"" <<pkgDirectory <<"\"\n"
              <<"      Package name is \"" <<pkgName <<"\"\n"
              <<"      Non-qualified class name is \"" <<className <<"\"\n"
              <<"      Qualified class name is \"" <<qualifiedClassName <<"\n";
#endif

    // Code similar to the addJavaSource.cpp unit test
    SgProject *project = SageInterface::getProject();
    assert(project!=NULL);
    std::string tempDirectory = SageInterface::getTempDirectory(project);
#if 1 /*FIXME[Robb P. Matzke 2014-04-01]: working around a bug in the Java support*/
    // The current Java support is not able to create parent directories (i.e., like "mkdir -p foo/bar/baz") so we must
    // do that explicitly to prevent getting a segmentation fault in the Java run time. But we cannot create "baz" because
    // we must allow Java to be able to do that part.
    {
        std::string dirName = tempDirectory;
        std::vector<std::string> components;
        boost::split(components, pkgDirectory, boost::is_any_of("/"), boost::token_compress_on);
        BOOST_FOREACH (const std::string &component, components) {
            if (!component.empty()) {
                dirName += "/" + component;
                boost::filesystem::create_directory(dirName);
                std::cerr <<"ROBB: created directory \"" <<dirName <<"\"\n";
            }
        }
        struct stat sb;
        int status UNUSED_VAR = stat(dirName.c_str(), &sb);
        assert(0==status);
        assert(boost::filesystem::is_directory(dirName));
        if (dirName!=tempDirectory) {
            boost::filesystem::remove_all(dirName); // removing leaf directory so Java can create it
            std::cerr <<"ROBB: removed directory \"" <<dirName <<"\"\n";
        }
    }
#endif

    // We need to make sure the package exists, but findOrInsertJavaPackage fails when the package name is the empty string.
    // Hope that package "" exists already.
    std::string classPath;
    if (pkgName.empty()) {
        classPath = className;
    } else {
        SgClassDefinition *pkgDef = SageInterface::findOrInsertJavaPackage(project, pkgName, true/* create dir if inserted */);
        assert(pkgDef!=NULL);
        classPath = pkgDirectory + "/" + className;
    }

    // Parse the source code
    SgFile *file = SageInterface::preprocessCompilationUnit(project, classPath, sourceCode);

#if 0
    /* FIXME[Robb P. Matzke 2014-04-01]: This directory is apparently needed after parsing and I'm not sure when its safe
     * to clean it up. The addJavaSource unit test does the cleanup after calling backend(), but we have no control over
     * that since it's done by the user sometime after snippet injections are finished (in fact, the snippet data structures
     * might already be destroyed by then). */
    SageInterface::destroyTempDirectory(tempDirectory);
#endif
#if 1 /*DEBUGGING [Robb P. Matzke 2014-04-01]*/
    std::cerr <<"ROBB: Java snippet has been parsed; file = (" <<file->class_name() <<"*)" <<file <<"\n";
#endif

    return file;
}
#endif

// Class method
SgSourceFile *
SnippetFile::parse(const std::string &fileName)
{
    assert(!fileName.empty());
    
    // We should never unparse the snippet to a separate file, so provide an invalid name to catch errors
    std::string outputName = "/SNIPPET_SHOULD_NOT_BE_UNPARSED/x";

    // Try to load the snippet by parsing its source file
    SgFile *file = NULL;
    if (SageInterface::is_Java_language()) {
#if 0 /* [Robb P. Matzke 2014-03-31] */
        // This appears not to work any better than SageBuilder::buildFile and Philippe Charles concurs that it might not.
        file = SageInterface::processFile(SageInterface::getProject(), fileName, false/* don't unparse */);
#elif defined(ROSE_BUILD_JAVA_LANGUAGE_SUPPORT)
        // This is the better way (but much more complicated) to parse a Java file.
        file = parseJavaFile(fileName);
#endif
    } else {
        file = SageBuilder::buildFile(fileName, outputName, SageInterface::getProject());
    }
    SgSourceFile *snippetAst = isSgSourceFile(file);
    assert(snippetAst!=NULL);

  // DQ (7/2/2020): Added assertion (fails for snippet tests).
     if (snippetAst->get_preprocessorDirectivesAndCommentsList() == NULL)
       {
         snippetAst->set_preprocessorDirectivesAndCommentsList(new ROSEAttributesListContainer());
       }
     ROSE_ASSERT(snippetAst->get_preprocessorDirectivesAndCommentsList() != NULL);

    attachPreprocessingInfo(snippetAst);
    resetConstantFoldedValues(file);
    snippetAst->set_skip_unparse(true);
    return snippetAst;
}

void
SnippetFile::findSnippetFunctions()
{
    struct SnippetFinder: SnippetAstTraversal {
        FunctionDefinitionMap &functions;
        SnippetFinder(FunctionDefinitionMap &functions): functions(functions) {}
        void operator()(SgNode *node, AstSimpleProcessing::Order when) {
            if (preorder==when) {
                if (SgFunctionDefinition *fdef = isSgFunctionDefinition(node)) {
                    SgFunctionDeclaration *fdecl = fdef->get_declaration();
                    SgFunctionType *ftype = fdecl ? fdecl->get_type() : NULL;
                    SgType *rettype = ftype ? ftype->get_return_type() : NULL;
                    ROSE_ASSERT(fdecl != NULL);
                    if (rettype==SageBuilder::buildVoidType() && // snippets must return void
                        !boost::contains(fdecl->get_qualified_name().getString(), "<")) // and not have funky names
                        functions[fdef->get_declaration()->get_qualified_name()].push_back(fdef);
                }
            }
        }
    };
    SnippetFinder(functions).traverse(ast);
}

SnippetPtr
SnippetFile::findSnippet(const std::string &snippetName)
{
    ASSERT_this();
    assert(!snippetName.empty());
    FunctionDefinitionMap::const_iterator found = functions.find(snippetName);
    if (found!=functions.end() && !found->second.empty())
        return SnippetPtr(new Snippet(snippetName, shared_from_this(), found->second.front()));
    return SnippetPtr();
}

std::vector<SnippetPtr>
SnippetFile::findSnippets(const std::string &snippetName)
{
    ASSERT_this();
    assert(!snippetName.empty());
    std::vector<SnippetPtr> retval;
    FunctionDefinitionMap::const_iterator found = functions.find(snippetName);
    if (found!=functions.end()) {
        BOOST_FOREACH (SgFunctionDefinition *fdef, found->second)
            retval.push_back(SnippetPtr(new Snippet(snippetName, shared_from_this(), fdef)));
    }
    return retval;
}

std::vector<std::string>
SnippetFile::getSnippetNames() const
{
    ASSERT_this();
    std::vector<std::string> retval;
    for (FunctionDefinitionMap::const_iterator fi=functions.begin(); fi!=functions.end(); ++fi)
        retval.push_back(fi->first);
    return retval;
}

bool
SnippetFile::globallyInjected(SgGlobal *destination)
{
    ASSERT_this();
    assert(destination!=NULL);
    return !globals.insert(destination).second; // return true iff not already present
}

bool
SnippetFile::fileIsIncluded(const std::string &filename, SgGlobal *destination_scope)
{
    return !headersIncluded[filename].insert(destination_scope).second;
}

void
SnippetFile::expandSnippets(SgNode *ast)
{
    typedef Map<SgFunctionCallExp*, std::string/*snippetname*/> SnippetCalls;

    // Find statements that are calls to snippets, but don't do anything yet
    struct FindSnippetCalls: SnippetAstTraversal {
        SnippetFile *self;
        SnippetCalls calls;

        FindSnippetCalls(SnippetFile *self): self(self) {}

        void operator()(SgNode *node, AstSimpleProcessing::Order when) {
            if (preorder==when) {
                if (SgExprStatement *stmt = isSgExprStatement(node)) {
                    if (SgFunctionCallExp *fcall = isSgFunctionCallExp(stmt->get_expression())) {
                        SgFunctionSymbol *fsym = fcall->getAssociatedFunctionSymbol();
                        SgFunctionDeclaration *fdecl = fsym ? fsym->get_declaration() : NULL;
                        std::string called_name = fdecl ? fdecl->get_qualified_name().getString() : std::string();
                        if (self->functions.exists(called_name))
                            calls.insert(std::make_pair(fcall, called_name));
                    }
                }
            }
        }
    } t1(this);
    t1.traverse(ast);

    // Replace each of the snippet calls by expanding the snippet. The expansion occurs right before the snippet call and then
    // the snippet call is removed.
    for (SnippetCalls::iterator ci=t1.calls.begin(); ci!=t1.calls.end(); ++ci) {
        SgFunctionCallExp *fcall = ci->first;
        assert(fcall!=NULL);
        const SgExpressionPtrList &fcall_args = fcall->get_args()->get_expressions();
        std::vector<SgNode*> actuals(fcall_args.begin(), fcall_args.end());
        SgStatement *toReplace = SageInterface::getEnclosingNode<SgStatement>(fcall);
        SnippetPtr snippet;
        BOOST_FOREACH (SnippetPtr candidate, findSnippets(ci->second)) {
            if (candidate->numberOfArguments() == actuals.size()) {
                if (snippet!=NULL) {
                    throw std::runtime_error("snippet call \"" + ci->second + "\" having " +
                                             StringUtility::plural(actuals.size(), "arguments") +
                                             " matches multiple snippet definitions");
                }
                snippet = candidate;
            }
        }
        if (snippet==NULL) {
            throw std::runtime_error("snippet call \"" + ci->second + "\" having " +
                                     StringUtility::plural(actuals.size(), "arguments") +
                                     " does not match any snippet definition");
        }
        snippet->insert(toReplace, actuals);
        SageInterface::removeStatement(toReplace);
    }
}

// class method
size_t
SnippetFile::loadVariableNames(const std::string &fileName)
{
    size_t nread = 0;
    if (FILE *dict = fopen(fileName.c_str(), "r")) {
        char *line = NULL;
        size_t linesz = 0;
        while (rose_getline(&line, &linesz, dict)>0) {
            std::string word = StringUtility::trim(line);
            if (!word.empty()) {
                if (!isalpha(word[0]) && '_'!=word[0])
                    word[0] = '_';
                for (size_t i=0; i<word.size(); ++i) {
                    if (!isalnum(word[i]) && '_'!=word[i]) {
                        word[i] = '_';
                    } else if (isupper(word[i])) {
                        word[i] = tolower(word[i]);
                    }
                }
                varNameList.push_back(word);
                ++nread;
            }
        }

     // DQ (2/28/2014): This causes my test code to fail (specimen2014_01.c in 
     // ResetParentPointers::evaluateInheritedAttribute() with a memory problem).
        free(line);

        fclose(dict);
    }
    return nread;
}

// class method
std::string
SnippetFile::randomVariableName()
{
    static size_t ncalls = 0;
    static LinearCongruentialGenerator random(2013120913ul);
    if (!varNameList.empty())
        return varNameList[random() % varNameList.size()] + StringUtility::numberToString(++ncalls);
    
    std::string name = "T_";
    static const char *letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    assert(strlen(letters)==2*26);
    for (size_t i=0; i<6; ++i)
        name += letters[random()%(2*26)];
    return name;
}

void
SnippetFile::doNotInsert(const std::string &name, SgType *type/*=NULL*/)
{
    assert(!name.empty());
    blackListedDeclarations[name].push_back(type);
}

bool
SnippetFile::isBlackListed(SgDeclarationStatement *decl)
{
    ASSERT_this();
    assert(decl!=NULL);
    bool retval = false;

    // Obtain the name and type for the declaration
    std::string name;
    SgType *type = NULL;
    if (SgClassDeclaration *classDecl = isSgClassDeclaration(decl)) {
        name = classDecl->get_qualified_name().getString();
        type = classDecl->get_type();
    } else if (SgEnumDeclaration *enumDecl = isSgEnumDeclaration(decl)) {
        name = enumDecl->get_qualified_name().getString();
        type = enumDecl->get_type();
    } else if (SgFunctionDeclaration *funcDecl = isSgFunctionDeclaration(decl)) {
        name = funcDecl->get_qualified_name().getString();
        type = funcDecl->get_type();
    } else if (SgTypedefDeclaration *typedefDecl = isSgTypedefDeclaration(decl)) {
        name = typedefDecl->get_qualified_name().getString();
        type = typedefDecl->get_type();
    } else if (SgVariableDeclaration *varDecl = isSgVariableDeclaration(decl)) {
        // We currently only support variable declarations having exactly one SgInitializedName, otherwise we would potentially
        // need to transform the SgVariableDeclaration while it was copied to the target AST in order to remove only those
        // SgInitializedName nodes that are black listed.
        const SgInitializedNamePtrList &inames = varDecl->get_variables();
        if (1!=inames.size()) {
            std::cerr <<"warning: SnippetFile::isBlackListed does not support declaration statements with more than"
                      <<" one variable.\n";
        } else {
            SgInitializedName *iname = inames.front();
            name = iname->get_qualified_name();
            type = iname->get_type();
        }
    }

    // Is the (name,type) pair black listed?  If a black listed type is null then only the name needs to match.
    SgTypePtrList blackListedTypes = blackListedDeclarations.get_value_or(name, SgTypePtrList());
    if (!blackListedTypes.empty()) {
        BOOST_FOREACH (SgType *blackListedType, blackListedTypes) {
            if (!blackListedType || type==blackListedType) {
                retval = true;
                break;
            }
        }
    }

    return retval;
}

/*******************************************************************************************************************************
 *                                      Snippet
 *******************************************************************************************************************************/


// Class method
SnippetPtr
Snippet::instance(const std::string &snippetName, const SnippetFilePtr &snippetFile)
{
    assert(!snippetName.empty());
    assert(snippetFile!=NULL);

    SnippetPtr retval = snippetFile->findSnippet(snippetName);
    if (!retval)
        throw std::runtime_error("Rose::Snippet: snippet '"+snippetName+"' not found in file '"+snippetFile->getName()+"'");
    return retval;
}

// Class method
SnippetPtr
Snippet::instanceFromFile(const std::string &snippetName, const std::string &fileName)
{
    assert(!snippetName.empty());
    assert(!fileName.empty());
    SnippetFilePtr snippetFile = SnippetFile::instance(fileName); // cached instance if possible
    return instance(snippetName, snippetFile);
}

size_t
Snippet::numberOfArguments() const
{
    ASSERT_this();
    assert(ast!=NULL);
    assert(ast->get_declaration()!=NULL);
    assert(ast->get_declaration()->get_parameterList()!=NULL);
    return ast->get_declaration()->get_parameterList()->get_args().size();
}

void
Snippet::insert(SgStatement *insertionPoint)
{
    std::vector<SgNode*> args;
    insert(insertionPoint, args);
}

void
Snippet::insert(SgStatement *insertionPoint, SgNode *arg1)
{
    std::vector<SgNode*> args;
    args.push_back(arg1);
    insert(insertionPoint, args);
}

void
Snippet::insert(SgStatement *insertionPoint, SgNode *arg1, SgNode *arg2)
{
    std::vector<SgNode*> args;
    args.push_back(arg1);
    args.push_back(arg2);
    insert(insertionPoint, args);
}

void
Snippet::insert(SgStatement *insertionPoint, SgNode *arg1, SgNode *arg2, SgNode *arg3)
{
    std::vector<SgNode*> args;
    args.push_back(arg1);
    args.push_back(arg2);
    args.push_back(arg3);
    insert(insertionPoint, args);
}

void
Snippet::insert(SgStatement *insertionPoint, SgNode *arg1, SgNode *arg2, SgNode *arg3, SgNode *arg4)
{
    std::vector<SgNode*> args;
    args.push_back(arg1);
    args.push_back(arg2);
    args.push_back(arg3);
    args.push_back(arg4);
    insert(insertionPoint, args);
}

void
Snippet::insert(SgStatement *insertionPoint, const std::vector<SgNode*> &actuals)
{
    using namespace StringUtility;
    ASSERT_this();
    assert(insertionPoint!=NULL);
    assert(ast!=NULL);

    SgFunctionDefinition *targetFunction = SageInterface::getEnclosingNode<SgFunctionDefinition>(insertionPoint);
    assert(targetFunction!=NULL);
    SgScopeStatement *targetFunctionScope = targetFunction->get_body();
    assert(targetFunctionScope!=NULL);

    // Find the first declaration statement in the insertion point's function
    SgDeclarationStatement *targetFirstDeclaration = NULL;
    SgStatementPtrList targetStatements = targetFunctionScope->generateStatementList();
    for (size_t i=0; targetFirstDeclaration==NULL && i<targetStatements.size(); ++i)
        targetFirstDeclaration = isSgDeclarationStatement(targetStatements[i]);
    SgStatement *targetFirstStatement = targetStatements.empty() ? NULL : targetStatements.front();

    // Copy into the target file other functions, variables, imports, etc. that are above the snippet SgFunctionDefinition in
    // the snippet's file but which the user wants copied nonetheless.  Some of these things might be referenced by the
    // snippet, and others might completely unrelated but the user wants them copied anyway.
    insertRelatedThings(insertionPoint);

    // Insert the snippet body after the insertion point
    SgTreeCopy deep;
    SgScopeStatement *toInsert = isSgScopeStatement(ast->get_body()->copy(deep));
    assert(toInsert!=NULL);

    // DQ (3/4/2014): This is a test of the structural equality of the original snippet and it's copy.
    // If they are different then we can't support fixing up the AST.  Transformations on the snippet 
    // should have been made after insertion into the AST.  The complexity of this test is a traversal 
    // of the copy of the snippet to be inserted (typically very small compared to the target application).
    // Note that we can enforce this test here, but not after calling the replaceArguments() function below.
    bool isStructurallyEquivalent = SageInterface::isStructurallyEquivalentAST(toInsert,ast->get_body());
    ROSE_ASSERT(isStructurallyEquivalent == true);

    // DQ (3/4/2014): I think this is untimately a fundamental problem later (e.g. for mangled name generation).
    // So we have to attached the current scope to the scope of the insertion point.  This will allow the 
    // SgStatement::get_scope() to work (which is a problem for some debugging code (at least).
    // Note that the semantics of the AST copy mechanism is that the parent of the copy is set to NULL 
    // (which is assumed to be fixed up when the copy is inserted into the AST).
    ROSE_ASSERT(toInsert->get_parent() == NULL);
    SgScopeStatement* new_scope = isSgScopeStatement(insertionPoint->get_parent());
    ROSE_ASSERT(new_scope != NULL);
    toInsert->set_parent(new_scope);
    ROSE_ASSERT(toInsert->get_parent() != NULL);

    renameTemporaries(toInsert);
    causeUnparsing(toInsert, targetFunction->get_file_info());

    switch (insertMechanism) {
        case INSERT_BODY: {
            // Insert the body all at once. This is efficient but doesn't work well because it means that variables declared in
            // one snippet can't be used in a later snippet injected into the same function.
            file->addInsertionRecord(SnippetInsertion(toInsert, ast->get_body(), insertionPoint));
            SageInterface::insertStatementBefore(insertionPoint, toInsert);
            break;
        }
        case INSERT_STMTS: {
            // Insert one statement at a time.  Snippet declarations are placed at the top of the injection point's function,
            // which means that snippet variables cannot always be inialized in their declarations because the initialization
            // expression might be something that's only well defined at the point of insertion.
            const SgStatementPtrList &stmts = toInsert->getStatementList();
            const SgStatementPtrList &stmtsOrig = ast->get_body()->getStatementList();
            assert(stmts.size()==stmtsOrig.size());
            for (size_t i=0; i<stmts.size(); ++i) {
                if (isSgDeclarationStatement(stmts[i])) {
                    switch (locDeclsPosition) {
                        case LOCDECLS_AT_BEGINNING:
                            if (targetFirstDeclaration!=NULL) {
                                file->addInsertionRecord(SnippetInsertion(stmts[i], stmtsOrig[i], targetFirstDeclaration));
                                SageInterface::insertStatementBefore(targetFirstDeclaration, stmts[i]);
                            } else {
                                file->addInsertionRecord(SnippetInsertion(stmts[i], stmtsOrig[i], targetFirstStatement));
                                SageInterface::insertStatementBefore(targetFirstStatement, stmts[i]);
                            }
                            break;
                        case LOCDECLS_AT_END:
                            file->addInsertionRecord(SnippetInsertion(stmts[i], stmtsOrig[i], targetFunctionScope));
                            SageInterface::insertStatementBeforeFirstNonDeclaration(stmts[i], targetFunctionScope);
                            break;
                        case LOCDECLS_AT_CURSOR:
                            file->addInsertionRecord(SnippetInsertion(stmts[i], stmtsOrig[i], insertionPoint));
                            SageInterface::insertStatementBefore(insertionPoint, stmts[i]);
                            break;
                    }
                } else {
                    file->addInsertionRecord(SnippetInsertion(stmts[i], stmtsOrig[i], insertionPoint));
                    SageInterface::insertStatementBefore(insertionPoint, stmts[i]);
                }
            }
            break;
        }
    }

    // Build a map binding formal argument symbols to their actual values.  The goal is, in the statements we just inserted, to
    // replace all references to snippet formal arguments with the actual arguments supplied to the Snippet::insert() call. The
    // actual arguments are SgInitializedName or SgExpression nodes from the context in which the snippet was inserted; the
    // references to formal arguments are the SgInitializedName nodes in the AST's that we just copied above (the
    // *declarations* for the formal arguments were *not* copied because they are the get_parameterList() of the snippet
    // functionin the snippet file--which we didn't insert into the target).
    ArgumentBindings bindings; // map from formal arg references to actual arguments
    SgFunctionDeclaration *snippet_fdecl = ast->get_declaration();
    const SgInitializedNamePtrList &formals = snippet_fdecl->get_parameterList()->get_args();
    if (actuals.size()!=formals.size()) {
        throw std::runtime_error("Rose::Snippet: mismatched snippet arguments: expected " +
                                 plural(formals.size(), "arguments") + " but got " +
                                 numberToString(actuals.size()));
    }
    for (size_t i=0; i<formals.size(); ++i) {
        SgSymbol *formalSymbol = formals[i]->search_for_symbol_from_symbol_table();
        assert(formalSymbol!=NULL);
        if (actuals[i]==NULL)
            throw std::runtime_error("Rose::Snippet: snippet '"+name+"' actual argument "+numberToString(i+1)+" is null");
        if (!isSgInitializedName(actuals[i]) && !isSgExpression(actuals[i]))
            throw std::runtime_error("Rose::Snippet: snippet '"+name+"' actual argument "+numberToString(i+1)+
                                     " must be a variable declaration or expression"
                                     " but has type "+actuals[i]->class_name());
        bindings[formalSymbol] = actuals[i];
    }
    replaceArguments(toInsert, bindings);

 // insertionPoint->get_file_info()->display("insertionPoint: test 4: debug");

    if (insertRecursively)
        file->expandSnippets(toInsert);

    if (fixupAst) {
     // DQ (2/26/2014): Adding support to fixup the AST fragment (toInsert) that is being inserted into the target AST.

     // Build a translation map so that we can save the mapping of scopes between the target AST and the snippet AST.
     // In the case of insertMechanism == INSERT_STMTS we also add the mapping of the scope used for the copy of the body
     // to the parent of the insertionPoint so that symbols can be reset into the scope of the insertionPoint.
     // This map connects snippet scopes (key) to target AST scopes (value).
     // std::map<SgNode*,SgNode*> translationMap;

     // If we are inserting into the end of a scope then we point to the scope since there is no last statement 
     // to insert a statement before.  In this case then insertionPointIsScope == true, else it is false. 
        bool insertionPointIsScope = false;

        if (insertMechanism == INSERT_BODY)
           {
          // Fill in the first entry to inlcude the mapping of the copy of the scope (body) to the associated scope of the
          // insertionPoint.
             SageBuilder::fixupCopyOfAstFromSeparateFileInNewTargetAst(insertionPoint, insertionPointIsScope, toInsert,
                                                                       ast->get_body());
           }
          else
           {
             if (insertMechanism == INSERT_STMTS)
                {
                  const SgStatementPtrList &stmts_copy_of_snippet_ast     = toInsert->getStatementList();
                  const SgStatementPtrList &stmts_in_original_snippet_ast = ast->get_body()->getStatementList();
                  ROSE_ASSERT(stmts_copy_of_snippet_ast.size() == stmts_in_original_snippet_ast.size());

                  for (size_t i = 0; i < stmts_copy_of_snippet_ast.size(); ++i) 
                     {
                    // We have to reset this since it can be changed by one of the cases below.
                       insertionPointIsScope = false;

                       if (isSgDeclarationStatement(stmts_copy_of_snippet_ast[i]) != NULL) 
                          {
                            switch (locDeclsPosition) 
                               {
                                 case LOCDECLS_AT_BEGINNING:
                                   // Fill in the first entry to inlcude the mapping of the copy of the scope (body) to the
                                   // associated scope of the insertionPoint.
                                      if (targetFirstDeclaration) {
                                          SageBuilder::fixupCopyOfAstFromSeparateFileInNewTargetAst(targetFirstDeclaration,
                                                                                                    insertionPointIsScope,
                                                                                                    stmts_copy_of_snippet_ast[i],
                                                                                                    stmts_in_original_snippet_ast[i]);
                                      } else {
                                          SageBuilder::fixupCopyOfAstFromSeparateFileInNewTargetAst(targetFirstStatement,
                                                                                                    insertionPointIsScope,
                                                                                                    stmts_copy_of_snippet_ast[i],
                                                                                                    stmts_in_original_snippet_ast[i]);
                                      }
                                      break;

                                 case LOCDECLS_AT_END:
                                   // We are providing the scope instead of the declaration reference.
                                      insertionPointIsScope = true;

                                   // Fill in the first entry to inlcude the mapping of the copy of the scope (body) to the
                                   // associated scope of the insertionPoint.
                                      SageBuilder::fixupCopyOfAstFromSeparateFileInNewTargetAst(targetFunctionScope,
                                                                                                insertionPointIsScope,
                                                                                                stmts_copy_of_snippet_ast[i],
                                                                                                stmts_in_original_snippet_ast[i]);
                                      break;

                                   case LOCDECLS_AT_CURSOR:
                                      SageBuilder::fixupCopyOfAstFromSeparateFileInNewTargetAst(insertionPoint,
                                                                                                insertionPointIsScope,
                                                                                                stmts_copy_of_snippet_ast[i],
                                                                                                stmts_in_original_snippet_ast[i]);
                                      break;
                               }
                          } 
                         else 
                          {
                         // Fill in the first entry to inlcude the mapping of the copy of the scope (body) to the associated
                         // scope of the insertionPoint.
                            SageBuilder::fixupCopyOfAstFromSeparateFileInNewTargetAst(insertionPoint,insertionPointIsScope,
                                                                                      stmts_copy_of_snippet_ast[i],
                                                                                      stmts_in_original_snippet_ast[i]);
                          }
                     }
                }
               else
                {
                  printf ("Error: insertMechanism should be either INSERT_BODY or INSERT_STMTS \n");
                  ROSE_ABORT();
                }
           }
    }
}

void
Snippet::causeUnparsing(SgNode *ast, Sg_File_Info */*target*/)
{
    // Mark the things we insert as being transformations so they get inserted into the output by backend()
    struct T1: SnippetAstTraversal {
        void operator()(SgNode *node, AstSimpleProcessing::Order when) {
            if (preorder==when) {
                if (SgLocatedNode *loc = isSgLocatedNode(node)) {
#if 0
                 // DQ (4/14/2014): This appears to work just fine, but I would have expected the longer version (below) would have been required.
                    loc->get_file_info()->setTransformation();
                    loc->setOutputInCodeGeneration();
#else
                 // DQ (3/1/2015): This is now being caught in the DOT file generation, so I think we need to use this better version.
                 // DQ (4/14/2014): This should be a more complete version to set all of the Sg_File_Info objects on a SgLocatedNode.
                    ROSE_ASSERT(loc->get_startOfConstruct() != NULL);
                    loc->get_startOfConstruct()->setTransformation();
                    loc->get_startOfConstruct()->setOutputInCodeGeneration();

                    ROSE_ASSERT(loc->get_endOfConstruct() != NULL);
                    loc->get_endOfConstruct()->setTransformation();
                    loc->get_endOfConstruct()->setOutputInCodeGeneration();

                    if (SgExpression* exp = isSgExpression(loc))
                       {
                         ROSE_ASSERT(exp->get_operatorPosition() != NULL);
                         exp->get_operatorPosition()->setTransformation();
                         exp->get_operatorPosition()->setOutputInCodeGeneration();
                       }
#endif
                }
            }
        }
    };
    T1().traverse(ast);
}

void
Snippet::renameTemporaries(SgNode *ast)
{
    ASSERT_this();

    struct: SnippetAstTraversal {
        void operator()(SgNode *node, AstSimpleProcessing::Order when) {
            if (SgInitializedName *vdecl = isSgInitializedName(node)) {
                if (preorder==when && 0==vdecl->get_name().getString().substr(0, 3).compare("tmp")) {
                    std::string newName = SnippetFile::randomVariableName();

                 // DQ (3/2/2014): Need to unload the associated symbol from the symbol tabel and reinsert it using the new
                 // name.  I assume this can not be a function parameter (else there will be no symbol).
                    SgSymbol* variableSymbol = vdecl->search_for_symbol_from_symbol_table();
                    ROSE_ASSERT(variableSymbol != NULL);
                    vdecl->get_scope()->remove_symbol(variableSymbol);

                    vdecl->set_name(newName);

                    vdecl->get_scope()->insert_symbol(newName,variableSymbol);
                }
            }
        }
    } t1;
    t1.traverse(ast);
}

void
Snippet::replaceVariable(SgVarRefExp *vref, SgExpression *replacement)
{
    struct Replacer: public SimpleReferenceToPointerHandler {
        SgNode *nodeToReplace, *replacement;
        size_t replaced;
        Replacer(SgNode *nodeToReplace, SgNode *replacement)
            : nodeToReplace(nodeToReplace), replacement(replacement), replaced(0) {}
        void operator()(SgNode *&node, const SgName &/*debugStringName*/, bool/*traverse*/) {
            if (node==nodeToReplace) {
                node = replacement;
                ++replaced;
            }
        }
    } replacer(vref, replacement);
    vref->get_parent()->processDataMemberReferenceToPointers(&replacer);
    assert(replacer.replaced==1);
}

// class method
void
Snippet::replaceArguments(SgNode *toInsert, const ArgumentBindings &bindings)
{
    struct T1: SnippetAstTraversal {
        const ArgumentBindings &bindings;
        T1(const ArgumentBindings &bindings): bindings(bindings) {}

        void operator()(SgNode *node, AstSimpleProcessing::Order when) {
            if (postorder==when) {                      // post-order because we're modifying the AST as we retreat
                if (SgVarRefExp *vref = isSgVarRefExp(node)) {
                    SgSymbol *formal_sym = vref->get_symbol(); // snippet symbol to be replaced
                    if (SgNode *bound = bindings.get_value_or(formal_sym, NULL)) {
                        if (SgInitializedName *replacement = isSgInitializedName(bound)) {
                            // Replace one variable reference with another. Rather than creating a new variable reference, we
                            // can just make the existing reference point to the replacement symbol.
                            SgVariableSymbol *new_sym = isSgVariableSymbol(replacement->search_for_symbol_from_symbol_table());
                            assert(new_sym!=NULL);
                            vref->set_symbol(new_sym);
                        } else if (SgExpression *replacement = isSgExpression(bound)) {
                            // The variable reference needs to be replaced by a new expression.
                            replaceVariable(vref, replacement);
                        } else {
                            assert(!"replacement is something weird");
                        }
                    }
                } else if (SgTypedefDeclaration *tdef = isSgTypedefDeclaration(node)) {
                    std::string tdef_name = tdef->get_name().getString();
                    for (ArgumentBindings::const_iterator bi=bindings.begin(); bi!=bindings.end(); ++bi) {
                        if (0==tdef_name.compare("typeof_" + bi->first->get_name().getString())) {
                            if (SgInitializedName *actual = isSgInitializedName(bi->second)) {
                                tdef->set_base_type(actual->get_type());
                            } else if (SgExpression *actual = isSgExpression(bi->second)) {
                                tdef->set_base_type(actual->get_type());
                            } else {
                                assert(!"actual is something weird");
                            }
                        }
                    }
                }
            }
        }
    } t1(bindings);
    t1.traverse(toInsert);
}

void
Snippet::removeIncludeDirectives(SgNode *node)
{
    if (SgLocatedNode *locnode = isSgLocatedNode(node)) {
        // AttachedPreprocessingInfoType is std::vector<PreprocessingInfo>
        if (AttachedPreprocessingInfoType *cpp = locnode->getAttachedPreprocessingInfo()) {
            AttachedPreprocessingInfoType::iterator iter=cpp->begin();
            while (iter!=cpp->end()) {
                if ((*iter)->getTypeOfDirective() == PreprocessingInfo::CpreprocessorIncludeDeclaration ||
                    (*iter)->getTypeOfDirective() == PreprocessingInfo::CpreprocessorIncludeNextDeclaration) {
                    iter = cpp->erase(iter);
                } else {
                    ++iter;
                }
            }
            // Remove the AttachedPreprocessingInfoType node from the AST if it is empty, otherwise an assertion in
            // SageInterface::insertStatement will fail: it checks that either the preprocessing list node is null or
            // non-empty.
            if (cpp->empty())
                locnode->set_attachedPreprocessingInfoPtr(NULL);
        }
    }
}

void
Snippet::insertRelatedThings(SgStatement *insertionPoint)
{
    ASSERT_this();
    assert(insertionPoint!=NULL);

    // Have we inserted stuff here already? Also mark that we've now done so.
    SgGlobal *ipointGlobalScope = SageInterface::getGlobalScope(insertionPoint);
    assert(ipointGlobalScope!=NULL);
    if (file->globallyInjected(ipointGlobalScope))
        return;

    // Language specific insertions
    if (SageInterface::is_Java_language()) {
        insertRelatedThingsForJava(insertionPoint);
    } else if (SageInterface::is_C_language()) {
        insertRelatedThingsForC(insertionPoint);
    }
}

void
Snippet::insertRelatedThingsForJava(SgStatement *insertionPoint)
{
    // The insertionPoint is some statement in a SgMemberFunctionDeclaration where this snippet was already inserted.  The
    // snippet's statements have already been inserted into the target function, and now we need to insert declarations that
    // are siblings of this snippet (in the snippet file) as siblings of the target SgMemberFunctionDeclaration into which this
    // snippet was inserted.
    SgStatement *topInsertionPoint = SageInterface::getEnclosingNode<SgMemberFunctionDeclaration>(insertionPoint);
    ROSE_ASSERT(topInsertionPoint || !"Java snippet must have been inserted into a SgMemberFunctionDeclaration");
    
    // Insert whole declarations from the snippet file into the target file.
    SgClassDefinition *snippetClass = SageInterface::getEnclosingNode<SgClassDefinition>(ast); // snippet's class
    ROSE_ASSERT(snippetClass || !"each java snippet must be a member of a class definition");
    BOOST_FOREACH (SgDeclarationStatement *decl, snippetClass->get_members()) {
        // Insert only those things that come from the snippet file, not from header files
        if (!decl->get_file_info()->isSameFile(ast->get_file_info()))
            continue;

        // If a declaration is blacklisted in the snippet file then don't insert it.
        if (file->isBlackListed(decl))
            continue;

        // Insert whole function definitions (snippets) only if the user asked for this feature.
        if (SgFunctionDeclaration *fdecl = isSgMemberFunctionDeclaration(decl)) {
            if (fdecl->get_definition()!=NULL && !file->getCopyAllSnippetDefinitions())
                continue;
        }

        // Insert this declaration
        SgTreeCopy deep;
        SgDeclarationStatement *declCopy = isSgDeclarationStatement(decl->copy(deep));
        causeUnparsing(declCopy, topInsertionPoint->get_file_info());
        file->addInsertionRecord(SnippetInsertion(declCopy, decl, topInsertionPoint));
        SageInterface::insertStatementBefore(topInsertionPoint, declCopy);

     // DQ (3/19/2014): Added fixup of AST for Java declarations copied into the AST.
        if (fixupAst) {
            // DQ (3/13/2014): Added more general support for AST fixup (after insertion into the AST).  If we are inserting into
            // the end of a scope then we point to the scope since there is no last statement to insert a statement before.  In
            // this case then insertionPointIsScope == true, else it is false.  I think that in the cases called by this function
            // insertionPointIsScope is always false.
            bool insertionPointIsScope        = false;
            SgStatement* toInsert             = declCopy;
            SgStatement* original_before_copy = decl;
            // std::map<SgNode*,SgNode*> translationMap;

            SageBuilder::fixupCopyOfAstFromSeparateFileInNewTargetAst(topInsertionPoint, insertionPointIsScope, toInsert,
                                                                      original_before_copy);
        }
    }

    // Copy import statements from the snippet's file into the target file.
    SgSourceFile *targetFile = isSgSourceFile(SageInterface::getEnclosingFileNode(insertionPoint));
    ROSE_ASSERT(targetFile || !"snippet insertion point must belong to a file");
    SgSourceFile *snippetFile = isSgSourceFile(SageInterface::getEnclosingFileNode(ast));
    ROSE_ASSERT(snippetFile || !"snippet must belong to a file");
    SgJavaImportStatementList *targetImports = targetFile->get_import_list();
    SgJavaImportStatementList *snippetImports = snippetFile->get_import_list();
    assert(targetImports!=NULL && snippetImports!=NULL);
    BOOST_FOREACH (SgJavaImportStatement *snippetImport, snippetImports->get_java_import_list()) {
        SgTreeCopy deep;
        SgJavaImportStatement *newImport = isSgJavaImportStatement(snippetImport->copy(deep));
        causeUnparsing(newImport, topInsertionPoint->get_file_info());
        file->addInsertionRecord(SnippetInsertion(newImport, snippetImport, targetImports));
        targetImports->get_java_import_list().push_back(newImport);
        newImport->set_parent(targetImports);
    }
}

void
Snippet::insertRelatedThingsForC(SgStatement *insertionPoint)
{
    // The insertionPoint was where we inserted the snippet. To insert related global stuff, we need another insertion point
    // that's near the top of the same file where the snippet was inserted.  We want to insert stuff after all the #include
    // directives, so look for the last global declaration that has #include attached in this file. That's where we'll do our
    // inserting.
    SgGlobal *ipointGlobalScope = SageInterface::getGlobalScope(insertionPoint);
    const SgDeclarationStatementPtrList &stmtList = ipointGlobalScope->get_declarations();
    SgStatement *firstDeclSameFile = NULL;              // first declaration in the insertion point's file
    SgStatement *lastDeclWithIncludes = NULL;           // last declaration in this file that has attached #include
    BOOST_FOREACH (SgStatement *decl, stmtList) {
        if (decl->get_file_info()->isSameFile(ipointGlobalScope->get_file_info())) {
            if (NULL==firstDeclSameFile)
                firstDeclSameFile = decl;
            if (AttachedPreprocessingInfoType *cpplist = decl->getAttachedPreprocessingInfo()) {
                BOOST_FOREACH (PreprocessingInfo *cpp, *cpplist) {
                    if (cpp->getTypeOfDirective() == PreprocessingInfo::CpreprocessorIncludeDeclaration ||
                        cpp->getTypeOfDirective() == PreprocessingInfo::CpreprocessorIncludeNextDeclaration) {
                        lastDeclWithIncludes = decl;
                        break;
                    }
                }
            }
        }
    }
    SgStatement *topInsertionPoint = lastDeclWithIncludes ? lastDeclWithIncludes : firstDeclSameFile;
    ROSE_ASSERT(topInsertionPoint || !"cannot find an insertion point for snippet global declarations");
    SgGlobal *snippetGlobalScope = SageInterface::getEnclosingNode<SgGlobal>(ast);
    assert(snippetGlobalScope!=NULL);

    // Insert declarations
    SgStatement *firstInserted = NULL; // earliest node (in traversal) we inserted into the target AST
    BOOST_FOREACH (SgDeclarationStatement *decl, snippetGlobalScope->get_declarations()) {
        // Insert only those things that come from the snippet file, not from header files
        if (!decl->get_file_info()->isSameFile(ast->get_file_info()))
            continue;

        // If a declaration is blacklisted in the snippet file then don't insert it.
        if (file->isBlackListed(decl))
            continue;

        // Insert whole function definitions (snippets) only if the user asked for this feature.
        if (SgFunctionDeclaration *fdecl = isSgFunctionDeclaration(decl)) {
            if (fdecl->get_definition()!=NULL && !file->getCopyAllSnippetDefinitions())
                continue;
        }

        // Insert this declaration
        SgTreeCopy deep;
        SgDeclarationStatement *declCopy = isSgDeclarationStatement(decl->copy(deep));
        removeIncludeDirectives(declCopy);
        causeUnparsing(declCopy, topInsertionPoint->get_file_info());

        // Error checking on the generated copy.
        SgClassDeclaration* classDeclaration_copy     = isSgClassDeclaration(declCopy);
        SgClassDeclaration* classDeclaration_original = isSgClassDeclaration(decl);
        if (classDeclaration_original != NULL)
           {
             ROSE_ASSERT(classDeclaration_copy != NULL);

             printf ("Error checking for copy of SgClassDeclaration = %p \n",classDeclaration_copy);

          // I think these types should be different,  if so then the associated non-defining 
          // declaration must be copied, and it's types match the defining declaration.
             ROSE_ASSERT(classDeclaration_copy->get_type() != classDeclaration_original->get_type());
             ROSE_ASSERT(classDeclaration_copy->get_firstNondefiningDeclaration() !=
                         classDeclaration_original->get_firstNondefiningDeclaration());

             SgClassDeclaration* nondefining_classDeclaration_copy =
                 isSgClassDeclaration(classDeclaration_copy->get_firstNondefiningDeclaration());
#ifndef NDEBUG
             SgClassDeclaration* nondefining_classDeclaration_original =
                 isSgClassDeclaration(classDeclaration_original->get_firstNondefiningDeclaration());
#endif
             ROSE_ASSERT(nondefining_classDeclaration_copy->get_type() != nondefining_classDeclaration_original->get_type());
             if (nondefining_classDeclaration_copy->get_type() != classDeclaration_copy->get_type())
                {
                  printf ("nondefining_classDeclaration_copy->get_type() = %p \n",nondefining_classDeclaration_copy->get_type());
                  printf ("classDeclaration_copy->get_type()             = %p \n",classDeclaration_copy->get_type());
                }
             ROSE_ASSERT(nondefining_classDeclaration_copy->get_type() == classDeclaration_copy->get_type());
             ROSE_ASSERT(nondefining_classDeclaration_original->get_type() == classDeclaration_original->get_type());

             ROSE_ASSERT(classDeclaration_copy->get_definingDeclaration() !=
                         classDeclaration_original->get_definingDeclaration());
#ifndef NDEBUG
             SgClassDeclaration* defining_classDeclaration_copy =
                 isSgClassDeclaration(classDeclaration_copy->get_definingDeclaration());
             SgClassDeclaration* defining_classDeclaration_original =
                 isSgClassDeclaration(classDeclaration_original->get_definingDeclaration());
             ROSE_ASSERT(defining_classDeclaration_original->get_type() == nondefining_classDeclaration_original->get_type());
             ROSE_ASSERT(defining_classDeclaration_copy->get_type() == nondefining_classDeclaration_copy->get_type());
#endif
           }

        file->addInsertionRecord(SnippetInsertion(declCopy, decl, topInsertionPoint));
        SageInterface::insertStatementBefore(topInsertionPoint, declCopy);
        if (!firstInserted)
            firstInserted = declCopy;
        
        if (fixupAst) {
            // DQ (3/13/2014): Added more general support for AST fixup (after insertion into the AST).  If we are inserting into
            // the end of a scope then we point to the scope since there is no last statement to insert a statement before.  In
            // this case then insertionPointIsScope == true, else it is false.  I think that in the cases called by this function
            // insertionPointIsScope is always false.
            bool insertionPointIsScope        = false;
            SgStatement* toInsert             = declCopy;
            SgStatement* original_before_copy = decl;
            // std::map<SgNode*,SgNode*> translationMap;

            SageBuilder::fixupCopyOfAstFromSeparateFileInNewTargetAst(topInsertionPoint, insertionPointIsScope, toInsert,
                                                                      original_before_copy);
        }
    }

    // If our topInsertionPoint had #include directives and we inserted stuff, then those include directives need to be moved
    // and reattached to the first node we inserted.
    if (firstInserted!=NULL && lastDeclWithIncludes!=NULL) {
        SageInterface::movePreprocessingInfo(lastDeclWithIncludes, firstInserted);

        // Since we moved the preprocessing info out of lastDeclWithIncludes, we need to also make sure that
        // lastDeclWithIncludes does not point to an AttachedPreprocessingInfoType node since
        // SageInterface::insertStatement requires that any AttachedPreprocessingInfoType is non-empty.
        if (lastDeclWithIncludes->getAttachedPreprocessingInfo()) {
            assert(lastDeclWithIncludes->get_attachedPreprocessingInfoPtr()->empty());
            lastDeclWithIncludes->set_attachedPreprocessingInfoPtr(NULL);
        }
    }
}

} // namespace
