#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <Rose/BinaryAnalysis/ByteCode/Jvm.h>

#include <Rose/As.h>
#include <Rose/BinaryAnalysis/Disassembler/Jvm.h>
#include <Rose/BinaryAnalysis/Partitioner2/BasicBlock.h>

#include <SgAsmJvmAttribute.h>
#include <SgAsmJvmAttributeTable.h>
#include <SgAsmJvmCodeAttribute.h>
#include <SgAsmJvmConstantPool.h>
#include <SgAsmJvmConstantPoolEntry.h>
#include <SgAsmJvmField.h>
#include <SgAsmJvmFieldTable.h>
#include <SgAsmJvmFileHeader.h>
#include <SgAsmJvmInstruction.h>
#include <SgAsmJvmMethod.h>
#include <SgAsmJvmMethodTable.h>
#include <SgAsmInstructionList.h>
#include <SgAsmIntegerValueExpression.h>
#include <SgAsmOperandList.h>

#include <Cxx_GrammarDowncast.h>

using PoolEntry = SgAsmJvmConstantPoolEntry;
using AddressSegment = Sawyer::Container::AddressSegment<Rose::BinaryAnalysis::Address,uint8_t>;
using opcode = Rose::BinaryAnalysis::JvmInstructionKind;

namespace Rose {
namespace BinaryAnalysis {
namespace ByteCode {

// Access flag, others include ACC_PUBLIC, ACC_PRIVATE, ...
constexpr uint16_t ACC_STATIC = 0x0008;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const uint8_t*
Code::bytes() const {
    return bytes_;
}

void
Code::bytes(const uint8_t *bytes) {
    bytes_ = bytes;
}

size_t
Code::size() const {
    return size_;
}

void
Code::size(size_t size) {
    size_ = size;
}

Address
Code::offset() const {
    return offset_;
}

void
Code::offset(Address offset) {
    offset_ = offset;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// JvmField
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

JvmField::Ptr
JvmField::instance(SgAsmJvmFileHeader* jfh, SgAsmJvmField* fld) {
    return Ptr(new JvmField(jfh, fld));
}

JvmField::JvmField(SgAsmJvmFileHeader* jfh, SgAsmJvmField* field)
  : jfh_{jfh}, sgField_{field} {
}

std::string
JvmField::name() const {
    auto pool = jfh_->get_constant_pool();
    if (sgField_->get_name_index() != 0) {
        return pool->get_utf8_string(sgField_->get_name_index());
    }
    return std::string{""};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// JvmMethod
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

JvmMethod::Ptr
JvmMethod::instance(std::string name, Address va, SgAsmJvmFileHeader* jfh, SgAsmJvmMethod* sgMethod) {
    return Ptr(new JvmMethod(std::move(name), va, jfh, sgMethod));
}

JvmMethod::JvmMethod(std::string name, Address va, SgAsmJvmFileHeader* jfh, SgAsmJvmMethod* sgMethod)
  : Method{std::move(name),va}, jfh_{jfh}, sgMethod_{sgMethod} {
    ASSERT_not_null(jfh_);
    ASSERT_not_null(sgMethod_);

    auto attribute_table = sgMethod_->get_attribute_table();
    for (auto attribute : attribute_table->get_attributes()) {
        if (auto codeAttr{isSgAsmJvmCodeAttribute(attribute)}) {
            const uint8_t* code{reinterpret_cast<const uint8_t*>(codeAttr->get_code())};
            // Set JvmCode object fields
            code_.bytes(code);
            code_.size(codeAttr->get_code_length());
            code_.offset(codeAttr->get_code_offset());
        }
    }
}

JvmMethod::Ptr
JvmMethod::promote(const Sawyer::SharedPointer<Method>& from) {
    if (from == nullptr) {
        return JvmMethod::Ptr();
    }
    JvmMethod::Ptr derived = from.dynamicCast<JvmMethod>();
    return derived;
}

bool JvmMethod::isStatic() const {
    return (sgMethod_->get_access_flags() & ACC_STATIC) != 0;
}

bool JvmMethod::isSystemReserved(const std::string &name) const {
    return JvmContainer::isJvmSystemReserved(name);
}

std::string JvmMethod::descriptor() const {
    auto pool = jfh_->get_constant_pool();
    if (sgMethod_->get_descriptor_index() != 0) {
        return pool->get_utf8_string(sgMethod_->get_descriptor_index());
    }
    return std::string{""};
}

SgAsmJvmConstantPool*
JvmMethod::constant_pool() {
    return jfh_->get_constant_pool();
}

const SgAsmInstructionList*
JvmMethod::instructions() const {
    return sgMethod_->get_instruction_list();
}

void
JvmMethod::decode(const Disassembler::Base::Ptr &disassembler) const {
    Address va{address_ + code_.offset()};
    Address endVa{va + code_.size()};

    auto disassemblerJvm{as<Disassembler::Jvm>(disassembler)};
    disassemblerJvm->codeOffset(va);

    MemoryMap::Ptr map = MemoryMap::instance();
    MemoryMap::Buffer::Ptr buf = MemoryMap::StaticBuffer::instance(code_.bytes(), code_.size());
    map->insert(AddressInterval::baseSize(va, buf->size()),
                AddressSegment(buf, /*offset*/0, MemoryMap::READABLE, "JVM code segment"));

    SgAsmInstruction* insn{nullptr};
    SgAsmInstructionList* insnList{sgMethod_->get_instruction_list()};

    while (va < endVa) {
        insn = disassembler->disassembleOne(map, va);
        if (insn) {
            va += insn->get_size();
            ASSERT_require2((666 != (int)insn->get_anyKind()), "unknown instruction");
            insnList->get_instructions().push_back(insn);
            insn->set_parent(insnList);
        }
        else {
            ASSERT_require2(false, "disassembly failed for lack of instruction");
        }
    }
    ASSERT_require(va == endVa);
}

void
JvmMethod::annotate()
{
  auto pool = jfh_->get_constant_pool();

  for (auto insn: instructions()->get_instructions()) {
    std::string comment{};
    auto jvmInsn{isSgAsmJvmInstruction(insn)};

    switch (jvmInsn->get_kind()) {
      case opcode::checkcast:
      case opcode::getfield:
      case opcode::getstatic:
      case opcode::instanceof:
      case opcode::invokedynamic:
      case opcode::invokeinterface:
      case opcode::invokespecial:
      case opcode::invokestatic:
      case opcode::invokevirtual:
      case opcode::new_:
      case opcode::putfield:
      case opcode::putstatic: {
        if (auto expr = isSgAsmIntegerValueExpression(insn->get_operandList()->get_operands()[0])) {
          comment = ByteCode::constantPoolEntryName(expr->get_value(), pool);
          expr->set_comment(comment);
          // Also store the name in the instruction's comment for later convenience in partitioning
          insn->set_comment(comment);
        }
        break;
      }
      default: ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// JvmInterface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

JvmInterface::Ptr
JvmInterface::instance(SgAsmJvmFileHeader* jfh, uint16_t index) {
    return Ptr(new JvmInterface(jfh, index));
}

JvmInterface::JvmInterface(SgAsmJvmFileHeader* jfh, uint16_t index)
  : jfh_{jfh}, index_{index} {
}

uint16_t
JvmInterface::index() const {
    return index_;
}

std::string
JvmInterface::name() const {
    auto pool = jfh_->get_constant_pool();
    if (0 != index()) {
        auto classEntry = pool->get_entry(index());
        if (classEntry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Class) {
            if (classEntry->get_name_index() != 0) {
                return pool->get_utf8_string(classEntry->get_name_index());
            }
        }
    }
    return std::string{""};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// JvmAttribute
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

JvmAttribute::Ptr
JvmAttribute::instance(SgAsmJvmFileHeader* jfh, uint16_t index) {
    return Ptr(new JvmAttribute(jfh, index));
}

uint16_t
JvmAttribute::index() const {
    return index_;
}

std::string
JvmAttribute::name() const {
    auto pool = jfh_->get_constant_pool();
    if (0 != index()) {
        auto entry = pool->get_entry(index());
        if (entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Utf8) {
            return pool->get_utf8_string(index());
        }
    }
    return std::string{""};
}

JvmAttribute::JvmAttribute(SgAsmJvmFileHeader* jfh, uint16_t index)
  : jfh_{jfh}, index_{index} {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// JvmClass
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

JvmClass::JvmClass(std::string name, NamespacePtr ns, SgAsmJvmFileHeader* jfh)
  : Class{std::move(name),jfh->get_baseVa(),ns}, jfh_{jfh}
{
    ASSERT_not_null(jfh);
    ASSERT_not_null(jfh->get_field_table());
    ASSERT_not_null(jfh->get_method_table());
    ASSERT_not_null(jfh->get_attribute_table());

    auto fields = jfh->get_field_table()->get_fields();
    auto methods = jfh->get_method_table()->get_methods();
    auto attributes = jfh->get_attribute_table()->get_attributes();
    const std::list<uint16_t>& interfaces = jfh->get_interfaces();

    for (auto sgField : fields) {
        fields_.push_back(JvmField::instance(jfh, sgField));
    }
    for (auto sgMethod : methods) {
        auto pool = jfh->get_constant_pool();
        ASSERT_not_null(pool);

        std::string methodName = constantPoolEntryName(sgMethod->get_name_index(), pool);

        // The method address is the address of the first instruction
        auto insns = sgMethod->get_instruction_list()->get_instructions();

        Address methodVa = insns.empty() ? 0 : insns.front()->get_address();

        // Need to create a unique va if methodVa is zero
#if 0
        ASSERT_require2(methodVa != 0, "Need unique method address");
        methods_.push_back(JvmMethod::instance(std::move(methodName), methodVa, jfh, sgMethod));
#else
        if (methodVa == 0) {
            // likely abstract and/or has no instructions, ignore for now
            // Address nextSyntheticVa = classBaseAddress; // assign to ++tail last instruction in the class.
        }
#endif
    }
    for (uint16_t index  : interfaces) {
        interfaces_.push_back(JvmInterface::instance(jfh, index));
    }
    for (auto sgAttribute : attributes) {
        attributes_.push_back(JvmAttribute::instance(jfh, sgAttribute->get_attribute_name_index()));
    }
}

JvmClass::Ptr
JvmClass::instance(std::string name, NamespacePtr ns, SgAsmJvmFileHeader* jfh) {
    return Ptr(new JvmClass(std::move(name), ns, jfh));
}

JvmClass::Ptr
JvmClass::promote(const Sawyer::SharedPointer<Class>& from) {
    if (from == nullptr) {
        return JvmClass::Ptr();
    }
    JvmClass::Ptr derived = from.dynamicCast<JvmClass>();
    ASSERT_not_null(derived);
    return derived;
}

std::string
constantPoolEntryName(uint16_t index, const SgAsmJvmConstantPool* pool) {
    SgAsmJvmConstantPoolEntry* entry = pool->get_entry(index);
    ASSERT_not_null(entry);

    switch (entry->get_tag()) {
      case PoolEntry::CONSTANT_Class: // 4.4.1  CONSTANT_Class_info table entry
      case PoolEntry::CONSTANT_NameAndType: // 4.4.6 CONSTANT_NameAndType_info table entry
      case PoolEntry::CONSTANT_Module: // 4.4.11 CONSTANT_Module_info table entry
      case PoolEntry::CONSTANT_Package: // 4.4.12 CONSTANT_Package_info table entry
          return ByteCode::constantPoolEntryName(entry->get_name_index(), pool);

      case PoolEntry::CONSTANT_String: // 4.4.2 CONSTANT_String_info table entry
          return ByteCode::constantPoolEntryName(entry->get_string_index(), pool);

      case PoolEntry::CONSTANT_Fieldref: // 4.4.3 CONSTANT_Fieldref_info table entry
      case PoolEntry::CONSTANT_Methodref: // 4.4.3 CONSTANT_Methodref_info table entry
      case PoolEntry::CONSTANT_InterfaceMethodref: { // 4.4.3 CONSTANT_InterfaceMethodref_info table entry
          std::string className{ByteCode::constantPoolEntryName(entry->get_class_index(), pool)};
          return className + "::" + ByteCode::constantPoolEntryName(entry->get_name_and_type_index(), pool);
      }

      case PoolEntry::CONSTANT_Utf8: // 4.4.7 CONSTANT_Utf8_info table entry
          return pool->get_utf8_string(index);

      case PoolEntry::CONSTANT_MethodHandle: // 4.4.8 CONSTANT_MethodHandle_info table entry
          return ByteCode::constantPoolEntryName(entry->get_reference_index(), pool);

      case PoolEntry::CONSTANT_MethodType: // 4.4.9 CONSTANT_MethodType_info table entry
          return ByteCode::constantPoolEntryName(entry->get_descriptor_index(), pool);

      case PoolEntry::CONSTANT_Dynamic: // 4.4.10 CONSTANT_Dynamic_info table entry
      case PoolEntry::CONSTANT_InvokeDynamic: // 4.4.10 CONSTANT_InvokeDynamic_info table entry
          return std::string{"bootstrap_method::"}
                 + ByteCode::constantPoolEntryName(entry->get_name_and_type_index(), pool);

      default: ;
    }

    return std::string{""};
}

std::string
JvmClass::super_name() const {
    auto pool = jfh_->get_constant_pool();
    auto super_index = jfh_->get_super_class();
    auto class_info = pool->get_entry(super_index);
    if (class_info->get_name_index() != 0) {
        return pool->get_utf8_string(class_info->get_name_index());
    }
    return std::string{""};
}

std::string
JvmClass::typeSeparator() const {
    return "::";
}

const std::vector<std::string>&
JvmClass::strings() {
    strings_.clear();
    auto pool = jfh_->get_constant_pool();
    for (auto entry : pool->get_entries()) {
        auto tag{entry->get_tag()};
        if (entry && (tag==PoolEntry::CONSTANT_Utf8 || tag==PoolEntry::CONSTANT_Utf8)) {
            strings_.push_back(std::string{entry->get_utf8_bytes(), entry->get_length()});
        }
    }
    return strings_;
}

SgAsmJvmConstantPool*
JvmClass::constant_pool() {
    return jfh_->get_constant_pool();
}

void
JvmClass::dump() {
    using std::cout;

    cout << "\n";
    cout << "----------------\n";
    cout << "class '" << name() << "'" << std::endl;
    cout << "----------------\n";
    cout << "   super: " << super_name() << "\n\n";

    cout << "constant pool\n";
    cout << "-----------\n";
    constant_pool()->dump(stdout, "", 1);
    cout << "-----------\n\n";

    if (interfaces().size() > 0) {
        cout << "interfaces\n";
        cout << "-----------\n";
        for (auto interface : interfaces()) {
            cout << "   interface: " << interface->name() << std::endl;
        }
        cout << "-----------\n\n";
    }

    if (fields().size() > 0) {
        cout << "fields\n";
        cout << "-----------\n";
        for (auto field : fields()) {
            cout << "   field: " << field->name() << std::endl;
        }
        cout << "-----------\n\n";
    }

    if (attributes().size() > 0) {
        cout << "attributes\n";
        cout << "-----------\n";
        for (auto attribute : attributes()) {
            cout << "   attribute: " << attribute->name() << std::endl;
        }
        cout << "-----------\n\n";
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// JvmContainer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JvmContainer::isSystemReserved(const std::string &name) const {
    return isJvmSystemReserved(name);
}

bool
JvmContainer::isJvmSystemReserved(const std::string &name)
{
  if (name.substr(0,5) == "java/" || name.substr(0,16) == "bootstrap_method") {
    return true;
  }
  return false;
}

std::string
JvmContainer::name() const {
  return "JvmContainer::name():UNIMPLEMENTED";
}

} // namespace
} // namespace
} // namespace

#endif
