#ifndef ROSE_BinaryAnalysis_ByteCode_Analysis_H
#define ROSE_BinaryAnalysis_ByteCode_Analysis_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Rose/BinaryAnalysis/Disassembler/BasicTypes.h>
#include <Rose/BinaryAnalysis/Partitioner2/BasicTypes.h>
#include <Rose/Progress.h>

class SgAsmInstructionList;

namespace Rose {
namespace BinaryAnalysis {
namespace ByteCode {

namespace P2 = Partitioner2;

using BasicBlockPtr = P2::BasicBlockPtr;
using PartitionerPtr = P2::PartitionerPtr;
using InstructionMap = std::map<Address, SgAsmInstruction*>;

// Forward references
class Class;
class Namespace;

using ClassPtr = Sawyer::SharedPointer<Class>;
using NamespacePtr = Sawyer::SharedPointer<Namespace>;

/** Base class for ByteCode Fields.
 *
 *  A Field stores information about fields in a class.
 */
class Field: public Sawyer::SharedObject,
             public Sawyer::SharedFromThis<Field> {
  public:
    /** Shared ownership pointer. */
    using Ptr = Sawyer::SharedPointer<Field>;

    virtual ~Field();

public:
    virtual std::string name() const = 0;

protected:
    Field();
};

/** Base class for ByteCode Methods.
 *
 *  A Method stores information about an instance method/function such as its name and
 *  instructions.
 */
class Method: public Sawyer::SharedObject,
              public Sawyer::SharedFromThis<Method> {
  public:
    /** Shared ownership pointer. */
    using Ptr = Sawyer::SharedPointer<Method>;

    virtual ~Method();

  public:
    const std::string& name() const;
    Address address() const;

    /** Complete initialization of the method once decoding is done. */
    void finalize();

    virtual bool returnsVoid() const;
    virtual bool isStatic() const = 0;
    virtual bool isSystemReserved(const std::string &name) const = 0;

    virtual const SgAsmInstructionList* instructions() const = 0;
    virtual void decode(const Disassembler::BasePtr&) const = 0;

    virtual std::string descriptor() const = 0;

    /* The method identity; class + name + descriptor */
    virtual std::string identity() const;

    /* Annotate the AST (.e.g., add comments to instructions) */
    virtual void annotate() = 0;

    /* Set of instruction branch targets */
    std::set<Address> targets() const;

    /* Accessors to the declaring class for this method */
    Class* declaringClass() const;
    void declaringClass(Class *declaringClass);

    /* Retrieve the instruction at the given address */
    SgAsmInstruction* instructionAt(Address va) const;

    // Methods associated with basic blocks (Rose::BinaryAnalysis::Partitioner2)
    //
    const std::vector<BasicBlockPtr>& blocks() const;
    void append(BasicBlockPtr bb);

    Method() = delete;

  protected:
    Method(std::string name, Address va);

  private:
    std::string name_;

  protected:
    Address address_ = 0;

  private:
    Class* class_ = nullptr; // non-owning pointer to the declaring class
    P2::FunctionPtr function_;
    std::vector<BasicBlockPtr> blocks_;
    InstructionMap instructionMap_;
};

/** Base class for ByteCode Interface.
 *
 *  An Interface stores information about an interface.
 */
class Interface: public Sawyer::SharedObject,
                 public Sawyer::SharedFromThis<Interface> {
  public:
    /** Shared ownership pointers. */
    using Ptr = Sawyer::SharedPointer<Interface>;

    virtual ~Interface();

    virtual std::string name() const = 0;

  protected:
    Interface();
};

/** Base class for ByteCode Attribute.
 *
 *  An Attribute stores information about an attribute.
 */
class Attribute: public Sawyer::SharedObject,
                 public Sawyer::SharedFromThis<Attribute> {
public:
    /** Shared ownership pointers. */
    using Ptr = Sawyer::SharedPointer<Attribute>;

    virtual ~Attribute();

    virtual std::string name() const = 0;

  protected:
    Attribute();
};

/** Base class for ByteCode Class.
 *
 *  An Class stores information about a class, for example its name.
 */
class Class: public Sawyer::SharedObject,
             public Sawyer::SharedFromThis<Class> {
  public:
    /** Shared ownership pointers. */
    using Ptr = Sawyer::SharedPointer<Class>;
    using NamespacePtr = Sawyer::SharedPointer<Namespace>; // forward declaration

    virtual ~Class();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Dynamic pointer cast. No-op since this is the base class
    //  static Class promote(const Class&);

    const std::string& name() const;
    const std::string& baseClassName() const;

    Address address() const;
    NamespacePtr nameSpace() const;

    ByteCode::Method::Ptr findMethod(const std::string &name, const std::string &descriptor) const;

    virtual std::string qualifiedName() const;
    virtual std::string typeSeparator() const = 0;
    virtual const std::vector<std::string>& strings();

    const std::vector<Field::Ptr>& fields() const;
    const std::vector<Method::Ptr>& methods() const;
    const std::vector<Attribute::Ptr>& attributes() const;
    const std::vector<Interface::Ptr>& interfaces() const;

    /** Complete initialization of the class once partitioning is done. */
    void finalize();

    virtual void partition(const PartitionerPtr &partitioner, std::map<std::string,Address> &discoveredFunctions,
                           const Progress::Ptr &progress = Progress::Ptr());
    virtual void digraph() const;
    virtual void dump() = 0;

    Class() = delete;

  private:
    std::string name_;
    Address address_ = 0; // Base virtual address assigned to this loaded class file
    NamespacePtr namespace_;

  protected:
    Class(std::string name, Address va, NamespacePtr ns);

    std::string baseClassName_;

    std::vector<Field::Ptr> fields_;
    std::vector<Method::Ptr> methods_;
    std::vector<Attribute::Ptr> attributes_;
    std::vector<Interface::Ptr> interfaces_;
    std::vector<std::string> strings_;
};

/** Base class for ByteCode Namespace.
 *
 *  A Namespace contains a vector of Classes.
 */
class Namespace: public Sawyer::SharedObject,
                 public Sawyer::SharedFromThis<Namespace> {
  public:
    /** Shared ownership pointer. */
    using Ptr = Sawyer::SharedPointer<Namespace>;

    virtual ~Namespace() = default;

    /** Allocating constructor. */
    static Ptr instance(std::string name);

    virtual std::string name() const;
    virtual void partition(const PartitionerPtr &partitioner,
                           std::map<std::string,Address> &discoveredFunctions);

    void append(Class::Ptr ptr);

    const std::vector<ByteCode::Class::Ptr>& classes() const;

  protected:
    Namespace() = delete;
    explicit Namespace(std::string name);

    std::vector<ByteCode::Class::Ptr> classes_;

  private:
    std::string name_;
};

/** Base class for ByteCode Container.
 *
 *  A Container contains a vector of Namespaces.
 */
class Container: public Sawyer::SharedObject,
                 public Sawyer::SharedFromThis<Container> {
  public:
    /** Shared ownership pointer. */
    using Ptr = Sawyer::SharedPointer<Container>;

    virtual ~Container() = default;

    /* A unique (per container) virtual address for system/library functions */
    static Address nextSystemReservedVa();

  public:
    virtual std::string name() const;
    virtual bool isSystemReserved(const std::string &name) const = 0;
    virtual void partition(const PartitionerPtr &partitioner);

    const std::vector<Namespace::Ptr>& namespaces() const;

  protected:
    Container() = delete;
    explicit Container(std::string name);

    std::vector<Namespace::Ptr> namespaces_;

  private:
    std::string name_;
    static Address nextSystemReservedVa_;
};

/** Class repository
 *
 *  A repository for containing Classes
 */
class ClassRepository: public Sawyer::SharedObject,
                        public Sawyer::SharedFromThis<ClassRepository> {
  public:
    /** Shared ownership pointer. */
    using Ptr = Sawyer::SharedPointer<ClassRepository>;

    ClassRepository() = default;
    virtual ~ClassRepository() = default;

  public:

    /** Allocating constructor. */
    static Ptr instance();

    /** Inserts a class into the repository.
     *
     *  Returns true if the class was inserted, or false if a class with the same name is already present.
     */
    bool insert(const Class::Ptr &cls);

    /** Returns true if a class with the given name is present. */
    bool contains(const std::string &name) const;

    /** Returns the Class with the given name, null if not present. */
    ByteCode::ClassPtr findClass(const std::string &name) const;

    /** Returns true if a value of the source class can be assigned to the target. */
    bool isAssignableTo(const ClassPtr &source, const ClassPtr &target) const;

  private:
    std::map<std::string, ClassPtr> classes_;
};

} // namespace
} // namespace
} // namespace

#endif
#endif
