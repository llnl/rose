#include <sage3basic.h>
#include <Rose/Source/Analysis/CppCompatibility.h>

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>
#include <boost/variant.hpp>

#include <nodeQuery.h>
#include <sageGeneric.h>
#include <sageInterface.h>

#include <Rose/Source/Analysis/CppHierarchyAnalysis.h>

#if __cplusplus >= 201703L
#define ROSE_IF_CONSTEXPR constexpr
#else
#define ROSE_IF_CONSTEXPR
#endif

namespace si = SageInterface;
namespace rca = Rose::Common::Analysis;
namespace rsa = Rose::Source::Analysis;

namespace Rose::Source::Analysis
{

/// ROSE strip mask used to compare C++ types without modifier or typedef aliases.
constexpr unsigned char STRIP_MODIFIER_ALIAS =
  SgType::STRIP_MODIFIER_TYPE | SgType::STRIP_TYPEDEF_TYPE;

SgProject* keyToRoot(rca::ASTRootType key)
{
  return reinterpret_cast<SgProject*>(key.key);
}

rca::ASTRootType rootToKey(const SgProject* n)
{
  return rca::ASTRootType{reinterpret_cast<std::uintptr_t>(n)};
}

SgClassDefinition* keyToClass(rca::ClassKeyType key)
{
  return reinterpret_cast<SgClassDefinition*>(key.key);
}

rca::ClassKeyType classToKey(const SgClassDefinition* n)
{
  return rca::ClassKeyType{reinterpret_cast<std::uintptr_t>(n)};
}

SgFunctionDeclaration* keyToFunction(rca::FunctionKeyType key)
{
  return reinterpret_cast<SgFunctionDeclaration*>(key.key);
}

rca::FunctionKeyType functionToKey(const SgFunctionDeclaration* n)
{
  return rca::FunctionKeyType{reinterpret_cast<std::uintptr_t>(n)};
}

SgInitializedName* keyToVariable(rca::VariableKeyType key)
{
  return reinterpret_cast<SgInitializedName*>(key.key);
}

rca::VariableKeyType variableToKey(const SgInitializedName* n)
{
  return rca::VariableKeyType{reinterpret_cast<std::uintptr_t>(n)};
}

SgFunctionType* keyToFunctionType(rca::FunctionTypeKeyType key)
{
  return reinterpret_cast<SgFunctionType*>(key.key);
}

rca::FunctionTypeKeyType functionTypeToKey(const SgFunctionType* n)
{
  return rca::FunctionTypeKeyType{reinterpret_cast<std::uintptr_t>(n)};
}

SgType* keyToType(rca::TypeKeyType key)
{
  return reinterpret_cast<SgType*>(key.key);
}

rca::TypeKeyType typeToKey(const SgType* n)
{
  return rca::TypeKeyType{reinterpret_cast<std::uintptr_t>(n)};
}

} // namespace Rose::Source::Analysis

namespace
{
  template <class V>
  int cmpValue(const V& lhs, const V& rhs)
  {
    if (lhs < rhs) return -1;
    if (rhs < lhs) return 1;
    return 0;
  }

  bool templated(const SgNode* n)
  {
    for (const SgNode* cur = n; cur; cur = cur->get_parent())
    {
      if (isSgGlobal(cur))
        return false;

      if (  isSgTemplateClassDeclaration(cur)
         || isSgTemplateClassDefinition(cur)
         || isSgTemplateFunctionDeclaration(cur)
         || isSgTemplateFunctionDefinition(cur)
         || isSgTemplateMemberFunctionDeclaration(cur)
         || isSgTemplateVariableDeclaration(cur)
         )
        return true;
    }

    return false;
  }

  bool isSecondaryClassDefinition(const SgClassDefinition& n)
  {
    SgClassDeclaration* decl = n.get_declaration();
    if (!decl)
      return false;

    SgDeclarationStatement* defdcl = decl->get_definingDeclaration();
    SgClassDeclaration* clsdef = isSgClassDeclaration(defdcl);
    if (!clsdef)
      return false;

    return &n != clsdef->get_definition();
  }

  const SgType*
  stripVacuousModifierType(const SgModifierType& ty)
  {
    const SgTypeModifier&          tymod = ty.get_typeModifier();
    const SgConstVolatileModifier& cvmod = tymod.get_constVolatileModifier();
    const bool                     hasCxxRelevantModifier = cvmod.isConst() || cvmod.isVolatile();

    return hasCxxRelevantModifier ? &ty : ty.get_base_type();
  }

  const SgType*
  stripTypeAlias(const SgType* ty)
  {
    if (const SgTypedefType* tydef = isSgTypedefType(ty))
      return stripTypeAlias(tydef->get_base_type());

    if (const SgDeclType* dclty = isSgDeclType(ty))
      return stripTypeAlias(dclty->get_base_type());

    if (const SgModifierType* modty = isSgModifierType(ty))
    {
      const SgType* stripped = stripVacuousModifierType(*modty);
      if (stripped != ty)
        return stripTypeAlias(stripped);
    }

    return ty;
  }

  const SgType*
  stripForClassType(const SgType* ty, int otherStrips = 0)
  {
    if (!ty)
      return nullptr;

    const unsigned char stripFlags = rsa::STRIP_MODIFIER_ALIAS
                                   | SgType::STRIP_REFERENCE_TYPE
                                   | SgType::STRIP_RVALUE_REFERENCE_TYPE
                                   | otherStrips;

    const SgType* stripped = ty->stripType(stripFlags);
    if (const SgDeclType* dclty = isSgDeclType(stripped))
      stripped = dclty->get_base_type();

    return stripped;
  }

  SgClassDefinition*
  classDefFromType(const SgType* ty, int otherStrips = 0)
  {
    const SgType* stripped = stripForClassType(ty, otherStrips);
    const SgClassType* classTy = isSgClassType(stripped);

    if (!classTy)
      return nullptr;

    return rsa::getClassDefOpt(*classTy);
  }

  std::string typeSignature(const SgType* ty, bool arrayDecay)
  {
    if (!ty)
      return {};

    const SgType* stripped = stripTypeAlias(ty);

    if (arrayDecay)
    {
      if (const SgArrayType* arr = isSgArrayType(stripped))
        return std::string{"*"} + typeSignature(arr->get_base_type(), false);
    }

    return stripped->unparseToString();
  }

  std::string functionSignature(const SgFunctionType& ft, bool includeReturnType)
  {
    std::ostringstream out;

    out << "(";
    const SgTypePtrList& args = ft.get_arguments();
    for (std::size_t i = 0; i < args.size(); ++i)
    {
      if (i) out << ",";
      out << typeSignature(args[i], true);
    }
    out << ")";

    if (const SgMemberFunctionType* mft = isSgMemberFunctionType(&ft))
    {
      if (mft->isConstFunc()) out << " const";
      if (mft->isVolatileFunc()) out << " volatile";
      if (mft->isLvalueReferenceFunc()) out << " &";
      if (mft->isRvalueReferenceFunc()) out << " &&";
    }

    if (includeReturnType)
      out << "->" << typeSignature(ft.get_return_type(), true);

    return out.str();
  }

  bool isVirtual(const SgFunctionDeclaration& dcl)
  {
    return dcl.get_functionModifier().isVirtual();
  }

  SgFunctionDeclaration*
  representativeDecl(const SgFunctionDeclaration* fn)
  {
    if (!fn)
      return nullptr;

    SgDeclarationStatement* first = fn->get_firstNondefiningDeclaration();
    if (SgFunctionDeclaration* firstFn = isSgFunctionDeclaration(first))
      return firstFn;

    return const_cast<SgFunctionDeclaration*>(fn);
  }

  void collectMembers(rca::ClassAnalysis::value_type& entry)
  {
    SgClassDefinition* clsdef = rsa::keyToClass(entry.first);
    if (!clsdef)
      return;

    const SgDeclarationStatementPtrList& members = clsdef->get_members();
    rsa::CppCompatibility compat;

    for (SgDeclarationStatement* member : members)
    {
      if (SgMemberFunctionDeclaration* memfun = isSgMemberFunctionDeclaration(member))
      {
        rca::FunctionKeyType mfnKey = compat.functionId(memfun);
        if (isVirtual(*memfun))
          entry.second.virtualFunctions().emplace_back(mfnKey);
      }
      else if (SgVariableDeclaration* memvar = isSgVariableDeclaration(member))
      {
        if (si::isStatic(memvar))
          continue;

        for (SgInitializedName* var : memvar->get_variables())
          entry.second.dataMembers().emplace_back(compat.variableId(var));
      }
    }
  }

  void collectMembers(rca::ClassAnalysis& classes)
  {
    for (rca::ClassAnalysis::value_type& entry : classes)
      collectMembers(entry);
  }

  void addClassDefinition(rca::ClassAnalysis& classes, SgClassDefinition* classDef)
  {
    if (!classDef || templated(classDef) || isSecondaryClassDefinition(*classDef))
      return;

    auto emplaced = classes.emplace(rsa::classToKey(classDef), rca::ClassData{});
    emplaced.first->second.abstractClass(classDef->get_isAbstract());
  }

  rca::ClassKeyType
  classKeyFromDeclaration(const SgClassDeclaration* dcl)
  {
    if (!dcl)
      return rca::ClassKeyType{};

    SgDeclarationStatement* defdcl = dcl->get_definingDeclaration();
    SgClassDeclaration* clsdef = isSgClassDeclaration(defdcl);
    if (!clsdef || !clsdef->get_definition())
      return rca::ClassKeyType{};

    return rsa::classToKey(clsdef->get_definition());
  }

  struct ConstSymbolTableIterator
  {
    using iterator_category = std::input_iterator_tag;
    using value_type = const SgSymbol&;
    using pointer = const SgSymbol*;
    using reference = const SgSymbol&;
    using difference_type = std::ptrdiff_t;

    ConstSymbolTableIterator(const SgScopeStatement& s, const SgSymbol* el)
    : scope(&s), current(el)
    {}

    reference operator*() const { return *current; }
    pointer operator->() const { return current; }

    ConstSymbolTableIterator& operator++()
    {
      current = scope->next_any_symbol();
      return *this;
    }

    bool operator==(const ConstSymbolTableIterator& that) const { return current == that.current; }
    bool operator!=(const ConstSymbolTableIterator& that) const { return current != that.current; }

    const SgScopeStatement* scope;
    const SgSymbol* current;
  };

  ConstSymbolTableIterator symbolBegin(const SgScopeStatement& scope)
  {
    return {scope, scope.first_any_symbol()};
  }

  ConstSymbolTableIterator symbolLimit(const SgScopeStatement& scope)
  {
    return {scope, nullptr};
  }

  using CvQualificationBase = std::tuple<const SgType*, std::uint_fast8_t>;
  struct CvQualification : CvQualificationBase
  {
    enum Kind : std::uint8_t
    {
      none = 0,
      constqual = (1 << 0),
      volatilequal = (1 << 1),
      all = constqual | volatilequal
    };

    using base = CvQualificationBase;
    using base::base;

    const SgType* type() const { return std::get<0>(*this); }
    bool hasConst() const { return (std::get<1>(*this) & constqual) == constqual; }
    bool hasVolatile() const { return (std::get<1>(*this) & volatilequal) == volatilequal; }
  };

  std::uint_fast8_t cvQualifier(const SgConstVolatileModifier& cvmod,
                                std::uint_fast8_t qual = 0)
  {
    if (cvmod.isVolatile()) qual += CvQualification::volatilequal;
    if (cvmod.isConst()) qual += CvQualification::constqual;
    return qual;
  }

  CvQualification removeCvQual(const SgType* ty,
                               std::uint_fast8_t qualifier = CvQualification::none)
  {
    if (const SgModifierType* modTy = isSgModifierType(ty))
      return removeCvQual(modTy->get_base_type(),
                          cvQualifier(modTy->get_typeModifier().get_constVolatileModifier(), qualifier));

    if (const SgTypedefType* tydef = isSgTypedefType(ty))
      return removeCvQual(tydef->get_base_type(), qualifier);

    if (const SgDeclType* declTy = isSgDeclType(ty))
      return removeCvQual(declTy->get_base_type(), qualifier);

    return {ty, qualifier};
  }

  const SgType* removeReference(const SgType* ty)
  {
    if (const SgReferenceType* refTy = isSgReferenceType(ty))
      return refTy->get_base_type();

    return ty;
  }

  const SgType* removeRvalueReference(const SgType* ty)
  {
    if (const SgRvalueReferenceType* refTy = isSgRvalueReferenceType(ty))
      return refTy->get_base_type();

    return ty;
  }

  using ParameterTypeDescBase = std::tuple<bool, bool, bool, bool, bool>;
  struct ParameterTypeDesc : ParameterTypeDescBase
  {
    using base = ParameterTypeDescBase;
    using base::base;

    bool paramHasClassType() const { return std::get<0>(*this); }
    bool withReference() const { return std::get<1>(*this); }
    bool withRValueReference() const { return std::get<2>(*this); }
    bool withConst() const { return std::get<3>(*this); }
    bool withVolatile() const { return std::get<4>(*this); }
  };

  ParameterTypeDesc
  specialMemberFunctionParameter(const SgMemberFunctionDeclaration& mfn, rca::ClassKeyType clkey)
  {
    const SgFunctionParameterList* paramList = mfn.get_parameterList();
    if (!paramList || paramList->get_args().empty())
      return {};

    const SgInitializedName* parm = paramList->get_args().front();
    if (!parm || !parm->get_type())
      return {};

    const SgType* paramTy = stripTypeAlias(parm->get_type());
    const SgType* unRefTy = removeReference(paramTy);
    const bool hasRef = paramTy != unRefTy;
    const SgType* unRvalueRefTy = removeRvalueReference(paramTy);
    const bool hasRValueRef = paramTy != unRvalueRefTy;
    const SgType* afterRefTy = hasRef ? unRefTy : unRvalueRefTy;
    CvQualification unCvTy = removeCvQual(afterRefTy);
    const SgClassType* paramClassTy = isSgClassType(unCvTy.type());

    if (!paramClassTy || rsa::classToKey(rsa::getClassDefOpt(*paramClassTy)) != clkey)
      return {};

    return {true, hasRef, hasRValueRef, unCvTy.hasConst(), unCvTy.hasVolatile()};
  }

  ParameterTypeDesc specialMemberFunctionParameter(const SgMemberFunctionDeclaration& mfn)
  {
    return specialMemberFunctionParameter(mfn, rsa::classToKey(&rsa::getClassDefForFunction(mfn)));
  }

  bool isMoveParameterType(ParameterTypeDesc parm)
  {
    return parm.paramHasClassType()
        && parm.withRValueReference()
        && !parm.withConst()
        && !parm.withVolatile();
  }

  bool isCopyParameterType(ParameterTypeDesc parm)
  {
    return parm.paramHasClassType() && !parm.withRValueReference();
  }

  std::tuple<std::size_t, std::size_t>
  countArguments(const SgFunctionDeclaration& n)
  {
    const SgFunctionParameterList* paramList = n.get_parameterList();
    if (!paramList)
      return {0, 0};

    const SgInitializedNamePtrList& params = paramList->get_args();
    const auto beg = params.begin();
    const auto lim = params.end();
    auto defaulted =
      [](const SgInitializedName* p) -> bool
      {
        return p && p->get_initializer();
      };

    const auto pos = std::find_if(beg, lim, defaulted);
    return {static_cast<std::size_t>(std::distance(beg, pos)), params.size()};
  }

  bool hasDefaultAssignType(const SgMemberFunctionDeclaration& mfn,
                            rca::ClassKeyType clkey,
                            std::function<bool(ParameterTypeDesc)> pred)
  {
    std::size_t minargs = 0;
    std::size_t maxargs = 0;
    std::tie(minargs, maxargs) = countArguments(mfn);

    return minargs == 1 && maxargs == 1 && pred(specialMemberFunctionParameter(mfn, clkey));
  }

  bool
  hasDefaultCopyAssignType(const SgMemberFunctionDeclaration& mfn, rca::ClassKeyType clkey)
  {
    return hasDefaultAssignType(mfn, clkey, isCopyParameterType);
  }

  bool
  hasDefaultMoveAssignType(const SgMemberFunctionDeclaration& mfn, rca::ClassKeyType clkey)
  {
    return hasDefaultAssignType(mfn, clkey, isMoveParameterType);
  }

  rsa::SpecialMemberFunctionPredicate
  isCompilerGeneratedCopyAssignOverriding()
  {
    return [](const rsa::SpecialMemberFunction& smf) -> bool
           {
             return smf.isCopyAssign() && smf.compilerGenerated();
           };
  }

  std::unordered_set<rca::FunctionKeyType>
  userDefinedFunctions(rca::ClassKeyType clazz)
  {
    std::unordered_set<rca::FunctionKeyType> res;
    SgClassDefinition* cls = rsa::keyToClass(clazz);
    if (!cls)
      return res;

    rsa::CppCompatibility compat;
    for (const SgDeclarationStatement* member : cls->get_members())
    {
      if (const SgMemberFunctionDeclaration* mfn = isSgMemberFunctionDeclaration(member))
        res.insert(compat.functionId(mfn));
    }

    return res;
  }

  struct CategorizeFunctions
  {
    explicit CategorizeFunctions(std::unordered_set<rca::FunctionKeyType> userDefined)
    : userDefinedFunctions(std::move(userDefined))
    {}

    rsa::SpecialMemberFunction none(rca::FunctionKeyType key) const
    {
      return {key, rsa::SpecialMemberFunction::notspecial, false, false, true};
    }

    rsa::SpecialMemberFunction
    special( const SgMemberFunctionDeclaration& mfn,
             std::uint_fast8_t kind,
             bool copyWithConstRef = false,
             bool compliant = true
           ) const
    {
      rsa::CppCompatibility compat;
      rca::FunctionKeyType key = compat.functionId(&mfn);
      const bool compGen = userDefinedFunctions.find(key) == userDefinedFunctions.end();

      return {key, static_cast<std::int8_t>(kind), compGen, copyWithConstRef, compliant};
    }

    bool compliantSignature(const SgMemberFunctionDeclaration& mfn, std::uint_fast8_t kind) const
    {
      if ((kind & rsa::SpecialMemberFunction::cassign) != rsa::SpecialMemberFunction::cassign)
        return true;

      ParameterTypeDesc param = specialMemberFunctionParameter(mfn);
      if (param.withReference())
        return true;

      return !(param.withConst() || param.withVolatile());
    }

    bool copyCallableWithConstRef(const SgMemberFunctionDeclaration& mfn,
                                  std::uint_fast8_t kind) const
    {
      const bool copyAssignOrCtor =
        ((kind & rsa::SpecialMemberFunction::cassign) == rsa::SpecialMemberFunction::cassign)
        || ((kind & rsa::SpecialMemberFunction::cctor) == rsa::SpecialMemberFunction::cctor);

      if (!copyAssignOrCtor)
        return false;

      ParameterTypeDesc param = specialMemberFunctionParameter(mfn);
      return !param.withReference() || param.withConst();
    }

    std::uint_fast8_t chooseKind(const SgMemberFunctionDeclaration& fn,
                                 std::uint_fast8_t cp,
                                 std::uint_fast8_t mv) const
    {
      ParameterTypeDesc param = specialMemberFunctionParameter(fn);
      if (isMoveParameterType(param))
        return mv;

      if (isCopyParameterType(param))
        return cp;

      return rsa::SpecialMemberFunction::notspecial;
    }

    rsa::SpecialMemberFunction categorize(const SgMemberFunctionSymbol* sym) const
    {
      if (!sym)
        return none(rca::FunctionKeyType{});

      const SgMemberFunctionDeclaration* fn = sym->get_declaration();
      if (!fn)
        return none(rca::FunctionKeyType{});

      if (fn->get_specialFunctionModifier().isDestructor())
        return special(*fn, rsa::SpecialMemberFunction::dtor);

      std::size_t minargs = 0;
      std::size_t maxargs = 0;
      std::tie(minargs, maxargs) = countArguments(*fn);

      if (fn->get_specialFunctionModifier().isConstructor())
      {
        std::uint_fast8_t kind = rsa::SpecialMemberFunction::ctor;
        if (minargs == 0)
          kind |= rsa::SpecialMemberFunction::dctor;
        if (minargs <= 1 && maxargs > 0)
          kind |= chooseKind(*fn, rsa::SpecialMemberFunction::cctor, rsa::SpecialMemberFunction::mctor);

        return special(*fn, kind, copyCallableWithConstRef(*fn, kind));
      }

      if (fn->get_name() == "operator=")
      {
        std::uint_fast8_t kind = rsa::SpecialMemberFunction::notspecial;
        if (maxargs == 1)
          kind |= chooseKind(*fn, rsa::SpecialMemberFunction::cassign, rsa::SpecialMemberFunction::massign);

        return special(*fn, kind, copyCallableWithConstRef(*fn, kind), compliantSignature(*fn, kind));
      }

      return none(rsa::functionToKey(fn));
    }

    rsa::SpecialMemberFunction operator()(const SgSymbol& sym) const
    {
      return categorize(isSgMemberFunctionSymbol(&sym));
    }

    std::unordered_set<rca::FunctionKeyType> userDefinedFunctions;
  };

  std::uint32_t languageStandardFromROSEmacroFile(const std::string& macroFile, std::size_t pos)
  {
    pos = macroFile.find("++", pos);
    if (pos == std::string::npos || pos + 3 >= macroFile.size())
      return 0;

    pos += 2;
    const std::size_t year = (macroFile[pos] - '0') * 10 + (macroFile[pos + 1] - '0');

    if (year == 98) return rsa::CppHierarchyAnalysis::Cxx98;
    if (year == 11) return rsa::CppHierarchyAnalysis::Cxx11;
    if (year == 14) return rsa::CppHierarchyAnalysis::Cxx14;
    if (year == 17) return rsa::CppHierarchyAnalysis::Cxx17;
    if (year == 20) return rsa::CppHierarchyAnalysis::Cxx20;
    if (year == 23) return rsa::CppHierarchyAnalysis::Cxx23;
    if (year == 26) return rsa::CppHierarchyAnalysis::Cxx26;
    return 0;
  }

  std::uint32_t languageStandardFromFile(const SgSourceFile& file)
  {
    switch (file.get_standard())
    {
      case SgFile::e_cxx98_standard:
      case SgFile::e_cxx03_standard:
        return rsa::CppHierarchyAnalysis::Cxx98;
      case SgFile::e_cxx11_standard:
        return rsa::CppHierarchyAnalysis::Cxx11;
      case SgFile::e_cxx14_standard:
        return rsa::CppHierarchyAnalysis::Cxx14;
      case SgFile::e_cxx17_standard:
        return rsa::CppHierarchyAnalysis::Cxx17;
      case SgFile::e_cxx20_standard:
        return rsa::CppHierarchyAnalysis::Cxx20;
      case SgFile::e_cxx23_standard:
        return rsa::CppHierarchyAnalysis::Cxx23;
      case SgFile::e_cxx26_standard:
        return rsa::CppHierarchyAnalysis::Cxx26;
      case SgFile::e_default_standard:
      {
        const std::size_t pos = file.getFileName().find("rose_edg_required_compiler_macros_");
        return pos == std::string::npos ? 0 : languageStandardFromROSEmacroFile(file.getFileName(), pos);
      }
      default:
        return 0;
    }
  }

  std::uint32_t standardVersion(const std::vector<SgFile*>& files)
  {
    std::uint32_t version = 0;

    for (const SgFile* file : files)
    {
      const SgSourceFile* sourceFile = isSgSourceFile(file);
      if (!sourceFile)
        continue;

      const std::uint32_t fileVersion = languageStandardFromFile(*sourceFile);
      if (fileVersion == 0)
        continue;

      if (version == 0)
        version = fileVersion;
      else if (version != fileVersion)
        throw std::runtime_error("Inconsistent C++ standards detected");
    }

    return version == 0 ? rsa::CppHierarchyAnalysis::Cxx98 : version;
  }

  struct CollectSourceFilesFromMemoryPool : ROSE_VisitTraversal
  {
    void visit(SgNode* node) override
    {
      if (SgSourceFile* sf = isSgSourceFile(node))
        allSourceFiles.emplace_back(sf);
    }

    std::vector<SgFile*> allSourceFiles;
  };
}


namespace Rose::Source::Analysis
{

std::ostream& operator<<(std::ostream& os, const SourceLocation& loc)
{
  return os << loc.file() << ":" << loc.startLine() << ":" << loc.startCol();
}

rca::VariableKeyType
CppCompatibility::variableId(const SgInitializedName* var) const
{
  return variableToKey(var);
}

rca::VariableKeyType
CppCompatibility::variableId(const SgVariableDeclaration* var) const
{
  if (!var || var->get_variables().empty())
    return rca::VariableKeyType{};

  return variableId(var->get_variables().front());
}

rca::FunctionKeyType
CppCompatibility::functionId(const SgFunctionDeclaration* fun) const
{
  return functionToKey(representativeDecl(fun));
}

std::string
CppCompatibility::nameOf(rca::VariableKeyType key) const
{
  SgInitializedName* var = keyToVariable(key);
  return var ? var->get_name().getString() : "<invalid-variable>";
}

std::string
CppCompatibility::nameOf(rca::FunctionKeyType key) const
{
  SgFunctionDeclaration* fn = keyToFunction(key);
  return fn ? fn->get_name().getString() : "<invalid-function>";
}

std::string
CppCompatibility::nameOf(rca::ClassKeyType key) const
{
  SgClassDefinition* cls = keyToClass(key);
  if (!cls || !cls->get_declaration())
    return "<invalid-class>";

  return cls->get_declaration()->get_qualified_name().getString();
}

namespace
{
  struct NumericIdVisitor : boost::static_visitor<std::intptr_t>
  {
    template <class Key>
    std::intptr_t operator()(Key key) const
    {
      return static_cast<std::intptr_t>(key.key);
    }
  };

  struct UniqueNameVisitor : boost::static_visitor<std::string>
  {
    explicit UniqueNameVisitor(const CppCompatibility& compatibility)
      : compatibility(compatibility)
    {}

    template <class Key>
    std::string operator()(Key key) const
    {
      if ROSE_IF_CONSTEXPR (std::is_same<Key, rca::ClassKeyType>::value)
        return compatibility.nameOf(rca::ClassKeyType{key.key});
      else if ROSE_IF_CONSTEXPR (std::is_same<Key, rca::FunctionKeyType>::value)
      {
        SgFunctionDeclaration* fn = keyToFunction(rca::FunctionKeyType{key.key});
        return fn ? fn->get_qualified_name().getString() + functionSignature(*fn->get_type(), true)
                  : "<invalid-function>";
      }
      else if ROSE_IF_CONSTEXPR (std::is_same<Key, rca::VariableKeyType>::value)
      {
        SgInitializedName* var = keyToVariable(rca::VariableKeyType{key.key});
        return var ? var->get_qualified_name().getString() : "<invalid-variable>";
      }
      else if ROSE_IF_CONSTEXPR (std::is_same<Key, rca::TypeKeyType>::value)
        return typeSignature(keyToType(rca::TypeKeyType{key.key}), false);
      else
        return std::to_string(key.key);
    }

    const CppCompatibility& compatibility;
  };

  struct LocationVisitor : boost::static_visitor<const SgLocatedNode*>
  {
    template <class Key>
    const SgLocatedNode* operator()(Key key) const
    {
      if ROSE_IF_CONSTEXPR (std::is_same<Key, rca::ClassKeyType>::value)
        return keyToClass(rca::ClassKeyType{key.key});
      else if ROSE_IF_CONSTEXPR (std::is_same<Key, rca::FunctionKeyType>::value)
        return keyToFunction(rca::FunctionKeyType{key.key});
      else if ROSE_IF_CONSTEXPR (std::is_same<Key, rca::VariableKeyType>::value)
        return keyToVariable(rca::VariableKeyType{key.key});
      else
        return nullptr;
    }
  };
}

std::intptr_t
CppCompatibility::numericId(rca::AnyKeyType id) const
{
  return boost::apply_visitor(NumericIdVisitor{}, id);
}

std::string
CppCompatibility::uniqueName(rca::AnyKeyType id) const
{
  return boost::apply_visitor(UniqueNameVisitor{*this}, id);
}

SourceLocation
CppCompatibility::location(rca::AnyKeyType id, bool) const
{
  const SgLocatedNode* locNode = boost::apply_visitor(LocationVisitor{}, id);

  if (!locNode || !locNode->get_file_info())
    return {"", 0, 0, 0, 0};

  const Sg_File_Info* fi = locNode->get_file_info();
  return {fi->get_filenameString(), fi->get_line(), fi->get_col(),
          fi->get_line(), fi->get_col()};
}

int
CppCompatibility::compareNames(rca::FunctionKeyType lhs, rca::FunctionKeyType rhs) const
{
  return cmpValue(nameOf(lhs), nameOf(rhs));
}

int
CppCompatibility::compareFunctionTypes(rca::FunctionTypeKeyType lhs,
                                        rca::FunctionTypeKeyType rhs,
                                        bool exclReturnType) const
{
  SgFunctionType* lhsType = keyToFunctionType(lhs);
  SgFunctionType* rhsType = keyToFunctionType(rhs);

  if (lhsType == rhsType) return 0;
  if (!lhsType) return -1;
  if (!rhsType) return 1;

  return cmpValue(functionSignature(*lhsType, !exclReturnType),
                  functionSignature(*rhsType, !exclReturnType));
}

int
CppCompatibility::compareFunctionsByType(rca::FunctionKeyType lhs,
                                          rca::FunctionKeyType rhs,
                                          bool exclReturnType) const
{
  SgFunctionDeclaration* lhsFn = keyToFunction(lhs);
  SgFunctionDeclaration* rhsFn = keyToFunction(rhs);

  if (lhsFn == rhsFn) return 0;
  if (!lhsFn) return -1;
  if (!rhsFn) return 1;

  return compareFunctionTypes(functionTypeToKey(lhsFn->get_type()),
                              functionTypeToKey(rhsFn->get_type()),
                              exclReturnType);
}

int
CppCompatibility::compareTypes(rca::TypeKeyType lhs, rca::TypeKeyType rhs) const
{
  SgType* lhsType = keyToType(lhs);
  SgType* rhsType = keyToType(rhs);

  if (lhsType == rhsType) return 0;
  if (!lhsType) return -1;
  if (!rhsType) return 1;

  return cmpValue(typeSignature(lhsType, false), typeSignature(rhsType, false));
}

rca::FunctionTypeRelation
CppCompatibility::haveSameOrCovariantReturn(const rca::ClassAnalysis& classes,
                                             rca::FunctionKeyType basId,
                                             rca::FunctionKeyType drvId) const
{
  SgFunctionDeclaration* bas = keyToFunction(basId);
  SgFunctionDeclaration* drv = keyToFunction(drvId);

  if (!bas || !drv || !bas->get_type() || !drv->get_type())
    return rca::FunctionTypeRelation::unrelated;

  return areSameOrCovariant(classes,
                            typeToKey(bas->get_type()->get_return_type()),
                            typeToKey(drv->get_type()->get_return_type()));
}

rca::FunctionTypeRelation
CppCompatibility::areSameOrCovariant(const rca::ClassAnalysis& classes,
                                      rca::TypeKeyType basId,
                                      rca::TypeKeyType drvId) const
{
  SgType* basType = keyToType(basId);
  SgType* drvType = keyToType(drvId);

  if (!basType || !drvType)
    return rca::FunctionTypeRelation::unrelated;

  if (compareTypes(basId, drvId) == 0)
    return rca::FunctionTypeRelation::sametype;

  SgClassDefinition* basClass = classDefFromType(basType, SgType::STRIP_POINTER_TYPE);
  SgClassDefinition* drvClass = classDefFromType(drvType, SgType::STRIP_POINTER_TYPE);

  if (!basClass || !drvClass)
    return rca::FunctionTypeRelation::unrelated;

  rca::ClassKeyType basKey = classToKey(basClass);
  rca::ClassKeyType drvKey = classToKey(drvClass);

  if (basKey == drvKey)
    return rca::FunctionTypeRelation::sametype;

  return classes.isBaseOf(basKey, drvKey)
       ? rca::FunctionTypeRelation::covariant
       : rca::FunctionTypeRelation::unrelated;
}

void
CppCompatibility::extractFromMemoryPool(rca::ClassAnalysis& classes) const
{
  struct CollectClassesFromMemoryPool : ROSE_VisitTraversal
  {
    explicit CollectClassesFromMemoryPool(rca::ClassAnalysis& c) : classes(c) {}

    void visit(SgNode* node) override
    {
      SgClassDeclaration* classDeclaration = isSgClassDeclaration(node);
      if (!classDeclaration || templated(classDeclaration))
        return;

      addClassDefinition(classes, classDeclaration->get_definition());
    }

    rca::ClassAnalysis& classes;
  };

  CollectClassesFromMemoryPool traversal(classes);
  SgClassDeclaration::traverseMemoryPoolNodes(traversal);
  SgTemplateInstantiationDecl::traverseMemoryPoolNodes(traversal);

  collectMembers(classes);
}

void
CppCompatibility::extractFromProject(rca::ClassAnalysis& classes, rca::ASTRootType root) const
{
  SgProject* project = keyToRoot(root);
  if (!project)
    return;

  Rose_STL_Container<SgNode*> defs = NodeQuery::querySubTree(project, V_SgClassDefinition);
  for (SgNode* node : defs)
    addClassDefinition(classes, isSgClassDefinition(node));

  collectMembers(classes);
}

rca::LanguageSpecificFn
CppCompatibility::languageSpecificProcessing() const
{
  return [](rca::ClassAnalysis&) {};
}

void
CppCompatibility::inheritanceEdges(rca::ClassKeyType clkey,
                                    std::function<void(rca::ClassKeyType, rca::ClassKeyType, bool)> fn) const
{
  SgClassDefinition* cls = keyToClass(clkey);
  if (!cls)
    return;

  for (const SgBaseClass* bscls : cls->get_inheritances())
  {
    if (!bscls || !bscls->get_isDirectBaseClass())
      continue;

    rca::ClassKeyType baseKey = classKeyFromDeclaration(bscls->get_base_class());
    if (!rca::isValidKey(baseKey))
    {
      msgWarn() << "Skipping C++ base class without available definition under "
                << nameOf(clkey) << std::endl;
      continue;
    }

    SgBaseClassModifier* baseModifier = bscls->get_baseClassModifier();
    fn(clkey, baseKey, baseModifier && baseModifier->isVirtual());
  }
}

rca::DataMemberType
CppCompatibility::typeOf(rca::VariableKeyType var) const
{
  SgInitializedName* initName = keyToVariable(var);
  if (!initName)
    return {rca::ClassKeyType{}, false, false, false, false};

  const SgType* type = initName->get_type();
  const bool hasInitializer = initName->get_initializer() != nullptr;
  bool isConst = false;
  bool hasReference = false;
  bool hasArray = false;

  for (const SgType* cur = type; cur;)
  {
    if (const SgModifierType* mod = isSgModifierType(cur))
    {
      isConst = isConst || mod->get_typeModifier().get_constVolatileModifier().isConst();
      cur = mod->get_base_type();
      continue;
    }

    if (const SgReferenceType* ref = isSgReferenceType(cur))
    {
      hasReference = true;
      cur = ref->get_base_type();
      continue;
    }

    if (const SgRvalueReferenceType* ref = isSgRvalueReferenceType(cur))
    {
      hasReference = true;
      cur = ref->get_base_type();
      continue;
    }

    if (const SgArrayType* arr = isSgArrayType(cur))
    {
      hasArray = true;
      cur = arr->get_base_type();
      continue;
    }

    break;
  }

  SgClassDefinition* cls = classDefFromType(type, SgType::STRIP_ARRAY_TYPE);
  return {classToKey(cls), isConst, hasReference, hasArray, hasInitializer};
}

bool
CppCompatibility::isPureVirtual(rca::FunctionKeyType id) const
{
  SgFunctionDeclaration* fn = keyToFunction(id);
  return fn && fn->get_functionModifier().isPureVirtual();
}

bool
CppCompatibility::hasDefinition(rca::FunctionKeyType id) const
{
  SgFunctionDeclaration* fn = keyToFunction(id);
  return fn && fn->get_definingDeclaration();
}

bool
CppCompatibility::isAbstract(rca::ClassKeyType clkey) const
{
  SgClassDefinition* cls = keyToClass(clkey);
  return cls && cls->get_isAbstract();
}

rca::FuncNameFn
CppCompatibility::functionNaming() const
{
  return [this](rca::FunctionKeyType key) { return nameOf(key); };
}

rca::ClassNameFn
CppCompatibility::classNaming() const
{
  return [this](rca::ClassKeyType key) { return nameOf(key); };
}

rca::VarNameFn
CppCompatibility::variableNaming() const
{
  return [this](rca::VariableKeyType key) { return nameOf(key); };
}

bool hasSpecialFunction(const CppHierarchyAnalysis& classes, rca::ClassKeyType clazz, SpecialMemberFunctionPredicate pred)
{
  return hasSpecialFunction(classes.specialMemberFunctions(clazz), std::move(pred));
}


bool
CppCompatibility::isAutoGeneratable(const rca::ClassAnalysis& all,
                                    rca::ClassKeyType clkey,
                                    rca::FunctionKeyType fnkey) const
{
  const CppHierarchyAnalysis& classes = dynamic_cast<const CppHierarchyAnalysis&>(all);

  if (!rca::isValidKey(clkey) || !rca::isValidKey(fnkey))
    return false;

  const SgMemberFunctionDeclaration* memfn = isSgMemberFunctionDeclaration(keyToFunction(fnkey));
  if (!memfn)
    return false;

  if (classToKey(&getClassDefForFunction(*memfn)) == clkey)
    return false;

  if (memfn->get_specialFunctionModifier().isDestructor())
    return !hasSpecialFunction(classes, clkey, isDtor());

  if (memfn->get_name() != "operator=")
    return false;

  if (hasDefaultCopyAssignType(*memfn, clkey))
    return hasSpecialFunction(classes, clkey, isCompilerGeneratedCopyAssignOverriding());

  if (hasDefaultMoveAssignType(*memfn, clkey))
    return hasSpecialFunction(classes, clkey, isMoveAssign());

  return false;
}

std::uint32_t
CppCompatibility::languageStandard() const
{
  CollectSourceFilesFromMemoryPool collector;
  SgSourceFile::traverseMemoryPoolNodes(collector);
  return standardVersion(collector.allSourceFiles);
}

rsa::SpecialMemberFunctionContainer
CppCompatibility::specialMemberFunctions(rca::ClassKeyType clazz) const
{
  SgClassDefinition* cls = keyToClass(clazz);
  if (!cls)
    return {};

  std::unordered_set<rca::FunctionKeyType> knownUserFunctions = userDefinedFunctions(clazz);
  CategorizeFunctions categorize{std::move(knownUserFunctions)};
  rsa::SpecialMemberFunctionContainer res;

  for (ConstSymbolTableIterator it = symbolBegin(*cls), lim = symbolLimit(*cls); it != lim; ++it)
  {
    rsa::SpecialMemberFunction smf = categorize(*it);
    if (smf.isSpecialFunc())
      res.push_back(smf);
  }

  return res;
}

Optional
CppCompatibility::classType(rca::FunctionKeyType fn) const
{
  SgFunctionDeclaration* fndcl = keyToFunction(fn);
  SgMemberFunctionDeclaration* memfn = isSgMemberFunctionDeclaration(fndcl);
  if (!memfn)
    return {};

  SgClassDefinition* cls = &getClassDefForFunction(*memfn);
  return classToKey(cls);
}

std::pair<rca::ClassKeyType, rca::TypeKeyType>
getClassCastInfo(rca::TypeKeyType tykey)
{
  SgType* ty = keyToType(tykey);
  SgClassDefinition* cls = classDefFromType(ty, SgType::STRIP_POINTER_TYPE);
  return {classToKey(cls), typeToKey(stripForClassType(ty, SgType::STRIP_POINTER_TYPE))};
}

SgClassDefinition&
getClassDef(const SgDeclarationStatement& n)
{
  const SgClassDeclaration* dcl = isSgClassDeclaration(&n);
  if (!dcl)
    throw std::runtime_error("declaration is not a class declaration");

  SgClassDefinition* def = dcl->get_definition();
  if (!def)
    throw std::runtime_error("class declaration does not define a class");

  return *def;
}

SgClassDefinition&
getClassDefForFunction(const SgMemberFunctionDeclaration& n)
{
  SgScopeStatement* scope = n.get_scope();
  SgClassDefinition* cls = isSgClassDefinition(scope);
  if (!cls)
    throw std::runtime_error("member function is not scoped by a class definition");

  return *cls;
}

SgClassDefinition*
getClassDefOpt(const SgClassType& n)
{
  const SgDeclarationStatement* dcl = n.get_declaration();
  if (!dcl)
    return nullptr;

  const SgClassDeclaration* clsdecl = isSgClassDeclaration(dcl->get_definingDeclaration());
  return clsdecl ? clsdecl->get_definition() : nullptr;
}

SgClassDefinition*
getClassDefOpt(const SgInitializedName& n)
{
  return classDefFromType(n.get_type());
}

SgFunctionDeclaration&
keyDecl(SgFunctionDeclaration& fn)
{
  return *representativeDecl(&fn);
}

boost::optional<std::string>
missingDiagnostics(rca::ClassKeyType key)
{
  if (SgClassDefinition* cls = keyToClass(key))
    if (templated(cls))
      return std::string{"class is under a template declaration/definition"};

  return {};
}

std::string
dbgInfo(rca::ClassKeyType key, std::size_t)
{
  CppCompatibility compat;
  return compat.nameOf(key);
}

} // namespace Rose::Source::Analysis
