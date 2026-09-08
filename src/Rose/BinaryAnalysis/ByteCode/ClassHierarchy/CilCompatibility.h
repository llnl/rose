
#ifndef ROSE_BinaryAnalysis_ByteCode_ClassHierarchy_CilCompatibility_H
#define ROSE_BinaryAnalysis_ByteCode_ClassHierarchy_CilCompatibility_H 1

#include <RoseFirst.h>
#include <string>

#include <Rose/Common/Analysis/AnalysisTypes.h>
#include <Rose/Common/Analysis/AbstractCompatibilityBridge.h>

namespace Rose::BinaryAnalysis::ByteCode::ClassHierarchy
{

/// a stateless mapping of CIL related AST nodes in ROSE to fundamental functions
///   of the class hierarchy analysis.
class CilCompatibility final : public Rose::Common::Analysis::AbstractCompatibilityBridge
{
  public:

    /// returns the source name of the function represented by \p key.
    /// \param key CIL function key to name.
    /// \return source name for \p key.
    std::string nameOf(Rose::Common::Analysis::FunctionKeyType key) const override;


    /// returns the source name of the class represented by \p key.
    /// \param key CIL class key to name.
    /// \return source name for \p key.
    std::string nameOf(Rose::Common::Analysis::ClassKeyType key) const override;


    /// returns true if the class represented by \p key is an abstract class.
    /// \param key CIL class key to inspect.
    /// \return true if \p key denotes an abstract class.
    bool isAbstract(Rose::Common::Analysis::ClassKeyType key) const override;


    /// returns true if the class represented by \p key is an abstract function.
    /// \param key CIL function key to inspect.
    /// \return true if \p key denotes an abstract function.
    /// \note
    ///   abstract member functions are called pure virtual in C++
    bool isPureVirtual(Rose::Common::Analysis::FunctionKeyType key) const override;


    /// compares the source name of the functions \p lhs and \p rhs.
    /// \param  lhs left-hand side function
    /// \param  rhs right-hand side function
    /// \return -1 iff nameOf(lhs) < nameOf(rhs)
    ///          0 iff nameOf(lhs) == nameOf(rhs)
    ///          1 iff nameOf(lhs) > nameOf(rhs)
    /// \note
    ///   abstract member functions are called pure virtual in C++
    int compareNames(Rose::Common::Analysis::FunctionKeyType lhs, Rose::Common::Analysis::FunctionKeyType rhs) const override;


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
    int compareFunctionsByType(Rose::Common::Analysis::FunctionKeyType lhs, Rose::Common::Analysis::FunctionKeyType rhs, bool exclReturnType = true) const override;


    /// tests if \p drvId has the same or covariant return type with respect to \p basId
    /// \param classes the extracted class analysis
    /// \param basId   ID of a function in a base class
    /// \param drvId   ID of a function in a derived class
    /// \return the return-type relationship between \p basId and \p drvId.
    /// \details
    ///    The function only checks the return type, and does not check whether the classes in which
    ///    the functions \p drvId and \p basId are defined are in a inheritance relationship.
    Rose::Common::Analysis::FunctionTypeRelation
    haveSameOrCovariantReturn(const Rose::Common::Analysis::ClassAnalysis& classes, Rose::Common::Analysis::FunctionKeyType basId, Rose::Common::Analysis::FunctionKeyType drvId) const override;


    /// extracts class information from the memory pool if available
    /// \param classes analysis object to populate.
    /// \return nothing.
    /// \details
    ///    1. For each class in the memory pool an entry in \p classes is created
    ///    2. Adds all member functions declared in a class to the class representation CRep
    ///    3. Adds all data members declared in a class to the class representation CRep
    void extractFromMemoryPool(Rose::Common::Analysis::ClassAnalysis& classes) const override;


    /// extracts class by walking the AST rooted at \p root.
    /// \param classes analysis object to populate.
    /// \param root abstract key for the AST root to traverse.
    /// \return nothing.
    /// \details
    ///    1. For each known class an entry in \p classes is created
    ///    2. Adds all member functions declared in a class to the class representation CRep
    ///    3. Adds all data members declared in a class to the class representation CRep
    void extractFromProject(Rose::Common::Analysis::ClassAnalysis& classes, Rose::Common::Analysis::ASTRootType root) const override;


    /// returns CIL-specific class post-processing.
    /// \return callback for CIL-specific post-processing.
    /// \details
    ///    CIL currently does not need additional processing after the common
    ///    class hierarchy analysis.
    Rose::Common::Analysis::LanguageSpecificFn
    languageSpecificProcessing() const override;


    /// calls the callback function \p fn(derived, base, isvirtual)
    ///   for all direct base classes of \p clkey
    /// \param clkey class whose direct inheritance edges are requested.
    /// \param fn callback invoked for each edge as (derived, base, isVirtual).
    /// \return nothing.
    void inheritanceEdges( Rose::Common::Analysis::ClassKeyType clkey,
                           std::function<void(Rose::Common::Analysis::ClassKeyType, Rose::Common::Analysis::ClassKeyType, bool)> fn
                         ) const override;


    /// returns the type of a non-static data member \p var.
    /// \param var data member key to inspect.
    /// \return summarized data member type properties.
    Rose::Common::Analysis::DataMemberType
    typeOf(Rose::Common::Analysis::VariableKeyType var) const override;
};

}

#endif /* ROSE_BinaryAnalysis_ByteCode_ClassHierarchy_CilCompatibility_H */
