#include <sage3basic.h>
#include <sageGeneric.h>

#include <Rose/Source/Analysis/CppHierarchyAnalysis.h>

#include <algorithm>

#include <boost/range/adaptors.hpp>
#include <boost/range/join.hpp>

namespace rca = Rose::Common::Analysis;
namespace adapt = boost::adaptors;

namespace Rose::Source::Analysis
{
const CppCompatibility&
CppHierarchyAnalysis::compatibilityBridge() const
{
  return compat;
}

CppCompatibility CppHierarchyAnalysis::compat;

rca::ASTRootType
CppHierarchyAnalysis::rootId(const SgProject* root) const
{
  return rootToKey(root);
}

SgProject*
CppHierarchyAnalysis::root(rca::ASTRootType key) const
{
  return keyToRoot(key);
}

rca::ClassKeyType
CppHierarchyAnalysis::classId(const SgClassDefinition* cls) const
{
  return classToKey(cls);
}

SgClassDefinition*
CppHierarchyAnalysis::classDefinition(rca::ClassKeyType key) const
{
  return keyToClass(key);
}

rca::FunctionKeyType
CppHierarchyAnalysis::functionId(const SgFunctionDeclaration* fun) const
{
  return compat.functionId(fun);
}

SgFunctionDeclaration*
CppHierarchyAnalysis::functionDeclaration(rca::FunctionKeyType key) const
{
  return keyToFunction(key);
}

rca::VariableKeyType
CppHierarchyAnalysis::variableId(SgInitializedName* var) const
{
  return compat.variableId(var);
}

rca::VariableKeyType
CppHierarchyAnalysis::variableId(SgVariableDeclaration* var) const
{
  return compat.variableId(var);
}

SgInitializedName*
CppHierarchyAnalysis::variableDeclaration(rca::VariableKeyType key) const
{
  return keyToVariable(key);
}

rca::FunctionTypeKeyType
CppHierarchyAnalysis::functionTypeId(const SgFunctionType* ty) const
{
  return functionTypeToKey(ty);
}

SgFunctionType*
CppHierarchyAnalysis::functionType(rca::FunctionTypeKeyType key) const
{
  return keyToFunctionType(key);
}

rca::TypeKeyType
CppHierarchyAnalysis::typeId(const SgType* ty) const
{
  return typeToKey(ty);
}

SgType*
CppHierarchyAnalysis::type(rca::TypeKeyType key) const
{
  return keyToType(key);
}

std::string
CppHierarchyAnalysis::nameOf(const SgClassDefinition* cls) const
{
  return compat.nameOf(classId(cls));
}

std::string
CppHierarchyAnalysis::nameOf(const SgFunctionDeclaration* fun) const
{
  return compat.nameOf(functionId(fun));
}

std::string
CppHierarchyAnalysis::nameOf(const SgInitializedName* var) const
{
  return compat.nameOf(variableToKey(var));
}

SpecialMemberFunctionContainer&
CppHierarchyAnalysis::specialMemberFunctions(rca::ClassKeyType clazz)
{
  return allSpecialMemberFunctions[clazz];
}

const SpecialMemberFunctionContainer&
CppHierarchyAnalysis::specialMemberFunctions(rca::ClassKeyType clazz) const
{
  static const SpecialMemberFunctionContainer empty;
  SpecialMemberFunctionMap::const_iterator pos = allSpecialMemberFunctions.find(clazz);

  return pos != allSpecialMemberFunctions.end() ? pos->second : empty;
}
/*
bool hasSpecialFunction(const CppHierarchyAnalysis& all,
                        rca::ClassKeyType clazz,
                        SpecialMemberFunctionPredicate pred)
{
  return hasSpecialFunction(all.specialMemberFunctions(clazz), std::move(pred));
}

bool hasSpecialFunction(const rca::ClassAnalysis& all,
                        rca::ClassKeyType clazz,
                        SpecialMemberFunctionPredicate pred)
{
  const CppHierarchyAnalysis* cpp = dynamic_cast<const CppHierarchyAnalysis*>(&all);
  return cpp && hasSpecialFunction(*cpp, clazz, std::move(pred));
}
*/
namespace
{
  enum class SpecialMemberUse
  {
    baseSubobject,
    memberSubobject
  };

  bool
  isUsableSpecialMember(const CppHierarchyAnalysis& all,
                        const SpecialMemberFunction& smf,
                        rca::ClassKeyType caller,
                        SpecialMemberUse use)
  {
    const rca::FunctionKeyType key = smf.function();

    // A keyless descriptor was synthesized only after its own prerequisites
    // were established.  Such implicitly declared special members are public.
    if (!rca::isValidKey(key))
      return true;

    const SgMemberFunctionDeclaration* function =
      isSgMemberFunctionDeclaration(all.functionDeclaration(key));
    if (!function || function->get_functionModifier().isMarkedDelete())
      return false;

    const SgAccessModifier& access =
      function->get_declarationModifier().get_accessModifier();
    if (access.isPublic())
      return true;

    const Optional declaringClass = all.compatibilityBridge().classType(key);
    if (!declaringClass)
      return false;

    if (access.isPrivate())
      return caller == *declaringClass;

    // A generated member of a derived class can use a protected special
    // member to initialize its base subobject.  That rule does not make the
    // protected member callable for an unrelated by-value data member.
    return access.isProtected()
        && use == SpecialMemberUse::baseSubobject
        && (caller == *declaringClass || all.isBaseOf(*declaringClass, caller));
  }

  bool
  hasUsableSpecialFunction(const CppHierarchyAnalysis& all,
                           rca::ClassKeyType clazz,
                           rca::ClassKeyType caller,
                           SpecialMemberUse use,
                           const SpecialMemberFunctionPredicate& pred)
  {
    const SpecialMemberFunctionContainer& functions = all.specialMemberFunctions(clazz);
    return std::any_of(functions.begin(), functions.end(),
                       [&all, caller, use, &pred](const SpecialMemberFunction& smf) {
                         return pred(smf) && isUsableSpecialMember(all, smf, caller, use);
                       });
  }

  std::function<bool(const rca::InheritanceDesc&)>
  baseHasSpecialFunction(const CppHierarchyAnalysis& all,
                         rca::ClassKeyType caller,
                         SpecialMemberFunctionPredicate pred)
  {
    return [&all, caller, p = std::move(pred)](const rca::InheritanceDesc& baseClass) -> bool
           {
             return hasUsableSpecialFunction(all, baseClass.getClass(), caller,
                                             SpecialMemberUse::baseSubobject, p);
           };
  }

  SpecialMemberFunctionPredicate isCopyCtorConstRef()
  {
    return [](const SpecialMemberFunction& smf) -> bool
           {
             return smf.isCopyCtor() && smf.copyCallableWithConstRefArgument();
           };
  }

  SpecialMemberFunctionPredicate isCopyAssignConstRef()
  {
    return [](const SpecialMemberFunction& smf) -> bool
           {
             return smf.isCopyAssign() && smf.copyCallableWithConstRefArgument();
           };
  }

  using VariablePredicate = std::function<bool(rca::VariableKeyType)>;

  VariablePredicate isDefaultConstructible(const CppHierarchyAnalysis& all, rca::ClassKeyType caller)
  {
    return [&all, caller](rca::VariableKeyType var) -> bool
           {
             const rca::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             rca::DataMemberType desc = compat.typeOf(var);

             // A default member initializer constructs the subobject itself,
             // so it does not need a default constructor.  This includes
             // reference members, which otherwise cannot be default-initialized.
             if (desc.hasInitializer())
               return true;

             if (desc.hasReference())
               return false;

             rca::ClassKeyType clazz = desc.classType();
             return !rca::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, isDefaultCtor());
           };
  }

  VariablePredicate isCopyConstructible(const CppHierarchyAnalysis& all, rca::ClassKeyType caller)
  {
    return [&all, caller](rca::VariableKeyType var) -> bool
           {
             const rca::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             rca::DataMemberType desc = compat.typeOf(var);
             rca::ClassKeyType clazz = desc.classType();

             return !rca::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, isCopyCtor());
           };
  }

  VariablePredicate isCopyConstructibleConstRef(const CppHierarchyAnalysis& all,
                                                rca::ClassKeyType caller)
  {
    return [&all, caller](rca::VariableKeyType var) -> bool
           {
             const rca::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             rca::DataMemberType desc = compat.typeOf(var);
             rca::ClassKeyType clazz = desc.classType();

             return !rca::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, isCopyCtorConstRef());
           };
  }

  VariablePredicate isMoveOrCopyConstructible(const CppHierarchyAnalysis& all,
                                              rca::ClassKeyType caller)
  {
    return [&all, caller, moveOrCopy = isMoveCtor() || isCopyCtor()]
           (rca::VariableKeyType var) -> bool
           {
             const rca::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             rca::DataMemberType desc = compat.typeOf(var);
             rca::ClassKeyType clazz = desc.classType();

             return !rca::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, moveOrCopy);
           };
  }

  VariablePredicate isCopyAssignable(const CppHierarchyAnalysis& all, rca::ClassKeyType caller)
  {
    return [&all, caller](rca::VariableKeyType var) -> bool
           {
             const rca::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             rca::DataMemberType desc = compat.typeOf(var);

             if (desc.hasReference() || desc.isConst())
               return false;

             rca::ClassKeyType clazz = desc.classType();
             return !rca::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, isCopyAssign());
           };
  }

  VariablePredicate isCopyAssignableConstRef(const CppHierarchyAnalysis& all,
                                             rca::ClassKeyType caller)
  {
    return [&all, caller](rca::VariableKeyType var) -> bool
           {
             const rca::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             rca::DataMemberType desc = compat.typeOf(var);

             if (desc.hasReference() || desc.isConst())
               return false;

             rca::ClassKeyType clazz = desc.classType();
             return !rca::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, isCopyAssignConstRef());
           };
  }

  VariablePredicate isMoveOrCopyAssignable(const CppHierarchyAnalysis& all,
                                           rca::ClassKeyType caller)
  {
    return [&all, caller, moveOrCopy = isMoveAssign() || isCopyAssign()]
           (rca::VariableKeyType var) -> bool
           {
             const rca::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             rca::DataMemberType desc = compat.typeOf(var);

             if (desc.hasReference() || desc.isConst())
               return false;

             rca::ClassKeyType clazz = desc.classType();
             return !rca::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, moveOrCopy);
           };
  }

  SpecialMemberFunctionContainer
  processDefaultCtor(const CppHierarchyAnalysis& all,
                     const rca::ClassAnalysis::value_type& rep,
                     SpecialMemberFunctionContainer funcs)
  {
    if (hasSpecialFunction(funcs, isUserDefinedCtor()))
      return funcs;

    if (hasSpecialFunction(funcs, isDefaultCtor()))
      return funcs;

    const rca::ClassData::AncestorContainer& bases = rep.second.ancestors();
    if (!std::all_of(bases.begin(), bases.end(),
                     baseHasSpecialFunction(all, rep.first, isDefaultCtor())))
      return funcs;

    const rca::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    if (!std::all_of(data.begin(), data.end(), isDefaultConstructible(all, rep.first)))
      return funcs;

    funcs.emplace_back(rca::FunctionKeyType{}, SpecialMemberFunction::dctor, true, false, true);
    return funcs;
  }

  SpecialMemberFunctionContainer
  processDefaultDtor(const CppHierarchyAnalysis& all,
                     const rca::ClassAnalysis::value_type& rep,
                     SpecialMemberFunctionContainer funcs)
  {
    if (hasSpecialFunction(funcs, isDtor()))
      return funcs;

    const rca::ClassData::AncestorContainer& bases = rep.second.ancestors();
    if (!std::all_of(bases.begin(), bases.end(),
                     baseHasSpecialFunction(all, rep.first, isDtor())))
      return funcs;

    const rca::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    const VariablePredicate isDestructible =
      [&all, caller = rep.first](rca::VariableKeyType var) -> bool
      {
        const rca::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
        rca::DataMemberType desc = compat.typeOf(var);
        rca::ClassKeyType clazz = desc.classType();

        return !rca::isValidKey(clazz)
            || hasUsableSpecialFunction(all, clazz, caller,
                                        SpecialMemberUse::memberSubobject, isDtor());
      };
    if (!std::all_of(data.begin(), data.end(), isDestructible))
      return funcs;

    funcs.emplace_back(rca::FunctionKeyType{}, SpecialMemberFunction::dtor, true, false, true);

    return funcs;
  }

  SpecialMemberFunctionContainer
  processDefaultCopyCtor(const CppHierarchyAnalysis& all,
                         const rca::ClassAnalysis::value_type& rep,
                         SpecialMemberFunctionContainer funcs)
  {
    if (hasSpecialFunction(funcs, isCopyCtor()))
      return funcs;

    if (all.languageStandard() >= CppHierarchyAnalysis::Cxx11)
    {
      const bool hasUserDefinedMoveFn = hasSpecialFunction(funcs, isMoveCtor(false))
                                     || hasSpecialFunction(funcs, isMoveAssign(false));
      if (hasUserDefinedMoveFn)
        return funcs;
    }

    const rca::ClassData::AncestorContainer& bases = rep.second.ancestors();
    const bool basesSupportConstRef = std::all_of(bases.begin(), bases.end(),
                                                  baseHasSpecialFunction(all, rep.first, isCopyCtorConstRef()));
    const bool basesHaveCopyCtor = basesSupportConstRef
                                || std::all_of(bases.begin(), bases.end(),
                                               baseHasSpecialFunction(all, rep.first, isCopyCtor()));
    if (!basesHaveCopyCtor)
      return funcs;

    const rca::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    const bool dataSupportConstRef = std::all_of(data.begin(), data.end(),
                                                 isCopyConstructibleConstRef(all, rep.first));
    const bool dataIsCopyConstructible = dataSupportConstRef
                                      || std::all_of(data.begin(), data.end(),
                                                     isCopyConstructible(all, rep.first));
    if (!dataIsCopyConstructible)
      return funcs;

    funcs.emplace_back(rca::FunctionKeyType{}, SpecialMemberFunction::cctor,
                       true, basesSupportConstRef && dataSupportConstRef, true);
    return funcs;
  }

  SpecialMemberFunctionContainer
  processDefaultMoveCtor(const CppHierarchyAnalysis& all,
                         const rca::ClassAnalysis::value_type& rep,
                         SpecialMemberFunctionContainer funcs)
  {
    if (all.languageStandard() < CppHierarchyAnalysis::Cxx11)
      return funcs;

    if (hasSpecialFunction(funcs, isMoveCtor()))
      return funcs;

    const bool hasUserDefinedSpecialFn = hasSpecialFunction(funcs, isCopyCtor(false))
                                      || hasSpecialFunction(funcs, isCopyAssign(false))
                                      || hasSpecialFunction(funcs, isMoveAssign(false))
                                      || hasSpecialFunction(funcs, isDtor(false));
    if (hasUserDefinedSpecialFn)
      return funcs;

    const rca::ClassData::AncestorContainer& bases = rep.second.ancestors();
    SpecialMemberFunctionPredicate moveOrCopyPred = isMoveCtor() || isCopyCtor();
    if (!std::all_of(bases.begin(), bases.end(),
                     baseHasSpecialFunction(all, rep.first, moveOrCopyPred)))
      return funcs;

    const rca::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    if (!std::all_of(data.begin(), data.end(), isMoveOrCopyConstructible(all, rep.first)))
      return funcs;

    funcs.emplace_back(rca::FunctionKeyType{}, SpecialMemberFunction::mctor, true, false, true);
    return funcs;
  }

  SpecialMemberFunctionContainer
  processDefaultCopyAssign(const CppHierarchyAnalysis& all,
                           const rca::ClassAnalysis::value_type& rep,
                           SpecialMemberFunctionContainer funcs)
  {
    if (hasSpecialFunction(funcs, isCopyAssign()))
      return funcs;

    if (all.languageStandard() >= CppHierarchyAnalysis::Cxx11)
    {
      const bool hasUserDefinedMoveFn = hasSpecialFunction(funcs, isMoveCtor(false))
                                     || hasSpecialFunction(funcs, isMoveAssign(false));
      if (hasUserDefinedMoveFn)
        return funcs;
    }

    const rca::ClassData::AncestorContainer& bases = rep.second.ancestors();
    const bool basesSupportConstRef = std::all_of(bases.begin(), bases.end(),
                                                  baseHasSpecialFunction(all, rep.first, isCopyAssignConstRef()));
    const bool basesHaveCopyAssign = basesSupportConstRef
                                  || std::all_of(bases.begin(), bases.end(),
                                                 baseHasSpecialFunction(all, rep.first, isCopyAssign()));
    if (!basesHaveCopyAssign)
      return funcs;

    const rca::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    const bool dataSupportConstRef = std::all_of(data.begin(), data.end(),
                                                 isCopyAssignableConstRef(all, rep.first));
    const bool dataAreCopyAssignable = dataSupportConstRef
                                    || std::all_of(data.begin(), data.end(),
                                                   isCopyAssignable(all, rep.first));
    if (!dataAreCopyAssignable)
      return funcs;

    funcs.emplace_back(rca::FunctionKeyType{}, SpecialMemberFunction::cassign,
                       true, basesSupportConstRef && dataSupportConstRef, true);
    return funcs;
  }

  SpecialMemberFunctionContainer
  processDefaultMoveAssign(const CppHierarchyAnalysis& all,
                           const rca::ClassAnalysis::value_type& rep,
                           SpecialMemberFunctionContainer funcs)
  {
    if (all.languageStandard() < CppHierarchyAnalysis::Cxx11)
      return funcs;

    if (hasSpecialFunction(funcs, isMoveAssign()))
      return funcs;

    const bool hasUserDefinedSpecialFn = hasSpecialFunction(funcs, isCopyCtor(false))
                                      || hasSpecialFunction(funcs, isCopyAssign(false))
                                      || hasSpecialFunction(funcs, isMoveCtor(false))
                                      || hasSpecialFunction(funcs, isDtor(false));
    if (hasUserDefinedSpecialFn)
      return funcs;

    const rca::ClassData::AncestorContainer& bases = rep.second.ancestors();
    SpecialMemberFunctionPredicate moveOrCopyPred = isMoveAssign() || isCopyAssign();
    if (!std::all_of(bases.begin(), bases.end(),
                     baseHasSpecialFunction(all, rep.first, moveOrCopyPred)))
      return funcs;

    const rca::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    if (!std::all_of(data.begin(), data.end(), isMoveOrCopyAssignable(all, rep.first)))
      return funcs;

    funcs.emplace_back(rca::FunctionKeyType{}, SpecialMemberFunction::massign, true, false, true);
    return funcs;
  }

  void analyzeAutoGeneratedMemberFunctions(CppHierarchyAnalysis& all,
                                           rca::ClassAnalysis::value_type& rep)
  {
    const CppCompatibility& compat = all.compatibilityBridge();
    SpecialMemberFunctionContainer funcs = compat.specialMemberFunctions(rep.first);

    funcs = processDefaultCtor(all, rep, std::move(funcs));
    funcs = processDefaultDtor(all, rep, std::move(funcs));
    funcs = processDefaultCopyCtor(all, rep, std::move(funcs));
    funcs = processDefaultCopyAssign(all, rep, std::move(funcs));
    funcs = processDefaultMoveCtor(all, rep, std::move(funcs));
    funcs = processDefaultMoveAssign(all, rep, std::move(funcs));

    all.specialMemberFunctions(rep.first) = std::move(funcs);
  }

  using RelationAccessorConst = const std::vector<rca::InheritanceDesc>& (rca::ClassData::*)() const;

  struct GetClass
  {
    rca::ClassKeyType operator()(const rca::InheritanceDesc& desc) const
    {
      return desc.getClass();
    }
  };

  struct GetType
  {
    rca::ClassKeyType operator()(rca::VariableKeyType key) const
    {
      return compat->typeOf(key).classType();
    }

    sg::NotNull<const rca::AbstractCompatibilityBridge> compat;
  };

  struct IsValidClassKey
  {
    bool operator()(rca::ClassKeyType key) const
    {
      return rca::isValidKey(key);
    }
  };

  template <class AnalysisT, class MemFnSelector>
  auto inheritanceRelationships(AnalysisT&, MemFnSelector selector)
  {
    return [selector](auto& elem) // -> someRange
           {
             return (elem.second.*selector)()
                    | adapt::filtered(rca::IsDirect{})
                    | adapt::transformed(GetClass{});
           };
  }

  auto valueTypes(const CppHierarchyAnalysis& all,
                  const rca::ClassAnalysis::value_type& clazz) // -> someRange
  {
    return clazz.second.dataMembers()
           | adapt::transformed(GetType{&all.compatibilityBridge()})
           | adapt::filtered(IsValidClassKey{});
  }

  template <class AnalysisT, class Fn, class Selector>
  void traversal(AnalysisT& all, Fn fn, rca::ClassKeyType curr, Selector selector)
  {
    typename AnalysisT::iterator pos = all.find(curr);
    if (pos == all.end())
    {
      (void)all.at(curr); // issue the standard missing-class diagnostic
      return;
    }

    auto& elem = *pos;
    const rca::AbstractCompatibilityBridge& compat = all.compatibilityBridge();

    for (rca::ClassKeyType clazz : selector(elem))
    {
      try
      {
        traversal(all, fn, clazz, selector);
      }
      catch (...)
      {
        msgError() << "  .. required from " << compat.nameOf(elem.first)
                   << "\n     and dependency " << compat.nameOf(clazz)
                   << "\n     CONSIDER: use Rose::Source::Analysis::analyzeCppClassesFromMemoryPool"
                   << " to find all classes."
                   << std::endl;
        throw;
      }
    }

    fn(elem);
  }

  template <class Fn>
  void traverseInTopologicalOrder(CppHierarchyAnalysis& all, Fn fn)
  {
    auto preOrderClasses =
           [&all](auto& elem) // -> someRange
           {
             RelationAccessorConst ancestors = &rca::ClassData::ancestors;

             return boost::range::join(inheritanceRelationships(all, ancestors)(elem),
                                       valueTypes(all, elem));
           };

    for (auto& elem : all)
      traversal(all, fn, elem.first, preOrderClasses);
  }
}

void analyzeSpecialMemberFunctions(CppHierarchyAnalysis& all)
{
  msgTrace() << "analyzing C++ special member functions " << std::endl;

  traverseInTopologicalOrder(all,
                             [&all](rca::ClassAnalysis::value_type& rep) -> void
                             {
                               analyzeAutoGeneratedMemberFunctions(all, rep);
                             });
}

CppHierarchyAnalysis
analyzeCppClasses(const SgProject* prj)
{
  if (prj == nullptr)
    return {};

  CppHierarchyAnalysis classes;

  rca::analyzeClasses(classes, rootToKey(prj));
  classes.languageStandard(classes.compatibilityBridge().languageStandard());
  analyzeSpecialMemberFunctions(classes);
  return classes;
}

CppHierarchyAnalysis
analyzeCppClassesFromMemoryPool()
{
  CppHierarchyAnalysis classes;

  rca::analyzeClassesFromMemoryPool(classes);
  classes.languageStandard(classes.compatibilityBridge().languageStandard());
  analyzeSpecialMemberFunctions(classes);
  return classes;
}

rca::VirtualFunctionAnalysis
analyzeCppVirtualFunctions(const rca::ClassAnalysis& classes, bool normalizedSignature)
{
  return rca::analyzeVirtualFunctions(classes, normalizedSignature);
}

} // namespace Rose::Source::Analysis
