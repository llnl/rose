// An example ROSE plugin
#ifndef ROSE_Common_Analysis_ClassHierarchyAnalysis_H
#define ROSE_Common_Analysis_ClassHierarchyAnalysis_H 1

#include <RoseFirst.h>

#include <vector>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <iostream>

#include <Rose/Common/Analysis/AnalysisTypes.h>
#include <Rose/Common/Analysis/AbstractCompatibilityBridge.h>

/*******
  How to use the Class Hierarchy Analysis

  sample code to set up the analyses:
  void runClassHierarchyAnalysis(SgProject* proj)
  {
    Rose::Source::Analysis::CppHierarchyAnalysis    classAnalysis = Rose::Source::Analysis::analyzeCppClasses(Rose::Source::Analysis::rootToKey(proj));
    Rose::Common::Analysis::VirtualFunctionAnalysis vfnAnalysis = Rose::Source::Analysis::analyzeCppVirtualFunctions(classAnalysis);

    ...
  }

********/


namespace
{
  // auxiliary functions

  inline
  auto msgTrace() -> std::ostream&
  {
    return std::cerr << "[TRACE] ";;
  }

  inline
  auto msgInfo() -> std::ostream&
  {
    return std::cerr << "[INFO]  ";
  }

  inline
  auto msgWarn() -> std::ostream&
  {
    return std::cerr << "[WARN]  ";
  }

  inline
  auto msgError() -> std::ostream&
  {
    return std::cerr << "[ERROR] ";
  }

  inline
  auto msgFatal() -> std::ostream&
  {
    return std::cerr << "[FATAL] ";
  }
}


namespace Rose::Common::Analysis
{

/// Describes one inheritance relationship for a class.
struct InheritanceDesc : std::tuple<ClassKeyType, bool, bool>
{

  /// Base tuple type used for storage.
  using base = std::tuple<ClassKeyType, bool, bool>;
  using base::base;

  /// Returns the ancestor class key.
  /// \return class key for the ancestor in this inheritance relationship.
  ClassKeyType getClass()       const { return std::get<0>(*this); }

  /// Returns true when this relationship uses virtual inheritance.
  /// \return true if the inheritance edge is virtual.
  bool         isVirtual()      const { return std::get<1>(*this); }

  /// Returns true when this relationship is a direct base-class edge.
  /// \return true if the relationship is a direct base edge.
  bool         isDirect()       const { return std::get<2>(*this); }

  /// Sets whether this relationship is direct.
  /// \param v true if the relationship is direct.
  void setDirect(bool v) { std::get<2>(*this) = v; }
};

/// Describes an override relationship for one virtual function.
/// \details
///   An override descriptor is stored on both sides of the relationship:
///   in VirtualFunctionDesc::overriders() of the overridden function and in
///   VirtualFunctionDesc::overridden() of the overriding function.
///
///   A declared override (the overriding function is declared in a class that
///   derives from the class declaring the overridden function) has an invalid
///   witness key. An implied override has a valid witness: the class at which a
///   function inherited along the non-virtual inheritance chain first meets a
///   function of a virtual base that it implements (e.g., in Java a superclass
///   method implementing an interface method of a subclass, JLS 8.4.8.1). The
///   overriding function's class is then not derived from the overridden
///   function's class; only the witness is. Implied overrides are only
///   computed when AbstractCompatibilityBridge::inheritedMethodsImplementVirtualBaseMethods
///   returns true.
struct OverrideDesc : std::tuple<FunctionKeyType, bool, ClassKeyType>
{

  /// Base tuple type used for storage.
  using base = std::tuple<FunctionKeyType, bool, ClassKeyType>;
  using base::base;

  /// Constructs a descriptor for a declared override (no witness).
  /// \param fn overriding or overridden function key.
  /// \param covariant true if the override uses a covariant return type.
  OverrideDesc(FunctionKeyType fn, bool covariant)
  : base(fn, covariant, ClassKeyType{})
  {}

  /// Returns the overriding or overridden function key.
  /// \return function key stored by this override descriptor.
  FunctionKeyType function()        const { return std::get<0>(*this); }

  /// Returns true when the override uses a covariant return type.
  /// \return true if this relationship uses a covariant return.
  bool            covariantReturn() const { return std::get<1>(*this); }

  /// returns true of an object adjustment is needed upon return.
  /// this should be a subset of covariantReturn() [only if the original
  /// and the new return types have different primary base classes],
  /// but currently we use the same value.
  /// \return true if a return-object adjustment is required.
  bool            adjustReturnObj() const { return std::get<1>(*this); }

  /// Returns the witness class of an implied override.
  /// \return class key of the class at which the inherited function and the
  ///         virtual base function meet; an invalid key for declared overrides.
  ClassKeyType    witness()         const { return std::get<2>(*this); }

  /// Returns true when this is an implied override (see class description).
  /// \return true if witness() is a valid class key.
  bool            isImplied()       const { return isValidKey(witness()); }
};

/// Container of override descriptors.
using OverrideContainer = std::vector<OverrideDesc>;

/// Tuple storage for virtual function metadata.
using VirtualFunctionDescBase = std::tuple<ClassKeyType, bool, OverrideContainer, OverrideContainer>;

/// Stores override metadata for one virtual function.
struct VirtualFunctionDesc : VirtualFunctionDescBase
{

  /// Base tuple type used for storage.
  using base = VirtualFunctionDescBase;

  /// Disabled because a virtual function descriptor needs a declaring class.
  VirtualFunctionDesc() = delete;

  /// Default destructor.
  ~VirtualFunctionDesc()                                     = default;

  /// Default copy constructor.
  /// \param other descriptor to copy.
  VirtualFunctionDesc(const VirtualFunctionDesc& other)      = default;

  /// Default move constructor.
  /// \param other descriptor to move from.
  VirtualFunctionDesc(VirtualFunctionDesc&& other)           = default;

  /// Default copy assignment.
  /// \param other descriptor to copy.
  /// \return reference to this descriptor.
  VirtualFunctionDesc& operator=(const VirtualFunctionDesc& other) = default;

  /// Default move assignment.
  /// \param other descriptor to move from.
  /// \return reference to this descriptor.
  VirtualFunctionDesc& operator=(VirtualFunctionDesc&& other)      = default;

  /// Constructs a descriptor for a virtual function declared in \p clKey.
  /// \param clKey class that declares the virtual function.
  /// \param pure true if the function is pure virtual or abstract.
  VirtualFunctionDesc(ClassKeyType clKey, bool pure)
  : base(clKey, pure, OverrideContainer{}, OverrideContainer{})
  {}

  /// returns a the class where this function is declared
  /// \return class key for the declaring class.
  ClassKeyType classId() const { return std::get<0>(*this); }

  /// Returns true when the represented function is pure virtual or abstract.
  /// \return true if this virtual function is pure virtual or abstract.
  bool isPureVirtual() const { return std::get<1>(*this); }

  /// returns a (const) reference to all functions that are overriding this function
  /// \return mutable or const container of overriding functions.
  /// \{
  const OverrideContainer& overriders() const { return std::get<2>(*this); }
  OverrideContainer&       overriders()       { return std::get<2>(*this); }

  /// \}

  /// returns a (const) reference to all functions that this function overrides
  /// \return mutable or const container of overridden functions.
  /// \{
  OverrideContainer&       overridden()       { return std::get<3>(*this); }
  const OverrideContainer& overridden() const { return std::get<3>(*this); }

  /// \}

  /// returns a (const) reference to all virtual functions that this function hides
  /// \details
  ///   a function from an ancestor is hidden, if it has the same signature
  ///   but a different, non co-variant return type.
  /// \{
  //~ OverrideContainer&       shadows()       { return std::get<4>(*this); }
  //~ const OverrideContainer& shadows() const { return std::get<4>(*this); }

  /// \}
};


/// holds data a class in a program
class ClassData
{
  public:

    /// Container of virtual function keys declared by a class.
    using VirtualFunctionContainer       = std::vector<FunctionKeyType>;

    /// Container of data member keys declared by a class.
    using DataMemberContainer            = std::vector<VariableKeyType>;

    /// Container of direct and indirect ancestor descriptors.
    using AncestorContainer              = std::vector<InheritanceDesc>;

    /// Container of direct and indirect descendant descriptors.
    using DescendantContainer            = std::vector<InheritanceDesc>;

    /// Container describing virtual base initialization order.
    using VirtualBaseOrderContainer      = std::vector<ClassKeyType>;

    /// Constructs an empty class data object.
    ClassData()                            = default;

    /// Default copy constructor.
    /// \param other class data to copy.
    ClassData(const ClassData& other)      = default;

    /// Default move constructor.
    /// \param other class data to move from.
    ClassData(ClassData&& other)           = default;

    /// Default copy assignment.
    /// \param other class data to copy.
    /// \return reference to this class data.
    ClassData& operator=(const ClassData& other) = default;

    /// Default move assignment.
    /// \param other class data to move from.
    /// \return reference to this class data.
    ClassData& operator=(ClassData&& other)      = default;

    /// Default destructor.
    ~ClassData()                           = default;

    /// returns a (const) reference to all direct and indirect ancestors
    /// \return mutable or const container of ancestor descriptors.
    /// \{
    AncestorContainer&         ancestors()        { return allAncestors; }
    const AncestorContainer&   ancestors()  const { return allAncestors; }

    /// \}

    /// returns a (const) reference to all direct and indirect descendants
    /// \return mutable or const container of descendant descriptors.
    /// \{
    DescendantContainer&       descendants()       { return allDescendants; }
    const DescendantContainer& descendants() const { return allDescendants; }

    /// \}

    /// returns a (const) reference to all virtual functions declared by this class
    /// \return mutable or const container of virtual function keys.
    /// \{
    VirtualFunctionContainer&       virtualFunctions()       { return allVirtualFunctions; }
    const VirtualFunctionContainer& virtualFunctions() const { return allVirtualFunctions; }

    /// \}

    /// returns a (const) reference to virtual base class construction/destruction order
    /// \return mutable or const container of virtual base class keys.
    /// \{
    VirtualBaseOrderContainer&       virtualBaseClassOrder()       { return virtualBaseOrder; }
    const VirtualBaseOrderContainer& virtualBaseClassOrder() const { return virtualBaseOrder; }

    /// \}

    /// returns a (const) reference to all data members declared by this class
    /// \return mutable or const container of data member keys.
    /// \{
    DataMemberContainer&       dataMembers()       { return allDataMembers; }
    const DataMemberContainer& dataMembers() const { return allDataMembers; }

    /// \}

    /// property indicating whether this class declares at least one virtual function
    /// \return true if at least one virtual function is declared by this class.
    bool declaresVirtualFunctions() const   { return virtualFunctions().size(); }

    /// property indicating whether this class inherits at least one virtual function
    /// \param val true if the class inherits virtual functions.
    /// \return true if the class inherits virtual functions.
    /// \{
    void inheritsVirtualFunctions(bool val) { hasInheritedVirtualMethods = val;  }
    bool inheritsVirtualFunctions() const   { return hasInheritedVirtualMethods; }

    /// \}

    /// returns true, iff the class requires a virtual table (vtable).
    /// \details
    ///   a vtable is required if a class uses virtual inheritance or virtual functions.
    /// \note
    ///   a vtable is a common but not the only way to implement the C++ object model.
    /// \return true if the analysis considers this class to require a vtable.
    bool hasVirtualTable() const;

    /// returns true, iff this class directly or indirectly uses virtual inheritance.
    /// \return true if any ancestor relationship uses virtual inheritance.
    bool hasVirtualInheritance() const;

    /// returns true, iff this inherits or declares virtual functions.
    /// \return true if this class declares or inherits virtual functions.
    bool hasVirtualFunctions() const;

    /// property indicating that is an abstract class
    /// \param bval true if this class is abstract.
    /// \return true if this class is abstract.
    /// \note the initial information is taken from the frontend (AST)
    ///       and not computed by the vtable generator.
    /// \{
    bool abstractClass()          const { return hasAbstractMethods; }
    void abstractClass(bool bval)       { hasAbstractMethods = bval; }

    /// \}

  private:
    AncestorContainer              allAncestors;
    DescendantContainer            allDescendants;
    VirtualFunctionContainer       allVirtualFunctions;
    DataMemberContainer            allDataMembers;
    VirtualBaseOrderContainer      virtualBaseOrder;

    bool                           hasInheritedVirtualMethods = false;
    bool                           hasAbstractMethods = false;
};

class ClassAnalysis;

/// Tuple storage for streaming diagnostics about an analyzed class.
using ClassAnalysisInfoBase = std::tuple<const ClassAnalysis*, ClassKeyType, const AbstractCompatibilityBridge*>;

/// Bundles class analysis state needed to print one class in diagnostics.
struct ClassAnalysisInfo : ClassAnalysisInfoBase
{

  /// Base tuple type used for storage.
  using base = ClassAnalysisInfoBase;
  using base::base;

  /// Returns the class analysis that owns the class key.
  /// \return pointer to the owning analysis.
  const ClassAnalysis*               analysis() { return std::get<0>(*this); }

  /// Returns the described class key.
  /// \return class key being described.
  ClassKeyType                       key()      { return std::get<1>(*this); }

  /// Returns the compatibility bridge used to name frontend objects.
  /// \return pointer to the compatibility bridge.
  const AbstractCompatibilityBridge* compat()   { return std::get<2>(*this); }
};

/// Writes a diagnostic representation of class analysis information.
/// \param os destination stream.
/// \param info diagnostic object to write.
/// \return \p os after writing.
std::ostream& operator<<(std::ostream& os, ClassAnalysisInfo info);


/// Base map type storing class data by class key.
using ClassAnalysisBase = std::unordered_map<ClassKeyType, ClassData>;

/// holds data about all classes in a program
class ClassAnalysis : ClassAnalysisBase
{
  public:

    /// Base map type used for storage.
    using base = ClassAnalysisBase;

    /// Constructs an analysis and records whether it covers a complete translation unit.
    /// \param fullTranslUnit true if the analysis covers the full translation unit.
    /// \param languageVersion language standard identifier used for language-specific rules.
    explicit
    ClassAnalysis(bool fullTranslUnit)
    : base(), completeTranslationUnit(fullTranslUnit)
    {}

    /// Constructs an empty analysis that does not claim complete-program coverage.
    ClassAnalysis() = default;

    using base::value_type;
    using base::mapped_type;
    using base::key_type;
    using base::begin;
    using base::end;
    using base::iterator;
    using base::const_iterator;
    using base::operator[];
    using base::emplace;
    using base::find;
    using base::size;
    using base::clear;

    /// replaces base::at with functions that can provide better diagnostics
    ///   in case \p k is not known.
    /// \param k class key to look up.
    /// \return mutable or const class data for \p k.
    /// \{
          mapped_type& at (const key_type& k);
    const mapped_type& at (const key_type& k) const;

    /// \}
    /// convenience function to access the map using a SgClassDefinition reference.
    //const mapped_type& at(const SgClassDefinition& clsdef) const;

    /// adds an inheritance edge to both classes \p descendant and \p ancestorKey
    /// \param descendant the entry for the descendant class
    /// \param ancestorKey the key of the ancestor class
    /// \param isVirtual indicates if the inheritance is virtual
    /// \param isDirect indicates if \p descendant and \p ancestorKey are child and parent
    /// \return nothing.
    void
    addInheritanceEdge(value_type& descendant, ClassKeyType ancestorKey, bool isVirtual, bool isDirect);

    /// adds an inheritance edge to the \p descendant class and \p ancestor class.
    /// \param descendant the entry for the descendant class.
    /// \param ancestor descriptor for the ancestor relationship to add.
    /// \return nothing.
    void
    addInheritanceEdge(value_type& descendant, const InheritanceDesc& ancestor);

    /// returns true, iff \p ancestorKey is a (direct or indirect) base class
    /// of \p descendantKey.
    /// \param ancestorKey potential base class.
    /// \param descendantKey potential derived class.
    /// \return true if \p ancestorKey is a direct or indirect base of \p descendantKey.
    /// \details
    ///   returns false when ancestorKey == descendantKey
    bool
    isBaseOf(ClassKeyType ancestorKey, ClassKeyType descendantKey) const;

    /// returns true, iff \p ancestorKey is a (direct or indirect) virtual base class
    /// of \p descendantKey.
    /// \param ancestorKey potential virtual base class.
    /// \param descendantKey potential derived class.
    /// \return true if \p ancestorKey is a direct or indirect virtual base of \p descendantKey.
    bool
    isVirtualBaseOf(ClassKeyType ancestorKey, ClassKeyType descendantKey) const;

    /// returns a list of concrete descendant classes of \p classkey (i.e., const SgClassDefinition*)
    /// \param classKey class whose descendants are requested.
    /// \return descriptors for concrete descendant classes known to the analysis.
    std::vector<InheritanceDesc>
    concreteDescendants(ClassKeyType classKey) const;

    /// property indicating whether this analysis was built for the whole AST/program
    /// or just to represent the inheritance hierarchy of a single class.
    /// \param  b true if whole program, false otherwise
    /// \return   true if whole program, false otherwise
    /// \{
    bool containsAllClasses()       const { return completeTranslationUnit; }
    void containsAllClasses(bool b)       { completeTranslationUnit = b; }

    /// \}

    /// returns a ClassAnalysisInfo object that can be used for writing information
    ///   to a stream.
    /// \begincode
    /// ClassAnalysis classAnalysis = ...
    /// ClassKeyType  somekey = ...
    /// std::cout << classAnalysis.classInfo(someKey) << std::endl;
    /// \endcode
    /// \param classKey class key to describe.
    /// \return diagnostic helper object for \p classKey.
    ClassAnalysisInfo classInfo(ClassKeyType classKey) const;


    /// Returns the frontend compatibility bridge associated with this analysis.
    /// \return compatibility bridge used by this analysis.
    virtual const AbstractCompatibilityBridge& compatibilityBridge() const = 0;

  private:
    bool completeTranslationUnit = false;
};


/// Describes a cast from one type key to another type key.
struct CastDesc : std::tuple<TypeKeyType, TypeKeyType>
{

  /// Base tuple type used for storage.
  using base = std::tuple<TypeKeyType, TypeKeyType>;
  using base::base;
};

}


namespace Rose::Common::Analysis
{


/// stores the results of virtual function analysis by Id
class VirtualFunctionAnalysis : private std::unordered_map<FunctionKeyType, VirtualFunctionDesc>
{
  public:

    /// Base map type used for storage.
    using base = std::unordered_map<FunctionKeyType, VirtualFunctionDesc>;

    /// Constructs an empty virtual function analysis using \p compatibilityBridge for diagnostics.
    /// \param compatibilityBridge bridge used for names and diagnostics.
    explicit
    VirtualFunctionAnalysis(const AbstractCompatibilityBridge& compatibilityBridge);

    /// Default destructor.
    ~VirtualFunctionAnalysis()                                    = default;

    /// Default move constructor.
    /// \param other analysis to move from.
    VirtualFunctionAnalysis(VirtualFunctionAnalysis&& other)      = default;

    /// Default move assignment.
    /// \param other analysis to move from.
    /// \return reference to this analysis.
    VirtualFunctionAnalysis& operator=(VirtualFunctionAnalysis&& other) = default;

  private:

    /// Disabled because a compatibility bridge is required.
    VirtualFunctionAnalysis()                                          = delete;

    /// Disabled because the analysis stores a bridge reference.
    /// \param other analysis that would be copied if copying were enabled.
    VirtualFunctionAnalysis(const VirtualFunctionAnalysis& other)      = delete;

    /// Disabled because the analysis stores a bridge reference.
    /// \param other analysis that would be copied if copy assignment were enabled.
    /// \return would return this analysis if copy assignment were enabled.
    VirtualFunctionAnalysis& operator=(const VirtualFunctionAnalysis& other) = delete;

  public:
    using base::value_type;
    using base::key_type;
    using base::mapped_type;
    using base::iterator;
    using base::const_iterator;
    using base::begin;
    using base::end;
    using base::operator[];
    //~ using base::at;
    using base::find;
    using base::emplace;
    using base::size;

    /// replaces base::at with functions that can provide better diagnostics
    ///   in case \p k is not known.
    /// \param k function key to look up.
    /// \return mutable or const virtual function descriptor for \p k.
    /// \{
          mapped_type& at (const key_type& k);
    const mapped_type& at (const key_type& k) const;

    /// \}
    /// returns a predicate testing if a function is virtual
    /// \return predicate that returns true for functions present in this analysis.
    /// \note the lifetime of the VirtualFunctionAnalysis object must exceed
    ///       the lifetime of the predicate.
    FunctionPredicate virtualFunctionTest() const;


    /// Returns the frontend compatibility bridge associated with this analysis.
    /// \return compatibility bridge used by this analysis.
    const AbstractCompatibilityBridge& compatibilityBridge() const { return compat; }

  private:
    const AbstractCompatibilityBridge& compat;
};

#if 0

/// A tuple for both ClassAnalysis and CastAnalysis
struct AnalysesTuple : std::tuple<ClassAnalysis, CastAnalysis>
{
  using base = std::tuple<ClassAnalysis, CastAnalysis>;
  using base::base;

  ClassAnalysis&       classAnalysis()       { return std::get<0>(*this); }
  const ClassAnalysis& classAnalysis() const { return std::get<0>(*this); }

  CastAnalysis&       castAnalysis()       { return std::get<1>(*this); }
  const CastAnalysis& castAnalysis() const { return std::get<1>(*this); }
};



/// collects the class hierarchy and all casts from a project
/// \note
///    It is preferred to use analyzeClassesFromMemoryPool, which
///    returns a complete set of classes in ROSE.
/// \{
AnalysesTuple analyzeClassesAndCasts(const AbstractCompatibilityBridge& compat, ASTRootType n);
AnalysesTuple analyzeClassesAndCasts(ASTRootType n);

/// \}

#endif

/// collects the class hierarchy from a project \p n and stores it into \p classes
/// \param classes analysis object to populate.
/// \param n abstract root key for the AST/project to traverse.
/// \return nothing.
/// \note
///    It is preferred to use analyzeClassesFromMemoryPool, which
///    returns a complete set of classes in ROSE.
void analyzeClasses(ClassAnalysis& classes, ASTRootType n);

/// collects the class hierarchy from memory pools and stores them in \p classes.
/// \param classes analysis object to populate.
/// \return nothing.
void analyzeClassesFromMemoryPool(ClassAnalysis& classes);

/// functions type that are used for the class hierarchy traversals
/// \{
using ClassAnalysisFn      = std::function<void(ClassAnalysis::value_type&)>;
using ClassAnalysisConstFn = std::function<void(const ClassAnalysis::value_type&)>;

/// \}


/// computes function overriders for all virtual functions
/// \param  classes a class analysis containing all relevant classes
/// \param  normalizedSignature true if signatures are normalized (only relevant for C++)
/// \return a class representing virtual functions and their relationships
/// \details
///    Records declared overrides: a function declared in a class overriding a
///    function declared in one of its (direct or indirect) ancestors.
///    If the compatibility bridge enables
///    AbstractCompatibilityBridge::inheritedMethodsImplementVirtualBaseMethods,
///    implied overrides are recorded as well: a function inherited along the
///    non-virtual inheritance chain that implements a function of a virtual
///    base which the class does not override itself (Java: a superclass method
///    implementing an interface method of a subclass). Implied overrides carry
///    the class at which they arise as OverrideDesc::witness and are recorded
///    once, at that class.
///    Every signature match is subject to AbstractCompatibilityBridge::canOverride.
///    A rejected pair is still recorded when the derived function overrides an
///    intermediate function that overrides the base function (transitive rule,
///    e.g., JVMS 5.4.5 for package-private methods).
VirtualFunctionAnalysis
analyzeVirtualFunctions(const ClassAnalysis& classes, bool normalizedSignature = false);

/// implements a top down traversal of the class hierarchy
/// \details
///    calls \p fn for each class in \p all exactly once. Guarantees that \p fn
///    on a base class is called before \p fn on a derived class.
/// \param all class analysis to traverse.
/// \param fn callback invoked for each class entry.
/// \return nothing.
/// \{
void topDownTraversal  (ClassAnalysis& all, ClassAnalysisFn fn);
void topDownTraversal  (const ClassAnalysis& all, ClassAnalysisConstFn fn);

/// \}
/// implements a bottom up traversal of the class hierarchy
/// \details
///    calls \p fn for each class in \p all exactly once. Guarantees that \p fn
///    on a derived class is called before \p fn on a base class.
/// \param all class analysis to traverse.
/// \param fn callback invoked for each class entry.
/// \return nothing.
/// \{
void bottomUpTraversal (ClassAnalysis& all, ClassAnalysisFn fn);
void bottomUpTraversal (const ClassAnalysis& all, ClassAnalysisConstFn fn);

/// \}
/// implements an unordered traversal of the class hierarchy
/// \details
///   \p fn is called for each class exactly once
/// \param all class analysis to traverse.
/// \param fn callback invoked for each class entry.
/// \return nothing.
/// \{
void unorderedTraversal(ClassAnalysis& all, ClassAnalysisFn fn);
void unorderedTraversal(const ClassAnalysis& all, ClassAnalysisConstFn fn);

/// \}

//
// convenience functors

struct IsDirect
{

  /// Returns true when \p desc describes a direct inheritance edge.
  /// \param desc inheritance descriptor to test.
  /// \return true if \p desc is a direct edge.
  bool operator()(const InheritanceDesc& desc) const { return desc.isDirect(); }
};


}
#endif /* CLASS_HIERARCHY_ANALYSIS_H */
