
#include "edgRose.h"

#define DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR 0

vector<SgStatement*>
EDG_ROSE_Translation::parse_statement_list_without_terminator(SourceSequenceList& sse, a_statement_ptr enclosing_block, a_statement_ptr statement_list, a_statement_ptr last_stmt)
   {
  // DQ (3/2/2013): This function is now also called in processing the switch statement (so we can later properly handle Duff's Device like switch statement complexity).

  // Note: This is only called for the processing in parse_statement(): case stmk_block.

  // Parse a statement list that may not have an end-of-construct entry (from a
  // compiler-generated block); skip the end-of-construct entry if it is the
  // correct one.  Based on gen_statement_list from cp_gen_be.c, we parse
  // statements until the last known statement (if it is not NULL), and then
  // parse switch clauses (?), pragmas, and macros after that.  Return sse set
  // to the first thing that is unrecognized.

  // DQ (2/2/2013): I would like to unify this function with the parse_statement_list_in_switch_body() 
  // function 
  // vector<SgStatement*> parse_statement_list_in_switch_body(SourceSequenceList& sse, a_statement_ptr last_stmt)

  // DQ (7/30/2013): Keep track of the depth of the number of recursive calls.
     static int depthCounter = 0;
     depthCounter++;

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("statement list without terminator depth = %d %p - %p, inside %p\n",depthCounter,statement_list, last_stmt, enclosing_block);
     indent_line(); debug_print("+ last statement source seq entry should be %p\n", last_stmt ? last_stmt->source_sequence_entry : NULL);

     if (last_stmt != NULL)
        {
          db_statement(last_stmt);
        }
#endif

#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR || 0
     printf ("########## In parse_statement_list_without_terminator(): depthCounter = %d enclosing_block = %p statement_list = %p last_stmt = %p \n",depthCounter,enclosing_block,statement_list,last_stmt);
#endif


#if NEW_DUFF_DEVICE_SWITCH_SUPPORT
  // DQ (12/18/2017): This code could be refactored with the code in the parse_statement_list().

  // DQ (11/24/2017): Build list of statements in block (so that we include switch case statements) and then mark 
  // off the ones that we visit as source sequence points. The ones not marked off are statements that are not 
  // source sequence points, and at least when these are switch case statements, we need to include them in the
  // translation to ROSE.  The will have to be interleved into the generated statement list for this block.
     vector<a_statement_ptr> block_list_perspective;

  // vector<a_statement_ptr> block_source_sequence_perspective;
     vector<a_source_sequence_entry_ptr> block_source_sequence_perspective;

  // DQ (11/26/2017): Added map to detect where to interleave switch case statements into blocks.
  // map<a_statement_ptr,SgStatement*> block_statement_map;
     map<a_source_sequence_entry_ptr,SgStatement*> block_statement_map;

  // Use a boolean variable to control if we will use the logic to weave the case statements into the resulting statement sequence.
     bool stmk_switch_case_to_process = false;

     if (statement_list != NULL)
        {
          for (a_statement_ptr s = statement_list; s != NULL; s = s->next)
             {
#ifdef DEBUG_CONVERSION
            // indent_line(); fprintf(stderr, "In parse_statement(): case stmk_block: + "); db_statement(s); debug_print("\n");
            // indent_line(); printf("In parse_statement(): case stmk_block: + "); db_statement(s); debug_print("\n");
               indent_line(); printf("In parse_statement_list_without_terminator(): statement_list != NULL: call db_statement(s = %p = %s) \n",s,summarize_statement_in_string(s)); // debug_print("\n");
#endif

            // DQ (11/24/2017): Detect the case of a switch body's stmk_switch_case statement.
               if (s->kind == stmk_switch_case)
                  {
                 // printf ("WARNING: In parse_statement_list_without_terminator(): case of switch body containing stmk_switch_case is not handled in new duff's device support \n");
                 // ROSE_ASSERT(false);
#if 0
                    printf ("In parse_statement(): case stmk_block: Calling summarize_statement(s = %p) \n",s);
                    summarize_statement(s);
#endif
                    stmk_switch_case_to_process = true;
                  }

               block_list_perspective.push_back(s);
             }
        }

#if 0
     printf ("In parse_statement_list_without_terminator(): block_list_perspective.size() = %zu \n",block_list_perspective.size());
#endif

// endif for NEW_DUFF_DEVICE_SWITCH_SUPPORT
#endif


  // DQ (1/15/2014): We have no way of knowing 
     if (has_end_of_block_terminator(sse,enclosing_block) == false && last_stmt == NULL)
        {
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
          printf ("In parse_statement_list_without_terminator(): has_end_of_block_terminator(sse,enclosing_block) == false && last_stmt == NULL: Calling summarize_statement(): enclosing_block = %p \n",enclosing_block);
          summarize_statement(enclosing_block);
#endif
          last_stmt = last_statement_in_list(statement_list);

#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
          printf ("Computed last_stmt = %p = %s \n",last_stmt,summarize_statement_in_string(last_stmt));
#endif
#if 0
          printf ("ERROR: We have not way of knowing when to terminate this block! enclosing_block = %p \n",enclosing_block);
          ROSE_ASSERT(false);
#endif
        }
       else
        {
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
          printf ("Either the block has a terminating end_of_construct, or the last_stmt is defined \n");
          if (last_stmt == NULL)
             {
               printf ("OK, the block has a terminating end_of_construct \n");
             }
            else
             {
               printf ("last_stmt = %p = %s \n",last_stmt,summarize_statement_in_string(last_stmt));

               printf ("In parse_statement_list_without_terminator(): calling summarize_statement(): enclosing_block = %p \n",enclosing_block);
               summarize_statement(enclosing_block);
             }
#endif
        }

#if 0
  // a_statement_ptr last_stmt = last_statement_in_list(statement_list);
     if (last_stmt == NULL)
        {
          ROSE_ASSERT(statement_list != NULL);

       // DQ (1/14/2014): Try to avoid computing the last_stmt is it was not provided as part of this functions API.
       // Computing the last sttement based on the statement_list can cause too few statements to be processed and
       // we end up leaving before the iek_src_seq_end_of_construct kind entry.
       // last_stmt = last_statement_in_list(statement_list);
        }
       else
        {
          ROSE_ASSERT(statement_list == NULL);
        }
#endif

#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
     printf ("In parse_statement_list_without_terminator(): After computing a possible last_stmt: enclosing_block = %p last_stmt = %p \n",enclosing_block,last_stmt);
#endif

     vector<SgStatement*> result;

  // DQ (1/14/2014): Allow the last statement to be NULL if it was not specificed in the API.
  // if (last_stmt != NULL)
        {
       // DQ (1/2/2018): Added debugging support.
          int loop_counter = 0;

       // DQ (1/2/2018): Save the previous stmt_decl statement so that we can map variable declarations to it in the block_statement_map.
          a_source_sequence_entry_ptr last_stmt_decl = NULL;

          while (sse)
             {
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
               printf ("In parse_statement_list_without_terminator(): loop_counter = %d depthCounter = %d TOP while loop for last_stmt != NULL: last_stmt = %p \n",loop_counter,depthCounter,last_stmt);
#endif
               an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR || 0
            // printf ("In parse_statement_list_without_terminator(): in while loop for last_stmt != NULL: last_stmt = %p current stmt kind = %s \n",last_stmt,summarize_entry_kind_in_string_kind(kind));
               printf (">>>>> IN LOOP: parse_statement_list_without_terminator(): in while loop for last_stmt != NULL: last_stmt = %p current entry kind = %p = %s \n",last_stmt,sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
               a_statement_ptr ptr = (kind == iek_statement) ? (a_statement_ptr)(sse->entity.ptr) : NULL;
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
               printf ("--------------- In parse_statement_list_without_terminator(): in while loop for last_stmt != NULL: last_stmt = %p current stmt ptr = %p \n",last_stmt,ptr);
#endif

#if NEW_DUFF_DEVICE_SWITCH_SUPPORT
#if 0
               if (ptr != NULL)
                  {
                    block_source_sequence_perspective.push_back(ptr);
                  }
#else
#if 1
            // bool addedToList = record_block_source_sequence_entries_for_statements (sse,block_source_sequence_perspective);
               record_block_source_sequence_entries_for_statements (sse,block_source_sequence_perspective);
#else
            // DQ (12/14/2017): Added data member access function to SourceSequenceList class to support this.
            // block_source_sequence_perspective.push_back((a_source_sequence_entry_ptr)(sse->entity.ptr));
            // block_source_sequence_perspective.push_back((a_source_sequence_entry_ptr)(sse.value()));
               a_statement_ptr statement = (kind == iek_statement) ? (a_statement_ptr)(sse->entity.ptr) : NULL;
               bool is_stmk_decl = (statement != NULL && statement->kind == stmk_decl);

#error "DEAD CODE!"

               if (is_stmk_decl == false)
                  {
                    block_source_sequence_perspective.push_back((a_source_sequence_entry_ptr)(sse.value()));
                  }
                 else
                  {
                    printf ("Skipping over case of stmk_decl \n");
                  }
#endif
#endif
#endif

            // DQ (2/2/2013): Trying to use parse_statement_list_without_terminator() from switch specific version.
            // Note that this will cause the sse to be incremented if this is a statement.
            // vector<SgStatement*> stmts = parse_statement(sse);
               vector<SgStatement*> stmts;

               a_source_sequence_entry_ptr saved_sse_value = sse.value();

               a_statement_ptr statement = (kind == iek_statement) ? (a_statement_ptr)(sse->entity.ptr) : NULL;
               bool is_stmk_decl = (statement != NULL && statement->kind == stmk_decl);

#if DEBUG_PARSE_STATEMENT_LIST || 0
               printf ("(REFACTOR to include in other call site): part 2 is_stmk_decl = %s \n",is_stmk_decl ? "true" : "false");
#endif
               if (is_stmk_decl == true)
                  {
#if 0
                 // DQ (8/21/2020): Add a comment to warning us about non-symetric code.
                    printf ("NOTE THAT WHERE THIS IS SIMILAR TO CODE IN parse_statement_list(): it is commented out \n");
#endif

                    last_stmt_decl = sse.value();

#if DEBUG_PARSE_STATEMENT_LIST || 0
                    printf ("In parse_statement_list_without_terminator(): before calling parse_statements: last_stmt_decl = %p = %s \n",
                         last_stmt_decl->entity.ptr,summarize_entry_kind_in_string_kind(last_stmt_decl->entity).c_str());
#endif
                  }

            // Reset the outer_kind so that we can continue.
               if (kind != iek_src_seq_end_of_construct)
                  {
#if DEBUG_PARSE_STATEMENT_LIST || 0
                    printf ("In parse_statement_list_without_terminator(): before calling parse_statements: sse = %p = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                    stmts = parse_statement(sse);

#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
                    printf ("In parse_statement_list_without_terminator(): after call to parse_statement(sse): stmts.size() = %zu \n",stmts.size());
#endif
                  }
                 else
                  {
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
                    printf ("Detected kind == iek_src_seq_end_of_construct: so skipping call to parse_statement(sse) to collect statements \n");
#endif
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }

#if NEW_DUFF_DEVICE_SWITCH_SUPPORT
               if (stmts.size() > 0)
                  {
#if 0
                 // ROSE_ASSERT(ptr != NULL);
                    if (ptr != NULL)
                       {
                         block_statement_map.insert(pair<a_statement_ptr,SgStatement*>(ptr,stmts[0]));
                       }
#else
#if DEBUG_PARSE_STATEMENT_LIST
                    printf ("before block_statement_map.insert(): saved_sse_value = %p \n",saved_sse_value);
#endif
                    if (last_stmt_decl != NULL)
                       {
#if DEBUG_PARSE_STATEMENT_LIST
                         printf ("Reset the saved_sse_value to be the last_stmt_decl = %p \n",last_stmt_decl);
#endif
                         saved_sse_value = last_stmt_decl;
                       }

                 // DQ (12/14/2017): Added data member access function to SourceSequenceList class to support this.
                 // block_statement_map.insert(pair<a_source_sequence_entry_ptr,SgStatement*>((a_source_sequence_entry_ptr)(sse->entity.ptr),stmts[0]));
                 // block_statement_map.insert(pair<a_source_sequence_entry_ptr,SgStatement*>(sse.value(),stmts[0]));
                 // block_statement_map.insert(pair<a_source_sequence_entry_ptr,SgStatement*>(saved_sse_value,stmts[0]));
                    setup_block_statement_map ( saved_sse_value, stmts, block_statement_map );
#endif
                  }
                 else
                  {
#if 0
                    printf ("WARNING: In parse_statement_list_without_terminator(): no statements collected \n");
#endif
#if 0
                    printf ("BEFORE pop_back(): block_source_sequence_perspective.size() = %zu \n",block_source_sequence_perspective.size());
#endif
#if 0
                    if (addedToList == true)
                       {
                         block_source_sequence_perspective.pop_back();
                       }
#endif
#if 0
                    printf ("AFTER pop_back(): block_source_sequence_perspective.size() = %zu \n",block_source_sequence_perspective.size());
#endif
                  }
#endif

#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
               printf ("--------------- DONE: In parse_statement_list_without_terminator(): in while loop for last_stmt != NULL: stmts.size() = %" PRIuPTR " \n",stmts.size());
#endif

#ifdef DEBUG_CONVERSION
            // Note that some iterations of this loop may result in empty sets of statements.
            // DQ (10/15/2012): Assert that there should be at least some statment in the list.  Note that when this is a stmt_decl it appears to not be processed properly by parse_statement().
            // ROSE_ASSERT(stmts.empty() == false);
               if (stmts.empty() == false)
                  {
                    printf ("Note that the vector of statements from parse_statement(sse) is empty (might be OK as long as we collect them on subsequent iterations) \n");
                  }
#endif
               result.insert(result.end(), stmts.begin(), stmts.end());

#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
               printf ("In parse_statement_list_without_terminator(): BASE of while loop for last_stmt != NULL: last_stmt = %p current statement sse->entity.kind = %s \n",
                       last_stmt,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
               if ( (kind == iek_statement) && (ptr == last_stmt) )
                  {
                    printf ("In parse_statement_list_without_terminator(): BASE of while loop for last_stmt != NULL: Previous criteria for breaking out of loop has been met, but will be ignored! \n");
                  }
#endif

            // DQ (10/15/2012): Insead of searching for the last statement, iterate until the end of the block (or use the previous test).
            // This fixes test2012_229.C (where compiler-generated blocks were not evaluated and there statements attributed to an outer scope).
            // if (kind == iek_statement && ptr == last_stmt)
               an_il_entry_kind next_kind = (an_il_entry_kind)(sse->entity.kind);
#if 0
               printf ("In parse_statement_list_without_terminator(): next_kind = %s \n",summarize_entry_kind_in_string_kind(next_kind));
               if (sse->next != NULL)
                  {
                    an_il_entry_kind next_next_kind = (an_il_entry_kind)(sse->next->entity.kind);
                    printf ("In parse_statement_list_without_terminator(): next_next_kind = %s \n",summarize_entry_kind_in_string_kind(next_next_kind));
                  }
                 else
                  {
                    printf ("In parse_statement_list_without_terminator(): sse->next == NULL \n");
                    if (next_kind == iek_src_seq_secondary_decl)
                       {
                         a_src_seq_secondary_decl_ptr src_seq_secondary_decl = (a_src_seq_secondary_decl_ptr)(sse->entity.ptr);

                         printf ("In parse_statement_list_without_terminator(): calling summarize_src_seq_secondary_decl() src_seq_secondary_decl = %p \n",src_seq_secondary_decl);
                         summarize_src_seq_secondary_decl(src_seq_secondary_decl);
                       }
                  }
#endif

#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
            // DQ (1/15/2014): Debugging code.
               if ( next_kind == iek_src_seq_end_of_construct )
                  {
                    printf ("##### DEBUG STATUS: next_kind == iek_src_seq_end_of_construct (we will exit this loop over statements) depthCounter = %d \n",depthCounter);
                  }

               if ( (last_stmt != NULL && ptr == last_stmt) )
                  {
                    printf ("##### DEBUG STATUS: (last_stmt != NULL && ptr == last_stmt): depthCounter = %d \n",depthCounter);
                  }

               if ( ( (kind == iek_statement) && (ptr == last_stmt) ) && ( (ptr != NULL) && (ptr->kind != stmk_decl) ) )
                  {
                    printf ("##### DEBUG STATUS: ( (kind == iek_statement) && (ptr == last_stmt) ) && ( (ptr != NULL) && (ptr->kind != stmk_decl) ): (we will exit this loop over statements) depthCounter = %d \n",depthCounter);
                  }
#endif
               if ( (next_kind == iek_src_seq_end_of_construct) || ( ( (kind == iek_statement) && (ptr == last_stmt) ) && ( (ptr != NULL) && (ptr->kind != stmk_decl) ) ) )
                  {
                 // Note that the while statement's conditional might be equivalent to this case (if so that would simplify this code). CHECK THIS!
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
                    printf ("In parse_statement_list_without_terminator(): breaking out of while loop for last_stmt != NULL: last_stmt = %p \n",last_stmt);
#endif
#if 0
                 // DQ (10/25/2012): I don't think that we ever need this code.
                 // We need to increment past some constructs appearing after the last statement.
                    while (next_kind != iek_src_seq_end_of_construct)
                       {
                         printf ("In parse_statement_list_without_terminator(): Increment to the iek_src_seq_end_of_construct: kind = %s \n",summarize_entry_kind_in_string_kind(next_kind));
#error "DEAD CODE!"
                         ++sse;
                         next_kind = (an_il_entry_kind)(sse->entity.kind);
                       }
#else
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
                    printf ("In parse_statement_list_without_terminator(): depthCounter = %d skipping increment to the iek_src_seq_end_of_construct (not wanted for processing case statements) \n",depthCounter);
#endif
#endif
                    break;
                  }

               loop_counter++;
             }

          ROSE_ASSERT (sse);
        }

#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
     printf ("In parse_statement_list_without_terminator(): top of while loop (read pragmas and macros) \n");
#endif

     while (sse)
        {
          an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);

#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
          printf ("In parse_statement_list_without_terminator(): in while loop kind = %s \n",summarize_entry_kind_in_string_kind(kind));
#endif

#if 0
       // DQ (12/29/2008): Skipping the handling of all the emun values as cases.  This maybe should not be a switch.
          switch (kind)
             {
               case iek_pragma:
               case iek_macro:
                  {
                    vector<SgStatement*> stmts = parse_statement(sse);
                    result.insert(result.end(), stmts.begin(), stmts.end());
                    break;
                  }
               default:
                  {
                    goto end;
                  }
             }
#else
       // This is a little be longer, but it avoids a switch that uses just two
       // case of 500+ and permits compilation using the "-Wswitch-enum" option.
          if (kind == iek_pragma || kind == iek_macro)
             {
               vector<SgStatement*> stmts = parse_statement(sse);
               result.insert(result.end(), stmts.begin(), stmts.end());
             }
            else
             {
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
               printf ("In parse_statement_list_without_terminator(): in while loop goto end label \n");
#endif

               goto end;
             }
#endif
        }

     ROSE_ASSERT (sse);

// Note label
end: ;

#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
     printf ("In parse_statement_list_without_terminator(): after label result.size() = %" PRIuPTR " \n",result.size());
#endif

#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
  // DQ (8/17/2012): I think that the source position of the closing scope is contained in here (so we don't want to ignore these).
     if (sse->entity.kind == iek_src_seq_end_of_construct)
        {
          printf ("@@@@@@@@@@@@@@@@@ In parse_statement_list_without_terminator(): found a iek_src_seq_end_of_construct kind \n");
        }
#endif

     if (sse->entity.kind == iek_src_seq_end_of_construct &&
        ((a_src_seq_end_of_construct_ptr)(sse->entity.ptr))->entity.kind == iek_statement &&
        ((a_src_seq_end_of_construct_ptr)(sse->entity.ptr))->entity.ptr == (char*)enclosing_block)
        {
#ifdef DEBUG_CONVERSION
          indent_line(); debug_print("skipping correct end of construct entry\n");
#endif
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
          printf ("In parse_statement_list_without_terminator(): at end (before ++sse): current entry kind = %s \n",summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
          ++sse;
#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR
          printf ("In parse_statement_list_without_terminator(): at end (after ++sse): current entry kind = %s \n",summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
        }

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("end of statement list without terminator %p result.size() = %" PRIuPTR "\n", last_stmt,result.size());
#endif

#if NEW_DUFF_DEVICE_SWITCH_SUPPORT
  // DQ (12/11/2017): Call to refactored code.
     if (stmk_switch_case_to_process == true)
        {
       // DQ (12/29/2017): Added test as part of debugging.
          if (block_statement_map.empty() == false && block_source_sequence_perspective.empty() == true)
             {
               printf ("ERROR: In parse_statement_list_without_terminator(): If there are entries in block_statement_map there should be entries in the block_source_sequence_perspective (empty) \n");
            // ROSE_ASSERT(false);
             }

#if 0
          printf ("In parse_statement_list_without_terminator(): calling setup_duffs_device_switch_data() \n");
#endif
          setup_duffs_device_switch_data(block_list_perspective,block_source_sequence_perspective,block_statement_map, result);
        }
// endif for NEW_DUFF_DEVICE_SWITCH_SUPPORT
#endif 


#if DEBUG_STATEMENT_LIST_WITHOUT_TERMINATOR || 0
     printf ("########## Leaving parse_statement_list_without_terminator(): depthCounter = %d after label result.size() = %" PRIuPTR " \n",depthCounter,result.size());
#endif

  // DQ (7/30/2013): Decrement the depth that we record.
     depthCounter--;

     return result;
   }
