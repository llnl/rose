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
    // C# test case for the remaining NewSlot handling:
    //   class Base { public virtual string Name() => "Base"; }
    //   class Derived : Base { public new virtual string Name() => "Derived"; }
    //
    // Derived.Name is FLG_VIRTUAL | FLG_NEW_SLOT and must not override
    // Base.Name. Handling NewSlot (and final virtual methods) awaits a common
    // hierarchy descriptor that preserves those CIL dispatch attributes.
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
  sg::NotNull<SgAsmCilMethodDef> method = keyToFunction(key);
  const uint8_t* nameBytes = method->get_Name_string();

  if (nameBytes)
    return std::string(reinterpret_cast<const char*>(nameBytes));

  return "<unnamed>";
}

std::string
CilCompatibility::nameOf(ClassKeyType key) const
{
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
      uint32_t baseFlags = baseTypeDef->get_Flags();
      bool isVirtual = (baseFlags & SgAsmCilTypeDef::FLG_INTERFACE) != 0;

      fn(clkey, baseKey, isVirtual);
    }
  }

  // Handle implemented interfaces via InterfaceImpl table
  // In CIL, class-to-interface relationships are stored in a separate InterfaceImpl table
  // We need to find all InterfaceImpl entries where Class field matches this TypeDef
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
  uint16_t flags = field->get_Flags();
  bool isConst = (flags & SgAsmCilField::FLG_INIT_ONLY) != 0;

  return DataMemberType{ClassKeyType{0}, isConst, false, false, false};
}

}
