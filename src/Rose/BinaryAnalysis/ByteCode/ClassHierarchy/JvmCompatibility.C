#include <sage3basic.h>
#include <Rose/BinaryAnalysis/ByteCode/ClassHierarchy/JvmCompatibility.h>

#include <sageGeneric.h>

#include <string>
#include <unordered_set>

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
  //
  // TODO: Match on the full descriptor (name + parameters + return type), as
  // the JVM does (JVMS 5.4.5), and treat bridge methods explicitly (see the
  // TODO in extractVirtualFunctions).  The current mix is inconsistent:
  //   - return types follow Java source semantics (excluded here, covariance
  //     checked separately in haveSameOrCovariantReturn);
  //   - parameter types follow bytecode semantics, so generic erasure is not
  //     matched: compareTo(LA;)I never pairs with
  //     Comparable.compareTo(Ljava/lang/Object;)I, only its bridge does;
  //   - the covariant/generic override is dropped entirely when one of the
  //     return classes (typically java/lang/Object or another JDK class) is
  //     not in the memory pool, because haveSameOrCovariantReturn then
  //     reports 'unrelated' and the common analysis discards the pair.
  // With full-descriptor matching, the bridge pairs exactly with the base
  // method, and the bridge's target gives the source-level override.
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

  // TODO: Replace the substring matching below with an exact binary-name
  // lookup (ideally through a name -> SgAsmJvmFileHeader map built once by the
  // bridge).  Substring matching accepts any class whose name contains the
  // target, e.g., "Foo" matches "a/b/FooBar" and "Foo$Inner", and the loop
  // does not stop at the first hit, so the last match wins.  The linear scan
  // over all classes per override pair is also quadratic in the number of
  // classes.
  //
  // TODO: Handle array return types.  Descriptors starting with '[' are
  // currently reported as unrelated, although Java array types are covariant
  // (String[] <: Object[]) and every array type is a subtype of Object,
  // Cloneable and java/io/Serializable.
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
    bool isStatic  = (access & 0x0008 /*ACC_STATIC*/)  != 0;
    bool isPrivate = (access & 0x0002 /*ACC_PRIVATE*/) != 0;

    // Static methods are not dispatched virtually.  Private methods are not
    // inherited and neither override nor can be overridden (JVMS 5.4.5); a
    // subclass may declare an unrelated method with the same signature:
    //
    //   class Base { private Object f(int n) { return null; } }
    //   class Derived extends Base {
    //     Object f(int n) { return null; } // not an override
    //   }
    //
    // Both class files contain f:(I)Ljava/lang/Object;, so keeping private
    // methods would let the common hierarchy analysis falsely pair them.
    // Excluding them also removes private synthetic lambda bodies
    // (lambda$name$N) and private interface methods (Java 9+).  The remaining
    // access rules of JVMS 5.4.5 (a package-private method is overridden only
    // within its run-time package, directly or transitively) are applied
    // pairwise in canOverride.
    //
    // TODO: ACC_FINAL (0x0010) methods cannot be overridden; this could be
    // recorded so that clients know a method has no possible overriders.
    //
    // TODO: Exclude bridge methods (ACC_BRIDGE, 0x0040) and, more generally,
    // ACC_SYNTHETIC (0x1000) methods from the virtual function list, or map a
    // bridge to its target.  javac emits a bridge with the erased/base
    // descriptor for covariant and generic overrides; together with the
    // return-type-excluding comparison in compareFunctionsByType, the bridge
    // and the real method compare equal and only one of them (arbitrarily,
    // since std::sort is not stable) gets paired with the base method.
    if (isStatic || isPrivate) continue;

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

bool
JvmCompatibility::inheritedMethodsImplementVirtualBaseMethods() const
{
  return true;
}

bool
JvmCompatibility::isInheritableImplementation(FunctionKeyType key) const
{
  if (!isValidKey(key)) return false;

  sg::NotNull<SgAsmJvmMethod> method = keyToFunction(key);
  std::uint16_t const         access = method->get_access_flags();
  bool const                  isPublic   = (access & 0x0001 /*ACC_PUBLIC*/)   != 0;
  bool const                  isAbstract = (access & 0x0400 /*ACC_ABSTRACT*/) != 0;

  return isPublic && !isAbstract;
}

namespace
{
  // returns the package part of a binary class name (empty for the unnamed package)
  std::string packageOf(const std::string& binaryClassName)
  {
    std::size_t const pos = binaryClassName.rfind('/');

    return pos == std::string::npos ? std::string{} : binaryClassName.substr(0, pos);
  }
}

bool
JvmCompatibility::canOverride(FunctionKeyType drv, FunctionKeyType bas) const
{
  if (!isValidKey(drv) || !isValidKey(bas)) return false;

  sg::NotNull<SgAsmJvmMethod> drvMethod = keyToFunction(drv);
  sg::NotNull<SgAsmJvmMethod> basMethod = keyToFunction(bas);
  std::uint16_t const         drvAccess = drvMethod->get_access_flags();
  std::uint16_t const         basAccess = basMethod->get_access_flags();

  // JVMS 5.4.5: neither the overriding nor the overridden method is private
  if ((drvAccess & 0x0002 /*ACC_PRIVATE*/) || (basAccess & 0x0002 /*ACC_PRIVATE*/))
    return false;

  // a public or protected method can be overridden from any package
  if (basAccess & (0x0001 /*ACC_PUBLIC*/ | 0x0004 /*ACC_PROTECTED*/))
    return true;

  // a package-private method is overridden only by a method declared in the
  // same run-time package (rule (a)); rule (b), the transitive case through an
  // intermediate overrider, is handled by the common analysis.
  SgAsmJvmFileHeader*         drvClass = getFileHeaderFromMethod(drvMethod);
  SgAsmJvmFileHeader*         basClass = getFileHeaderFromMethod(basMethod);

  if (!drvClass || !basClass) return false;

  return packageOf(nameOf(classToKey(drvClass))) == packageOf(nameOf(classToKey(basClass)));
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


  // returns the binary class name referenced by the CONSTANT_Class entry at \p idx
  std::string
  referencedClassName(SgAsmJvmConstantPool& pool, std::uint16_t idx)
  {
    sg::NotNull<SgAsmJvmConstantPoolEntry> entry = pool.get_entry(idx);
    std::uint16_t const                    nameIndex = entry->get_name_index();
    sg::NotNull<SgAsmJvmConstantPoolEntry> nameEntry = pool.get_entry(nameIndex);

    return to_string(nameEntry);
  }

  // finds the class file with binary name \p name in the memory pool, or nullptr
  SgAsmJvmFileHeader*
  findClassByName(const JvmCompatibility& bridge, const std::string& name)
  {
    ClassAncestorVisitor ancestorVisitor(name, &bridge);

    SgAsmJvmFileHeader::traverseMemoryPoolNodes(ancestorVisitor);
    return ancestorVisitor.found;
  }

  // warns once per missing ancestor name that an inheritance edge was dropped
  void
  warnUnresolvedAncestor(const JvmCompatibility& bridge, ClassKeyType clkey, const char* kind, const std::string& name)
  {
    static std::unordered_set<std::string> reported;

    if (!reported.insert(name).second) return;

    msgWarn() << "JVM class hierarchy: " << kind << " " << name
              << " of " << bridge.nameOf(clkey) << " is not in the memory pool;"
              << " the inheritance edge and the methods inherited from it are not represented."
              << " (further occurrences of " << name << " are not reported)"
              << std::endl;
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
  bool const                        isInterface = (jvmFileHeader->get_access_flags() & 0x0200 /*ACC_INTERFACE*/) != 0;

  // Get superclass index
  std::uint16_t const               superClassIndex = jvmFileHeader->get_super_class();

  // JVMS 4.1 requires every interface to name java/lang/Object as its
  // super_class, but that is a formality: an interface has no superclass
  // subobject.  Emitting the edge would make the common analysis
  // (integrateIndirectInheritance) copy Object once per implemented interface
  // into every implementing class, i.e., several Object "subobjects" in C++
  // terms, and duplicate override edges against Object's methods (toString,
  // equals, hashCode, ...).  Object is reached exactly once through the
  // superclass chain of every class, so interfaces get no superclass edge.
  // This also leaves interfaces with an empty non-virtual inheritance chain,
  // which the implied-overrider computation in the common analysis relies on.
  if (superClassIndex != 0 && !isInterface)
  {
    std::string const superName = referencedClassName(*pool, superClassIndex);

    if (SgAsmJvmFileHeader* superClass = findClassByName(*this, superName))
      fn(clkey, classToKey(superClass), false /*no virtual inheritance in Java*/);
    else
      warnUnresolvedAncestor(*this, clkey, "superclass", superName);
  }

  // Get interfaces
  for (std::uint16_t interfaceIndex : jvmFileHeader->get_interfaces())
  {
    std::string const interfaceName = referencedClassName(*pool, interfaceIndex);

    if (SgAsmJvmFileHeader* interface = findClassByName(*this, interfaceName))
    {
      // Interfaces in Java are modelled as abstract virtual base classes
      fn(clkey, classToKey(interface), true /*virtual inheritance*/);
    }
    else
    {
      warnUnresolvedAncestor(*this, clkey, "interface", interfaceName);
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
      // TODO: Use an exact binary-name lookup instead of the bidirectional
      // substring match below (see the TODO in haveSameOrCovariantReturn).
      // The match here is even looser: candidateName containing targetName
      // or targetName containing candidateName both count, so a field of type
      // La/b/Foo; can resolve to Foo$Inner or FooBar, and a full memory-pool
      // traversal is performed for every field.
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
