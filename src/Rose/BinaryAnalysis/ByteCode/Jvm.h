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

class JvmCode final : public Code {
  public:
    /** Shared ownership pointer. */
    using Ptr = Sawyer::SharedPointer<JvmCode>;

    /** Allocating constructor. */
    static Ptr instance(uint8_t* bytes, size_t size, Address offset);

  public:
    const uint8_t* bytes() const override;
    size_t size() const override;
    Address offset() const override;

    void bytes(const uint8_t* buf);
    void size(size_t sz);
    void offset(Address off);

    JvmCode(uint8_t* bytes, size_t size, Address offset);

  private:
    const uint8_t* bytes_;
    size_t size_;
    Address offset_;
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

    const Code & code() const override;
    const SgAsmInstructionList* instructions() const override;
    void decode(const Disassembler::BasePtr &disassembler) const override;

    void annotate() override;

    std::string descriptor() const override;

    SgAsmJvmConstantPool* constant_pool();

    JvmMethod() = delete;
    JvmMethod(std::string name, Address va, SgAsmJvmFileHeader* jfh, SgAsmJvmMethod* method);

  private:
    SgAsmJvmFileHeader* jfh_;
    SgAsmJvmMethod* sgMethod_;
    JvmCode code_;
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
