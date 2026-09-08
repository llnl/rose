#ifndef CODETHORN_OBJECT_LAYOUT_ANALYSIS_H
#define CODETHORN_OBJECT_LAYOUT_ANALYSIS_H 1

#include <boost/variant.hpp>

#include "RoseCompatibility.h"
#include "ClassHierarchyAnalysis.h"
// with C++ 17
// #include <variant>


namespace CodeThorn
{

/// describes a subobject in a class hierarchy
using SubobjectBase = std::tuple<ClassKeyType, bool, bool>;
struct Subobject : SubobjectBase
{
  using base = SubobjectBase;
  using base::base;

  std::tuple_element<0, base>::type
  baseClass() const { return std::get<0>(*this); }

  std::tuple_element<1, base>::type
  isVirtualBase() const { return std::get<1>(*this); }

  std::tuple_element<2, base>::type
  isDirectBase() const { return std::get<2>(*this); }
};

/// describes a field in a class
using FieldBase = std::tuple<VariableKeyType>;
struct Field : FieldBase
{
  using base = FieldBase;
  using base::base;

  std::tuple_element<0, base>::type
  id() const { return std::get<0>(*this); }
};

using VTableBase = std::tuple<ClassKeyType, bool>;
struct VTable : VTableBase
{
  using base = VTableBase;
  using base::base;

  std::tuple_element<0, base>::type
  associatedClass() const { return std::get<0>(*this); }

  std::tuple_element<1, base>::type
  primaryVTable() const { return std::get<1>(*this); }
};


// in C++17 use STL
// using ObjectLayoutElement = std::variant<Subobject, Field>;
using ObjectLayoutElement = boost::variant<Subobject, Field, VTable>;

using ObjectLayoutEntryBase = std::tuple<size_t, ObjectLayoutElement>;

struct ObjectLayoutEntry : ObjectLayoutEntryBase
{
  using base = ObjectLayoutEntryBase;
  using base::base;

  size_t                     offset()  const { return std::get<0>(*this); }
  const ObjectLayoutElement& element() const { return std::get<1>(*this); }
};

class ObjectLayout : std::vector<ObjectLayoutEntry>
{
  public:
    using base = std::vector<ObjectLayoutEntry>;
    using base::base;

    using base::value_type;
    using base::iterator;
    using base::const_iterator;
    using base::begin;
    using base::end;
    using base::emplace_back;
    using base::at;

    /// returns true, iff this is an abstract class
    /// \note this information is carried over from ClassData
    /// \{
    bool abstractClass()          const { return hasAbstractMethods; }
    void abstractClass(bool bval)       { hasAbstractMethods = bval; }
    /// \}

  private:
    bool hasAbstractMethods = false;
};

using ObjectLayoutAnalysisBase = std::unordered_map<ClassKeyType, ObjectLayout>;

class ROSE_DEPRECATED("use Rose::Source::Analysis::CppLayoutAnalysis") ObjectLayoutAnalysis : ObjectLayoutAnalysisBase
{
  public:
    using base = ObjectLayoutAnalysisBase;
    using base::base;

    using base::value_type;
    using base::mapped_type;
    using base::key_type;
    using base::begin;
    using base::end;
    using base::iterator;
    using base::const_iterator;
    //~ using base::operator[];
    using base::emplace;
    using base::find;
    using base::size;
    using base::clear;

    /// replaces base::at with functions that can provide better diagnostics
    ///   in case \p k is not known.
    /// \{
          mapped_type& at (const key_type& k);
    const mapped_type& at (const key_type& k) const;
    /// \}
};

/// computes a sample object layout table for classes in \p all
/// \param all                   the result of the class hierarchy analysis
/// \param onlyClassesWithVTable if true, only classes the layout is only computed for classes with a vtable
/// \details
///    The generated object layout assumes an object model that is vtable based.
///    The generated layout is similar but not exactly the same as the IA64 Itanium object model.
ObjectLayoutAnalysis
computeObjectLayouts(const ClassAnalysis& all, bool onlyClassesWithVTable = true);

// \note using declaration for backward compatibility
using ObjectLayoutContainer = ObjectLayoutAnalysis;

using VirtualCallOffsetBase = std::tuple<std::ptrdiff_t, ClassKeyType>;

/// \note part of the IA64 object model, currently not computed
struct VirtualCallOffset : VirtualCallOffsetBase
{
  using base = VirtualCallOffsetBase;
  using base::base;

  std::tuple_element<0, base>::type
  offset() const { return std::get<0>(*this); }

  std::tuple_element<1, base>::type
  assoicatedClass() const { return std::get<1>(*this); }
};

using VirtualBaseOffsetBase = std::tuple<std::ptrdiff_t, ClassKeyType>;

/// \note part of the IA64 object model, currently not computed
struct VirtualBaseOffset : VirtualBaseOffsetBase
{
  using base = VirtualBaseOffsetBase;
  using base::base;

  std::tuple_element<0, base>::type
  offset() const { return std::get<0>(*this); }

  std::tuple_element<1, base>::type
  associatedClass() const { return std::get<1>(*this); }
};


using OffsetToTopBase = std::tuple<std::ptrdiff_t>;

/// \brief the offset to the top of the object (e.g., used for casts to void*)
///        returns the number of v-table entries, between the sub-object and
///        the address of the primary vtable.
struct OffsetToTop : OffsetToTopBase
{
  using base = OffsetToTopBase;
  using base::base;

  /// A return value of 0 indicates the primary object,
  /// a return value of 1 indicates a secondary object.
  std::tuple_element<0, base>::type
  offset() const { return std::get<0>(*this); }
};

using TypeInfoPointerBase = std::tuple<ClassKeyType>;

/// \brief pointer to RTTI
struct TypeInfoPointer : TypeInfoPointerBase
{
  using base = TypeInfoPointerBase;
  using base::base;

  std::tuple_element<0, base>::type
  classtype() const { return std::get<0>(*this); }
};


using VirtualFunctionEntryBase = std::tuple<FunctionKeyType, ClassKeyType, bool, bool, bool>;

/// \brief virtual function pointer
struct VirtualFunctionEntry : VirtualFunctionEntryBase
{
  using base = VirtualFunctionEntryBase;
  using base::base;

  /// returns the function representation
  std::tuple_element<0, base>::type function() const { return std::get<0>(*this); }

  /// property indicating the class where this function was (or will be [x1]) defined.
  /// \details x1: if needsCompilerGeneration is set, the class indicates a compiler
  ///          generated function that has not been generated in the AST.
  ///          In this case function() returns an overriden function in a base class.
  /// \{
  std::tuple_element<1, base>::type getClass() const { return std::get<1>(*this); }
  void setClass(std::tuple_element<1, base>::type val) { std::get<1>(*this) = val; }
  /// \}

  /// returns if the returned object requires a this pointer adjustment compared
  ///   to the original entry in the vtable.
  /// \todo see comment in OverriderDesc
  std::tuple_element<2, base>::type adjustReturnObj() const { return std::get<2>(*this); }

  /// returns true if this is a pure virtual function, making the the class pure virtual.
  std::tuple_element<3, base>::type isPureVirtual() const { return std::get<3>(*this); }

  /// property indicating if the function will be compiler generated
  /// \{
  std::tuple_element<4, base>::type needsCompilerGeneration() const { return std::get<4>(*this); }
  void needsCompilerGeneration(std::tuple_element<4, base>::type val) { std::get<4>(*this) = val; }
  /// \}
};


using VTableLayoutElement = boost::variant< VirtualCallOffset,
                                            VirtualBaseOffset,
                                            OffsetToTop,
                                            TypeInfoPointer,
                                            VirtualFunctionEntry
                                          >;

using VTableSectionBase = std::tuple<ClassKeyType, std::uint32_t, std::uint32_t, std::uint32_t, bool>;

/// A VTableSection corresponds to a segment of a vtable that is associated with a class
///   a direct non-virtual base class, or a virtual base class (direct and indirect).
struct VTableSection : VTableSectionBase
{
  using base = VTableSectionBase;
  using base::base;

  /// returns the associated base class; if no primary base is present associatedClass resolves
  ///   to the current class.
  std::tuple_element<0, base>::type associatedClass() const { return std::get<0>(*this); }
  std::tuple_element<1, base>::type numInherited()    const { return std::get<1>(*this); }
  std::tuple_element<2, base>::type numTotal()        const { return std::get<2>(*this); }
  std::tuple_element<3, base>::type startOffset()     const { return std::get<3>(*this); }
  std::tuple_element<4, base>::type virtualBase()     const { return std::get<4>(*this); }

  void associatedClass(std::tuple_element<0, base>::type val) { std::get<0>(*this) = val; }
  void numInherited(std::tuple_element<1, base>::type val)    { std::get<1>(*this) = val; }
  void numTotal(std::tuple_element<2, base>::type val)        { std::get<2>(*this) = val; }
  void startOffset(std::tuple_element<3, base>::type val)     { std::get<3>(*this) = val; }
  void virtualBase(std::tuple_element<4, base>::type val)     { std::get<4>(*this) = val; }
};

using VTableLayoutBase = std::vector<VTableLayoutElement>;

/// VTable Model
class VTableLayout : VTableLayoutBase
{
  public:
    using base = VTableLayoutBase;
    using VTableSections = std::vector<VTableSection>;

    explicit
    VTableLayout(ClassKeyType cl)
    : base(), clazz(cl), sections(), isAbstract(false)
    {}

    VTableLayout() = delete;

    using base::value_type;
    using base::iterator;
    using base::const_iterator;
    using base::begin;
    using base::insert;
    using base::end;
    //~ using base::emplace_back;
    using base::push_back;

    /// returns all vtable sections
    /// \{
          VTableSections& vtableSections()       { return sections; }
    const VTableSections& vtableSections() const { return sections; }
    /// \}

    /// returns the vtable section associated with a virtual base class \p key
    const VTableSection& virtualBaseSection(ClassKeyType key) const;

    /// creates a new vtable section
    VTableSection& createVTableSection();

    /// returns the vtable base address (the index where a vtable would point at)
    size_t vtableAddress() const;

    /// the class associated with this vtable
    ClassKeyType getClass() const { return clazz; }

    /// property abstractClass
    /// \{
    bool   isAbstractClass() const { return isAbstract; }
    void   isAbstractClass(bool v) { isAbstract = v; }
    /// \}

    /// return the \p i ^th element in vtable section \p sec
    /// \{
          VTableLayoutElement& at(const VTableSection& sec, std::size_t i);
    const VTableLayoutElement& at(const VTableSection& sec, std::size_t i) const;
    /// \}

  private:
    ClassKeyType   clazz;
    VTableSections sections;
    bool           isAbstract;
};


using VTableLayoutAnalysisBase = std::unordered_map<ClassKeyType, VTableLayout>;

/// A collection of vtables for classes that have one
class ROSE_DEPRECATED("use Rose::Source::Analysis::VTableLayoutAnalysis") VTableLayoutAnalysis : VTableLayoutAnalysisBase
{
  public:
    using base = VTableLayoutAnalysisBase;
    using base::base;

    using base::value_type;
    using base::mapped_type;
    using base::key_type;
    using base::begin;
    using base::end;
    using base::iterator;
    using base::const_iterator;
    //~ using base::operator[];
    using base::emplace;
    using base::find;
    using base::size;
    using base::clear;

    /// replaces base::at with functions that can provide better diagnostics
    ///   in case \p k is not known.
    /// \{
          mapped_type& at (const key_type& k);
    const mapped_type& at (const key_type& k) const;
    /// \}
};

// \note using declaration for backward compatibility
using VTableLayoutContainer = VTableLayoutAnalysis;

/// computes a sample virtual function table for classes in \p all
/// \param all the result of the class hierarchy analysis
/// \param vfa result of virtual function analysis
/// \details
///    The generated layout is similar but not exactly the same as the IA64 Itanium object model.
VTableLayoutAnalysis
computeVTableLayouts(const ClassAnalysis& all, const VirtualFunctionAnalysis& vfa);

VTableLayoutAnalysis
computeVTableLayouts(const ClassAnalysis& all, const VirtualFunctionAnalysis& vfa, const CompatibilityBridge& compat);

}

#endif
