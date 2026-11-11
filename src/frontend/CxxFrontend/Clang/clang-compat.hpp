#ifndef ROSE_CLANG_COMPAT_HPP
#define ROSE_CLANG_COMPAT_HPP

// (10/30/2026) lin32: Select APIs using the linked Clang headers, not __clang_major__ of the compiler building ROSE.
#include "clang/Basic/Version.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#if CLANG_VERSION_MAJOR < 19
#include "clang/AST/ExprOpenMP.h"
#endif
// (10/30/2026) lin32: LocInfoType still exists in Clang 22; include it explicitly instead of relying on transitive includes.
#include "clang/AST/LocInfoType.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/Type.h"
#include "clang/Basic/SourceManager.h"
#include <string>

namespace RoseClangCompat {

// (10/30/2026) lin32: Clang 19 renamed OMPArraySectionExpr and its statement kind to ArraySectionExpr.
// (10/30/2026) lin32: Share these aliases between the ROSE and DOT visitors while retaining older Clang support.
#if CLANG_VERSION_MAJOR >= 19
using ArraySectionExpr = clang::ArraySectionExpr;
constexpr clang::Stmt::StmtClass ArraySectionExprClass = clang::Stmt::ArraySectionExprClass;
#else
using ArraySectionExpr = clang::OMPArraySectionExpr;
constexpr clang::Stmt::StmtClass ArraySectionExprClass = clang::Stmt::OMPArraySectionExprClass;
#endif

// (10/30/2026) lin32: Clang 22 changed NestedNameSpecifier from a pointer-based API to a value type.
#if CLANG_VERSION_MAJOR >= 22
using NestedNameSpecifier = clang::NestedNameSpecifier;
#else
using NestedNameSpecifier = clang::NestedNameSpecifier *;
#endif

// (10/30/2026) lin32: Clang 22 combines namespace and alias qualifiers under Kind::Namespace.
// (10/30/2026) lin32: Recover their distinct declarations for ROSE's mutable traversal API.
inline clang::NamespaceDecl *getAsNamespace(NestedNameSpecifier qualifier) {
#if CLANG_VERSION_MAJOR >= 22
    if (qualifier.getKind() != clang::NestedNameSpecifier::Kind::Namespace)
        return nullptr;
    return const_cast<clang::NamespaceDecl *>(
        llvm::dyn_cast<clang::NamespaceDecl>(qualifier.getAsNamespaceAndPrefix().Namespace));
#else
    return qualifier->getAsNamespace();
#endif
}

inline clang::NamespaceAliasDecl *getAsNamespaceAlias(NestedNameSpecifier qualifier) {
#if CLANG_VERSION_MAJOR >= 22
    if (qualifier.getKind() != clang::NestedNameSpecifier::Kind::Namespace)
        return nullptr;
    return const_cast<clang::NamespaceAliasDecl *>(
        llvm::dyn_cast<clang::NamespaceAliasDecl>(qualifier.getAsNamespaceAndPrefix().Namespace));
#else
    return qualifier->getAsNamespaceAlias();
#endif
}

inline clang::CXXRecordDecl *getAsRecordDecl(NestedNameSpecifier qualifier) {
#if CLANG_VERSION_MAJOR >= 22
    return qualifier.getAsRecordDecl();
#else
    return qualifier->getAsRecordDecl();
#endif
}

inline bool isGlobal(NestedNameSpecifier qualifier) {
#if CLANG_VERSION_MAJOR >= 22
    return qualifier.getKind() == clang::NestedNameSpecifier::Kind::Global;
#else
    return qualifier->getKind() == clang::NestedNameSpecifier::Global;
#endif
}

// (10/30/2026) lin32: Clang 22 removed ElaboratedType; elaboration and tag ownership live on underlying types.
// (10/30/2026) lin32: Report no wrapper on Clang 22 so existing type-unwrapping loops continue to terminate.
inline bool isElaboratedType(const clang::Type *type) {
#if CLANG_VERSION_MAJOR >= 22
    (void)type;
    return false;
#else
    return llvm::isa<clang::ElaboratedType>(type);
#endif
}

// (10/30/2026) lin32: With no ElaboratedType wrapper in Clang 22, the qualified type is already the named type.
inline clang::QualType getNamedType(clang::QualType type) {
#if CLANG_VERSION_MAJOR >= 22
    return type;
#else
    return llvm::cast<clang::ElaboratedType>(type.getTypePtr())->getNamedType();
#endif
}

// (10/30/2026) lin32: Use TagType::isTagOwned() in Clang 22 to preserve inline struct/enum definition ownership.
inline clang::TagDecl *getOwnedTagDecl(const clang::Type *type) {
#if CLANG_VERSION_MAJOR >= 22
    const auto *tag = llvm::dyn_cast<clang::TagType>(type);
    return tag && tag->isTagOwned() ? tag->getDecl() : nullptr;
#else
    const auto *elaborated = llvm::dyn_cast<clang::ElaboratedType>(type);
    return elaborated ? elaborated->getOwnedTagDecl() : nullptr;
#endif
}

// (10/30/2026) lin32: FileEntry no longer provides getName() in newer Clang; obtain names through FileEntryRef.
// (10/30/2026) lin32: Preserve the legacy API below Clang 18 and handle file IDs without a file entry safely.
inline std::string getFileName(const clang::SourceManager &sourceManager, clang::FileID fileID) {
#if CLANG_VERSION_MAJOR >= 18
    auto file = sourceManager.getFileEntryRefForID(fileID);
#else
    const auto *file = sourceManager.getFileEntryForID(fileID);
#endif
    return file ? file->getName().str() : std::string();
}

} // namespace RoseClangCompat

#endif
