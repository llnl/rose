
#include "edgRose.h"

#undef CHECKING

using namespace std;
using namespace SageInterface;
using namespace SageBuilder;
using namespace OmpSupport;


SourceSequenceList::SourceSequenceList(a_source_sequence_entry_ptr sse): sse(sse), sublist_stack(), printed(false) {}

a_source_sequence_entry_ptr SourceSequenceList::operator->() const
   {
     ROSE_ASSERT (sse != NULL);
     if (!printed) {
        printed = true;
     }

     return sse;
   }

SourceSequenceList::operator bool() const
   {
     return (sse != NULL);
   }


bool
SourceSequenceList::isNull() const
   {
     return (sse == NULL);
   }

a_source_sequence_entry_ptr
SourceSequenceList::value() const
   {
     return sse;
   }

SourceSequenceList& 
SourceSequenceList::operator++()
   {
  // DQ (10/25/2012): Note that debugging statements are helpful in debugging where constructs 
  // have or need to increment over blocks that may or may not be present (e.g. the initial 
  // support for switch detects if the switch contains an explicit body but generates on in 
  // the ROSE translation regardless (but we need to both skip over a possible block and its 
  // end of block as well)).  Debugging this level of detail is supported using the debugging 
  // support containd in this function.

  // As I recall, this flag makes sure that multiple references to the same value of sse 
  // (by using the operator-> for example) does not generate additional output.
     printed = false;

#define DEBUG_USING_SSE 0

     if (sse == NULL)
        {
#if DEBUG_USING_SSE
          printf ("In SourceSequenceList::operator++(): sse == NULL so return this = %p \n",this);
#endif
          return *this;
        }

#if DEBUG_USING_SSE
     printf ("In SourceSequenceList::operator++(): increment to next point in sequence sse = %p = %s sse->next = %p = %s \n",
          sse,summarize_entry_kind_in_string_kind(sse->entity).c_str(),sse->next,(sse->next != NULL) ? summarize_entry_kind_in_string_kind(sse->next->entity).c_str() : "null");

     if (sse->entity.kind == iek_src_seq_end_of_construct)
        {
          a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);
          printf ("In SourceSequenceList::operator++(): associated entry to end_of_construct (before increment) sse = %p = %s \n",
               src_seq_end_of_construct->entity.ptr,summarize_entry_kind_in_string_kind(src_seq_end_of_construct->entity).c_str());
        }
#endif
     sse = sse->next;

#if DEBUG_USING_SSE
     printf ("In SourceSequenceList::operator++(): at test for end of current list: before while (sse == NULL && !sublist_stack.empty()) sse = %p \n",sse);
#endif
     while (sse == NULL && !sublist_stack.empty())
        {
          sse = sublist_stack.back();
          sublist_stack.pop_back();

#if DEBUG_USING_SSE
          printf ("In SourceSequenceList::operator++(): pop sse list stack sse = %p = %s \n",sse,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
        }

     if (sse == NULL)
        {
#if DEBUG_USING_SSE
          printf ("In SourceSequenceList::operator++(): after popping stack sse == NULL so return this = %p \n",this);
#endif
          return *this;
        }

     while (sse->entity.kind == iek_src_seq_sublist)
        {
          sublist_stack.push_back(sse->next);
          sse = ((a_src_seq_sublist_ptr)(sse->entity.ptr))->source_sequence_list;

#if DEBUG_USING_SSE
          printf ("In SourceSequenceList::operator++(): found iek_src_seq_sublist: push to stack sse = %p = %s \n",sse,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
        }

#if DEBUG_USING_SSE
     printf ("Leaving SourceSequenceList::operator++(): returning sse = %p = %s ptr = %p \n",sse,(sse != NULL) ? summarize_entry_kind_in_string_kind(sse->entity).c_str() : "null",(sse != NULL) ? sse->entity.ptr : NULL);
#endif

     return *this;
   }


