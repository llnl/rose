
#include "edgRose.h"

std::vector<SgStatement*>
EDG_ROSE_Translation::parse_statement_list(SourceSequenceList& sse, an_il_entry_kind orig_kind, void* orig_ptr)
   {
#define DEBUG_PARSE_STATEMENT_LIST 0

#if DEBUG_PARSE_STATEMENT_LIST || 0
     int counter = 0;
     printf ("In parse_statement_list(): starting statement counter = %d \n",counter);
#endif

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("statement list (orig_ptr = %p)\n", orig_ptr);
#endif

     vector<SgStatement*> result;
     ROSE_ASSERT (sse);

  // DQ (13/12/2017): When this is a block, then we want to setup the lists of possible cases 
  // to support the new design for switch support that is capable of supporting Duff's device.
     a_statement_ptr statement_list = NULL;
     if (orig_ptr != NULL && orig_kind == iek_statement && ((a_statement_ptr)orig_ptr)->kind == stmk_block)
        {
          a_statement_ptr block_statement = (a_statement_ptr) orig_ptr;

          statement_list = block_statement->variant.block.statements;

       // printf ("Found a block in parse_statement_list(): Exiting as a test! \n");
       // ROSE_ASSERT(false);
        }

#if NEW_DUFF_DEVICE_SWITCH_SUPPORT
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
               indent_line(); printf("In parse_statement_list(): statement_list != NULL: call db_statement(s = %p = %s) \n",s,summarize_statement_in_string(s)); // debug_print("\n");
#endif
            // DQ (11/24/2017): Detect the case of a switch body's stmk_switch_case statement.
               if (s->kind == stmk_switch_case)
                  {
#if 0
                    printf ("NOTE: In parse_statement_list(): case of switch body containing stmk_switch_case \n");
#endif
                 // ROSE_ASSERT(false);
#if 0
                    printf ("In parse_statement(): case stmk_block: Calling summarize_statement(s = %p) \n",s);
                    summarize_statement(s);
#endif
                    stmk_switch_case_to_process = true;
                  }
#if 0
               printf ("In parse_statement_list(): stmk_switch_case_to_process = %s \n",stmk_switch_case_to_process ? "true" : "false");
#endif
            // DQ (12/14/2017): We might want to avoid any case where s->position.seq == 0 so this could be more general.
            // Specifically implicit statements are not present in the source and this don't appear in the source sequence lists.
               bool skip_implicit_return = false;
               if (s->kind == stmk_return && s->position.seq == 0)
                  {
#if 0
                    printf ("Detected an implicit return statement that we should skip inserting into the block_list_perspective vector \n");
#endif
                    skip_implicit_return = true;
                  }

               if (skip_implicit_return == false)
                  {
                    block_list_perspective.push_back(s);
                  }
             }
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

#if 0
     printf ("In parse_statement_list(): block_list_perspective.size() = %zu \n",block_list_perspective.size());
#endif

// endif for NEW_DUFF_DEVICE_SWITCH_SUPPORT
#endif

  // DQ (12/14/2017): Save the previous stmt_decl statement so that we can map variable declarations to it in the block_statement_map.
     a_source_sequence_entry_ptr last_stmt_decl = NULL;

     while (sse)
        {
#if DEBUG_PARSE_STATEMENT_LIST || 0
          printf ("(((((((((((((((((((((( In parse_statement_list(): statement counter = %d (orig_ptr = %p) entry = %p kind = %s \n",
               counter++,orig_ptr,sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
          an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);

          if (kind == iek_src_seq_end_of_construct)
             {
            // DQ (8/17/2012): I think that the source position of the closing scope is contained in here (so we don't want to ignore these).
#if DEBUG_PARSE_STATEMENT_LIST
               printf ("@@@@@@@@@@@@@@@@@ In parse_statement_list(): found iek_src_seq_end_of_construct kind: statement counter = %d \n",counter++);
#endif
            // DQ (10/29/2012): This an example of a long sought solution of a bug that was effecting the use of GNU statement expressions.
            // Note that if we have processed a variable that contained a defining declaration of a type, then the type was processed already, 
            // we will have skipped over any stmk_block entry generated as part of a statement expression, and now we want to detect if this 
            // is a compiler generated iek_src_seq_end_of_construct added to support that variable and which we would have to skip over.
            // If this is the correct handling, then this addresses test2012_130.c, and a number of other test codes.
               a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);
#if DEBUG_PARSE_STATEMENT_LIST
               printf ("In parse_statement_list(): associated entry to end_of_construct (before increment) sse = %p = %s \n",
                    src_seq_end_of_construct->entity.ptr,summarize_entry_kind_in_string_kind(src_seq_end_of_construct->entity).c_str());
#endif
#if DEBUG_PARSE_STATEMENT_LIST
               if (src_seq_end_of_construct->entity.kind == iek_statement)
                  {
                    printf ("In parse_statement_list(): associated entry to end_of_construct (before increment) sse = %p = %s \n",
                         src_seq_end_of_construct->entity.ptr,summarize_entry_kind_in_string_kind(src_seq_end_of_construct->entity).c_str());
#if 0
                    printf ("We need to identify this case of src_seq_end_of_construct->entity.kind == iek_statement \n");
                    ROSE_ASSERT(false);
#endif
                  }
#endif
               if (src_seq_end_of_construct->entity.kind == iek_variable)
                  {
                    a_variable_ptr variable = (a_variable_ptr) src_seq_end_of_construct->entity.ptr;
#if DEBUG_PARSE_STATEMENT_LIST
                    printf ("Identify the variable = %p \n",variable);
#endif
                    ROSE_ASSERT(variable != NULL);
#if 0
                    printf ("In parse_statement_list(): Calling summarize_variable() \n");
                    summarize_variable(variable);
#endif
#if DEBUG_PARSE_STATEMENT_LIST
                    printf ("################## Warning: In parse_statement_list(): EDG 4.7 does not correctly record embedded_source_sequence_entries = %s \n",variable->embedded_source_sequence_entries ? "true" : "false");
#endif
                 // DQ (6/21/2013): EDG 4.7 behaves differently here and does not correctly record the embedded_source_sequence_entries.
                 // I don't know how to detect this (see test2013_08.c for an example).
                    if (variable->embedded_source_sequence_entries == true)
                       {
                      // ROSE_ASSERT(variable->embedded_source_sequence_entries == false);
#if DEBUG_PARSE_STATEMENT_LIST
                         printf ("In parse_statement_list(): Found a case (variable) where we need to skip over the iek_src_seq_end_of_construct entry \n");
#endif
                         ++sse;
                       }
                  }
                 else
                  {
                    if (src_seq_end_of_construct->entity.kind == iek_routine)
                       {
#if 0
                         a_routine_ptr routine = (a_routine_ptr) src_seq_end_of_construct->entity.ptr;
                         printf ("In parse_statement_list(): Calling summarize_routine() \n");
                         summarize_routine(routine);
#endif
#if DEBUG_PARSE_STATEMENT_LIST
                         printf ("In parse_statement_list(): Found a case (routine) where we need to skip over the iek_src_seq_end_of_construct entry \n");
#endif
                      // DQ (6/21/2013): We need to skip over this to get to the rest of the entries.
                         ++sse;
#if 0
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                       }
                      else
                       {
                         if (src_seq_end_of_construct->entity.kind == iek_statement)
                            {
#if DEBUG_PARSE_STATEMENT_LIST
                              printf ("In parse_statement_list(): Found a case (statement) where we need to skip over the iek_src_seq_end_of_construct entry \n");
#endif
                           // DQ (3/22/2014): We need to skip over this to get to the rest of the entries.
                              ++sse;

                              a_statement_ptr statement = (a_statement_ptr) (src_seq_end_of_construct->entity.ptr);
                              ROSE_ASSERT(statement != NULL);
                              a_statement_kind statement_kind = statement->kind;

                           // For some statements we don't want to terminate the iteration over the statment list (new for EDG 4.8).
                              if (statement_kind == stmk_end_test_while)
                                 {
#if DEBUG_PARSE_STATEMENT_LIST
                                   printf ("detected statement->kind == stmk_end_test_while: skip over the iek_src_seq_end_of_construct entry \n");
#endif
                                 }
                                else
                                 {
                                // If this is not a stmk_end_test_while then skip over the iek_src_seq_end_of_construct entry AND end the iteration over statements.
#if DEBUG_PARSE_STATEMENT_LIST
                                   printf ("================ If this is not a stmk_end_test_while then skip over the iek_src_seq_end_of_construct entry AND end the iteration over statements \n");
#endif
                                   goto end;
                                 }
                            }
                           else
                            {

                           // DQ (5/16/2015): Added code to handle case of iek_typedef (see test2015_144.c).
                           // Note this is an example of a sequence of source sequence list nodes that are 
                           // specific to a type (with an associated end_of_construct).
                              if (src_seq_end_of_construct->entity.kind == iek_type)
                                 {
#if DEBUG_PARSE_STATEMENT_LIST
                                   printf ("In parse_statement_list(): Found a case (iek_type) where we need to skip over the iek_src_seq_end_of_construct entry \n");
#endif
                                   ++sse;
#if 0
                                   printf ("Exting as a test! \n");
                                   ROSE_ASSERT(false);
#endif
                                // goto end;
                                 }
                                else
                                 {
                           // DQ (6/21/2013): Original code.

                           // This is the more typical case where the iek_src_seq_end_of_construct means that it is the end of the stmk_block.
                              check_end_of_construct(orig_kind, orig_ptr, sse);

                           // printf ("In parse_statement_list(): case kind == iek_src_seq_end_of_construct: Commented out call to ++sse \n");
                              ++sse;

                           // a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);
#if DEBUG_PARSE_STATEMENT_LIST
                              printf ("In parse_statement_list(): before leaving the loop (goto end): associated entry to end_of_construct (after increment) sse = %p = %s \n",
                                   src_seq_end_of_construct->entity.ptr,summarize_entry_kind_in_string_kind(src_seq_end_of_construct->entity).c_str());
#endif
                              goto end;
                                 }
                            }
                       }
                  }
             }
            else
             {
#if NEW_DUFF_DEVICE_SWITCH_SUPPORT
               ROSE_ASSERT(sse);
            // ROSE_ASSERT(sse->entity.kind == iek_statement || sse->entity.kind == iek_variable);
#if DEBUG_PARSE_STATEMENT_LIST || 0
            // bool addedToList = record_block_source_sequence_entries_for_statements (sse,block_source_sequence_perspective);
               bool addedToList = false;
#endif
               record_block_source_sequence_entries_for_statements (sse,block_source_sequence_perspective);
#endif

#if DEBUG_PARSE_STATEMENT_LIST || 0
               printf ("In parse_statement_list(): calling record_block_source_sequence_entries_for_statements: addedToList = %s sse = %p = %s \n",
                    addedToList ? "true" : "false",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

#if DEBUG_PARSE_STATEMENT_LIST || 0
               printf ("In parse_statement_list(): before calling parse_statements: sse = %p = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
               a_source_sequence_entry_ptr saved_sse_value = sse.value();
#if 0
               printf ("saved_sse_value = %p \n",saved_sse_value);
#endif

            // DQ (12/26/2017): We need to trigger the use of the correct statement in the map. May need to be refactored!!!
               a_statement_ptr statement = (kind == iek_statement) ? (a_statement_ptr)(sse->entity.ptr) : NULL;
            // a_type_ptr type_statement = (sse->entity.kind == iek_type) ? (a_type_ptr)(sse->entity.ptr) : NULL;

            // bool is_stmk_decl = (statement != NULL && statement->kind == stmk_decl);
            // bool is_stmk_decl = ( (statement != NULL && statement->kind == stmk_decl) || (type_statement != NULL && type_statement->kind == tk_typeref));
               bool is_stmk_decl = (statement != NULL && statement->kind == stmk_decl);

#if DEBUG_PARSE_STATEMENT_LIST || 0
               printf ("(REFACTOR to include in other call site): part 1 is_stmk_decl = %s \n",is_stmk_decl ? "true" : "false");
#endif
               if (is_stmk_decl == true)
                  {
#if 0
                 // DQ (8/21/2020): Comment this out since we don't want to skip over the stmk_decl (in C_tests/test2020_14.c).
                    last_stmt_decl = sse.value();
#if DEBUG_PARSE_STATEMENT_LIST || 0
                    printf ("In parse_statement_list(): before calling parse_statements: last_stmt_decl = %p = %s \n",last_stmt_decl->entity.ptr,summarize_entry_kind_in_string_kind(last_stmt_decl->entity).c_str());
#endif
#else
#if 0
                    printf ("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");
                    printf ("$$$$$$$$$$$$$$$ This change appear to fix C_tests/test2020_14.c), but I don't yet understand why! \n");
                    printf ("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");
#endif
#endif
                  }

               vector<SgStatement*> stmts = parse_statement(sse);

#if DEBUG_PARSE_STATEMENT_LIST
               printf ("@@@@@@@@@@@@@@@@ In parse_statement_list(): output the collected statements: \n");
               for (size_t i = 0; i < stmts.size(); i++)
                  {
                    printf ("   --- stmts[%zu] = %p = %s \n",i,stmts[i],stmts[i]->class_name().c_str());
                  }
#endif

#if NEW_DUFF_DEVICE_SWITCH_SUPPORT

#if DEBUG_PARSE_STATEMENT_LIST
               printf ("Collected statements: stmts.size() = %zu \n",stmts.size());
#endif
               if (stmts.size() > 0)
                  {
#if 0
                 // ROSE_ASSERT(ptr != NULL);
#error "DEAD CODE!"
                    if (statement != NULL)
                       {
                         block_statement_map.insert(pair<a_statement_ptr,SgStatement*>(statement,stmts[0]));
                       }
                      else
                       {
#if 0
                         printf ("WARNING: In parse_statement_list(): what was collectioned was not a statement! \n");
#endif

#error "DEAD CODE!"
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

#if 1
                    setup_block_statement_map ( saved_sse_value, stmts, block_statement_map );
#else
                 // DQ (12/14/2017): Added data member access function to SourceSequenceList class to support this.
                 // block_statement_map.insert(pair<a_source_sequence_entry_ptr,SgStatement*>((a_source_sequence_entry_ptr)(sse->entity.ptr),stmts[0]));
                 // block_source_sequence_perspective.push_back((a_source_sequence_entry_ptr)(sse.value()));
                 // block_statement_map.insert(pair<a_source_sequence_entry_ptr,SgStatement*>(sse.value(),stmts[0]));
                 // block_statement_map.insert(pair<a_source_sequence_entry_ptr,SgStatement*>(saved_sse_value,stmts[0]));
                 // block_statement_map.insert(pair<a_source_sequence_entry_ptr,SgStatement*>(last_stmt_decl,stmts[0]));
                    block_statement_map.insert(pair<a_source_sequence_entry_ptr,SgStatement*>(saved_sse_value,stmts[0]));
#error "DEAD CODE!"
                    SgLabelStatement* labelStatement = isSgLabelStatement(stmts[0]);
                    if (labelStatement != NULL)
                       {
                      // Then add the next sse entry and the label statement body to the block_statement_map.

#if DEBUG_PARSE_STATEMENT_LIST
                         printf ("Adding the label's body into the block_statement_map \n");
#endif
                         SourceSequenceList sse_body = saved_sse_value;
                         ++sse_body;
                         a_source_sequence_entry_ptr next_sse_value = sse_body.value();

#error "DEAD CODE!"
                         SgStatement* label_body = labelStatement->get_statement();
                         ROSE_ASSERT(label_body != NULL);

#if DEBUG_PARSE_STATEMENT_LIST
                         printf ("Processing label body = %p = %s \n",label_body,label_body->class_name().c_str());
#endif
                         block_statement_map.insert(pair<a_source_sequence_entry_ptr,SgStatement*>(next_sse_value,label_body));
                       }
#error "DEAD CODE!"
#endif

#endif
                 // DQ (12/14/2017): Clear the last stmt_decl used as a key into the block_statement_map.
                    last_stmt_decl = NULL;

                  }
                 else
                  {
#if DEBUG_PARSE_STATEMENT_LIST
                    printf ("WARNING: In parse_statement_list(): no statements collected \n");
#endif
                 // DQ (12/20/2017): Remove the entry in the block_source_sequence_perspective since it didn't translate into a SgStatement.
                 // This could have been a statement expression or perhaps something else.
                 // ROSE_ASSERT(block_source_sequence_perspective.empty() == false);

#if DEBUG_PARSE_STATEMENT_LIST
                    printf ("BEFORE pop_back(): block_source_sequence_perspective.size() = %zu \n",block_source_sequence_perspective.size());
#endif
#if 0
                 // addedToList &= (block_source_sequence_perspective.empty() == false);
                    if (block_source_sequence_perspective.empty() == true)
                       {
                         addedToList = false;
                       }
#endif
#if DEBUG_PARSE_STATEMENT_LIST
                    printf ("In parse_statement_list(): addedToList = %s \n",addedToList ? "true" : "false");
#endif
#if 0
                    if (addedToList == true)
                       {
                         block_source_sequence_perspective.pop_back();
                       }
#endif
#if DEBUG_PARSE_STATEMENT_LIST
                    printf ("AFTER pop_back(): block_source_sequence_perspective.size() = %zu \n",block_source_sequence_perspective.size());
#endif
                  }
#endif

#if DEBUG_PARSE_STATEMENT_LIST
               printf ("In parse_statement_list(): result.size() = %" PRIuPTR " stmts.size() = %" PRIuPTR " \n",result.size(),stmts.size());
               for (size_t i = 0; i < stmts.size(); i++)
                  {
                    printf ("   --- parse_statement_list(): stmts[%" PRIuPTR "] = %p = %s \n",i,stmts[i],stmts[i]->class_name().c_str());
                  }
#endif
               result.insert(result.end(), stmts.begin(), stmts.end());


#if DEBUG_PARSE_STATEMENT_LIST
               printf ("In parse_statement_list(): (after insert into evolving result statement list) result.size() = %" PRIuPTR " \n",result.size());
               for (size_t i = 0; i < result.size(); i++)
                  {
                    printf ("   --- parse_statement_list(): result[%" PRIuPTR "] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
                  }
#endif
             }

#if DEBUG_PARSE_STATEMENT_LIST || 0
          printf (")))))))))))))))))))))) In parse_statement_list(): END OF WHILE LOOP: statement counter = %d (orig_ptr = %p) entry = %p kind = %s \n",
               counter++,orig_ptr,sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

     ROSE_ASSERT (sse);

// Note label
end: ;

#if NEW_DUFF_DEVICE_SWITCH_SUPPORT
  // DQ (12/13/2017): Call to refactored code.

#if DEBUG_PARSE_STATEMENT_LIST
     printf ("block_list_perspective.size()            = %zu \n",block_list_perspective.size());
     printf ("block_source_sequence_perspective.size() = %zu \n",block_source_sequence_perspective.size());
     printf ("block_statement_map.size()               = %zu \n",block_statement_map.size());
     printf ("result.size()                            = %zu \n",result.size());
#endif

#if 0
     if (block_statement_map.size() == 0)
        {
          printf ("ERROR: In parse_statement_list(): there should be entries in the block_statement_map (empty) \n");
       // ROSE_ASSERT(false);
        }
#endif

     if (stmk_switch_case_to_process == true)
        {
       // DQ (12/29/2017): Added test as part of debugging.
          if (block_statement_map.empty() == false && block_source_sequence_perspective.empty() == true)
             {
               printf ("ERROR: In parse_statement_list(): If there are entries in block_statement_map there should be entries in the block_source_sequence_perspective (empty) \n");
            // ROSE_ASSERT(false);
             }
#if 0
          printf ("block_list_perspective.size()            = %zu \n",block_list_perspective.size());
          printf ("block_source_sequence_perspective.size() = %zu \n",block_source_sequence_perspective.size());
          printf ("block_statement_map.size()               = %zu \n",block_statement_map.size());
          printf ("result.size()                            = %zu \n",result.size());
          printf ("^^^^^^^^^^^^^^^^^^^^^^^^ In parse_statement_list(): calling setup_duffs_device_switch_data() \n");
#endif
#if 0
          printf ("block_statement_map.size() = %zu \n",block_statement_map.size());
          int counter = 0;
          for (map<a_source_sequence_entry_ptr,SgStatement*>::iterator i = block_statement_map.begin(); i != block_statement_map.end(); i++)
             {
               printf ("   --- block_statement_map[%d]: first = %p  entity = %p = %s second = %p = %s \n",counter,
                    i->first,i->first->entity.ptr,summarize_entry_kind_in_string_kind(i->first->entity).c_str(),
                    i->second,i->second->class_name().c_str());
               counter++;
             }
#endif
          setup_duffs_device_switch_data(block_list_perspective,block_source_sequence_perspective,block_statement_map, result);

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
#endif

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("end of statement list (orig_ptr = %p)\n", orig_ptr);
#endif

#if DEBUG_PARSE_STATEMENT_LIST || 0
     printf ("Leaving parse_statement_list(): leaving parse_statement_list(): result.size() = %" PRIuPTR " \n",result.size());
     for (size_t i = 0; i < result.size(); i++)
        {
          printf ("   --- parse_statement_list(): result[%" PRIuPTR "] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
        }
#endif

#if DEBUG_PARSE_STATEMENT_LIST || 0
     printf ("Leaving parse_statement_list(): ending statement counter = %d \n",counter);
#endif

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

     return result;
   }
