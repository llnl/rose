
#ifndef ROSE_Common_Analysis_AbstractCompatibilityBridge_H
#define ROSE_Common_Analysis_AbstractCompatibilityBridge_H 1

#include <Rose/Common/Analysis/AnalysisTypes.h>

namespace Rose::Common::Analysis
{

struct ClassAnalysis;

/// Callback type for language-specific post-processing of a class analysis.
/// \param classes class analysis to post-process.
/// \return nothing.
using LanguageSpecificFn = std::function<void(ClassAnalysis&)>;

/// This is a stateless, abstract base class for implementing some functionality
///    required by the class hierarchy analysis onto different subsets of the AST.
///    e.g., C++ analysis, JVM analysis, ...
class AbstractCompatibilityBridge
{
  public:

    /// Constructs a stateless compatibility bridge.
    AbstractCompatibilityBridge()  = default;

    /// Destroys a compatibility bridge through the base type.
    virtual
    ~AbstractCompatibilityBridge() = default;

  private:

    /// Disabled because bridge implementations are treated as stateless singletons.
    /// \param other source bridge; unused because copying is disabled.
    AbstractCompatibilityBridge(const AbstractCompatibilityBridge& other)      = delete;

    /// Disabled because bridge implementations are treated as stateless singletons.
    /// \param other source bridge; unused because moving is disabled.
    AbstractCompatibilityBridge(AbstractCompatibilityBridge&& other)           = delete;

    /// Disabled because bridge implementations are treated as stateless singletons.
    /// \param other source bridge; unused because copy assignment is disabled.
    /// \return would return this object if assignment were enabled.
    AbstractCompatibilityBridge& operator=(const AbstractCompatibilityBridge& other) = delete;

    /// Disabled because bridge implementations are treated as stateless singletons.
    /// \param other source bridge; unused because move assignment is disabled.
    /// \return would return this object if assignment were enabled.
    AbstractCompatibilityBridge& operator=(AbstractCompatibilityBridge&& other)      = delete;

  public:
    // not sure if clone will be needed..
    // virtual AbstractCompatibilityBridge& clone() const = delete;


    /// returns the source name of the function represented by \p key.
    /// \param key abstract function key to name.
    /// \return frontend-specific source name for \p key.
    virtual
    std::string
    nameOf(FunctionKeyType key) const = 0;


    /// returns the source name of the class represented by \p key.
    /// \param key abstract class key to name.
    /// \return frontend-specific source name for \p key.
    virtual
    std::string
    nameOf(ClassKeyType key) const = 0;


    /// returns true if the class represented by \p key is an abstract class.
    /// \param key abstract class key to test.
    /// \return true if \p key denotes an abstract class.
    virtual
    bool
    isAbstract(ClassKeyType key) const = 0;


    /// returns true if the class represented by \p key is an abstract function.
    /// \param key abstract function key to test.
    /// \return true if \p key denotes an abstract member function.
    /// \note
    ///   abstract member functions are called pure virtual in C++
    virtual
    bool
    isPureVirtual(FunctionKeyType key) const = 0;


    /// compares the source name of the functions \p lhs and \p rhs.
    /// \param  lhs left-hand side function
    /// \param  rhs right-hand side function
    /// \return -1 iff nameOf(lhs) < nameOf(rhs)
    ///          0 iff nameOf(lhs) == nameOf(rhs)
    ///          1 iff nameOf(lhs) > nameOf(rhs)
    /// \note
    ///   abstract member functions are called pure virtual in C++
    virtual
    int
    compareNames(FunctionKeyType lhs, FunctionKeyType rhs) const = 0;


    /// compares the canonical types of functions \p lhs and \p rhs
    /// \param lhs left-hand side function
    /// \param rhs right-hand side function
    /// \param exclReturnType if true the return types of \p lhs and \p rhs
    ///        are not considered (though the return types of potential arguments
    ///        and return types are). This can be useful for obtaining equality of
    ///        functions that have covariant return types.
    /// \return 0 iff lhs and rhs have the same type
    ///          1 iff lhs > rhs
    ///          -1 iff lhs < rhs
    /// \details
    ///    if lhs and rhs are not equal, the ordering is not specified but produces consistent results
    virtual
    int
    compareFunctionsByType(FunctionKeyType lhs, FunctionKeyType rhs, bool exclReturnType = true) const = 0;


    /// tests if \p drvId has the same or covariant return type with respect to \p basId
    /// \param classes the extracted class analysis
    /// \param basId   ID of a function in a base class
    /// \param drvId   ID of a function in a derived class
    /// \return the return-type relationship between \p basId and \p drvId.
    /// \details
    ///    The function only checks the return type, and does not check whether the classes in which
    ///    the functions \p drvId and \p basId are defined are in a inheritance relationship.
    virtual
    FunctionTypeRelation
    haveSameOrCovariantReturn(const ClassAnalysis& classes, FunctionKeyType basId, FunctionKeyType drvId) const = 0;


    /// extracts class information from the memory pool if available
    /// \param classes analysis object to populate with discovered classes.
    /// \details
    ///    1. For each class in the memory pool an entry in \p classes is created
    ///    2. Adds all member functions declared in a class to the class representation CRep
    ///    3. Adds all data members declared in a class to the class representation CRep
    virtual
    void
    extractFromMemoryPool(ClassAnalysis& classes) const = 0;


    /// extracts class by walking the AST rooted at \p root.
    /// \param classes analysis object to populate with discovered classes.
    /// \param root abstract key for the AST root to traverse.
    /// \details
    ///    1. For each known class an entry in \p classes is created
    ///    2. Adds all member functions declared in a class to the class representation CRep
    ///    3. Adds all data members declared in a class to the class representation CRep
    virtual
    void
    extractFromProject(ClassAnalysis& classes, ASTRootType root) const = 0;


    /// returns a function that handles language specific post-processing for
    ///   the extract classes.
    /// \return a function post-processing all classes
    /// \details
    ///    a use case is applying additional analysis rules after the common
    ///    inheritance relationships have been computed.
    virtual
    LanguageSpecificFn
    languageSpecificProcessing() const = 0;


    /// calls the callback function \p fn(derived, base, isvirtual)
    ///   for all direct base classes of \p clkey
    /// \param clkey class whose direct inheritance edges are requested.
    /// \param fn callback invoked for each edge as (derived, base, isVirtual).
    virtual
    void
    inheritanceEdges(ClassKeyType clkey, std::function<void(ClassKeyType, ClassKeyType, bool)> fn) const = 0;


    /// returns the type of a non-static data member \p var.
    /// \param var data member key to inspect.
    /// \return summarized data member type properties.
    virtual
    DataMemberType
    typeOf(VariableKeyType var) const = 0;

    /// returns whether a function with \p fnkey's signature can be implicitly generated in \p clkey.
    /// \param classes class analysis context.
    /// \param clkey class for which implicit generation is tested.
    /// \param fnkey function whose signature is tested.
    /// \return true if the bridge can determine that the function can be implicitly generated.
    /// \note The base implementation is conservative and returns false.
    virtual
    bool
    isAutoGeneratable(const ClassAnalysis& classes, ClassKeyType clkey, FunctionKeyType fnkey) const
    {
      (void)classes;
      (void)clkey;
      (void)fnkey;
      return false;
    }

    /// returns the active language standard as a C++ __cplusplus-style value when available.
    /// \return language standard version, or zero when the bridge cannot provide one.
    virtual
    std::uint32_t
    languageStandard() const { return 0; }

    /// returns whether a method inherited along the non-virtual inheritance chain
    ///   implements an otherwise unimplemented function of a virtual base.
    /// \return true if the language has this rule; the base implementation returns false.
    /// \details
    ///    This is the rule of Java (JLS 8.4.8.1, JVMS 5.4.6) and CIL (ECMA-335 II.12.2):
    ///    for a class C extending B and implementing interface I, a concrete method B.m
    ///    inherited by C implements I.m. C++ has no such rule; the class stays abstract.
    ///    When enabled, analyzeVirtualFunctions records such relationships as implied
    ///    overrides with C as witness (see OverrideDesc::witness).
    /// \note
    ///    Only the rule above is modelled. The selection among default methods of
    ///    unrelated interfaces (JVMS 5.4.6, maximally-specific superinterface methods)
    ///    is not.
    virtual
    bool
    inheritedMethodsImplementVirtualBaseMethods() const { return false; }

    /// returns whether the inherited function \p fnkey qualifies as an implementation
    ///   of a virtual base function (see inheritedMethodsImplementVirtualBaseMethods).
    /// \param fnkey an inherited function that matches a virtual base function by signature.
    /// \return true if \p fnkey may implement the virtual base function.
    /// \details
    ///    Only consulted when inheritedMethodsImplementVirtualBaseMethods returns true.
    ///    A Java bridge returns true for non-abstract public methods (JLS 8.4.8.4:
    ///    an inherited abstract method does not implement; interface methods are
    ///    public, so the implementing method must be public).
    virtual
    bool
    isInheritableImplementation(FunctionKeyType fnkey) const
    {
      (void)fnkey;
      return true;
    }

    /// returns whether \p drv may override \p bas under language rules beyond
    ///   name and signature, such as access control.
    /// \param drv function in a derived class (or inherited by it) that matches \p bas by signature.
    /// \param bas function in an ancestor class.
    /// \return true if the override relation may be recorded for the pair; the base implementation returns true.
    /// \details
    ///    In C++ access does not affect overriding, so the default applies. The JVM
    ///    (JVMS 5.4.5) excludes private methods on either side and lets a
    ///    package-private method be overridden only from the same run-time package.
    ///    If this query rejects a pair, the analysis still records the relation when
    ///    it holds transitively: \p drv overrides an intermediate function that in
    ///    turn overrides \p bas (JVMS 5.4.5, rule (b)). Bridges therefore only need
    ///    to decide the direct relation.
    virtual
    bool
    canOverride(FunctionKeyType drv, FunctionKeyType bas) const
    {
      (void)drv;
      (void)bas;
      return true;
    }

    /// returns whether the virtual function \p fnkey cannot be overridden.
    /// \param fnkey virtual function to inspect.
    /// \return true if \p fnkey is final (e.g., a sealed override in C#); the base implementation returns false.
    /// \details
    ///    The result is recorded in the VirtualFunctionDesc so that clients
    ///    know that the function has no possible overriders. Bridges for
    ///    languages in which a final function is not part of the override
    ///    relation also reject such pairs in canOverride.
    virtual
    bool
    isFinal(FunctionKeyType fnkey) const
    {
      (void)fnkey;
      return false;
    }

    /// returns whether \p drv, declared in \p drvClass, may override \p bas,
    ///   declared in \p basClass, with knowledge of the classes in between.
    /// \param classes class analysis context; inheritance relations are complete.
    /// \param drvClass class that declares \p drv; invalid for an implied override,
    ///        where the declaring class of the inherited function is not known.
    /// \param drv function in a derived class that matches \p bas by signature.
    /// \param basClass class that declares \p bas.
    /// \param bas function in an ancestor class.
    /// \return true if the override relation may be recorded for the pair;
    ///         the base implementation forwards to canOverride(drv, bas).
    /// \details
    ///    Languages whose override rules depend on the classes between \p drvClass
    ///    and \p basClass (e.g., a CIL method that starts a new vtable slot in an
    ///    intermediate class) implement this query. The analysis calls this
    ///    function; it is not necessary to also implement canOverride(drv, bas).
    ///    Rejected pairs are resolved transitively as described for canOverride.
    virtual
    bool
    canOverrideInContext( const ClassAnalysis& classes,
                          ClassKeyType drvClass,
                          FunctionKeyType drv,
                          ClassKeyType basClass,
                          FunctionKeyType bas
                        ) const
    {
      (void)classes;
      (void)drvClass;
      (void)basClass;
      return canOverride(drv, bas);
    }

    // no state
};

} // namespace

#endif /*_ABSTRACT_COMPATIBILITY_BRIDGE_H*/
