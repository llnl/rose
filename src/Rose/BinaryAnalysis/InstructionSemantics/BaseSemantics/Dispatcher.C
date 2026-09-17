#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Dispatcher.h>

#include <Rose/Affirm.h>
#include <Rose/As.h>
#include <Rose/AST/Traversal.h>
#include <Rose/BinaryAnalysis/Architecture/Base.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Exception.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/RegisterStateGeneric.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/RiscOperators.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/State.h>
#include <Rose/BinaryAnalysis/Partitioner2/Engine.h>
#include <Rose/BinaryAnalysis/RegisterDictionary.h>

#include <SgAsmBinaryAdd.h>
#include <SgAsmBinaryAsr.h>
#include <SgAsmBinaryLsl.h>
#include <SgAsmBinaryLsr.h>
#include <SgAsmBinaryMultiply.h>
#include <SgAsmBinaryPostupdate.h>
#include <SgAsmBinaryPreupdate.h>
#include <SgAsmBinaryRor.h>
#include <SgAsmBinarySubtract.h>
#include <SgAsmDirectRegisterExpression.h>
#include <SgAsmIndirectRegisterExpression.h>
#include <SgAsmInstruction.h>
#include <SgAsmIntegerValueExpression.h>
#include <SgAsmMemoryReferenceExpression.h>
#include <SgAsmType.h>
#include <SgAsmUnarySignedExtend.h>
#include <SgAsmUnaryTruncate.h>
#include <SgAsmUnaryUnsignedExtend.h>

#include <Cxx_GrammarDowncast.h>
#include <sageInterface.h>

#include <boost/scope_exit.hpp>

using namespace Sawyer::Message::Common;

namespace Rose {
namespace BinaryAnalysis {
namespace InstructionSemantics {
namespace BaseSemantics {

Dispatcher::Dispatcher() {}

Dispatcher::Dispatcher(const Architecture::Base::ConstPtr &arch)
    : architecture_(arch) {
    ASSERT_not_null(architecture_);
}

Dispatcher::Dispatcher(const Architecture::Base::ConstPtr &arch, const RiscOperators::Ptr &ops)
    : architecture_(arch), operators_(ops) {
    ASSERT_not_null(architecture_);
    ASSERT_not_null(operators_);
}

Dispatcher::~Dispatcher() {
    for (InsnProcessors::iterator iter = iproc_table.begin(); iter != iproc_table.end(); ++iter)
        delete *iter;
}

Architecture::Base::ConstPtr
Dispatcher::architecture() const {
    ASSERT_not_null(architecture_);
    return architecture_;
}

RiscOperators::Ptr
Dispatcher::operators() const {
    ASSERT_not_null(operators_);
    return operators_;
}

void
Dispatcher::operators(const RiscOperators::Ptr &ops) {
    ASSERT_not_null(ops);
    operators_ = ops;
}

State::Ptr
Dispatcher::currentState() const {
    return operators() ? operators()->currentState() : State::Ptr();
}

SValue::Ptr
Dispatcher::protoval() const {
    return operators() ? operators()->protoval() : SValue::Ptr();
}

SgAsmInstruction*
Dispatcher::currentInstruction() const {
    return operators() ? operators()->currentInstruction() : NULL;
}

uint32_t
Dispatcher::allocateObjectId() {
    return nextObjectId_++;
}

const ByteCode::ClassRepository&
Dispatcher::classRepository() const {
    ASSERT_not_null(classes_);
    return *classes_;
}

void
Dispatcher::classRepository(const ByteCode::ClassRepository* repo) {
    classes_ = repo;
}

SValue::Ptr
Dispatcher::undefined_(size_t nBits) const {
    ASSERT_not_null(operators());
    return operators()->undefined_(nBits);
}

SValue::Ptr
Dispatcher::unspecified_(size_t nBits) const {
    ASSERT_not_null(operators());
    return operators()->unspecified_(nBits);
}

SValue::Ptr
Dispatcher::number_(size_t nbits, uint64_t number) const {
    ASSERT_not_null(operators());
    return operators()->number_(nbits, number);
}

int8_t
Dispatcher::asS1(const SgAsmExpression *expr) {
    auto ivExpr = isSgAsmIntegerValueExpression(expr);
    ASSERT_not_null(ivExpr);
    return static_cast<int8_t>(ivExpr->get_signedValue());
}

uint8_t
Dispatcher::asU1(const SgAsmExpression *expr) {
    auto ivExpr = isSgAsmIntegerValueExpression(expr);
    ASSERT_not_null(ivExpr);
    return static_cast<uint8_t>(ivExpr->get_signedValue());
}

int16_t
Dispatcher::asS2(const SgAsmExpression *expr) {
    auto ivExpr = isSgAsmIntegerValueExpression(expr);
    ASSERT_not_null(ivExpr);
    return static_cast<int16_t>(ivExpr->get_signedValue());
}

uint16_t
Dispatcher::asU2(const SgAsmExpression *expr) {
    auto ivExpr = isSgAsmIntegerValueExpression(expr);
    ASSERT_not_null(ivExpr);
    return static_cast<uint16_t>(ivExpr->get_signedValue());
}

int32_t
Dispatcher::asS4(const SgAsmExpression *expr) {
    auto ivExpr = isSgAsmIntegerValueExpression(expr);
    ASSERT_not_null(ivExpr);
    return static_cast<int32_t>(ivExpr->get_signedValue());
}

SValue::Ptr
Dispatcher::makeConstant(ValueKind kind, int64_t value) {
    size_t nBits;

    switch (kind) {
        case ValueKind::Integer32:
            nBits = 32;
            break;
        case ValueKind::Integer64:
            nBits = 64;
            break;
        default: ASSERT_not_reachable("invalid kind for makeConstant");
    }

    // Create the SValue and set its type/kind
    auto sval = operators_->number_(nBits, value);
    ASSERT_not_null(sval);

    sval->kind(kind);
    return sval;
}

RegisterDictionary::Ptr
Dispatcher::registerDictionary() const {
    return architecture()->registerDictionary();
}

size_t
Dispatcher::addressWidth() const {
    return architecture()->bitsPerWord();
}

RegisterDescriptor
Dispatcher::instructionPointerRegister() const {
    return architecture()->registerDictionary()->instructionPointerRegister();
}

RegisterDescriptor
Dispatcher::stackPointerRegister() const {
    return architecture()->registerDictionary()->stackPointerRegister();
}

RegisterDescriptor
Dispatcher::stackFrameRegister() const {
    return architecture()->registerDictionary()->stackFrameRegister();
}

RegisterDescriptor
Dispatcher::callReturnRegister() const {
    return architecture()->registerDictionary()->callReturnRegister();
}

void
Dispatcher::advanceInstructionPointer(SgAsmInstruction *insn) {
    operators()->comment("advancing instruction pointer");
    RegisterDescriptor ipReg = instructionPointerRegister();
    size_t nBits = ipReg.nBits();
    BaseSemantics::SValue::Ptr ipValue;
    if (!autoResetInstructionPointer_ && operators()->currentState() && operators()->currentState()->registerState()) {
        auto grState = as<BaseSemantics::RegisterStateGeneric>(operators()->currentState()->registerState());
        if (grState && grState->is_partly_stored(ipReg))
            ipValue = operators()->readRegister(ipReg);
    }
    if (!ipValue)
        ipValue = operators()->number_(nBits, insn->get_address());
    ipValue = operators()->add(ipValue, operators()->number_(nBits, insn->get_size()));
    operators()->writeRegister(ipReg, ipValue);
}

void
Dispatcher::processCommon() {
    SgAsmInstruction *insn = notnull(operators()->currentInstruction());
    InsnProcessor *iproc = iprocLookup(insn);
    try {
        if (!iproc)
            throw NotImplemented("no semantics for \"" + insn->get_mnemonic() + "\" instruction", insn);
        iproc->process(shared_from_this(), insn);
    } catch (Exception &e) {
        // If the exception was thrown by something that didn't have an instruction available, then add the instruction
        if (!e.insn)
            e.insn = insn;
        e.insn->incrementSemanticFailure();
        throw;
    }
}

void
Dispatcher::processInstruction(SgAsmInstruction *insn) {
    operators()->startInstruction(insn);
    BOOST_SCOPE_EXIT(this_) {
        this_->operators()->currentInstruction(nullptr);
    } BOOST_SCOPE_EXIT_END;

    processCommon();
    operators()->finishInstruction(insn);
}

void
Dispatcher::processDelaySlot(SgAsmInstruction *insn) {
    if (insn) {
        // Remember the outer instruction -- the one that we were previously processing and which has a delay slot. We'll eventually
        // return to processing the outer instruction.
        SgAsmInstruction *outerInsn = operators()->currentInstruction();
        ASSERT_not_null(outerInsn);

        // Raise an interrupt if an illegal (control transfer) instruction is found in the delay slot
        SValue::Ptr raise{operators()->boolean_(false)};
        if (architecture()->isControlTransfer(notnull(insn))) {
            raise = operators()->boolean_(true);
        }
        operators()->raiseInterrupt(mips_signal_exception, mips_reserved_instruction, raise);

        // Execute the inner instruction -- this is the delay slot and is executing within the context of the outer instruction.
        // Before returning (either normally or via exception) make sure we restore the outer instruction. In the case of an
        // exception, the outer instruction might catch the exception from the inner instruction and resume its own processing.
        operators()->comment("starting delay slot");
        operators()->startInstruction(insn);
        BOOST_SCOPE_EXIT(outerInsn, this_) {
            this_->operators()->currentInstruction(outerInsn);
            this_->operators()->comment("resuming after delay slot");
        } BOOST_SCOPE_EXIT_END;

        processCommon();
        operators()->finishInstruction(insn);
    }
}

InsnProcessor *
Dispatcher::iprocLookup(SgAsmInstruction *insn)
{
    int key = iprocKey(insn);
    ASSERT_require(key>=0);
    return iprocGet(key);
}

void
Dispatcher::iprocReplace(SgAsmInstruction *insn, InsnProcessor *iproc)
{
    iprocSet(iprocKey(insn), iproc);
}

void
Dispatcher::iprocSet(int key, InsnProcessor *iproc)
{
    ASSERT_require(key>=0);
    if ((size_t)key>=iproc_table.size())
        iproc_table.resize(key+1, NULL);
    iproc_table[key] = iproc;
}

InsnProcessor *
Dispatcher::iprocGet(int key)
{
    if (key<0 || (size_t)key>=iproc_table.size())
        return NULL;
    return iproc_table[key];
}

RegisterDescriptor
Dispatcher::findRegister(const std::string &regname, size_t nbits/*=0*/, bool allowMissing) const
{
    RegisterDictionary::Ptr regdict = registerDictionary();
    if (!regdict)
        throw Exception("no register dictionary", currentInstruction());

    const RegisterDescriptor reg = regdict->find(regname);
    if (!reg) {
        if (allowMissing)
            return reg;
        std::ostringstream ss;
        ss <<"Invalid register \"" <<regname <<"\" in dictionary \"" <<regdict->name() <<"\"";
        throw Exception(ss.str(), currentInstruction());
    }

    if (nbits>0 && reg.nBits()!=nbits) {
        std::ostringstream ss;
        ss <<"Invalid " <<nbits <<"-bit register: \"" <<regname <<"\" is "
           <<reg.nBits() <<" " <<(1==reg.nBits()?"byte":"bytes");
        throw Exception(ss.str(), currentInstruction());
    }
    return reg;
}

void
Dispatcher::initializeState(const State::Ptr&) {}

void
Dispatcher::decrementRegisters(SgAsmExpression *e) {
    using namespace Rose::AST::Traversal;
    RiscOperators::Ptr ops = operators();
    forwardPre<SgAsmRegisterReferenceExpression>(e, [&ops](SgAsmRegisterReferenceExpression *rre) {
        RegisterDescriptor reg = rre->get_descriptor();
        if (rre->get_adjustment() < 0) {
            SValue::Ptr adj = ops->number_(64, (int64_t)rre->get_adjustment());
            if (reg.nBits() <= 64) {
                adj = ops->unsignedExtend(adj, reg.nBits());  // truncate
            } else {
                adj = ops->signExtend(adj, reg.nBits());      // extend
            }
            ops->writeRegister(reg, ops->add(ops->readRegister(reg), adj));
        }
    });
}

void
Dispatcher::incrementRegisters(SgAsmExpression *e) {
    using namespace Rose::AST::Traversal;
    RiscOperators::Ptr ops = operators();
    forwardPre<SgAsmRegisterReferenceExpression>(e, [&ops](SgAsmRegisterReferenceExpression *rre) {
        RegisterDescriptor reg = rre->get_descriptor();
        if (rre->get_adjustment() > 0) {
            SValue::Ptr adj = ops->unsignedExtend(ops->number_(64, (int64_t)rre->get_adjustment()), reg.nBits());
            ops->writeRegister(reg, ops->add(ops->readRegister(reg), adj));
        }
    });
}

void
Dispatcher::preUpdate(SgAsmExpression *e, const BaseSemantics::SValue::Ptr &enabled) {
    using namespace Rose::AST::Traversal;
    forwardPre<SgAsmBinaryPreupdate>(e, [this, &enabled](SgAsmBinaryPreupdate *op) {
        if (enabled->isTrue()) {
            // Definitely updating, therefore not necessary to read old value
            BaseSemantics::SValue::Ptr newValue = read(op->get_rhs());
            write(op->get_lhs(), newValue);
        } else if (!enabled->isConcrete() || enabled->toUnsigned().get() != 0) {
            // Maybe updating (but not "definitely not updating")
            BaseSemantics::SValue::Ptr oldValue = read(op->get_lhs());
            BaseSemantics::SValue::Ptr newValue = read(op->get_rhs());
            BaseSemantics::SValue::Ptr maybe = operators()->ite(enabled, newValue, oldValue);
            write(op->get_lhs(), maybe);
        }
    });
}

void
Dispatcher::postUpdate(SgAsmExpression *e, const BaseSemantics::SValue::Ptr &enabled) {
    using namespace Rose::AST::Traversal;
    forwardPre<SgAsmBinaryPostupdate>(e, [this, &enabled](SgAsmBinaryPostupdate *op) {
        if (enabled->isTrue()) {
            // Definitely updating, therefore not necessary to read old value
            BaseSemantics::SValue::Ptr newValue = read(op->get_rhs());
            write(op->get_lhs(), newValue);
        } else if (!enabled->isConcrete() || enabled->toUnsigned().get() != 0) {
            // Maybe updating (but not "definitely not updating")
            BaseSemantics::SValue::Ptr oldValue = read(op->get_lhs());
            BaseSemantics::SValue::Ptr newValue = read(op->get_rhs());
            BaseSemantics::SValue::Ptr maybe = operators()->ite(enabled, newValue, oldValue);
            write(op->get_lhs(), maybe);
        }
    });
}
    
SValue::Ptr
Dispatcher::effectiveAddress(SgAsmExpression *e, size_t nbits/*=0*/)
{
    BaseSemantics::SValue::Ptr retval;
#if 1 // DEBUGGING [Robb P. Matzke 2015-08-04]
    ASSERT_always_require(retval==NULL);
#endif
    if (0==nbits)
        nbits = addressWidth();

    if (SgAsmMemoryReferenceExpression *mre = isSgAsmMemoryReferenceExpression(e)) {
        retval = effectiveAddress(mre->get_address(), nbits);
    } else if (SgAsmRegisterReferenceExpression *rre = isSgAsmRegisterReferenceExpression(e)) {
        retval = read(rre, nbits);
    } else if (SgAsmBinaryAdd *op = isSgAsmBinaryAdd(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_lhs(), nbits);
        BaseSemantics::SValue::Ptr rhs = effectiveAddress(op->get_rhs(), nbits);
        retval = operators()->add(lhs, rhs);
    } else if (SgAsmBinarySubtract *op = isSgAsmBinarySubtract(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_lhs(), nbits);
        BaseSemantics::SValue::Ptr rhs = effectiveAddress(op->get_rhs(), nbits);
        retval = operators()->subtract(lhs, rhs);
    } else if (SgAsmBinaryPreupdate *op = isSgAsmBinaryPreupdate(e)) {
        // The add operation is ignored here, but performed by preUpdate.
        retval = effectiveAddress(op->get_lhs(), nbits);
    } else if (SgAsmBinaryPostupdate *op = isSgAsmBinaryPostupdate(e)) {
        // The add operation is ignored here, but performed by postUpdate.
        retval = effectiveAddress(op->get_lhs(), nbits);
    } else if (SgAsmBinaryMultiply *op = isSgAsmBinaryMultiply(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_lhs(), nbits);
        BaseSemantics::SValue::Ptr rhs = effectiveAddress(op->get_rhs(), nbits);
        retval = operators()->unsignedMultiply(lhs, rhs);
    } else if (SgAsmIntegerValueExpression *ival = isSgAsmIntegerValueExpression(e)) {
        retval = operators()->number_(ival->get_significantBits(), ival->get_value());
    } else if (SgAsmUnaryUnsignedExtend *op = isSgAsmUnaryUnsignedExtend(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_operand(), op->get_operand()->get_nBits());
        retval = operators()->unsignedExtend(lhs, op->get_nBits());
    } else if (SgAsmUnarySignedExtend *op = isSgAsmUnarySignedExtend(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_operand(), op->get_operand()->get_nBits());
        retval = operators()->signExtend(lhs, op->get_nBits());
    } else if (SgAsmUnaryTruncate *op = isSgAsmUnaryTruncate(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_operand(), op->get_operand()->get_nBits());
        retval = operators()->unsignedExtend(lhs, op->get_nBits()); // yes, can be used for truncation
    } else if (SgAsmBinaryLsl *op = isSgAsmBinaryLsl(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_lhs(), nbits);
        BaseSemantics::SValue::Ptr rhs = effectiveAddress(op->get_rhs(), nbits);
        retval = operators()->shiftLeft(lhs, rhs);
    } else if (SgAsmBinaryLsr *op = isSgAsmBinaryLsr(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_lhs(), nbits);
        BaseSemantics::SValue::Ptr rhs = effectiveAddress(op->get_rhs(), nbits);
        retval = operators()->shiftRight(lhs, rhs);
    } else if (SgAsmBinaryAsr *op = isSgAsmBinaryAsr(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_lhs(), nbits);
        BaseSemantics::SValue::Ptr rhs = effectiveAddress(op->get_rhs(), nbits);
        retval = operators()->shiftRightArithmetic(lhs, rhs);
    } else if (SgAsmBinaryRor *op = isSgAsmBinaryRor(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_lhs(), nbits);
        BaseSemantics::SValue::Ptr rhs = effectiveAddress(op->get_rhs(), nbits);
        retval = operators()->rotateRight(lhs, rhs);
    } else if (SgAsmUnaryUnsignedExtend *op = isSgAsmUnaryUnsignedExtend(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_operand(), op->get_operand()->get_nBits());
        retval = operators()->unsignedExtend(lhs, op->get_nBits());
    }

    ASSERT_not_null(retval);
    if (retval->nBits() < nbits) {
        retval = operators()->signExtend(retval, nbits);
    } else if (retval->nBits() > nbits) {
        retval = operators()->extract(retval, 0, nbits);
    }
    return retval;
}

RegisterDescriptor
Dispatcher::segmentRegister(SgAsmMemoryReferenceExpression *mre)
{
    if (mre!=NULL && mre->get_segment()!=NULL) {
        if (SgAsmRegisterReferenceExpression *rre = isSgAsmRegisterReferenceExpression(mre->get_segment())) {
            return rre->get_descriptor();
        }
    }
    return RegisterDescriptor();
}

void
Dispatcher::write(SgAsmExpression *e, const SValue::Ptr &value, size_t addr_nbits/*=0*/)
{
    ASSERT_not_null(e);
    ASSERT_not_null(value);
    if (SgAsmDirectRegisterExpression *re = isSgAsmDirectRegisterExpression(e)) {
        // If the expression type is different than the register lvalue, we need to adjust what part of the register we're writing.
        RegisterDescriptor reg = re->get_descriptor();
        if (reg.nBits() > e->get_nBits())
            reg = RegisterDescriptor(reg.majorNumber(), reg.minorNumber(), reg.offset(), e->get_nBits());
        operators()->writeRegister(reg, value);
    } else if (SgAsmIndirectRegisterExpression *re = isSgAsmIndirectRegisterExpression(e)) {
        SValue::Ptr offset = operators()->readRegister(re->get_offset());
        if (!offset->isConcrete()) {
            std::string offset_name = registerDictionary()->lookup(re->get_offset());
            offset_name = offset_name.empty() ? "" : "(" + offset_name + ") ";
            throw Exception("indirect register offset " + offset_name + "must have a concrete value", NULL);
        }
        size_t idx = (offset->toUnsigned().get() + re->get_index()) % re->get_modulus();
        RegisterDescriptor reg = re->get_descriptor();
        reg.majorNumber(reg.majorNumber() + re->get_stride().majorNumber() * idx);
        reg.minorNumber(reg.minorNumber() + re->get_stride().minorNumber() * idx);
        reg.offset(reg.offset() + re->get_stride().offset() * idx);
        reg.nBits(reg.nBits() + re->get_stride().nBits() * idx);
        operators()->writeRegister(reg, value);
    } else if (SgAsmMemoryReferenceExpression *mre = isSgAsmMemoryReferenceExpression(e)) {
        SValue::Ptr addr = effectiveAddress(mre, addr_nbits);
        ASSERT_require(addr->nBits() == architecture()->bitsPerWord());
        operators()->writeMemory(segmentRegister(mre), addr, value, operators()->boolean_(true));
    } else {
        ASSERT_not_implemented(e->class_name());
    }
}

SValue::Ptr
Dispatcher::read(SgAsmExpression *e, size_t value_nbits/*=0*/, size_t addr_nbits/*=0*/)
{
    ASSERT_not_null(e);
    if (0 == value_nbits) {
        SgAsmType *expr_type = e->get_type();
        ASSERT_not_null(expr_type);
        value_nbits = expr_type->get_nBits();
        ASSERT_require(value_nbits != 0);
    }

    SValue::Ptr retval;
    if (SgAsmDirectRegisterExpression *re = isSgAsmDirectRegisterExpression(e)) {
        retval = operators()->readRegister(re->get_descriptor());
    } else if (SgAsmIndirectRegisterExpression *re = isSgAsmIndirectRegisterExpression(e)) {
        SValue::Ptr offset = operators()->readRegister(re->get_offset());
        if (!offset->isConcrete()) {
            std::string offset_name = registerDictionary()->lookup(re->get_offset());
            offset_name = offset_name.empty() ? "" : "(" + offset_name + ") ";
            throw Exception("indirect register offset " + offset_name + "must have a concrete value", NULL);
        }
        size_t idx = (offset->toUnsigned().get() + re->get_index()) % re->get_modulus();
        RegisterDescriptor reg = re->get_descriptor();
        reg.majorNumber(reg.majorNumber() + re->get_stride().majorNumber() * idx);
        reg.minorNumber(reg.minorNumber() + re->get_stride().minorNumber() * idx);
        reg.offset(reg.offset() + re->get_stride().offset() * idx);
        reg.nBits(reg.nBits() + re->get_stride().nBits() * idx);
        retval = operators()->readRegister(reg);
    } else if (SgAsmMemoryReferenceExpression *mre = isSgAsmMemoryReferenceExpression(e)) {
        BaseSemantics::SValue::Ptr addr = effectiveAddress(mre, addr_nbits);
        ASSERT_require(addr->nBits() == architecture()->bitsPerWord());
        BaseSemantics::SValue::Ptr dflt = undefined_(value_nbits);
        retval = operators()->readMemory(segmentRegister(mre), addr, dflt, operators()->boolean_(true));
    } else if (SgAsmValueExpression *ve = isSgAsmValueExpression(e)) {
        uint64_t val = SageInterface::getAsmSignedConstant(ve);
        retval = operators()->number_(value_nbits, val);
    } else if (SgAsmBinaryAdd *sum = isSgAsmBinaryAdd(e)) {
        SgAsmExpression *lhs = sum->get_lhs();
        SgAsmExpression *rhs = sum->get_rhs();
        size_t nbits = std::max(lhs->get_nBits(), rhs->get_nBits());
        retval = operators()->add(operators()->signExtend(read(lhs, lhs->get_nBits(), addr_nbits), nbits),
                                  operators()->signExtend(read(rhs, rhs->get_nBits(), addr_nbits), nbits));
    } else if (SgAsmBinarySubtract *diff = isSgAsmBinarySubtract(e)) {
        SgAsmExpression *lhs = diff->get_lhs();
        SgAsmExpression *rhs = diff->get_rhs();
        size_t nbits = std::max(lhs->get_nBits(), rhs->get_nBits());
        retval = operators()->subtract(operators()->signExtend(read(lhs, lhs->get_nBits(), addr_nbits), nbits),
                                       operators()->signExtend(read(rhs, rhs->get_nBits(), addr_nbits), nbits));
    } else if (SgAsmBinaryMultiply *product = isSgAsmBinaryMultiply(e)) {
        SgAsmExpression *lhs = product->get_lhs();
        SgAsmExpression *rhs = product->get_rhs();
        retval = operators()->unsignedMultiply(read(lhs, lhs->get_nBits()), read(rhs, rhs->get_nBits()));
    } else if (SgAsmUnaryTruncate *op = isSgAsmUnaryTruncate(e)) {
        BaseSemantics::SValue::Ptr operand = read(op->get_operand(), op->get_operand()->get_nBits(), addr_nbits);
        retval = operators()->unsignedExtend(operand, op->get_nBits());
    } else if (SgAsmBinaryLsr *op = isSgAsmBinaryLsr(e)) {
        BaseSemantics::SValue::Ptr lhs = read(op->get_lhs(), op->get_lhs()->get_nBits(), addr_nbits);
        BaseSemantics::SValue::Ptr rhs = read(op->get_rhs(), op->get_rhs()->get_nBits(), addr_nbits);
        retval = operators()->shiftRight(lhs, rhs);
    } else if (SgAsmBinaryAsr *op = isSgAsmBinaryAsr(e)) {
        BaseSemantics::SValue::Ptr lhs = read(op->get_lhs(), op->get_lhs()->get_nBits(), addr_nbits);
        BaseSemantics::SValue::Ptr rhs = read(op->get_rhs(), op->get_rhs()->get_nBits(), addr_nbits);
        retval = operators()->shiftRightArithmetic(lhs, rhs);
    } else if (SgAsmBinaryLsl *op = isSgAsmBinaryLsl(e)) {
        BaseSemantics::SValue::Ptr lhs = read(op->get_lhs(), op->get_lhs()->get_nBits(), addr_nbits);
        BaseSemantics::SValue::Ptr rhs = read(op->get_rhs(), op->get_rhs()->get_nBits(), addr_nbits);
        retval = operators()->shiftLeft(lhs, rhs);
    } else if (SgAsmBinaryRor *op = isSgAsmBinaryRor(e)) {
        BaseSemantics::SValue::Ptr lhs = read(op->get_lhs(), op->get_lhs()->get_nBits(), addr_nbits);
        BaseSemantics::SValue::Ptr rhs = read(op->get_rhs(), op->get_rhs()->get_nBits(), addr_nbits);
        retval = operators()->rotateRight(lhs, rhs);
    } else if (SgAsmUnaryUnsignedExtend *op = isSgAsmUnaryUnsignedExtend(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_operand(), op->get_operand()->get_nBits());
        retval = operators()->unsignedExtend(lhs, op->get_nBits());
    } else if (SgAsmUnarySignedExtend *op = isSgAsmUnarySignedExtend(e)) {
        BaseSemantics::SValue::Ptr lhs = effectiveAddress(op->get_operand(), op->get_operand()->get_nBits());
        retval = operators()->signExtend(lhs, op->get_nBits());
    } else if (SgAsmBinaryPostupdate *op = isSgAsmBinaryPostupdate(e)) {
        return read(op->get_lhs(), value_nbits, addr_nbits);
    } else if (SgAsmBinaryPreupdate *op = isSgAsmBinaryPreupdate(e)) {
        return read(op->get_lhs(), value_nbits, addr_nbits);
    } else {
        ASSERT_not_implemented(e->class_name());
    }

    // Make sure the return value is the requested width. The unsignedExtend() can expand or shrink values.
    ASSERT_not_null(retval);
    if (retval->nBits()!=value_nbits)
        retval = operators()->unsignedExtend(retval, value_nbits);
    return retval;
}

} // namespace
} // namespace
} // namespace
} // namespace

#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT_IMPLEMENT(Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::Dispatcher);
#endif

#endif
