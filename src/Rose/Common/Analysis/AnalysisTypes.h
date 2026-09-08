

#ifndef ROSE_Common_Analysis_AnalysisTypes_H
#define ROSE_Common_Analysis_AnalysisTypes_H 1

#include <RoseFirst.h>

#include <cstdint>
#include <tuple>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <boost/variant.hpp>


namespace Rose::Common::Analysis
{

/// Relationship between two function types for override analysis.
enum struct FunctionTypeRelation : int
{
  /// The function types are not override-compatible.
  unrelated = 0,

  /// The function types are identical for the analysis purpose.
  sametype  = 1,

  /// The function types differ by a covariant return type.
  covariant = 2
};

/// Strongly typed wrapper around an abstract pointer-sized analysis key.
/// \tparam tag distinguishes this key from other abstract key domains.
template <int /*tag*/>
struct KeyType
{
  /// Integer storage type used for abstract keys.
  using type = std::uintptr_t;

  /// Raw integer key value; zero represents an invalid key.
  type key = 0;
};

/// Returns true when two keys have the same raw value.
/// \tparam tag key domain being compared.
/// \param lhs left-hand side key.
/// \param rhs right-hand side key.
/// \return true if \p lhs and \p rhs contain the same raw key value.
template <int tag>
inline
bool
operator==(KeyType<tag> lhs, KeyType<tag> rhs) noexcept
{
  return lhs.key == rhs.key;
}

/// Returns true when two keys have different raw values.
/// \tparam tag key domain being compared.
/// \param lhs left-hand side key.
/// \param rhs right-hand side key.
/// \return true if \p lhs and \p rhs contain different raw key values.
template <int tag>
inline
bool
operator!=(KeyType<tag> lhs, KeyType<tag> rhs) noexcept
{
  return lhs.key != rhs.key;
}

/// Provides a strict weak ordering over keys by raw value.
/// \tparam tag key domain being compared.
/// \param lhs left-hand side key.
/// \param rhs right-hand side key.
/// \return true if \p lhs is ordered before \p rhs by raw key value.
template <int tag>
inline
bool
operator<(KeyType<tag> lhs, KeyType<tag> rhs) noexcept
{
  return lhs.key < rhs.key;
}

/// Returns true when \p lhs is less than or equal to \p rhs by raw value.
/// \tparam tag key domain being compared.
/// \param lhs left-hand side key.
/// \param rhs right-hand side key.
/// \return true if \p lhs is less than or equal to \p rhs by raw key value.
template <int tag>
inline
bool
operator<=(KeyType<tag> lhs, KeyType<tag> rhs) noexcept
{
  return lhs.key <= rhs.key;
}

/// Returns true when \p lhs is greater than \p rhs by raw value.
/// \tparam tag key domain being compared.
/// \param lhs left-hand side key.
/// \param rhs right-hand side key.
/// \return true if \p lhs is greater than \p rhs by raw key value.
template <int tag>
inline
bool
operator>(KeyType<tag> lhs, KeyType<tag> rhs) noexcept
{
  return lhs.key > rhs.key;
}

/// Returns true when \p lhs is greater than or equal to \p rhs by raw value.
/// \tparam tag key domain being compared.
/// \param lhs left-hand side key.
/// \param rhs right-hand side key.
/// \return true if \p lhs is greater than or equal to \p rhs by raw key value.
template <int tag>
inline
bool
operator>=(KeyType<tag> lhs, KeyType<tag> rhs) noexcept
{
  return lhs.key >= rhs.key;
}

/*
template <int tag>
inline
std::ostream&
operator<<(std::ostream& os, KeyType<tag> k) noexcept
{
  return os << reinterpret_cast<void*>(k.key);
}
*/

/// Predicate object that tests whether an abstract key is nonzero.
struct IsValidKey
{

  /// Returns true when \p k is a nonzero key.
  /// \tparam tag key domain being tested.
  /// \param k key to test.
  /// \return true if \p k has a nonzero raw value.
  template <int tag>
  bool operator()(KeyType<tag> k) const { return k != KeyType<tag>{}; }
};


/// Returns true when \p k is a nonzero key.
/// \tparam tag key domain being tested.
/// \param k key to test.
/// \return true if \p k has a nonzero raw value.
template <int tag>
inline
bool
isValidKey(KeyType<tag> k) { return IsValidKey{}(k); }

/// Distinguishes the different abstract key domains.
enum KeyTags { tgNone, tgRoot, tgClass, tgFunc, tgVar, tgFuncType, tgOtherType, tgCast, tgOtherExpr };

/// Abstract key for an AST root object.
using ASTRootType           = KeyType<tgRoot>;

/// Abstract key for a function declaration.
using FunctionKeyType       = KeyType<tgFunc>;

/// Abstract key for a class definition.
using ClassKeyType          = KeyType<tgClass>;

/// Abstract key for a variable or data member.
using VariableKeyType       = KeyType<tgVar>;

/// Abstract key for a function type.
using FunctionTypeKeyType   = KeyType<tgFuncType>;

/// Abstract key for a non-function type.
using TypeKeyType           = KeyType<tgOtherType>;

/// Abstract key for a cast expression.
using CastKeyType           = KeyType<tgCast>;

/// Abstract key for an expression.
using ExpressionKeyType     = KeyType<tgOtherExpr>;

/// Variant that can store any analysis key type.
using AnyKeyType            = boost::variant< ASTRootType, FunctionKeyType, ClassKeyType, VariableKeyType
                                              , TypeKeyType, FunctionTypeKeyType
                                              , ExpressionKeyType, CastKeyType
                                              >;


/// Tuple storage for data member type properties.
using DataMemberTypeBase = std::tuple<ClassKeyType, bool, bool, bool, bool>;

/// Describes the class-valued type and selected qualifiers of a data member.
struct DataMemberType : DataMemberTypeBase
{

  /// Base tuple type used for storage.
  using base = DataMemberTypeBase;

  using base::base;

  /// Returns the class type of this data member, if represented by the frontend.
  /// \return class key for the member type, or an invalid key when no class type is represented.
  ClassKeyType classType()      const { return std::get<0>(*this); }

  /// Returns true when the data member type is const-qualified.
  /// \return true if the member type carries a const qualifier.
  bool         isConst()        const { return std::get<1>(*this); }

  /// Returns true when the data member type contains a reference.
  /// \return true if the member type is or contains a reference.
  bool         hasReference()   const { return std::get<2>(*this); }

  /// Returns true when the data member type contains an array.
  /// \return true if the member type is or contains an array.
  bool         hasArray()       const { return std::get<3>(*this); }

  /// Returns true when the data member has an initializer.
  /// \return true if the data member has an initializer.
  bool         hasInitializer() const { return std::get<4>(*this); }
};



/// type of a class naming function
/// \param key class key to name.
/// \return display name for \p key.
using ClassNameFn   = std::function<std::string(ClassKeyType)>;

/// type of a function naming function
/// \param key function key to name.
/// \return display name for \p key.
using FuncNameFn    = std::function<std::string(FunctionKeyType)>;

/// type of a variable naming function
/// \param key variable key to name.
/// \return display name for \p key.
using VarNameFn     = std::function<std::string(VariableKeyType)>;

/// a predicate type for functions
/// \param key function key to test.
/// \return true if \p key satisfies the predicate.
using FunctionPredicate = std::function<bool(FunctionKeyType)>;

}


namespace std
{

  /// Hash support for Rose::Common::Analysis::KeyType so keys can be used in unordered containers.
  /// \tparam tag key domain being hashed.
  template <int tag>
  struct hash<::Rose::Common::Analysis::KeyType<tag>>
  {

    /// Returns the hash of the wrapped raw key value.
    /// \param k key to hash.
    /// \return hash value derived from the raw key value.
    std::size_t operator()(const ::Rose::Common::Analysis::KeyType<tag>& k) const noexcept
    {
      return std::hash<typename ::Rose::Common::Analysis::KeyType<tag>::type>{}(k.key);
    }
  };
}


#endif /*_ANALYSIS_TYPES*/
