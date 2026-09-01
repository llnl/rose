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

// Forward references
class Class;
class Namespace;

using ClassPtr = Sawyer::SharedPointer<Class>;
using NamespacePtr = Sawyer::SharedPointer<Namespace>;

/** Base class for ByteCode Code.
 *
 *  A Code object stores raw instructions for a Method.
 */
class Code: public Sawyer::SharedObject,
            public Sawyer::SharedFromThis<Code> {
  public:
    /** Shared ownership pointer. */
    using Ptr = Sawyer::SharedPointer<Code>;

    virtual ~Code();

public:
    virtual const uint8_t* bytes() const = 0;
    virtual size_t size() const = 0;
    virtual Address offset() const = 0;

protected:
    Code();
};

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

    virtual bool isStatic() const = 0;
    virtual bool isSystemReserved(const std::string &name) const = 0;

    virtual const Code & code() const = 0;
    virtual const SgAsmInstructionList* instructions() const = 0;
    virtual void decode(const Disassembler::BasePtr&) const = 0;

    virtual std::string descriptor() const = 0;

    /* Annotate the AST (.e.g., add comments to instructions) */
    virtual void annotate() = 0;

    /* Set of instruction branch targets */
    std::set<Address> targets() const;

    // Accessors to ByteCode::Class containing this method
    ClassPtr analysisClass();
    void analysisClass(ClassPtr);

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

    ClassPtr class_; // Class containing this method
    P2::FunctionPtr function_;
    std::vector<BasicBlockPtr> blocks_;
};

/** Base class for ByteCode Interface.
 *
 *  An Interface stores information about an interface.
 */
class Interface: public Sawyer::SharedObject,
                 public Sawyer::SharedFromThis<Class> {
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
    Address address() const;

    virtual std::string super_name() const = 0;
    virtual std::string typeSeparator() const = 0;

    virtual const std::vector<std::string>& strings();

    const std::vector<Field::Ptr>& fields() const;
    const std::vector<Method::Ptr>& methods() const;
    const std::vector<Attribute::Ptr>& attributes() const;
    const std::vector<Interface::Ptr>& interfaces() const;

    virtual void partition(const PartitionerPtr &partitioner, std::map<std::string,Address> &discoveredFunctions,
                           const Progress::Ptr &progress = Progress::Ptr());
    virtual void digraph() const;
    virtual void dump() = 0;

    Class() = delete;

  private:
    std::string name_;
    /** Base virtual address assigned to this loaded class file. */
    Address address_ = 0;
    NamespacePtr namespace_;

  protected:
    Class(std::string name, Address va, NamespacePtr ns);

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

    virtual ~Namespace();

    /** Allocating constructor. */
    static Ptr instance();

    virtual std::string name() const;
    virtual void partition(const PartitionerPtr &partitioner,
                           std::map<std::string,Address> &discoveredFunctions);

    void append(Class::Ptr ptr);

    const std::vector<ByteCode::Class::Ptr>& classes() const;

  protected:
    Namespace();
    std::vector<ByteCode::Class::Ptr> classes_;
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

    virtual ~Container();

    /* A unique (per container) virtual address for system/library functions */
    static Address nextSystemReservedVa();

  public:
    virtual std::string name() const = 0;
    virtual bool isSystemReserved(const std::string &name) const = 0;
    virtual void partition(const PartitionerPtr &partitioner);

    const std::vector<Namespace::Ptr>& namespaces() const;

  protected:
    Container();
    std::vector<Namespace::Ptr> namespaces_;

  private:
    static Address nextSystemReservedVa_;
};

} // namespace
} // namespace
} // namespace

#endif
#endif
