#include <sage3basic.h>
#include <Rose/Source/Analysis/ExternalMangling.h>

#include <cctype>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include <set> // ?
#include <iostream> // ?

#include <boost/range/adaptors.hpp>
#include <boost/range/numeric.hpp>
#include <boost/algorithm/string/replace.hpp>
//~ #include <boost/core/bit.hpp>
//~ #include <boost/functional/hash.hpp>

#include <Combinatorics.h>
#include <Rose/Diagnostics.h>
#include <sageGeneric.h>
#include <sageInterface.h>
#include <Rose/Source/Analysis/CppCompatibility.h>

// #include "CodeThornLib.h"
// #include "RoseCompatibility.h"


#define WITH_CUSTOM_LINKAGE_HANDLER 0 // the code is not yet ready

namespace si = SageInterface;
namespace adapt = boost::adaptors;
//~ namespace ct = CodeThorn;
namespace rsa = Rose::Source::Analysis;

namespace
{
inline
auto roseTrace() -> decltype(Rose::Diagnostics::mlog[Sawyer::Message::TRACE])
{
  return Rose::Diagnostics::mlog[Sawyer::Message::TRACE];
}

inline
auto roseInfo() -> decltype(Rose::Diagnostics::mlog[Sawyer::Message::INFO])
{
  return Rose::Diagnostics::mlog[Sawyer::Message::INFO];
}

inline
auto roseWarn() -> decltype(Rose::Diagnostics::mlog[Sawyer::Message::WARN])
{
  return Rose::Diagnostics::mlog[Sawyer::Message::WARN];
}

inline
auto roseError() -> decltype(Rose::Diagnostics::mlog[Sawyer::Message::ERROR])
{
  return Rose::Diagnostics::mlog[Sawyer::Message::ERROR];
}

inline
auto roseFatal() -> decltype(Rose::Diagnostics::mlog[Sawyer::Message::FATAL])
{
  return Rose::Diagnostics::mlog[Sawyer::Message::FATAL];
}
}


namespace
{
  std::set<std::string> unhandledSageNodeTypes;

  // ROSE string name mangling literals

  constexpr const char* const mglVoid                     = "v";
  constexpr const char* const mglGlobalVoid               = "gv";

  constexpr const char* const mglDefaultType              = "__DefTpb_";
  constexpr const char* const mglDefaultTypeDelim         = "__DefTpe_";

  constexpr const char* const mglBool                     = "b";

  constexpr const char* const mglChar                     = "c";
  constexpr const char* const mglSignedChar               = "si";
  constexpr const char* const mglUnsignedChar             = "Uc";
  constexpr const char* const mglWChar                    = "wc";
  constexpr const char* const mglChar16                   = "c16";
  constexpr const char* const mglChar32                   = "c32";

  constexpr const char* const mglSignedShort              = "s";
  constexpr const char* const mglUnsignedShort            = "Us";
  constexpr const char* const mglSignedInt                = "i";
  constexpr const char* const mglUnsignedInt              = "Ui";
  constexpr const char* const mglSignedLong               = "l";
  constexpr const char* const mglUnsignedLong             = "Ul";

  constexpr const char* const mglSignedLongLong           = "SL";
  constexpr const char* const mglUnsignedLongLong         = "UL";
  constexpr const char* const mglSignedInt128             = "SL128";
  constexpr const char* const mglUnsignedInt128           = "UL128";

  constexpr const char* const mglFloat                    = "f";
  constexpr const char* const mglDouble                   = "d";
  constexpr const char* const mglLongDouble               = "L";
  constexpr const char* const mglFloat16                  = "x";
  constexpr const char* const mglFloat32                  = "y";
  constexpr const char* const mglFloat80                  = "w";
  constexpr const char* const mglFloat128                 = "q";

  constexpr const char* const mglComplex                  = "Complex";

  constexpr const char* const mglEllipse                  = "e";
  constexpr const char* const mglNullptr                  = "nullptr_t";

  constexpr const char* const mglFunctionType             = "_Fb_";
  constexpr const char* const mglFunctionTypeDelim        = "_Fe_";
  constexpr const char* const mglMemberFunctionType       = "__MFb_";

  constexpr const char* const mglPointerType              = "_Pb_";
  constexpr const char* const mglPointerTypeDelim         = "_Pe_";
  constexpr const char* const mglPointerMemberType        = "_PMb_";

  constexpr const char* const mglReferenceType            = "_Rb_";
  constexpr const char* const mglReferenceTypeDelim       = "_Re_";

  constexpr const char* const mglRvalueReferenceType      = "_RValueb_";
  constexpr const char* const mglRvalueReferenceTypeDelim = "_RValuee_";

  constexpr const char* const mglArrayType                = "_Ab_";
  constexpr const char* const mglArrayTypeDelim           = "_Ae_";
  constexpr const char* const mglArrayIndex               = "_index_";
  constexpr const char* const mglArrayVL                  = "vla_dim";

  constexpr const char* const mglAnonymousTypeName        = "_anontype_";

  constexpr const char* const mglModifierAliased          = "A";
  constexpr const char* const mglModifierNotNull          = "N";
  constexpr const char* const mglModifierConst            = "C";
  constexpr const char* const mglModifierVolatile         = "V";
  constexpr const char* const mglModifierRestrict         = "RST";
  constexpr const char* const mglModifierUpcShared        = "SHD";
  constexpr const char* const mglModifierUpcStrict        = "STR";
  constexpr const char* const mglModifierUpcRelaxed       = "RLX";

  constexpr const char* const mglMemFuncConst             = "_cf";
  constexpr const char* const mglMemFuncVolatile          = "_vf";
  constexpr const char* const mglMemFuncRestrict          = "_rf";
  constexpr const char* const mglMemFuncLvalueRef         = "_lvf";
  constexpr const char* const mglMemFuncRvalueRef         = "_rvf";

  constexpr const char* const mglMemFuncDtor              = "__dt";
  constexpr const char* const mglFuncOperatorNew          = "__nw";
  constexpr const char* const mglFuncOperatorNewArray     = "__na";
  constexpr const char* const mglFuncOperatorDel          = "__dl";
  constexpr const char* const mglFuncOperatorDelArray     = "__da";
  constexpr const char* const mglFuncOperatorConv         = "__opB__";
  constexpr const char* const mglFuncOperatorConvDelim    = "__opE__";

  constexpr const char* const mglFuncOperatorArrow        = "__rf";
  constexpr const char* const mglFuncOperatorArrowStar    = "__rm";
  constexpr const char* const mglFuncOperatorEquals       = "__eq";
  constexpr const char* const mglFuncOperatorNotEquals    = "__ne";
  constexpr const char* const mglFuncOperatorLogicalAnd   = "__aa";
  constexpr const char* const mglFuncOperatorLogicalOr    = "__oo";
  constexpr const char* const mglFuncOperatorNot          = "__nt";
  constexpr const char* const mglFuncOperatorXor          = "__er";
  constexpr const char* const mglFuncOperatorOr           = "__or";
  constexpr const char* const mglFuncOperatorAnd          = "__ad";
  constexpr const char* const mglFuncOperatorLShift       = "__ls";
  constexpr const char* const mglFuncOperatorRShift       = "__rs";
  constexpr const char* const mglFuncOperatorDec          = "__mm";
  constexpr const char* const mglFuncOperatorInc          = "__pp";
  constexpr const char* const mglFuncOperatorComplement   = "__cm";
  constexpr const char* const mglFuncOperatorComma        = "__cm";
  constexpr const char* const mglFuncOperatorApply        = "__cl";
  constexpr const char* const mglFuncOperatorSubscript    = "__xi";
  constexpr const char* const mglFuncOperatorAssign       = "__as";
  constexpr const char* const mglFuncOperatorPlus         = "__pl";
  constexpr const char* const mglFuncOperatorMinus        = "__mi";
  constexpr const char* const mglFuncOperatorMul          = "__ml";
  constexpr const char* const mglFuncOperatorDiv          = "__dv";
  constexpr const char* const mglFuncOperatorMod          = "__md";
  constexpr const char* const mglFuncOperatorAndAssign    = "__aad";
  constexpr const char* const mglFuncOperatorOrAssign     = "__aor";
  constexpr const char* const mglFuncOperatorXorAssign    = "__aer";
  constexpr const char* const mglFuncOperatorLShiftAssign = "__adv";
  constexpr const char* const mglFuncOperatorRShiftAssign = "__amd";
  constexpr const char* const mglFuncOperatorPlusAssign   = "__apl";
  constexpr const char* const mglFuncOperatorMinusAssign  = "__ami";
  constexpr const char* const mglFuncOperatorMulAssign    = "__amu";
  constexpr const char* const mglFuncOperatorDivAssign    = "__adv";
  constexpr const char* const mglFuncOperatorModAssign    = "__amd";
  constexpr const char* const mglFuncOperatorLess         = "__lt";
  constexpr const char* const mglFuncOperatorLessThan     = "__le";
  constexpr const char* const mglFuncOperatorGreater      = "__gt";
  constexpr const char* const mglFuncOperatorGreaterThan  = "__gq";
  constexpr const char* const mglFuncOperatorLiteral      = "__li";

  constexpr const char* const mglTypedefDeclaration       = "__tdefB__";
  constexpr const char* const mglTypedefDeclarationDelim  = "__tdefE__";
  constexpr const char* const mglEnum                     = "_enum_";
  constexpr const char* const mglClass                    = "_class_";
  constexpr const char* const mglTemplateArgs             = "_tas_";
  constexpr const char* const mglTemplateArgsDelim        = "_tab_";
  constexpr const char* const mglTemplateArgumentPack     = "start_of_pack_expansion_argument";
  constexpr const char* const mglParameterIdx             = "_parmidx_";
  constexpr const char* const mglVariableLoc              = "_varlocs_";
  constexpr const char* const mglUnknownType              = "__unknowntype";

  constexpr const char* const mglElideFunctionScope       = "_function_parameter_scope_";

  constexpr const char* const mglSimpleSep                = "_";
  constexpr const char* const mglExtendedSep              = "_sep_";

  template <class Seq>
  void append(std::string& s, const Seq& seq) { s.append(seq.begin(), seq.end()); }

  void append(std::string& s, const SgName& name) { append(s, name.getString()); }
  void append(std::string& s, const char* el) { s.append(el); }

  void join_(std::string&) {}

  template<typename First, typename... Rest>
  void join_(std::string& s, First&& first, Rest&&... rest)
  {
    append(s, first);

    join_(s, rest...);
  }

  template <class... Args>
  std::string join(std::string s, Args&&... args)
  {
    join_(s, std::forward<Args>(args)...);
    return s;
  }

  void joinNames_(std::string&) {}

  template<typename First, typename... Rest>
  void joinNames_(std::string& os, First&& first, Rest&&... rest)
  {
    os.append(first);

    joinNames_(os, rest...);
  }

  template <class... Args>
  std::string joinNames(std::string s, Args&&... args)
  {
    joinNames_(s, std::forward<Args>(args)...);
    return s;
  }

  const SgScopeStatement*
  enclosingScope(const SgNode& n)
  {
    return si::getEnclosingScope(const_cast<SgNode*>(&n));
  }

  template <class SageDeclaration>
  const SageDeclaration&
  definingDeclarationIfAvailable(const SageDeclaration& dcl)
  {
    SgDeclarationStatement* defdcl = dcl.get_definingDeclaration();

    if (defdcl == nullptr)
      return dcl;

    ASSERT_require(dcl.variantT() == defdcl->variantT());
    return static_cast<const SageDeclaration&>(*defdcl);
  }

  std::string replaceFirst(std::string s, const std::string& pat, const std::string& rep)
  {
    boost::algorithm::replace_first(s, pat, rep);
    return s;
  }

  std::string todo(const char* knd, const SgNode& n)
  {
    std::string res = knd + n.class_name();

    unhandledSageNodeTypes.insert(res);
    return res;
  }

  template <class SageDeclarationPtr>
  bool anonymousTypedef_(SageDeclarationPtr dcl)
  {
    return (  dcl
           //~ && (!dcl->get_isAutonomousDeclaration())
           && dcl->get_isUnNamed()
           );
  }

/*
  bool anonymousTypedef(const SgClassDeclaration* dcl)
  {
    return anonymousTypedef_(dcl);
  }

  bool anonymousTypedef(const SgEnumDeclaration* dcl)
  {
    return anonymousTypedef_(dcl);
  }
*/

#if WITH_CUSTOM_LINKAGE_HANDLER
  using StaticSuffixFn = std::function<std::string()>;

  StaticSuffixFn noStaticSuffix()
  {
    return []() -> std::string { return {}; };
  }

  StaticSuffixFn noStaticSuffixWarning()
  {
    return
        []() -> std::string
        {
          msgWarn() << "mangling node with internal/static linkage w/o location context"
                    << std::endl;
          return {};
        };
  }
#endif /* WITH_CUSTOM_LINKAGE_HANDLER */

  std::string
  internalLinkageFromFile(const SgLocatedNode& n)
  {
    sg::NotNull<const Sg_File_Info> fi = n.get_file_info();
    std::string                     linkageCandidate = fi->get_filenameString();

    if (linkageCandidate == "NULL_FILE")
    {
      if (const SgDeclarationStatement* dcl = isSgDeclarationStatement(&n))
      {
        // PP: For any declaration with internal linkage, a defining declaration
        //     should be available..
        if (const SgDeclarationStatement* defdcl = dcl->get_definingDeclaration())
          return internalLinkageFromFile(*defdcl);
      }
    }

    if (linkageCandidate.size() == 0)
      return internalLinkageFromFile(SG_DEREF(isSgLocatedNode(n.get_parent())));

    if (linkageCandidate != "NULL_FILE") // \pp todo revise
      return rsa::mangleFilename(linkageCandidate);

    msgWarn() << "mangling node with internal linkage w/o usable location info: " << n.class_name()
              << std::endl;
    return {};
  }


#if WITH_CUSTOM_LINKAGE_HANDLER
  StaticSuffixFn
  staticSuffixFromLocation(const SgLocatedNode& n)
  {
    return [&n]() -> std::string { return internalLinkageFromFile(n); };
  }
#endif /* WITH_CUSTOM_LINKAGE_HANDLER */

  struct ExternalManglingContext
  {
    enum Arguments : bool
    {
      WITH_UNIQUE_MAIN     = true,
      NO_UNIQUE_MAIN       = !WITH_UNIQUE_MAIN,
      DEFAULT_UNIQUE_MAIN  = NO_UNIQUE_MAIN,

      WITH_RETURN_TYPE     = true,
      NO_RETURN_TYPE       = !WITH_RETURN_TYPE,
      DEFAULT_RETURN_TYPE  = WITH_RETURN_TYPE,

      ARRAY_TO_PTR         = true,
      NO_ARRAY_TO_PTR      = !ARRAY_TO_PTR,
      DEFAULT_ARRAY_TO_PTR = NO_ARRAY_TO_PTR,
    };

    ExternalManglingContext inclReturnType(bool b = DEFAULT_RETURN_TYPE) const;
    ExternalManglingContext arrayDecay(bool b = DEFAULT_ARRAY_TO_PTR) const;
    ExternalManglingContext resetTypeFlags() const;

#if WITH_CUSTOM_LINKAGE_HANDLER
    ExternalManglingContext localLinkageSuffix(StaticSuffixFn fn = noStaticSuffixWarning()) const;
#endif /* WITH_CUSTOM_LINKAGE_HANDLER */

    bool           makeMainUnique     = DEFAULT_UNIQUE_MAIN;
    bool           withReturnType     = DEFAULT_RETURN_TYPE;
    bool           arrayAsPointerType = DEFAULT_ARRAY_TO_PTR;

#if WITH_CUSTOM_LINKAGE_HANDLER
    StaticSuffixFn internalLinkageGen = noStaticSuffixWarning();
#endif /* WITH_CUSTOM_LINKAGE_HANDLER */
  };

  ExternalManglingContext
  ExternalManglingContext::inclReturnType(bool b) const
  {
    ExternalManglingContext cpy(*this);

    cpy.withReturnType = b;
    return cpy;
  }

  ExternalManglingContext
  ExternalManglingContext::arrayDecay(bool b) const
  {
    ExternalManglingContext cpy(*this);

    cpy.arrayAsPointerType = b;
    return cpy;
  }

  ExternalManglingContext
  ExternalManglingContext::resetTypeFlags() const
  {
    ExternalManglingContext cpy(*this);

    cpy.withReturnType     = DEFAULT_RETURN_TYPE;
    cpy.arrayAsPointerType = DEFAULT_ARRAY_TO_PTR;

    return cpy;
  }

#if WITH_CUSTOM_LINKAGE_HANDLER
  ExternalManglingContext
  ExternalManglingContext::localLinkageSuffix(StaticSuffixFn fn) const
  {
    ExternalManglingContext cpy(*this);

    cpy.internalLinkageGen = fn;
    return cpy;
  }

  template<typename T, typename... U>
  std::size_t addressOf(std::function<T(U...)> f)
  {
    using fnPtrType = T(*)(U...);

    fnPtrType* fnPointer = f.template target<fnPtrType>();

    return reinterpret_cast<std::size_t>(*fnPointer);
  }
#endif /*WITH_CUSTOM_LINKAGE_HANDLER*/

  bool
  operator==(const ExternalManglingContext& lhs, const ExternalManglingContext& rhs)
  {
    return (  (lhs.makeMainUnique == rhs.makeMainUnique)
           && (lhs.withReturnType == rhs.withReturnType)
           && (lhs.arrayAsPointerType == rhs.arrayAsPointerType)
#if WITH_CUSTOM_LINKAGE_HANDLER
           && (addressOf(lhs.internalLinkageGen) == addressOf(rhs.internalLinkageGen))
#endif /*WITH_CUSTOM_LINKAGE_HANDLER*/
           );
  }


  std::string externalName(const SgNode& n, ExternalManglingContext ctx = {});

  std::string externalName(sg::NotNull<const SgNode> n, ExternalManglingContext ctx = {})
  {
    return externalName(*n, std::move(ctx));
  }

  struct ExternalMangler : sg::DispatchHandler<std::string>
  {
      using Ctx = ExternalManglingContext;

      explicit
      ExternalMangler(Ctx context)
      : Base(), ctx(std::move(context))
      {}

      //
      // helpers
      //

      /// mangles the base type of \p n
      template <class SageNode>
      std::string mangleBase(const SageNode& n)
      {
        std::string mangled;

        std::swap(res, mangled);
        handle(sg::asBaseType(n));
        std::swap(res, mangled);

        return mangled;
      }

      std::string mangleEnclosingScope(const SgScopeStatement& n);
      std::string mangleEnclosingScope(sg::NotNull<const SgScopeStatement> n) { return mangleEnclosingScope(*n); }

      std::string mangleParentNode(const SgNode& n);

      std::string mangleScope(const SgScopeStatement& n);
      std::string mangleScope(sg::NotNull<const SgScopeStatement> n)
      {
        return mangleScope(*n);
      }

      //~ std::string mangleModifier(const SgFunctionModifier&);
      //~ std::string mangleModifier(const SgDeclarationModifier&);
      //~ std::string mangleModifier(const SgTypeModifier&);
      //~ std::string mangleModifier(const SgConstVolatileModifier&);

      // bool withReturn = Ctx::WITH_RETURN_TYPE, bool pointerToArray = Ctx::NO_ARRAY_TO_PTR

      std::string mangleType(const SgType&, ExternalManglingContext ctx);
      std::string mangleType(sg::NotNull<const SgType> n, ExternalManglingContext ctx)
      {
        return mangleType(*n, std::move(ctx));
      }

      std::string mangleReturnType(const SgType&);
      std::string mangleReturnType(sg::NotNull<const SgType> n)
      {
        return mangleReturnType(*n);
      }

      template <class ForwardIterator>
      std::string mangleTypeRange(ForwardIterator beg, ForwardIterator lim);

      template <class Sequence>
      std::string mangleNodeSequence(Sequence&& rng, const char* listsep = mglSimpleSep, bool arrayToPointer = Ctx::NO_ARRAY_TO_PTR);

      std::string mangleTypeRange(const std::vector<SgType*>& vec);

      std::string mangleDeclaration(const SgDeclarationStatement&);
      std::string mangleDeclaration(sg::NotNull<const SgDeclarationStatement> n)
      {
        return mangleDeclaration(*n);
      }

      std::string mangleInternalLinkage(const SgLocatedNode&);

      std::string mangleInternalLinkageIfStatic(const SgDeclarationStatement&);
      std::string mangleInternalLinkageIfStatic(const sg::NotNull<SgDeclarationStatement> n)
      {
        return mangleInternalLinkageIfStatic(*n);
      }

      std::string mangleInternalLinkageIfStaticVariable(const SgVariableDeclaration* n)
      {
        if (n) return mangleInternalLinkageIfStatic(*n);

        return {};
      }

      std::string mangleInternalLinkageIfAnonymous(const SgDeclarationStatement& n, const std::string& name);

      std::string mangleInitializedName(const SgInitializedName&);
      std::string mangleInitializedName(sg::NotNull<const SgInitializedName> n)
      {
        return mangleInitializedName(*n);
      }

  /*
      std::string mangleParameterlist(const SgFunctionParameterList&);
      std::string mangleParameterlist(sg::NotNull<const SgFunctionParameterList> n)
      {
        return mangleParameterlist(*n);
      }
  */
      std::string mangleArrayIndexExpressionOpt(bool vla, const SgExpression* idx);
      std::string mangleMemberFunctionQualifiers(const SgMemberFunctionType& n);
      std::string mangleIndirection(const char* prefix, const char* suffix, const SgType*);


      //
      //

      std::string scopeQualifiedName(const SgNamespaceDeclarationStatement& n);
      std::string scopeQualifiedName(const SgFunctionDeclaration& n, std::string linkage = {});
      std::string scopeQualifiedName(const SgInitializedName& n);


      std::string functionName(const SgFunctionDeclaration& n);
      std::string functionName(const SgFunctionDeclaration& n, std::string name);
      std::string simpleName(std::string n, const char* caller = "");
      // std::string simpleName(const char* prefix, std::string n, const char* caller = "");


      //
      // handlers
      //

      void handle(const SgNode& n) { SG_UNEXPECTED_NODE(n); }

      //
      // types

      //~ void handle(const SgType& n) { res = todo("typ:", n); }

      void handle(const SgTypeVoid&)                  { res = mglVoid; }
      void handle(const SgTypeGlobalVoid&)            { res = mglGlobalVoid; }

      void handle(const SgTypeBool&)                  { res = mglBool; }

      void handle(const SgTypeChar&)                  { res = mglChar; }
      void handle(const SgTypeSignedChar&)            { res = mglSignedChar; }
      void handle(const SgTypeUnsignedChar&)          { res = mglUnsignedChar; }
      void handle(const SgTypeWchar&)                 { res = mglWChar; }
      void handle(const SgTypeChar16&)                { res = mglChar16; }
      void handle(const SgTypeChar32&)                { res = mglChar32; }

      void handle(const SgTypeShort&)                 { res = mglSignedShort; }
      void handle(const SgTypeSignedShort&)           { res = mglSignedShort; }
      void handle(const SgTypeUnsignedShort&)         { res = mglUnsignedShort; }

      void handle(const SgTypeInt&)                   { res = mglSignedInt; }
      void handle(const SgTypeSignedInt&)             { res = mglSignedInt; }
      void handle(const SgTypeUnsignedInt&)           { res = mglUnsignedInt; }

      void handle(const SgTypeLong&)                  { res = mglSignedLong; }
      void handle(const SgTypeSignedLong&)            { res = mglSignedLong; }
      void handle(const SgTypeUnsignedLong&)          { res = mglUnsignedLong; }

      void handle(const SgTypeLongLong&)              { res = mglSignedLongLong; }
      void handle(const SgTypeSignedLongLong&)        { res = mglSignedLongLong; }
      void handle(const SgTypeUnsignedLongLong&)      { res = mglUnsignedLongLong; }
      void handle(const SgTypeSigned128bitInteger&)   { res = mglSignedInt128; }
      void handle(const SgTypeUnsigned128bitInteger&) { res = mglUnsignedInt128; }

      void handle(const SgTypeFloat&)                 { res = mglFloat; }
      void handle(const SgTypeDouble&)                { res = mglDouble; }
      void handle(const SgTypeLongDouble&)            { res = mglLongDouble; }
      void handle(const SgTypeFloat16&)               { res = mglFloat16; }
      void handle(const SgTypeFloat32&)               { res = mglFloat32; }
      void handle(const SgTypeFloat80&)               { res = mglFloat80; }
      void handle(const SgTypeFloat128&)              { res = mglFloat128; }

      void handle(const SgTypeComplex& n)
      {
        ASSERT_require(n.get_type_kind() == nullptr);
        res = join( mglComplex
                  , mangleType(n.get_base_type(), ctx.resetTypeFlags())
                  );
      }

      void handle(const SgTypeEllipse&)           { res = mglEllipse; }
      void handle(const SgTypeNullptr&)           { res = mglNullptr; }
      // void handle(const SgTypeUnknown&)           { res = mglUnknownType; /*todo*/}

      void handle(const SgDeclType& n)            { res = mangleType(n.get_base_type(), ctx.resetTypeFlags()); }

      void handle(const SgTypedefType& n)
      {
        sg::NotNull<const SgType> baseTy = n.get_base_type();

        res = mangleType(*baseTy, ctx.inclReturnType(Ctx::WITH_RETURN_TYPE));
      }

      void handle(const SgTypeDefault& n)
      {
        res = join( mglDefaultType
                  , n.get_name()
                  , mglDefaultTypeDelim
                  );
      }

      void handle(const SgPointerType& n)
      {
        res = mangleIndirection(mglPointerType, mglPointerTypeDelim, n.get_base_type());
      }

      void handle(const SgReferenceType& n)       { res = mangleIndirection(mglReferenceType, mglReferenceTypeDelim, n.get_base_type()); }
      void handle(const SgRvalueReferenceType& n) { res = mangleIndirection(mglRvalueReferenceType, mglRvalueReferenceTypeDelim, n.get_base_type()); }

      void handle(const SgPointerMemberType& n)
      {
        res = replaceFirst( mangleBase(n), mglPointerType,
                            join(mglPointerMemberType, mangleType(n.get_class_type(), ctx.resetTypeFlags()))
                          );
      }

      void handle(const SgArrayType& n);

      void handle(const SgFunctionType& n)
      {
        sg::NotNull<const SgFunctionParameterTypeList> funParameterTypeList = n.get_argument_list();
        const SgTypePtrList&                           parmTypeList = funParameterTypeList->get_arguments();

        res = join( mglFunctionType
                  , mangleReturnType(n.get_return_type()) // return types are not part of the C function name mangling
                  , mangleNodeSequence(parmTypeList, mglSimpleSep, Ctx::ARRAY_TO_PTR)
                  , mglFunctionTypeDelim
                  );
      }

      void handle(const SgMemberFunctionType& n)
      {
        std::string functionTypeName = mangleBase(n);

        res = join( mglMemberFunctionType
                  , mangleType(n.get_class_type(), ctx.resetTypeFlags())
                  , mangleMemberFunctionQualifiers(n)
                  , functionTypeName
                  );
      }

      void handle(const SgModifierType& n)
      {
        const SgTypeModifier& type_mod = n.get_typeModifier();

        // Ada?
        if (type_mod.isAliased()) res += mglModifierAliased;
        if (type_mod.isNotNull()) res += mglModifierNotNull;

        {
          const SgConstVolatileModifier& cv_mod = type_mod.get_constVolatileModifier();

          if (cv_mod.isConst())      res += mglModifierConst;
          if (cv_mod.isVolatile())   res += mglModifierVolatile;
          if (type_mod.isRestrict()) res += mglModifierRestrict;
        }

        {
          const SgUPC_AccessModifier& upc_mod = type_mod.get_upcModifier();

          if (upc_mod.get_isShared())  res += (mglModifierUpcShared + std::to_string(upc_mod.get_layout()));
          if (upc_mod.isUPC_Strict())  res += mglModifierUpcStrict;
          if (upc_mod.isUPC_Relaxed()) res += mglModifierUpcRelaxed;
        }

        res += mangleType(n.get_base_type(), ctx.inclReturnType(Ctx::WITH_RETURN_TYPE));
      }

      void handle(const SgNamedType& n) { res = mangleDeclaration(n.get_declaration()); }

      //
      // others

      void handle(const SgTemplateArgument& n)
      {
        switch (n.get_argumentType())
        {
          case SgTemplateArgument::type_argument:
            res = mangleType(n.get_type(), ctx.resetTypeFlags());
            break;

          case SgTemplateArgument::nontype_argument:
            if (const SgExpression* expr = n.get_expression())
              res = mangleExpression(expr);
            else
              res = SG_DEREF(n.get_initializedName()).get_name();
            break;

          case SgTemplateArgument::template_template_argument:
            res = mangleDeclaration(n.get_templateDeclaration());
            break;

         // DQ (5/19/2014): Added support for varadic template argument.
          case SgTemplateArgument::start_of_pack_expansion_argument:
            res = mglTemplateArgumentPack;
            break;

          default:
            ROSE_ABORT();
        }
      }


      //

      void handle(const SgInitializedName& n)
      {
        res = join( scopeQualifiedName(n)
                  , mangleInternalLinkageIfStaticVariable(isSgVariableDeclaration(n.get_parent()))
                  );
      }

      //
      // scopes

      void handle(const SgScopeStatement& n)
      {
        res = todo("scp:", n);
      }

      void handle(const SgGlobal&)                      {}
      void handle(const SgBasicBlock& n)                { res = mangleEnclosingScope(n); }

      // scopes with declarations
      void handle(const SgClassDefinition& n)           { res = mangleParentNode(n); }
      void handle(const SgFunctionDefinition& n)        { res = mangleParentNode(n); }
      void handle(const SgNamespaceDefinitionStatement& n) { res = mangleParentNode(n); }


      //
      // declarations

      void handle(const SgDeclarationStatement& n)
      {
        res = todo("dcl:", n);
      }


      void handle(const SgEnumDeclaration& n);
      void handle(const SgClassDeclaration& n);

      void handle(const SgFunctionParameterList& n) { res = mangleParentNode(n); }

      void handle(const SgTemplateInstantiationDecl& n)
      {
        res = join( mangleScope(n.get_scope())
                  , simpleName(n.get_templateName(), "tcl")
                  , mglTemplateArgs
                  , mangleNodeSequence(n.get_templateArguments(), mglExtendedSep)
                  , mglTemplateArgsDelim
                  );
      }


      void handle(const SgNamespaceDeclarationStatement& n)
      {
        res = join( scopeQualifiedName(n)
                  , mangleInternalLinkageIfAnonymous(n, n.get_name())
                  );
      }

      void handle(const SgFunctionDeclaration& n)
      {
        res = scopeQualifiedName(n, mangleInternalLinkageIfStatic(n));
      }

      void handle(const SgMemberFunctionDeclaration& n)
      {
        res = scopeQualifiedName(n);
      }

      void handle(const SgTemplateInstantiationFunctionDecl& n)
      {
        res = join( mangleScope(n.get_scope())
                  , simpleName(functionName(n, n.get_templateName()), "tfn")
                  , mglTemplateArgs
                  , mangleNodeSequence(n.get_templateArguments(), mglExtendedSep)
                  , mglTemplateArgsDelim
                  , mangleType(n.get_type(), ctx.inclReturnType(Ctx::NO_RETURN_TYPE).arrayDecay())
                  , mangleInternalLinkageIfStatic(n)
                  );
      }

      void handle(const SgTemplateInstantiationMemberFunctionDecl& n)
      {
        res = join( mangleScope(n.get_scope())
                  , simpleName(functionName(n, n.get_templateName()), "tmf")
                  , mglTemplateArgs
                  , mangleNodeSequence(n.get_templateArguments(), mglExtendedSep)
                  , mglTemplateArgsDelim
                  , mangleType(n.get_type(), ctx.inclReturnType(Ctx::NO_RETURN_TYPE).arrayDecay())
                  );
      }

      void handle(const SgTypedefDeclaration& n)
      {
        res = join( mangleScope(n.get_scope())
                  , mglTypedefDeclaration
                  , simpleName(n.get_name(), "tdf")
                  , mglTypedefDeclarationDelim
                  );
      }

      void handle(const SgTemplateInstantiationTypedefDeclaration& n)
      {
        res = join( mangleScope(n.get_scope())
                  , mglTypedefDeclaration
                  , simpleName(n.get_templateName(), "tid")
                  , mglTemplateArgs
                  , mangleNodeSequence(n.get_templateArguments(), mglExtendedSep)
                  , mglTemplateArgsDelim
                  , mglTypedefDeclarationDelim
                  );
      }

    private:
      ExternalManglingContext ctx;
  };

  std::string ExternalMangler::simpleName(std::string name, const char* dbg)
  {
    if (name.size() == 0)
      msgError() << "sn " << dbg << ": '" << name << "'" << std::endl;

    return name;
  }
/*
  std::string ExternalMangler::simpleName(const char* prefix, std::string name, const char* dbg)
  {
    std::string mangled;

    mangled.append(prefix);
    mangled.append(std::to_string(name.size()));
    mangled.append(name);

    return mangled;
  }
*/

  bool isMainFunction(const SgFunctionDeclaration& n)
  {
    return (n.get_name() == "main") && isSgGlobal(n.get_scope());
  }

  std::string fortranFunctionName(const SgFunctionDeclaration& n, std::string name)
  {
    if (n.get_declarationModifier().isBind())
    {
      std::string binding = n.get_binding_label();

      if (!binding.empty())
        return binding;

      // \todo
      // shall we generate name according to linkage?
    }

    boost::to_lower(name);

    // according to documentation, gfortran appends two underlines
    //   if the name already contains one.
    //   https://en.wikipedia.org/wiki/Name_mangling#Fortran
    //   https://stackoverflow.com/questions/7175405/compiler-agnostic-fortran-name-mangling-function
    //
    // However tests with gcc 8 (and 12) and simple subroutines do not show that behavior
    //   so the extra underline is turned off.
    bool const withExtraUnderline = false;
    bool const extraUnderline     = name.find("_") != std::string::npos;
    if (extraUnderline & withExtraUnderline) name += '_';

    name += '_';
    return name;
  }

  std::string ExternalMangler::functionName(const SgFunctionDeclaration& n)
  {
    return functionName(n, n.get_name());
  }

  /// tests if \p name designates a proper C++ operator or if \p name is a mere
  ///   function name starting with the character sequence operator.
  bool isProperCxxOperator(std::string name)
  {
    static const std::string operkey = "operator";

    // First, check if the name starts with "operator"
    if (!boost::starts_with(name, operkey))
      return false;

    // Check the character following "operator"
    int ch = name[operkey.size()];

    // Check if the character following "operator" is a valid
    //   character in a C++ identifier.
    // \todo does not consider C++ unicode characters
    return (!isalnum(ch) && ch != '_');
  }


  std::string ExternalMangler::functionName(const SgFunctionDeclaration& n, std::string name)
  {
    if (si::is_Fortran_language())
      return fortranFunctionName(n, std::move(name));

    const SgSpecialFunctionModifier& special = n.get_specialFunctionModifier();

    // Special case: destructor names
    if (special.isDestructor())
      return mglMemFuncDtor;

    const bool  isOperator = special.isOperator() || isProperCxxOperator(name);

    if (isOperator)
    {
      std::string mangled;

      // note, testing the ending of a token
      //   requires that longer operators with the same ending characters
      //   need to be tested before operators with a subset of ending tokens.
      //   e.g., test new[] before [], ++ before +, etc.
      if (boost::ends_with(name, "="))
      {
        if (boost::ends_with(name, "<<="))
          mangled = mglFuncOperatorLShiftAssign;
        else if (boost::ends_with(name, ">>="))
          mangled = mglFuncOperatorRShiftAssign;
        else if (boost::ends_with(name, "=="))
          mangled = mglFuncOperatorEquals;
        else if (boost::ends_with(name, "!="))
          mangled = mglFuncOperatorNotEquals;
        else if (boost::ends_with(name, "+="))
          mangled = mglFuncOperatorPlusAssign;
        else if (boost::ends_with(name, "-="))
          mangled = mglFuncOperatorMinusAssign;
        else if (boost::ends_with(name, "*="))
          mangled = mglFuncOperatorMulAssign;
        else if (boost::ends_with(name, "/="))
          mangled = mglFuncOperatorDivAssign;
        else if (boost::ends_with(name, "%="))
          mangled = mglFuncOperatorModAssign;
        else if (boost::ends_with(name, "|="))
          mangled = mglFuncOperatorOrAssign;
        else if (boost::ends_with(name, "&="))
          mangled = mglFuncOperatorAndAssign;
        else if (boost::ends_with(name, "^="))
          mangled = mglFuncOperatorXorAssign;
        else if (boost::ends_with(name, "<="))
          mangled = mglFuncOperatorLessThan;
        else if (boost::ends_with(name, ">="))
          mangled = mglFuncOperatorGreaterThan;
        else // if (boost::ends_with(name, "="))
          mangled = mglFuncOperatorAssign;
      }
      else if (boost::ends_with(name, "++"))
        mangled = mglFuncOperatorInc;
      else if (boost::ends_with(name, "+"))
        mangled = mglFuncOperatorPlus;
      else if (boost::ends_with(name, "--"))
        mangled = mglFuncOperatorDec;
      else if (boost::ends_with(name, "-"))
        mangled = mglFuncOperatorMinus;
      else if (boost::ends_with(name, "->*"))
        mangled = mglFuncOperatorArrowStar;
      else if (boost::ends_with(name, "*"))
        mangled = mglFuncOperatorMul;
      else if (boost::ends_with(name, "/"))
        mangled = mglFuncOperatorDiv;
      else if (boost::ends_with(name, "()"))
        mangled = mglFuncOperatorApply;
      else if (boost::ends_with(name, "<<"))
        mangled = mglFuncOperatorLShift;
      else if (boost::ends_with(name, "<"))
        mangled = mglFuncOperatorLess;
      else if (boost::ends_with(name, "->"))
        mangled = mglFuncOperatorArrow;
      else if (boost::ends_with(name, ">>"))
        mangled = mglFuncOperatorRShift;
      else if (boost::ends_with(name, ">"))
        mangled = mglFuncOperatorGreater;
      else if (boost::ends_with(name, "%"))
        mangled = mglFuncOperatorMod;
      else if (boost::ends_with(name, "!"))
        mangled = mglFuncOperatorNot;
      else if (boost::ends_with(name, "^"))
        mangled = mglFuncOperatorXor;
      else if (boost::ends_with(name, "~"))
        mangled = mglFuncOperatorComplement;
      else if (boost::ends_with(name, "new"))
        mangled = mglFuncOperatorNew;
      else if (boost::ends_with(name, "new[]"))
        mangled = mglFuncOperatorNewArray;
      else if (boost::ends_with(name, "delete"))
        mangled = mglFuncOperatorDel;
      else if (boost::ends_with(name, "delete[]"))
        mangled = mglFuncOperatorDelArray;
      else if (boost::ends_with(name, "[]"))
        mangled = mglFuncOperatorSubscript;
      else if (boost::ends_with(name, "&&"))
        mangled = mglFuncOperatorLogicalAnd;
      else if (boost::ends_with(name, "&"))
        mangled = mglFuncOperatorAnd;
      else if (boost::ends_with(name, "||"))
        mangled = mglFuncOperatorLogicalOr;
      else if (boost::ends_with(name, "|"))
        mangled = mglFuncOperatorOr;
      else if (boost::ends_with(name, ","))
        mangled = mglFuncOperatorComma;
      else if (boost::starts_with(name, "operator \"\" "))
        mangled = join( mglFuncOperatorLiteral
                      , name.substr(std::string{"operator \"\" "}.size())
                      );
      else if (special.isConversion())
        mangled = join( mglFuncOperatorConv
                      , mangleType(SG_DEREF(n.get_type()).get_return_type(), ctx.resetTypeFlags())
                      , mglFuncOperatorConvDelim
                      );

      if (mangled.size() == 0)
      {
        msgError() << "unable to mangle operator: " << name
                   << std::endl;

        mangled = name;
      }

      return mangled;
    }


    if (ctx.makeMainUnique && isMainFunction(n))
    {
      return join( name
                 , mglSimpleSep
                 , mangleInternalLinkage(n)
                 );
    }

    return name;
  }


  struct GetType
  {
    template <class SageNode>
    const SgType* operator()(sg::NotNull<const SageNode> n)
    {
      return n->get_type();
    }
  };

  struct MangleNode
  {
    std::string operator()(const SgNode* n) const
    {
      return externalName(n, ctx);
    }

    ExternalManglingContext ctx;
  };

  struct ConcatStrings
  {
    std::string operator()(std::string lhs, const std::string& rhs)
    {
      lhs.reserve(lhs.size() + rhs.size() + 1);
      lhs += sep;
      lhs += rhs;

      sep = listsep;
      return lhs;
    }

    const char* listsep;
    const char* sep = "";
  };

  template <class Sequence>
  std::string
  ExternalMangler::mangleNodeSequence(Sequence&& seq, const char* sep, bool arrayToPointer)
  {
    return boost::accumulate( seq | adapt::transformed(MangleNode{ctx.resetTypeFlags().arrayDecay(arrayToPointer)}),
                              std::string{},
                              ConcatStrings{sep}
                            );
  }

  template <class ForwardIterator>
  std::string
  ExternalMangler::mangleTypeRange(ForwardIterator beg, ForwardIterator lim)
  {
    return mangleNodeSequence(boost::make_iterator_range(beg, lim));
  }

/*
  std::string ExternalMangler::mangleParameterlist(const SgFunctionParameterList& n)
  {
    // auto typeList = n.get_args() | adapt::transformed(GetType{});

    return ""; // boost::accumulate(typeList, std::string{}, ConcatStrings{});
  }
*/
  // ExternalManglingContext{ctx.makeMainUnique, withReturnType, pointerToArray}

  std::string
  ExternalMangler::mangleType(const SgType& n, ExternalManglingContext ctx)
  {
    return externalName(n, std::move(ctx));
  }

  std::string
  ExternalMangler::mangleReturnType(const SgType& n)
  {
    if (!ctx.withReturnType)
      return {};

    return mangleType(n, ctx.resetTypeFlags());
  }

  std::string
  ExternalMangler::mangleDeclaration(const SgDeclarationStatement& n)
  {
    return externalName(n, ExternalManglingContext{ctx.makeMainUnique});
  }

  std::string
  mangleLocation(rsa::SourceLocation loc, const char* prefix)
  {
    return join( prefix
               , rsa::mangleFilename(loc.file())
               , mglSimpleSep
               , std::to_string(loc.startLine())
               , mglSimpleSep
               , std::to_string(loc.startCol())
               );
  }

  std::string
  ExternalMangler::mangleInternalLinkage(const SgLocatedNode& n)
  {
#if WITH_CUSTOM_LINKAGE_HANDLER
    return ctx.internalLinkageGen();
#else
    return internalLinkageFromFile(n);
#endif /*WITH_CUSTOM_LINKAGE_HANDLER*/
  }

  std::string
  ExternalMangler::mangleInternalLinkageIfStatic(const SgDeclarationStatement& n)
  {
    if (si::isStatic(const_cast<SgDeclarationStatement*>(&n)))
      return mangleInternalLinkage(n);

    return {};
  }

  std::string
  ExternalMangler::mangleInternalLinkageIfAnonymous(const SgDeclarationStatement& n, const std::string& name)
  {
    if (name.size() == 0)
      return mangleInternalLinkage(n);

    return {};
  }

  std::string
  ExternalMangler::mangleInitializedName(const SgInitializedName& n)
  {
    return externalName(n, ExternalManglingContext{ctx.makeMainUnique});
  }

  std::string
  ExternalMangler::mangleScope(const SgScopeStatement& n)
  {
    return externalName(n, ExternalManglingContext{ctx.makeMainUnique});
  }

  std::string
  ExternalMangler::mangleEnclosingScope(const SgScopeStatement& n)
  {
    return externalName(enclosingScope(n), ExternalManglingContext{ctx.makeMainUnique});
  }

  std::string
  ExternalMangler::mangleParentNode(const SgNode& n)
  {
    return externalName(n.get_parent(), ExternalManglingContext{ctx.makeMainUnique});
  }


  std::string
  ExternalMangler::mangleArrayIndexExpressionOpt(bool vla, const SgExpression* idx)
  {
    if (idx == nullptr)
      return {};

    if (const SgExpression* alt = idx->get_alternativeExpr())
      return mangleArrayIndexExpressionOpt(vla, alt);

    if (/*const SgNullExpression* valExp =*/ isSgNullExpression(idx))
      return {};

    if (const SgValueExp* valExp = isSgValueExp(idx))
      return valExp->get_constant_folded_value_as_string();

    if (const SgCastExp* castExp = isSgCastExp(idx))
      return mangleArrayIndexExpressionOpt(vla, castExp->get_operand());

    if (vla)
      return mglArrayVL;
/*
 *  \todo consider using SageInterface constant folding as fallback.
    // fallback code for when constants are not preserved by the frontend
    CodeThorn::AbstractValue aVal = CodeThorn::evaluateExpressionWithEmptyState(const_cast<SgExpression*>(idx));
    if (aVal.isConstInt())
      return std::to_string(aVal.getIntValue());
*/

    roseError() << "UNABLE to mangle array index of type: " << idx->class_name()
                << "  was ROSE's frontend called with SgProject::e_original_expressions_and_folded_values"
                << "      or SgProject::e_folded_values_only?"
                << "\n  using string for mangling: " << idx->unparseToString()
                << std::endl;

    return idx->unparseToString();
    // SG_UNEXPECTED_NODE(*idx);
  }

  std::string
  ExternalMangler::mangleMemberFunctionQualifiers(const SgMemberFunctionType& n)
  {
    std::stringstream mangled;

    if (n.isConstFunc())           mangled << mglMemFuncConst;
    if (n.isVolatileFunc())        mangled << mglMemFuncVolatile;
    if (n.isRestrictFunc())        mangled << mglMemFuncRestrict;
    if (n.isLvalueReferenceFunc()) mangled << mglMemFuncLvalueRef;
    if (n.isRvalueReferenceFunc()) mangled << mglMemFuncRvalueRef;

    return mangled.str();
  }


  std::string
  ExternalMangler::mangleIndirection(const char* prefix, const char* suffix, const SgType* baseTy)
  {
    std::string tyName = mangleType(baseTy, ctx.resetTypeFlags());

    if (tyName.empty()) tyName = mglAnonymousTypeName;

    return join(prefix, tyName, suffix);
  }

  void
  ExternalMangler::handle(const SgArrayType& n)
  {
    if (ctx.arrayAsPointerType)
    {
      res = mangleIndirection(mglPointerType, mglPointerTypeDelim, n.get_base_type());
      return;
    }

    res = join( mglArrayType
              , mangleType(n.get_base_type(), ctx.resetTypeFlags())
              , mglArrayIndex
              , mangleArrayIndexExpressionOpt(n.get_is_variable_length_array(), n.get_index())
              , mglArrayTypeDelim
              );
  }

  //
  // alternative hash_combine: https://stackoverflow.com/a/50978188

  inline
  std::uint64_t rotl(std::uint64_t x, int s) noexcept
  {
    // consider using std::rotl (C++20) or boost::core::rotl (newer boost versions).
    constexpr int N = std::numeric_limits<std::uint64_t>::digits;

    const int r = s & (N - 1);

    if (r == 0) return x;  // x >> N is UB before C++20

    return (x << r) | (x >> (N - r));
  }

  inline
  std::uint64_t xor_shift(std::uint64_t n, int i) { return n ^ (n >> i); }

  // a hash function with another name as to not confuse with std::hash
  inline
  std::uint64_t stable_hash_distribute(std::uint64_t n) {
    constexpr std::uint64_t p = 0x5555555555555555ull;    // pattern of alternating 0 and 1
    constexpr std::uint64_t c = 17316035218449499591ull;  // random uneven integer constant;

    return c * xor_shift(p * xor_shift(n, 32), 32);
  }

  inline
  std::uint64_t stable_hash_combine(std::uint64_t seed, std::uint64_t comp) {
    const std::uint64_t distr = stable_hash_distribute(comp);

    //~ return std::rotl(seed, std::numeric_limits<std::uint64_t>::digits/3) ^ distr;
    return rotl(seed, std::numeric_limits<std::uint64_t>::digits/3) ^ distr;
  }


  /// Combines two hashes \p lhs and \p rhs
  /// \param  lhs one hash value
  /// \param  rhs another hash value
  /// \result the combined hash value
  /// \details
  ///    uses boost::hash_combine, though other [better?] ways
  ///    exist: https://stackoverflow.com/a/50978188
  std::uint64_t
  hashCombine(std::uint64_t lhs, std::uint64_t rhs)
  {
    //~ boost::hash_combine(lhs, rhs);
    //~ return lhs;
    return stable_hash_combine(lhs, rhs);
  }

  /// Computes a hash value based on the elements of a declaration's \p n elements.
  /// \param  n the declaration
  /// \result a hash value for \p n over \p n's elements.
  /// \details
  ///    Elements are defined as components of a declaration.
  ///    e.g., enumerators are the elements of an enum.
  /// \note
  ///    The hash computed for a declaration is a stand-in for the missing name
  ///    in an anonymous declaration.
  ///    The computed hash should be stable against some source code modification,
  ///    hence, the file location is not considered suitable. Instead, we compute a hash over
  ///    elements. The computed hash does not include all elements (e.g., initializers)
  ///    to make the hash survive some modification the declaration's body, similar to
  ///    how a class body can be modified without changing its name.
  /// \{
  std::uint64_t
  hashContent(const SgEnumDeclaration& n)
  {
    const SgEnumDeclaration& dcl = definingDeclarationIfAvailable(n);
    std::hash<std::string>   hasher;
    const SgType* const      baseType = dcl.get_field_type();
    std::uint64_t const      initHash = baseType ? hasher(externalName(*baseType)) : 0;

    auto elemHasher =
        [](std::uint64_t acc, sg::NotNull<const SgInitializedName> enumerator) -> std::uint64_t
        {
          std::hash<std::string> hashGen;

          return hashCombine(acc, hashGen(externalName(*enumerator)));
        };

    return boost::accumulate(dcl.get_enumerators(), initHash, elemHasher);
  }

  std::uint64_t
  hashContent(const SgClassDeclaration& n)
  {
    const SgClassDeclaration&            dcl = definingDeclarationIfAvailable(n);
    sg::NotNull<const SgClassDefinition> clsdef = dcl.get_definition();

    auto elemHasher =
        [](std::uint64_t acc, sg::NotNull<const SgDeclarationStatement> member) -> std::uint64_t
        {
          std::hash<std::string> hashGen;

          return hashCombine(acc, hashGen(externalName(*member)));
        };

    // PP: More elements could be included...
    return boost::accumulate(clsdef->get_members(), 0, elemHasher);
  }
  /// \}


  template <class SageDeclarationStatement>
  std::string handleAnonymousName(const SageDeclarationStatement& n, std::string name)
  {
    if (name.rfind("__anonymous_0x", 0) == std::string::npos)
      return name;

    // mangle anonymous enums or classes as their typedef name.
    if (SgTypedefDeclaration* tdef = isSgTypedefDeclaration(n.get_parent()))
      name = externalName(*tdef);
    else
    {
      msgWarn() << "mangling anonymous declaration " << n.class_name()
                << "\n  parent: " << (n.get_parent() ? n.get_parent()->class_name() : std::string{"null"})
                << std::endl;
      // \todo similarly we could pull the name from variable (and parameter) declarations...
      name = Rose::Combinatorics::toBase62String(hashContent(n));
    }

    return join(mglAnonymousTypeName, name);
  }

  void ExternalMangler::handle(const SgEnumDeclaration& n)
  {
    std::string                         enumName = handleAnonymousName(n, n.get_name());
    sg::NotNull<const SgScopeStatement> scope = n.get_scope();
    std::string                         qualifiers = isSgFunctionDefinition(scope)
                                                         ?  std::string{mglElideFunctionScope}
                                                         :  mangleScope(scope);

    res = join( qualifiers
              , mglEnum
              , simpleName(enumName, "enm")
              );
  }


  void ExternalMangler::handle(const SgClassDeclaration& n)
  {
    std::string className = handleAnonymousName(n, n.get_name());

    // \todo how about class/unions in parameter lists? Handle like enums?

    res = join( mangleScope(n.get_scope())
              , mglClass
              , simpleName(className, "cls")
              );
  }


  std::string
  ExternalMangler::scopeQualifiedName(const SgNamespaceDeclarationStatement& n)
  {
    std::string             name  = n.get_name();
    const SgScopeStatement* scope = n.get_scope();

    if (name.size() == 0)
      name = mangleInternalLinkageIfAnonymous(n, name);

    return join( mangleScope(scope)
               , simpleName(name, "nsp")
               );
  }


  const SgScopeStatement*
  scopeForFunctionMangling(const SgFunctionDeclaration& n)
  {
    bool const useCMangling = si::is_C_language() || n.get_linkage() == "C" || isMainFunction(n);
    bool const useFortranMangling = !useCMangling && si::is_Fortran_language();

    if (useCMangling || useFortranMangling)
      return nullptr;

    return n.get_scope();
  }

  std::string
  ExternalMangler::scopeQualifiedName(const SgFunctionDeclaration& n, std::string linkage)
  {
    const SgScopeStatement* scope = scopeForFunctionMangling(n);

    if (!scope)
      return simpleName(functionName(n), "fn2");

    return join( joinNames(mangleScope(n.get_scope()), linkage, simpleName(functionName(n), "fun"))
               , mangleType(n.get_type(), ctx.inclReturnType(Ctx::NO_RETURN_TYPE))
               );
  }

  std::string
  ExternalMangler::scopeQualifiedName(const SgInitializedName& n)
  {
    const SgScopeStatement*   scope     = enclosingScope(n);
    const bool                skipScope = isSgGlobal(scope);
    std::string               name      = n.get_name();
    sg::NotNull<const SgNode> parent    = n.get_parent();

    // \todo
    //   this needs to be extended to identify variables that need
    //   to be mangled using source location information, otherwise the
    //   generated names would not be unique.
    //   ex:
    // \code
    //   // ex 1:
    //   void foo()
    //   {
    //      { int i = 0; }
    //      { int i = 0; } // different i
    //   }
    // \endcode

    if (const SgFunctionParameterList* parmlst = isSgFunctionParameterList(parent))
    {
      // function parameters are modeled using indices, so that parameter names
      //   can be uniquely identified although they may be renamed in the definition.
      //   ex:
      // \code
      //   void foo(int i);
      //   void foo(int j); // i and j designate the same variable
      // \endcode
      const SgInitializedNamePtrList& parms = parmlst->get_args();
      auto const                      beg   = parms.begin();
      std::size_t                     num   = std::distance(beg, std::find(beg, parms.end(), &n));
      sg::NotNull<const SgNode>       root  = parmlst->get_parent();

      name = join(mglParameterIdx, std::to_string(num));
    }
    else if (name.size() == 0)
    {
      rsa::CppCompatibility compat;

      name = mangleLocation(compat.location(compat.variableId(&n), true/*resolve instantiations*/), mglVariableLoc);

      msgWarn() << "mangling an unnamed parameter in: " << parent->unparseToString()
                << " as '" << name << "'"
                << std::endl;
    }

    if (skipScope)
      return simpleName(name, "in2");

    return join( mangleScope(scope)
               , simpleName(name, "ini")
               );
  }

  using MangledNameHashKey = std::tuple<const SgNode*, ExternalManglingContext>;

  struct MangledNameHashKeyFn
  {
    std::size_t operator()(const MangledNameHashKey& key) const
    {
      std::size_t hashval = std::hash<const void*>{}(std::get<0>(key));

#if WITH_CUSTOM_LINKAGE_HANDLER
      hashval = hashCombine(hashval, std::hash<std::size_t>{}(addressOf(std::get<1>(key).internalLinkageGen)));
#endif /*WITH_CUSTOM_LINKAGE_HANDLER*/

      return ( hashval
             ^ (std::hash<bool>{}(std::get<1>(key).makeMainUnique) << 2)
             ^ (std::hash<bool>{}(std::get<1>(key).withReturnType) << 1)
             ^ (std::hash<bool>{}(std::get<1>(key).arrayAsPointerType) << 0)
             );
    }
  };

  using MangledNameHash = std::unordered_map<MangledNameHashKey, std::string, MangledNameHashKeyFn>;

  std::unordered_map<std::string, uint64_t> shortMangledNameCache;
  std::unordered_map<uint64_t, std::string> mangledNameHashCollisionCheckMap;
  MangledNameHash                           alreadyMangledShort;
  MangledNameHash                           alreadyMangledLong;

  std::string externalName(const SgNode& n, ExternalManglingContext ctx)
  {
    std::string& cached = alreadyMangledLong[{&n, ctx}];

    if (cached.size() == 0)
      cached = sg::dispatch(ExternalMangler{std::move(ctx)}, &n);

    return cached;
  }
}

namespace Rose::Source::Analysis
{
std::string
mangleFilename(boost::string_view filename)
  {
    std::string res;

    res.reserve(filename.size());

    auto replCh = [](char c) -> char
                  {
                    if ((c == '/') | (c == '\\') | (c == '.'))
                      c = '_';

                    return c;
                  };

    std::transform(filename.begin(), filename.end(), std::back_inserter(res), replCh);
    return res;
}


NameShortener longNames()
{
  return [](std::string s) -> std::string { return s; };
}

NameShortener hashNames()
{
  constexpr bool withCollisionDetection = true;

  return [](std::string oldMangledName) -> std::string
         {
           if (oldMangledName.size() <= 40)
             return oldMangledName;

           std::uint64_t idNumber = 0;
           auto          shortMNIter = shortMangledNameCache.find(oldMangledName);

           if (shortMNIter != shortMangledNameCache.end())
           {
             idNumber = shortMNIter->second;
           }
           else
           {
             Rose::Combinatorics::HasherSha256Builtin hasher;

             hasher.insert(oldMangledName);
             hasher.digest();
             idNumber = hasher.toU64();

             if (withCollisionDetection)
             {
               auto res = mangledNameHashCollisionCheckMap.insert({idNumber, oldMangledName});

               if ((!res.second) && (res.first->second != oldMangledName))
               {
                 std::cerr << "hash collision." << std::endl;
                 ROSE_ABORT();
               }
             }

             shortMangledNameCache.insert({oldMangledName, idNumber});
           }

           std::ostringstream mn;
           mn << 'L' << Rose::Combinatorics::toBase62String(idNumber) << 'R';
           return mn.str();
         };
}


//~ NameShortener compressNames(int windowsize)
//~ {
  //~ return [w=windowsize](std::string s) -> std::string { return compress(std::move(s), w); };
//~ }

const std::string& _mangle( const SgNode& n,
                            NameShortener shorten,
                            ExternalManglingContext ctx = {}
                          )
{
  std::string& cached = alreadyMangledShort[{&n, ctx}];

  if (cached.size() == 0)
  {
    cached = shorten(externalName(n, ctx));
/*
    if (cached.size() == 0)
    {
      std::cerr << n.get_parent()->get_parent()->unparseToString() << "/" << typeid(*n.get_parent()->get_parent()).name()
                << "\n  " << externalName(n, withUniqueMain, withReturnType, withPointerToArrayDecay)
                << "\n  " << cached
                << std::endl;
    }
*/
    ASSERT_require(cached.size() != 0);
  }

  return cached;
}

const std::string& mangle(const SgDeclarationStatement& n, NameShortener shorten, bool withUniqueMain)
{
  const bool unableToMangle = (  isSgFunctionParameterList(&n)
                              || isSgVariableDeclaration(&n)
                              || isSgStaticAssertionDeclaration(&n)
                              || isSgCtorInitializerList(&n)
                              );

  if (unableToMangle)
    throw std::runtime_error{std::string{"unable to mangle type: "} + n.class_name()};

  return _mangle(n, std::move(shorten), ExternalManglingContext{withUniqueMain});
}

const std::string& mangle(const SgDeclarationStatement* n, NameShortener shorten, bool withUniqueMain)
{
  ASSERT_not_null(n);

  return mangle(*n, std::move(shorten), withUniqueMain);
}

const std::string& mangle(const SgType& n, NameShortener shorten, bool withReturnType, bool withArrayToPointerDecay)
{
  return _mangle( n,
                  std::move(shorten),
                  ExternalManglingContext{false/*no unique main*/, withReturnType, withArrayToPointerDecay}
                );
}

const std::string& mangle(const SgType* n, NameShortener shorten, bool withReturnType, bool withArrayToPointerDecay)
{
  ASSERT_not_null(n);

  return mangle(*n, std::move(shorten), withReturnType, withArrayToPointerDecay);
}

const std::string& mangle(const SgInitializedName& n, NameShortener shorten)
{
  return _mangle(n, std::move(shorten));
}

const std::string& mangle(const SgInitializedName* n, NameShortener shorten)
{
  ASSERT_not_null(n);

  return mangle(*n, std::move(shorten));
}


void printUnhandledNodes(std::ostream& os)
{
  if (unhandledSageNodeTypes.size() == 0)
    return;

  os << "* Unhandled sage-nodes:\n";
  for (const std::string& s : unhandledSageNodeTypes)
    os << "    " << s << std::endl;
}

void clearManglingCache()
{
  shortMangledNameCache.clear();
  mangledNameHashCollisionCheckMap.clear();
  alreadyMangledShort.clear();
  alreadyMangledLong.clear();
}

}
