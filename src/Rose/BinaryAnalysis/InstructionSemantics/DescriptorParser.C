#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <Rose/BinaryAnalysis/InstructionSemantics/DescriptorParser.h>

#include <Sawyer/Assert.h>

namespace Rose {
namespace BinaryAnalysis {
namespace InstructionSemantics {

size_t
MethodDescriptor::argumentSlotCount() const {
    size_t nSlots = 0;

    for (const DescriptorType &argument: arguments)
        nSlots += argument.isCategory2() ? 2 : 1;

    return nSlots;
}

DescriptorType
DescriptorParser::parseFieldDescriptor(const std::string &descriptor) {
    ASSERT_require(!descriptor.empty());

    size_t pos = 0;
    DescriptorType result = parseType(descriptor, pos, false);

    ASSERT_require(pos == descriptor.size());
    return result;
}

MethodDescriptor
DescriptorParser::parseMethodDescriptor(const std::string &descriptor) {
    ASSERT_require(!descriptor.empty());

    size_t pos = 0;
    ASSERT_require(descriptor[pos++] == '(');

    MethodDescriptor result;

    while (pos < descriptor.size() && descriptor[pos] != ')') {
        result.arguments.push_back(parseType(descriptor, pos, false));
    }

    ASSERT_require(pos < descriptor.size());
    ASSERT_require(descriptor[pos++] == ')');

    result.returnType = parseType(descriptor, pos, true);

    ASSERT_require(pos == descriptor.size());
    return result;
}

DescriptorType
DescriptorParser::parseType(const std::string &descriptor, size_t &pos, bool allowVoid) {
    ASSERT_require(pos < descriptor.size());

    const size_t begin = pos;
    ValueKind kind = ValueKind::Invalid;

    switch (descriptor[pos++]) {
      case 'B':
      case 'C':
      case 'I':
      case 'S':
      case 'Z':
          kind = ValueKind::Integer32;
          break;
      case 'J':
          kind = ValueKind::Integer64;
          break;
      case 'F':
          kind = ValueKind::Float32;
          break;
      case 'D':
          kind = ValueKind::Float64;
          break;
      case 'L':
          while (pos < descriptor.size() && descriptor[pos] != ';') ++pos;

          ASSERT_require(pos < descriptor.size());
          ++pos; // consume ';'
          kind = ValueKind::ObjectReference;
          break;
      case '[':
          while (pos < descriptor.size() && descriptor[pos] == '[') ++pos;

          ASSERT_require(pos < descriptor.size());

          if (descriptor[pos] == 'L') {
              ++pos;
              while (pos < descriptor.size() && descriptor[pos] != ';') ++pos;

              ASSERT_require(pos < descriptor.size());
              ++pos;                          // consume ';'
          }
          else {
              switch (descriptor[pos++]) {
                case 'B':
                case 'C':
                case 'D':
                case 'F':
                case 'I':
                case 'J':
                case 'S':
                case 'Z':
                    break;
                default: ASSERT_not_reachable("invalid array component descriptor");
              }
          }
          kind = ValueKind::ArrayReference;
          break;
      case 'V':
          ASSERT_require(allowVoid);
          kind = ValueKind::Invalid;
          break;
      default: ASSERT_not_reachable("invalid descriptor");
    }

    DescriptorType result;
    result.kind = kind;
    result.descriptor = descriptor.substr(begin, pos - begin);
    return result;
}

} // namespace InstructionSemantics
} // namespace BinaryAnalysis
} // namespace Rose

#ifdef ROSE_HAVE_BOOST_SERIALIZATION_LIB
BOOST_CLASS_EXPORT_IMPLEMENT(Rose::BinaryAnalysis::InstructionSemantics::DescriptorParser);
#endif

#endif
