#ifndef ROSE_Source_Analysis_CppLayoutAnalysis_H
#define ROSE_Source_Analysis_CppLayoutAnalysis_H 1

#include <RoseFirst.h>

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include <boost/variant.hpp>

#include <Rose/Common/Analysis/ClassHierarchyAnalysis.h>
#include <Rose/Source/Analysis/CppCompatibility.h>

namespace Rose::Source::Analysis
{

/// Tuple storage for an object-layout base subobject entry.
using SubobjectBase = std::tuple<Rose::Common::Analysis::ClassKeyType, bool, bool>;

/// Describes a base class subobject in a C++ object layout.
struct Subobject : SubobjectBase
{

  /// Base tuple type used for storage.
  using base = SubobjectBase;
  using base::base;

  /// Returns the base class represented by this subobject.
  /// \return class key for the represented base class.
  Rose::Common::Analysis::ClassKeyType baseClass()    const { return std::get<0>(*this); }

  /// Returns true when this is a virtual base subobject.
  /// \return true if this subobject is a virtual base.
  bool                 isVirtualBase() const { return std::get<1>(*this); }

  /// Returns true when this is a direct base subobject.
  /// \return true if this subobject is a direct base.
  bool                 isDirectBase()  const { return std::get<2>(*this); }
};

/// Tuple storage for an object-layout data field entry.
using FieldBase = std::tuple<Rose::Common::Analysis::VariableKeyType>;

/// Describes a data member field in a C++ object layout.
struct Field : FieldBase
{

  /// Base tuple type used for storage.
  using base = FieldBase;
  using base::base;

  /// Returns the variable key for this field.
  /// \return variable key represented by this field.
  Rose::Common::Analysis::VariableKeyType id() const { return std::get<0>(*this); }
};

/// Tuple storage for an object-layout vtable pointer entry.
using VTableBase = std::tuple<Rose::Common::Analysis::ClassKeyType, bool>;

/// Describes a vtable pointer entry in a C++ object layout.
struct VTable : VTableBase
{

  /// Base tuple type used for storage.
  using base = VTableBase;
  using base::base;

  /// Returns the class associated with this vtable pointer.
  /// \return class key associated with this vtable pointer.
  Rose::Common::Analysis::ClassKeyType associatedClass() const { return std::get<0>(*this); }

  /// Returns true when this vtable pointer is the primary vtable.
  /// \return true if this is the primary vtable pointer.
  bool                 primaryVTable()   const { return std::get<1>(*this); }
};

/// Variant for all element kinds that can appear in an object layout.
using ObjectLayoutElement = boost::variant<Subobject, Field, VTable>;

/// Tuple storage for a positioned object-layout element.
using ObjectLayoutEntryBase = std::tuple<std::size_t, ObjectLayoutElement>;

/// Describes one object-layout element at an offset.
struct ObjectLayoutEntry : ObjectLayoutEntryBase
{

  /// Base tuple type used for storage.
  using base = ObjectLayoutEntryBase;
  using base::base;

  /// Returns the byte-like layout offset.
  /// \return offset associated with this object layout entry.
  std::size_t                offset()  const { return std::get<0>(*this); }

  /// Returns the layout element at this offset.
  /// \return object layout element stored at this offset.
  const ObjectLayoutElement& element() const { return std::get<1>(*this); }
};

/// Stores the ordered object layout entries for one class.
class ObjectLayout : private std::vector<ObjectLayoutEntry>
{
  public:

    /// Base vector type used for storage.
    using base = std::vector<ObjectLayoutEntry>;
    using base::value_type;
    using base::iterator;
    using base::const_iterator;
    using base::begin;
    using base::end;
    using base::emplace_back;
    using base::at;

    /// Returns true when this layout belongs to an abstract class.
    /// \return true if this layout belongs to an abstract class.
    bool abstractClass() const { return hasAbstractMethods; }

    /// Sets whether this layout belongs to an abstract class.
    /// \param bval true if this layout belongs to an abstract class.
    void abstractClass(bool bval) { hasAbstractMethods = bval; }

  private:
    bool hasAbstractMethods = false;
};

/// Base map type storing object layouts by class key.
using CppLayoutAnalysisBase = std::unordered_map<Rose::Common::Analysis::ClassKeyType, ObjectLayout>;

/// Stores computed C++ object layouts by class key.
class CppLayoutAnalysis : private CppLayoutAnalysisBase
{
  public:

    /// Base map type used for storage.
    using base = CppLayoutAnalysisBase;
    using base::value_type;
    using base::mapped_type;
    using base::key_type;
    using base::begin;
    using base::end;
    using base::iterator;
    using base::const_iterator;
    using base::emplace;
    using base::find;
    using base::size;
    using base::clear;

    /// Returns the layout for \p k, with diagnostics when the key is unknown.
    /// \param k class key whose object layout is requested.
    /// \return mutable object layout for \p k.
    mapped_type& at(const key_type& k);

    /// Returns the layout for \p k, with diagnostics when the key is unknown.
    /// \param k class key whose object layout is requested.
    /// \return object layout for \p k.
    const mapped_type& at(const key_type& k) const;
};

/// Compatibility alias for computed C++ object layouts.
using ObjectLayoutContainer = CppLayoutAnalysis;

/// Computes object layouts for all classes in \p all.
/// \param all class analysis to lay out.
/// \param onlyClassesWithVTable true to compute layouts only for classes requiring a vtable.
/// \return computed object layouts keyed by class.
CppLayoutAnalysis
computeObjectLayouts(const Rose::Common::Analysis::ClassAnalysis& all, bool onlyClassesWithVTable = true);

/// Tuple storage for a virtual-call-offset vtable entry.
using VirtualCallOffsetBase = std::tuple<std::ptrdiff_t, Rose::Common::Analysis::ClassKeyType>;

/// Describes a vtable entry containing a virtual call offset.
struct VirtualCallOffset : VirtualCallOffsetBase
{

  /// Base tuple type used for storage.
  using base = VirtualCallOffsetBase;
  using base::base;

  /// Returns the offset value.
  /// \return virtual call offset value.
  std::ptrdiff_t        offset() const { return std::get<0>(*this); }

  /// Returns the class associated with this offset.
  /// \return class key associated with this offset.
  Rose::Common::Analysis::ClassKeyType associatedClass() const { return std::get<1>(*this); }
};

/// Tuple storage for a virtual-base-offset vtable entry.
using VirtualBaseOffsetBase = std::tuple<std::ptrdiff_t, Rose::Common::Analysis::ClassKeyType>;

/// Describes a vtable entry containing a virtual base offset.
struct VirtualBaseOffset : VirtualBaseOffsetBase
{

  /// Base tuple type used for storage.
  using base = VirtualBaseOffsetBase;
  using base::base;

  /// Returns the offset value.
  /// \return virtual base offset value.
  std::ptrdiff_t        offset() const { return std::get<0>(*this); }

  /// Returns the class associated with this offset.
  /// \return class key associated with this offset.
  Rose::Common::Analysis::ClassKeyType associatedClass() const { return std::get<1>(*this); }
};

/// Tuple storage for an offset-to-top vtable entry.
using OffsetToTopBase = std::tuple<std::ptrdiff_t>;

/// Describes a vtable entry containing an offset-to-top value.
struct OffsetToTop : OffsetToTopBase
{

  /// Base tuple type used for storage.
  using base = OffsetToTopBase;
  using base::base;

  /// Returns the offset-to-top value.
  /// \return offset-to-top value.
  std::ptrdiff_t offset() const { return std::get<0>(*this); }
};

/// Tuple storage for a type-info-pointer vtable entry.
using TypeInfoPointerBase = std::tuple<Rose::Common::Analysis::ClassKeyType>;

/// Describes a vtable entry containing a type-info pointer.
struct TypeInfoPointer : TypeInfoPointerBase
{

  /// Base tuple type used for storage.
  using base = TypeInfoPointerBase;
  using base::base;

  /// Returns the class whose type-info pointer is referenced.
  /// \return class key whose type-info pointer is referenced.
  Rose::Common::Analysis::ClassKeyType classtype() const { return std::get<0>(*this); }
};

/// Tuple storage for a virtual function vtable entry.
using VirtualFunctionEntryBase =
  std::tuple<Rose::Common::Analysis::FunctionKeyType, Rose::Common::Analysis::ClassKeyType, bool, bool, bool>;

/// Describes a virtual function entry in a vtable layout.
struct VirtualFunctionEntry : VirtualFunctionEntryBase
{

  /// Base tuple type used for storage.
  using base = VirtualFunctionEntryBase;
  using base::base;

  /// Returns the function represented by this vtable entry.
  /// \return function key represented by this vtable entry.
  Rose::Common::Analysis::FunctionKeyType function() const { return std::get<0>(*this); }

  /// Returns the class associated with this vtable entry.
  /// \return class key associated with this vtable entry.
  Rose::Common::Analysis::ClassKeyType getClass() const { return std::get<1>(*this); }

  /// Sets the class associated with this vtable entry.
  /// \param val class key to associate with this entry.
  void setClass(Rose::Common::Analysis::ClassKeyType val) { std::get<1>(*this) = val; }

  /// Returns true when a return-object adjustment is required.
  /// \return true if a return-object adjustment is required.
  bool adjustReturnObj() const { return std::get<2>(*this); }

  /// Returns true when this entry refers to a pure virtual function.
  /// \return true if this entry refers to a pure virtual function.
  bool isPureVirtual() const { return std::get<3>(*this); }

  /// Returns true when the compiler must generate this function.
  /// \return true if this function must be compiler-generated.
  bool needsCompilerGeneration() const { return std::get<4>(*this); }

  /// Sets whether the compiler must generate this function.
  /// \param val true if this function must be compiler-generated.
  void needsCompilerGeneration(bool val) { std::get<4>(*this) = val; }
};

/// Variant for all element kinds that can appear in a vtable layout.
using VTableLayoutElement = boost::variant< VirtualCallOffset,
                                            VirtualBaseOffset,
                                            OffsetToTop,
                                            TypeInfoPointer,
                                            VirtualFunctionEntry >;

/// Tuple storage for a contiguous section in a vtable layout.
using VTableSectionBase =
  std::tuple<Rose::Common::Analysis::ClassKeyType, std::uint32_t, std::uint32_t, std::uint32_t, bool>;

/// Describes a vtable section associated with one class subobject.
struct VTableSection : VTableSectionBase
{

  /// Base tuple type used for storage.
  using base = VTableSectionBase;
  using base::base;

  /// Returns the class associated with this vtable section.
  /// \return class key associated with this vtable section.
  Rose::Common::Analysis::ClassKeyType associatedClass() const { return std::get<0>(*this); }

  /// Returns the number of inherited virtual function entries.
  /// \return number of inherited entries.
  std::uint32_t        numInherited()    const { return std::get<1>(*this); }

  /// Returns the total number of virtual function entries.
  /// \return total number of entries.
  std::uint32_t        numTotal()        const { return std::get<2>(*this); }

  /// Returns the section start offset in the vtable.
  /// \return section start offset.
  std::uint32_t        startOffset()     const { return std::get<3>(*this); }

  /// Returns true when this section belongs to a virtual base.
  /// \return true if this section belongs to a virtual base.
  bool                 virtualBase()     const { return std::get<4>(*this); }

  /// Sets the associated class key.
  /// \param val class key to associate with this section.
  void associatedClass(Rose::Common::Analysis::ClassKeyType val) { std::get<0>(*this) = val; }

  /// Sets the number of inherited entries.
  /// \param val inherited entry count.
  void numInherited(std::uint32_t val) { std::get<1>(*this) = val; }

  /// Sets the total number of entries.
  /// \param val total entry count.
  void numTotal(std::uint32_t val) { std::get<2>(*this) = val; }

  /// Sets the section start offset.
  /// \param val section start offset.
  void startOffset(std::uint32_t val) { std::get<3>(*this) = val; }

  /// Sets whether this section belongs to a virtual base.
  /// \param val true if this section belongs to a virtual base.
  void virtualBase(bool val) { std::get<4>(*this) = val; }
};

/// Stores the vtable layout for one class.
class VTableLayout : private std::vector<VTableLayoutElement>
{
  public:

    /// Base vector type used for storage.
    using base = std::vector<VTableLayoutElement>;

    /// Container of vtable sections.
    using VTableSections = std::vector<VTableSection>;

    /// Constructs an empty vtable layout for class \p cl.
    /// \param cl class key associated with this layout.
    explicit VTableLayout(Rose::Common::Analysis::ClassKeyType cl);

    /// Disabled because every vtable layout must have an associated class.
    VTableLayout() = delete;

    using base::value_type;
    using base::iterator;
    using base::const_iterator;
    using base::begin;
    using base::insert;
    using base::end;
    using base::push_back;

    /// Returns mutable vtable section metadata.
    /// \return mutable container of vtable sections.
    VTableSections& vtableSections() { return sections; }

    /// Returns vtable section metadata.
    /// \return container of vtable sections.
    const VTableSections& vtableSections() const { return sections; }

    /// Returns the section associated with virtual base \p key.
    /// \param key virtual base class key.
    /// \return vtable section associated with \p key.
    const VTableSection& virtualBaseSection(Rose::Common::Analysis::ClassKeyType key) const;

    /// Appends and returns a new vtable section.
    /// \return mutable reference to the appended vtable section.
    VTableSection& createVTableSection();

    /// Returns the logical address of the first vtable entry.
    /// \return logical vtable address used by the layout model.
    std::size_t vtableAddress() const;

    /// Returns the class associated with this vtable layout.
    /// \return class key associated with this vtable layout.
    Rose::Common::Analysis::ClassKeyType getClass() const { return clazz; }

    /// Returns true when this layout belongs to an abstract class.
    /// \return true if this layout belongs to an abstract class.
    bool isAbstractClass() const { return isAbstract; }

    /// Sets whether this layout belongs to an abstract class.
    /// \param v true if this layout belongs to an abstract class.
    void isAbstractClass(bool v) { isAbstract = v; }

    /// Returns the vtable element \p i within section \p sec.
    /// \param sec vtable section containing the requested element.
    /// \param i index within \p sec.
    /// \return mutable vtable layout element.
    VTableLayoutElement& at(const VTableSection& sec, std::size_t i);

    /// Returns the vtable element \p i within section \p sec.
    /// \param sec vtable section containing the requested element.
    /// \param i index within \p sec.
    /// \return vtable layout element.
    const VTableLayoutElement& at(const VTableSection& sec, std::size_t i) const;

  private:
    Rose::Common::Analysis::ClassKeyType clazz;
    VTableSections sections;
    bool isAbstract;
};

/// Base map type storing vtable layouts by class key.
using VTableLayoutAnalysisBase = std::unordered_map<Rose::Common::Analysis::ClassKeyType, VTableLayout>;

/// Stores computed C++ vtable layouts by class key.
class VTableLayoutAnalysis : private VTableLayoutAnalysisBase
{
  public:

    /// Base map type used for storage.
    using base = VTableLayoutAnalysisBase;
    using base::value_type;
    using base::mapped_type;
    using base::key_type;
    using base::begin;
    using base::end;
    using base::iterator;
    using base::const_iterator;
    using base::emplace;
    using base::find;
    using base::size;
    using base::clear;

    /// Returns the vtable layout for \p k, with diagnostics when the key is unknown.
    /// \param k class key whose vtable layout is requested.
    /// \return mutable vtable layout for \p k.
    mapped_type& at(const key_type& k);

    /// Returns the vtable layout for \p k, with diagnostics when the key is unknown.
    /// \param k class key whose vtable layout is requested.
    /// \return vtable layout for \p k.
    const mapped_type& at(const key_type& k) const;
};

/// Compatibility alias for computed C++ vtable layouts.
using VTableLayoutContainer = VTableLayoutAnalysis;

/// Computes vtable layouts using the compatibility bridge attached to \p all.
/// \param all class analysis to lay out.
/// \param vfa virtual function analysis for \p all.
/// \return computed vtable layouts keyed by class.
VTableLayoutAnalysis
computeVTableLayouts(const Rose::Common::Analysis::ClassAnalysis& all,
                     const Rose::Common::Analysis::VirtualFunctionAnalysis& vfa);

/// Computes vtable layouts using an explicit C++ compatibility bridge.
/// \param all class analysis to lay out.
/// \param vfa virtual function analysis for \p all.
/// \param compat C++ compatibility bridge to use for frontend queries.
/// \return computed vtable layouts keyed by class.
VTableLayoutAnalysis
computeVTableLayouts(const Rose::Common::Analysis::ClassAnalysis& all,
                     const Rose::Common::Analysis::VirtualFunctionAnalysis& vfa,
                     const CppCompatibility& compat);

} // namespace Rose::Source::Analysis

#endif
