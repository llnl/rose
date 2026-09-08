#include <sage3basic.h>
#include <Rose/BinaryAnalysis/ByteCode/ClassHierarchy/JvmCompatibility.h>

#include <sageGeneric.h>

#include <string>

#include <Rose/Common/Analysis/ClassHierarchyAnalysis.h>

namespace Rose::BinaryAnalysis::ByteCode::ClassHierarchy
{

using namespace Rose::Common::Analysis;

// Helper functions to convert keys to ROSE IR node pointers
inline
sg::NotNull<SgAsmJvmFileHeader>
keyToClass(ClassKeyType key)
{
  return reinterpret_cast<SgAsmJvmFileHeader*>(key.key);
}

inline
ClassKeyType
classToKey(sg::NotNull<SgAsmJvmFileHeader> n)
{
  return ClassKeyType{reinterpret_cast<std::uintptr_t>(&*n)};
}

inline
sg::NotNull<SgAsmJvmMethod>
keyToFunction(FunctionKeyType key)
{
  return reinterpret_cast<SgAsmJvmMethod*>(key.key);
}

inline
FunctionKeyType
functionToKey(sg::NotNull<SgAsmJvmMethod> n)
{
  return FunctionKeyType{reinterpret_cast<std::uintptr_t>(&*n)};
}

inline
sg::NotNull<SgAsmJvmField>
keyToVariable(VariableKeyType key)
{
  return reinterpret_cast<SgAsmJvmField*>(key.key);
}

inline
VariableKeyType
variableToKey(sg::NotNull<SgAsmJvmField> n)
{
  return VariableKeyType{reinterpret_cast<std::uintptr_t>(&*n)};
}

// Helper function to get file header from method by traversing parent chain
// Method -> MethodTable -> FileHeader
inline
SgAsmJvmFileHeader*
getFileHeaderFromMethod(SgAsmJvmMethod* method)
{
  if (!method) return nullptr;

  SgNode* parent = method->get_parent();
  if (!parent) return nullptr;

  SgNode* grandparent = parent->get_parent();
  if (!grandparent) return nullptr;

  return isSgAsmJvmFileHeader(grandparent);
}

// Helper function to get file header from field by traversing parent chain
// Field -> FieldTable -> FileHeader
inline
SgAsmJvmFileHeader*
getFileHeaderFromField(SgAsmJvmField* field)
{
  if (!field) return nullptr;

  SgNode* parent = field->get_parent();
  if (!parent) return nullptr;

  SgNode* grandparent = parent->get_parent();
  if (!grandparent) return nullptr;

  return isSgAsmJvmFileHeader(grandparent);
}

inline
sg::NotNull<SgProject>
keyToRoot(ASTRootType key)
{
  return reinterpret_cast<SgProject*>(key.key);
}

inline
ASTRootType
rootToKey(sg::NotNull<SgProject> n)
{
  return ASTRootType{reinterpret_cast<std::uintptr_t>(&*n)};
}

std::string
to_string(sg::NotNull<SgAsmJvmConstantPoolEntry> entry)
{
  ASSERT_require(entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Utf8);

  return std::string(entry->get_utf8_bytes(), entry->get_length());
}

// nameOf - returns the source name of the function represented by key
std::string
JvmCompatibility::nameOf(FunctionKeyType key) const
{
  if (!isValidKey(key)) throw std::runtime_error{"invalid function key"};

  sg::NotNull<SgAsmJvmMethod>       method = keyToFunction(key);

  // Get the file header by traversing parent chain
  SgAsmJvmFileHeader* fileHeader = getFileHeaderFromMethod(method);
  if (!fileHeader) throw std::runtime_error{"cannot find file header for method"};

  sg::NotNull<SgAsmJvmConstantPool> pool = fileHeader->get_constant_pool();

  // Get method name from constant pool
  std::uint16_t const               nameIndex  = method->get_name_index();
  if (nameIndex == 0) return "<unknown>";

  SgAsmJvmConstantPoolEntry* entry = pool->get_entry(nameIndex);
  if (!entry) return "<invalid>";

  if (entry->get_tag() != SgAsmJvmConstantPoolEntry::CONSTANT_Utf8)
    return "<non-utf8>";

  return to_string(entry);
}

// nameOf - returns the source name of the class represented by key
std::string JvmCompatibility::nameOf(ClassKeyType key) const
{
  if (!isValidKey(key)) throw std::runtime_error{"invalid class key"};

  sg::NotNull<SgAsmJvmFileHeader>        fileHeader = keyToClass(key);
  sg::NotNull<SgAsmJvmConstantPool>      pool       = fileHeader->get_constant_pool();
  uint16_t const                         classIndex = fileHeader->get_this_class();
  ASSERT_require(classIndex > 0);

  sg::NotNull<SgAsmJvmConstantPoolEntry> classEntry = pool->get_entry(classIndex);
  ASSERT_require(SgAsmJvmConstantPoolEntry::CONSTANT_Class == classEntry->get_tag());

  uint16_t const                         nameIndex = classEntry->get_name_index();
  ASSERT_require(nameIndex > 0);
  return to_string(pool->get_entry(nameIndex));
}

// isAbstract - returns true if the class represented by key is an abstract class
bool JvmCompatibility::isAbstract(ClassKeyType key) const
{
  if (!isValidKey(key)) throw std::runtime_error{"invalid class key"};

  sg::NotNull<SgAsmJvmFileHeader> fileHeader = keyToClass(key);

  // Check if class has ACC_ABSTRACT flag (0x0400)
  return (fileHeader->get_access_flags() & 0x0400) != 0;
}

// isPureVirtual - returns true if the function is abstract (pure virtual in C++ terms)
bool JvmCompatibility::isPureVirtual(FunctionKeyType key) const
{
  if (!isValidKey(key)) throw std::runtime_error{"invalid function key"};

  sg::NotNull<SgAsmJvmMethod> method = keyToFunction(key);

  // Check if method has ACC_ABSTRACT flag (0x0400)
  return (method->get_access_flags() & 0x0400) != 0;
}

// compareNames - compares the source names of two functions
int JvmCompatibility::compareNames(FunctionKeyType lhs, FunctionKeyType rhs) const
{
  std::string lhsName = nameOf(lhs);
  std::string rhsName = nameOf(rhs);

  if (lhsName < rhsName) return -1;
  if (lhsName > rhsName) return 1;
  return 0;
}

// compareFunctionsByType - compares the canonical types of functions
int JvmCompatibility::compareFunctionsByType(FunctionKeyType lhs, FunctionKeyType rhs, bool exclReturnType) const
{
  if (lhs == rhs) return 0;
  if (!isValidKey(lhs)) return -1;
  if (!isValidKey(rhs)) return 1;

  sg::NotNull<SgAsmJvmMethod>       lhsMethod = keyToFunction(lhs);
  sg::NotNull<SgAsmJvmMethod>       rhsMethod = keyToFunction(rhs);
  SgAsmJvmFileHeader*               lhsFileHeader = getFileHeaderFromMethod(lhsMethod);
  SgAsmJvmFileHeader*               rhsFileHeader = getFileHeaderFromMethod(rhsMethod);
  if (!lhsFileHeader || !rhsFileHeader) return lhs.key < rhs.key ? -1 : 1;
  sg::NotNull<SgAsmJvmConstantPool> lhsPool = lhsFileHeader->get_constant_pool();
  sg::NotNull<SgAsmJvmConstantPool> rhsPool = rhsFileHeader->get_constant_pool();
  uint16_t const                    lhsDescIndex = lhsMethod->get_descriptor_index();
  uint16_t const                    rhsDescIndex = rhsMethod->get_descriptor_index();

  SgAsmJvmConstantPoolEntry* lhsDescEntry = lhsPool->get_entry(lhsDescIndex);
  SgAsmJvmConstantPoolEntry* rhsDescEntry = rhsPool->get_entry(rhsDescIndex);
  if (!lhsDescEntry || !rhsDescEntry) return lhs.key < rhs.key ? -1 : 1;
  if (lhsDescEntry->get_tag() != SgAsmJvmConstantPoolEntry::CONSTANT_Utf8 ||
      rhsDescEntry->get_tag() != SgAsmJvmConstantPoolEntry::CONSTANT_Utf8)
    return lhs.key < rhs.key ? -1 : 1;

  std::string                       lhsDesc = to_string(lhsDescEntry);
  std::string                       rhsDesc = to_string(rhsDescEntry);

  // exclReturnType compares the parameter portion of a JVM method descriptor.
  // For example:
  //
  //   class Base { Object f(int n) { return null; } } // (I)Ljava/lang/Object;
  //   class Derived extends Base {
  //     @Override String f(int n) { return null; }    // (I)Ljava/lang/String;: override
  //     Object f() { return null; }                    // ()Ljava/lang/Object;: overload
  //   }
  //
  // Java source-level overriding ignores the return type, so the first two
  // declarations have equal parameter types and their return relationship
  // must be checked separately.  The latter two have different parameters
  // and must not match.  javac normally emits a synthetic bridge with
  // descriptor (I)Ljava/lang/Object; for the covariant override; its bytecode
  // targets f:(I)Ljava/lang/String;, so it does not recurse.  JVM method
  // resolution distinguishes those full descriptors, even though this
  // source-level hierarchy analysis deliberately excludes the return type.
  if (exclReturnType)
  {
    // Retain only parameter types, including the closing delimiter.
    std::size_t const lhsEnd = lhsDesc.find(')');
    std::size_t const rhsEnd = rhsDesc.find(')');

    if (lhsEnd != std::string::npos && rhsEnd != std::string::npos)
    {
      lhsDesc = lhsDesc.substr(0, lhsEnd + 1);
      rhsDesc = rhsDesc.substr(0, rhsEnd + 1);
    }
  }

  if (lhsDesc < rhsDesc) return -1;
  if (lhsDesc > rhsDesc) return 1;
  return 0;
}

// haveSameOrCovariantReturn - tests if drvId has the same or covariant return type with respect to basId
FunctionTypeRelation
JvmCompatibility::haveSameOrCovariantReturn(const ClassAnalysis& classes, FunctionKeyType basId, FunctionKeyType drvId) const
{
  if (!isValidKey(basId) || !isValidKey(drvId))
    return FunctionTypeRelation::unrelated;

  sg::NotNull<SgAsmJvmMethod>       basMethod = keyToFunction(basId);
  sg::NotNull<SgAsmJvmMethod>       drvMethod = keyToFunction(drvId);
  SgAsmJvmFileHeader*               basFileHeader = getFileHeaderFromMethod(basMethod);
  SgAsmJvmFileHeader*               drvFileHeader = getFileHeaderFromMethod(drvMethod);
  if (!basFileHeader || !drvFileHeader) return FunctionTypeRelation::unrelated;
  sg::NotNull<SgAsmJvmConstantPool> basPool = basFileHeader->get_constant_pool();
  sg::NotNull<SgAsmJvmConstantPool> drvPool = drvFileHeader->get_constant_pool();
  std::uint16_t const               basDescIndex = basMethod->get_descriptor_index();
  std::uint16_t const               drvDescIndex = drvMethod->get_descriptor_index();

  SgAsmJvmConstantPoolEntry* basDescEntry = basPool->get_entry(basDescIndex);
  SgAsmJvmConstantPoolEntry* drvDescEntry = drvPool->get_entry(drvDescIndex);
  if (!basDescEntry || !drvDescEntry) return FunctionTypeRelation::unrelated;
  if (basDescEntry->get_tag() != SgAsmJvmConstantPoolEntry::CONSTANT_Utf8 ||
      drvDescEntry->get_tag() != SgAsmJvmConstantPoolEntry::CONSTANT_Utf8)
    return FunctionTypeRelation::unrelated;

  std::string const                 basDesc = to_string(basDescEntry);
  std::string const                 drvDesc = to_string(drvDescEntry);

  // Extract return types from descriptors (after ')')
  std::size_t const                 basRetPos = basDesc.find(')');
  std::size_t const                 drvRetPos = drvDesc.find(')');

  if (basRetPos == std::string::npos || drvRetPos == std::string::npos)
    return FunctionTypeRelation::unrelated;

  std::string const                 basRetType = basDesc.substr(basRetPos + 1);
  std::string const                 drvRetType = drvDesc.substr(drvRetPos + 1);

  // Same return type
  if (basRetType == drvRetType)
    return FunctionTypeRelation::sametype;

  // Check for covariant return: both must be reference types (start with 'L')
  if (basRetType.empty() || drvRetType.empty())
    return FunctionTypeRelation::unrelated;

  if (basRetType[0] != 'L' || drvRetType[0] != 'L')
    return FunctionTypeRelation::unrelated;

  // Extract class names from descriptors (format: Lpackage/ClassName;)
  std::string const basClassName = basRetType.substr(1, basRetType.find(';') - 1);
  std::string const drvClassName = drvRetType.substr(1, drvRetType.find(';') - 1);

  // Convert to ClassKeyType and check inheritance relationship
  ClassKeyType basClass{0};
  ClassKeyType drvClass{0};

  for (const auto& entry : classes) {
    std::string className = nameOf(entry.first);
    // Replace / with . for comparison or match directly
    if (className == basClassName || className.find(basClassName) != std::string::npos)
      basClass = entry.first;

    if (className == drvClassName || className.find(drvClassName) != std::string::npos)
      drvClass = entry.first;
  }

  if (!isValidKey(basClass) || !isValidKey(drvClass))
    return FunctionTypeRelation::unrelated;

  // Check if drvClass is a subclass of basClass
  if (classes.isBaseOf(basClass, drvClass))
    return FunctionTypeRelation::covariant;

  return FunctionTypeRelation::unrelated;
}

// extractVirtualFunctions - extracts non-static member functions from a JVM file header
// and adds them to the class's virtual function list
static
void extractVirtualFunctions(SgAsmJvmFileHeader* jvmFileHeader, ClassData& classData)
{
  // Get the method table
  SgAsmJvmMethodTable* methodTable = jvmFileHeader->get_method_table();
  if (!methodTable) return;

  // Get the methods from the method table
  const SgAsmJvmMethodPtrList& methods = methodTable->get_methods();

  // Get constant pool for checking method names
  SgAsmJvmConstantPool* pool = jvmFileHeader->get_constant_pool();
  if (!pool) return;

  for (SgAsmJvmMethod* method : methods)
  {
    if (!method) continue;

    // Check if method is virtual (non-static, non-private, non-final)
    std::uint16_t access = method->get_access_flags();
    bool isStatic = (access & 0x0008) != 0;

    // TODO: Exclude ACC_PRIVATE methods from override analysis.  A private
    // method is not inherited and cannot be overridden, but Java permits a
    // subclass to declare an unrelated method with the same signature:
    //
    //   class Base { private Object f(int n) { return null; } }
    //   class Derived extends Base {
    //     Object f(int n) { return null; } // not an override
    //   }
    //
    // Both class files contain f:(I)Ljava/lang/Object;, so including both
    // methods lets the common hierarchy analysis falsely pair them.
    if (isStatic) continue;

    // Skip constructors and static initializers (they are not virtual)
    std::uint16_t nameIndex = method->get_name_index();
    if (nameIndex > 0)
    {
      SgAsmJvmConstantPoolEntry* nameEntry = pool->get_entry(nameIndex);
      if (nameEntry && nameEntry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Utf8)
      {
        std::string methodName = to_string(nameEntry);
        if (methodName == "<init>" || methodName == "<clinit>")
          continue;
      }
    }

    FunctionKeyType funcKey = functionToKey(method);
    classData.virtualFunctions().push_back(funcKey);
  }
}

// extractDataMembers - extracts non-static fields from a JVM file header
// and adds them to the class's data member list
static
void extractDataMembers(SgAsmJvmFileHeader* jvmFileHeader, ClassData& classData)
{
  // Get the field table
  SgAsmJvmFieldTable* fieldTable = jvmFileHeader->get_field_table();
  if (!fieldTable) return;

  // Get the fields from the field table
  const SgAsmJvmFieldPtrList& fields = fieldTable->get_fields();

  for (SgAsmJvmField* field : fields)
  {
    if (!field) continue;

    // Only add non-static fields as data members
    std::uint16_t access = field->get_access_flags();
    bool isStatic = (access & 0x0008) != 0;

    if (!isStatic)
      classData.dataMembers().push_back(variableToKey(field));
  }
}

// extractFromMemoryPool - extracts class information from the memory pool
void JvmCompatibility::extractFromMemoryPool(ClassAnalysis& classes) const
{
  msgTrace() << "Extracting JVM classes from memory pool" << std::endl;

  // Iterate over all SgAsmJvmFileHeader nodes in the memory pool
  struct JvmFileHeaderVisitor : ROSE_VisitTraversal
  {
    void visit(SgNode* node) override
    {
      if (SgAsmJvmFileHeader* fh = isSgAsmJvmFileHeader(node))
        fileHeaders.push_back(fh);
    }

    std::vector<SgAsmJvmFileHeader*> fileHeaders;
  };

  JvmFileHeaderVisitor visitor;
  SgAsmJvmFileHeader::traverseMemoryPoolNodes(visitor);

  for (SgAsmJvmFileHeader* jvmFileHeader : visitor.fileHeaders)
  {
    ClassKeyType classKey = classToKey(jvmFileHeader);

    // Create or get the class entry
    ClassData& classData = classes[classKey];

    // Set abstract flag
    classData.abstractClass(isAbstract(classKey));

    // Extract virtual functions
    extractVirtualFunctions(jvmFileHeader, classData);

    // Extract data members (fields)
    extractDataMembers(jvmFileHeader, classData);
  }
}

// extractFromProject - extracts class information from the AST rooted at root
void JvmCompatibility::extractFromProject(ClassAnalysis& classes, ASTRootType root) const
{
  msgTrace() << "Extracting JVM classes from project" << std::endl;

  if (root.key == 0)
  {
    msgError() << "Invalid root node" << std::endl;
    return;
  }

  SgNode* rootNode = keyToRoot(root);
  if (!rootNode)
  {
    msgError() << "Cannot convert root key to SgNode" << std::endl;
    return;
  }

  // Query all JVM file headers in the subtree
  std::vector<SgNode*> jvmClasses = NodeQuery::querySubTree(rootNode, V_SgAsmJvmFileHeader);

  for (SgNode* node : jvmClasses)
  {
    SgAsmJvmFileHeader* jvmFileHeader = isSgAsmJvmFileHeader(node);
    if (!jvmFileHeader) continue;

    ClassKeyType        classKey = classToKey(jvmFileHeader);

    // Create or get the class entry
    ClassData&          classData = classes[classKey];

    // Set abstract flag
    classData.abstractClass(isAbstract(classKey));

    // Extract virtual functions
    extractVirtualFunctions(jvmFileHeader, classData);

    // Extract data members (fields)
    extractDataMembers(jvmFileHeader, classData);
  }
}

LanguageSpecificFn
JvmCompatibility::languageSpecificProcessing() const
{
  return [](ClassAnalysis&) {};
}

namespace
{
  // Find the interface in the memory pool
  struct ClassAncestorVisitor : ROSE_VisitTraversal
  {
    ClassAncestorVisitor(std::string name, const JvmCompatibility* b)
      : targetName(std::move(name)), bridge(b), found(nullptr)
    {}

    void visit(SgNode* node) override
    {
      if (found) return;
      sg::NotNull<SgAsmJvmFileHeader> fh = isSgAsmJvmFileHeader(node);

      if (bridge->nameOf(classToKey(fh)) == targetName)
        found = fh;
    }

    std::string             targetName;
    const JvmCompatibility* bridge;
    SgAsmJvmFileHeader*     found;
  };


  SgAsmJvmFileHeader*
  findAncestor(const JvmCompatibility& bridge, SgAsmJvmConstantPool& pool, std::uint16_t idx)
  {
    sg::NotNull<SgAsmJvmConstantPoolEntry> entry = pool.get_entry(idx);
    std::uint16_t const                    ancestorNameIndex = entry->get_name_index();
    sg::NotNull<SgAsmJvmConstantPoolEntry> ancestorNameEntry = pool.get_entry(ancestorNameIndex);
    std::string const                      ancestorName = to_string(ancestorNameEntry);
    ClassAncestorVisitor                   ancestorVisitor(ancestorName, &bridge);

    SgAsmJvmFileHeader::traverseMemoryPoolNodes(ancestorVisitor);
    return ancestorVisitor.found;
  }

}

// inheritanceEdges - calls the callback function for all direct base classes
void JvmCompatibility::inheritanceEdges(
    ClassKeyType clkey,
    std::function<void(ClassKeyType, ClassKeyType, bool)> fn) const
{
  if (!isValidKey(clkey)) return;

  sg::NotNull<SgAsmJvmFileHeader>   jvmFileHeader = keyToClass(clkey);
  sg::NotNull<SgAsmJvmConstantPool> pool = jvmFileHeader->get_constant_pool();

  // Get superclass index
  std::uint16_t const               superClassIndex = jvmFileHeader->get_super_class();

  if (superClassIndex != 0)
  {
    if (SgAsmJvmFileHeader* superClass = findAncestor(*this, *pool, superClassIndex))
    {
      ClassKeyType superKey = classToKey(superClass);

      fn(clkey, superKey, false /*no virtual inheritance in Java*/);
    }
  }

  // Get interfaces
  for (std::uint16_t interfaceIndex : jvmFileHeader->get_interfaces())
  {
    if (SgAsmJvmFileHeader* interface = findAncestor(*this, *pool, interfaceIndex))
    {
      ClassKeyType interfaceKey = classToKey(interface);

      // Interfaces in Java are similar to virtual inheritance
      fn(clkey, interfaceKey, true/*virtual inheritance*/);
    }
  }
}

// typeOf - returns the type of a non-static data member
DataMemberType
JvmCompatibility::typeOf(VariableKeyType var) const
{
  if (!isValidKey(var))
    return DataMemberType{ClassKeyType{0}, false, false, false, false};

  sg::NotNull<SgAsmJvmField>        field = keyToVariable(var);
  SgAsmJvmFileHeader*               fileHeader = getFileHeaderFromField(field);
  if (!fileHeader) return DataMemberType{ClassKeyType{0}, false, false, false, false};
  sg::NotNull<SgAsmJvmConstantPool> pool = fileHeader->get_constant_pool();
  std::uint16_t const               descriptorIndex = field->get_descriptor_index();
  if (descriptorIndex == 0)
    return DataMemberType{ClassKeyType{0}, false, false, false, false};

  std::string                       descriptor = to_string(pool->get_entry(descriptorIndex));

  if (descriptor.empty())
    return DataMemberType{ClassKeyType{0}, false, false, false, false};

  // Parse the descriptor
  bool isConst        = false; // Java doesn't have const at the bytecode level (final is different)
  bool hasReference   = false;
  bool hasArray       = false;
  bool hasInitializer = false; // We can't easily determine this from bytecode
  ClassKeyType classType{0};

  // Check for array
  if (descriptor[0] == '[')
  {
    hasArray = true;

    // Skip array dimension markers
    size_t i = 0;
    while (i < descriptor.length() && descriptor[i] == '[') ++i;

    if (i < descriptor.length())
      descriptor = descriptor.substr(i);
  }

  // Check if it's a reference type (class or interface)
  if (!descriptor.empty() && descriptor[0] == 'L')
  {
    hasReference = true;

    // Extract class name (format: Lpackage/ClassName;)
    size_t endPos = descriptor.find(';');
    if (endPos != std::string::npos)
    {
      std::string className = descriptor.substr(1, endPos - 1);

      // Try to find the class in the memory pool
      class ClassTypeVisitor : public ROSE_VisitTraversal
      {
        public:
          void visit(SgNode* node) override
          {
            if (found) return;

            if (SgAsmJvmFileHeader* fh = isSgAsmJvmFileHeader(node))
            {
              std::string candidateName = bridge->nameOf(classToKey(fh));
              bool const  candidateMatches = (  (candidateName == targetName)
                                             || (candidateName.find(targetName) != std::string::npos)
                                             || (targetName.find(candidateName) != std::string::npos)
                                             );

              if (candidateMatches)
                found = fh;
            }
          }

          std::string             targetName;
          SgAsmJvmFileHeader*     found = nullptr;
          JvmCompatibility const* bridge = nullptr;
      };

      ClassTypeVisitor classVisitor;
      classVisitor.targetName = className;
      classVisitor.bridge = this;
      SgAsmJvmFileHeader::traverseMemoryPoolNodes(classVisitor);

      if (classVisitor.found)
        classType = classToKey(classVisitor.found);
    }
  }

  return DataMemberType{classType, isConst, hasReference, hasArray, hasInitializer};
}

} // namespace Rose::BinaryAnalysis::ByteCode::ClassHierarchy
