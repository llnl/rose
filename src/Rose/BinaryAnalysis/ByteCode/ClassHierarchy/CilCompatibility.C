#include <sage3basic.h>
#include <Rose/BinaryAnalysis/ByteCode/ClassHierarchy/CilCompatibility.h>

#include <sageGeneric.h>

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <Rose/Common/Analysis/ClassHierarchyAnalysis.h>

namespace Rose::BinaryAnalysis::ByteCode::ClassHierarchy
{

using namespace Rose::Common::Analysis;

// Helper to convert keys to AST nodes and vice versa

sg::NotNull<SgAsmCilMethodDef>
keyToFunction(FunctionKeyType key)
{
  return reinterpret_cast<SgAsmCilMethodDef*>(key.key);
}

sg::NotNull<SgAsmCilTypeDef>
keyToClass(ClassKeyType key)
{
  return reinterpret_cast<SgAsmCilTypeDef*>(key.key);
}

sg::NotNull<SgAsmCilField>
keyToVariable(VariableKeyType key)
{
  return reinterpret_cast<SgAsmCilField*>(key.key);
}

FunctionKeyType
functionToKey(sg::NotNull<SgAsmCilMethodDef> n)
{
  return FunctionKeyType{reinterpret_cast<std::uintptr_t>(&*n)};
}

ClassKeyType
classToKey(sg::NotNull<SgAsmCilTypeDef> n)
{
  return ClassKeyType{reinterpret_cast<std::uintptr_t>(&*n)};
}

VariableKeyType
variableToKey(sg::NotNull<SgAsmCilField> n)
{
  return VariableKeyType{reinterpret_cast<std::uintptr_t>(&*n)};
}

sg::NotNull<SgProject>
keyToRoot(ASTRootType key)
{
  return reinterpret_cast<SgProject*>(key.key);
}

ASTRootType
rootToKey(sg::NotNull<SgProject> n)
{
  return ASTRootType{reinterpret_cast<std::uintptr_t>(&*n)};
}

// CIL Attribute Flags
// From ECMA-335 specification
static constexpr uint16_t CIL_FIELD_INITONLY = 0x0020;

// Helper function to get metadata root from a CIL metadata node
// Navigate up the AST to find the SgAsmCilMetadataRoot
inline
SgAsmCilMetadataRoot*
getMetadataRootFromNode(SgNode* node)
{
  if (!node) return nullptr;

  // Try direct cast first
  if (SgAsmCilMetadataRoot* root = isSgAsmCilMetadataRoot(node))
    return root;

  // Traverse up the parent chain
  SgNode* current = node;
  while (current) {
    if (SgAsmCilMetadataRoot* root = isSgAsmCilMetadataRoot(current))
      return root;
    current = current->get_parent();
  }

  return nullptr;
}

// Helper functions to extract virtual functions and data members

static
void
extractVirtualFunctions(SgAsmCilTypeDef* typeDef, SgAsmCilMetadataHeap* metadataHeap,
                        const std::vector<SgAsmCilTypeDef*>& allTypeDefs, size_t typeDefIndex,
                        ClassData& classData)
{
  if (!typeDef || !metadataHeap) return;

  // Get the MethodDefTable
  SgAsmCilMethodDefTable* methodDefTable = metadataHeap->get_MethodDefTable();
  if (!methodDefTable) return;

  const std::vector<SgAsmCilMethodDef*>& allMethods = methodDefTable->get_elements();
  if (allMethods.empty()) return;

  // Get the method range for this TypeDef
  // get_MethodList() returns a 1-based index
  uint32_t methodListIdx = typeDef->get_MethodList();
  if (methodListIdx == 0) return;

  size_t beg = methodListIdx - 1; // Convert to 0-based

  // Calculate end: either the next TypeDef's MethodList or end of table
  size_t end = allMethods.size();
  if (typeDefIndex + 1 < allTypeDefs.size())
  {
    uint32_t nextMethodListIdx = allTypeDefs[typeDefIndex + 1]->get_MethodList();
    if (nextMethodListIdx > 0)
      end = nextMethodListIdx - 1;
  }

  // Iterate through methods belonging to this TypeDef
  for (size_t i = beg; i < end && i < allMethods.size(); ++i)
  {
    SgAsmCilMethodDef* method = allMethods[i];
    if (!method) continue;

    // Skip constructors (.ctor, .cctor)
    const uint8_t* nameBytes = method->get_Name_string();
    if (nameBytes)
    {
      std::string methodName(reinterpret_cast<const char*>(nameBytes));
      if (methodName == ".ctor" || methodName == ".cctor")
        continue;
    }

    // Check whether this instance method participates in virtual dispatch.
    uint16_t flags = method->get_Flags();
    bool isStatic = (flags & SgAsmCilMethodDef::FLG_STATIC) != 0;
    bool isVirtual = (flags & SgAsmCilMethodDef::FLG_VIRTUAL) != 0;

    // Only virtual instance methods participate in override matching. The slot
    // rules (new slot, final, access, return type) are applied pairwise by
    // canOverrideInContext, which also sees the classes between the two
    // methods.
    //
    // Note: A method that starts a new slot does not override. A method that
    //   reuses a slot overrides only the nearest method of that slot, not the
    //   methods that a new-slot method of an intermediate class hides.
    //   \code{.cs}
    //   class Base    { public virtual string Name() => "Base"; }
    //   class Derived : Base { public new virtual string Name() => "Derived"; }
    //   class Leaf    : Derived { public override string Name() => "Leaf"; }
    //   \endcode
    //   Derived.Name is FLG_VIRTUAL | FLG_NEW_SLOT and does not override
    //   Base.Name. Leaf.Name overrides Derived.Name only.
    //
    // Note: C# emits `virtual final newslot` for implicit interface
    //   implementations. For an interface method neither flag prevents the
    //   match; the implementing method must be public.
    //   \code{.cs}
    //   interface IShape { double Area(); }
    //   class Square : IShape { public double Area() => 1.0; } // virtual final newslot
    //   \endcode
    //
    // Note: An assembly (or family-and-assembly) virtual is overridden from
    //   another assembly only by a public method, and not at all if it is
    //   marked strict (ECMA-335 II.10.3.3). Private and compiler-controlled
    //   virtuals never match by name.
    //   \code{.cs}
    //   // assembly A
    //   public class Base { internal virtual void F() {} }
    //   // assembly B (no InternalsVisibleTo)
    //   public class Derived : Base { internal virtual void F() {} } // new slot, not an override
    //   \endcode
    //
    // TODO(CIL): InternalsVisibleTo is not modeled. A friend assembly is
    //   treated as a different assembly, so an override of an internal virtual
    //   from a friend assembly is missed.
    // PRIORITY: LOW
    //
    // TODO(CIL): FLG_FINAL is enforced when overrides are matched, but clients
    //   cannot learn that a method has no possible overriders.
    //   \code{.cs}
    //   class Base { public virtual void F() {} }
    //   class Mid : Base { public sealed override void F() {} } // final
    //   \endcode
    // PRIORITY: HIGH
    //
    // Note: static virtual / static abstract interface members (C# 11) are
    //   excluded by the isStatic test on purpose. They are not dispatched
    //   through an object. The runtime resolves a constrained call on a type
    //   parameter to one static method per concrete type when the generic is
    //   instantiated, so there is no override relation to model.
    //   \code{.cs}
    //   interface IAdd<T> { static abstract T Zero { get; } }
    //   struct MyInt : IAdd<MyInt> { public static MyInt Zero => default; }
    //   \endcode
    //
    // TODO(CIL): The MethodImpl table (.override) is not consulted.
    //   Explicit interface implementations, overrides under a different
    //   name, covariant returns (C# 9), and default-interface-method
    //   overrides are expressed there and are missed by name matching.
    //   canOverride can only reject pairs and cannot add them, so this needs
    //   a new hook in the common analysis (for example, a bridge query that
    //   reports explicit override pairs for a class).
    //   \code{.cs}
    //   interface IFoo { void Bar(); }
    //   class C : IFoo { void IFoo.Bar() {} } // private virtual final newslot "Ns.IFoo.Bar" + MethodImpl
    //   \endcode
    // PRIORITY: HIGH
    //
    // TODO(CIL): Implied overrides are not enabled.
    //   CIL, like Java, lets a method inherited from a base class implement an
    //   interface method of a subclass (ECMA-335 II.12.2). The common analysis
    //   records this when the bridge returns true from
    //   inheritedMethodsImplementVirtualBaseMethods, and
    //   isInheritableImplementation selects the qualifying methods (the JVM
    //   bridge accepts public, non-abstract methods; the CIL condition must be
    //   checked against II.12.2). CilCompatibility overrides neither.
    //   \code{.cs}
    //   interface IFoo { void M(); }
    //   class B { public virtual void M() {} }
    //   class C : B, IFoo {}   // B.M implements IFoo.M; witness class is C
    //   // if B.M were not virtual, C# emits a forwarding stub C.M (virtual final newslot)
    //   \endcode
    // PRIORITY: MEDIUM
    if (!isStatic && isVirtual)
    {
      FunctionKeyType funcKey = functionToKey(method);
      classData.virtualFunctions().push_back(funcKey);
    }
  }
}

static
void
extractDataMembers(SgAsmCilTypeDef* typeDef, SgAsmCilMetadataHeap* metadataHeap,
                   const std::vector<SgAsmCilTypeDef*>& allTypeDefs, size_t typeDefIndex,
                   ClassData& classData)
{
  if (!typeDef || !metadataHeap) return;

  // Get the FieldTable
  SgAsmCilFieldTable* fieldTable = metadataHeap->get_FieldTable();
  if (!fieldTable) return;

  const std::vector<SgAsmCilField*>& allFields = fieldTable->get_elements();
  if (allFields.empty()) return;

  // Get the field range for this TypeDef
  // get_FieldList() returns a 1-based index
  uint32_t fieldListIdx = typeDef->get_FieldList();
  if (fieldListIdx == 0) return;

  size_t beg = fieldListIdx - 1; // Convert to 0-based

  // Calculate end: either the next TypeDef's FieldList or end of table
  size_t end = allFields.size();
  if (typeDefIndex + 1 < allTypeDefs.size())
  {
    uint32_t nextFieldListIdx = allTypeDefs[typeDefIndex + 1]->get_FieldList();
    if (nextFieldListIdx > 0)
      end = nextFieldListIdx - 1;
  }

  // Iterate through fields belonging to this TypeDef
  for (size_t i = beg; i < end && i < allFields.size(); ++i)
  {
    SgAsmCilField* field = allFields[i];
    if (!field) continue;

    // Check if field is static
    uint16_t flags = field->get_Flags();
    bool isStatic = (flags & SgAsmCilField::FLG_STATIC) != 0;

    // Only add non-static fields (instance data members)
    //
    // TODO(CIL): The field's type is not recorded here (see typeOf). For a
    //   value-type field the declaring type embeds the field type, which
    //   matters for layout and composition relations.
    //   \code{.cs}
    //   struct Point { int x, y; }
    //   class Shape { Point origin; }  // Shape embeds Point; not visible in the hierarchy
    //   \endcode
    // PRIORITY: MEDIUM
    if (!isStatic)
    {
      VariableKeyType varKey = variableToKey(field);
      classData.dataMembers().push_back(varKey);
    }
  }
}

namespace
{
  std::string
  cilString(const uint8_t* bytes)
  {
    return bytes ? std::string(reinterpret_cast<const char*>(bytes)) : std::string();
  }

  struct CilTypeIdentity
  {
    std::string assemblyName;
    std::string typeNamespace;
    std::string typeName;

    bool operator<(const CilTypeIdentity& that) const
    {
      if (assemblyName != that.assemblyName) return assemblyName < that.assemblyName;
      if (typeNamespace != that.typeNamespace) return typeNamespace < that.typeNamespace;
      return typeName < that.typeName;
    }
  };

  // TODO(CIL): Memory-pool extraction mixes all loaded metadata roots, while
  //   extractFromProject restricts classes to one project. Inheritance edges
  //   (inheritanceEdges) are always resolved against the memory pool, so an
  //   analysis built from a project can contain an edge to a class that was
  //   never added to ClassAnalysis, and addInheritanceEdge then throws
  //   std::out_of_range. Resolution must be limited to the analyzed set, or
  //   missing bases must be modeled as external classes.
  //   \code{.cs}
  //   // Lib.dll (loaded in the memory pool, but not under the analyzed root)
  //   public class Base {}
  //   // App.exe (analyzed project)
  //   class Derived : Base {}  // edge Derived -> Base has no ClassAnalysis entry
  //   \endcode
  // PRIORITY: LOW
  std::vector<SgAsmCilMetadataRoot*>
  loadedCilMetadataRoots()
  {
    struct CilMetadataRootVisitor : ROSE_VisitTraversal
    {
      void visit(SgNode* node) override
      {
        if (SgAsmCilMetadataRoot* root = isSgAsmCilMetadataRoot(node))
          metadataRoots.push_back(root);
      }

      std::vector<SgAsmCilMetadataRoot*> metadataRoots;
    };

    CilMetadataRootVisitor visitor;
    SgAsmCilMetadataRoot::traverseMemoryPoolNodes(visitor);
    return visitor.metadataRoots;
  }

  std::string
  assemblyName(SgAsmCilMetadataRoot* metadataRoot)
  {
    if (!metadataRoot)
      return std::string();

    SgAsmCilMetadataHeap* metadataHeap = metadataRoot->get_MetadataHeap();
    if (!metadataHeap)
      return std::string();

    SgAsmCilAssemblyTable* assemblyTable = metadataHeap->get_AssemblyTable();
    if (!assemblyTable)
      return std::string();

    const std::vector<SgAsmCilAssembly*>& assemblies = assemblyTable->get_elements();
    if (assemblies.empty() || !assemblies.front())
      return std::string();

    return cilString(assemblies.front()->get_Name_string());
  }

  std::string
  assemblyName(const SgAsmCilAssemblyRef* assemblyRef)
  {
    return assemblyRef ? cilString(assemblyRef->get_Name_string()) : std::string();
  }

  // TODO(CIL): Performance. inheritanceEdges constructs a resolver per class,
  //   and each construction traverses the memory pool and indexes every
  //   TypeDef. Build the index once per analysis (or cache it per metadata
  //   root) and pass it to inheritanceEdges. The JVM bridge has the same
  //   problem (a memory-pool traversal per edge and per name lookup).
  // PRIORITY: LOW
  //
  // TODO(CIL): Type identity is {assembly, namespace, name}.
  //   - Nested types have an empty namespace and a simple name, so nested
  //     types with the same name in different enclosing types collide, and
  //     findUnique then returns nullptr (the edge is silently dropped).
  //     The NestedClass table must be part of the identity.
  //   - The same assembly loaded twice produces duplicate identities and
  //     drops all edges to its types.
  //   \code{.cs}
  //   class A { public class Enumerator {} }
  //   class B { public class Enumerator {} }  // same {asm, "", "Enumerator"} as A.Enumerator
  //   class C : A.Enumerator {}               // TypeRef resolution is ambiguous
  //   \endcode
  // PRIORITY: MEDIUM
  //
  // TODO(CIL): Unresolvable types (bases from assemblies that are not
  //   loaded, e.g., System.Object, System.ValueType, System.Exception) are
  //   dropped without a trace. The JVM bridge at least warns once per missing
  //   ancestor name (warnUnresolvedAncestor); CIL should do the same in
  //   inheritanceEdges. Beyond that, the hierarchy cannot distinguish "derives
  //   from an external class" from "has no base class". Consider
  //   synthesizing an opaque external ClassData entry keyed on the TypeRef.
  //   \code{.cs}
  //   class MyError : System.Exception {}  // corelib not loaded => no base edge
  //   \endcode
  // PRIORITY: HIGH
  class CilMetadataResolver
  {
    public:
      CilMetadataResolver()
      {
        for (SgAsmCilMetadataRoot* metadataRoot : loadedCilMetadataRoots())
          index(metadataRoot);
      }

      SgAsmCilTypeDef* resolve(const SgAsmCilMetadata* metadata) const
      {
        if (!metadata)
          return nullptr;

        if (SgAsmCilTypeDef* typeDef = isSgAsmCilTypeDef(const_cast<SgAsmCilMetadata*>(metadata)))
          return typeDef;

        const SgAsmCilTypeRef* typeRef = isSgAsmCilTypeRef(metadata);

        // TODO(CIL): TypeSpec (generic instantiations) is not resolved, so
        //   base and interface edges to instantiated generic types are
        //   dropped. Resolve the TypeSpec blob to the generic type definition
        //   so that the hierarchy stays connected. The type arguments are not
        //   kept: per-instantiation class identity, variance, and constraints
        //   are not modeled. Matching overrides through the instantiation
        //   (Base<int>.Put(int) vs. Base<T>.Put(T)) additionally requires
        //   substitution, see compareFunctionsByType.
        //   \code{.cs}
        //   class Base<T> { public virtual void Put(T t) {} }
        //   class IntBox : Base<int> { public override void Put(int t) {} }
        //   class Cmp : System.IComparable<Cmp> { public int CompareTo(Cmp o) => 0; }
        //   \endcode
        //   Extends of IntBox and the InterfaceImpl of Cmp are TypeSpec rows.
        if (!typeRef)
          return nullptr;

        return resolve(typeRef);
      }

    private:
      using TypeIndex = std::map<CilTypeIdentity, std::vector<SgAsmCilTypeDef*> >;

      TypeIndex typeIndex;

      void index(SgAsmCilMetadataRoot* metadataRoot)
      {
        if (!metadataRoot)
          return;

        const std::string ownerAssembly = assemblyName(metadataRoot);
        SgAsmCilMetadataHeap* metadataHeap = metadataRoot->get_MetadataHeap();
        if (!metadataHeap)
          return;

        SgAsmCilTypeDefTable* typeDefTable = metadataHeap->get_TypeDefTable();
        if (!typeDefTable)
          return;

        for (SgAsmCilTypeDef* typeDef : typeDefTable->get_elements())
        {
          if (!typeDef)
            continue;

          CilTypeIdentity id{ownerAssembly,
                             cilString(typeDef->get_TypeNamespace_string()),
                             cilString(typeDef->get_TypeName_string())};

          typeIndex[id].push_back(typeDef);
        }
      }

      SgAsmCilTypeDef* findUnique(const CilTypeIdentity& id) const
      {
        TypeIndex::const_iterator found = typeIndex.find(id);
        if (found == typeIndex.end())
          return nullptr;

        const std::vector<SgAsmCilTypeDef*>& matches = found->second;
        if (matches.size() != 1)
          return nullptr;

        return matches.front();
      }

      SgAsmCilTypeDef* resolve(const SgAsmCilTypeRef* typeRef) const
      {
        // TODO: Support all ECMA-335 TypeRef ResolutionScope variants.  This
        // resolver currently accepts only AssemblyRef, so Module, ModuleRef,
        // and nested TypeRef scopes can silently omit base or interface edges.
        // PRIORITY: LOW
        //
        // TODO(CIL): Type forwarding is not followed. A reference assembly
        //   names the type in the facade assembly, while the definition lives
        //   elsewhere, so the {assembly, namespace, name} lookup misses it.
        //   \code{.cs}
        //   // compiled against System.Runtime.dll (facade); the type is
        //   // defined in System.Private.CoreLib.dll
        //   class MyList : System.Collections.Generic.List<int> {}
        //   \endcode
        // PRIORITY: LOW
        //
        // TODO(CIL): A nested TypeRef has another TypeRef as ResolutionScope.
        //   \code{.cs}
        //   class Derived : Lib.Outer.Inner {}  // TypeRef(Inner) scope = TypeRef(Outer)
        //   \endcode
        // PRIORITY: LOW
        const SgAsmCilMetadata* scope = typeRef->get_ResolutionScope_object();
        const SgAsmCilAssemblyRef* assemblyRef = isSgAsmCilAssemblyRef(scope);

        if (!assemblyRef)
          return nullptr;

        CilTypeIdentity id{assemblyName(assemblyRef),
                           cilString(typeRef->get_TypeNamespace_string()),
                           cilString(typeRef->get_TypeName_string())};

        return findUnique(id);
      }
  };

  // Helper functor to process a single MetadataRoot
  struct MetadataRootProcessor
  {
    const CilCompatibility& compat;
    ClassAnalysis& classes;

    void operator()(SgAsmCilMetadataRoot* metadataRoot) const
    {
      if (!metadataRoot) return;

      // Get the metadata heap which contains all the tables
      SgAsmCilMetadataHeap* metadataHeap = metadataRoot->get_MetadataHeap();
      if (!metadataHeap) return;

      // Get the TypeDef table which contains all type definitions
      SgAsmCilTypeDefTable* typeDefTable = metadataHeap->get_TypeDefTable();
      if (!typeDefTable) return;

      // Process each TypeDef in the table
      const std::vector<SgAsmCilTypeDef*>& typeDefs = typeDefTable->get_elements();

      for (size_t i = 0; i < typeDefs.size(); ++i)
      {
        SgAsmCilTypeDef* typeDef = typeDefs[i];
        if (!typeDef) continue;

        // The first TypeDef row is the pseudo type <Module> (II.22.37). It
        // only holds global methods and fields, which do not participate in
        // virtual dispatch. The row stays in typeDefs so that the MethodList
        // and FieldList ranges of the following rows keep their meaning.
        if (i == 0 && cilString(typeDef->get_TypeName_string()) == "<Module>")
          continue;

        ClassKeyType clKey = classToKey(typeDef);
        ClassData& classData = classes[clKey];

        // TODO(CIL): ClassData has no notion of sealed or value types, and CIL
        //   types nest. Only the abstract flag is transferred. (Interfaces
        //   are modeled as abstract virtual base classes, as in the JVM
        //   bridge; see inheritanceEdges.)
        //   \code{.cs}
        //   sealed class Leaf {}               // no descendants possible
        //   struct Point {}                    // value type, implicitly sealed
        //   class Outer { class Inner {} }     // NestedClass table is not read
        //   \endcode
        // PRIORITY: MEDIUM

        // Set abstract flag
        classData.abstractClass(compat.isAbstract(clKey));

        // Extract virtual functions and data members
        extractVirtualFunctions(typeDef, metadataHeap, typeDefs, i, classData);
        extractDataMembers(typeDef, metadataHeap, typeDefs, i, classData);
      }
    }
  };
}

// CIL signature services (ECMA-335 II.23.2)
//
// Signatures are blobs with module-local type tokens. The services below
// decode a method signature into canonical text in which every type token has
// been replaced by a module-independent type identity, so that signatures of
// different modules can be compared.

namespace
{
  /// bounds checked reader for signature blobs
  struct BlobReader
  {
    BlobReader(const std::vector<std::uint8_t>& b, std::size_t p)
    : blob(b), pos(p), limit(b.size()), good(p <= b.size())
    {}

    bool ok() const { return good; }

    std::uint8_t byte()
    {
      if (pos >= limit)
      {
        good = false;
        return 0;
      }

      return blob[pos++];
    }

    std::uint8_t peek()
    {
      if (pos >= limit)
      {
        good = false;
        return 0;
      }

      return blob[pos];
    }

    /// reads a compressed unsigned integer (II.23.2)
    std::uint32_t compressed()
    {
      const std::uint32_t b0 = byte();

      if ((b0 & 0x80) == 0)
        return b0;

      if ((b0 & 0xC0) == 0x80)
      {
        const std::uint32_t b1 = byte();

        return ((b0 & 0x3F) << 8) | b1;
      }

      if ((b0 & 0xE0) == 0xC0)
      {
        const std::uint32_t b1 = byte();
        const std::uint32_t b2 = byte();
        const std::uint32_t b3 = byte();

        return ((b0 & 0x1F) << 24) | (b1 << 16) | (b2 << 8) | b3;
      }

      good = false;
      return 0;
    }

    const std::vector<std::uint8_t>& blob;
    std::size_t                      pos;
    std::size_t                      limit;
    bool                             good;
  };

  /// per metadata root information needed to decode signatures
  struct ModuleInfo
  {
    SgAsmCilMetadataHeap*                                              heap = nullptr;
    const std::vector<std::uint8_t>*                                   blob = nullptr;
    std::string                                                        assembly;
    std::unordered_map<const SgAsmCilTypeDef*, const SgAsmCilTypeDef*> enclosing;
  };

  /// decoded method signature (canonical text, see decodeType)
  struct MethodSig
  {
    bool          valid        = false;
    std::uint32_t genericArity = 0;
    std::size_t   paramCount   = 0;
    std::string   params;      ///< calling convention, generic arity, and parameter types
    std::string   ret;         ///< return type including custom modifiers
    std::string   retCore;     ///< return type without leading custom modifiers
    std::string   retClassId;  ///< identity of the return type if it is a plain class type
  };
}

/// caches data that is derived from the CIL AST
struct CilMetadataCache
{
  std::unordered_map<const SgAsmCilMetadataRoot*, ModuleInfo> modules;
  std::unordered_map<const SgAsmCilMethodDef*, MethodSig>     signatures;
  std::map<std::string, SgAsmCilTypeDef*>                     typeById;  ///< nullptr if ambiguous
  std::set<std::string>                                       warnings;
  bool                                                        typeIndexBuilt = false;
};

namespace
{
  void
  warnOnce(CilMetadataCache& cache, const std::string& msg)
  {
    if (cache.warnings.insert(msg).second)
      msgWarn() << msg << std::endl;
  }

  ModuleInfo&
  moduleInfo(CilMetadataCache& cache, SgAsmCilMetadataRoot* root)
  {
    std::unordered_map<const SgAsmCilMetadataRoot*, ModuleInfo>::iterator pos = cache.modules.find(root);

    if (pos != cache.modules.end())
      return pos->second;

    ModuleInfo mi;

    mi.assembly = assemblyName(root);
    mi.heap     = root->get_MetadataHeap();

    if (SgAsmCilUint8Heap* blobHeap = root->get_BlobHeap())
      mi.blob = &blobHeap->get_Stream();

    // nested type -> enclosing type (NestedClass table, II.22.32)
    SgAsmCilNestedClassTable* nestedTable = mi.heap ? mi.heap->get_NestedClassTable() : nullptr;

    if (nestedTable)
      for (SgAsmCilNestedClass* nested : nestedTable->get_elements())
      {
        if (!nested) continue;

        const SgAsmCilTypeDef* nestedDef    = isSgAsmCilTypeDef(nested->get_NestedClass_object());
        const SgAsmCilTypeDef* enclosingDef = isSgAsmCilTypeDef(nested->get_EnclosingClass_object());

        if (nestedDef && enclosingDef)
          mi.enclosing[nestedDef] = enclosingDef;
      }

    return cache.modules.emplace(root, std::move(mi)).first->second;
  }

  /// the identity of a type defined in a module: [Assembly]Namespace.Name,
  /// nested types are Enclosing/Name.
  std::string
  typeDefIdentity(const ModuleInfo& mi, const SgAsmCilTypeDef* typeDef, int depth = 0)
  {
    const std::string name = cilString(typeDef->get_TypeName_string());

    std::unordered_map<const SgAsmCilTypeDef*, const SgAsmCilTypeDef*>::const_iterator outer = mi.enclosing.find(typeDef);

    if (outer != mi.enclosing.end() && depth < 32)
      return typeDefIdentity(mi, outer->second, depth + 1) + "/" + name;

    const std::string ns = cilString(typeDef->get_TypeNamespace_string());

    return "[" + mi.assembly + "]" + (ns.empty() ? std::string() : ns + ".") + name;
  }

  /// the identity of a referenced type, in the same format as typeDefIdentity.
  std::string
  typeRefIdentity(const ModuleInfo& mi, const SgAsmCilTypeRef* typeRef, int depth = 0)
  {
    const std::string       name  = cilString(typeRef->get_TypeName_string());
    const SgAsmCilMetadata* scope = typeRef->get_ResolutionScope_object();

    if (const SgAsmCilTypeRef* outer = isSgAsmCilTypeRef(scope))
      if (depth < 32)
        return typeRefIdentity(mi, outer, depth + 1) + "/" + name;

    // Module, ModuleRef, and a missing scope refer to the current assembly
    const SgAsmCilAssemblyRef* assemblyRef = isSgAsmCilAssemblyRef(scope);
    const std::string          ns          = cilString(typeRef->get_TypeNamespace_string());

    return "[" + (assemblyRef ? assemblyName(assemblyRef) : mi.assembly) + "]" + (ns.empty() ? std::string() : ns + ".") + name;
  }

  std::string decodeType(CilMetadataCache& cache, ModuleInfo& mi, BlobReader& rd, int depth);

  /// returns the identity of the type designated by a TypeDefOrRef coded index
  /// (II.23.2.8), or an empty string if it cannot be determined.
  std::string
  tokenIdentity(CilMetadataCache& cache, ModuleInfo& mi, std::uint32_t token, int depth)
  {
    const std::uint32_t tag = token & 0x3;
    const std::size_t   row = token >> 2;

    if (!mi.heap || row == 0 || depth > 32)
      return std::string();

    if (tag == 0)
    {
      SgAsmCilTypeDefTable* table = mi.heap->get_TypeDefTable();

      if (table && row <= table->get_elements().size() && table->get_elements()[row-1])
        return typeDefIdentity(mi, table->get_elements()[row-1]);
    }
    else if (tag == 1)
    {
      SgAsmCilTypeRefTable* table = mi.heap->get_TypeRefTable();

      if (table && row <= table->get_elements().size() && table->get_elements()[row-1])
        return typeRefIdentity(mi, table->get_elements()[row-1]);
    }
    else if (tag == 2 && mi.blob)
    {
      SgAsmCilTypeSpecTable* table = mi.heap->get_TypeSpecTable();

      if (table && row <= table->get_elements().size() && table->get_elements()[row-1])
      {
        BlobReader rd(*mi.blob, table->get_elements()[row-1]->get_Signature());
        const std::uint32_t len = rd.compressed();

        if (rd.pos + len > rd.blob.size())
          return std::string();

        rd.limit = rd.pos + len;

        const std::string res = decodeType(cache, mi, rd, depth + 1);

        return rd.ok() ? res : std::string();
      }
    }

    return std::string();
  }

  std::string
  classTokenText(CilMetadataCache& cache, ModuleInfo& mi, BlobReader& rd, int depth)
  {
    std::string id = tokenIdentity(cache, mi, rd.compressed(), depth + 1);

    if (id.empty())
    {
      rd.good = false;
      id = "?";
    }

    return id;
  }

  /// reads a custom modifier (after CMOD_REQD / CMOD_OPT) and returns its text.
  std::string
  modifierText(CilMetadataCache& cache, ModuleInfo& mi, BlobReader& rd, bool required, int depth)
  {
    return std::string(required ? "modreq(" : "modopt(") + classTokenText(cache, mi, rd, depth) + ") ";
  }

  bool parseMethodSig(CilMetadataCache& cache, ModuleInfo& mi, BlobReader& rd, MethodSig& sig, int depth);

  std::string
  joinTexts(const std::vector<std::string>& parts)
  {
    std::string res;

    for (const std::string& part : parts)
    {
      if (!res.empty()) res += ",";
      res += part;
    }

    return res;
  }

  /// decodes a Type (II.23.2.12) into canonical text.
  std::string
  decodeType(CilMetadataCache& cache, ModuleInfo& mi, BlobReader& rd, int depth)
  {
    if (depth > 32)
    {
      rd.good = false;
      return "?";
    }

    const std::uint8_t elementType = rd.byte();

    switch (elementType)
    {
      case 0x01: return "void";
      case 0x02: return "bool";
      case 0x03: return "char";
      case 0x04: return "int8";
      case 0x05: return "uint8";
      case 0x06: return "int16";
      case 0x07: return "uint16";
      case 0x08: return "int32";
      case 0x09: return "uint32";
      case 0x0A: return "int64";
      case 0x0B: return "uint64";
      case 0x0C: return "float32";
      case 0x0D: return "float64";
      case 0x0E: return "string";
      case 0x16: return "typedref";
      case 0x18: return "native int";
      case 0x19: return "native uint";
      case 0x1C: return "object";

      case 0x0F: return decodeType(cache, mi, rd, depth + 1) + "*";
      case 0x10: return decodeType(cache, mi, rd, depth + 1) + "&";
      case 0x1D: return decodeType(cache, mi, rd, depth + 1) + "[]";
      case 0x45: return "pinned " + decodeType(cache, mi, rd, depth + 1);

      case 0x11: // VALUETYPE
      case 0x12: // CLASS
        return classTokenText(cache, mi, rd, depth);

      case 0x13: return "!"  + std::to_string(rd.compressed());
      case 0x1E: return "!!" + std::to_string(rd.compressed());

      case 0x1F: // CMOD_REQD
      case 0x20: // CMOD_OPT
      {
        const std::string mod = modifierText(cache, mi, rd, elementType == 0x1F, depth);

        return mod + decodeType(cache, mi, rd, depth + 1);
      }

      case 0x14: // ARRAY
      {
        const std::string               elem = decodeType(cache, mi, rd, depth + 1);
        const std::uint32_t             rank = rd.compressed();
        const std::uint32_t             numSizes = rd.compressed();
        std::vector<std::string>        sizes;
        std::vector<std::string>        lowerBounds;

        for (std::uint32_t i = 0; i < numSizes && rd.ok(); ++i)
          sizes.push_back(std::to_string(rd.compressed()));

        const std::uint32_t numBounds = rd.compressed();

        for (std::uint32_t i = 0; i < numBounds && rd.ok(); ++i)
          lowerBounds.push_back(std::to_string(rd.compressed()));

        return elem + "[" + std::to_string(rank) + ":" + joinTexts(sizes) + ":" + joinTexts(lowerBounds) + "]";
      }

      case 0x15: // GENERICINST
      {
        rd.byte(); // CLASS or VALUETYPE

        const std::string        generic = classTokenText(cache, mi, rd, depth);
        const std::uint32_t      numArgs = rd.compressed();
        std::vector<std::string> args;

        for (std::uint32_t i = 0; i < numArgs && rd.ok(); ++i)
          args.push_back(decodeType(cache, mi, rd, depth + 1));

        return generic + "<" + joinTexts(args) + ">";
      }

      case 0x1B: // FNPTR
      {
        MethodSig inner;

        parseMethodSig(cache, mi, rd, inner, depth + 1);
        return "fnptr " + inner.params + "->" + inner.ret;
      }

      default:
        rd.good = false;
        return "?";
    }
  }

  /// decodes a MethodDefSig (II.23.2.1), positioned at the calling convention byte.
  bool
  parseMethodSig(CilMetadataCache& cache, ModuleInfo& mi, BlobReader& rd, MethodSig& sig, int depth)
  {
    const std::uint8_t callConv = rd.byte();
    const std::uint8_t kind     = callConv & 0x0F;

    // FIELD, LOCAL_SIG, PROPERTY, and GENERICINST (MethodSpec) are not method signatures
    if (kind == 0x06 || kind == 0x07 || kind == 0x08 || kind == 0x0A)
      return false;

    if (callConv & 0x10) // GENERIC
      sig.genericArity = rd.compressed();

    const std::uint32_t numParams = rd.compressed();

    // return type: custom modifiers, then the type
    std::string mods;

    while (rd.ok() && (rd.peek() == 0x1F || rd.peek() == 0x20))
    {
      const bool required = rd.byte() == 0x1F;

      mods += modifierText(cache, mi, rd, required, depth);
    }

    if (rd.ok() && rd.peek() == 0x12)
    {
      rd.byte();
      sig.retClassId = classTokenText(cache, mi, rd, depth);
      sig.retCore    = sig.retClassId;
    }
    else
    {
      sig.retCore = decodeType(cache, mi, rd, depth + 1);
    }

    sig.ret = mods + sig.retCore;

    // parameters
    std::vector<std::string> params;
    std::uint32_t            seen = 0;

    while (seen < numParams && rd.ok())
    {
      if (rd.peek() == 0x41) // SENTINEL
      {
        rd.byte();
        params.push_back("...");
        continue;
      }

      params.push_back(decodeType(cache, mi, rd, depth + 1));
      ++seen;
    }

    sig.paramCount = params.size();
    sig.params     = std::string(kind == 0x05 ? "vararg" : "")
                   + (callConv & 0x10 ? "<" + std::to_string(sig.genericArity) + ">" : std::string())
                   + "(" + joinTexts(params) + ")";

    return rd.ok();
  }

  /// returns the decoded signature of \p method (cached).
  const MethodSig&
  methodSignature(CilMetadataCache& cache, const SgAsmCilMethodDef* method)
  {
    std::unordered_map<const SgAsmCilMethodDef*, MethodSig>::iterator pos = cache.signatures.find(method);

    if (pos != cache.signatures.end())
      return pos->second;

    MethodSig sig;

    if (SgAsmCilMetadataRoot* root = getMetadataRootFromNode(const_cast<SgAsmCilMethodDef*>(method)))
    {
      ModuleInfo& mi = moduleInfo(cache, root);

      if (mi.blob)
      {
        BlobReader          rd(*mi.blob, method->get_Signature());
        const std::uint32_t len = rd.compressed(); // blob length prefix

        if (rd.ok() && rd.pos + len <= rd.blob.size())
        {
          rd.limit  = rd.pos + len;
          sig.valid = parseMethodSig(cache, mi, rd, sig, 0);
        }
      }
    }

    if (!sig.valid)
      warnOnce(cache, "CIL class hierarchy: cannot decode the signature of method "
                      + cilString(method->get_Name_string()) + "; override matching may be incomplete.");

    return cache.signatures.emplace(method, std::move(sig)).first->second;
  }

  void
  buildTypeIndex(CilMetadataCache& cache)
  {
    if (cache.typeIndexBuilt)
      return;

    cache.typeIndexBuilt = true;

    for (SgAsmCilMetadataRoot* root : loadedCilMetadataRoots())
    {
      ModuleInfo&           mi    = moduleInfo(cache, root);
      SgAsmCilTypeDefTable* table = mi.heap ? mi.heap->get_TypeDefTable() : nullptr;

      if (!table) continue;

      for (SgAsmCilTypeDef* typeDef : table->get_elements())
      {
        if (!typeDef) continue;

        std::pair<std::map<std::string, SgAsmCilTypeDef*>::iterator, bool> ins =
            cache.typeById.emplace(typeDefIdentity(mi, typeDef), typeDef);

        if (!ins.second)
          ins.first->second = nullptr; // ambiguous identity
      }
    }
  }

  /// returns the loaded type with identity \p id, or nullptr if there is none or it is ambiguous.
  SgAsmCilTypeDef*
  typeByIdentity(CilMetadataCache& cache, const std::string& id)
  {
    buildTypeIndex(cache);

    std::map<std::string, SgAsmCilTypeDef*>::const_iterator pos = cache.typeById.find(id);

    return pos != cache.typeById.end() ? pos->second : nullptr;
  }

  int
  compareText(const std::string& lhs, const std::string& rhs)
  {
    const int res = lhs.compare(rhs);

    return res < 0 ? -1 : (res > 0 ? 1 : 0);
  }
}

CilMetadataCache&
CilCompatibility::cache() const
{
  if (!cacheStore)
    cacheStore = std::make_shared<CilMetadataCache>();

  return *cacheStore;
}

void
CilCompatibility::resetCache() const
{
  cacheStore.reset();
}

// CilCompatibility implementation

std::string
CilCompatibility::nameOf(FunctionKeyType key) const
{
  // TODO(CIL): The name has neither declaring class nor signature, so
  //   overloads and explicit interface implementations print ambiguously in
  //   the text and JSON exports. Unnamed methods all compare equal in
  //   compareNames.
  //   \code{.cs}
  //   class C { public virtual void F(int i) {}  public virtual void F(string s) {} }
  //   // both are printed as "F"
  //   \endcode
  // PRIORITY: MEDIUM
  sg::NotNull<SgAsmCilMethodDef> method = keyToFunction(key);
  const uint8_t* nameBytes = method->get_Name_string();

  if (nameBytes)
    return std::string(reinterpret_cast<const char*>(nameBytes));

  return "<unnamed>";
}

std::string
CilCompatibility::nameOf(ClassKeyType key) const
{
  // TODO(CIL): The name is {namespace}.{type name} only. It lacks the
  //   enclosing type (NestedClass table) and the assembly, so nested types
  //   and same-named types from different assemblies are ambiguous.
  //   \code{.cs}
  //   namespace N { class A { public class Inner {} } class B { public class Inner {} } }
  //   // both nested types are printed as "Inner" (the namespace of a nested TypeDef is empty)
  //   \endcode
  // PRIORITY: MEDIUM
  sg::NotNull<SgAsmCilTypeDef> typeDef = keyToClass(key);

  // Get namespace and type name
  const uint8_t* namespaceBytes = typeDef->get_TypeNamespace_string();
  const uint8_t* typeNameBytes = typeDef->get_TypeName_string();

  std::string result;

  if (namespaceBytes)
  {
    result = std::string(reinterpret_cast<const char*>(namespaceBytes));
    if (!result.empty() && typeNameBytes)
      result += ".";
  }

  if (typeNameBytes)
    result += std::string(reinterpret_cast<const char*>(typeNameBytes));

  return result.empty() ? "<unnamed>" : result;
}

bool
CilCompatibility::isAbstract(ClassKeyType key) const
{
  sg::NotNull<SgAsmCilTypeDef> typeDef = keyToClass(key);
  uint32_t const flags = typeDef->get_Flags();
  bool const     isInterface = (flags & SgAsmCilTypeDef::FLG_INTERFACE) != 0;
  bool const     isAbstract  = (flags & SgAsmCilTypeDef::FLG_ABSTRACT) != 0;

  // A type is abstract if it has the Abstract flag or is an Interface
  //
  // Note: Interfaces are modeled as abstract classes (as in the JVM bridge),
  //   so a client cannot tell an interface from an abstract class through
  //   this query; it can through the virtual inheritance edge (inheritanceEdges).
  //
  // TODO(CIL): Static classes (abstract and sealed) are also reported as abstract.
  //   \code{.cs}
  //   interface IFoo { void F(); }               // FLG_INTERFACE | FLG_ABSTRACT
  //   abstract class Base { public abstract void F(); }  // FLG_ABSTRACT
  //   static class Util {}                       // FLG_ABSTRACT | FLG_SEALED
  //   \endcode
  // PRIORITY: HIGH
  return isInterface | isAbstract;
}

bool
CilCompatibility::isPureVirtual(FunctionKeyType key) const
{
  sg::NotNull<SgAsmCilMethodDef> method = keyToFunction(key);
  uint16_t const flags = method->get_Flags();
  bool const     isVirtual  = (flags & SgAsmCilMethodDef::FLG_VIRTUAL) != 0;
  bool const     isAbstract = (flags & SgAsmCilMethodDef::FLG_ABSTRACT) != 0;

  // A method is pure virtual if it's both Virtual and Abstract
  // Note: ECMA-335 requires Abstract methods to also have Virtual flag set,
  // so we must check both flags
  return isVirtual & isAbstract;
}

int
CilCompatibility::compareNames(FunctionKeyType lhs, FunctionKeyType rhs) const
{
  std::string lhsName = nameOf(lhs);
  std::string rhsName = nameOf(rhs);

  if (lhsName < rhsName) return -1;
  if (lhsName > rhsName) return 1;
  return 0;
}

int
CilCompatibility::compareFunctionsByType(FunctionKeyType lhs, FunctionKeyType rhs, bool exclReturnType) const
{
  // Signatures are compared by their canonical text (see methodSignature),
  // in which type tokens are replaced by module-independent type identities.
  // This makes methods of different modules comparable and distinguishes
  // generic arity and parameter types (including by-reference, arrays, and
  // custom modifiers).
  //
  // Note: The slot of a CIL virtual method is selected by its full signature,
  //   including the return type, and IL allows overloading by return type.
  //   The common analysis passes exclReturnType = true when it orders
  //   functions. This comparison honors the request; a pair whose return types
  //   differ must therefore be rejected where the slot rules are applied
  //   (canOverride). C# 9 covariant returns are expressed with a MethodImpl
  //   (.override) and are not found by name matching.
  //
  // TODO(CIL): Overrides under generic instantiation need substitution of
  //   the base type's type arguments, which a pairwise comparator without
  //   class context cannot do. Generic method arity and method type
  //   parameters (!!0) compare structurally and need no substitution.
  //   \code{.cs}
  //   class Base<T> { public virtual void Put(T t) {} }                // Put(!0)
  //   class IntBox : Base<int> { public override void Put(int t) {} }  // Put(int32)
  //   class Gen { public virtual void M<U>(U u) {} }                   // generic arity 1, M(!!0)
  //   \endcode
  // PRIORITY: LOW
  if (lhs == rhs) return 0;
  if (!isValidKey(lhs)) return -1;
  if (!isValidKey(rhs)) return 1;

  CilMetadataCache& metadata = cache();
  const MethodSig&  lhsSig   = methodSignature(metadata, keyToFunction(lhs));
  const MethodSig&  rhsSig   = methodSignature(metadata, keyToFunction(rhs));

  // undecodable signatures are ordered by key and precede all decodable ones
  if (!lhsSig.valid || !rhsSig.valid)
  {
    if (lhsSig.valid != rhsSig.valid)
      return lhsSig.valid ? 1 : -1;

    return lhs.key < rhs.key ? -1 : 1;
  }

  if (int res = compareText(lhsSig.params, rhsSig.params))
    return res;

  return exclReturnType ? 0 : compareText(lhsSig.ret, rhsSig.ret);
}

FunctionTypeRelation
CilCompatibility::haveSameOrCovariantReturn(const ClassAnalysis& classes, FunctionKeyType basId, FunctionKeyType drvId) const
{
  CilMetadataCache& metadata = cache();
  const MethodSig&  basSig   = methodSignature(metadata, keyToFunction(basId));
  const MethodSig&  drvSig   = methodSignature(metadata, keyToFunction(drvId));

  if (!basSig.valid || !drvSig.valid)
    return FunctionTypeRelation::unrelated;

  // identical return types (the custom modifiers may differ)
  if (basSig.retCore == drvSig.retCore)
    return FunctionTypeRelation::sametype;

  // Only class types are covariant; value types have an exact return type.
  // retClassId is empty unless the return type is a plain class type.
  if (drvSig.retClassId.empty())
    return FunctionTypeRelation::unrelated;

  // every class type is covariant to System.Object (ELEMENT_TYPE_OBJECT)
  if (basSig.retCore == "object")
    return FunctionTypeRelation::covariant;

  if (basSig.retClassId.empty())
    return FunctionTypeRelation::unrelated;

  SgAsmCilTypeDef* basType = typeByIdentity(metadata, basSig.retClassId);
  SgAsmCilTypeDef* drvType = typeByIdentity(metadata, drvSig.retClassId);

  if (!basType || !drvType)
    return FunctionTypeRelation::unrelated;

  // covariant if the derived return type has the base return type as ancestor
  const ClassKeyType basClassKey  = classToKey(basType);
  auto               drvClassIter = classes.find(classToKey(drvType));

  if (drvClassIter == classes.end())
    return FunctionTypeRelation::unrelated;

  for (const InheritanceDesc& ancestor : drvClassIter->second.ancestors())
    if (ancestor.getClass() == basClassKey)
      return FunctionTypeRelation::covariant;

  return FunctionTypeRelation::unrelated;
}

namespace
{
  bool
  isInterfaceClass(ClassKeyType cls)
  {
    return isValidKey(cls) && (keyToClass(cls)->get_Flags() & SgAsmCilTypeDef::FLG_INTERFACE) != 0;
  }

  bool
  inAssembly(CilMetadataCache& metadata, const SgAsmCilMethodDef* method, const std::string& assembly)
  {
    SgAsmCilMetadataRoot* root = getMetadataRootFromNode(const_cast<SgAsmCilMethodDef*>(method));

    return root && moduleInfo(metadata, root).assembly == assembly;
  }

  /// Returns true if a class between \p drvClass and \p basClass declares a
  /// virtual method with the signature of \p drv that starts a new slot or is
  /// final. Such a method blocks \p drv from overriding the method of \p basClass.
  bool
  blockedByIntermediateMethod( const CilCompatibility& bridge,
                               const ClassAnalysis& classes,
                               ClassKeyType drvClass,
                               FunctionKeyType drv,
                               ClassKeyType basClass
                             )
  {
    ClassAnalysis::const_iterator drvEntry = classes.find(drvClass);

    if (drvEntry == classes.end())
      return false;

    for (const InheritanceDesc& mid : drvEntry->second.ancestors())
    {
      const ClassKeyType midClass = mid.getClass();

      if (midClass == basClass || isInterfaceClass(midClass))
        continue;

      ClassAnalysis::const_iterator midEntry = classes.find(midClass);

      if (midEntry == classes.end())
        continue;

      // only classes that derive from basClass lie between drvClass and basClass
      const ClassData::AncestorContainer& midAncestors = midEntry->second.ancestors();
      auto                                isBase = [basClass](const InheritanceDesc& desc) -> bool
                                                   {
                                                     return desc.getClass() == basClass;
                                                   };

      if (std::find_if(midAncestors.begin(), midAncestors.end(), isBase) == midAncestors.end())
        continue;

      for (FunctionKeyType fn : midEntry->second.virtualFunctions())
      {
        if (bridge.compareNames(fn, drv) != 0 || bridge.compareFunctionsByType(fn, drv, false /* incl. return type */) != 0)
          continue;

        const std::uint16_t flags = keyToFunction(fn)->get_Flags();

        if (flags & (SgAsmCilMethodDef::FLG_NEW_SLOT | SgAsmCilMethodDef::FLG_FINAL))
          return true;
      }
    }

    return false;
  }
}

bool
CilCompatibility::canOverrideInContext( const ClassAnalysis& classes,
                                        ClassKeyType drvClass,
                                        FunctionKeyType drv,
                                        ClassKeyType basClass,
                                        FunctionKeyType bas
                                      ) const
{
  if (!isValidKey(drv) || !isValidKey(bas))
    return false;

  sg::NotNull<SgAsmCilMethodDef> drvMethod = keyToFunction(drv);
  sg::NotNull<SgAsmCilMethodDef> basMethod = keyToFunction(bas);
  const std::uint16_t            drvFlags  = drvMethod->get_Flags();
  const std::uint16_t            basFlags  = basMethod->get_Flags();
  const std::uint16_t            drvAccess = drvFlags & SgAsmCilMethodDef::FLG_MEMBERACCESS_MASK;
  const std::uint16_t            basAccess = basFlags & SgAsmCilMethodDef::FLG_MEMBERACCESS_MASK;
  CilMetadataCache&              metadata  = cache();

  // The slot of a virtual method is selected by its full signature (II.10.3.2),
  // so methods that differ in the return type do not match by name. Covariant
  // returns (C# 9) are connected through the MethodImpl table.
  const MethodSig& drvSig = methodSignature(metadata, drvMethod);
  const MethodSig& basSig = methodSignature(metadata, basMethod);

  if (drvSig.valid && basSig.valid && drvSig.ret != basSig.ret)
    return false;

  // private and compiler-controlled methods do not take part in name based
  // matching (explicit interface implementations are private and use MethodImpl).
  if (drvAccess == SgAsmCilMethodDef::FLG_PRIVATE || drvAccess == SgAsmCilMethodDef::FLG_COMPILER_CONTROLLED)
    return false;

  if (isInterfaceClass(basClass))
  {
    // An interface method that matches by name is implicitly implemented by a
    // public virtual method of a class. C# emits it as "virtual final newslot",
    // so neither flag may reject the pair. An interface method that matches
    // by name hides the base interface method (or overrides it through MethodImpl).
    if (isInterfaceClass(drvClass))
      return false;

    return drvAccess == SgAsmCilMethodDef::FLG_PUBLIC;
  }

  if (basAccess == SgAsmCilMethodDef::FLG_PRIVATE || basAccess == SgAsmCilMethodDef::FLG_COMPILER_CONTROLLED)
    return false;

  // final methods cannot be overridden, new-slot methods do not override
  if ((basFlags & SgAsmCilMethodDef::FLG_FINAL) || (drvFlags & SgAsmCilMethodDef::FLG_NEW_SLOT))
    return false;

  // II.10.3.3: an assembly (or family-and-assembly) method can be overridden
  // from another assembly only by a public method, and not at all if the base
  // method is marked strict.
  if (basAccess == SgAsmCilMethodDef::FLG_ASSEM || basAccess == SgAsmCilMethodDef::FLG_FAM_AND_ASSEM)
  {
    SgAsmCilMetadataRoot* basRoot = getMetadataRootFromNode(basMethod);

    if (basRoot && !inAssembly(metadata, drvMethod, moduleInfo(metadata, basRoot).assembly))
    {
      if ((basFlags & SgAsmCilMethodDef::FLG_STRICT) || drvAccess != SgAsmCilMethodDef::FLG_PUBLIC)
        return false;
    }
  }

  if (isValidKey(drvClass) && isValidKey(basClass) && blockedByIntermediateMethod(*this, classes, drvClass, drv, basClass))
    return false;

  return true;
}

void
CilCompatibility::extractFromMemoryPool(ClassAnalysis& classes) const
{
  msgTrace() << "Extracting CIL classes from memory pool" << std::endl;

  // the AST may have changed since the last extraction
  resetCache();

  // Process each metadata root
  MetadataRootProcessor processor{*this, classes};
  for (SgAsmCilMetadataRoot* metadataRoot : loadedCilMetadataRoots())
    processor(metadataRoot);
}

void
CilCompatibility::extractFromProject(ClassAnalysis& classes, ASTRootType root) const
{
  if (root.key == 0)
    return;

  SgNode* rootNode = keyToRoot(root);
  if (!rootNode)
    return;

  // the AST may have changed since the last extraction
  resetCache();

  // Query for all MetadataRoot nodes in the AST
  //
  // TODO(CIL): Classes are limited to this project, but inheritanceEdges
  //   resolves bases through the memory pool (see loadedCilMetadataRoots).
  // PRIORITY: LOW
  std::vector<SgNode*> metadataRootNodes = NodeQuery::querySubTree(rootNode, V_SgAsmCilMetadataRoot);

  MetadataRootProcessor processor{*this, classes};
  for (SgNode* node : metadataRootNodes)
  {
    SgAsmCilMetadataRoot* metadataRoot = isSgAsmCilMetadataRoot(node);
    processor(metadataRoot);
  }
}

LanguageSpecificFn
CilCompatibility::languageSpecificProcessing() const
{
  return [](ClassAnalysis&) {};
}

void
CilCompatibility::inheritanceEdges(ClassKeyType clkey, std::function<void(ClassKeyType, ClassKeyType, bool)> fn) const
{
  sg::NotNull<SgAsmCilTypeDef> typeDef = keyToClass(clkey);

  // TODO(CIL): A resolver (memory-pool traversal plus type index) is built
  //   on every call, i.e., once per class. Hoist it out of this function.
  // PRIORITY: LOW
  CilMetadataResolver resolver;

  // Get the base class (Extends field)
  const SgAsmCilMetadata* baseMetadata = typeDef->get_Extends_object();

  if (baseMetadata)
  {
    SgAsmCilTypeDef* baseTypeDef = resolver.resolve(baseMetadata);

    if (baseTypeDef)
    {
      ClassKeyType baseKey = classToKey(baseTypeDef);

      // Check if base is an interface (interfaces are virtually inherited in our mapping)
      //
      // Note: "virtual edge" means "interface edge", as in the JVM bridge.
      //   The common analysis also attaches C++ virtual-inheritance semantics
      //   to such edges (virtualBaseClassOrder, hasVirtualInheritance, and
      //   thus hasVirtualTable), which do not apply to CIL. Clients should
      //   ignore those properties. The implied-override computation relies on
      //   this mapping.
      uint32_t baseFlags = baseTypeDef->get_Flags();
      bool isVirtual = (baseFlags & SgAsmCilTypeDef::FLG_INTERFACE) != 0;

      fn(clkey, baseKey, isVirtual);
    }
  }

  // Handle implemented interfaces via InterfaceImpl table
  // In CIL, class-to-interface relationships are stored in a separate InterfaceImpl table
  // We need to find all InterfaceImpl entries where Class field matches this TypeDef
  //
  // TODO(CIL): Scanning the metadata tree per class is O(classes * tree).
  //   Iterate the InterfaceImpl table once and group the entries by class.
  // PRIORITY: LOW
  //
  // TODO(CIL): Interface re-implementation is not modeled. Listing an
  //   interface again in a derived class changes interface dispatch, but the
  //   duplicate edge is merged away by the common analysis.
  //   \code{.cs}
  //   interface IFoo { void F(); }
  //   class Base : IFoo { public virtual void F() {} }
  //   class Derived : Base, IFoo { void IFoo.F() {} }  // re-implements IFoo.F
  //   \endcode
  // PRIORITY: HIGH
  SgAsmCilMetadataRoot* metadataRoot = getMetadataRootFromNode(typeDef);

  if (metadataRoot)
  {
    // Query all InterfaceImpl nodes from the metadata root
    std::vector<SgNode*> interfaceImplNodes = NodeQuery::querySubTree(metadataRoot, V_SgAsmCilInterfaceImpl);

    for (SgNode* node : interfaceImplNodes)
    {
      SgAsmCilInterfaceImpl* interfaceImpl = isSgAsmCilInterfaceImpl(node);
      if (!interfaceImpl) continue;

      // Get the class that implements the interface
      const SgAsmCilMetadata* classMetadata = interfaceImpl->get_Class_object();

      // Check if this InterfaceImpl entry refers to our TypeDef
      if (classMetadata == static_cast<const SgAsmCilMetadata*>(&*typeDef))
      {
        // Get the interface being implemented
        const SgAsmCilMetadata* interfaceMetadata = interfaceImpl->get_Interface_object();

        if (interfaceMetadata)
        {
          SgAsmCilTypeDef* interfaceTypeDef = resolver.resolve(interfaceMetadata);

          if (interfaceTypeDef)
          {
            ClassKeyType interfaceKey = classToKey(interfaceTypeDef);

            // Interfaces are always virtually inherited in our C++ mapping
            fn(clkey, interfaceKey, true);
          }
        }
      }
    }
  }
}

DataMemberType
CilCompatibility::typeOf(VariableKeyType var) const
{
  sg::NotNull<SgAsmCilField> field = keyToVariable(var);

  // For now, return basic type information
  // A full implementation would parse the signature blob
  //
  // TODO(CIL): Only isConst is filled in, and it is derived from InitOnly
  //   (readonly), which is not C++ const. The JVM bridge reports false for
  //   final fields; decide on one convention for both (this bridge should
  //   follow unless the common model defines isConst as "assigned only in a
  //   constructor"). The class type, reference, array, and initializer
  //   information require decoding the field signature blob; the JVM bridge
  //   does this for descriptors (but resolves class names by substring, which
  //   should not be copied).
  //   \code{.cs}
  //   struct Point { int x; }
  //   class Shape {
  //     readonly int id;       // InitOnly => reported as const
  //     Point origin;          // value-type field: classType should be Point
  //     int[] values;          // hasArray should be true
  //     ref int alias;         // (ref struct only) hasReference should be true
  //   }
  //   \endcode
  // PRIORITY: MEDIUM
  uint16_t flags = field->get_Flags();
  bool isConst = (flags & SgAsmCilField::FLG_INIT_ONLY) != 0;

  return DataMemberType{ClassKeyType{0}, isConst, false, false, false};
}

}
