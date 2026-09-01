#ifndef ROSE_BinaryAnalysis_ByteCode_Cil_H
#define ROSE_BinaryAnalysis_ByteCode_Cil_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Rose/BinaryAnalysis/ByteCode/Analysis.h>

class SgAsmCilMetadata;
class SgAsmCilMetadataRoot;
class SgAsmCilMethodDef;
class SgAsmInstructionList;
class SgAsmIntegerValueExpression;

namespace Rose {
namespace BinaryAnalysis {
namespace ByteCode {

class CilCode final : public Code {
  public:
    /** Shared ownership pointer. */
    using Ptr = Sawyer::SharedPointer<CilCode>;

  public:
    const uint8_t* bytes() const override;
    size_t size() const override;
    Address offset() const override;

    CilCode(uint8_t* bytes, size_t size, Address offset);

  private:
    const uint8_t* bytes_;
    size_t size_;
    Address offset_;
};

class CilField final : public Field {
  public:
    /** Shared ownership pointer. */
    using Ptr = Sawyer::SharedPointer<CilField>;

  public:
    std::string name() const override;
    CilField() = delete;
};

class CilMethod final : public Method {
  public:
    /** Shared ownership pointer. */
    using Ptr = Sawyer::SharedPointer<CilMethod>;

    /** Allocating constructor. */
    static Ptr instance(std::string name, Address, SgAsmCilMetadataRoot*, SgAsmCilMethodDef*);

  public:
    bool isStatic() const override;
    bool isSystemReserved(const std::string &name) const override;

    const Code & code() const override;
    const SgAsmInstructionList* instructions() const override;
    void decode(const Disassembler::BasePtr &disassembler) const override;

    std::string descriptor() const override;

    void annotate() override;

    CilMethod() = delete;
    CilMethod(std::string name, Address, SgAsmCilMetadataRoot*, SgAsmCilMethodDef*);

  private:
    SgAsmCilMetadataRoot* mdr_;
    SgAsmCilMethodDef* sgMethod_;
    SgAsmInstructionList* insns_;
    CilCode code_;
};

class CilInterface final : public Interface {
  public:
    std::string name() const override;
    CilInterface() = delete;
};

class CilAttribute final : public Attribute {
  public:
    std::string name() const override;
    CilAttribute() = delete;
};

class CilClass final : public Class {
  public:
    /** Shared ownership pointers. */
    using Ptr = Sawyer::SharedPointer<CilClass>;

    /** Allocating constructor. */
    static Ptr instance(std::string name, NamespacePtr ns, SgAsmCilMetadataRoot*, size_t, size_t);

    static Class::Ptr promote(const Sawyer::SharedPointer<Class>& from);
    static std::string objectName(const SgAsmCilMetadata*, SgAsmCilMetadataRoot*);

    std::string super_name() const override;
    std::string typeSeparator() const override;
    void dump() override;

    CilClass() = delete;
    CilClass(std::string name, NamespacePtr ns, SgAsmCilMetadataRoot*, size_t, size_t);

private:
    SgAsmCilMetadataRoot* mdr_;
    //WARNING: not used yet!
    //SgAsmCilTypeDef* sgCilTypeDef_;
};

class CilNamespace final : public Namespace {
public:
    /** Shared ownership pointers. */
    using Ptr = Sawyer::SharedPointer<CilNamespace>;

    std::string name() const override;

    /** Allocating constructor. */
    static Ptr instance(SgAsmCilMetadataRoot*, const std::string &);

    CilNamespace() = delete;
    CilNamespace(SgAsmCilMetadataRoot*, const std::string &);

private:
    //WARNING: not used yet!
    //SgAsmCilMetadataRoot* mdr_;
    std::string name_;
};

class CilContainer final : public Container {
  public:
    /** Shared ownership pointer. */
    using Ptr = Sawyer::SharedPointer<CilContainer>;

  public:
    std::string name() const override;
    bool isSystemReserved(const std::string &name) const override;
    static  bool isCilSystemReserved(const std::string &name);

    void printAssemblies(std::ostream& os) const;
    void printMethods(std::ostream& os, size_t beg, size_t lim) const;
    void printModules(std::ostream& os) const;
    void printTypeDefs(std::ostream& os) const;

    static SgAsmCilMetadata* resolveToken(SgAsmIntegerValueExpression*, SgAsmCilMetadataRoot*);

    CilContainer() = delete;
    explicit CilContainer(SgAsmCilMetadataRoot*);

  private:
    SgAsmCilMetadataRoot* mdr_;
};

} // namespace
} // namespace
} // namespace

#endif
#endif
