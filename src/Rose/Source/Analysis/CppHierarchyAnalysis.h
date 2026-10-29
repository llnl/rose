#ifndef ROSE_Source_Analysis_CppHierarchyAnalysis_H
#define ROSE_Source_Analysis_CppHierarchyAnalysis_H 1

#include <RoseFirst.h>

#include <cstdint>
#include <functional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <Rose/Common/Analysis/ClassHierarchyAnalysis.h>
#include <Rose/Source/Analysis/CppCompatibility.h>
#include <Rose/Source/Analysis/CppSpecialMemberAnalysis.h>

class SgProject;
class SgClassDefinition;
class SgFunctionDeclaration;
class SgInitializedName;
class SgVariableDeclaration;
class SgFunctionType;
class SgType;

namespace Rose::Source::Analysis
{
/// Binds Rose::Common::Analysis::ClassAnalysis to C++ ROSE AST nodes and stores C++-specific metadata.
class CppHierarchyAnalysis final : public Rose::Common::Analysis::ClassAnalysis
{
  public:

    /// Base class-analysis type.
    using base = Rose::Common::Analysis::ClassAnalysis;
    using base::base;

    /// Map from class keys to their detected special member functions.
    using SpecialMemberFunctionMap =
      std::unordered_map<Rose::Common::Analysis::ClassKeyType, SpecialMemberFunctionContainer>;

    /// Returns the C++ compatibility bridge.
    /// \return compatibility bridge used by this C++ analysis.
    const CppCompatibility&
    compatibilityBridge() const override;

    /// Converts a ROSE project root pointer to an abstract analysis key.
    /// \param root ROSE project pointer to wrap.
    /// \return abstract key for \p root.
    Rose::Common::Analysis::ASTRootType
    rootId(const SgProject* root) const;

    /// Converts an abstract analysis key back to a ROSE project root pointer.
    /// \param key abstract root key to unwrap.
    /// \return ROSE project pointer represented by \p key.
    SgProject*
    root(Rose::Common::Analysis::ASTRootType key) const;

    /// Converts a ROSE class definition pointer to an abstract analysis key.
    /// \param cls ROSE class definition pointer to wrap.
    /// \return abstract key for \p cls.
    Rose::Common::Analysis::ClassKeyType
    classId(const SgClassDefinition* cls) const;

    /// Converts an abstract class key back to a ROSE class definition pointer.
    /// \param key abstract class key to unwrap.
    /// \return ROSE class definition pointer represented by \p key.
    SgClassDefinition*
    classDefinition(Rose::Common::Analysis::ClassKeyType key) const;

    /// Converts a ROSE function declaration pointer to an abstract analysis key.
    /// \param fun ROSE function declaration pointer to wrap.
    /// \return abstract key for \p fun.
    Rose::Common::Analysis::FunctionKeyType
    functionId(const SgFunctionDeclaration* fun) const;

    /// Converts an abstract function key back to a ROSE function declaration pointer.
    /// \param key abstract function key to unwrap.
    /// \return ROSE function declaration pointer represented by \p key.
    SgFunctionDeclaration*
    functionDeclaration(Rose::Common::Analysis::FunctionKeyType key) const;

    /// Converts a ROSE initialized-name pointer to an abstract variable key.
    /// \param var ROSE initialized-name pointer to wrap.
    /// \return abstract key for \p var.
    Rose::Common::Analysis::VariableKeyType
    variableId(SgInitializedName* var) const;

    /// Converts a ROSE variable declaration pointer to an abstract variable key.
    /// \param var ROSE variable declaration pointer to wrap.
    /// \return abstract key for \p var.
    Rose::Common::Analysis::VariableKeyType
    variableId(SgVariableDeclaration* var) const;

    /// Converts an abstract variable key back to a ROSE initialized-name pointer.
    /// \param key abstract variable key to unwrap.
    /// \return ROSE initialized-name pointer represented by \p key.
    SgInitializedName*
    variableDeclaration(Rose::Common::Analysis::VariableKeyType key) const;

    /// Converts a ROSE function type pointer to an abstract function type key.
    /// \param ty ROSE function type pointer to wrap.
    /// \return abstract key for \p ty.
    Rose::Common::Analysis::FunctionTypeKeyType
    functionTypeId(const SgFunctionType* ty) const;

    /// Converts an abstract function type key back to a ROSE function type pointer.
    /// \param key abstract function type key to unwrap.
    /// \return ROSE function type pointer represented by \p key.
    SgFunctionType*
    functionType(Rose::Common::Analysis::FunctionTypeKeyType key) const;

    /// Converts a ROSE type pointer to an abstract type key.
    /// \param ty ROSE type pointer to wrap.
    /// \return abstract key for \p ty.
    Rose::Common::Analysis::TypeKeyType
    typeId(const SgType* ty) const;

    /// Converts an abstract type key back to a ROSE type pointer.
    /// \param key abstract type key to unwrap.
    /// \return ROSE type pointer represented by \p key.
    SgType*
    type(Rose::Common::Analysis::TypeKeyType key) const;

    /// Returns a printable source name for a ROSE class definition.
    /// \param cls ROSE class definition to name.
    /// \return printable source name for \p cls.
    std::string
    nameOf(const SgClassDefinition* cls) const;

    /// Returns a printable source name for a ROSE function declaration.
    /// \param fun ROSE function declaration to name.
    /// \return printable source name for \p fun.
    std::string
    nameOf(const SgFunctionDeclaration* fun) const;

    /// Returns a printable source name for a ROSE initialized name (field).
    /// \param var ROSE initialized name to name.
    /// \return printable source name for \p var.
    std::string
    nameOf(const SgInitializedName* var) const;

    /// Returns mutable special member function metadata for \p clazz.
    /// \param clazz class key whose metadata is requested.
    /// \return mutable container of special member function descriptors.
    SpecialMemberFunctionContainer&
    specialMemberFunctions(Rose::Common::Analysis::ClassKeyType clazz);

    /// Returns special member function metadata for \p clazz.
    /// \param clazz class key whose metadata is requested.
    /// \return container of special member function descriptors.
    const SpecialMemberFunctionContainer&
    specialMemberFunctions(Rose::Common::Analysis::ClassKeyType clazz) const;

    /// returns the version identifier of the language standard.
    /// \return language standard identifier associated with this analysis.
    std::uint32_t languageStandard() const { return langVersion; }

    /// Sets the version identifier of the language standard.
    /// \param v language standard identifier to record.
    void languageStandard(std::uint32_t v) { langVersion = v; }

    /// C++98 language standard identifier.
    static constexpr std::uint32_t Cxx98 = 199711L;

    /// C++11 language standard identifier.
    static constexpr std::uint32_t Cxx11 = 201103L;

    /// C++14 language standard identifier.
    static constexpr std::uint32_t Cxx14 = 201402L;

    /// C++17 language standard identifier.
    static constexpr std::uint32_t Cxx17 = 201703L;

    /// C++20 language standard identifier.
    static constexpr std::uint32_t Cxx20 = 202002L;

    /// C++23 language standard identifier.
    static constexpr std::uint32_t Cxx23 = 202302L;

    /// Draft C++26 language standard identifier.
    static constexpr std::uint32_t Cxx26 = 202600L;


  private:
    static CppCompatibility compat;

    SpecialMemberFunctionMap allSpecialMemberFunctions;
    std::uint32_t            langVersion = Cxx98;
};

/// Builds a C++ class hierarchy analysis by traversing the AST rooted at \p root.
/// \param root abstract key for the AST root to traverse.
/// \return populated C++ hierarchy analysis.
CppHierarchyAnalysis analyzeCppClasses(const SgProject*);

/// Builds a C++ class hierarchy analysis from the ROSE memory pool.
/// \return populated C++ hierarchy analysis.
CppHierarchyAnalysis analyzeCppClassesFromMemoryPool();

/// Returns true when class \p clazz in \p all has a special member matching \p pred.
/// \param all C++ hierarchy analysis to query.
/// \param clazz class key to inspect.
/// \param pred predicate used to select special member descriptors.
/// \return true if \p clazz has a matching special member function.
bool hasSpecialFunction(const CppHierarchyAnalysis& all,
                        Rose::Common::Analysis::ClassKeyType clazz,
                        SpecialMemberFunctionPredicate pred);

/*
/// Returns true when \p all is a C++ analysis and class \p clazz has a special member matching \p pred.
/// \param all class analysis to query.
/// \param clazz class key to inspect.
/// \param pred predicate used to select special member descriptors.
/// \return true if \p all is a C++ analysis and \p clazz has a matching special member function.
bool hasSpecialFunction(const Rose::Common::Analysis::ClassAnalysis& all,
                        Rose::Common::Analysis::ClassKeyType clazz,
                        SpecialMemberFunctionPredicate pred);
*/

/// Detects and records C++ special member functions for all classes in \p all.
/// Each class is processed after its direct bases and class-valued data-member
/// dependencies.  If a dependency is absent from an incomplete analysis, this
/// reports the dependency and throws rather than using an incomplete result.
/// \param all C++ hierarchy analysis to update.
/// \return nothing.
void analyzeSpecialMemberFunctions(CppHierarchyAnalysis& all);

/// Computes virtual function override relationships for C++ classes.
/// \param classes class analysis containing all relevant classes.
/// \param normalizedSignature true if signatures are already normalized.
/// \return virtual function relationship analysis for \p classes.
Rose::Common::Analysis::VirtualFunctionAnalysis
analyzeCppVirtualFunctions(const Rose::Common::Analysis::ClassAnalysis& classes,
                           bool normalizedSignature = false);

} // namespace Rose::Source::Analysis

#endif
