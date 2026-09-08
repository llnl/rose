#include <sage3basic.h>
#include <sageGeneric.h>

#include <Rose/Source/Analysis/CppHierarchyAnalysis.h>

#include <algorithm>

#include <boost/range/adaptors.hpp>
#include <boost/range/join.hpp>

namespace ga = Rose::Common::Analysis;
namespace adapt = boost::adaptors;

namespace Rose::Source::Analysis
{
const CppCompatibility&
CppHierarchyAnalysis::compatibilityBridge() const
{
  return compat;
}

CppCompatibility CppHierarchyAnalysis::compat;

ga::ASTRootType
CppHierarchyAnalysis::rootId(SgProject* root) const
{
  return rootToKey(root);
}

SgProject*
CppHierarchyAnalysis::root(ga::ASTRootType key) const
{
  return keyToRoot(key);
}

ga::ClassKeyType
CppHierarchyAnalysis::classId(const SgClassDefinition* cls) const
{
  return classToKey(cls);
}

SgClassDefinition*
CppHierarchyAnalysis::classDefinition(ga::ClassKeyType key) const
{
  return keyToClass(key);
}

ga::FunctionKeyType
CppHierarchyAnalysis::functionId(const SgFunctionDeclaration* fun) const
{
  return compat.functionId(fun);
}

SgFunctionDeclaration*
CppHierarchyAnalysis::functionDeclaration(ga::FunctionKeyType key) const
{
  return keyToFunction(key);
}

ga::VariableKeyType
CppHierarchyAnalysis::variableId(SgInitializedName* var) const
{
  return compat.variableId(var);
}

ga::VariableKeyType
CppHierarchyAnalysis::variableId(SgVariableDeclaration* var) const
{
  return compat.variableId(var);
}

SgInitializedName*
CppHierarchyAnalysis::variableDeclaration(ga::VariableKeyType key) const
{
  return keyToVariable(key);
}

ga::FunctionTypeKeyType
CppHierarchyAnalysis::functionTypeId(const SgFunctionType* ty) const
{
  return functionTypeToKey(ty);
}

SgFunctionType*
CppHierarchyAnalysis::functionType(ga::FunctionTypeKeyType key) const
{
  return keyToFunctionType(key);
}

ga::TypeKeyType
CppHierarchyAnalysis::typeId(const SgType* ty) const
{
  return typeToKey(ty);
}

SgType*
CppHierarchyAnalysis::type(ga::TypeKeyType key) const
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
CppHierarchyAnalysis::specialMemberFunctions(ga::ClassKeyType clazz)
{
  return allSpecialMemberFunctions[clazz];
}

const SpecialMemberFunctionContainer&
CppHierarchyAnalysis::specialMemberFunctions(ga::ClassKeyType clazz) const
{
  static const SpecialMemberFunctionContainer empty;
  SpecialMemberFunctionMap::const_iterator pos = allSpecialMemberFunctions.find(clazz);

  return pos != allSpecialMemberFunctions.end() ? pos->second : empty;
}
/*
bool hasSpecialFunction(const CppHierarchyAnalysis& all,
                        ga::ClassKeyType clazz,
                        SpecialMemberFunctionPredicate pred)
{
  return hasSpecialFunction(all.specialMemberFunctions(clazz), std::move(pred));
}

bool hasSpecialFunction(const ga::ClassAnalysis& all,
                        ga::ClassKeyType clazz,
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
                        ga::ClassKeyType caller,
                        SpecialMemberUse use)
  {
    const ga::FunctionKeyType key = smf.function();

    // A keyless descriptor was synthesized only after its own prerequisites
    // were established.  Such implicitly declared special members are public.
    if (!ga::isValidKey(key))
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
                           ga::ClassKeyType clazz,
                           ga::ClassKeyType caller,
                           SpecialMemberUse use,
                           const SpecialMemberFunctionPredicate& pred)
  {
    const SpecialMemberFunctionContainer& functions = all.specialMemberFunctions(clazz);
    return std::any_of(functions.begin(), functions.end(),
                       [&all, caller, use, &pred](const SpecialMemberFunction& smf) {
                         return pred(smf) && isUsableSpecialMember(all, smf, caller, use);
                       });
  }

  std::function<bool(const ga::InheritanceDesc&)>
  baseHasSpecialFunction(const CppHierarchyAnalysis& all,
                         ga::ClassKeyType caller,
                         SpecialMemberFunctionPredicate pred)
  {
    return [&all, caller, p = std::move(pred)](const ga::InheritanceDesc& baseClass) -> bool
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

  using VariablePredicate = std::function<bool(ga::VariableKeyType)>;

  VariablePredicate isDefaultConstructible(const CppHierarchyAnalysis& all, ga::ClassKeyType caller)
  {
    return [&all, caller](ga::VariableKeyType var) -> bool
           {
             const ga::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             ga::DataMemberType desc = compat.typeOf(var);

             // A default member initializer constructs the subobject itself,
             // so it does not need a default constructor.  This includes
             // reference members, which otherwise cannot be default-initialized.
             if (desc.hasInitializer())
               return true;

             if (desc.hasReference())
               return false;

             ga::ClassKeyType clazz = desc.classType();
             return !ga::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, isDefaultCtor());
           };
  }

  VariablePredicate isCopyConstructible(const CppHierarchyAnalysis& all, ga::ClassKeyType caller)
  {
    return [&all, caller](ga::VariableKeyType var) -> bool
           {
             const ga::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             ga::DataMemberType desc = compat.typeOf(var);
             ga::ClassKeyType clazz = desc.classType();

             return !ga::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, isCopyCtor());
           };
  }

  VariablePredicate isCopyConstructibleConstRef(const CppHierarchyAnalysis& all,
                                                ga::ClassKeyType caller)
  {
    return [&all, caller](ga::VariableKeyType var) -> bool
           {
             const ga::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             ga::DataMemberType desc = compat.typeOf(var);
             ga::ClassKeyType clazz = desc.classType();

             return !ga::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, isCopyCtorConstRef());
           };
  }

  VariablePredicate isMoveOrCopyConstructible(const CppHierarchyAnalysis& all,
                                              ga::ClassKeyType caller)
  {
    return [&all, caller, moveOrCopy = isMoveCtor() || isCopyCtor()]
           (ga::VariableKeyType var) -> bool
           {
             const ga::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             ga::DataMemberType desc = compat.typeOf(var);
             ga::ClassKeyType clazz = desc.classType();

             return !ga::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, moveOrCopy);
           };
  }

  VariablePredicate isCopyAssignable(const CppHierarchyAnalysis& all, ga::ClassKeyType caller)
  {
    return [&all, caller](ga::VariableKeyType var) -> bool
           {
             const ga::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             ga::DataMemberType desc = compat.typeOf(var);

             if (desc.hasReference() || desc.isConst())
               return false;

             ga::ClassKeyType clazz = desc.classType();
             return !ga::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, isCopyAssign());
           };
  }

  VariablePredicate isCopyAssignableConstRef(const CppHierarchyAnalysis& all,
                                             ga::ClassKeyType caller)
  {
    return [&all, caller](ga::VariableKeyType var) -> bool
           {
             const ga::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             ga::DataMemberType desc = compat.typeOf(var);

             if (desc.hasReference() || desc.isConst())
               return false;

             ga::ClassKeyType clazz = desc.classType();
             return !ga::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, isCopyAssignConstRef());
           };
  }

  VariablePredicate isMoveOrCopyAssignable(const CppHierarchyAnalysis& all,
                                           ga::ClassKeyType caller)
  {
    return [&all, caller, moveOrCopy = isMoveAssign() || isCopyAssign()]
           (ga::VariableKeyType var) -> bool
           {
             const ga::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
             ga::DataMemberType desc = compat.typeOf(var);

             if (desc.hasReference() || desc.isConst())
               return false;

             ga::ClassKeyType clazz = desc.classType();
             return !ga::isValidKey(clazz)
                 || hasUsableSpecialFunction(all, clazz, caller,
                                             SpecialMemberUse::memberSubobject, moveOrCopy);
           };
  }

  SpecialMemberFunctionContainer
  processDefaultCtor(const CppHierarchyAnalysis& all,
                     const ga::ClassAnalysis::value_type& rep,
                     SpecialMemberFunctionContainer funcs)
  {
    if (hasSpecialFunction(funcs, isUserDefinedCtor()))
      return funcs;

    if (hasSpecialFunction(funcs, isDefaultCtor()))
      return funcs;

    const ga::ClassData::AncestorContainer& bases = rep.second.ancestors();
    if (!std::all_of(bases.begin(), bases.end(),
                     baseHasSpecialFunction(all, rep.first, isDefaultCtor())))
      return funcs;

    const ga::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    if (!std::all_of(data.begin(), data.end(), isDefaultConstructible(all, rep.first)))
      return funcs;

    funcs.emplace_back(ga::FunctionKeyType{}, SpecialMemberFunction::dctor, true, false, true);
    return funcs;
  }

  SpecialMemberFunctionContainer
  processDefaultDtor(const CppHierarchyAnalysis& all,
                     const ga::ClassAnalysis::value_type& rep,
                     SpecialMemberFunctionContainer funcs)
  {
    if (hasSpecialFunction(funcs, isDtor()))
      return funcs;

    const ga::ClassData::AncestorContainer& bases = rep.second.ancestors();
    if (!std::all_of(bases.begin(), bases.end(),
                     baseHasSpecialFunction(all, rep.first, isDtor())))
      return funcs;

    const ga::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    const VariablePredicate isDestructible =
      [&all, caller = rep.first](ga::VariableKeyType var) -> bool
      {
        const ga::AbstractCompatibilityBridge& compat = all.compatibilityBridge();
        ga::DataMemberType desc = compat.typeOf(var);
        ga::ClassKeyType clazz = desc.classType();

        return !ga::isValidKey(clazz)
            || hasUsableSpecialFunction(all, clazz, caller,
                                        SpecialMemberUse::memberSubobject, isDtor());
      };
    if (!std::all_of(data.begin(), data.end(), isDestructible))
      return funcs;

    funcs.emplace_back(ga::FunctionKeyType{}, SpecialMemberFunction::dtor, true, false, true);

    return funcs;
  }

  SpecialMemberFunctionContainer
  processDefaultCopyCtor(const CppHierarchyAnalysis& all,
                         const ga::ClassAnalysis::value_type& rep,
                         SpecialMemberFunctionContainer funcs)
  {
    if (hasSpecialFunction(funcs, isCopyCtor()))
      return funcs;

    if (all.languageStandard() >= ga::ClassAnalysis::Cxx11)
    {
      const bool hasUserDefinedMoveFn = hasSpecialFunction(funcs, isMoveCtor(false))
                                     || hasSpecialFunction(funcs, isMoveAssign(false));
      if (hasUserDefinedMoveFn)
        return funcs;
    }

    const ga::ClassData::AncestorContainer& bases = rep.second.ancestors();
    const bool basesSupportConstRef = std::all_of(bases.begin(), bases.end(),
                                                  baseHasSpecialFunction(all, rep.first, isCopyCtorConstRef()));
    const bool basesHaveCopyCtor = basesSupportConstRef
                                || std::all_of(bases.begin(), bases.end(),
                                               baseHasSpecialFunction(all, rep.first, isCopyCtor()));
    if (!basesHaveCopyCtor)
      return funcs;

    const ga::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    const bool dataSupportConstRef = std::all_of(data.begin(), data.end(),
                                                 isCopyConstructibleConstRef(all, rep.first));
    const bool dataIsCopyConstructible = dataSupportConstRef
                                      || std::all_of(data.begin(), data.end(),
                                                     isCopyConstructible(all, rep.first));
    if (!dataIsCopyConstructible)
      return funcs;

    funcs.emplace_back(ga::FunctionKeyType{}, SpecialMemberFunction::cctor,
                       true, basesSupportConstRef && dataSupportConstRef, true);
    return funcs;
  }

  SpecialMemberFunctionContainer
  processDefaultMoveCtor(const CppHierarchyAnalysis& all,
                         const ga::ClassAnalysis::value_type& rep,
                         SpecialMemberFunctionContainer funcs)
  {
    if (all.languageStandard() < ga::ClassAnalysis::Cxx11)
      return funcs;

    if (hasSpecialFunction(funcs, isMoveCtor()))
      return funcs;

    const bool hasUserDefinedSpecialFn = hasSpecialFunction(funcs, isCopyCtor(false))
                                      || hasSpecialFunction(funcs, isCopyAssign(false))
                                      || hasSpecialFunction(funcs, isMoveAssign(false))
                                      || hasSpecialFunction(funcs, isDtor(false));
    if (hasUserDefinedSpecialFn)
      return funcs;

    const ga::ClassData::AncestorContainer& bases = rep.second.ancestors();
    SpecialMemberFunctionPredicate moveOrCopyPred = isMoveCtor() || isCopyCtor();
    if (!std::all_of(bases.begin(), bases.end(),
                     baseHasSpecialFunction(all, rep.first, moveOrCopyPred)))
      return funcs;

    const ga::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    if (!std::all_of(data.begin(), data.end(), isMoveOrCopyConstructible(all, rep.first)))
      return funcs;

    funcs.emplace_back(ga::FunctionKeyType{}, SpecialMemberFunction::mctor, true, false, true);
    return funcs;
  }

  SpecialMemberFunctionContainer
  processDefaultCopyAssign(const CppHierarchyAnalysis& all,
                           const ga::ClassAnalysis::value_type& rep,
                           SpecialMemberFunctionContainer funcs)
  {
    if (hasSpecialFunction(funcs, isCopyAssign()))
      return funcs;

    if (all.languageStandard() >= ga::ClassAnalysis::Cxx11)
    {
      const bool hasUserDefinedMoveFn = hasSpecialFunction(funcs, isMoveCtor(false))
                                     || hasSpecialFunction(funcs, isMoveAssign(false));
      if (hasUserDefinedMoveFn)
        return funcs;
    }

    const ga::ClassData::AncestorContainer& bases = rep.second.ancestors();
    const bool basesSupportConstRef = std::all_of(bases.begin(), bases.end(),
                                                  baseHasSpecialFunction(all, rep.first, isCopyAssignConstRef()));
    const bool basesHaveCopyAssign = basesSupportConstRef
                                  || std::all_of(bases.begin(), bases.end(),
                                                 baseHasSpecialFunction(all, rep.first, isCopyAssign()));
    if (!basesHaveCopyAssign)
      return funcs;

    const ga::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    const bool dataSupportConstRef = std::all_of(data.begin(), data.end(),
                                                 isCopyAssignableConstRef(all, rep.first));
    const bool dataAreCopyAssignable = dataSupportConstRef
                                    || std::all_of(data.begin(), data.end(),
                                                   isCopyAssignable(all, rep.first));
    if (!dataAreCopyAssignable)
      return funcs;

    funcs.emplace_back(ga::FunctionKeyType{}, SpecialMemberFunction::cassign,
                       true, basesSupportConstRef && dataSupportConstRef, true);
    return funcs;
  }

  SpecialMemberFunctionContainer
  processDefaultMoveAssign(const CppHierarchyAnalysis& all,
                           const ga::ClassAnalysis::value_type& rep,
                           SpecialMemberFunctionContainer funcs)
  {
    if (all.languageStandard() < ga::ClassAnalysis::Cxx11)
      return funcs;

    if (hasSpecialFunction(funcs, isMoveAssign()))
      return funcs;

    const bool hasUserDefinedSpecialFn = hasSpecialFunction(funcs, isCopyCtor(false))
                                      || hasSpecialFunction(funcs, isCopyAssign(false))
                                      || hasSpecialFunction(funcs, isMoveCtor(false))
                                      || hasSpecialFunction(funcs, isDtor(false));
    if (hasUserDefinedSpecialFn)
      return funcs;

    const ga::ClassData::AncestorContainer& bases = rep.second.ancestors();
    SpecialMemberFunctionPredicate moveOrCopyPred = isMoveAssign() || isCopyAssign();
    if (!std::all_of(bases.begin(), bases.end(),
                     baseHasSpecialFunction(all, rep.first, moveOrCopyPred)))
      return funcs;

    const ga::ClassData::DataMemberContainer& data = rep.second.dataMembers();
    if (!std::all_of(data.begin(), data.end(), isMoveOrCopyAssignable(all, rep.first)))
      return funcs;

    funcs.emplace_back(ga::FunctionKeyType{}, SpecialMemberFunction::massign, true, false, true);
    return funcs;
  }

  void analyzeAutoGeneratedMemberFunctions(CppHierarchyAnalysis& all,
                                           ga::ClassAnalysis::value_type& rep)
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

  using RelationAccessorConst = const std::vector<ga::InheritanceDesc>& (ga::ClassData::*)() const;

  struct GetClass
  {
    ga::ClassKeyType operator()(const ga::InheritanceDesc& desc) const
    {
      return desc.getClass();
    }
  };

  struct GetType
  {
    ga::ClassKeyType operator()(ga::VariableKeyType key) const
    {
      return compat->typeOf(key).classType();
    }

    sg::NotNull<const ga::AbstractCompatibilityBridge> compat;
  };

  struct IsValidClassKey
  {
    bool operator()(ga::ClassKeyType key) const
    {
      return ga::isValidKey(key);
    }
  };

  template <class AnalysisT, class MemFnSelector>
  auto inheritanceRelationships(AnalysisT&, MemFnSelector selector)
  {
    return [selector](auto& elem) // -> someRange
           {
             return (elem.second.*selector)()
                    | adapt::filtered(ga::IsDirect{})
                    | adapt::transformed(GetClass{});
           };
  }

  auto valueTypes(const CppHierarchyAnalysis& all,
                  const ga::ClassAnalysis::value_type& clazz) // -> someRange
  {
    return clazz.second.dataMembers()
           | adapt::transformed(GetType{&all.compatibilityBridge()})
           | adapt::filtered(IsValidClassKey{});
  }

  template <class AnalysisT, class Fn, class Selector>
  void traversal(AnalysisT& all, Fn fn, ga::ClassKeyType curr, Selector selector)
  {
    typename AnalysisT::iterator pos = all.find(curr);
    if (pos == all.end())
    {
      (void)all.at(curr); // issue the standard missing-class diagnostic
      return;
    }

    auto& elem = *pos;
    const ga::AbstractCompatibilityBridge& compat = all.compatibilityBridge();

    for (ga::ClassKeyType clazz : selector(elem))
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
             RelationAccessorConst ancestors = &ga::ClassData::ancestors;

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
                             [&all](ga::ClassAnalysis::value_type& rep) -> void
                             {
                               analyzeAutoGeneratedMemberFunctions(all, rep);
                             });
}

CppHierarchyAnalysis
analyzeCppClasses(Rose::Common::Analysis::ASTRootType root)
{
  CppHierarchyAnalysis classes;

  Rose::Common::Analysis::analyzeClasses(classes, root);
  analyzeSpecialMemberFunctions(classes);
  return classes;
}

CppHierarchyAnalysis
analyzeCppClassesFromMemoryPool()
{
  CppHierarchyAnalysis classes;

  Rose::Common::Analysis::analyzeClassesFromMemoryPool(classes);
  analyzeSpecialMemberFunctions(classes);
  return classes;
}

Rose::Common::Analysis::VirtualFunctionAnalysis
analyzeCppVirtualFunctions(const Rose::Common::Analysis::ClassAnalysis& classes, bool normalizedSignature)
{
  return Rose::Common::Analysis::analyzeVirtualFunctions(classes, normalizedSignature);
}

} // namespace Rose::Source::Analysis
