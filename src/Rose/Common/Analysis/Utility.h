/// Algorithmic utilities
///



#ifndef ROSE_Common_Analysis_Utility_H
#define ROSE_Common_Analysis_Utility_H 1

namespace
{

  /// pseudo type to indicate that an element is not in a sequence
  struct unavailable_t {};

  /// \brief  traverses two ordered associative sequences in order of their elements.
  ///         The elements in the sequences must be convertible. A merge object
  ///         is called with sequence elements in order of their keys in [aa1, zz1) and [aa2, zz2).
  /// \tparam Iterator1 an iterator of an ordered container
  /// \tparam Iterator2 an iterator of an ordered container
  /// \tparam BinaryOperators a merge object that provides three binary operator()
  ///         functions binding to the following arguments.
  ///         - void operator()(Iterator1::value_type&, unavailable_t);
  ///           called when an element is in sequence 1 but not in sequence 2.
  ///         - void operator()(unavailable_t, Iterator2::value_type&);
  ///           called when an element is in sequence 2 but not in sequence 1.
  ///         - void operator()(Iterator1::value_type&, Iterator2::value_type&);
  ///           called when an element is in both sequences.
  /// \tparam LessThanComparator compares elements in sequences.
  ///         The LessThanComparator needs to support calls with the following arguments:
  ///         - bool operator(Iterator1::value_type&, Iterator2::value_type&)
  ///         - bool operator(Iterator2::value_type&, Iterator1::value_type&)
  /// \param aa1 beginning iterator for the first ordered sequence.
  /// \param zz1 ending iterator for the first ordered sequence.
  /// \param aa2 beginning iterator for the second ordered sequence.
  /// \param zz2 ending iterator for the second ordered sequence.
  /// \param binop merge visitor invoked for elements present in one or both sequences.
  /// \param comp strict weak ordering comparator for cross-sequence elements.
  /// \return the merge visitor after all callbacks have been applied.
  template <class Iterator1, class Iterator2, class BinaryOperators, class LessThanComparator>
  BinaryOperators
  mergeOrderedSequences( Iterator1 aa1, Iterator1 zz1,
                         Iterator2 aa2, Iterator2 zz2,
                         BinaryOperators binop,
                         LessThanComparator comp
                       )
  {
    static constexpr unavailable_t unavail;

    while (aa1 != zz1 && aa2 != zz2)
    {
      if (comp(*aa1, *aa2))
      {
        binop(*aa1, unavail);
        ++aa1;
      }
      else if (comp(*aa2, *aa1))
      {
        binop(unavail, *aa2);
        ++aa2;
      }
      else
      {
        binop(*aa1, *aa2);
        ++aa1; ++aa2;
      }
    }

    while (aa1 != zz1)
    {
      binop(*aa1, unavail);
      ++aa1;
    }

    while (aa2 != zz2)
    {
      binop(unavail, *aa2);
      ++aa2;
    }

    return binop;
  }
}

#endif
