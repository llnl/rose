#include <Rose/Common/Analysis/ClassHierarchyExport.h>

#include <string>
#include <nlohmann/json.h>

namespace ga = Rose::Common::Analysis;

namespace
{
  struct JsonString
  {
    const std::string& value;
  };

  std::ostream& operator<<(std::ostream& os, JsonString string)
  {
    return os << nlohmann::json(string.value).dump();
  }

  void edge(std::ostream& os, ga::ClassKeyType src, ga::ClassKeyType tgt, std::string lbl, std::string attr)
  {
    os << "  p" << src.key << " -> p" << tgt.key
       << "[ taillabel = \"" << lbl << "\" " << attr << "];" << std::endl;
  }

  void node(std::ostream& os, ga::ClassKeyType n, std::string lbl, std::string attr)
  {
    os << "  p" << n.key << "[ label = \"" << lbl << "\" " << attr << "];" << std::endl;
  }

  void dotHeader(std::ostream& os, const std::string& kind, const std::string& s)
  {
    os << kind << ' ' << s << " {" << std::endl;
  }

  void dotFooter(std::ostream& os)
  {
    os << "}" << std::endl;
  }

  void dotInheritance(std::ostream& os, const ga::ClassAnalysis& all, ga::ClassFilterFn include)
  {
    static constexpr const char* normalColor  = "color=green";
    static constexpr const char* virtualColor = "color=red";

    os << "edge[dir=back arrowtail=empty]" << std::endl;

    for (const ga::ClassAnalysis::value_type& elem : all)
    {
      if (!include(elem.first))
        continue;

      for (const ga::InheritanceDesc& child : elem.second.descendants())
      {
        if (child.isDirect() && include(child.getClass()))
          edge(os, elem.first, child.getClass(), "", child.isVirtual() ? virtualColor : normalColor);
      }
    }
  }

  void printFunctions(std::ostream& os,
                      const std::string& desc,
                      ga::ClassNameFn& className,
                      ga::FuncNameFn& funcName,
                      const ga::VirtualFunctionAnalysis& vfuns,
                      const ga::OverrideContainer& functions,
                      bool skipEmpty = true)
  {
    if (skipEmpty && functions.empty())
      return;

    os << "    " << desc << ": ";

    for (const ga::OverrideDesc& fn : functions)
    {
      const ga::FunctionKeyType funid = fn.function();
      const ga::VirtualFunctionDesc& other = vfuns.at(funid);

      os << className(other.classId())
         << "::" << funcName(funid)
         << " #" << funid.key
         << (fn.covariantReturn() ? " (covariant)" : "")
         << ", ";
    }

    os << std::endl;
  }

  void writeFunctionsInClass(std::ostream& os,
                             ga::ClassNameFn& className,
                             ga::FuncNameFn& funcName,
                             const ga::VirtualFunctionAnalysis& vfuns,
                             const ga::ClassAnalysis::value_type& clazz,
                             bool withOverridden)
  {
    os << "Class " << className(clazz.first) << " "
       << clazz.second.virtualFunctions().size() << "\n" << std::endl;

    for (ga::FunctionKeyType vfnId : clazz.second.virtualFunctions())
    {
      const ga::VirtualFunctionDesc& vfn = vfuns.at(vfnId);

      os << "  Function " << funcName(vfnId)
         << " #" << vfnId.key
         << (vfn.isPureVirtual() ? " (pure virtual)" : "")
         << std::endl;

      printFunctions(os, "Overriden by", className, funcName, vfuns, vfn.overriders(), false);

      if (withOverridden)
        printFunctions(os, "Overrides", className, funcName, vfuns, vfn.overridden());
    }

    os << std::endl;
  }
}

namespace Rose::Common::Analysis
{

ClassFilterFn noClassFilter()
{
  return [](ClassKeyType) { return true; };
}

ClassFilterFn baseName(const ClassAnalysis&)
{
  return noClassFilter();
}

void classHierarchyDot(std::ostream& os,
                       ClassNameFn& nameOf,
                       ClassFilterFn include,
                       const ClassAnalysis& classes)
{
  static constexpr const char* classColor = "color=blue";

  dotHeader(os, "digraph", "\"Class Relationships\"");

  bottomUpTraversal(classes,
                    [&os, &nameOf, include](const ClassAnalysis::value_type& elem)
                    {
                      if (include(elem.first))
                        node(os, elem.first, nameOf(elem.first), classColor);
                    });

  dotInheritance(os, classes, include);
  dotFooter(os);
}

void virtualFunctionsTxt(std::ostream& os,
                         ClassNameFn& className,
                         FuncNameFn& funcName,
                         ClassFilterFn include,
                         const ClassAnalysis& classes,
                         const VirtualFunctionAnalysis& vfuns,
                         bool withOverridden)
{
  topDownTraversal(classes,
                   [&os, &className, &funcName, &vfuns, &include, withOverridden]
                   (const ClassAnalysis::value_type& elem)
                   {
                     if (!include(elem.first))
                       return;

                     writeFunctionsInClass(os, className, funcName, vfuns, elem, withOverridden);
                   });
}

void classHierarchyJson(std::ostream& os,
                        ClassNameFn& className,
                        FuncNameFn& funcName,
                        ClassNameFn& classId,
                        FuncNameFn& funcId,
                        ClassFilterFn include,
                        const ClassAnalysis& classes,
                        const VirtualFunctionAnalysis* vfuns)
{
  os << "{\"classes\":[";
  bool firstClass = true;

  for (const ClassAnalysis::value_type& clazz : classes)
  {
    if (!include(clazz.first))
      continue;

    if (!firstClass) os << ",";
    firstClass = false;

    os << "{\"id\":" << JsonString{classId(clazz.first)}
       << ",\"name\":" << JsonString{className(clazz.first)}
       << ",\"abstract\":" << (clazz.second.abstractClass() ? "true" : "false")
       << ",\"ancestors\":[";

    bool firstAncestor = true;
    for (const InheritanceDesc& ancestor : clazz.second.ancestors())
    {
      if (!include(ancestor.getClass()))
        continue;

      if (!firstAncestor) os << ",";
      firstAncestor = false;

      os << "{\"id\":" << JsonString{classId(ancestor.getClass())}
         << ",\"name\":" << JsonString{className(ancestor.getClass())}
         << ",\"virtual\":" << (ancestor.isVirtual() ? "true" : "false")
         << ",\"direct\":" << (ancestor.isDirect() ? "true" : "false")
         << "}";
    }

    os << "],\"virtualFunctions\":[";
    bool firstFunction = true;
    for (FunctionKeyType fn : clazz.second.virtualFunctions())
    {
      if (!firstFunction) os << ",";
      firstFunction = false;

      os << "{\"id\":" << JsonString{funcId(fn)}
         << ",\"name\":" << JsonString{funcName(fn)};

      if (vfuns)
      {
        const auto pos = vfuns->find(fn);
        if (pos != vfuns->end())
        {
          os << ",\"pureVirtual\":" << (pos->second.isPureVirtual() ? "true" : "false")
             << ",\"overrides\":[";

          bool firstOverride = true;
          for (const OverrideDesc& ov : pos->second.overridden())
          {
            if (!firstOverride) os << ",";
            firstOverride = false;

            os << "{\"id\":" << JsonString{funcId(ov.function())}
               << ",\"name\":" << JsonString{funcName(ov.function())}
               << ",\"covariant\":" << (ov.covariantReturn() ? "true" : "false")
               << "}";
          }

          os << "],\"overriddenBy\":[";
          bool firstOverrider = true;
          for (const OverrideDesc& ov : pos->second.overriders())
          {
            if (!firstOverrider) os << ",";
            firstOverrider = false;

            os << "{\"id\":" << JsonString{funcId(ov.function())}
               << ",\"name\":" << JsonString{funcName(ov.function())}
               << ",\"covariant\":" << (ov.covariantReturn() ? "true" : "false")
               << "}";
          }
          os << "]";
        }
      }

      os << "}";
    }

    os << "]}";
  }

  os << "]}" << std::endl;
}

} // namespace Rose::Common::Analysis
