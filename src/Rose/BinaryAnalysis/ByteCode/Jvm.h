#ifndef ROSE_BinaryAnalysis_ByteCode_Jvm_H
#define ROSE_BinaryAnalysis_ByteCode_Jvm_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/BasicTypes.h>
#include <Rose/BinaryAnalysis/ByteCode/Analysis.h>

class SgAsmJvmConstantPool;
class SgAsmJvmField;
class SgAsmJvmFileHeader;
class SgAsmJvmMethod;
class SgAsmInstructionList;

namespace Rose {
namespace BinaryAnalysis {
namespace ByteCode {


/** ByteCode Code class.
 *
 *  A Code object stores raw instructions for a JvmMethod.
 */
class Code {
  public:
    void bytes(const uint8_t *bytes);
    const uint8_t* bytes() const;

    void size(size_t size);
    size_t size() const;

    void offset(Address offset);
    Address offset() const;

  private:
    const uint8_t *bytes_ = nullptr; // non-owning pointer to bytes
    size_t size_ = 0;
    Address offset_ = 0;
};


class JvmField final : public Field {
  public:
    /** Shared ownership pointers. */
    using Ptr = Sawyer::SharedPointer<JvmField>;

    /** Allocating constructor. */
    static Ptr instance(SgAsmJvmFileHeader* jfh, SgAsmJvmField* fld);

  public:
    std::string name() const override;

    JvmField() = delete;
    JvmField(SgAsmJvmFileHeader* jfh, SgAsmJvmField* field);

  private:
    SgAsmJvmFileHeader* jfh_;
    SgAsmJvmField* sgField_;
};

class JvmMethod final : public Method {
  public:
    /** Shared ownership pointers. */
    using Ptr = Sawyer::SharedPointer<JvmMethod>;

    /** Allocating constructor. */
    static Ptr instance(std::string name, Address va, SgAsmJvmFileHeader* jfh, SgAsmJvmMethod* method);

    static JvmMethod::Ptr promote(const Sawyer::SharedPointer<Method>& from);

    bool isStatic() const override;
    bool isSystemReserved(const std::string &name) const override;

    const SgAsmInstructionList* instructions() const override;
    void decode(const Disassembler::BasePtr &disassembler) const override;

    void annotate() override;

    std::string descriptor() const override;

    SgAsmJvmConstantPool* constant_pool();

    JvmMethod() = delete;
    JvmMethod(std::string name, Address va, SgAsmJvmFileHeader* jfh, SgAsmJvmMethod* m);

  private:
    SgAsmJvmFileHeader* jfh_ = nullptr;
    SgAsmJvmMethod* sgMethod_ = nullptr;
    Code code_; // contains raw bytes from the JVM class file for disassembly and partitioning
};

class JvmInterface final : public Interface {
  public:
    /** Shared ownership pointers. */
    using Ptr = Sawyer::SharedPointer<JvmInterface>;

    /** Allocating constructor. */
    static Ptr instance(SgAsmJvmFileHeader* jfh, uint16_t index);

    std::string name() const override;
    uint16_t index() const;

    JvmInterface() = delete;
    JvmInterface(SgAsmJvmFileHeader* jfh, uint16_t index);

  private:
    SgAsmJvmFileHeader* jfh_;
    uint16_t index_;
};

class JvmAttribute final : public Attribute {
  public:
    /** Shared ownership pointers. */
    using Ptr = Sawyer::SharedPointer<JvmAttribute>;

    /** Allocating constructor. */
    static Ptr instance(SgAsmJvmFileHeader* jfh, uint16_t index);

    std::string name() const override;
    uint16_t index() const;

    JvmAttribute() = delete;
    JvmAttribute(SgAsmJvmFileHeader* jfh, uint16_t index);

  private:
    SgAsmJvmFileHeader* jfh_;
    uint16_t index_;
};

class JvmClass final : public Class {
  public:
    /** Shared ownership pointers. */
    using Ptr = Sawyer::SharedPointer<JvmClass>;

    /** Allocating constructor. */
    static Ptr instance(std::string name, NamespacePtr ns, SgAsmJvmFileHeader* jfh);

    static JvmClass::Ptr promote(const Sawyer::SharedPointer<Class>& from);

    std::string super_name() const override;
    std::string typeSeparator() const override;
    void dump() override;

    const std::vector<std::string>& strings() override;
    SgAsmJvmConstantPool* constant_pool();

    JvmClass() = delete;
    JvmClass(std::string name, NamespacePtr ns, SgAsmJvmFileHeader* jfh);

  private:
    SgAsmJvmFileHeader* jfh_;
};

class JvmContainer final : public Container {
  public:
    std::string name() const override;
    bool isSystemReserved(const std::string &name) const override;
    static bool isJvmSystemReserved(const std::string &name);
};

/** The string found at the given index into the constant pool. */
std::string constantPoolEntryName(uint16_t index, const SgAsmJvmConstantPool *pool);

} // namespace
} // namespace
} // namespace

#endif
#endif
