// Support for the C++17 init-statement of selection statements (EDG 7.0 IL):
//
//     if (init; cond) ...     if constexpr (init; cond) ...     switch (init; cond) ...
//
// EDG does not put the init-statement into the statement list.  The enk_condition node that is the
// selection statement's expression points to it (a_condition_supplement::initialization).  For a
// declaration (stmk_decl) the declared entities follow the stmk_decl entry in the source sequence
// list and are terminated by an iek_src_seq_end_of_construct entry for the stmk_decl; an expression
// init-statement is a single stmk_expr entry.  The condition's dynamic_init is NULL unless the
// condition itself also declares a variable.
//
// SageIII has no init-statement slot on SgIfStmt / SgSwitchStatement, so the statement is converted
// as the semantically equivalent
//
//     { init; if (cond) ... }
//
// parse_condition_init_statement() builds that enclosing (compiler-generated, unparsed) SgBasicBlock,
// pushes it onto the scope stack, converts the init-statement into it and advances sse past the
// init-statement's entries.  It returns NULL (and does nothing) when the condition has no
// init-statement.  finish_condition_init_statement() appends the finished selection statement to
// the block, pops the scope and returns the block (or the statement itself when there was no block).

#include "edgRose.h"

#define DEBUG_CONDITION_INIT_STATEMENT 0

SgBasicBlock*
EDG_ROSE_Translation::parse_condition_init_statement(an_expr_node_ptr e, SourceSequenceList& sse)
   {
     if (e == NULL || (an_expr_node_kind)(e->kind) != enk_condition)
          return NULL;

     a_condition_supplement_ptr condition_supplement = e->variant.condition;
     ROSE_ASSERT(condition_supplement != NULL);

     a_statement_ptr init_statement = condition_supplement->initialization;
     if (init_statement == NULL)
          return NULL;

#if DEBUG_CONDITION_INIT_STATEMENT
     printf ("In parse_condition_init_statement(): init_statement = %p kind = %s \n",init_statement,summarize_statement_in_string(init_statement));
#endif

     SgBasicBlock* initBlock = set_generated_file_info_to_unparse(buildBasicBlock_nfi());
     ROSE_ASSERT(initBlock != NULL);
     initBlock->set_parent(topScopeStack());
     pushScopeStack(initBlock);

     std::vector<SgStatement*> stmts;

     if (init_statement->kind == stmk_decl)
        {
       // The stmk_decl entry itself yields no SgStatement (parse_statement() skips it, as for any
       // declaration statement); the declared variables and types are separate entries that
       // parse_statement() converts.  Stop at the end-of-construct entry for the stmk_decl.
          while (true)
             {
               ROSE_ASSERT(sse);
               if (sse->entity.kind == iek_src_seq_end_of_construct)
                  {
                    a_src_seq_end_of_construct_ptr end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);
                    ROSE_ASSERT(end_of_construct != NULL);
                    if (end_of_construct->entity.kind == iek_statement && (a_statement_ptr)(end_of_construct->entity.ptr) == init_statement)
                       {
                         ++sse;
                         break;
                       }
                  }

               void* entry_before = sse->entity.ptr;
               std::vector<SgStatement*> entry_stmts = parse_statement(sse);
               stmts.insert(stmts.end(), entry_stmts.begin(), entry_stmts.end());

               if (sse && sse->entity.ptr == entry_before)
                  {
                    fprintf (stderr, "Error: In parse_condition_init_statement(): parse_statement() did not advance past entry %p (%s) \n",
                         entry_before, summarize_entry_kind_in_string_kind(sse->entity).c_str());
                    ROSE_ASSERT(false);
                    abort();
                  }
             }
        }
       else
        {
       // An expression init-statement ("if (f(); x)"): one stmk_expr entry and no end-of-construct marker.
          ROSE_ASSERT(sse);
          ROSE_ASSERT(sse->entity.kind == iek_statement);
          ROSE_ASSERT((a_statement_ptr)(sse->entity.ptr) == init_statement);
          stmts = parse_statement(sse);
        }

     for (size_t i = 0; i < stmts.size(); i++)
        {
          ROSE_ASSERT(stmts[i] != NULL);
          initBlock->append_statement(stmts[i]);
          stmts[i]->set_parent(initBlock);
        }

#if DEBUG_CONDITION_INIT_STATEMENT
     printf ("Leaving parse_condition_init_statement(): initBlock = %p with %zu statements \n",initBlock,stmts.size());
#endif

     return initBlock;
   }


SgStatement*
EDG_ROSE_Translation::finish_condition_init_statement(SgBasicBlock* initBlock, SgStatement* selection_statement)
   {
     ROSE_ASSERT(selection_statement != NULL);

     if (initBlock == NULL)
          return selection_statement;

     ROSE_ASSERT(topScopeStack() == initBlock);
     initBlock->append_statement(selection_statement);
     selection_statement->set_parent(initBlock);
     popScopeStack();

     return initBlock;
   }
