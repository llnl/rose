#include <sage3basic.h>
#include <Rose/Source/Analysis/ItaniumMangling.h>

#include <map>
#include <algorithm>
#include <iterator>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <cctype>

#include <sageInterfaceAda.h>
#include <sageGeneric.h>

#include <Rose/Source/Analysis/ExternalMangling.h>

// =============================================================================
// CONST-CORRECTNESS STRATEGY
// =============================================================================
//
// SubstitutionTable: Stores const pointers (const SgType*, const SgScopeStatement*)
// - The table never modifies the pointed-to objects
// - No const_cast needed when adding or finding entries
//
// =============================================================================

// =============================================================================
// REVIEW TODOs
// =============================================================================
//
// Comments tagged TODO(itanium-review) mark known deviations from the Itanium C++ ABI and
// other known flaws that remain unresolved.  DONE(itanium-review) records reviewed items
// whose implementation now addresses the issue.  Each item names
//   ABI:  the relevant section of the specification, as "<title> (#anchor)" under
//         https://itanium-cxx-abi.github.io/cxx-abi/abi.html   (chapter 5.1, Mangling)
//   Test: the regression fixture that exercises it (tests repo,
//         tests/nonsmoke/functional/roseTests/Source/Analysis/ItaniumMangling_<name>.cc,
//         golden file ItaniumManglingReference/gcc/ItaniumMangling_<name>.symbols),
//         or states that no test exists yet.
// =============================================================================

namespace
{

// =============================================================================
// Itanium C++ ABI Mangling Constants
// =============================================================================

// Core mangling prefixes and delimiters
constexpr const char* const ABI_PREFIX              = "_Z";
constexpr const char* const ABI_NESTED_BEGIN        = "N";
constexpr const char* const ABI_NESTED_END          = "E";
constexpr const char* const ABI_CONST_QUALIFIER     = "K";
constexpr const char* const ABI_VOLATILE_QUALIFIER  = "V";
constexpr const char* const ABI_RESTRICT_QUALIFIER  = "r";

// Type qualifiers
constexpr const char* const ABI_POINTER             = "P";
constexpr const char* const ABI_REFERENCE           = "R";
constexpr const char* const ABI_RVALUE_REFERENCE    = "O";
constexpr const char* const ABI_ARRAY               = "A";
constexpr const char* const ABI_FUNCTION            = "F";
constexpr const char* const ABI_MEMBER_POINTER      = "M";

// Builtin type encodings
constexpr const char* const ABI_TYPE_VOID           = "v";
constexpr const char* const ABI_TYPE_BOOL           = "b";
constexpr const char* const ABI_TYPE_CHAR           = "c";
constexpr const char* const ABI_TYPE_SIGNED_CHAR    = "a";
constexpr const char* const ABI_TYPE_UNSIGNED_CHAR  = "h";
constexpr const char* const ABI_TYPE_WCHAR          = "w";
constexpr const char* const ABI_TYPE_CHAR16         = "Ds";
constexpr const char* const ABI_TYPE_CHAR32         = "Di";
constexpr const char* const ABI_TYPE_SHORT          = "s";
constexpr const char* const ABI_TYPE_UNSIGNED_SHORT = "t";
constexpr const char* const ABI_TYPE_INT            = "i";
constexpr const char* const ABI_TYPE_UNSIGNED_INT   = "j";
constexpr const char* const ABI_TYPE_LONG           = "l";
constexpr const char* const ABI_TYPE_UNSIGNED_LONG  = "m";
constexpr const char* const ABI_TYPE_LONG_LONG      = "x";
constexpr const char* const ABI_TYPE_UNSIGNED_LONG_LONG = "y";
constexpr const char* const ABI_TYPE_FLOAT          = "f";
constexpr const char* const ABI_TYPE_DOUBLE         = "d";
constexpr const char* const ABI_TYPE_LONG_DOUBLE    = "e";
constexpr const char* const ABI_TYPE_ELLIPSIS       = "z";

// Constructor/Destructor encodings
constexpr const char* const ABI_CTOR_COMPLETE       = "C1";
constexpr const char* const ABI_DTOR_COMPLETE       = "D1";

// Special encodings
// Standard substitutions are recognized structurally from the AST below.  They
// are predefined ABI substitutions, not entries in ManglingContext's ordinary
// substitution table.
// ABI: Compression (#mangling-compression)
constexpr const char* const ABI_STD_NAMESPACE       = "St";
constexpr const char* const ABI_STD_ALLOCATOR       = "Sa";
constexpr const char* const ABI_STD_BASIC_STRING    = "Sb";
constexpr const char* const ABI_STD_STRING          = "Ss";
constexpr const char* const ABI_STD_ISTREAM         = "Si";
constexpr const char* const ABI_STD_OSTREAM         = "So";
constexpr const char* const ABI_STD_IOSTREAM        = "Sd";
constexpr const char* const ABI_ANONYMOUS_NAMESPACE = "12_GLOBAL__N_1";
constexpr const char* const ABI_LITERAL_OPERATOR    = "li";

// Local linkage encodings (internal linkage)
// Itanium ABI standard: 'L' prefix for entities with internal linkage
constexpr const char* const ABI_LOCAL_LINKAGE       = "L";

// ROSE Extension for local linkage (using Itanium vendor extension prefix)
// Format: _R<mangled_filename> to disambiguate static symbols across files
// This is a ROSE-specific extension to the Itanium ABI
constexpr const char* const ITANIUM_ROSE_EXT        = "_R";

// =============================================================================
// Cache for mangled names
// =============================================================================

struct CacheKey
{
    const void* ptr;
    int flags;

    bool operator<(const CacheKey& other) const
    {
        if (ptr != other.ptr) return ptr < other.ptr;
        return flags < other.flags;
    }
};

// TODO(itanium-review): this cache is a mutable global.  It is not thread-safe (unlocked map),
//   keyed on raw node addresses (stale when a node is deleted and its address reused, and never
//   invalidated when a transformation changes the AST), and unbounded.  Returned references
//   dangle after clearItaniumNameCache().  Options: per-project cache owned by the caller,
//   invalidation hook, or return std::string by value.  No test yet.
// PRIORITY: low
std::map<CacheKey, std::string> mangleCache;

template <class Mangler>
const std::string&
cachedItaniumName(const SgNode& n, int flags, Mangler mangler)
{
    CacheKey key{&n, flags};
    auto found = mangleCache.find(key);
    if (found != mangleCache.end())
        return found->second;

    return mangleCache.emplace(key, mangler()).first->second;
}

// =============================================================================
// Substitution table for name compression
// Per Itanium ABI: S_, S0_, S1_, etc. for previously seen entities
// Tracks both types and scopes (namespaces, classes) for back-references
// SgType and SgScopeStatement both inherit from SgNode, so we store the common base
// =============================================================================

struct SubstitutionEntry
{
    const SgNode* node;
    int templateParameterPosition;

    SubstitutionEntry(const SgNode* n) : node(n), templateParameterPosition(-1) {}
    SubstitutionEntry(const SgType* t) : node(t), templateParameterPosition(-1) {}
    SubstitutionEntry(const SgScopeStatement* s) : node(s), templateParameterPosition(-1) {}
    explicit SubstitutionEntry(int pos) : node(nullptr), templateParameterPosition(pos) {}

    bool matches(const SgType* t) const
    {
        return node == t;
    }

    bool matches(const SgScopeStatement* s) const
    {
        return node == s;
    }

    bool matchesTemplateParameter(int pos) const
    {
        return templateParameterPosition == pos;
    }
};

std::string encodeSequenceId(int value);

// A completed class template-id is a distinct ABI substitution candidate from
// its template prefix.  Node identity cannot represent that candidate: ROSE
// may use different declaration, scope, and canonical-type nodes for the
// same specialization.
struct CompletedClassTemplateKey
{
    const SgTemplateClassDeclaration* templateDeclaration;
    std::string argumentEncoding;

    bool operator<(const CompletedClassTemplateKey& other) const
    {
        if (templateDeclaration != other.templateDeclaration)
            return templateDeclaration < other.templateDeclaration;
        return argumentEncoding < other.argumentEncoding;
    }
};

struct SubstitutionTable
{
    std::vector<SubstitutionEntry> entries;
    std::map<CompletedClassTemplateKey, int> completedClassTemplateEntries;

    void clear()
    {
        entries.clear();
        completedClassTemplateEntries.clear();
    }

    int find(const SgType* type) const
    {
        auto found = std::find_if(entries.begin(), entries.end(),
                                  [type](const SubstitutionEntry& entry) { return entry.matches(type); });
        return found == entries.end() ? -1 : static_cast<int>(std::distance(entries.begin(), found));
    }

    int find(const SgScopeStatement* scope) const
    {
        auto found = std::find_if(entries.begin(), entries.end(),
                                  [scope](const SubstitutionEntry& entry) { return entry.matches(scope); });
        return found == entries.end() ? -1 : static_cast<int>(std::distance(entries.begin(), found));
    }

    int find(const SgNode* node) const
    {
        auto found = std::find_if(entries.begin(), entries.end(),
                                  [node](const SubstitutionEntry& entry) { return entry.node == node; });
        return found == entries.end() ? -1 : static_cast<int>(std::distance(entries.begin(), found));
    }

    int findTemplateParameter(int pos) const
    {
        auto found = std::find_if(entries.begin(), entries.end(),
                                  [pos](const SubstitutionEntry& entry) { return entry.matchesTemplateParameter(pos); });
        return found == entries.end() ? -1 : static_cast<int>(std::distance(entries.begin(), found));
    }

    void add(const SgType* type)
    {
        entries.push_back(SubstitutionEntry(type));
    }

    void add(const SgScopeStatement* scope)
    {
        entries.push_back(SubstitutionEntry(scope));
    }

    void add(const SgNode* node)
    {
        entries.push_back(SubstitutionEntry(node));
    }

    void addCompletedClassTemplate(const CompletedClassTemplateKey& key)
    {
        entries.push_back(SubstitutionEntry(static_cast<const SgNode*>(nullptr)));
        completedClassTemplateEntries[key] = static_cast<int>(entries.size() - 1);
    }

    int findCompletedClassTemplate(const CompletedClassTemplateKey& key) const
    {
        auto found = completedClassTemplateEntries.find(key);
        return found == completedClassTemplateEntries.end() ? -1 : found->second;
    }

    void addTemplateParameter(int pos)
    {
        entries.push_back(SubstitutionEntry(pos));
    }

    std::string encode(int index) const
    {
        if (index == 0) return "S_";
        return "S" + encodeSequenceId(index - 1) + "_";
    }
};

// Per-mangling state is intentionally explicit.  A public mangle operation
// owns one context and passes it through every recursive helper; this avoids
// thread-local state leaking substitutions between otherwise independent calls.
struct ManglingContext {
    SubstitutionTable substitutions;
    int templateParameterPositionOffset = 0;
};

/// Itanium sequence identifiers are base 36, with uppercase letters for
/// values above nine.  The caller handles the special zero spelling (S_/T_).
std::string
encodeSequenceId(int value)
{
    ASSERT_require(value >= 0);
    static constexpr char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string result;
    do {
        result.insert(result.begin(), digits[value % 36]);
        value /= 36;
    } while (value != 0);
    return result;
}

// =============================================================================
// Forward declarations
// =============================================================================

std::string mangleType(ManglingContext& context, sg::NotNull<const SgType> type, bool withReturnType, bool withArrayToPointerDecay);
std::string mangleName(const std::string& name);
std::string mangleTemplateArguments(ManglingContext& context, const SgTemplateArgumentPtrList& args);
std::string mangleQualifiedName(ManglingContext& context, const SgFunctionDeclaration& decl, bool localLinkage = false);
std::string mangleNestedEncodedName(ManglingContext& context, const SgScopeStatement& scope, const std::string& encodedName, const std::string& memberQualifiers = "", bool localLinkage = false);
std::string mangleNestedName(ManglingContext& context, const SgScopeStatement& scope, const std::string& name, const std::string& memberQualifiers = "", bool localLinkage = false);
std::string mangleFunctionParameters(ManglingContext& context, const SgFunctionDeclaration& funcDecl,
                                     bool arrayToPointerDecay,
                                     const SgFunctionDeclaration* templateInstantiation = nullptr);
std::string standardClassSubstitution(const SgTemplateInstantiationDecl& classDecl);

CompletedClassTemplateKey
completedClassTemplateKey(const SgTemplateInstantiationDecl& classDecl)
{
    sg::NotNull<const SgTemplateClassDeclaration> templateDecl = classDecl.get_templateDeclaration();

    if (SgTemplateClassDeclaration* first = isSgTemplateClassDeclaration(templateDecl->get_firstNondefiningDeclaration()))
        templateDecl = first;

    // This encoding is solely a semantic identity key.  Use an isolated
    // context so computing it cannot consume candidates in the ABI encoding
    // currently being produced.
    ManglingContext keyContext;

    return { templateDecl, mangleTemplateArguments(keyContext, classDecl.get_templateArguments()) };
}

// =============================================================================
// Type canonicalization
// =============================================================================

sg::NotNull<const SgType>
getCanonicalType(sg::NotNull<const SgType> type)
{
    // Preserve all type constructors and qualifiers while following typedefs.
    return type->stripType(SgType::STRIP_TYPEDEF_TYPE);
}

// =============================================================================
// Simple name mangling
// =============================================================================

std::string mangleName(const std::string& name)
{
    std::ostringstream oss;

    oss << name.length() << name;
    return oss.str();
}

std::string mangleTemplateParameterType(int position)
{
    return position <= 0
              ? "T_"
              : "T" + encodeSequenceId(position - 1) + "_";
}

std::string
mangleTemplateParameterUse(ManglingContext& context, int rawPosition)
{
    // TODO(itanium-review): the offset/fallback heuristic below guesses the parameter level from the
    //   enclosing class template.  Replace with explicit (level, position) from the template parameter.
    //   Test: none dedicated; indirectly ItaniumMangling_templates, ItaniumMangling_ctor_substitution.
    //   ABI:  Template Parameters (#mangle.template-param)
    // PRIORITY: LOW
    int position = rawPosition - context.templateParameterPositionOffset;
    if (position < 0)
        position = rawPosition;

    int substIndex = context.substitutions.findTemplateParameter(position);
    if (substIndex >= 0)
        return context.substitutions.encode(substIndex);

    context.substitutions.addTemplateParameter(position);
    return mangleTemplateParameterType(position);
}

// =============================================================================
// Internal linkage (local linkage) detection and mangling
// =============================================================================

std::string roseIdentityToken(const std::string& value)
{
    static constexpr char hex[] = "0123456789abcdef";
    std::string result;

    result.reserve(value.size() * 2);
    for (unsigned char ch : value) {
        result.push_back(hex[ch >> 4]);
        result.push_back(hex[ch & 0x0f]);
    }

    return result;
}

/// Check if a function declaration has static storage (internal linkage)
bool isStaticFunction(const SgFunctionDeclaration& funcDecl)
{
    if (isSgMemberFunctionDeclaration(&funcDecl))
      return false;

    // Check the storage modifier
    // ROSE getters are const methods returning non-const references
    const SgDeclarationModifier& declMod = funcDecl.get_declarationModifier();
    const SgStorageModifier& storageMod = declMod.get_storageModifier();

    return storageMod.isStatic();
}

/// Check whether a variable declaration has internal linkage.
bool isStaticVariable(const SgVariableDeclaration* varDecl)
{
    // A static data member has class scope but external class-qualified
    // identity; it is not a file-scope internal-linkage variable.
    if (!varDecl || isSgClassDefinition(varDecl->get_scope()))
      return false;

    // Check the storage modifier first.  Namespace-scope const objects also
    // have internal linkage unless an explicit extern declaration overrides it.
    const SgDeclarationModifier& declMod = varDecl->get_declarationModifier();
    const SgStorageModifier& storageMod = declMod.get_storageModifier();
    if (storageMod.isStatic())
        return true;

    if (!isSgGlobal(varDecl->get_scope()) && !isSgNamespaceDefinitionStatement(varDecl->get_scope()))
        return false;

    if (storageMod.isExtern())
        return false;

    return declMod.get_typeModifier().get_constVolatileModifier().isConst();
}

bool
isUnnamedNamespace(const SgNamespaceDeclarationStatement* declaration)
{
    return declaration && declaration->get_isUnnamedNamespace();
}

/// Return whether the ROSE local-linkage marker is needed in addition to the
/// scope encoding.  An anonymous namespace already encodes its internal
/// linkage as _GLOBAL__N_1; ROSE also reports that linkage through isStatic().
/// Therefore that storage flag alone must not cause an additional 'L'.
bool hasLocalLinkageMarker(bool hasStaticStorage, bool inAnonymousNamespace)
{
    return hasStaticStorage && !inAnonymousNamespace;
}

const SgFunctionDeclaration*
isGlobalMainFunction(const SgDeclarationStatement* dcl)
{
    const SgFunctionDeclaration* const func = isSgFunctionDeclaration(dcl);

    bool const globalMain = (  func
                            && func->get_name() == "main"
                            && isSgGlobal(func->get_scope())
                            );

    return globalMain ? func : nullptr;
}

/// Fortran function names are not governed by the Itanium C++ ABI.  Preserve
/// the external spelling used by the established ExternalMangling API instead.
std::string
fortranFunctionName(const SgFunctionDeclaration& declaration)
{
    if (declaration.get_declarationModifier().isBind())
    {
        const std::string binding = declaration.get_binding_label();
        if (!binding.empty())
            return binding;
    }

    std::string name = declaration.get_name().getString();
    std::transform( name.begin(), name.end(), name.begin(),
                    [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); }
                  );

    // Match ExternalMangling's convention.  In particular, names containing
    // an underscore still receive only one trailing underscore.
    return name + "_";
}

/// Check if a scope is or contains an anonymous namespace
/// Returns true if the scope hierarchy includes an anonymous namespace
bool
isInAnonymousNamespace(sg::NotNull<const SgScopeStatement> scope)
{
    if (isSgGlobal(scope))
        return false;

    if (const SgNamespaceDefinitionStatement* nsDef = isSgNamespaceDefinitionStatement(scope))
    {
        SgNamespaceDeclarationStatement* nsDecl = nsDef->get_namespaceDeclaration();
        if (isUnnamedNamespace(nsDecl))
            return true;
    }

    // Recursively check parent scope
    return isInAnonymousNamespace(scope->get_scope());
}

// Initialized names for parameters are not consistently parented below the
// function definition in every ROSE AST.  In particular, a parameter may be
// attached to the declaration's parameter list.  Look for either form before
// falling back to the node's lexical scope.
const SgFunctionDeclaration*
findEnclosingFunctionDeclaration(const SgInitializedName& var)
{
    if (const SgFunctionDeclaration* fundecl = sg::ancestor<SgFunctionDeclaration>(&var))
        return fundecl;

    const SgScopeStatement* scope = var.get_scope();

    while (scope && !isSgGlobal(scope))
    {
        if (const SgFunctionDefinition* definition = isSgFunctionDefinition(scope))
            return definition->get_declaration();

        const SgScopeStatement* parentScope = scope->get_scope();
        if (parentScope == scope)
            break;

        scope = parentScope;
    }

    return nullptr;
}

// ROSE's ExternalMangling distinguishes otherwise identical block-scope
// entities by source location.  The Itanium ABI has no spelling for that
// distinction, so retain it in the existing ROSE suffix namespace.  The
// suffix is deliberately limited to non-linker-visible entities; ordinary
// compiler-visible symbols remain ABI-compatible.
std::string
mangleRoseLocalIdentity(const SgInitializedName& name)
{
    std::string ordinal;

    if (const SgFunctionDeclaration* owner = findEnclosingFunctionDeclaration(name))
    {
        if (SgFunctionParameterList* parameters = owner->get_parameterList())
        {
            const SgInitializedNamePtrList& names = parameters->get_args();
            auto found = std::find(names.begin(), names.end(), &name);

            if (found != names.end())
                ordinal = "_param_" + std::to_string(std::distance(names.begin(), found));
        }
    }

    Sg_File_Info* info = name.get_file_info();
    if (!info || info->isCompilerGenerated())
    {
        // A template instantiation can contain several compiler-generated
        // parameter nodes at the same synthetic position.  Their lexical
        // owner is insufficient: ExternalMangling deliberately gives these
        // nodes distinct identities.  Preserve that established identity in
        // the ROSE-only suffix rather than collapsing it to one ABI spelling.
        std::string externalIdentity;

        try
        {
            externalIdentity = Rose::Source::Analysis::mangle(&name);
        }
        catch (const std::exception&)
        {
            // The caller only reaches this path for an accepted Itanium node;
            // retain the older stable spelling if ExternalMangling declines.
        }

        return (  std::string(ITANIUM_ROSE_EXT)
               + "local"
               + ordinal
               + ( externalIdentity.empty()
                       ? ""
                       : "_external_" + roseIdentityToken(externalIdentity)
                 )
               );
    }

    std::ostringstream oss;

    oss << ITANIUM_ROSE_EXT << "local_"
        << Rose::Source::Analysis::mangleFilename(info->get_filenameString())
        << "_" << info->get_line()
        << "_" << info->get_col() << ordinal;
    return oss.str();
}

// Parameters and non-static data members have no ABI linker spelling, but
// they are nevertheless first-class ROSE entities.  Give them an explicitly
// ROSE-reserved identity rather than returning the unqualified source name.
// The function signature plus parameter position is stable across translation
// units; likewise, the class-qualified field spelling is stable for a member.
std::string
mangleRoseParameterIdentity(const SgInitializedName& name, const SgFunctionDeclaration& function)
{
    sg::NotNull<const SgFunctionParameterList> parameters = function.get_parameterList();
    const SgInitializedNamePtrList& names = parameters->get_args();
    auto found = std::find(names.begin(), names.end(), &name);

    if (found == names.end())
        throw std::runtime_error("Initialized name is not a function parameter");

    const std::size_t  position = std::distance(names.begin(), found);
    ManglingContext    context;

    // Both operations add substitution candidates.  Keep their order explicit:
    // C++ does not sequence the operands of operator+.
    const std::string  qualifiedName = mangleQualifiedName(context, function);
    const std::string  parameterEncoding = mangleFunctionParameters(context, function, true);
    const std::string  owner = qualifiedName + parameterEncoding;
    std::ostringstream oss;

    oss << ABI_PREFIX << ITANIUM_ROSE_EXT << "param_"
        << owner.length() << owner << "_" << position;

    // This deliberately uses the owning function rather than the parameter's
    // lexical scope: compiler-generated template parameters are sometimes
    // given global scope despite belonging to a function.
    if (isStaticFunction(function) || isInAnonymousNamespace(function.get_scope()))
        oss << mangleRoseLocalIdentity(name);

    return oss.str();
}

// Declarations such as namespaces, aliases, and class declarations are
// first-class identities to ExternalMangling, although the Itanium ABI does
// not emit a linker symbol for each of them.  Keep those identities in one
// deliberately separate, ROSE-reserved spelling.  The source position is
// included for AST declarations because it is also the tie breaker used by
// ExternalMangling for anonymous and repeated declarations.
std::string
mangleRoseDeclarationIdentity( const SgDeclarationStatement& decl,
                               const std::string& category,
                               const std::string& detail = ""
                             )
{
    std::ostringstream oss;

    oss << ABI_PREFIX << ITANIUM_ROSE_EXT << category;
    if (!detail.empty())
        oss << "_" << detail.length() << detail;

    if (Sg_File_Info* info = decl.get_file_info())
    {
        if (!info->isCompilerGenerated())
            oss << "_" << Rose::Source::Analysis::mangleFilename(info->get_filenameString())
                << "_" << info->get_line() << "_" << info->get_col();
    }

    return oss.str();
}

std::string
mangleRoseExternalDeclarationIdentity( const SgDeclarationStatement& decl,
                                       const std::string& category
                                     )
{
    try
    {
        return ( std::string(ABI_PREFIX)
               + ITANIUM_ROSE_EXT
               + category
               + "_external_"
               + roseIdentityToken(Rose::Source::Analysis::mangle(&decl))
               );
    }
    catch (const std::exception&)
    {
        return mangleRoseDeclarationIdentity(decl, category);
    }
}

/// Generate internal linkage suffix for static entities
/// Itanium ABI standard: uses 'L' prefix to indicate internal linkage
/// ROSE extension: adds _R<filename> to disambiguate across translation units
/// @param node The node to get filename from
/// @param useRoseExtension If true, append ROSE extension with filename
std::string
mangleInternalLinkage(sg::NotNull<const SgLocatedNode> node, bool useRoseExtension)
{
    if (!useRoseExtension)
        return "";  // Standard Itanium ABI doesn't add file info

    // ROSE getters are const methods returning non-const pointers
    Sg_File_Info* fileInfo = node->get_file_info();
    if (!fileInfo)
        throw std::runtime_error("Cannot mangle internal linkage without source location: " +
                                 node->class_name());

    std::string filename = fileInfo->get_filenameString();
    if (filename.empty() || filename == "NULL_FILE")
        throw std::runtime_error("Cannot mangle internal linkage without source filename: " +
                                 node->class_name());

    // Return ROSE extension: _R<mangled_filename>
    return Rose::Source::Analysis::itaniumLocalLinkageSuffix(filename);
}

// =============================================================================
// Scope hierarchy building
// =============================================================================

std::vector<const SgScopeStatement*>
getScopeHierarchy(const SgScopeStatement& scope)
{
    if (isSgGlobal(&scope))
        return {};

    // TODO(itanium-review): this borrows an Ada helper (SageInterface::Ada::pathToGlobal) for C++
    //   scope walking; move a language-neutral scope-path helper into SageInterface.
    // PRIORITY: VERY LOW
    const SageInterface::Ada::ScopePath path = SageInterface::Ada::pathToGlobal(scope);

    return std::vector<const SgScopeStatement*>(path.begin(), path.end());
}

std::string
mangleClassUnqualifiedName(ManglingContext& context, const SgClassDeclaration& classDecl)
{
    if (const SgTemplateInstantiationDecl* tmplClass = isSgTemplateInstantiationDecl(&classDecl))
    {
        const std::string templateName = tmplClass->get_templateName().getString();
        if (templateName.empty())
            throw std::runtime_error("Template instantiation has no template name");

        return mangleName(templateName) + mangleTemplateArguments(context, tmplClass->get_templateArguments());
    }

    std::string className = classDecl.get_name().getString();

    // TODO(itanium-review): ASSERT_require may compile out in release builds, and the NotNull
    //   wrappers abort rather than throw, while the public API documents std::runtime_error.
    //   Unify failure reporting; catch (std::exception&) blocks elsewhere in this file also swallow
    //   ExternalMangling failures silently.  No test yet.
    // PRIORITY: LOW
    ASSERT_require(className.find('<') == std::string::npos);
    return mangleName(className);
}

// A class can be encountered either as a type or as an enclosing scope.  A
// defining declaration is not guaranteed for explicit instantiations, so use
// the canonical declaration as the common substitution identity in both
// paths.
const SgNode*
classSubstitutionKey(const SgClassDeclaration& classDecl)
{
    if (SgClassDeclaration* first = isSgClassDeclaration(classDecl.get_firstNondefiningDeclaration()))
        return first;

    return &classDecl;
}

bool
isStdNamespace(const SgNamespaceDefinitionStatement& ns)
{
    const SgNamespaceDeclarationStatement* declaration = ns.get_namespaceDeclaration();

    return (  declaration
           && declaration->get_name().getString() == "std"
           && isSgGlobal(declaration->get_scope())
           );
}

bool
isDirectStdNamespaceScope(const SgScopeStatement& scope)
{
    if (const SgNamespaceDefinitionStatement* ns = isSgNamespaceDefinitionStatement(&scope))
      return isStdNamespace(*ns);

    return false;
}

// =============================================================================
// Nested name mangling (with member-function qualifiers and local linkage support)
// Format: N [CV/ref qualifiers] <scope1> <scope2> ... [L] <name> E
// The 'L' prefix indicates internal linkage (static functions/variables)
// =============================================================================

template <typename EncodedNameCallback>
std::string
mangleNestedEncodedNameWithCallback( ManglingContext& context,
                                     const SgScopeStatement& scope,
                                     EncodedNameCallback encodeName,
                                     const std::string& memberQualifiers = "",
                                     bool localLinkage = false
                                   )
{
    std::vector<const SgScopeStatement*> scopes = getScopeHierarchy(scope);

    // DONE(itanium-review): candidate order.  The ABI numbers candidates in order of appearance:
    //   prefix scopes, then the template prefix (e.g. ns::f, ns::Box), then the template arguments.
    //   Callers compute the encoded name (and thereby register the template arguments) BEFORE
    //   this function registers the prefix scopes, and the template prefix is never registered.
    //   Fix by letting the callback run after the prefix is emitted and registering the
    //   template prefix before the arguments.
    //   Tests: ItaniumMangling_substitution_order (ns::f<ns::A*>, ns::Box<ns::A*>::set),
    //          ItaniumMangling_ctor_substitution (ns::NsBox<ns::Bar*>)
    //   ABI:  Compression (#mangling-compression), Nested Names (#mangle.nested-name),
    //         Names (#mangle.name) for the template-prefix production
    // REBUTTAL(itanium-review): This is no longer true.  Callers provide
    //   encodeName as a callback, which runs only after this function emits
    //   the prefix scopes; class-type callers also register the template
    //   prefix before mangling its arguments.  The dedicated substitution
    //   order and constructor-substitution fixtures cover these cases.

    // Unqualified name - if local linkage, we'll handle 'L' at the top level
    if (scopes.empty())
        return encodeName();

    // The standard-namespace substitution is itself a <prefix>.  A direct
    // member of ::std:: therefore uses St followed by its unqualified name,
    // rather than a nested-name production (NSt...E).  This also supplies the
    // canonical St11char_traitsIcE spelling used by Ss/Si/So/Sd matching.
    if (scopes.size() == 1 && isDirectStdNamespaceScope(*scopes.front()) &&
        memberQualifiers.empty() && !localLinkage)
        return std::string(ABI_STD_NAMESPACE) + encodeName();

    // Nested name: N [CV/ref qualifiers] <prefix> [L] <unqualified-name> E
    std::ostringstream oss;

    oss << ABI_NESTED_BEGIN;
    oss << memberQualifiers;

    // TODO(itanium-review): only namespaces and classes are emitted below; any other scope kind
    //   (function-local classes, SgBasicBlock, SgFunctionDefinition, template class definitions)
    //   is silently skipped, so local classes of the same name in different functions collide.
    //   The ABI uses the local-name encoding Z <function encoding> E <entity>.  No test yet.
    //   ABI:  Scope Encoding (#mangling-scope; grammar of <local-name>), Closure Types (#closure-types)
    // PRIORITY: LOW

    // Process scopes outermost first.  A previously registered class is a
    // candidate for its complete qualified prefix, not merely for the class
    // component.  Thus, after seeing ns::A, ns::A::B must use S0_ for ns::A
    // rather than emit substitutions for ns and A separately.
    std::vector<const SgScopeStatement*> outerScopes(scopes.rbegin(), scopes.rend());

    for (std::size_t scopeIndex = 0; scopeIndex < outerScopes.size();)
    {
        std::size_t substitutedThrough = scopeIndex;
        int prefixSubstitution = -1;
        for (std::size_t candidateIndex = outerScopes.size(); candidateIndex > scopeIndex; --candidateIndex)
        {
            const SgClassDefinition* classDef = isSgClassDefinition(outerScopes[candidateIndex - 1]);
            if (!classDef)
                continue;

            sg::NotNull<const SgClassDeclaration> classDecl = classDef->get_declaration();
            int substIndex = -1;

            if (const SgTemplateInstantiationDecl* tmplClass = isSgTemplateInstantiationDecl(classDecl))
                substIndex = context.substitutions.findCompletedClassTemplate(completedClassTemplateKey(*tmplClass));
            else
                substIndex = context.substitutions.find(classSubstitutionKey(*classDecl));

            if (substIndex >= 0)
            {
                prefixSubstitution = substIndex;
                substitutedThrough = candidateIndex;
                break;
            }
        }

        if (prefixSubstitution >= 0)
        {
            oss << context.substitutions.encode(prefixSubstitution);
            scopeIndex = substitutedThrough;
            continue;
        }

        const SgScopeStatement* s = outerScopes[scopeIndex++];

        // St is a predefined ABI substitution.  It must win even if this
        // scope was recorded by an older path: predefined substitutions do
        // not consume ordinary substitution-table positions.
        if (const SgNamespaceDefinitionStatement* ns = isSgNamespaceDefinitionStatement(s))
        {
            if (isStdNamespace(*ns))
            {
                // A standard class substitution covers the ::std prefix as
                // well as the class name.  In particular, a member of
                // std::allocator<int> is N Sa Ii E <member> E, not
                // N St 9allocator Ii E <member> E.  This path is distinct
                // from TypeMangler::handle(SgClassType), which covers the
                // same substitutions when the class is used as a type.
                if (scopeIndex < outerScopes.size())
                {
                    if (const SgClassDefinition* nextClass = isSgClassDefinition(outerScopes[scopeIndex]))
                    {
                        if (const SgTemplateInstantiationDecl* nextTemplate = isSgTemplateInstantiationDecl(nextClass->get_declaration()))
                        {
                            std::string standard = standardClassSubstitution(*nextTemplate);

                            if (!standard.empty())
                            {
                                oss << standard;

                                if (standard == ABI_STD_ALLOCATOR || standard == ABI_STD_BASIC_STRING)
                                {
                                    oss << mangleTemplateArguments(context, nextTemplate->get_templateArguments());
                                    context.substitutions.addCompletedClassTemplate(completedClassTemplateKey(*nextTemplate));
                                }

                                ++scopeIndex;
                                continue;
                            }
                        }
                    }
                }

                oss << ABI_STD_NAMESPACE;
                continue;
            }
        }

        // Check if this scope is already in the substitution table
        // SubstitutionTable now stores const pointers
        int substIndex = context.substitutions.find(s);
        if (substIndex >= 0)
        {
            // Use substitution instead of encoding again
            oss << context.substitutions.encode(substIndex);
        }
        else
        {
            // Encode the scope and add it to substitution table
            if (const SgNamespaceDefinitionStatement* ns = isSgNamespaceDefinitionStatement(s))
            {
                // ROSE getters are const methods returning non-const pointers
                SgNamespaceDeclarationStatement* nsDecl = ns->get_namespaceDeclaration();
                std::string nsName = nsDecl->get_name().getString();

                if (isUnnamedNamespace(nsDecl))
                    oss << ABI_ANONYMOUS_NAMESPACE;
                else
                    oss << mangleName(nsName);

                // The ABI permits the anonymous namespace spelling as a
                // substitution candidate too; omitting it shifts later IDs.
                context.substitutions.add(s);
            }
            else if (const SgClassDefinition* classDef = isSgClassDefinition(s))
            {
                // ROSE getters are const methods returning non-const pointers
                sg::NotNull<const SgClassDeclaration> classDecl = classDef->get_declaration();
                const SgNode* const classKey = classSubstitutionKey(*classDecl);

                if (const SgTemplateInstantiationDecl* tmplClass = isSgTemplateInstantiationDecl(classDecl))
                {
                    const CompletedClassTemplateKey completedKey = completedClassTemplateKey(*tmplClass);
                    const int completedIndex = context.substitutions.findCompletedClassTemplate(completedKey);

                    if (completedIndex >= 0)
                    {
                        oss << context.substitutions.encode(completedIndex);
                        continue;
                    }

                    const std::string templateName = tmplClass->get_templateName().getString();

                    if (templateName.empty())
                        throw std::runtime_error("Template instantiation has no template name");

                    // The template prefix is a candidate before its arguments.
                    oss << mangleName(templateName);
                    context.substitutions.add(classKey);
                    oss << mangleTemplateArguments(context, tmplClass->get_templateArguments());
                    context.substitutions.addCompletedClassTemplate(completedKey);
                }
                else
                {
                    oss << mangleClassUnqualifiedName(context, *classDecl);
                    context.substitutions.add(classKey);
                }
            }
        }
    }

    // Add local linkage marker before the unqualified name if needed
    // Format: N<scopes>L<name>E for static in namespace
    if (localLinkage)
    {
        oss << ABI_LOCAL_LINKAGE;
    }

    // Add the unqualified name, already encoded as either a source name or
    // a special-name/operator code.
    oss << encodeName();
    oss << ABI_NESTED_END;

    return oss.str();
}

std::string
mangleNestedEncodedName( ManglingContext& context,
                         const SgScopeStatement& scope,
                         const std::string& encodedName,
                         const std::string& memberQualifiers,
                         bool localLinkage
                       )
{
    return mangleNestedEncodedNameWithCallback( context,
                                                scope,
                                                [&]() { return encodedName; },
                                                memberQualifiers,
                                                localLinkage
                                              );
}

std::string
mangleNestedName( ManglingContext& context,
                  const SgScopeStatement& scope,
                  const std::string& name,
                  const std::string& memberQualifiers,
                  bool localLinkage
                )
{
    return mangleNestedEncodedName(context, scope, mangleName(name), memberQualifiers, localLinkage);
}

std::string
mangleMemberFunctionCvQualifiers(const SgMemberFunctionType& mft)
{
    std::string result;

    if (mft.isVolatileFunc())
        result += ABI_VOLATILE_QUALIFIER;

    if (mft.isConstFunc())
        result += ABI_CONST_QUALIFIER;

    return result;
}

std::string
mangleMemberFunctionRefQualifier(const SgMemberFunctionType& mft)
{
    if (mft.isLvalueReferenceFunc())
        return ABI_REFERENCE;

    if (mft.isRvalueReferenceFunc())
        return ABI_RVALUE_REFERENCE;

    return {};
}

std::string
mangleMemberFunctionQualifiers(const SgMemberFunctionType& mft)
{
    return ( mangleMemberFunctionCvQualifiers(mft)
           + mangleMemberFunctionRefQualifier(mft)
           );
}

const SgTemplateArgumentPtrList*
getTemplateArguments(const SgFunctionDeclaration& funcDecl)
{
    if (const SgTemplateInstantiationFunctionDecl* tmplFunc = isSgTemplateInstantiationFunctionDecl(&funcDecl))
        return &tmplFunc->get_templateArguments();

    if (const SgTemplateInstantiationMemberFunctionDecl* tmplMember = isSgTemplateInstantiationMemberFunctionDecl(&funcDecl))
        return &tmplMember->get_templateArguments();

    return nullptr;
}

std::string
getTemplateInstantiationName(const SgFunctionDeclaration& funcDecl)
{
    std::string templateName;

    if (const SgTemplateInstantiationFunctionDecl* tmplFunc = isSgTemplateInstantiationFunctionDecl(&funcDecl))
        templateName = tmplFunc->get_templateName().getString();
    else if (const SgTemplateInstantiationMemberFunctionDecl* tmplMember = isSgTemplateInstantiationMemberFunctionDecl(&funcDecl))
        templateName = tmplMember->get_templateName().getString();

    if (templateName.empty())
        throw std::runtime_error("Template function instantiation has no template name");

    return templateName;
}

const SgFunctionDeclaration*
getTemplatePatternDeclaration(const SgFunctionDeclaration& funcDecl)
{
    if (const SgTemplateInstantiationFunctionDecl* tmplFunc = isSgTemplateInstantiationFunctionDecl(&funcDecl))
        return tmplFunc->get_templateDeclaration();

    if (const SgTemplateInstantiationMemberFunctionDecl* tmplMember = isSgTemplateInstantiationMemberFunctionDecl(&funcDecl))
        return tmplMember->get_templateDeclaration();

    return nullptr;
}

bool
hasCLinkage(const SgFunctionDeclaration& declaration)
{
    if (declaration.get_linkage() == "C")
        return true;

    const SgFunctionDeclaration* first = isSgFunctionDeclaration(declaration.get_firstNondefiningDeclaration());

    if (first && first->get_linkage() == "C")
        return true;

    const SgFunctionDeclaration* defining = isSgFunctionDeclaration(declaration.get_definingDeclaration());

    return defining && defining->get_linkage() == "C";
}

// =============================================================================
// Operator name mangling and special member functions
// =============================================================================

// DONE(itanium-review): constructors and destructors bypass the regular nested-name path:
//   - the class is not registered as a substitution candidate, so the copy constructor is
//     _ZN3FooC1ERK3Foo instead of _ZN3FooC1ERKS_
//   - the class name is the raw get_name() (no template arguments, no mangleClassUnqualifiedName)
//   - the third copy of the find("unnamed") anonymous-namespace test lives here
//   Route through mangleNestedEncodedName with the C1/D1 code as the unqualified name.
//   Tests: ItaniumMangling_ctor_substitution (Foo, ns::Bar, Box<int>, ns::NsBox<ns::Bar*>),
//          ItaniumMangling_unnamed_like_namespace
//   ABI:  Constructors and Destructors (#mangling-special-ctor-dtor), Compression (#mangling-compression)
// REBUTTAL(itanium-review): This concern is obsolete.  The implementation
//   below delegates to mangleNestedEncodedName, so class scopes are registered
//   as substitution candidates and template/anonymous-class handling is shared
//   with ordinary nested names.  ItaniumMangling_ctor_substitution exercises
//   the relevant constructor encodings.
std::string
mangleSpecialMemberName( ManglingContext& context,
                         const SgScopeStatement& scope,
                         const char* specialName
                       )
{
    return mangleNestedEncodedName(context, scope, specialName);
}

std::string
mangleQualifiedName(ManglingContext& context, const SgFunctionDeclaration& decl, bool localLinkage)
{
    const SgType* conversionTargetType = nullptr;
    std::string   name = decl.get_name().getString();

    if (decl.get_specialFunctionModifier().isConversion())
    {
        sg::NotNull<const SgFunctionType> funcType = decl.get_type();

        conversionTargetType = funcType->get_return_type();
        ASSERT_not_null(conversionTargetType);
    }

    const SgMemberFunctionDeclaration* const   memberFunc = isSgMemberFunctionDeclaration(&decl);
    sg::NotNull<const SgFunctionParameterList> parameters = decl.get_parameterList();
    const std::size_t                          explicitArity = parameters->get_args().size();
    const bool                                 unaryOperator = explicitArity == (memberFunc ? 0 : 1);
    std::string                                specialName;

    // The four operators below have distinct unary and binary productions.
    if (name == "operator+")              specialName = unaryOperator ? "ps" : "pl";
    else if (name == "operator-")         specialName = unaryOperator ? "ng" : "mi";
    else if (name == "operator*")         specialName = unaryOperator ? "de" : "ml";
    else if (name == "operator&")         specialName = unaryOperator ? "ad" : "an";
    else if (name == "operator/")         specialName = "dv";
    else if (name == "operator%")         specialName = "rm";
    else if (name == "operator|")         specialName = "or";
    else if (name == "operator^")         specialName = "eo";
    else if (name == "operator=")         specialName = "aS";
    else if (name == "operator+=")        specialName = "pL";
    else if (name == "operator-=")        specialName = "mI";
    else if (name == "operator*=")        specialName = "mL";
    else if (name == "operator/=")        specialName = "dV";
    else if (name == "operator%=")        specialName = "rM";
    else if (name == "operator&=")        specialName = "aN";
    else if (name == "operator|=")        specialName = "oR";
    else if (name == "operator^=")        specialName = "eO";
    else if (name == "operator<<")        specialName = "ls";
    else if (name == "operator>>")        specialName = "rs";
    else if (name == "operator<<=")       specialName = "lS";
    else if (name == "operator>>=")       specialName = "rS";
    else if (name == "operator==")        specialName = "eq";
    else if (name == "operator!=")        specialName = "ne";
    else if (name == "operator<")         specialName = "lt";
    else if (name == "operator>")         specialName = "gt";
    else if (name == "operator<=")        specialName = "le";
    else if (name == "operator>=")        specialName = "ge";
    else if (name == "operator<=>")       specialName = "ss";
    else if (name == "operator!")         specialName = "nt";
    else if (name == "operator&&")        specialName = "aa";
    else if (name == "operator||")        specialName = "oo";
    else if (name == "operator~")         specialName = "co";
    else if (name == "operator++")        specialName = "pp";
    else if (name == "operator--")        specialName = "mm";
    else if (name == "operator,")         specialName = "cm";
    else if (name == "operator->*")       specialName = "pm";
    else if (name == "operator->")        specialName = "pt";
    else if (name == "operator co_await") specialName = "aw";
    else if (name == "operator()")        specialName = "cl";
    else if (name == "operator[]")        specialName = "ix";
    else if (name == "operator new")      specialName = "nw";
    else if (name == "operator new[]")    specialName = "na";
    else if (name == "operator delete")   specialName = "dl";
    else if (name == "operator delete[]") specialName = "da";

    // Check for user-defined literal operators (C++11)
    // Format: operator"" _suffix  (note: ROSE may include space)
    // Mangling: li<length><suffix>
    if (specialName.empty() && name.find("operator") == 0)
    {
        // Find the quotes;
        size_t const quotePos = name.find("\"\"");
        if (quotePos != std::string::npos)
        {
            // Extract everything after the quotes
            std::string suffix = name.substr(quotePos + 2);
            // Skip any whitespace
            size_t const firstNonSpace = suffix.find_first_not_of(" \t");
            if (firstNonSpace != std::string::npos)
            {
                suffix = suffix.substr(firstNonSpace);
                // Encode as: li<length><suffix>
                std::ostringstream oss;
                oss << ABI_LITERAL_OPERATOR << suffix.length() << suffix;

                specialName = oss.str();
            }
        }
    }

    std::string memberQualifiers;

    // Check for constructor/destructor
    if (const SgMemberFunctionDeclaration* memberFunc = isSgMemberFunctionDeclaration(&decl))
    {
        // ROSE getters are const methods returning non-const pointers
        sg::NotNull<const SgClassDefinition>  classDef = isSgClassDefinition(memberFunc->get_scope());
        sg::NotNull<const SgClassDeclaration> classDecl = classDef->get_declaration();
        std::string                           className = classDecl->get_name().getString();

        // DONE(itanium-review): for a member of a class template instance className is e.g. "Box<int>"
        //   while the constructor is named "Box", so this comparison fails and constructors and
        //   destructors are mangled as ordinary members (_ZN3BoxIiE3BoxEi, _ZN3BoxIiE4~BoxEv instead
        //   of _ZN3BoxIiEC1Ei, _ZN3BoxIiED1Ev).  Compare against the template name for
        //   SgTemplateInstantiationDecl, or use the SgSpecialFunctionModifier flags.
        //   Test: ItaniumMangling_ctor_substitution
        //   ABI:  Constructors and Destructors (#mangling-special-ctor-dtor)
        // The special-function flags remain correct for class-template
        // instantiations, whose class spelling contains arguments while a
        // constructor's source name does not.
        if (memberFunc->get_specialFunctionModifier().isConstructor() || name == className)
            return mangleSpecialMemberName(context, *classDef, ABI_CTOR_COMPLETE);

        if (memberFunc->get_specialFunctionModifier().isDestructor() || name == "~" + className)
            return mangleSpecialMemberName(context, *classDef, ABI_DTOR_COMPLETE);

        sg::NotNull<const SgMemberFunctionType> mft = isSgMemberFunctionType(memberFunc->get_type());

        memberQualifiers = mangleMemberFunctionQualifiers(*mft);
    }

    // ROSE getters are const methods returning non-const pointers
    sg::NotNull<const SgScopeStatement> const scope = decl.get_scope();

    if (conversionTargetType)
    {
        auto typeMangler = [&]() { return std::string("cv") + mangleType(context, conversionTargetType, true, false); };

        return mangleNestedEncodedNameWithCallback(context, *scope, typeMangler, memberQualifiers, localLinkage);
    }

    if (!specialName.empty())
        return mangleNestedEncodedName(context, *scope, specialName, memberQualifiers, localLinkage);


    if (const SgTemplateArgumentPtrList* templateArgs = getTemplateArguments(decl))
    {
        if (!templateArgs->empty())
        {
            // Evaluate the arguments only after the enclosing prefix has been
            // emitted, preserving the ABI's substitution-number order.
            auto templArgGen =
                     [&]() -> std::string
                     {
                         const std::string prefix = mangleName(getTemplateInstantiationName(decl));

                         // A template-id's prefix is a substitution candidate before
                         // its arguments are encoded.  This matters even at global
                         // scope, where mangleNestedEncodedNameWithCallback has no
                         // enclosing scope to register.  For example, in
                         // f<int>(T), the prefix f<int> occupies entry zero and the
                         // repeated T is therefore S0_, rather than S_.
                         context.substitutions.add(&decl);

                         return prefix + mangleTemplateArguments(context, *templateArgs);
                     };

            return mangleNestedEncodedNameWithCallback(context, *scope, templArgGen, memberQualifiers, localLinkage);
        }
    }

    return mangleNestedName(context, *scope, name, memberQualifiers, localLinkage);
}

// =============================================================================
// Top-level function-parameter qualifier stripping
// Top-level const, volatile, and restrict qualifiers don't affect function
// type identity.  Qualifiers nested below a type constructor remain part of
// the encoding, e.g. int* __restrict* is PrPi rather than PPi.
// =============================================================================

sg::NotNull<const SgType>
stripTopLevelFunctionParameterQualifiers(sg::NotNull<const SgType> type)
{
    // Strip only modifiers that qualify the parameter type itself.
    // ROSE isSg* have const overloads
    while (const SgModifierType* modType = isSgModifierType(type))
    {
        const SgTypeModifier& modifier = modType->get_typeModifier();
        const bool skipTopLevelQualifiers = (  modifier.get_constVolatileModifier().isConst()
                                            || modifier.get_constVolatileModifier().isVolatile()
                                            || modifier.isRestrict()
                                            );

        // Top-level qualifiers do not participate in a function type.
        if (!skipTopLevelQualifiers)
          break;

        type = modType->get_base_type();
    }

    return type;
}

// =============================================================================
// Function parameter mangling
// =============================================================================

std::string
mangleFunctionParameterType(ManglingContext& context, sg::NotNull<const SgType> parameterType)
{
    parameterType = stripTopLevelFunctionParameterQualifiers(getCanonicalType(parameterType));

    // Parameter types undergo exactly one adjustment: an array or function
    // parameter decays to a pointer, but constructors below that pointer keep
    // their own qualifiers and bounds.
    if (isSgFunctionType(parameterType))
        return std::string(ABI_POINTER) + mangleType(context, parameterType, true, false);

    return mangleType(context, parameterType, true, true);
}

int
templateParameterPosition(const SgType& type)
{
    if (const SgTemplateType* templateType = isSgTemplateType(&type))
        return templateType->get_template_parameter_position();

    if (const SgNonrealType* nonrealType = isSgNonrealType(&type))
    {
        const SgNonrealDecl* nonrealDecl = isSgNonrealDecl(nonrealType->get_declaration());
        if (nonrealDecl && nonrealDecl->get_is_template_param())
            return nonrealDecl->get_template_parameter_position() - 1;
    }

    return -1;
}

bool
isTemplateParameterPack(const SgFunctionDeclaration& instantiation, const SgType& parameterType)
{
    const int position = templateParameterPosition(*getCanonicalType(&parameterType));
    if (position < 0)
        return false;

    const SgTemplateArgumentPtrList* arguments = getTemplateArguments(instantiation);
    if (!arguments)
        return false;

    std::size_t argumentIndex = 0;
    for (int parameterIndex = 0; argumentIndex < arguments->size(); ++parameterIndex)
    {
        const SgTemplateArgument* argument = (*arguments)[argumentIndex];
        const bool isPack = argument &&
            argument->get_argumentType() == SgTemplateArgument::start_of_pack_expansion_argument;

        if (parameterIndex == position)
            return isPack;

        ++argumentIndex;
        if (isPack)
        {
            while (argumentIndex < arguments->size() && (*arguments)[argumentIndex] &&
                   (*arguments)[argumentIndex]->get_is_pack_element())
                ++argumentIndex;
        }
    }

    return false;
}

std::string
mangleFunctionParameters( ManglingContext& context,
                          const SgFunctionDeclaration& funcDecl,
                          bool arrayToPointerDecay,
                          const SgFunctionDeclaration* templateInstantiation
                        )
{
    SgFunctionParameterList const* paramList = funcDecl.get_parameterList();
    if (!paramList)
        return ABI_TYPE_VOID;  // No parameters = void

    const SgInitializedNamePtrList& params = paramList->get_args();
    if (params.empty())
        return ABI_TYPE_VOID;  // No parameters = void

    std::ostringstream oss;
    for (sg::NotNull<const SgInitializedName> param : params)
    {
        sg::NotNull<const SgType> paramType = param->get_type();

        // A function parameter declared from a template parameter pack is a
        // pack expansion in the ABI function-type grammar.  The expansion
        // marker applies to the type encoding, e.g. Args... becomes DpT_.
        const bool encodePack = (  param->get_is_parameter_pack()
                                || (templateInstantiation && isTemplateParameterPack(*templateInstantiation, *paramType))
                                );

        if (encodePack)
            oss << "Dp";

        if (arrayToPointerDecay)
            oss << mangleFunctionParameterType(context, paramType);
        else
            oss << mangleType(context, paramType, true, false);
    }

    return oss.str();
}

std::string
mangleTemplateFunctionSignature(ManglingContext& context, const SgFunctionDeclaration& funcDecl)
{
    const SgFunctionDeclaration* patternDecl = getTemplatePatternDeclaration(funcDecl);
    if (!patternDecl || !funcDecl.isTemplateFunction())
        return mangleFunctionParameters(context, funcDecl, true);

    int savedOffset = context.templateParameterPositionOffset;
    if (const SgTemplateInstantiationMemberFunctionDecl* tmplMember = isSgTemplateInstantiationMemberFunctionDecl(&funcDecl))
    {
        if (const SgClassDefinition* classDef = isSgClassDefinition(tmplMember->get_scope()))
        {
            if (const SgTemplateInstantiationDecl* tmplClass = isSgTemplateInstantiationDecl(classDef->get_declaration()))
                context.templateParameterPositionOffset = static_cast<int>(tmplClass->get_templateArguments().size());
        }
    }

    std::ostringstream oss;
    if (const SgFunctionType* funcType = patternDecl->get_type())
        oss << mangleType(context, funcType->get_return_type(), true, false);

    oss << mangleFunctionParameters(context, *patternDecl, true, &funcDecl);
    context.templateParameterPositionOffset = savedOffset;
    return oss.str();
}

// =============================================================================
// Type mangling (comprehensive)
// Handles all C++ type constructs with substitution support
// Uses ROSE sg::dispatch mechanism for type-based dispatch
// =============================================================================

// =============================================================================
// Constant expression evaluation for array dimensions
// =============================================================================

std::string
extractConstantValue(const SgExpression* expr);

struct ConstantValueExtractor : sg::DispatchHandler<std::string>
{
    void handle(const SgNode& n)                   { SG_UNEXPECTED_NODE(n); }
    void handle(const SgExpression&)               { res = ""; }

    void handle(const SgIntVal& n)                 { res = std::to_string(n.get_value()); }
    void handle(const SgUnsignedLongVal& n)        { res = std::to_string(n.get_value()); }
    void handle(const SgLongIntVal& n)             { res = std::to_string(n.get_value()); }
    void handle(const SgLongLongIntVal& n)         { res = std::to_string(n.get_value()); }
    void handle(const SgShortVal& n)               { res = std::to_string(n.get_value()); }
    void handle(const SgUnsignedIntVal& n)         { res = std::to_string(n.get_value()); }
    void handle(const SgUnsignedLongLongIntVal& n) { res = std::to_string(n.get_value()); }
    void handle(const SgUnsignedShortVal& n)       { res = std::to_string(n.get_value()); }
    void handle(const SgCharVal& n)                { res = std::to_string(static_cast<int>(n.get_value())); }
    void handle(const SgSignedCharVal& n)          { res = std::to_string(static_cast<int>(n.get_value())); }
    void handle(const SgUnsignedCharVal& n)        { res = std::to_string(static_cast<unsigned int>(n.get_value())); }
    void handle(const SgChar16Val& n)              { res = std::to_string(n.get_value()); }
    void handle(const SgChar32Val& n)              { res = std::to_string(n.get_value()); }
    void handle(const SgEnumVal& n)                { res = std::to_string(n.get_value()); }
    void handle(const SgBoolValExp& n)             { res = n.get_value() ? "1" : "0"; }

    void handle(const SgValueExp& n)               { res = n.get_constant_folded_value_as_string(); }
    void handle(const SgCastExp& n)                { res = extractConstantValue(n.get_operand()); }
};

/// Extracts a constant integer value from an expression.  Uses sg::dispatch
/// for literals, folded values, and casts; returns an empty string for values
/// that do not have a compile-time integer representation.
std::string
extractConstantValue(const SgExpression* expr)
{
    // When frontend is called with e_original_expressions_and_folded_values,
    // prefer the alternative expression containing the folded value.
    if (SgExpression* foldedExpr = expr->get_alternativeExpr())
        expr = foldedExpr;

    return sg::dispatch(ConstantValueExtractor(), expr);
}

// DONE(itanium-review): a bound that depends directly on a template parameter
// is encoded as a dependent expression instead of being rejected.  ROSE may
// represent the reference as SgTemplateParameterVal, SgNonrealRefExp, or
// SgVarRefExp; all three forms produce the applicable T<n>_ encoding.
// Test: ItaniumMangling_template_array_bound
// ABI: Array Types (#mangle.array-type), Template Parameters (#mangle.template-param)

/// Encode the restricted dependent expressions permitted as an Itanium array
/// bound.  A template parameter is an <expr> in the array-type grammar, so a
/// parameter such as N is encoded as T_ (or T0_, T1_, ...), rather than as a
/// numeric bound.
///
/// Keep this deliberately narrow.  Compound dependent expressions (N + 1,
/// sizeof(T), etc.) require the general Itanium expression grammar and must
/// not be mistaken for a plain template-parameter reference.
std::string
mangleDependentArrayBound(ManglingContext& context, SgExpression* expr)
{
    // EDG can preserve an otherwise transparent integral conversion around
    // the non-type template parameter.
    if (SgCastExp* castExpr = isSgCastExp(expr))
        return mangleDependentArrayBound(context, castExpr->get_operand());

    if (SgTemplateParameterVal* parameter = isSgTemplateParameterVal(expr))
    {
        const int position = parameter->get_template_parameter_position();
        if (position >= 0)
            return mangleTemplateParameterUse(context, position);

        return "";
    }

    if (const SgVarRefExp* reference = isSgVarRefExp(expr))
    {
        sg::NotNull<const SgVariableSymbol> symbol = reference->get_symbol();
        sg::NotNull<const SgInitializedName> name = symbol->get_declaration();
        sg::NotNull<const SgTemplateParameter> parameter = isSgTemplateParameter(name->get_parent());
        sg::NotNull<const SgNode> owner = parameter->get_parent();

        const SgTemplateParameterPtrList* parameters = nullptr;

        if (const SgTemplateFunctionDeclaration* declaration = isSgTemplateFunctionDeclaration(owner))
            parameters = &declaration->get_templateParameters();
        else if (const SgTemplateMemberFunctionDeclaration* declaration = isSgTemplateMemberFunctionDeclaration(owner))
            parameters = &declaration->get_templateParameters();
        else if (const SgTemplateClassDeclaration* declaration = isSgTemplateClassDeclaration(owner))
            parameters = &declaration->get_templateParameters();

        if (parameters)
        {
            auto found = std::find(parameters->begin(), parameters->end(), parameter);

            if (found != parameters->end())
                return mangleTemplateParameterUse(context,
                                                  static_cast<int>(std::distance(parameters->begin(), found)));
        }
    }

    if (SgNonrealRefExp* reference = isSgNonrealRefExp(expr))
    {
        SgNonrealSymbol* symbol = reference->get_symbol();
        SgNonrealDecl* declaration = symbol ? symbol->get_declaration() : nullptr;

        if (declaration && declaration->get_is_template_param())
        {
            // SgNonrealDecl uses one-based positions; the ABI encoding uses
            // zero-based positions (T_ denotes position zero).
            const int position = declaration->get_template_parameter_position();

            if (position > 0)
                return mangleTemplateParameterUse(context, position - 1);
        }
    }

    return "";
}

std::string
mangleTemplateArgument(ManglingContext& context, sg::NotNull<const SgTemplateArgument> arg)
{
    switch (arg->get_argumentType())
    {
        case SgTemplateArgument::type_argument:
            return mangleType(context, arg->get_type(), true, false);

        case SgTemplateArgument::nontype_argument:
        {
            SgExpression* expr = arg->get_expression();
            if (!expr)
            {
                // ROSE represents a non-type template argument using either
                // an expression or an initialized name.  The latter is a
                // valid AST form, but it needs ABI support beyond the
                // constant-expression encoding implemented below.
                sg::NotNull<const SgInitializedName> initializedName = arg->get_initializedName();
                throw std::runtime_error( "Itanium mangling does not support non-type template arguments "
                                          "represented by SgInitializedName: "
                                        + initializedName->get_name().getString()
                                        );
            }

            std::string value = extractConstantValue(expr);
            if (value.empty())
                throw std::runtime_error("Unsupported non-type template argument: " + expr->class_name());

            if (!value.empty() && value[0] == '-')
                value = "n" + value.substr(1);

            std::ostringstream oss;
            oss << "L" << mangleType(context, expr->get_type(), true, false) << value << "E";
            return oss.str();
        }

        case SgTemplateArgument::template_template_argument:
        {
            const SgDeclarationStatement* templateDecl = arg->get_templateDeclaration();
            if (const SgTemplateDeclaration* td = isSgTemplateDeclaration(templateDecl))
                return mangleName(td->get_name().getString());

            throw std::runtime_error("Itanium mangling expected SgTemplateDeclaration for "
                                     "template-template argument");
        }

        case SgTemplateArgument::start_of_pack_expansion_argument:
            return "Dp";

        case SgTemplateArgument::argument_undefined:
        default:
            throw std::runtime_error("Itanium mangling: unexpected SgTemplateArgument::get_argumentType");
    }
}

std::string
mangleTemplateArguments(ManglingContext& context, const SgTemplateArgumentPtrList& args)
{
    if (args.empty()) return "";

    std::ostringstream oss;
    std::size_t i = 0;

    oss << "I";
    while (i < args.size())
    {
        sg::NotNull<const SgTemplateArgument> arg = args[i];

        if (arg->get_argumentType() == SgTemplateArgument::start_of_pack_expansion_argument)
        {
            oss << "J";

            ++i;
            while (i < args.size())
            {
                sg::NotNull<const SgTemplateArgument> maybePack = args[i];

                if (!maybePack->get_is_pack_element())
                    break;

                oss << mangleTemplateArgument(context, maybePack);
                ++i;
            }

            oss << "E";
        }
        else
        {
            oss << mangleTemplateArgument(context, arg);
            ++i;
        }
    }

    oss << "E";
    return oss.str();
}

// Return an ABI standard substitution for a direct member of ::std::, or an
// empty string when no standard substitution applies.  The argument probe
// deliberately uses an isolated context so deciding whether a
// complete standard specialization matches cannot perturb the substitutions
// for the enclosing encoding.
std::string
standardClassSubstitution(const SgTemplateInstantiationDecl& classDecl)
{
    if (!isDirectStdNamespaceScope(*classDecl.get_scope()))
        return {};

    const std::string name = classDecl.get_templateName().getString();

    if (name == "allocator")
        return ABI_STD_ALLOCATOR;

    bool const unabbrevClass = (  name != "basic_string"
                               && name != "basic_istream"
                               && name != "basic_ostream"
                               && name != "basic_iostream"
                               );

    if (unabbrevClass)
        return {};

    ManglingContext   probeContext;
    const std::string arguments = mangleTemplateArguments(probeContext, classDecl.get_templateArguments());

    if (name == "basic_string")
    {
        if (arguments == "IcSt11char_traitsIcESaIcEE")
            return ABI_STD_STRING;

        return ABI_STD_BASIC_STRING;
    }

    if (arguments != "IcSt11char_traitsIcEE")
        return {};

    if (name == "basic_istream")
        return ABI_STD_ISTREAM;

    if (name == "basic_ostream")
        return ABI_STD_OSTREAM;

    return ABI_STD_IOSTREAM;
}

// Helper struct for dispatching type mangling using ROSE sg::dispatch
struct TypeMangler : sg::DispatchHandler<std::string>
{
    ManglingContext& context;
    bool withReturnType;
    bool withArrayToPointerDecay;

    TypeMangler(ManglingContext& manglingContext, bool retType, bool arrayDecay)
    : context(manglingContext), withReturnType(retType), withArrayToPointerDecay(arrayDecay)
    {}

    // non-type node handler
    void handle(const SgNode& n) { SG_UNEXPECTED_NODE(n); }

    void handle(const SgType& n)
    {
        throw std::runtime_error("Unsupported type in Itanium mangling: " + n.class_name());
    }

    // Builtin types (not substitutable per Itanium ABI)
    void handle(const SgTypeVoid&)             { res = ABI_TYPE_VOID; }
    void handle(const SgTypeBool&)             { res = ABI_TYPE_BOOL; }
    void handle(const SgTypeChar&)             { res = ABI_TYPE_CHAR; }
    void handle(const SgTypeSignedChar&)       { res = ABI_TYPE_SIGNED_CHAR; }
    void handle(const SgTypeUnsignedChar&)     { res = ABI_TYPE_UNSIGNED_CHAR; }
    void handle(const SgTypeWchar&)            { res = ABI_TYPE_WCHAR; }
    void handle(const SgTypeChar16&)           { res = ABI_TYPE_CHAR16; }
    void handle(const SgTypeChar32&)           { res = ABI_TYPE_CHAR32; }
    void handle(const SgTypeShort&)            { res = ABI_TYPE_SHORT; }
    void handle(const SgTypeUnsignedShort&)    { res = ABI_TYPE_UNSIGNED_SHORT; }
    void handle(const SgTypeInt&)              { res = ABI_TYPE_INT; }
    void handle(const SgTypeUnsignedInt&)      { res = ABI_TYPE_UNSIGNED_INT; }
    void handle(const SgTypeLong&)             { res = ABI_TYPE_LONG; }
    void handle(const SgTypeUnsignedLong&)     { res = ABI_TYPE_UNSIGNED_LONG; }
    void handle(const SgTypeLongLong&)         { res = ABI_TYPE_LONG_LONG; }
    void handle(const SgTypeUnsignedLongLong&) { res = ABI_TYPE_UNSIGNED_LONG_LONG; }
    void handle(const SgTypeFloat&)            { res = ABI_TYPE_FLOAT; }
    void handle(const SgTypeDouble&)           { res = ABI_TYPE_DOUBLE; }
    void handle(const SgTypeLongDouble&)       { res = ABI_TYPE_LONG_DOUBLE; }
    void handle(const SgTypeEllipse&)          { res = ABI_TYPE_ELLIPSIS; }

    void handle(const SgAutoType&)             { res = "Da"; }

    void handle(const SgTemplateType& templateType)
    {
        res = mangleTemplateParameterUse(context, templateType.get_template_parameter_position());
    }

    void handle(const SgNonrealType& nonrealType)
    {
        const SgNonrealDecl* nonrealDecl = isSgNonrealDecl(nonrealType.get_declaration());
        if (!nonrealDecl || !nonrealDecl->get_is_template_param())
            throw std::runtime_error("Unsupported nonreal type in Itanium mangling");

        // EDG stores SgNonrealDecl positions starting at one, whereas the
        // Itanium T_ spelling represents the first (zero-indexed) parameter.
        const int position = nonrealDecl->get_template_parameter_position();
        if (position <= 0)
            throw std::runtime_error("Invalid template parameter position in Itanium mangling");

        res = mangleTemplateParameterUse(context, position - 1);
    }

    // Pointer to member types
    void handle(const SgPointerMemberType& pmType)
    {
        sg::NotNull<const SgType> classType  = pmType.get_class_type();
        sg::NotNull<const SgType> memberType = pmType.get_base_type();
        std::ostringstream        oss;

        oss << ABI_MEMBER_POINTER;
        oss << mangleType(context, classType, true, false);

        if (const SgMemberFunctionType* memberFunctionTy = isSgMemberFunctionType(memberType))
            oss << mangleMemberFunctionCvQualifiers(*memberFunctionTy);

        oss << mangleType(context, memberType, true, false);

        res = oss.str();
        context.substitutions.add(&pmType);
    }

    // Pointer types
    void handle(const SgPointerType& ptrType)
    {
        sg::NotNull<const SgType> baseType = ptrType.get_base_type();

        res = std::string(ABI_POINTER) + mangleType(context, baseType, true, false);
        context.substitutions.add(&ptrType);
    }

    // Reference types
    void handle(const SgReferenceType& refType)
    {
        sg::NotNull<const SgType> baseType = refType.get_base_type();

        res = std::string(ABI_REFERENCE) + mangleType(context, baseType, true, false);
        context.substitutions.add(&refType);
    }

    // Rvalue reference types (C++11)
    void handle(const SgRvalueReferenceType& rrefType)
    {
        sg::NotNull<const SgType> baseType = rrefType.get_base_type();

        res = std::string(ABI_RVALUE_REFERENCE) + mangleType(context, baseType, true, false);
        context.substitutions.add(&rrefType);
    }

    // Const/volatile modifiers
    void handle(const SgModifierType& modType)
    {
        std::string           result;
        const SgTypeModifier& modifier = modType.get_typeModifier();
        bool emitsQualifier = false;

        // The ABI requires qualifiers in r, V, K order.  A modifier that
        // contributes no ABI qualifier is transparent and must not consume a
        // substitution-table entry.
        if (modifier.isRestrict())
        {
            result += ABI_RESTRICT_QUALIFIER;
            emitsQualifier = true;
        }

        if (modifier.get_constVolatileModifier().isVolatile())
        {
            result += ABI_VOLATILE_QUALIFIER;
            emitsQualifier = true;
        }

        if (modifier.get_constVolatileModifier().isConst())
        {
            result += ABI_CONST_QUALIFIER;
            emitsQualifier = true;
        }

        sg::NotNull<const SgType> baseType = modType.get_base_type();

        result += mangleType(context, baseType, withReturnType, withArrayToPointerDecay);
        res = result;
        if (emitsQualifier)
            context.substitutions.add(&modType);
    }

    // Array types
    void handle(const SgArrayType& arrayType)
    {
        if (withArrayToPointerDecay)
        {
            // Decay to pointer - don't add to substitution table as it's transformed
            sg::NotNull<const SgType> baseType = arrayType.get_base_type();

            res = std::string(ABI_POINTER) + mangleType(context, baseType, true, false);
            return;
        }

        if (arrayType.get_is_variable_length_array())
            throw std::runtime_error("Itanium mangling does not support variable-length array bounds");

        // Mangle as array: A<size>_<element-type>
        std::ostringstream oss;
        oss << ABI_ARRAY;

        // Array dimensions are part of an array type's ABI encoding.  Do
        // not silently omit a bound that we cannot encode: doing so would
        // make distinct pointer-to-array types collide.
        SgExpression* indexExpr = arrayType.get_index();

        if (indexExpr && !isSgNullExpression(indexExpr))
        {
            std::string constantValue = extractConstantValue(indexExpr);
            if (!constantValue.empty())
                oss << constantValue;
            else
            {
                std::string dependentBound = mangleDependentArrayBound(context, indexExpr);
                if (dependentBound.empty())
                    throw std::runtime_error("Itanium mangling does not support nonconstant array bounds: " +
                                             indexExpr->unparseToString());

                oss << dependentBound;
            }
        }

        // If no size, it's an incomplete array A_<type>
        sg::NotNull<const SgType> baseType = arrayType.get_base_type();

        oss << "_" << mangleType(context, baseType, true, false);
        res = oss.str();
        context.substitutions.add(&arrayType);
    }

    // Function types
    void handle(const SgFunctionType& funcType)
    {
        std::ostringstream oss;
        oss << ABI_FUNCTION;

        if (withReturnType)
        {
            sg::NotNull<const SgType> retType = funcType.get_return_type();
            oss << mangleType(context, retType, true, false);
        }

        // Function parameter arrays always undergo the language-mandated
        // array-to-pointer adjustment, even if this function type is itself
        // nested below a pointer, reference, or another type constructor.
        const SgTypePtrList& paramTypes = funcType.get_arguments();

        if (paramTypes.empty())
        {
            oss << ABI_TYPE_VOID;  // void parameter list
        }
        else
        {
            for (sg::NotNull<const SgType> paramType : paramTypes)
                oss << mangleFunctionParameterType(context, paramType);
        }

        // In a function-type production, a member function's cv qualifiers
        // precede the F (the pointer-to-member handler emits those), whereas
        // its ref qualifier follows the bare function type and precedes E.
        if (const SgMemberFunctionType* memberFunction = isSgMemberFunctionType(&funcType))
            oss << mangleMemberFunctionRefQualifier(*memberFunction);

        oss << "E";
        res = oss.str();
        context.substitutions.add(&funcType);
    }

    // Class/struct/union types
    void handle(const SgClassType& classType)
    {
        sg::NotNull<const SgClassDeclaration>    classDecl  = isSgClassDeclaration(classType.get_declaration());
        const SgTemplateInstantiationDecl* const tmplClass  = isSgTemplateInstantiationDecl(classDecl);
        const SgNode* const                      classKey   = classSubstitutionKey(*classDecl);

        if (tmplClass)
        {
            // The standard substitutions encode either a complete type
            // (Ss/Si/So/Sd) or a template prefix (Sa/Sb).  They must be
            // considered before ordinary name emission.  Sa/Sb template-ids
            // are nevertheless ordinary completed-type candidates after
            // their arguments have been emitted, as required by the ABI.
            std::string standard = standardClassSubstitution(*tmplClass);
            if (!standard.empty())
            {
                bool const stdAbbrev = (  standard == ABI_STD_ALLOCATOR
                                       || standard == ABI_STD_BASIC_STRING
                                       );

                if (stdAbbrev)
                {
                    const CompletedClassTemplateKey completedKey = completedClassTemplateKey(*tmplClass);
                    const int abbrevIndex = context.substitutions.findCompletedClassTemplate(completedKey);

                    if (abbrevIndex >= 0)
                    {
                        res = context.substitutions.encode(abbrevIndex);
                        return;
                    }

                    res = standard + mangleTemplateArguments(context, tmplClass->get_templateArguments());
                    context.substitutions.addCompletedClassTemplate(completedKey);
                }
                else
                {
                    res = std::move(standard);
                }

                return;
            }

            const int substIndex = context.substitutions.findCompletedClassTemplate(completedClassTemplateKey(*tmplClass));
            if (substIndex >= 0)
            {
                res = context.substitutions.encode(substIndex);
                return;
            }
        }

        const int substIndex = context.substitutions.find(classKey);
        if (substIndex >= 0 && (tmplClass == nullptr))
        {
            res = context.substitutions.encode(substIndex);
            return;
        }

        sg::NotNull<const SgScopeStatement> scope = classDecl->get_scope();

        // Delay the template arguments until the nested-name prefix has been
        // emitted.  The class definition is the canonical substitution key
        // both here and when the class later appears as a scope.
        auto templInstanceFn =
            [&]()
            {
                if (tmplClass)
                {
                    const CompletedClassTemplateKey completedKey = completedClassTemplateKey(*tmplClass);
                    const std::string templateName = tmplClass->get_templateName().getString();
                    if (templateName.empty())
                        throw std::runtime_error("Template instantiation has no template name");

                    const std::string prefix = mangleName(templateName);

                    context.substitutions.add(classKey);
                    const std::string result = prefix + mangleTemplateArguments(context, tmplClass->get_templateArguments());

                    // The prefix (ns::Box) and the completed template-id
                    // (ns::Box<ns::A*>) are separate ABI substitution candidates.
                    // A later use of the class type must refer to the latter.
                    context.substitutions.addCompletedClassTemplate(completedKey);
                    return result;
                }

                const std::string result = mangleClassUnqualifiedName(context, *classDecl);
                context.substitutions.add(classKey);

                return result;
            };

        res = mangleNestedEncodedNameWithCallback(context, *scope, templInstanceFn);
    }

    // Enum types
    void handle(const SgEnumType& enumType)
    {
        sg::NotNull<const SgEnumDeclaration> enumDecl = isSgEnumDeclaration(enumType.get_declaration());
        std::string enumName = enumDecl->get_name().getString();

        if (enumName.empty())
        {
            // TODO(itanium-review): an unnamed enum is not int: it needs the unnamed-type encoding
            //   (Ut_ / Ut<n>_, or the typedef name for linkage purposes), otherwise f(enum{..}) and
            //   f(int) collide.  Anonymous classes instead use whatever name ROSE generated, which
            //   is inconsistent with this.  No dedicated test; see ItaniumMangling_anonymous_enums.
            //   ABI:  Unnamed Type Names (#mangle.unnamed-type-name), Class, Union, Enum Types (#mangling.named)
            // Anonymous enum - treat as int
            // PRIORITY: LOW
            res = ABI_TYPE_INT;
        }
        else
        {
            sg::NotNull<const SgScopeStatement> scope = enumDecl->get_scope();

            res = mangleNestedName(context, *scope, enumName);
            context.substitutions.add(&enumType);
        }
    }

    // Typedef types - resolve to canonical type
    void handle(const SgTypedefType& typedefType)
    {
        // Get the canonical (underlying) type and dispatch to its handler
        sg::NotNull<const SgType> baseType = typedefType.get_base_type();

        res = mangleType(context, baseType, withReturnType, withArrayToPointerDecay);
    }
};

std::string
mangleType(ManglingContext& context, sg::NotNull<const SgType> type, bool withReturnType, bool withArrayToPointerDecay)
{
    // Get canonical type (resolve typedefs)
    sg::NotNull<const SgType> canonicalType = getCanonicalType(type);

    // Check for type substitution (compression)
    // Per Itanium ABI, we can substitute previously seen non-builtin types
    // Note: Builtin types are NOT substitutable, but the handlers don't add them
    // to the substitution table, so this check is safe for all types
    // Class template specializations have a semantic completed-template-id
    // identity.  Let the class handler consult it before any raw-node lookup.
    if (isSgClassType(canonicalType))
    {
        TypeMangler mangler(context, withReturnType, withArrayToPointerDecay);
        return sg::dispatch(mangler, canonicalType);
    }

    int substIndex = context.substitutions.find(canonicalType);
    if (substIndex >= 0)
        return context.substitutions.encode(substIndex);

    // Use sg::dispatch to handle the type (including builtins)
    TypeMangler mangler(context, withReturnType, withArrayToPointerDecay);
    return sg::dispatch(mangler, canonicalType);
}

} // anonymous namespace

// =============================================================================
// Public API implementation
// =============================================================================

namespace Rose::Source::Analysis
{

namespace {

std::string
mangleDeclaration(const SgDeclarationStatement& n, bool withUniqueMain)
{
    std::string result;

    // Get the first non-defining declaration (canonical declaration)
    // ROSE getters are const methods returning non-const pointers
    const SgDeclarationStatement* decl = &n;
    const bool isTemplateInstantiationDecl = (  isSgTemplateInstantiationFunctionDecl(decl)
                                             || isSgTemplateInstantiationMemberFunctionDecl(decl)
                                             );

    if (!isTemplateInstantiationDecl)
    {
        if (SgDeclarationStatement* firstNonDef = n.get_firstNondefiningDeclaration())
            decl = firstNonDef;
    }

    // The program entry point has C linkage spelling even in C++.
    // withUniqueMain is a ROSE extension for projects that contain more than
    // one input translation unit with an entry point.
    if (/*const SgFunctionDeclaration* funcDecl =*/ isGlobalMainFunction(decl))
    {
        result = "main";

        if (withUniqueMain)
            result += mangleInternalLinkage(&n, true);

        return result;
    }

    // The Itanium ABI applies to C++, not Fortran.  Return the Fortran
    // linker spelling so this API remains compatible with ExternalMangling.
    if (SageInterface::is_Fortran_language())
    {
        if (const SgFunctionDeclaration* funcDecl = isSgFunctionDeclaration(decl))
            return fortranFunctionName(*funcDecl);
    }

    // Condition 1: Check for C language source file
    if (SageInterface::is_C_language())
    {
        // TODO(itanium-review): is_C_language() is a project-global query, so a project mixing C and
        //   C++ sources mangles C++ functions as C (or the reverse).  Decide per declaration/file.
        // C language - no mangling
        // PRIORITY: LOW
        if (const SgFunctionDeclaration* funcDecl = isSgFunctionDeclaration(decl))
        {
            // ROSE getters are const methods returning non-const pointers
            result = funcDecl->get_name().getString();

            // C has no ABI mangling, but a file-scope static function has
            // internal linkage and therefore needs a ROSE-local identity
            // when multiple C translation units are analyzed together.
            if (isStaticFunction(*funcDecl))
            {
                sg::NotNull<const SgFunctionDeclaration> origDecl = isSgFunctionDeclaration(&n);
                result += mangleInternalLinkage(origDecl, true);
            }

            return result;
        }
    }

    // TODO(itanium-review): SgTemplateFunctionDeclaration and SgTemplateMemberFunctionDeclaration
    //   derive from SgFunctionDeclaration and are mangled here like ordinary functions
    //   (e.g. _Z3fooT_), producing a plausible but non-ABI name for a template pattern, which has
    //   no linker identity.  Reject or give them the ROSE-extension identity (see
    //   mangleRoseExternalDeclarationIdentity).  The reference driver skips them today.
    //   ABI:  Names (#mangle.name)
    // PRIORITY: LOW
    if (const SgFunctionDeclaration* funcDecl = isSgFunctionDeclaration(decl))
    {
        // Condition 2: Check for C linkage (extern "C").  Linkage may live
        // on a redeclaration rather than the declaration selected above.
        sg::NotNull<const SgFunctionDeclaration> origDecl = isSgFunctionDeclaration(&n);
        if (hasCLinkage(*origDecl) || hasCLinkage(*funcDecl))
        {
            // C linkage - no mangling
            result = funcDecl->get_name().getString();
            const bool internal = isStaticFunction(*funcDecl) ||
                                  isInAnonymousNamespace(funcDecl->get_scope());
            if (internal)
                result += mangleInternalLinkage(origDecl, true);
            return result;
        }

        // Mangle C++ function
        std::ostringstream oss;
        oss << ABI_PREFIX;

        // Anonymous namespaces encode internal linkage themselves as
        // _GLOBAL__N_1; do not add a redundant local-linkage marker.
        const SgScopeStatement* funcScope = funcDecl->get_scope();
        bool inAnonNS = isInAnonymousNamespace(funcScope);
        bool hasLocalLinkage = hasLocalLinkageMarker(isStaticFunction(*funcDecl), inAnonNS);

        // Get the scope to determine if this is at global level
        bool isGlobalScope = (isSgGlobal(funcScope) != nullptr);

        // For global static functions: add 'L' right after _Z
        // For namespaced static functions: 'L' will be added inside nested name by mangleQualifiedName
        if (hasLocalLinkage && isGlobalScope)
            oss << ABI_LOCAL_LINKAGE;

        ManglingContext context;

        // Pass localLinkage flag to mangleQualifiedName
        oss << mangleQualifiedName(context, *funcDecl, hasLocalLinkage);

        // Add parameter types. Function template instantiations include the
        // dependent return type in the function type encoding.
        oss << mangleTemplateFunctionSignature(context, *funcDecl);

        // ROSE Extension: Add filename suffix for entities with internal linkage
        // Format: _R<mangled_filename>
        // This extends the Itanium ABI to provide unique names across translation units
        // CRITICAL: Use original declaration (&n) not canonical (decl) for file info
        // because canonical declaration may have NULL_FILE as location
        if (hasLocalLinkage || inAnonNS)
        {
            Sg_File_Info const* fileInfo = origDecl->get_file_info();
            std::string filename = fileInfo ? fileInfo->get_filenameString() : "";

            bool const invalidFile = (  !fileInfo
                                     || fileInfo->isCompilerGenerated()
                                     || filename.empty()
                                     || filename == "NULL_FILE"
                                     || filename == "compilerGenerated"
                                     );

            if (invalidFile)
            {
                if (const SgFunctionDeclaration* patternDecl = getTemplatePatternDeclaration(*funcDecl))
                    origDecl = patternDecl;
            }

            oss << mangleInternalLinkage(origDecl, true);
        }

        result = oss.str();
    } else if (isSgNamespaceDeclarationStatement(decl)) {
        // Namespace declarations have no linker spelling.  They nevertheless
        // carry a distinct ExternalMangling identity (notably for reopening
        // and anonymous namespaces), so expose that through the ROSE domain.
        result = mangleRoseExternalDeclarationIdentity(*decl, "namespace");
    } else if (isSgClassDeclaration(decl)) {
        // A named class's type continues to use the normal Itanium type
        // encoding.  This declaration result represents the broader ROSE AST
        // identity, for which the ABI has no standalone symbol.
        const char* category = isSgTemplateInstantiationDecl(decl)
                                  ? "template_instantiation"
                                  : (isSgTemplateClassDeclaration(decl) ? "template_class" : "class");
        result = mangleRoseExternalDeclarationIdentity(*decl, category);
    } else if (isSgEnumDeclaration(decl)) {
        result = mangleRoseExternalDeclarationIdentity(*decl, "enum");
    } else if (isSgTypedefDeclaration(decl)) {
        result = mangleRoseExternalDeclarationIdentity(*decl, "typedef");
    } else if (isSgTemplateInstantiationDirectiveStatement(decl)) {
        result = mangleRoseExternalDeclarationIdentity(*decl, "template_instantiation_directive");
    } else {
        throw std::runtime_error("Unsupported declaration type for mangling");
    }

    return result;
}

std::string
mangleTypeName(const SgType& n, bool withReturnType, bool withArrayToPointerDecay)
{
    ManglingContext context;
    return mangleType(context, &n, withReturnType, withArrayToPointerDecay);
}

} // anonymous namespace

// Declaration mangling - cache wrapper.
const std::string&
itaniumName(const SgDeclarationStatement& n, bool withUniqueMain)
{
    return cachedItaniumName(n, withUniqueMain ? 1 : 0,
                             [&]() { return mangleDeclaration(n, withUniqueMain); });
}

// Declaration mangling - pointer version (delegates to reference version)
const std::string&
itaniumName(const SgDeclarationStatement* n, bool withUniqueMain)
{
    if (!n)
        throw std::runtime_error("Cannot mangle null declaration");

    return itaniumName(*n, withUniqueMain);
}

// Type mangling - cache wrapper.
const std::string&
itaniumName(const SgType& n, bool withReturnType, bool withArrayToPointerDecay)
{
    const int flags = (withReturnType ? 1 : 0) | (withArrayToPointerDecay ? 2 : 0);
    return cachedItaniumName(n, flags,
                             [&]() { return mangleTypeName(n, withReturnType, withArrayToPointerDecay); });
}

// Type mangling - pointer version (delegates to reference version)
const std::string&
itaniumName(const SgType* n, bool withReturnType, bool withArrayToPointerDecay)
{
    if (!n)
        throw std::runtime_error("Cannot mangle null type");

    return itaniumName(*n, withReturnType, withArrayToPointerDecay);
}

namespace
{

std::string
mangleInitializedName(const SgInitializedName& n)
{
    if (isSgCtorInitializerList(n.get_parent()))
        throw std::runtime_error("Cannot mangle constructor-initializer target");

    ManglingContext context;

    std::string name = n.get_name().getString();
    sg::NotNull<const SgScopeStatement> scope = n.get_scope();

    // Function parameters and non-static data members are not emitted as
    // independent ABI symbols.  ROSE still needs stable, unique identities
    // for cross-translation-unit analysis, so encode them in its reserved
    // extension namespace before applying ordinary C/C++ linkage handling.
    if (const SgFunctionDeclaration* function = findEnclosingFunctionDeclaration(n))
    {
        if (SgFunctionParameterList* parameters = function->get_parameterList())
        {
            const SgInitializedNamePtrList& names = parameters->get_args();
            if (std::find(names.begin(), names.end(), &n) != names.end())
                return mangleRoseParameterIdentity(n, *function);
        }
    }

    if (/*const SgClassDefinition* classDefinition =*/ isSgClassDefinition(scope))
        // Fields have no independent ABI symbol, but their class-qualified
        // spelling is stable across the syntax-oriented member declarations
        // that ROSE can create (including static-member definitions).
        return std::string(ABI_PREFIX) + mangleNestedName(context, *scope, name);

    // Condition 1: Check for C language source file
    if (SageInterface::is_C_language())
        return name;

    // Condition 2: Check for C linkage (extern "C") - variables
    // NOTE: ROSE doesn't propagate extern "C" linkage to SgVariableDeclaration
    // the same way it does for SgFunctionDeclaration. This is a known limitation.
    // Variables declared with extern "C" will be mangled as C++ variables.

    // Check if this is a local variable or parameter. ROSE commonly gives
    // block-scope variables an SgBasicBlock scope, so walk parents instead of
    // relying only on n.get_scope().
    // Local variable or parameter - these typically aren't mangled
    // but we'll mangle them with the function scope
    // Local variables and parameters have no ABI linker spelling.  Use
    // the long ExternalMangling identity as the ROSE extension payload:
    // this is the one canonical identity that survives both source and
    // compiler-generated template AST forms.  The fallback keeps the
    // historical owner-based spelling for nodes ExternalMangling rejects.
    if (findEnclosingFunctionDeclaration(n))
        return ( std::string(ABI_PREFIX)
               + ITANIUM_ROSE_EXT
               + "local_external_"
               + roseIdentityToken(Rose::Source::Analysis::mangle(&n))
               );

    // TODO(itanium-review): a non-static variable at global scope has no mangling in the ABI: its
    //   symbol is the plain identifier x, and only namespace/class members are _ZN..E.  A static
    //   one is _ZL1x.  This emits _Z1x for the common case.  Variables are not covered by the
    //   nm-based function reference; add a name-level test.
    //   ABI:  General (#mangling-general), Names (#mangle.name)
    // Global or namespace-scope variable
    std::ostringstream oss;

    oss << ABI_PREFIX;

    // Check if this is a static variable (internal linkage)
    // Get the parent variable declaration to check storage modifier
    const SgVariableDeclaration* varDecl = isSgVariableDeclaration(n.get_parent());
    bool const hasStaticStorage = isStaticVariable(varDecl);
    bool const inAnonNS         = isInAnonymousNamespace(scope);
    bool const hasLocalLinkage  = hasLocalLinkageMarker(hasStaticStorage, inAnonNS);

    // Determine if this is at global scope
    bool const isGlobalScope = (isSgGlobal(scope) != nullptr);

    // For global static variables: add 'L' right after _Z
    // For namespaced static variables: 'L' will be added inside nested name
    if (hasLocalLinkage && isGlobalScope)
        oss << ABI_LOCAL_LINKAGE;

    oss << mangleNestedName(context, *scope, name, "", hasLocalLinkage);

    // ROSE Extension: Add filename suffix for static variables to disambiguate across files
    // Format: _R<mangled_filename>
    // This extends the Itanium ABI to provide unique names for static symbols
    if ((hasLocalLinkage || inAnonNS) && varDecl)
        oss << mangleInternalLinkage(varDecl, true);

    return oss.str();
}

} // anonymous namespace

// Initialized-name mangling - cache wrapper.
const std::string&
itaniumName(const SgInitializedName& n)
{
    return cachedItaniumName(n, 0, [&]() { return mangleInitializedName(n); });
}

// InitializedName mangling - pointer version (delegates to reference version)
const std::string&
itaniumName(const SgInitializedName* n)
{
    if (!n)
        throw std::runtime_error("Cannot mangle null initialized name");

    return itaniumName(*n);
}

void
clearItaniumNameCache()
{
    mangleCache.clear();
}

std::string
itaniumLocalLinkageSuffix(boost::string_view filename)
{
    return std::string(ITANIUM_ROSE_EXT) + mangleFilename(filename);
}

} // namespace Rose::Source::Analysis
