#include <sage3basic.h>
#include <Rose/Source/Analysis/CppClassLayoutExport.h>
#include <Rose/Source/Analysis/ExternalMangling.h>

#include <boost/variant.hpp>


#include <nlohmann/json.h>

namespace ga = Rose::Common::Analysis;
namespace sa = Rose::Source::Analysis;

namespace
{
  std::string classId(ga::ClassKeyType key)
  {
    SgClassDefinition* definition = sa::keyToClass(key);
    ASSERT_not_null(definition);
    return sa::mangle(definition->get_declaration());
  }

  std::string functionId(ga::FunctionKeyType key)
  {
    return sa::mangle(sa::keyToFunction(key));
  }

  std::string variableId(ga::VariableKeyType key)
  {
    return sa::mangle(sa::keyToVariable(key));
  }

  void dotHeader(std::ostream& os, const std::string& kind, const std::string& s)
  {
    os << kind << ' ' << s << " {" << std::endl;
  }

  void dotFooter(std::ostream& os)
  {
    os << "}" << std::endl;
  }

  void writeVirtualBaseOrder(std::ostream& os,
                             ga::ClassNameFn& className,
                             const ga::ClassAnalysis::value_type& clazz)
  {
    const ga::ClassData::VirtualBaseOrderContainer& vbases =
      clazz.second.virtualBaseClassOrder();

    if (vbases.empty())
      return;

    os << "Class " << className(clazz.first) << "\n    ";

    for (ga::ClassKeyType basecls : vbases)
      os << className(basecls) << ", ";

    os << '\n' << std::endl;
  }

  struct ObjectLayoutElementPrinter : boost::static_visitor<void>
  {
    ObjectLayoutElementPrinter(std::ostream& out, ga::ClassNameFn& classNamer, ga::VarNameFn& varNamer)
    : os(out), className(classNamer), varName(varNamer)
    {}

    virtual ~ObjectLayoutElementPrinter() = default;

    virtual std::string escapeName(std::string s) const { return s; }

    virtual void operator()(const Rose::Source::Analysis::Subobject& subobj)
    {
      out() << "subobj " << escapeName(className(subobj.baseClass()))
            << (subobj.isVirtualBase() ? " (virtual)" : "");
    }

    virtual void operator()(const Rose::Source::Analysis::Field& fld)
    {
      out() << "field  " << escapeName(varName(fld.id()));
    }

    virtual void operator()(const Rose::Source::Analysis::VTable& vtbl)
    {
      out() << "vtable " << escapeName(className(vtbl.associatedClass()))
            << (vtbl.primaryVTable() ? " (primary)" : "");
    }

    virtual void printEntry(const Rose::Source::Analysis::ObjectLayoutEntry& el)
    {
      boost::apply_visitor(*this, el.element());
    }

    virtual void printClassHeader(const Rose::Source::Analysis::CppLayoutAnalysis::value_type& el)
    {
      out() << "class " << escapeName(className(el.first));
    }

    virtual void printClassFooter() {}

    std::ostream& out() { return os; }

    private:
      std::ostream& os;
      ga::ClassNameFn& className;
      ga::VarNameFn& varName;
  };

  using InheritanceEdgeDotBase = std::tuple<const void*, ga::ClassKeyType, bool, bool>;
  struct InheritanceEdgeDot : InheritanceEdgeDotBase
  {
    using base = InheritanceEdgeDotBase;
    using base::base;

    const void*      subobj() const { return std::get<0>(*this); }
    ga::ClassKeyType clazz()  const { return std::get<1>(*this); }
    bool             isVirtual() const { return std::get<2>(*this); }
    bool             isDirect()  const { return std::get<3>(*this); }
  };

  struct ObjectLayoutElementPrinterDot : ObjectLayoutElementPrinter
  {
    ObjectLayoutElementPrinterDot(std::ostream& out,
                                  ga::ClassNameFn& classNamer,
                                  ga::VarNameFn& varNamer,
                                  ga::ClassFilterFn incl)
    : ObjectLayoutElementPrinter(out, classNamer, varNamer), include(incl), edges(), currentClass()
    {}

    void elemBegin(const void* elem) { out() << "|<p" << reinterpret_cast<std::uintptr_t>(elem) << ">"; }
    void elemEnd() {}

    void classBegin(ga::ClassKeyType elem, bool abstractClass)
    {
      out() << "p" << elem.key << "["
            << (abstractClass ? "fontcolor=red " : "")
            << "label=\"<0>";
    }

    void classEnd() { out() << "\"];" << std::endl; }

    template <class ClassElem>
    void elem(const ClassElem& el)
    {
      elemBegin(&el);
      ObjectLayoutElementPrinter::operator()(el);
      elemEnd();
    }

    void operator()(const Rose::Source::Analysis::Field& el) override { elem(el); }
    void operator()(const Rose::Source::Analysis::VTable& el) override { elem(el); }

    void operator()(const Rose::Source::Analysis::Subobject& el) override
    {
      elem(el);
      if (include(el.baseClass()))
        edges.emplace_back(&el, el.baseClass(), el.isVirtualBase(), el.isDirectBase());
    }

    void printClassHeader(const Rose::Source::Analysis::CppLayoutAnalysis::value_type& el) override
    {
      currentClass = el.first;
      classBegin(currentClass, el.second.abstractClass());
      out() << "=";
      ObjectLayoutElementPrinter::printClassHeader(el);
      out() << "=";
    }

    void printClassFooter() override
    {
      classEnd();

      for (const InheritanceEdgeDot& rel : edges)
      {
        out() << "p" << currentClass.key << ":p" << reinterpret_cast<std::uintptr_t>(rel.subobj())
              << " -> p" << rel.clazz().key << ":0"
              << "[color=" << (rel.isVirtual() ? "red" : "green")
              << (rel.isDirect() ? "" : " style=dotted")
              << "];" << std::endl;
      }

      edges.clear();
      currentClass = ga::ClassKeyType{};
    }

    std::string escapeName(std::string s) const override
    {
      std::string res;
      res.reserve(s.size());
      for (char c : s)
      {
        if (c == '<' || c == '>')
          res += '\\';
        res += c;
      }
      return res;
    }

    private:
      ga::ClassFilterFn include;
      std::vector<InheritanceEdgeDot> edges;
      ga::ClassKeyType currentClass;
  };

  struct ObjectLayoutElementPrinterTxt : ObjectLayoutElementPrinter
  {
    using ObjectLayoutElementPrinter::ObjectLayoutElementPrinter;

    void printEntry(const Rose::Source::Analysis::ObjectLayoutEntry& el) override
    {
      out() << el.offset() << " ";
      ObjectLayoutElementPrinter::printEntry(el);
      out() << std::endl;
    }

    void printClassHeader(const Rose::Source::Analysis::CppLayoutAnalysis::value_type& el) override
    {
      ObjectLayoutElementPrinter::printClassHeader(el);
      if (el.second.abstractClass()) out() << " [abstract]";
      out() << std::endl;
    }
  };

  struct VTableLayoutElementPrinter : boost::static_visitor<void>
  {
    VTableLayoutElementPrinter(std::ostream& out, ga::ClassNameFn& classNamer, ga::FuncNameFn& funcNamer)
    : os(out), className(classNamer), funcName(funcNamer), classKey()
    {}

    virtual ~VTableLayoutElementPrinter() = default;

    virtual std::string escapeName(std::string s) const { return s; }

    virtual void operator()(const Rose::Source::Analysis::VirtualCallOffset&) { out() << "callOfs (todo)"; }
    virtual void operator()(const Rose::Source::Analysis::VirtualBaseOffset&) { out() << "baseOfs (todo)"; }
    virtual void operator()(const Rose::Source::Analysis::OffsetToTop&) { out() << "ofsToTop (todo)"; }
    virtual void operator()(const Rose::Source::Analysis::TypeInfoPointer&) { out() << "typeinfo (todo)"; }

    virtual void operator()(const Rose::Source::Analysis::VirtualFunctionEntry& vfn)
    {
      out() << escapeName(funcName(vfn.function()))
            << " " << vfn.function().key
            << (vfn.getClass() != classKey ? " inh" : "")
            << (vfn.adjustReturnObj() ? " covar" : "")
            << (vfn.needsCompilerGeneration() ? " compgen" : "")
            << (vfn.isPureVirtual() ? " pure" : "");
    }

    virtual void printClassHeader(const Rose::Source::Analysis::VTableLayoutAnalysis::value_type& el)
    {
      classKey = el.first;
      out() << "class " << escapeName(className(classKey))
            << (el.second.isAbstractClass() ? " abstract " : "");
    }

    virtual void printClassFooter() { classKey = ga::ClassKeyType{}; }

    std::ostream& out() { return os; }

    virtual void printEntry(const Rose::Source::Analysis::VTableLayoutElement& el)
    {
      boost::apply_visitor(*this, el);
    }

    private:
      std::ostream& os;
      ga::ClassNameFn& className;
      ga::FuncNameFn& funcName;
      ga::ClassKeyType classKey;
  };

  struct VTableLayoutElementPrinterTxt : VTableLayoutElementPrinter
  {
    using VTableLayoutElementPrinter::VTableLayoutElementPrinter;

    void printEntry(const Rose::Source::Analysis::VTableLayoutElement& el) override
    {
      VTableLayoutElementPrinter::printEntry(el);
      out() << std::endl;
    }

    void printClassFooter() override { out() << std::endl; }

    void printClassHeader(const Rose::Source::Analysis::VTableLayoutAnalysis::value_type& el) override
    {
      VTableLayoutElementPrinter::printClassHeader(el);
      out() << std::endl;
    }
  };

  template<class Printer, class LayoutContainer>
  void prnLayout(Printer&& prn, ga::ClassFilterFn include, const LayoutContainer& cont)
  {
    for (const typename LayoutContainer::value_type& entry : cont)
    {
      if (!include(entry.first)) continue;

      prn.printClassHeader(entry);
      for (const auto& elem : entry.second)
        prn.printEntry(elem);
      prn.printClassFooter();
    }
  }

  struct JsonString
  {
    const std::string& value;
  };

  std::ostream& operator<<(std::ostream& os, JsonString string)
  {
    return os << nlohmann::json(string.value).dump();
  }

  struct ObjectJson : boost::static_visitor<void>
  {
    ObjectJson(std::ostream& out, ga::ClassNameFn& classNamer, ga::VarNameFn& varNamer)
    : os(out), className(classNamer), varName(varNamer)
    {}

    void operator()(const Rose::Source::Analysis::Subobject& sub) const
    {
      os << "{\"kind\":\"subobject\",\"class\":" << JsonString{className(sub.baseClass())}
         << ",\"classId\":" << JsonString{classId(sub.baseClass())}
         << ",\"virtual\":" << (sub.isVirtualBase() ? "true" : "false")
         << ",\"direct\":" << (sub.isDirectBase() ? "true" : "false")
         << "}";
    }

    void operator()(const Rose::Source::Analysis::Field& field) const
    {
      os << "{\"kind\":\"field\",\"id\":" << JsonString{variableId(field.id())}
         << ",\"name\":" << JsonString{varName(field.id())} << "}";
    }

    void operator()(const Rose::Source::Analysis::VTable& vtable) const
    {
      os << "{\"kind\":\"vtable\",\"class\":" << JsonString{className(vtable.associatedClass())}
         << ",\"classId\":" << JsonString{classId(vtable.associatedClass())}
         << ",\"primary\":" << (vtable.primaryVTable() ? "true" : "false")
         << "}";
    }

    std::ostream& os;
    ga::ClassNameFn& className;
    ga::VarNameFn& varName;
  };

  struct VTableJson : boost::static_visitor<void>
  {
    VTableJson(std::ostream& out, ga::ClassNameFn& classNamer, ga::FuncNameFn& funcNamer)
    : os(out), className(classNamer), funcName(funcNamer)
    {}

    void operator()(const Rose::Source::Analysis::VirtualCallOffset& off) const
    {
      os << "{\"kind\":\"virtualCallOffset\",\"offset\":" << off.offset()
         << ",\"class\":" << JsonString{className(off.associatedClass())} << "}";
    }

    void operator()(const Rose::Source::Analysis::VirtualBaseOffset& off) const
    {
      os << "{\"kind\":\"virtualBaseOffset\",\"offset\":" << off.offset()
         << ",\"class\":" << JsonString{className(off.associatedClass())} << "}";
    }

    void operator()(const Rose::Source::Analysis::OffsetToTop& off) const
    {
      os << "{\"kind\":\"offsetToTop\",\"offset\":" << off.offset() << "}";
    }

    void operator()(const Rose::Source::Analysis::TypeInfoPointer& info) const
    {
      os << "{\"kind\":\"typeInfo\",\"class\":" << JsonString{className(info.classtype())}
         << ",\"classId\":" << JsonString{classId(info.classtype())} << "}";
    }

    void operator()(const Rose::Source::Analysis::VirtualFunctionEntry& fn) const
    {
      os << "{\"kind\":\"function\",\"id\":" << JsonString{functionId(fn.function())}
         << ",\"name\":" << JsonString{funcName(fn.function())}
         << ",\"class\":" << JsonString{className(fn.getClass())}
         << ",\"classId\":" << JsonString{classId(fn.getClass())}
         << ",\"covariant\":" << (fn.adjustReturnObj() ? "true" : "false")
         << ",\"compilerGenerated\":" << (fn.needsCompilerGeneration() ? "true" : "false")
         << ",\"pureVirtual\":" << (fn.isPureVirtual() ? "true" : "false")
         << "}";
    }

    std::ostream& os;
    ga::ClassNameFn& className;
    ga::FuncNameFn& funcName;
  };

  using json = nlohmann::json;

  struct LegacyObjectJson : boost::static_visitor<json>
  {
    json operator()(const Rose::Source::Analysis::Subobject& sub) const
    {
      return {{"kind", "sub"}, {"base", classId(sub.baseClass())},
              {"virtual", sub.isVirtualBase()}, {"direct", sub.isDirectBase()}};
    }

    json operator()(const Rose::Source::Analysis::Field& field) const
    {
      sa::CppCompatibility compat;
      return {{"kind", "fld"}, {"id", variableId(field.id())},
              {"name", compat.nameOf(field.id())}};
    }

    json operator()(const Rose::Source::Analysis::VTable& vtable) const
    {
      return {{"kind", "vtable"}, {"class", classId(vtable.associatedClass())},
              {"primary", vtable.primaryVTable()}};
    }
  };

  struct LegacyVTableJson : boost::static_visitor<json>
  {
    json operator()(const Rose::Source::Analysis::VirtualCallOffset&) const
    {
      return {{"kind", "VirtualCallOffset"}};
    }

    json operator()(const Rose::Source::Analysis::VirtualBaseOffset&) const
    {
      return {{"kind", "VirtualBaseOffset"}};
    }

    json operator()(const Rose::Source::Analysis::OffsetToTop&) const
    {
      return {{"kind", "OffsetToTop"}};
    }

    json operator()(const Rose::Source::Analysis::TypeInfoPointer&) const
    {
      return {{"kind", "ti"}};
    }

    json operator()(const Rose::Source::Analysis::VirtualFunctionEntry& function) const
    {
      return {{"kind", "fun"}, {"id", functionId(function.function())},
              {"class", classId(function.getClass())},
              {"covariant", function.adjustReturnObj()},
              {"compgen", function.needsCompilerGeneration()},
              {"abstract", function.isPureVirtual()}};
    }
  };
}

namespace Rose::Source::Analysis
{

void virtualBaseClassInitOrderTxt(std::ostream& os,
                                  ga::ClassNameFn& className,
                                  ga::ClassFilterFn include,
                                  const ga::ClassAnalysis& classes)
{
  topDownTraversal(classes,
                   [&os, &className, &include](const ga::ClassAnalysis::value_type& elem)
                   {
                     if (include(elem.first))
                       writeVirtualBaseOrder(os, className, elem);
                   });
}

void classLayoutTxt(std::ostream& os,
                    ga::ClassNameFn& className,
                    ga::VarNameFn& varName,
                    ga::ClassFilterFn include,
                    const CppLayoutAnalysis& classLayout)
{
  prnLayout(ObjectLayoutElementPrinterTxt{os, className, varName}, include, classLayout);
}

void classLayoutDot(std::ostream& os,
                    ga::ClassNameFn& className,
                    ga::VarNameFn& varName,
                    ga::ClassFilterFn include,
                    const CppLayoutAnalysis& classLayout)
{
  dotHeader(os, "digraph", "\"C++ Class Layout\"");
  os << "rankdir = LR;\n"
     << "node [shape=record];\n";

  prnLayout(ObjectLayoutElementPrinterDot{os, className, varName, include}, include, classLayout);
  dotFooter(os);
}

void vtableLayoutTxt(std::ostream& os,
                     ga::ClassNameFn& className,
                     ga::FuncNameFn& funcName,
                     ga::ClassFilterFn include,
                     const VTableLayoutAnalysis& vtableLayout)
{
  prnLayout(VTableLayoutElementPrinterTxt{os, className, funcName}, include, vtableLayout);
}

void classLayoutJson(std::ostream& os,
                     ga::ClassNameFn& className,
                     ga::VarNameFn& varName,
                     ga::ClassFilterFn include,
                     const CppLayoutAnalysis& classLayout)
{
  os << "{\"classes\":[";
  bool firstClass = true;

  for (const CppLayoutAnalysis::value_type& entry : classLayout)
  {
    if (!include(entry.first))
      continue;

    if (!firstClass) os << ",";
    firstClass = false;

    os << "{\"id\":" << JsonString{classId(entry.first)}
       << ",\"name\":" << JsonString{className(entry.first)}
       << ",\"abstract\":" << (entry.second.abstractClass() ? "true" : "false")
       << ",\"layout\":[";

    bool firstEntry = true;
    for (const ObjectLayoutEntry& elem : entry.second)
    {
      if (!firstEntry) os << ",";
      firstEntry = false;

      os << "{\"offset\":" << elem.offset() << ",\"element\":";
      boost::apply_visitor(ObjectJson{os, className, varName}, elem.element());
      os << "}";
    }

    os << "]}";
  }

  os << "]}" << std::endl;
}

void vtableLayoutJson(std::ostream& os,
                      ga::ClassNameFn& className,
                      ga::FuncNameFn& funcName,
                      ga::ClassFilterFn include,
                      const VTableLayoutAnalysis& vtableLayout)
{
  os << "{\"classes\":[";
  bool firstClass = true;

  for (const VTableLayoutAnalysis::value_type& entry : vtableLayout)
  {
    if (!include(entry.first))
      continue;

    if (!firstClass) os << ",";
    firstClass = false;

    os << "{\"id\":" << JsonString{classId(entry.first)}
       << ",\"name\":" << JsonString{className(entry.first)}
       << ",\"abstract\":" << (entry.second.isAbstractClass() ? "true" : "false")
       << ",\"sections\":[";

    bool firstSection = true;
    for (const VTableSection& sec : entry.second.vtableSections())
    {
      if (!firstSection) os << ",";
      firstSection = false;

      os << "{\"class\":" << JsonString{className(sec.associatedClass())}
         << ",\"classId\":" << JsonString{classId(sec.associatedClass())}
         << ",\"numInherited\":" << sec.numInherited()
         << ",\"numTotal\":" << sec.numTotal()
         << ",\"startOffset\":" << sec.startOffset()
         << ",\"virtualBase\":" << (sec.virtualBase() ? "true" : "false")
         << "}";
    }

    os << "],\"entries\":[";
    bool firstEntry = true;
    for (const VTableLayoutElement& elem : entry.second)
    {
      if (!firstEntry) os << ",";
      firstEntry = false;
      boost::apply_visitor(VTableJson{os, className, funcName}, elem);
    }

    os << "]}";
  }

  os << "]}" << std::endl;
}

void cppClassLayoutJson(std::ostream& os,
                        ga::ClassFilterFn include,
                        const CppLayoutAnalysis& classLayout,
                        const VTableLayoutAnalysis& vtableLayout)
{
  sa::CppCompatibility compat;
  json result = {{"classes", json::array()}};

  for (const CppLayoutAnalysis::value_type& layout : classLayout)
  {
    if (!include(layout.first))
      continue;

    json clazz = {{"id", classId(layout.first)},
                  {"name", compat.nameOf(layout.first)},
                  {"layout", json::array()},
                  {"abstract", layout.second.abstractClass()}};

    for (const ObjectLayoutEntry& entry : layout.second)
      clazz["layout"].push_back(boost::apply_visitor(LegacyObjectJson{}, entry.element()));

    const auto vtable = vtableLayout.find(layout.first);
    if (vtable != vtableLayout.end())
    {
      clazz["vtable"] = json::array();
      for (const VTableLayoutElement& entry : vtable->second)
        clazz["vtable"].push_back(boost::apply_visitor(LegacyVTableJson{}, entry));

      clazz["sections"] = json::array();
      for (const VTableSection& section : vtable->second.vtableSections())
      {
        clazz["sections"].push_back({{"class", classId(section.associatedClass())},
                                      {"inherited", section.numInherited()},
                                      {"size", section.numTotal()},
                                      {"at", section.startOffset()},
                                      {"virtual", section.virtualBase()}});
      }
    }

    result["classes"].push_back(std::move(clazz));
  }

  os << result;
}

} // namespace Rose::Source::Analysis
