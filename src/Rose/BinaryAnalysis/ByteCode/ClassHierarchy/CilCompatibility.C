#include <sage3basic.h>
#include <Rose/BinaryAnalysis/ByteCode/ClassHierarchy/CilCompatibility.h>

#include <sageGeneric.h>

#include <map>
#include <string>
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

    // Only virtual instance methods participate in override matching.
    //
    // TODO(CIL): NewSlot is not honored.
    //   The common analysis matches virtual functions by name and signature
    //   against all ancestors, so a method that starts a new vtable slot is
    //   reported as an override. The hook for this is
    //   AbstractCompatibilityBridge::canOverride(drv, bas), which
    //   CilCompatibility does not implement yet. It must reject the pair when
    //   drv is NewSlot (it hides bas). Because canOverride sees only the two
    //   functions, it must also reject (Leaf.Name, Base.Name) when a NewSlot
    //   method of the same signature lies between the two classes; either walk
    //   the base chain in the bridge, or extend the hook to receive the
    //   ClassAnalysis. The transitive rule of the common analysis does not
    //   restore such pairs: Derived.Name does not override Base.Name.
    //   \code{.cs}
    //   class Base    { public virtual string Name() => "Base"; }
    //   class Derived : Base { public new virtual string Name() => "Derived"; }
    //   class Leaf    : Derived { public override string Name() => "Leaf"; }
    //   \endcode
    //   Derived.Name is FLG_VIRTUAL | FLG_NEW_SLOT and must not override
    //   Base.Name. Leaf.Name reuses the slot of Derived.Name and must
    //   override only Derived.Name, not Base.Name.
    // PRIORITY: MEDIUM
    //
    // TODO(CIL): FLG_FINAL is not recorded and not enforced.
    //   canOverride (see above) should reject a pair whose base method is
    //   final, again taking an intervening final method into account.
    //   C# also emits `virtual final newslot` for implicit interface
    //   implementations, which currently cannot be told apart from a plain
    //   virtual method. As in the JVM bridge (ACC_FINAL TODO), clients also
    //   have no way to learn that a method has no possible overriders.
    //   \code{.cs}
    //   interface IShape { double Area(); }
    //   class Square : IShape { public double Area() => 1.0; } // virtual final newslot
    //   class Base { public virtual void F() {} }
    //   class Mid : Base { public sealed override void F() {} } // final
    //   class Leaf : Mid { public new void F() {} }             // cannot override Mid.F
    //   \endcode
    // PRIORITY: HIGH
    //
    // TODO(CIL): Access modifiers (FLG_MEMBERACCESS_MASK, FLG_STRICT) are not considered.
    //   Implement them in canOverride, as the JVM bridge does for JVMS 5.4.5:
    //   private and compiler-controlled virtuals are never overridable by
    //   name (they are removed from the pair, as the JVM bridge removes
    //   private methods), and assembly / family-and-assembly virtuals are not
    //   overridable from another assembly (the derived method gets a new slot
    //   instead). The assembly identity takes the place of the JVM run-time
    //   package; InternalsVisibleTo is not modeled. The transitive rule of
    //   the common analysis applies as for the JVM.
    //   \code{.cs}
    //   // assembly A
    //   public class Base { internal virtual void F() {} }
    //   // assembly B (no InternalsVisibleTo)
    //   public class Derived : Base { internal virtual void F() {} } // new slot, not an override
    //   \endcode
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
    // PRIORITY: HIGH
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

        ClassKeyType clKey = classToKey(typeDef);
        ClassData& classData = classes[clKey];

        // TODO(CIL): The first TypeDef row is the pseudo type <Module>, which
        //   holds global methods and fields. It is currently registered as an
        //   ordinary class and should be skipped (or marked) here.
        // PRIORITY: HIGH
        //
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
  sg::NotNull<SgAsmCilMethodDef> lhsMethod = keyToFunction(lhs);
  sg::NotNull<SgAsmCilMethodDef> rhsMethod = keyToFunction(rhs);

  // TODO(CIL): Signature is a module-local offset into the blob heap, not a
  // canonical method type. Comparing offsets makes separately encoded but
  // equivalent signatures different, and can make unrelated signatures in
  // different metadata heaps compare equal. exclReturnType is therefore only
  // a placeholder and currently has no effect.
  //
  // To implement this correctly:
  //   1. Obtain each method's metadata root and blob heap.
  //   2. Decode the complete method signature, preserving the return type
  //      separately from calling convention, generic arity, and parameter
  //      types (including modifiers, arrays, by-reference, and generic types).
  //   3. Compare normalized parameter signatures when exclReturnType is true;
  //      compare the normalized return type as well when it is false.
  //   4. Resolve TypeDefOrRef identities across metadata heaps before comparing
  //      types, and provide a deterministic semantic ordering.
  //   5. Add fixtures for duplicate blobs, colliding offsets in separate
  //      modules, and covariant-return overrides.
  //
  // Until then, offset comparison is insufficient for override matching.
  // PRIORITY: HIGH
  //
  // TODO(CIL): Because the offset includes the return type, a covariant
  //   override never compares equal to its base method, so the override edge
  //   is lost. This is correct for slot matching: the CLR selects a slot by
  //   the full signature including the return type, and IL allows overloading
  //   by return type. Therefore, do not ignore the return type here. The JVM
  //   bridge currently does, but it carries a TODO to move to full-descriptor
  //   matching, because javac emits synthetic bridge methods that make the
  //   two compare equal. CIL has no bridge methods.
  //   C# 9 covariant returns are expressed with a MethodImpl (.override)
  //   and should be reported through an explicit-override hook (see the
  //   MethodImpl TODO in extractVirtualFunctions).
  //   \code{.cs}
  //   class Animal {}
  //   class Dog : Animal {}
  //   class Base    { public virtual Animal Make() => new Animal(); }
  //   class Derived : Base { public override Dog Make() => new Dog(); }  // C# 9 covariant return
  //   \endcode
  // PRIORITY: HIGH
  //
  // TODO(CIL): Offsets are module-local. A method in App.exe overriding a
  //   method of Lib.dll can never compare equal, and equal offsets from
  //   different modules compare equal for unrelated signatures. Compare a
  //   canonical signature string instead (as the JVM bridge does with
  //   descriptors), resolving type tokens to type identities.
  //   \code{.cs}
  //   // Lib.dll
  //   public class Base { public virtual void Run(int x) {} }
  //   // App.exe
  //   class Derived : Base { public override void Run(int x) {} }
  //   \endcode
  // PRIORITY: LOW
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
  uint32_t lhsSigIdx = lhsMethod->get_Signature();
  uint32_t rhsSigIdx = rhsMethod->get_Signature();

  if (exclReturnType)
  {
    // For now, use simple comparison
    // A full implementation would parse the signature blob and compare parameter types
    if (lhsSigIdx < rhsSigIdx) return -1;
    if (lhsSigIdx > rhsSigIdx) return 1;
    return 0;
  }
  else
  {
    // Compare including return type
    if (lhsSigIdx < rhsSigIdx) return -1;
    if (lhsSigIdx > rhsSigIdx) return 1;
    return 0;
  }
}

// Helper to parse return type from CIL method signature blob
// Returns the index into the blob where the return type starts, or 0 if parsing fails
static
uint32_t
parseReturnTypeFromSignature(const std::vector<uint8_t>& blobHeap, uint32_t sigIndex, std::vector<uint8_t>& returnType)
{
  returnType.clear();

  if (sigIndex >= blobHeap.size())
    return 0;

  // Read blob size (compressed integer at sigIndex)
  uint32_t idx = sigIndex;

  // Decompress blob size (ECMA-335 II.23.2) and skip past it
  if ((blobHeap[idx] & 0x80) == 0) {
    idx += 1;
  } else if ((blobHeap[idx] & 0xC0) == 0x80) {
    if (idx + 1 >= blobHeap.size()) return 0;
    idx += 2;
  } else if ((blobHeap[idx] & 0xE0) == 0xC0) {
    if (idx + 3 >= blobHeap.size()) return 0;
    idx += 4;
  } else {
    return 0;
  }

  if (idx >= blobHeap.size()) return 0;

  // Skip calling convention byte
  //
  // TODO(CIL): The calling convention byte is skipped without being
  //   interpreted. If its GENERIC bit (0x10) is set, a compressed generic
  //   parameter count precedes ParamCount, and the code below then reads the
  //   generic arity as the parameter count and the real ParamCount as the
  //   return type.
  //   \code{.cs}
  //   class C { public virtual T Id<T>(T t) => t; }  // blob: 0x30 <arity=1> <params=1> !!0 !!0
  //   \endcode
  // PRIORITY: HIGH
  idx += 1;

  if (idx >= blobHeap.size()) return 0;

  // Skip parameter count (compressed integer)
  if ((blobHeap[idx] & 0x80) == 0) {
    idx += 1;
  } else if ((blobHeap[idx] & 0xC0) == 0x80) {
    idx += 2;
  } else if ((blobHeap[idx] & 0xE0) == 0xC0) {
    idx += 4;
  } else {
    return 0;
  }

  // Now at return type
  // Read the type signature (simplified - just read the next few bytes as the type encoding)
  // A full implementation would parse the entire type signature recursively
  uint32_t returnTypeStart = idx;

  if (idx >= blobHeap.size()) return 0;

  // Read at least one byte for the element type
  //
  // TODO(CIL): Only the first byte (and the token for CLASS / VALUETYPE) is
  //   captured. The following return types therefore compare as identical
  //   although they differ: VAR / MVAR with different indices, SZARRAY,
  //   GENERICINST, PTR, and BYREF with different element types. Custom
  //   modifiers (CMOD_REQD / CMOD_OPT) and BYREF prefixes are not skipped, so
  //   the element type is misread. The compressed-integer decoding above also
  //   lacks bounds checks.
  //   \code{.cs}
  //   class A { public virtual int[]          F() => null; }
  //   class B : A { public override int[]     F() => null; }  // same
  //   class C { public virtual string[]       F() => null; }  // different, but starts with SZARRAY like A.F
  //   class D { public virtual ref int        R() => ref x;  int x; }  // BYREF prefix
  //   \endcode
  // PRIORITY: LOW
  returnType.push_back(blobHeap[idx]);
  idx += 1;

  // For class/valuetype types, also read the TypeDefOrRef token
  if ((returnType[0] == 0x11 || returnType[0] == 0x12) && idx < blobHeap.size()) {
    // Read compressed TypeDefOrRef token
    if ((blobHeap[idx] & 0x80) == 0) {
      returnType.push_back(blobHeap[idx]);
      idx += 1;
    } else if ((blobHeap[idx] & 0xC0) == 0x80 && idx + 1 < blobHeap.size()) {
      returnType.push_back(blobHeap[idx]);
      returnType.push_back(blobHeap[idx + 1]);
      idx += 2;
    } else if ((blobHeap[idx] & 0xE0) == 0xC0 && idx + 3 < blobHeap.size()) {
      returnType.push_back(blobHeap[idx]);
      returnType.push_back(blobHeap[idx + 1]);
      returnType.push_back(blobHeap[idx + 2]);
      returnType.push_back(blobHeap[idx + 3]);
      idx += 4;
    }
  }

  return returnTypeStart;
}

// Helper to extract TypeDef/TypeRef from return type encoding
static
uint32_t
getTypeTokenFromReturnType(const std::vector<uint8_t>& returnType)
{
  if (returnType.empty())
    return 0;

  uint8_t elementType = returnType[0];

  // 0x11 = ELEMENT_TYPE_VALUETYPE, 0x12 = ELEMENT_TYPE_CLASS
  if ((elementType == 0x11 || elementType == 0x12) && returnType.size() > 1) {
    // Decompress token
    uint32_t token = 0;
    size_t idx = 1;

    if ((returnType[idx] & 0x80) == 0) {
      token = returnType[idx];
    } else if ((returnType[idx] & 0xC0) == 0x80 && idx + 1 < returnType.size()) {
      token = ((returnType[idx] & 0x3F) << 8) | returnType[idx + 1];
    } else if ((returnType[idx] & 0xE0) == 0xC0 && idx + 3 < returnType.size()) {
      token = ((returnType[idx] & 0x1F) << 24) | (returnType[idx + 1] << 16) |
              (returnType[idx + 2] << 8) | returnType[idx + 3];
    }

    return token;
  }

  return 0;
}

FunctionTypeRelation
CilCompatibility::haveSameOrCovariantReturn(const ClassAnalysis& classes, FunctionKeyType basId, FunctionKeyType drvId) const
{
  sg::NotNull<SgAsmCilMethodDef> basMethod = keyToFunction(basId);
  sg::NotNull<SgAsmCilMethodDef> drvMethod = keyToFunction(drvId);

  // Get metadata root to access blob heap
  SgAsmCilMetadataRoot* basRoot = getMetadataRootFromNode(basMethod);
  SgAsmCilMetadataRoot* drvRoot = getMetadataRootFromNode(drvMethod);

  if (!basRoot || !drvRoot)
    return FunctionTypeRelation::unrelated;

  // Get blob heaps
  SgAsmCilUint8Heap* basBlobHeap = basRoot->get_BlobHeap();
  SgAsmCilUint8Heap* drvBlobHeap = drvRoot->get_BlobHeap();

  if (!basBlobHeap || !drvBlobHeap)
    return FunctionTypeRelation::unrelated;

  const std::vector<uint8_t>& basBlob = basBlobHeap->get_Stream();
  const std::vector<uint8_t>& drvBlob = drvBlobHeap->get_Stream();

  // Get signature indices
  uint32_t basSigIdx = basMethod->get_Signature();
  uint32_t drvSigIdx = drvMethod->get_Signature();

  // Parse return types from signatures
  std::vector<uint8_t> basReturnType;
  std::vector<uint8_t> drvReturnType;

  if (!parseReturnTypeFromSignature(basBlob, basSigIdx, basReturnType) ||
      !parseReturnTypeFromSignature(drvBlob, drvSigIdx, drvReturnType)) {
    return FunctionTypeRelation::unrelated;
  }

  // If return types are identical, they have the same type
  //
  // TODO(CIL): The byte-wise comparison is wrong across modules. A
  //   TypeDefOrRef token is module-local, so equal bytes can denote different
  //   types, and different bytes (TypeRef vs. TypeDef) can denote the same
  //   type. Tokens must be resolved to classes before comparing.
  // PRIORITY: LOW
  if (basReturnType == drvReturnType)
    return FunctionTypeRelation::sametype;

  // Check for covariance: derived return type must be derived from base return type
  // This requires both to be reference types (CLASS)
  if (basReturnType.empty() || drvReturnType.empty())
    return FunctionTypeRelation::unrelated;

  // Both must be class types for covariance
  if (basReturnType[0] != 0x12 || drvReturnType[0] != 0x12)
    return FunctionTypeRelation::unrelated;

  // Extract type tokens (TypeDefOrRef coded indices)
  uint32_t basTypeToken = getTypeTokenFromReturnType(basReturnType);
  uint32_t drvTypeToken = getTypeTokenFromReturnType(drvReturnType);

  if (basTypeToken == 0 || drvTypeToken == 0)
    return FunctionTypeRelation::unrelated;

  // Decode TypeDefOrRef tokens and find corresponding TypeDef nodes
  // TypeDefOrRef encoding (ECMA-335 II.24.2.6):
  //   Lower 2 bits: 00=TypeDef, 01=TypeRef, 10=TypeSpec
  //   Upper bits: row index
  SgAsmCilTypeDef* basTypeDef = nullptr;
  SgAsmCilTypeDef* drvTypeDef = nullptr;

  // Get metadata heap to access TypeDef/TypeRef tables
  SgAsmCilMetadataHeap* basMetadataHeap = basRoot->get_MetadataHeap();
  SgAsmCilMetadataHeap* drvMetadataHeap = drvRoot->get_MetadataHeap();

  if (basMetadataHeap && drvMetadataHeap)
  {
    // Decode base type token
    uint32_t basTableKind = basTypeToken & 0x3;
    uint32_t basRowIndex = basTypeToken >> 2;

    if (basTableKind == 0) // TypeDef
    {
      // Query TypeDef table for the row
      SgAsmCilTypeDefTable* typeDefTable = basMetadataHeap->get_TypeDefTable();
      if (typeDefTable && basRowIndex > 0)
      {
        const std::vector<SgAsmCilTypeDef*>& typeDefs = typeDefTable->get_elements();
        // Row indices are 1-based in CIL
        if (basRowIndex <= typeDefs.size())
          basTypeDef = typeDefs[basRowIndex - 1];
      }
    }
    // Note: TypeRef resolution would require loading external assemblies
    // For now, we only handle TypeDef (types in the current assembly)
    //
    // TODO(CIL): TypeRef and TypeSpec return types are not resolved, and
    //   covariance is only recognized for class types. Reuse
    //   CilMetadataResolver here once it handles TypeSpec and cross-module
    //   references. The caller reports "covariant return assertion failed"
    //   and drops the override edge when this returns unrelated.
    //   \code{.cs}
    //   // Lib.dll: public class Animal {}
    //   // App.exe
    //   class Dog : Lib.Animal {}
    //   class Base    { public virtual Lib.Animal Make() => null; }  // TypeRef return type
    //   class Derived : Base { public override Dog Make() => null; } // TypeDef return type
    //   \endcode
    // PRIORITY: LOW

    // Decode derived type token
    uint32_t drvTableKind = drvTypeToken & 0x3;
    uint32_t drvRowIndex = drvTypeToken >> 2;

    if (drvTableKind == 0) // TypeDef
    {
      SgAsmCilTypeDefTable* typeDefTable = drvMetadataHeap->get_TypeDefTable();
      if (typeDefTable && drvRowIndex > 0)
      {
        const std::vector<SgAsmCilTypeDef*>& typeDefs = typeDefTable->get_elements();
        if (drvRowIndex <= typeDefs.size())
          drvTypeDef = typeDefs[drvRowIndex - 1];
      }
    }
  }

  // If we couldn't resolve both types, we can't determine covariance
  if (!basTypeDef || !drvTypeDef)
    return FunctionTypeRelation::unrelated;

  // Map TypeDef nodes to ClassKeyType
  ClassKeyType basClassKey = classToKey(basTypeDef);
  ClassKeyType drvClassKey = classToKey(drvTypeDef);

  // Check if derived class inherits from base class
  // Use the ClassAnalysis to check inheritance relationship
  auto drvClassIter = classes.find(drvClassKey);
  if (drvClassIter == classes.end())
    return FunctionTypeRelation::unrelated;

  const ClassData& drvClassData = drvClassIter->second;

  // Check if basClassKey is in the ancestor list of drvClassKey
  for (const InheritanceDesc& ancestor : drvClassData.ancestors())
  {
    if (std::get<0>(ancestor) == basClassKey)
    {
      // Found covariant return type: drvType inherits from basType
      return FunctionTypeRelation::covariant;
    }
  }

  // Types are different and not in inheritance relationship
  return FunctionTypeRelation::unrelated;
}

void
CilCompatibility::extractFromMemoryPool(ClassAnalysis& classes) const
{
  msgTrace() << "Extracting CIL classes from memory pool" << std::endl;

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
