// An example ROSE plugin
#ifndef CODETHORN_CLASS_HIERARCHY_ANALYSIS_H
#define CODETHORN_CLASS_HIERARCHY_ANALYSIS_H 1

#include <vector>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <iostream>

#include <ROSE_DEPRECATED.h>

#include "RoseCompatibility.h"

//~ #include "Label.h"
//~ #include "CodeThornLib.h"


//~ #include "FunctionId.h"


namespace CodeThorn
{
  using CompatibilityBridge = RoseCompatibilityBridge;
  extern Sawyer::Message::Facility logger; // from "CodeThornLib.h"
}

namespace CodeThorn
{
  using CompatibilityBridge = RoseCompatibilityBridge;
  using CodeThorn::ASTRootType;
  using CodeThorn::CastKeyType;
  using CodeThorn::ClassKeyType;
  using CodeThorn::ClassNameFn;
  using CodeThorn::DataMemberType;
  using CodeThorn::FuncNameFn;
  using CodeThorn::FunctionKeyType;
  using CodeThorn::FunctionPredicate;
  using CodeThorn::FunctionTypeKeyType;
  using CodeThorn::IsNotNull;
  using CodeThorn::RoseCompatibilityBridge;
  using CodeThorn::STRIP_MODIFIER_ALIAS;
  using CodeThorn::TypeKeyType;
  using CodeThorn::VarNameFn;
  using CodeThorn::VariableKeyType;
  using CodeThorn::getClassCastInfo;
  using CodeThorn::inheritanceEdges;
}

/*******
  How to use the Class Hierarchy Analysis

  sample code to set up the analyses:
  void runClassHierarchyAnalysis( CodeThorn::VariableIdMapping& varmap,
                                  const CodeThorn::FunctionIdMapping& funmap,
                                  SgProject* proj
                                )
  {
    // varmap's and funmap's lifetime must exceed the lifetime of rcb
    const RoseCompatibilityBridge     rcb{varmap, funmap};
    const ClassAnalysis               classAnalysis = analyzeClasses(rcb, proj);
    const VirtualFunctionCallAnalysis vfnAnalysis = virtualFunctionAnalysis(rcb, classAnalysis);

    ...
  }

********/


namespace
{
  // auxiliary functions

  inline
  auto msgTrace() -> decltype(CodeThorn::logger[Sawyer::Message::TRACE])
  {
    return CodeThorn::logger[Sawyer::Message::TRACE];
  }

  inline
  auto msgInfo() -> decltype(CodeThorn::logger[Sawyer::Message::INFO])
  {
    return CodeThorn::logger[Sawyer::Message::INFO];
  }

  inline
  auto msgWarn() -> decltype(CodeThorn::logger[Sawyer::Message::WARN])
  {
    return CodeThorn::logger[Sawyer::Message::WARN];
  }

  inline
  auto msgError() -> decltype(CodeThorn::logger[Sawyer::Message::ERROR])
  {
    return CodeThorn::logger[Sawyer::Message::ERROR];
  }

  inline
  auto msgFatal() -> decltype(CodeThorn::logger[Sawyer::Message::FATAL])
  {
    return CodeThorn::logger[Sawyer::Message::FATAL];
  }
}


namespace CodeThorn
{

///
struct InheritanceDesc : std::tuple<ClassKeyType, bool, bool>
{
  using base = std::tuple<ClassKeyType, bool, bool>;
  using base::base;

  ClassKeyType getClass()       const { return std::get<0>(*this); }
  bool         isVirtual()      const { return std::get<1>(*this); }
  bool         isDirect()       const { return std::get<2>(*this); }

  void setDirect(bool v) { std::get<2>(*this) = v; }
};

struct OverrideDesc : std::tuple<FunctionKeyType, bool>
{
  using base = std::tuple<FunctionKeyType, bool>;
  using base::base;

  FunctionKeyType function()        const { return std::get<0>(*this); }
  bool            covariantReturn() const { return std::get<1>(*this); }

  /// returns true of an object adjustment is needed upon return.
  /// this should be a subset of covariantReturn() [only if the original
  /// and the new return types have different primary base classes],
  /// but currently we use the same value.
  bool            adjustReturnObj() const { return std::get<1>(*this); }
};

using OverrideContainer = std::vector<OverrideDesc>;

using VirtualFunctionDescBase = std::tuple<ClassKeyType, bool, OverrideContainer, OverrideContainer>;

struct VirtualFunctionDesc : VirtualFunctionDescBase
{
  using base = VirtualFunctionDescBase;

  VirtualFunctionDesc() = delete;

  ~VirtualFunctionDesc()                                     = default;
  VirtualFunctionDesc(const VirtualFunctionDesc&)            = default;
  VirtualFunctionDesc(VirtualFunctionDesc&&)                 = default;
  VirtualFunctionDesc& operator=(const VirtualFunctionDesc&) = default;
  VirtualFunctionDesc& operator=(VirtualFunctionDesc&&)      = default;

  VirtualFunctionDesc(ClassKeyType clKey, bool pure)
  : base(clKey, pure, OverrideContainer{}, OverrideContainer{})
  {}

  /// returns a the class where this function is declared
  ClassKeyType classId() const { return std::get<0>(*this); }

  //
  bool isPureVirtual() const { return std::get<1>(*this); }

  /// returns a (const) reference to all functions that are overriding this function
  /// \{
  const OverrideContainer& overriders() const { return std::get<2>(*this); }
  OverrideContainer&       overriders()       { return std::get<2>(*this); }
  /// \}

  /// returns a (const) reference to all functions that this function overrides
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
    using VirtualFunctionContainer       = std::vector<FunctionKeyType>;
    using DataMemberContainer            = std::vector<VariableKeyType>;
    using AncestorContainer              = std::vector<InheritanceDesc>;
    using DescendantContainer            = std::vector<InheritanceDesc>;
    using VirtualBaseOrderContainer      = std::vector<ClassKeyType>;
    using SpecialMemberFunctionContainer = CodeThorn::SpecialMemberFunctionContainer;

    ClassData()                            = default;
    ClassData(const ClassData&)            = default;
    ClassData(ClassData&&)                 = default;
    ClassData& operator=(const ClassData&) = default;
    ClassData& operator=(ClassData&&)      = default;
    ~ClassData()                           = default;

    /// returns a (const) reference to all direct and indirect ancestors
    /// \{
    AncestorContainer&         ancestors()        { return allAncestors; }
    const AncestorContainer&   ancestors()  const { return allAncestors; }
    /// \}

    /// returns a (const) reference to all direct and indirect descendants
    /// \{
    DescendantContainer&       descendants()       { return allDescendants; }
    const DescendantContainer& descendants() const { return allDescendants; }
    /// \}

    /// returns a (const) reference to all virtual functions declared by this class
    /// \{
    VirtualFunctionContainer&       virtualFunctions()       { return allVirtualFunctions; }
    const VirtualFunctionContainer& virtualFunctions() const { return allVirtualFunctions; }
    /// \}

    /// returns a (const) reference to virtual base class construction/destruction order
    /// \{
    VirtualBaseOrderContainer&       virtualBaseClassOrder()       { return virtualBaseOrder; }
    const VirtualBaseOrderContainer& virtualBaseClassOrder() const { return virtualBaseOrder; }
    /// \}

    /// returns a (const) reference to all data members declared by this class
    /// \{
    DataMemberContainer&       dataMembers()       { return allDataMembers; }
    const DataMemberContainer& dataMembers() const { return allDataMembers; }
    /// \}

    /// property indicating whether this class declares at least one virtual function
    bool declaresVirtualFunctions() const   { return virtualFunctions().size(); }

    /// property indicating whether this class inherits at least one virtual function
    /// \{
    void inheritsVirtualFunctions(bool val) { hasInheritedVirtualMethods = val;  }
    bool inheritsVirtualFunctions() const   { return hasInheritedVirtualMethods; }
    /// \}

    /// returns true, iff the class requires a virtual table (vtable).
    /// \details
    ///   a vtable is required if a class uses virtual inheritance or virtual functions.
    /// \note
    ///   a vtable is a common but not the only way to implement the C++ object model.
    bool hasVirtualTable() const;

    /// returns true, iff this class directly or indirectly uses virtual inheritance.
    bool hasVirtualInheritance() const;

    /// returns true, iff this inherits or declares virtual functions.
    bool hasVirtualFunctions() const;

    /// property indicating that is an abstract class
    /// \note the initial information is taken from the frontend (AST)
    ///       and not computed by the vtable generator.
    /// \{
    bool abstractClass()          const { return hasAbstractMethods; }
    void abstractClass(bool bval)       { hasAbstractMethods = bval; }
    /// \}

    const SpecialMemberFunctionContainer&
    specialMemberFunctions() const;

    SpecialMemberFunctionContainer&
    specialMemberFunctions();

#if COPY_CTOR_ANALYSIS
    SpecialMemberFunctionContainer
    specialMemberFunctions(SpecialMemberFunction::Kind knd) const;
#endif /*COPY_CTOR_ANALYSIS*/

  private:
    AncestorContainer              allAncestors;
    DescendantContainer            allDescendants;
    VirtualFunctionContainer       allVirtualFunctions;
    DataMemberContainer            allDataMembers;
    SpecialMemberFunctionContainer allSpecialMemberFunctions;
    VirtualBaseOrderContainer      virtualBaseOrder;

    bool                           hasInheritedVirtualMethods = false;
    bool                           hasAbstractMethods = false;
};

class ClassAnalysis;

using ClassAnalysisInfoBase = std::tuple<const ClassAnalysis*, ClassKeyType>;

struct ClassAnalysisInfo : ClassAnalysisInfoBase
{
  using base = ClassAnalysisInfoBase;
  using base::base;

  const ClassAnalysis* analysis() { return std::get<0>(*this); }
  ClassKeyType         key()      { return std::get<1>(*this); }
};

std::ostream& operator<<(std::ostream&, ClassAnalysisInfo);


using ClassAnalysisBase = std::unordered_map<ClassKeyType, ClassData>;

/// holds data about all classes in a program
class ROSE_DEPRECATED("use Rose::Source::Analysis::CppHierarchyAnalysis") ClassAnalysis : ClassAnalysisBase
{
  public:
    using base = ClassAnalysisBase;

    explicit
    ClassAnalysis(bool fullTranslUnit = false, std::uint32_t languageVersion = 199711L)
    : base(), completeTranslationUnit(fullTranslUnit), langVersion(languageVersion)
    {}

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
    /// \{
          mapped_type& at (const key_type& k);
    const mapped_type& at (const key_type& k) const;
    /// \}

    /// convenience function to access the map using a SgClassDefinition reference.
    const mapped_type& at(const SgClassDefinition& clsdef) const;

    /// adds an inheritance edge to both classes \p descendant and \p ancestorKey
    /// \param descendant the entry for the descendant class
    /// \param ancestorKey the key of the ancestor class
    /// \param isVirtual indicates if the inheritance is virtual
    /// \param isDirect indicates if \p descendant and \p ancestorKey are child and parent
    void
    addInheritanceEdge(value_type& descendant, ClassKeyType ancestorKey, bool isVirtual, bool isDirect);

    /// adds an inheritance edge to the \p descendant class and \p ancestor class.
    void
    addInheritanceEdge(value_type& descendant, const InheritanceDesc& ancestor);

    /// returns true, iff \p ancestorKey is a (direct or indirect) base class
    /// of \p descendantKey.
    /// \details
    ///   returns false when ancestorKey == descendantKey
    bool
    isBaseOf(ClassKeyType ancestorKey, ClassKeyType descendantKey) const;

    /// returns true, iff \p ancestorKey is a (direct or indirect) virtual base class
    /// of \p descendantKey.
    bool
    isVirtualBaseOf(ClassKeyType ancestorKey, ClassKeyType descendantKey) const;

    /// returns a list of concrete descendant classes of \p classkey (i.e., const SgClassDefinition*)
    std::vector<InheritanceDesc>
    concreteDescendants(ClassKeyType classKey) const;

    /// returns true if this class was built from the AST root (SgProject)
    /// returns false if this class was built specifically to represent
    ///   the inheritance hierarchy of a single class.
    bool containsAllClasses() const { return completeTranslationUnit; }

    /// returns a ClassAnalysisInfo object that can be used for writing information
    ///   to a stream.
    /// \begincode
    /// ClassAnalysis classAnalysis = ...
    /// ClassKeyType  somekey = ...
    /// std::cout << classAnalysis.classInfo(someKey) << std::endl;
    /// \endcode
    ClassAnalysisInfo classInfo(ClassKeyType classKey) const;

    /// returns the version identifier of the standard
    std::uint32_t languageStandard() const { return langVersion; }

    static constexpr std::uint32_t Cxx98 = 199711L;
    static constexpr std::uint32_t Cxx11 = 201103L;
    static constexpr std::uint32_t Cxx14 = 201402L;
    static constexpr std::uint32_t Cxx17 = 201703L;
    static constexpr std::uint32_t Cxx20 = 202002L;
    static constexpr std::uint32_t Cxx23 = 202302L;
    static constexpr std::uint32_t Cxx26 = 202600L; // \todo needs correction

  private:
    bool          completeTranslationUnit;
    std::uint32_t langVersion;
};


/// describes cast as from type to to type
struct CastDesc : std::tuple<TypeKeyType, TypeKeyType>
{
  using base = std::tuple<TypeKeyType, TypeKeyType>;
  using base::base;
};

}

namespace std
{
  template<> struct hash<CodeThorn::CastDesc>
  {
    std::size_t operator()(const CodeThorn::CastDesc& dsc) const noexcept
    {
      std::size_t h1 = std::hash<const void*>{}(std::get<0>(dsc));
      std::size_t h2 = std::hash<const void*>{}(std::get<1>(dsc));

      return h1 ^ (h2 >> 4);
    }
  };
}

namespace CodeThorn
{

/// collects casts and program locations where they occur
class ROSE_DEPRECATED("use ROSE Source analysis APIs") CastAnalysis : std::unordered_map<CastDesc, std::vector<CastKeyType> >
{
  public:
    using base = std::unordered_map<CastDesc, std::vector<CastKeyType> >;
    using base::base;

    using base::value_type;
    using base::key_type;
    using base::iterator;
    using base::const_iterator;
    using base::begin;
    using base::end;
    using base::operator[];
    using base::at;
    using base::find;
    using base::emplace;
    using base::size;
};


/// stores the results of virtual function analysis by Id
class ROSE_DEPRECATED("use Rose::Common::Analysis::VirtualFunctionAnalysis") VirtualFunctionAnalysis : private std::unordered_map<FunctionKeyType, VirtualFunctionDesc>
{
  public:
    using base = std::unordered_map<FunctionKeyType, VirtualFunctionDesc>;
    using base::base;

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
    /// \{
          mapped_type& at (const key_type& k);
    const mapped_type& at (const key_type& k) const;
    /// \}

    /// returns a predicate testing if a function is virtual
    /// \note the lifetime of the VirtualFunctionAnalysis object must exceed
    ///       the lifetime of the predicate.
    FunctionPredicate virtualFunctionTest() const;
};


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
AnalysesTuple analyzeClassesAndCasts(const CompatibilityBridge& compat, ASTRootType n);
AnalysesTuple analyzeClassesAndCasts(ASTRootType n);
/// \}

/// collects the class hierarchy from a project
/// \note
///    It is preferred to use analyzeClassesFromMemoryPool, which
///    returns a complete set of classes in ROSE.
/// \{
ClassAnalysis analyzeClasses(const CompatibilityBridge& compat, ASTRootType n);
ClassAnalysis analyzeClasses(ASTRootType n);
ClassAnalysis analyzeClass(ClassKeyType n);
/// \}

/// collects the class hierarchy from memory pools
ClassAnalysis analyzeClassesFromMemoryPool();

/// functions type that are used for the class hierarchy traversals
/// \{
using ClassAnalysisFn      = std::function<void(ClassAnalysis::value_type&)>;
using ClassAnalysisConstFn = std::function<void(const ClassAnalysis::value_type&)>;
/// \}


/// computes function overriders for all virtual functions
/// \{
VirtualFunctionAnalysis
analyzeVirtualFunctions(const CompatibilityBridge& compat, const ClassAnalysis& classes, bool normalizedSignature = false);

VirtualFunctionAnalysis
analyzeVirtualFunctions(const ClassAnalysis& classes, bool normalizedSignature = false);
/// \}

/// implements a top down traversal of the class hierarchy
/// \details
///    calls \p fn for each class in \p all exactly once. Guarantees that \p fn
///    on a base class is called before \p fn on a derived class.
/// \{
void topDownTraversal  (ClassAnalysis& all, ClassAnalysisFn fn);
void topDownTraversal  (const ClassAnalysis& all, ClassAnalysisConstFn fn);
/// \}

/// implements a bottom up traversal of the class hierarchy
/// \details
///    calls \p fn for each class in \p all exactly once. Guarantees that \p fn
///    on a derived class is called before \p fn on a base class.
/// \{
void bottomUpTraversal (ClassAnalysis& all, ClassAnalysisFn fn);
void bottomUpTraversal (const ClassAnalysis& all, ClassAnalysisConstFn fn);
/// \}

/// implements an unordered traversal of the class hierarchy
/// \details
///   \p fn is called for each class exactly once
/// \{
void unorderedTraversal(ClassAnalysis& all, ClassAnalysisFn fn);
void unorderedTraversal(const ClassAnalysis& all, ClassAnalysisConstFn fn);
/// \}

//
// tools for querying special member functions

/// a predicate type to querying special member functions
using SpecialMemberFunctionPredicate = std::function<bool(const SpecialMemberFunction&)>;

/// Generate predicates for querying special member functions.
/// \param  anyOp if false only user-defined operators match;
///         otherwise user-defined and compiler-generated match.
/// \result a predicate testing special member functions.
/// \{
SpecialMemberFunctionPredicate isCopyCtor(bool anyOp = true);
SpecialMemberFunctionPredicate isCopyAssign(bool anyOp = true);
SpecialMemberFunctionPredicate isMoveCtor(bool anyOp = true);
SpecialMemberFunctionPredicate isMoveAssign(bool anyOp = true);
SpecialMemberFunctionPredicate isDefaultCtor(bool anyOp = true);
SpecialMemberFunctionPredicate isDtor(bool anyOp = true);
SpecialMemberFunctionPredicate isUserDefinedCtor();
/// \}


/// Query special member functions
/// \{
bool hasSpecialFunction(const SpecialMemberFunctionContainer& specials, SpecialMemberFunctionPredicate pred);
bool hasSpecialFunction(const ClassAnalysis& all, ClassKeyType clazz, SpecialMemberFunctionPredicate pred);
/// \}


//
// convenience functors

struct IsDirect
{
  bool operator()(const InheritanceDesc& desc) const { return desc.isDirect(); }
};


}
#endif /* CLASS_HIERARCHY_ANALYSIS_H */
