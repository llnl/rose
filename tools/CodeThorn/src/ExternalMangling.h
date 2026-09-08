
#ifndef _CT_EXTERNAL_MANGLING_HPP
#define _CT_EXTERNAL_MANGLING_HPP 261005

#include <boost/utility/string_view.hpp>
#include <Rose/Source/Analysis/ExternalMangling.h>

namespace CodeThorn
{

using Rose::Source::Analysis::NameShortener;
using Rose::Source::Analysis::longNames;
using Rose::Source::Analysis::hashNames;
using Rose::Source::Analysis::mangle;
using Rose::Source::Analysis::clearManglingCache;
using Rose::Source::Analysis::printUnhandledNodes;

/// Compatibility overload for CodeThorn callers that pass a boost string view.
inline std::string mangleFilename(const boost::string_view& filename)
{
  return Rose::Source::Analysis::mangleFilename(std::string(filename.data(), filename.size()));
}


}

#endif /*_CT_EXTERNAL_MANGLING_HPP */
