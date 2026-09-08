
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

    // no state
};

} // namespace

#endif /*_ABSTRACT_COMPATIBILITY_BRIDGE_H*/
