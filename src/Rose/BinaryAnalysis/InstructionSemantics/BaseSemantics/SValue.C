#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/SValue.h>

#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Formatter.h>
#include <Rose/BinaryAnalysis/SmtSolver.h>
#include <Rose/BitOps.h>

#include <sstream>

namespace Rose {
namespace BinaryAnalysis {
namespace InstructionSemantics {
namespace BaseSemantics {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SValue::WithFormatter
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SValue::WithFormatter::WithFormatter(const SValue::Ptr &svalue, Formatter &fmt)
    : obj(svalue), fmt(fmt) {}

void
SValue::WithFormatter::print(std::ostream &stream) const {
    obj->print(stream, fmt);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SValue
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SValue::SValue()
    : width(0) {}

SValue::SValue(const size_t nbits)
    : width(nbits) {}

SValue::SValue(const SValue &other)
    : Sawyer::SharedObject(other), width(other.width) {}

SValue& SValue::operator=(const SValue&) = default;
    
SValue::~SValue() {}

SValue::Ptr
SValue::createMerged(const SValue::Ptr &other, const MergerPtr &merger, const SmtSolver::Ptr &solver) const {
    return createOptionalMerge(other, merger, solver).orElse(copy());
}

SValue::Ptr
SValue::promote(const SValue::Ptr &x) {
    ASSERT_not_null(x);
    return x;
}

size_t
SValue::nBits() const {
    return get_width();
}

ValueKind
SValue::kind() const {
    return kind_;
}

void
SValue::kind(ValueKind k) {
    kind_ = k;
}

const std::string&
SValue::typeDescriptor() const {
    return typeDescriptor_;
}

void
SValue::typeDescriptor(const std::string &s) {
    typeDescriptor_ = s;
}

bool
SValue::hasTypeDescriptor() const {
    return typeDescriptor_.size() > 0;
}

const std::string&
SValue::symbolName() const {
    return symbolName_;
}

void
SValue::symbolName(const std::string &s) {
    symbolName_ = s;
}

bool
SValue::hasSymbolName() const {
    return symbolName_.size() > 0;
}

SValuePtr
SValue::arrayLength() const {
    return arrayLength_;
}

void
SValue::arrayLength(const SValuePtr &sval) {
    arrayLength_ = sval;
}

bool
SValue::hasArrayLength() const {
    return arrayLength_ != nullptr;
}

const std::vector<SValuePtr>&
SValue::arrayDimensions() const {
    return arrayDimensions_;
}

void
SValue::arrayDimensions(const std::vector<SValuePtr> &dims) {
    arrayDimensions_ = dims;
}

SValue::WithFormatter
SValue::with_format(Formatter &fmt) {
    return WithFormatter(SValuePtr(this), fmt);
}

SValue::WithFormatter
SValue::operator+(Formatter &fmt) {
    return with_format(fmt);
}

bool
SValue::isConcrete() const {
    return is_number();
}

Sawyer::Optional<uint64_t>
SValue::toUnsigned() const {
    if (isConcrete() && nBits() <= 64) {
        return get_number();
    } else {
        return Sawyer::Nothing();
    }
}

Sawyer::Optional<int64_t>
SValue::toSigned() const {
    if (auto val = toUnsigned()) {
        uint64_t uval = BitOps::signExtend(*val, nBits());
        int64_t ival = static_cast<int64_t>(uval);

        // The above static_cast is implementation defined when uval >= 2^63, but the cast from signed to unsigned doesn't have
        // implementation defined behavior. This allows us to check that the above static cast did what we want.
        ASSERT_require2(static_cast<uint64_t>(ival) == uval, "this compiler's unsigned-to-signed static_cast is strange");
        return ival;
    } else {
        return Sawyer::Nothing();
    }
}

bool
SValue::mustEqual(const SValue::Ptr &other, const SmtSolverPtr &solver) const {
    return must_equal(other, solver);
}

bool
SValue::mayEqual(const SValue::Ptr &other, const SmtSolverPtr &solver) const {
    return may_equal(other, solver);
}

bool
SValue::isTrue() const {
    return toUnsigned().orElse(0) != 0;
}

bool
SValue::isFalse() const {
    return toUnsigned().orElse(1) == 0;
}

std::string
SValue::comment() const {
    return get_comment();
}

void
SValue::comment(const std::string &s) const {
    set_comment(s);
}

std::ostream&
operator<<(std::ostream &o, const SValue &x) {
    x.print(o);
    return o;
}

std::ostream&
operator<<(std::ostream &o, const SValue::WithFormatter &x) {
    x.print(o);
    return o;
}

void
SValue::print(std::ostream &stream) const {
    Formatter fmt;
    print(stream, fmt);
}

SValue::WithFormatter
SValue::operator+(const std::string &linePrefix) {
    static Formatter fmt;
    fmt.set_line_prefix(linePrefix);
    return with_format(fmt);
}

std::string
SValue::toString() const {
    std::ostringstream ss;
    print(ss);
    return ss.str();
}

std::string
toString(ValueKind kind) {
    switch (kind) {
        case ValueKind::Unknown:            return "Unknown";
        case ValueKind::Integer32:          return "Integer32";
        case ValueKind::Integer64:          return "Integer64";
        case ValueKind::NativeInt:          return "NativeInt";
        case ValueKind::Float32:            return "Float32";
        case ValueKind::Float64:            return "Float64";
        case ValueKind::ArrayReference:     return "ArrayReference";
        case ValueKind::ObjectReference:    return "ObjectReference";
        case ValueKind::ManagedPointer:     return "ManagedPointer";
        case ValueKind::UnmanagedPointer:   return "UnmanagedPointer";
        case ValueKind::ReturnAddress:      return "ReturnAddress";
        case ValueKind::Category2Tail:      return "Category2Tail";
        case ValueKind::Invalid:            return "Invalid";
    }
    ASSERT_not_reachable("invalid ValueKind");
}

std::ostream&
operator<<(std::ostream &o, ValueKind kind) {
    return o << toString(kind);
}

std::string
SValue::get_comment() const {
    return "";
}

void
SValue::set_comment(const std::string&) const {};

bool
SValue::isJvmCategory1() {
    return !isJvmCategory2();
}

bool
SValue::isJvmCategory2() {
    return kind() == ValueKind::Integer64 || kind() == ValueKind::Float64;
}

} // namespace
} // namespace
} // namespace
} // namespace

#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT_IMPLEMENT(Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::SValue);
#endif

#endif
