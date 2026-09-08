#ifndef ROSE_Source_Analysis_CppCompatibility_H
#define ROSE_Source_Analysis_CppCompatibility_H 1

#include <Rose/Common/Analysis/AbstractCompatibilityBridge.h>
#include <Rose/Common/Analysis/ClassHierarchyAnalysis.h>
#include <Rose/Source/Analysis/CppSpecialMemberAnalysis.h>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <boost/optional/optional.hpp>
#include <boost/utility/string_view.hpp>


struct SgProject;
struct SgDeclarationStatement;
struct SgFunctionDeclaration;
struct SgMemberFunctionDeclaration;
struct SgVariableDeclaration;
struct SgClassDefinition;
struct SgInitializedName;
struct SgType;
struct SgFunctionType;
struct SgClassType;

namespace Rose::Source::Analysis
{

/// Optional C++ class key result.
using Optional = boost::optional<Rose::Common::Analysis::ClassKeyType>;

/// Non-owning string view used for source text ranges.
using StringView = boost::string_view;

/// Tuple storage for a source file location range.
using SourceLocationBase =
  std::tuple<StringView, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t>;

/// Describes a source file range using one-based line and column positions.
struct SourceLocation : SourceLocationBase
{

  /// Base tuple type used for storage.
  using base = SourceLocationBase;
  using base::base;

  /// Returns the source filename.
  /// \return source filename view.
  StringView    file()      const { return std::get<0>(*this); }

  /// Returns the starting line.
  /// \return one-based starting line.
  std::uint32_t startLine() const { return std::get<1>(*this); }

  /// Returns the starting column.
  /// \return one-based starting column.
  std::uint32_t startCol()  const { return std::get<2>(*this); }

  /// Returns the limiting line.
  /// \return one-based limiting line.
  std::uint32_t limitLine() const { return std::get<3>(*this); }

  /// Returns the limiting column.
  /// \return one-based limiting column.
  std::uint32_t limitCol()  const { return std::get<4>(*this); }
};

/// Writes a source location in a compact textual form.
/// \param os destination stream.
/// \param loc source location to write.
/// \return \p os after writing.
std::ostream& operator<<(std::ostream& os, const SourceLocation& loc);

/// Converts an abstract root key to a ROSE project pointer.
/// \param key abstract root key to unwrap.
/// \return ROSE project pointer represented by \p key.
SgProject* keyToRoot(Rose::Common::Analysis::ASTRootType key);

/// Converts a ROSE project pointer to an abstract root key.
/// \param n ROSE project pointer to wrap.
/// \return abstract key for \p n.
Rose::Common::Analysis::ASTRootType rootToKey(SgProject* n);

/// Converts an abstract class key to a ROSE class definition pointer.
/// \param key abstract class key to unwrap.
/// \return ROSE class definition pointer represented by \p key.
SgClassDefinition* keyToClass(Rose::Common::Analysis::ClassKeyType key);

/// Converts a ROSE class definition pointer to an abstract class key.
/// \param n ROSE class definition pointer to wrap.
/// \return abstract key for \p n.
Rose::Common::Analysis::ClassKeyType classToKey(const SgClassDefinition* n);

/// Converts an abstract function key to a ROSE function declaration pointer.
/// \param key abstract function key to unwrap.
/// \return ROSE function declaration pointer represented by \p key.
SgFunctionDeclaration* keyToFunction(Rose::Common::Analysis::FunctionKeyType key);

/// Converts a ROSE function declaration pointer to an abstract function key.
/// \param n ROSE function declaration pointer to wrap.
/// \return abstract key for \p n.
Rose::Common::Analysis::FunctionKeyType functionToKey(const SgFunctionDeclaration* n);

/// Converts an abstract variable key to a ROSE initialized-name pointer.
/// \param key abstract variable key to unwrap.
/// \return ROSE initialized-name pointer represented by \p key.
SgInitializedName* keyToVariable(Rose::Common::Analysis::VariableKeyType key);

/// Converts a ROSE initialized-name pointer to an abstract variable key.
/// \param n ROSE initialized-name pointer to wrap.
/// \return abstract key for \p n.
Rose::Common::Analysis::VariableKeyType variableToKey(const SgInitializedName* n);

/// Converts an abstract function type key to a ROSE function type pointer.
/// \param key abstract function type key to unwrap.
/// \return ROSE function type pointer represented by \p key.
SgFunctionType* keyToFunctionType(Rose::Common::Analysis::FunctionTypeKeyType key);

/// Converts a ROSE function type pointer to an abstract function type key.
/// \param n ROSE function type pointer to wrap.
/// \return abstract key for \p n.
Rose::Common::Analysis::FunctionTypeKeyType functionTypeToKey(const SgFunctionType* n);

/// Converts an abstract type key to a ROSE type pointer.
/// \param key abstract type key to unwrap.
/// \return ROSE type pointer represented by \p key.
SgType* keyToType(Rose::Common::Analysis::TypeKeyType key);

/// Converts a ROSE type pointer to an abstract type key.
/// \param n ROSE type pointer to wrap.
/// \return abstract key for \p n.
Rose::Common::Analysis::TypeKeyType typeToKey(const SgType* n);

/// Implements the Common compatibility interface for C++ ROSE AST nodes.
class CppCompatibility final : public Rose::Common::Analysis::AbstractCompatibilityBridge
{
  public:

    /// Converts a ROSE initialized-name pointer to an abstract variable key.
    /// \param var ROSE initialized-name pointer to wrap.
    /// \return abstract key for \p var.
    Rose::Common::Analysis::VariableKeyType variableId(const SgInitializedName* var) const;

    /// Converts a ROSE variable declaration pointer to an abstract variable key.
    /// \param var ROSE variable declaration pointer to wrap.
    /// \return abstract key for \p var.
    Rose::Common::Analysis::VariableKeyType variableId(const SgVariableDeclaration* var) const;

    /// Converts a ROSE function declaration pointer to an abstract function key.
    /// \param fun ROSE function declaration pointer to wrap.
    /// \return abstract key for \p fun.
    Rose::Common::Analysis::FunctionKeyType functionId(const SgFunctionDeclaration* fun) const;

    /// Returns the source name of a variable represented by \p key.
    /// \param key abstract variable key to name.
    /// \return source name for \p key.
    std::string nameOf(Rose::Common::Analysis::VariableKeyType key) const;

    /// Returns the source name of a function represented by \p key.
    /// \param key abstract function key to name.
    /// \return source name for \p key.
    std::string nameOf(Rose::Common::Analysis::FunctionKeyType key) const override;

    /// Returns the source name of a class represented by \p key.
    /// \param key abstract class key to name.
    /// \return source name for \p key.
    std::string nameOf(Rose::Common::Analysis::ClassKeyType key) const override;

    /// Returns a numeric representation of any abstract key.
    /// \param id abstract key variant to inspect.
    /// \return numeric representation of \p id.
    std::intptr_t numericId(Rose::Common::Analysis::AnyKeyType id) const;

    /// Returns a unique frontend-derived name for any abstract key.
    /// \param id abstract key variant to name.
    /// \return frontend-derived unique name for \p id.
    std::string uniqueName(Rose::Common::Analysis::AnyKeyType id) const;

    /// Returns source location information for any abstract key.
    /// \param id abstract key variant whose source location is requested.
    /// \param resolveInstantiations true to resolve template instantiation locations when supported.
    /// \return source location for \p id.
    /// \note If the source location cannot be resolved, the exact fallback values are implementation-defined.
    SourceLocation location(Rose::Common::Analysis::AnyKeyType id, bool resolveInstantiations = false) const;

    /// Compares function source names.
    /// \param lhs left-hand side function key.
    /// \param rhs right-hand side function key.
    /// \return negative, zero, or positive according to the implementation-defined name ordering.
    int compareNames(Rose::Common::Analysis::FunctionKeyType lhs, Rose::Common::Analysis::FunctionKeyType rhs) const override;

    /// Compares two function types by canonical form.
    /// \param lhs left-hand side function type key.
    /// \param rhs right-hand side function type key.
    /// \param exclReturnType true to ignore top-level return type differences.
    /// \return negative, zero, or positive according to the canonical type ordering.
    int compareFunctionTypes(Rose::Common::Analysis::FunctionTypeKeyType lhs, Rose::Common::Analysis::FunctionTypeKeyType rhs, bool exclReturnType = true) const;

    /// Compares two functions by canonical function type.
    /// \param lhs left-hand side function key.
    /// \param rhs right-hand side function key.
    /// \param exclReturnType true to ignore top-level return type differences.
    /// \return negative, zero, or positive according to the canonical function type ordering.
    int compareFunctionsByType(Rose::Common::Analysis::FunctionKeyType lhs, Rose::Common::Analysis::FunctionKeyType rhs, bool exclReturnType = true) const override;

    /// Compares two non-function types by canonical form.
    /// \param lhs left-hand side type key.
    /// \param rhs right-hand side type key.
    /// \return negative, zero, or positive according to the canonical type ordering.
    int compareTypes(Rose::Common::Analysis::TypeKeyType lhs, Rose::Common::Analysis::TypeKeyType rhs) const;

    /// Tests whether \p drvId has the same or a covariant return type relative to \p basId.
    /// \param classes class hierarchy used to evaluate covariance.
    /// \param basId base-class function key.
    /// \param drvId derived-class function key.
    /// \return relationship between the return types of \p basId and \p drvId.
    Rose::Common::Analysis::FunctionTypeRelation
    haveSameOrCovariantReturn(const Rose::Common::Analysis::ClassAnalysis& classes,
                              Rose::Common::Analysis::FunctionKeyType basId,
                              Rose::Common::Analysis::FunctionKeyType drvId) const override;

    /// Tests whether two type keys denote the same or covariant class type.
    /// \param classes class hierarchy used to evaluate covariance.
    /// \param basId base return type key.
    /// \param drvId derived return type key.
    /// \return relationship between \p basId and \p drvId.
    Rose::Common::Analysis::FunctionTypeRelation
    areSameOrCovariant(const Rose::Common::Analysis::ClassAnalysis& classes,
                       Rose::Common::Analysis::TypeKeyType basId,
                       Rose::Common::Analysis::TypeKeyType drvId) const;

    /// Extracts C++ class information from the ROSE memory pool.
    /// \param classes analysis object to populate.
    /// \return nothing.
    void extractFromMemoryPool(Rose::Common::Analysis::ClassAnalysis& classes) const override;

    /// Extracts C++ class information by traversing the AST rooted at \p root.
    /// \param classes analysis object to populate.
    /// \param root abstract key for the AST root to traverse.
    /// \return nothing.
    void extractFromProject(Rose::Common::Analysis::ClassAnalysis& classes, Rose::Common::Analysis::ASTRootType root) const override;

    /// Returns C++-specific class-analysis post-processing.
    /// \return callback that performs C++-specific post-processing.
    Rose::Common::Analysis::LanguageSpecificFn languageSpecificProcessing() const override;

    /// Calls \p fn for each direct base-class edge of \p clkey.
    /// \param clkey class whose direct inheritance edges are requested.
    /// \param fn callback invoked for each edge as (derived, base, isVirtual).
    /// \return nothing.
    void inheritanceEdges(Rose::Common::Analysis::ClassKeyType clkey,
                          std::function<void(Rose::Common::Analysis::ClassKeyType, Rose::Common::Analysis::ClassKeyType, bool)> fn) const override;

    /// Returns the type summary of a non-static data member.
    /// \param var data member key to inspect.
    /// \return summarized data member type properties.
    Rose::Common::Analysis::DataMemberType typeOf(Rose::Common::Analysis::VariableKeyType var) const override;

    /// Returns true when \p id denotes a pure virtual C++ function.
    /// \param id function key to inspect.
    /// \return true if \p id denotes a pure virtual function.
    bool isPureVirtual(Rose::Common::Analysis::FunctionKeyType id) const override;

    /// Returns true when \p id has an attached function definition.
    /// \param id function key to inspect.
    /// \return true if \p id has an attached definition.
    bool hasDefinition(Rose::Common::Analysis::FunctionKeyType id) const;

    /// Returns true when \p clkey denotes an abstract C++ class.
    /// \param clkey class key to inspect.
    /// \return true if \p clkey denotes an abstract class.
    bool isAbstract(Rose::Common::Analysis::ClassKeyType clkey) const override;

    /// Returns a function key naming callback.
    /// \return callback that maps function keys to names.
    Rose::Common::Analysis::FuncNameFn functionNaming() const;

    /// Returns a class key naming callback.
    /// \return callback that maps class keys to names.
    Rose::Common::Analysis::ClassNameFn classNaming() const;

    /// Returns a variable key naming callback.
    /// \return callback that maps variable keys to names.
    Rose::Common::Analysis::VarNameFn variableNaming() const;

    /// Returns true when \p fnkey can be implicitly generated for \p clkey.
    /// \param all class analysis context.
    /// \param clkey class for which implicit generation is tested.
    /// \param fnkey function whose signature is tested.
    /// \return true if \p fnkey can be implicitly generated for \p clkey.
    bool isAutoGeneratable(const Rose::Common::Analysis::ClassAnalysis& all,
                           Rose::Common::Analysis::ClassKeyType clkey,
                           Rose::Common::Analysis::FunctionKeyType fnkey) const override;

    /// Returns the detected C++ language standard value.
    /// \return C++ standard version as a __cplusplus-style value, or zero if unavailable.
    std::uint32_t languageStandard() const override;

    /// Extracts special member functions declared or generated for \p clazz.
    /// \param clazz class whose special member functions are requested.
    /// \return detected special member function descriptors for \p clazz.
    SpecialMemberFunctionContainer specialMemberFunctions(Rose::Common::Analysis::ClassKeyType clazz) const;

    /// Returns the class that owns \p fn when it can be determined.
    /// \param fn function key to inspect.
    /// \return owning class key, or an empty optional when the owner is unavailable.
    Optional classType(Rose::Common::Analysis::FunctionKeyType fn) const;
};

/// Returns class and type keys used for class-cast analysis of \p tykey.
/// \param tykey type key to inspect.
/// \return pair of class key and canonical type key used for cast analysis.
std::pair<Rose::Common::Analysis::ClassKeyType, Rose::Common::Analysis::TypeKeyType>
getClassCastInfo(Rose::Common::Analysis::TypeKeyType tykey);

/// Returns the class definition represented by a declaration statement.
/// \param n declaration statement to inspect.
/// \return class definition represented by \p n.
SgClassDefinition& getClassDef(const SgDeclarationStatement& n);

/// Returns the class definition associated with a member function declaration.
/// \param n member function declaration to inspect.
/// \return class definition that owns \p n.
SgClassDefinition& getClassDefForFunction(const SgMemberFunctionDeclaration& n);

/// Returns the class definition for a class type when available.
/// \param n class type to inspect.
/// \return class definition for \p n, or null when unavailable.
SgClassDefinition* getClassDefOpt(const SgClassType& n);

/// Returns the class definition associated with an initialized name when available.
/// \param n initialized name to inspect.
/// \return associated class definition, or null when unavailable.
SgClassDefinition* getClassDefOpt(const SgInitializedName& n);

/// Returns the canonical declaration used as a function key.
/// \param fn function declaration to canonicalize.
/// \return canonical declaration used as the function key.
SgFunctionDeclaration& keyDecl(SgFunctionDeclaration& fn);

/// Returns diagnostics explaining why a class key could not be resolved, if available.
/// \param key class key that failed to resolve.
/// \return diagnostic string, or an empty optional when no diagnostic is available.
boost::optional<std::string>
missingDiagnostics(Rose::Common::Analysis::ClassKeyType key);

/// Returns debug information for a class key and up to \p numParents parent nodes.
/// \param key class key to describe.
/// \param numParents maximum number of parent nodes to include.
/// \return human-readable debug information.
std::string dbgInfo(Rose::Common::Analysis::ClassKeyType key, std::size_t numParents = 3);

} // namespace Rose::Source::Analysis

#endif
