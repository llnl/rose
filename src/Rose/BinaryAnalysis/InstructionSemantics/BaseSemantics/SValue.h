#ifndef ROSE_BinaryAnalysis_InstructionSemantics_BaseSemantics_SValue_H
#define ROSE_BinaryAnalysis_InstructionSemantics_BaseSemantics_SValue_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Rose/BinaryAnalysis/BasicTypes.h>
#include <Combinatorics.h>                              // ROSE

#include <Sawyer/SharedObject.h>
#include <Sawyer/SmallObject.h>

#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#endif

namespace Rose {
namespace BinaryAnalysis {
namespace InstructionSemantics {
namespace BaseSemantics {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     CIL and JVM Semantic Values
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Kind of an SValue for emulating JVM and CIL instructions. */
enum class ValueKind {
    Unknown,
    Integer32,       // JVM Integer
    Integer64,       // JVM Long
    NativeInt,       // CIL native integer
    Float32,         // JVM Float
    Float64,         // JVM Double
    ArrayReference,
    ObjectReference,
    ManagedPointer,
    UnmanagedPointer,
    ReturnAddress,

    Category2Tail,   // second slot of long/double
    Invalid          // unusable/destroyed/corrupted slot
};

/** Nullness state of a reference value. */
enum class Nullness {
    DefinitelyNull,
    MaybeNull,
    DefinitelyNonNull
};

/** Prints a ValueKind to a string. */
std::string toString(ValueKind);

/** Prints a ValueKind to an output stream. */
std::ostream& operator<<(std::ostream&, ValueKind);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Semantic Values
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Base class for semantic values.
 *
 *  A semantic value represents a datum from the specimen being analyzed. The datum could be from memory, it could be something
 *  stored in a register, it could be the result of some computation, etc.  The datum in the specimen has a datum type that
 *  might be only partially known; the datum value could, for instance, be 32-bits but unknown whether it is integer or
 *  floating point.
 *
 *  The various semantic domains will define SValue subclasses that are appropriate for that domain--a concrete domain will
 *  define an SValue that specimen data in a concrete form, an interval domain will define an SValue that represents specimen
 *  data in intervals, etc.
 *
 *  Semantics value objects are allocated on the heap and reference counted.  The BaseSemantics::SValue is an abstract class
 *  that defines the interface.  See the Rose::BinaryAnalysis::InstructionSemantics namespace for an overview of how the parts
 *  fit together.*/
class SValue: public Sawyer::SharedObject, public Sawyer::SharedFromThis<SValue>, public Sawyer::SmallObject {
public:
    /** Shared-ownership pointer. */
    using Ptr = SValuePtr;

protected:
    size_t width;                               /** Width of the value in bits. Typically (not always) a power of two. */
    ValueKind kind_ = ValueKind::Unknown;       /** Kind of the value type. */
    Nullness nullness_ = Nullness::MaybeNull;   /** Nullness state for reference-like values. */
    SValuePtr arrayLength_;                     /** Symbolic length of the array if an ArrayReference. */
    std::vector<SValuePtr> arrayDimensions_;    /** Known lengths of nested array dimensions. */
    std::string typeDescriptor_;   /** Description of the type if a Reference, empty means no descriptor. */
    std::string symbolName_;       /** Name of the symbolic program entity represented by this value; empty means none. */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Serialization
#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, const unsigned /*version*/) {
        s & BOOST_SERIALIZATION_NVP(width);
        s & BOOST_SERIALIZATION_NVP(kind_);
        s & BOOST_SERIALIZATION_NVP(nullness_);
        s & BOOST_SERIALIZATION_NVP(arrayLength_);
        s & BOOST_SERIALIZATION_NVP(arrayDimensions_);
        s & BOOST_SERIALIZATION_NVP(typeDescriptor_);
        s & BOOST_SERIALIZATION_NVP(symbolName_);
    }
#endif
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Normal, protected, C++ constructors
protected:
    SValue();                                           // needed for serialization
    explicit SValue(size_t nbits);                      // hot
    SValue(const SValue&);
    SValue& operator=(const SValue&);

public:
    virtual ~SValue();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Allocating static constructor.  None are needed--this class is abstract.

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Allocating virtual constructors.  undefined_() needs underscores, so we do so consistently for all
    // these allocating virtual c'tors.  However, we use copy() rather than copy_() because this one is fundamentally
    // different: the object (this) is used for more than just selecting which virtual method to invoke.
    //
    // The naming scheme we use here is a bit different than for most other objects for historical reasons.  Most other classes
    // use "create" and "clone" as the virtual constructor names, but SValue uses names ending in undercore, and "copy". The
    // other difference (at least in this base class) is that we don't define any real constructors or static allocating
    // constructors (usually named "instance")--it's because this is an abstract class.
public:
    /** Create a new undefined semantic value.  The new semantic value will have the same dynamic type as the value
     *  on which this virtual method is called.  This is the most common way that a new value is created. The @ref unspecified_
     *  method is closely related.
     *
     *  @sa unspecified_ */
    virtual SValuePtr undefined_(size_t nbits) const = 0; // hot

    /** Create a new unspecified semantic value. The new semantic value will have the same dynamic type as the value on which
     *  this virtual method is called.  Undefined (@ref undefined_) and unspecified are closely related.  Unspecified values
     *  are the same as undefined values except they're instantiated as the result of some machine instruction where the ISA
     *  documentation indicates that the value is unspecified (e.g., status flags for x86 shift and rotate instructions).
     *
     *  Most semantic domains make no distinction between undefined and unspecified.
     *
     *  @sa undefined_ */
    virtual SValuePtr unspecified_(size_t nbits) const = 0;

    /** Data-flow bottom value.
     *
     *  Returns a new value that represents bottom in a data-flow analysis. If a semantic domain can represent a bottom value
     *  then the @ref isBottom predicate is true when invoked on this method's return value. If a semantic domain cannot
     *  support a bottom value, then it may return some other value. */
    virtual SValuePtr bottom_(size_t nBits) const = 0;

    /** Create a new concrete semantic value. The new value will represent the specified concrete value and have the same
     *  dynamic type as the value on which this virtual method is called. This is the most common way that a new constant is
     *  created.  The @p number is truncated to contain @p nbits bits (higher order bits are cleared). */
    virtual SValuePtr number_(size_t nbits, uint64_t number) const = 0; // hot

    /** Create a new, Boolean value. The new semantic value will have the same dynamic type as the value on
     *  which this virtual method is called. This is how 1-bit flag register values (among others) are created. The base
     *  implementation uses number_() to construct a 1-bit value whose bit is zero (false) or one (true). */
    virtual SValuePtr boolean_(bool value) const { return number_(1, value?1:0); }

    /** Create a new value from an existing value, changing the width if @p new_width is non-zero. Increasing the width
     *  logically adds zero bits to the most significant side of the value; decreasing the width logically removes bits from the
     *  most significant side of the value. */
    virtual SValuePtr copy(size_t new_width=0) const = 0;

    /** Possibly create a new value by merging two existing values.
     *
     *  This method optionally returns a new semantic value as the data-flow merge of @p this and @p other.  If the two inputs
     *  are "equal" in some sense of the dataflow implementation then nothing is returned, otherwise a new value is returned.
     *  Typical usage is like this:
     *
     * @code
     *  if (SValuePtr merged = v1->createOptionalMerge(v2).orDefault()) {
     *      std::cout <<"v1 and v2 were merged to " <<*merged <<"\n";
     *  } else {
     *      std::cout <<"no merge is necessary\n";
     *  }
     *
     *  or
     *
     * @code
     *  SValuePtr merge;
     *  if (v1->createOptionalMerge(v2).assignTo(merged)) {
     *      std::cout <<"v1 and v2 were merged to " <<*merged <<"\n";
     *  } else {
     *      std::cout <<"v1 and v2 are equal in some sense (no merge necessary)\n";
     *  }
     * @endcode
     *
     *  If you always want a copy regardless of whether the merge is necessary, then use the @ref createMerged convenience
     *  function instead. */
    virtual Sawyer::Optional<SValuePtr>
    createOptionalMerge(const SValuePtr &other, const MergerPtr &merger, const SmtSolverPtr &solver) const = 0;

    /** Create a new value by merging two existing values.
     *
     *  This is a convenience wrapper around @ref createOptionalMerge. It always returns a newly constructed semantic value
     *  regardless of whether a merge was necessary.  In order to determine if a merge was necessary one can compare the
     *  return value to @p this using @ref must_equal, although doing so is more expensive than calling @ref
     *  createOptionalMerge. */
    SValuePtr createMerged(const SValuePtr &other, const MergerPtr&, const SmtSolverPtr&) const /*final*/;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Dynamic pointer casts. No-ops since this is the base class
public:
    static SValuePtr promote(const SValuePtr&);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // The rest of the API...
public:
    /** Property: value width.
     *
     *  Returns the width of the value in bits. One generally doesn't change the width after a value is created.
     *
     * This function should not be overridden by subclasses. Instead, it calls @ref get_width to do its work. */
    size_t nBits() const /*final*/;

    /** Property: value kind. */
    virtual ValueKind kind() const;
    virtual void kind(ValueKind k);

    /** Property: array length. */
    virtual void arrayLength(const SValuePtr &sval);
    virtual SValuePtr arrayLength() const;
    virtual bool hasArrayLength() const;

    /** Property: array dimensions. */
    virtual void arrayDimensions(const std::vector<SValuePtr> &dims);
    virtual const std::vector<SValuePtr>& arrayDimensions() const;

    /** Property: type descriptor. */
    virtual void typeDescriptor(const std::string &s);
    virtual const std::string& typeDescriptor() const;
    virtual bool hasTypeDescriptor() const;

    /** Property: symbol name. */
    virtual void symbolName(const std::string &s);
    virtual const std::string& symbolName() const;
    virtual bool hasSymbolName() const;

    /** Determines whether a value is JVM category-1.
     *
     *  Returns true if this value represents a JVM category-1 object, with kind not ValueKind::Integer64 nor
     *  ValueKind::Float64.  It has no meaning for other architectures. */
    bool isJvmCategory1();

    /** Determines whether a value is JVM category-2.
     *
     *  Returns true if this value represents a JVM category-2 object, with kind either ValueKind::Integer64 or
     *  ValueKind::Float64.  If so the value will require two slots when stored.  It has no meaning for other
     *  architectures. */
    bool isJvmCategory2();

    /** Determines whether a value is a data-flow bottom.
     *
     *  Returns true if this value represents a bottom value for data-flow analysis.  Any RiscOperation performed on an operand
     *  whose isBottom predicate returns true will itself return a bottom value.  This includes operations like "xor x x" which
     *  would normally return zero. */
    virtual bool isBottom() const = 0;

    /** Determines if the value is a concrete number.
     *
     *  Concrete numbers can be created with the @ref number_ and @ref boolean_ methods, virtual constructors, or other
     *  means. Some semantic domains have only concrete numbers.
     *
     *  This function should not be overridden by subclasses. Instead, it calls the virtual @ref is_number method to do its
     *  work. */
    bool isConcrete() const /*final*/;

    /** Converts a concrete value to a native unsigned integer.
     *
     *  If this is a concrete value (see @ref isConcrete) and an unsigned integer interpretation of the bits fits in a 64-bit
     *  unsigned integer (@c uint64_t) then that interpretation is returned, otherwise nothing is returned.
     *
     *  This function should not be overridden by subclasses. Instead, it calls @ref get_number to do its work. */
    Sawyer::Optional<uint64_t> toUnsigned() const /*final*/;

    /** Converts a concrete value to a native signed integer.
     *
     *  If this is a concrete value (see @ref isConcrete) and a signed integer interpretation of the bits fits in a 64-bit
     *  signed integer (@c int64_t) then that interpretation is returned, otherwise nothing is returned.
     *
     *  This function should not be overridden by subclasses. Instead, it calls @ref get_number to do its work. */
    Sawyer::Optional<int64_t> toSigned() const /*final*/;

    /** Tests two values for equality.
     *
     *  Returns true if this value and the @p other value must be equal, and false if they might not be equal.
     *
     *  This function should not be overridden by subclasses. Instead, it calls @ref must_equal to do its work. */
    bool mustEqual(const SValuePtr &other, const SmtSolverPtr &solver = SmtSolverPtr()) const /*final*/;

    /** Tests two values for possible equality.
     *
     *  Returns true if this value and the @p other value might be equal, and false if they cannot be equal.
     *
     *  This function should not be overridden by subclasses. Instead, it calls @ref may_equal to do its work. */
    bool mayEqual(const SValuePtr &other, const SmtSolverPtr &solver = SmtSolverPtr()) const /*final*/;

    /** Returns true if concrete non-zero.
     *
     *  This is not virtual since it can be implemented in terms of other functions. */
    bool isTrue() const /*final*/;

    /** Returns true if concrete zero.
     *
     *  This is not virtual since it can be implemented in terms of other functions. */
    bool isFalse() const /*final*/;

    /** Property: Nullness.
     *
     *  Whether a value is definitely null, definitely non-null, or maybe null.
     */
    Nullness nullness() const;
    void nullness(Nullness value);

    /** Property: Comment.
     *
     *  Some subclasses support associating a string comment with a value.  If the class does not support comments, then
     *  this property will always have an empty value regardless of what's assigned. Since comments do not affect computation,
     *  their value is allowed to be changed even for const objects.
     *
     *  This function should not be overridden by subclasses. Instead, it calls @ref get_comment and @ref set_comment to do its
     *  work.
     *
     * @{ */
    std::string comment() const /*final*/;
    void comment(const std::string&) const /*final*/;   // const is intentional (see documentation)
    /** @} */

    /** Hash this semantic value.
     *
     *  Hashes the value by appending it to the specified hasher. */
    virtual void hash(Combinatorics::Hasher&) const = 0;

    /** Print a value to a stream using default format. The value will normally occupy a single line and not contain leading
     *  space or line termination.  See also, with_format().
     *  @{ */
    void print(std::ostream&) const;
    virtual void print(std::ostream&, Formatter&) const = 0;
    /** @} */

    /** Render this symbolic expression as a string.
     *
     *  Don't use this if you're just printing the value, since it's more efficient to stream the value with operator<< or using
     *  the @ref print method. */
    std::string toString() const;

    /** SValue with formatter. See with_formatter(). */
    class WithFormatter {
        SValuePtr obj;
        Formatter &fmt;
    public:
        WithFormatter(const SValuePtr&, Formatter&);
        void print(std::ostream&) const;
    };

    /** Used for printing values with formatting. The usual way to use this is:
     * @code
     *  SValuePtr val = ...;
     *  Formatter fmt = ...;
     *  std::cout <<"The value is: " <<(*val+fmt) <<"\n";
     * @endcode
     * @{ */
    WithFormatter with_format(Formatter&);
    WithFormatter operator+(Formatter&);
    WithFormatter operator+(const std::string &linePrefix);
    /** @} */
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // This is the virtual interface that uses names that are not consistent with most of the rest of binary analysis.  Calling
    // these directly is deprecated and we may make them protected at some time. [Robb Matzke 2021-03-18].
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public: // for backward compatibility for now, but assume protected
    /** Virtual API. See @ref isConcrete. */
    virtual bool is_number() const = 0;

    /** Virtual API. See @ref toUnsigned and @ref toSigned. */
    virtual uint64_t get_number() const = 0;

    /** Virtual API. See @ref nBits.
     *
     *  @{ */
    virtual size_t get_width() const { return width; }
    virtual void set_width(size_t nbits) { width = nbits; }
    /** @} */

    /** Virtual API. See @ref mustEqual. */
    virtual bool must_equal(const SValuePtr &other, const SmtSolverPtr &solver = SmtSolverPtr()) const = 0;

    /** Virtual API. See @ref mayEqual. */
    virtual bool may_equal(const SValuePtr &other, const SmtSolverPtr &solver = SmtSolverPtr()) const = 0;

    /** Some subclasses support the ability to add comments to values. We define no-op versions of these methods here
     *  because it makes things easier.  The base class tries to be as small as possible by not storing comments at
     *  all. Comments should not affect any computation (comparisons, hash values, etc), and therefore are allowed to be
     *  modified even for const objects.
     * @{ */
    virtual std::string get_comment() const;
    virtual void set_comment(const std::string&) const; // const is intended; cf. doxygen comment
    /** @} */
};

std::ostream& operator<<(std::ostream&, const SValue&);
std::ostream& operator<<(std::ostream&, const SValue::WithFormatter&);

} // namespace
} // namespace
} // namespace
} // namespace

#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::SValue);
#endif

#endif
#endif
