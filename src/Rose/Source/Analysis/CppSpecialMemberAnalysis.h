#ifndef ROSE_Source_Analysis_CppSpecialMemberAnalysis_H
#define ROSE_Source_Analysis_CppSpecialMemberAnalysis_H 1

#include <RoseFirst.h>

#include <cstdint>
#include <functional>
#include <tuple>
#include <utility>
#include <vector>

#include <Rose/Common/Analysis/AnalysisTypes.h>

namespace Rose::Source::Analysis
{
/// Tuple storage for C++ special member function metadata.
using SpecialMemberFunctionBase =
  std::tuple<Rose::Common::Analysis::FunctionKeyType, std::int8_t, bool, bool, bool>;

/// Describes a constructor, assignment operator, or destructor with C++ special-member properties.
///
/// This is a C++-language concept and deliberately belongs to Source/Analysis,
/// rather than the language-neutral Common hierarchy analysis.
struct SpecialMemberFunction : SpecialMemberFunctionBase
{
  using base = SpecialMemberFunctionBase;
  using base::base;

  enum Kind : std::uint8_t
  {
    notspecial = 0,
    ctor       = (1 << 0),
    dctor      = ctor + (1 << 1),
    cctor      = ctor + (1 << 2),
    mctor      = ctor + (1 << 3),
    dtor       = (1 << 4),
    cassign    = (1 << 5),
    massign    = (1 << 6)
  };

  Rose::Common::Analysis::FunctionKeyType function() const { return std::get<0>(*this); }
  std::int8_t kind() const { return std::get<1>(*this); }
  bool isSpecialFunc() const;
  bool isConstructor() const;
  bool isDefaultCtor() const;
  bool isCopyCtor() const;
  bool isMoveCtor() const;
  bool isDestructor() const;
  bool isCopyAssign() const;
  bool isMoveAssign() const;
  bool compilerGenerated() const { return std::get<2>(*this); }
  bool copyCallableWithConstRefArgument() const { return std::get<3>(*this); }
  bool standardConforming() const { return std::get<4>(*this); }
};

using SpecialMemberFunctionContainer = std::vector<SpecialMemberFunction>;
using SpecialMemberFunctionPredicate = std::function<bool(const SpecialMemberFunction&)>;

SpecialMemberFunctionPredicate isCopyCtor(bool anyOp = true);
SpecialMemberFunctionPredicate isCopyAssign(bool anyOp = true);
SpecialMemberFunctionPredicate isMoveCtor(bool anyOp = true);
SpecialMemberFunctionPredicate isMoveAssign(bool anyOp = true);
SpecialMemberFunctionPredicate isDefaultCtor(bool anyOp = true);
SpecialMemberFunctionPredicate isDtor(bool anyOp = true);
SpecialMemberFunctionPredicate isUserDefinedCtor();
bool hasSpecialFunction(const SpecialMemberFunctionContainer&, SpecialMemberFunctionPredicate);

inline
SpecialMemberFunctionPredicate
operator||(SpecialMemberFunctionPredicate lhs, SpecialMemberFunctionPredicate rhs)
{
  return [lhs = std::move(lhs), rhs = std::move(rhs)](const SpecialMemberFunction& smf) {
    return lhs(smf) || rhs(smf);
  };
}
} // namespace Rose::Source::Analysis

#endif
