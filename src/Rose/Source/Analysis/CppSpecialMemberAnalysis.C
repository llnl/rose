#include <Rose/Source/Analysis/CppSpecialMemberAnalysis.h>

namespace Rose::Source::Analysis
{
bool SpecialMemberFunction::isSpecialFunc() const { return kind() != notspecial; }
bool SpecialMemberFunction::isConstructor() const { return (kind() & ctor) == ctor; }
bool SpecialMemberFunction::isDefaultCtor() const { return (kind() & dctor) == dctor; }
bool SpecialMemberFunction::isCopyCtor() const { return (kind() & cctor) == cctor; }
bool SpecialMemberFunction::isMoveCtor() const { return (kind() & mctor) == mctor; }
bool SpecialMemberFunction::isDestructor() const { return (kind() & dtor) == dtor; }
bool SpecialMemberFunction::isCopyAssign() const { return (kind() & cassign) == cassign; }
bool SpecialMemberFunction::isMoveAssign() const { return (kind() & massign) == massign; }

SpecialMemberFunctionPredicate isCopyCtor(bool anyOp)
{
  return [anyOp](const SpecialMemberFunction& smf) {
    return smf.isCopyCtor() && (anyOp || !smf.compilerGenerated());
  };
}

SpecialMemberFunctionPredicate isCopyAssign(bool anyOp)
{
  return [anyOp](const SpecialMemberFunction& smf) {
    return smf.isCopyAssign() && (anyOp || !smf.compilerGenerated());
  };
}

SpecialMemberFunctionPredicate isMoveCtor(bool anyOp)
{
  return [anyOp](const SpecialMemberFunction& smf) {
    return smf.isMoveCtor() && (anyOp || !smf.compilerGenerated());
  };
}

SpecialMemberFunctionPredicate isMoveAssign(bool anyOp)
{
  return [anyOp](const SpecialMemberFunction& smf) {
    return smf.isMoveAssign() && (anyOp || !smf.compilerGenerated());
  };
}

SpecialMemberFunctionPredicate isDefaultCtor(bool anyOp)
{
  return [anyOp](const SpecialMemberFunction& smf) {
    return smf.isDefaultCtor() && (anyOp || !smf.compilerGenerated());
  };
}

SpecialMemberFunctionPredicate isDtor(bool anyOp)
{
  return [anyOp](const SpecialMemberFunction& smf) {
    return smf.isDestructor() && (anyOp || !smf.compilerGenerated());
  };
}

SpecialMemberFunctionPredicate isUserDefinedCtor()
{
  return [](const SpecialMemberFunction& smf) {
    return smf.isConstructor() && !smf.compilerGenerated();
  };
}

bool hasSpecialFunction(const SpecialMemberFunctionContainer& specials,
                        SpecialMemberFunctionPredicate pred)
{
  for (const SpecialMemberFunction& smf : specials)
    if (pred(smf))
      return true;
  return false;
}
} // namespace Rose::Source::Analysis
