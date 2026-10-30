
#include "edgRose.h"

namespace EDG_ROSE_Translation {

void setupDeclarationScope( SgDeclarationStatement* decl)
   {
  // DQ (7/18/2017): Adding support for class declarations built from template parameters.  The scope for 
  // these must be the SgDeclarationScope that is contained in the nondefining template declaration.

#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
  // Debugging code.
     printf ("In setupDeclarationScope(): decl = %p = %s = %s \n",decl,decl->class_name().c_str(),SageInterface::get_name(decl).c_str());
     SgDeclarationStatement* definingDeclaration = decl->get_definingDeclaration();
     if (definingDeclaration != NULL)
        {
          printf ("In setupDeclarationScope(): definingDeclaration = %p = %s \n",definingDeclaration,definingDeclaration->class_name().c_str());
          if (decl == definingDeclaration)
             {
               printf ("   --- IS defining declaration \n");
             }
            else
             {
               printf ("   --- is NOT defining declaration \n");
             }
        }

     SgDeclarationStatement* firstNondefiningDeclaration = decl->get_firstNondefiningDeclaration();
     if (firstNondefiningDeclaration != NULL)
        {
          printf ("In setupDeclarationScope(): firstNondefiningDeclaration = %p = %s \n",firstNondefiningDeclaration,firstNondefiningDeclaration->class_name().c_str());
          if (decl == firstNondefiningDeclaration)
             {
               printf ("   --- IS firstNondefining declaration \n");
             }
            else
             {
               printf ("   --- is NOT firstNondefining declaration \n");
             }
        }
#endif

  // Check if this has already been setup.
     if (decl->get_declarationScope() == NULL)
        {
       // Check if there are any additions to the internal_intermediate_scope.
          ROSE_ASSERT(internal_intermediate_scope != NULL);
          size_t numberOfClassesBuiltFromTemplateParameters = internal_intermediate_scope->symbol_table_size();
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
          printf ("In setupDeclarationScope(): numberOfClassesBuiltFromTemplateParameters = %zu \n",numberOfClassesBuiltFromTemplateParameters);
#endif
          if (numberOfClassesBuiltFromTemplateParameters > 0)
             {
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
               printf ("In setupDeclarationScope(): Detected classes built from template parameters, stored in decl = %p = %s \n",decl,decl->class_name().c_str());
#endif
               decl->set_declarationScope(internal_intermediate_scope);
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
               printf ("In setupDeclarationScope(): Detected classes built from template parameters: using internal_intermediate_scope = %p \n",internal_intermediate_scope);
#endif
               ROSE_ASSERT(current_global_scope != NULL);
               ROSE_ASSERT(current_global_scope->get_startOfConstruct() != NULL);

               Sg_File_Info *startOfConstruct = new Sg_File_Info (current_global_scope->get_startOfConstruct()->get_filenameString());
               ROSE_ASSERT(startOfConstruct != NULL);

               internal_intermediate_scope->set_parent(decl);

            // If decl is a defining declaration, then find the associated scope (e.g. SgTemplateClassDefinition).
               SgFunctionDeclaration* function_decl = isSgFunctionDeclaration(decl);
               SgClassDeclaration*    class_decl    = isSgClassDeclaration(decl);
               ROSE_ASSERT(function_decl != NULL || class_decl != NULL);

               SgScopeStatement* scope = NULL;
               if (function_decl != NULL) 
                  {
                    scope = function_decl->get_definition();
                  }
                 else
                  {
                    if (class_decl != NULL) 
                       {
                         scope = class_decl->get_definition();
                       }
                  }

               if (scope != NULL)
                  {
                 // DQ (7/25/2017): Adding the alias symbols will permit references to the new classes built from 
                 // the template parameters to be found, this can happen in the preinitialization list (see test2017_62.C).
#if 0
                    printf ("In setupDeclarationScope(): Need to add alias symbols from the SgDeclarationScope symbols into the SgTemplateClassDefinition: scope = %p = %s \n",scope,scope->class_name().c_str());
#endif
                 // Insert SgAlias symbols into the declaration's associated scope.

                 // ROSE_ASSERT(scope->get_symbol_table() != NULL);
                    ROSE_ASSERT(internal_intermediate_scope->get_symbol_table() != NULL);

                 // SgSymbolTable::BaseHashType* internalTable = scope->get_symbol_table()->get_table();
                    SgSymbolTable::BaseHashType* internalTable = internal_intermediate_scope->get_symbol_table()->get_table();
                    ROSE_ASSERT(internalTable != NULL);
#if 0
                    int counter = 0;
#endif
                    SgSymbolTable::hash_iterator i = internalTable->begin();
                    while (i != internalTable->end())
                       {
                         ROSE_ASSERT ( (*i).first.str() != NULL );
                         ROSE_ASSERT ( isSgSymbol( (*i).second ) != NULL );
#if 0
                         printf ("Symbol number: %d (pair.first (SgName) = %s) pair.second (SgSymbol) class_name() = %s \n",counter,(*i).first.str(),(*i).second->class_name().c_str());
#endif
                         SgName name      = (*i).first;
                         SgSymbol* symbol = (*i).second;

                         ROSE_ASSERT(symbol != NULL);

                         SgAliasSymbol* aliasSymbol = new SgAliasSymbol(symbol);
                         ROSE_ASSERT(aliasSymbol != NULL);

                         scope->insert_symbol(name, aliasSymbol);

                      // We could also define the causalNode for the alias (this can help with debugging).
                      // ROSE_ASSERT(causalNode != NULL);
                      // aliasSymbol->get_causal_nodes().push_back(causalNode);

                         i++;
#if 0
                         counter++;
#endif
                       }

#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }

            // Reset to NULL before assiging to a new valid scope object.
               internal_intermediate_scope = NULL;

               internal_intermediate_scope = new SgDeclarationScope(startOfConstruct);
               ROSE_ASSERT(internal_intermediate_scope != NULL);

            // We need this to have a valid parent, and global scope might be all that is available.
            // Or maybe we need to fixup the get_scope() function to detect the case of a SgDeclarationScope and return NULL (instead of an error).
               internal_intermediate_scope->set_parent(current_global_scope);
               ROSE_ASSERT(internal_intermediate_scope->get_parent() != NULL);
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
               printf ("In setupDeclarationScope(): reinitialize internal_intermediate_scope = %p \n",internal_intermediate_scope);
#endif
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
               printf ("In setupDeclarationScope(): $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");
#endif
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }
            else
             {
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
               printf ("The intermediate scope was not used to construct any classes from template parameters \n");
#endif
             }
        }
       else
        {
       // I think this should be an error.
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
          printf ("The declaration scope for classes built from template parameters have already been setup \n");
       // ROSE_ASSERT(false);
#endif
        }
   }

SgTemplateArgumentPtrList setupMatchingTemplateArguments(bool buildTemplateInstantiation, SgFunctionDeclaration* first_nondefining_declaration, a_template_arg_ptr templateArgs, SgScopeStatement* scopeOfFunction)
   {
  // DQ (5/21/2016): Refactored code from convert_routine (only called in convert_routine()).

     SgTemplateArgumentPtrList templateArgumentsList;

#if 0
     printf ("In setupMatchingTemplateArguments(): setting templateArgumentsList using first_nondefining_declaration or parse_template_arguments() function \n");
     printf ("   --- buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
     printf ("   --- first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,(first_nondefining_declaration != NULL) ? first_nondefining_declaration->class_name().c_str() : "null");
     printf ("   --- templateArgs = %p \n",templateArgs);
     printf ("   --- scopeOfFunction = %p = %s \n",scopeOfFunction,scopeOfFunction->class_name().c_str());
#endif

  // DQ (1/30/2013): We need to be careful about the use of template arguments, since the symbol table lookup requires 
  // that the pointers to SgTemplateArguments must match by value.  The is a problem below where there are assertions 
  // using the lookup via the function: retrieveAssociatedClassSymbol().
     bool templateArgumentListSetup = false;
     if (first_nondefining_declaration != NULL)
        {
          SgTemplateInstantiationFunctionDecl*       templateFunctionInstantiationDeclaration       = isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration);
          SgTemplateInstantiationMemberFunctionDecl* templateMemberFunctionInstantiationDeclaration = isSgTemplateInstantiationMemberFunctionDecl(first_nondefining_declaration);
          if (templateFunctionInstantiationDeclaration != NULL)
             {
            // STL list copy
               templateArgumentsList = templateFunctionInstantiationDeclaration->get_templateArguments();
#if 0
               printf ("templateArgumentsList set from templateFunctionInstantiationDeclaration \n");
#endif
               templateArgumentListSetup = true;
             }
            else
             {
               if (templateMemberFunctionInstantiationDeclaration != NULL)
                  {
                 // STL list copy
                    templateArgumentsList = templateMemberFunctionInstantiationDeclaration->get_templateArguments();
#if 0
                    printf ("templateArgumentsList set from templateMemberFunctionInstantiationDeclaration \n");
#endif
                    templateArgumentListSetup = true;
                  }
                 else
                  {
                 // DQ (8/8/2013): Added case to detect when templateArgumentsList is not setup.
// #ifdef DEBUG_CONVERSION
#if 0
                    printf ("Case not handled: (templateArgumentsList NOT setup): (nothing to do here) first_nondefining_declaration = %p = %s \n",
                         first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
#endif
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
             }
        }

     if (buildTemplateInstantiation == true && templateArgs != NULL && templateArgumentListSetup == false)
        {
#if 0
          printf ("In setupMatchingTemplateArguments(): Calling parse_template_arguments() \n");
#endif
#if 0
       // DQ (2/11/2019): Adding support to track the min_template_arguments.
          a_class_type_supplement_ptr classTypeSupplement = p->variant.class_struct_union.extra_info;
          ROSE_ASSERT((p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union) && classTypeSupplement != NULL);
#if 0
          printf ("In setupMatchingTemplateArguments(): classTypeSupplement->min_template_arguments = %d \n",classTypeSupplement->min_template_arguments);
#endif
#else
       // printf ("In setupMatchingTemplateArguments(): the EDG type is not available to support query on the class extra info \n");
#endif
          templateArgumentsList = parse_template_arguments(templateArgs,scopeOfFunction);
#if 0
          printf ("templateArgumentsList set from parse_template_arguments() \n");
#endif
          templateArgumentListSetup = true;
        }

  // DQ (7/6/2016): Make sure that at least one of the branches setup the templateArgumentsList.
     ROSE_ASSERT((buildTemplateInstantiation == false) || (first_nondefining_declaration == NULL) || (templateArgumentListSetup == true));

     return templateArgumentsList;
   }

unsigned int setupFunctionTypeModifiers(a_routine_type_supplement_ptr sup)
   {
  // DQ (3/8/2012): Refactored the support for function type modifiers.

  // ROSE_ASSERT(p->kind == tk_routine);
  // a_routine_type_supplement_ptr sup = p_type->variant.routine.extra_info;

     unsigned int functionConstVolatileFlags = 0;

     ROSE_ASSERT(sup != NULL);

#if 0
     printf ("In setupFunctionTypeModifiers() TOP: sup->qualifiers = %" PRIuPTR " sup->this_qualifiers = %" PRIuPTR " \n",(size_t)(sup->qualifiers),(size_t)(sup->this_qualifiers));
#endif

  // Location of last bit used so that we can add thr reference qualification bits after this.
  // unsigned long last_bit = tqt_upc_relaxed;
  // ROSE_ASSERT(last_bit < 30);

#if 0
  // printf ("In setupFunctionTypeModifiers(): TQ_CONST = %" PRIuPTR " TQ_VOLATILE = %" PRIuPTR " TQ_RESTRICT = %" PRIuPTR " \n",(size_t)TQ_CONST,(size_t)TQ_VOLATILE,(size_t)TQ_RESTRICT);
     printf ("In setupFunctionTypeModifiers(): TQ_CONST = %" PRIuPTR " TQ_VOLATILE = %" PRIuPTR " (TQ_CONST | TQ_VOLATILE) = %" PRIuPTR " TQ_RESTRICT = %" PRIuPTR " \n",
          (size_t)TQ_CONST,(size_t)TQ_VOLATILE,(size_t)(TQ_CONST | TQ_VOLATILE),(size_t)TQ_RESTRICT);
#endif

  // if (sup->qualifiers & (TQ_CONST | TQ_VOLATILE))
     if ( (sup->qualifiers & TQ_CONST) && (sup->qualifiers & TQ_VOLATILE) )
        {
#if 0
          printf ("##### Mark member function as const volatile. \n");
#endif
          functionConstVolatileFlags |= (SgMemberFunctionType::e_const | SgMemberFunctionType::e_volatile);
        }
       else
        {
       // DQ (1/19/2019): If not using both const and volatile, then set them seperately.
          if (sup->qualifiers & TQ_CONST)
             {
#if 0
               printf ("##### Mark member function as const. \n");
#endif
               functionConstVolatileFlags |= SgMemberFunctionType::e_const;
             }
            else
             {
#if 0
               printf ("##### Member function NOT marked as const. \n");
#endif
             }

          if (sup->qualifiers & TQ_VOLATILE)
             {
#if 0
               printf ("##### Mark member function as volatile. \n");
#endif
               functionConstVolatileFlags |= SgMemberFunctionType::e_volatile;
             }
        }

  // if (sup->qualifiers & TQ_RESTRICT)
     if (sup->this_qualifiers & TQ_RESTRICT)
        {
#if 0
          printf ("##### Mark member function as restrict. \n");
#endif
          functionConstVolatileFlags |= SgMemberFunctionType::e_restrict;
        }

     if (sup->ref_qualifiers == rqk_lvalue)
        {
#if 0
          printf ("##### Mark member function as lvalue reference. \n");
#endif
          functionConstVolatileFlags |= SgMemberFunctionType::e_ref_qualifier_lvalue;
        }

     if (sup->ref_qualifiers == rqk_rvalue)
        {
#if 0
          printf ("##### Mark member function as rvalue reference. \n");
#endif
          functionConstVolatileFlags |= SgMemberFunctionType::e_ref_qualifier_rvalue;
        }

#if MICROSOFT_EXTENSIONS_ALLOWED
     if (sup->qualifiers & TQ_UNALIGNED)
        {
          printf ("##### Mark member function as aligned (Microsoft extension not yet supported) \n");
        }
#endif
#if NEAR_AND_FAR_ALLOWED
     if (sup->qualifiers & TQ_NEAR)
        {
          printf ("##### Mark member function as near (Microsoft extension not yet supported) \n");
        }
     if (sup->qualifiers & TQ_FAR)
        {
          printf ("##### Mark member function as far (Microsoft extension not yet supported) \n");
        }
#endif
#if UPC_EXTENSIONS_ALLOWED
     if (sup->qualifiers & TQ_UPC_SHARED)
        {
          printf ("##### Mark member function as UPC shared (UPC function type extension not yet supported) \n");
        }
     if (sup->qualifiers & TQ_UPC_STRICT)
        {
          printf ("##### Mark member function as UPC strict (UPC function type extension not yet supported) \n");
        }
     if (sup->qualifiers & TQ_UPC_RELAXED)
        {
          printf ("##### Mark member function as UPC relaxed (UPC function type extension not yet supported) \n");
        }
#endif

#if 0
     printf ("In setupFunctionTypeModifiers(): Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

     return functionConstVolatileFlags;
   }

void setup_block_statement_map ( a_source_sequence_entry_ptr                     sse_value,
                                                  vector<SgStatement*>                          & stmts,
                                                  map<a_source_sequence_entry_ptr,SgStatement*> & block_statement_map )
   {
  // DQ (1/2/2018): This function refactors the code to setup the block_statement_map to support the generality of switch statements.
  // This function is called from both parse_statement_list() and parse_statement_list_without_terminator().

     block_statement_map.insert(pair<a_source_sequence_entry_ptr,SgStatement*>(sse_value,stmts[0]));

  // Note that we have to handle the switch statement as a special case, becasue it hides the following statement in the processing of the block.
     SgLabelStatement* labelStatement = isSgLabelStatement(stmts[0]);
     if (labelStatement != NULL)
        {
       // Then add the next sse entry and the label statement body to the block_statement_map.

#if DEBUG_PARSE_STATEMENT_LIST
          printf ("Adding the label's body into the block_statement_map \n");
#endif
          SourceSequenceList sse_body = sse_value;
          ++sse_body;
          a_source_sequence_entry_ptr next_sse_value = sse_body.value();

          SgStatement* label_body = labelStatement->get_statement();
          ROSE_ASSERT(label_body != NULL);

#if DEBUG_PARSE_STATEMENT_LIST
          printf ("Processing label body = %p = %s \n",label_body,label_body->class_name().c_str());
#endif
          block_statement_map.insert(pair<a_source_sequence_entry_ptr,SgStatement*>(next_sse_value,label_body));
        }
   }

static int compute_number_of_declarations_before_switch_case_statement (vector<a_statement_ptr>::iterator stmt_itr, vector<a_statement_ptr> & block_list_perspective, int previous_count)
   {
     int edg_variable_declaration_counter  = 0;
     int rose_variable_declaration_counter = 0;

  // DQ (4/4/2021): The count needs to be computed on the block_list_perspective, instead of the result.
#if 0
     printf ("In compute_number_of_declarations_before_switch_case_statement(): Compute the number of variable declarations up front \n");
#endif

  // for (size_t i = 0; i < block_list_perspective.size(); i++)
  // vector<a_statement_ptr>::iterator stmt_itr = block_list_perspective.begin();

     int edg_stmt_counter = 0;
  // while (stmt_itr != block_list_perspective.end())
     while ((*stmt_itr)->kind == stmk_decl && stmt_itr != block_list_perspective.end())
        {
#if 0
       // printf ("   --- block_list_perspective[%zu] = %p \n",i,block_list_perspective[i]);
          printf ("   --- edg_stmt_counter = %d stmt_itr = %p \n",edg_stmt_counter,*stmt_itr);
#endif
       // DQ (4/4/2021): If this is a variable declaration, now many variables are there?
          if ((*stmt_itr)->kind == stmk_decl)
             {
#if 0
               printf ("   ---   --- --- found stmt_decl \n");
#endif
               an_il_entity_list_entry_ptr entities = (*stmt_itr)->variant.decl.entities;

               if (entities == NULL)
                  {
#if 1
                    printf ("in loop over stmt_itr: Found entities == NULL: \n");
#endif
#if 1
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }

               while (entities != NULL)
                  {
#if 0
                    a_tagged_pointer entity = entities->entity;
                    printf ("   ---   --- --- --- entities = %p \n",entities);
                    printf ("   ---   --- --- --- entity.kind = %d = %s \n",entity.kind,summarize_entry_kind_in_string_kind((an_il_entry_kind)(entity.kind)));
                    printf ("   ---   --- --- --- entity.ptr  = %p \n",entity.ptr);
#endif
                    edg_variable_declaration_counter++;
#if 0
                    printf ("in loop over entities: after increment: edg_variable_declaration_counter = %d \n",edg_variable_declaration_counter);
#endif
                    entities = entities->next;
                  }
             }

          edg_stmt_counter++;

          stmt_itr++;
        }

#if 0
     printf ("In compute_number_of_declarations_before_switch_case_statement(): before adjustment: edg_variable_declaration_counter = %d edg_stmt_counter = %d \n",edg_variable_declaration_counter,edg_stmt_counter);
#endif

  // This is the number of extra ROSE statments that will be generated ans which we need to skip over before we inset a case or default statement.
  // rose_variable_declaration_counter = edg_variable_declaration_counter - edg_stmt_counter;
     rose_variable_declaration_counter = (edg_variable_declaration_counter - edg_stmt_counter) + previous_count;

#if 0
  // printf ("Leaving compute_number_of_declarations_before_switch_case_statement(): edg_variable_declaration_counter = %d \n",edg_variable_declaration_counter);
     printf ("Leaving compute_number_of_declarations_before_switch_case_statement(): rose_variable_declaration_counter = %d \n",rose_variable_declaration_counter);
#endif

#if 0
     a_statement_ptr location_edg_statement = *stmt_itr;
     if (stmt_itr == block_list_perspective.end())
        {
#if 0
          printf ("In compute_number_of_declarations_before_switch_case_statement(): location_edg_statement == block_list_perspective.end(): reset location_edg_statement = NULL \n");
#endif
          location_edg_statement = NULL;
        }

     if (location_edg_statement != NULL)
        {
          printf ("In compute_number_of_declarations_before_switch_case_statement(): iterate past SgVariableDeclaration nodes: location_edg_statement = %p \n",location_edg_statement);
        }
       else
        {
          printf ("In compute_number_of_declarations_before_switch_case_statement(): iterate past SgVariableDeclaration nodes: location_edg_statement == NULL \n");
        }
#endif

     if (stmt_itr != block_list_perspective.end())
        {
          stmt_itr++;
#if 0
          printf ("*stmt_itr = %p \n",*stmt_itr);
#endif
        }

     return rose_variable_declaration_counter;
   }


#define DEBUG_SETUP_SWITCH_DATA 0

void setup_duffs_device_switch_data(vector<a_statement_ptr>                       & block_list_perspective,
                                                     vector<a_source_sequence_entry_ptr>           & block_source_sequence_perspective, 
                                                     map<a_source_sequence_entry_ptr,SgStatement*> & block_statement_map,
                                                     vector<SgStatement*>                          & result )
   {
  // DQ (12/11/2017): This function refactors the code for the handling of blocks.  It is called by 
  // both parse_statement_list_without_terminator() and parse_statement_list().

#if DEBUG_SETUP_SWITCH_DATA || 0
     printf ("At top of setup_duffs_device_switch_data(): result.size() = %zu \n",result.size());

     printf ("block_list_perspective.size() = %zu \n",block_list_perspective.size());
     for (size_t i = 0; i < block_list_perspective.size(); i++)
        {
          printf ("   --- block_list_perspective[%zu] = %p \n",i,block_list_perspective[i]);
          if (block_list_perspective[i] != NULL)
             {
               printf ("   ---   --- block_list_perspective[%zu] = %p = %s \n",i,block_list_perspective[i],summarize_statement_in_string(block_list_perspective[i]));
             }

       // DQ (4/4/2021): If this is a variable declaration, now many variables are there?
          if (block_list_perspective[i]->kind == stmk_decl)
             {
               printf ("   ---   --- --- found stmt_decl \n");
               an_il_entity_list_entry_ptr entities = block_list_perspective[i]->variant.decl.entities;
               if (entities == NULL)
                  {
                    printf ("Could this be an attribute? \n");
                    printf ("Calling summarize_statement() \n");
                    summarize_statement(block_list_perspective[i]);
                  }
               while (entities != NULL)
                  {
                    a_tagged_pointer entity = entities->entity;
#if DEBUG_SETUP_SWITCH_DATA || 0
                    printf ("   ---   --- --- --- entities = %p \n",entities);
                    printf ("   ---   --- --- --- entity.kind = %d = %s \n",entity.kind,summarize_entry_kind_in_string_kind((an_il_entry_kind)(entity.kind)));
                    printf ("   ---   --- --- --- entity.ptr  = %p \n",entity.ptr);
#endif
                    entities = entities->next;
                  }
             }
        }

     printf ("Before loop to insert case statements: output the result vector of statements: result.size() = %zu \n",result.size());
     for (size_t i = 0; i < result.size(); i++)
        {
          printf ("   --- result[%zu] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
        }

     printf ("Adding case statements: block_list_perspective.size() = %zu \n",block_list_perspective.size());
     for (size_t i = 0; i < block_list_perspective.size(); i++)
        {
          printf ("   --- block_list_perspective[%zu] = %p \n",i,block_list_perspective[i]);
          if (block_list_perspective[i] != NULL)
             {
               printf ("   ---   --- block_list_perspective[%zu] = %p = %s \n",i,block_list_perspective[i],summarize_statement_in_string(block_list_perspective[i]));
               if (block_list_perspective[i]->kind == stmk_switch_case)
                  {
                 // Add the associated siwtch case to the result vector.
                    a_switch_case_entry_ptr edg_switch_case_info = block_list_perspective[i]->variant.switch_case.extra_info;
                    ROSE_ASSERT(edg_switch_case_info != NULL);
                  }
             }
        }

     printf ("block_source_sequence_perspective.size() = %zu \n",block_source_sequence_perspective.size());
     for (size_t i = 0; i < block_source_sequence_perspective.size(); i++)
        {
          printf ("   --- block_source_sequence_perspective[%zu] = %p \n",i,block_source_sequence_perspective[i]);
       // if (block_list_perspective[i] != NULL)
          if (block_source_sequence_perspective[i] != NULL)
             {
            // printf ("   ---   --- block_source_sequence_perspective[%zu] = %p = %s \n",i,block_source_sequence_perspective[i],summarize_statement_in_string(block_source_sequence_perspective[i]));
               printf ("   ---   --- block_source_sequence_perspective[%zu] = %p = %s \n",i,block_source_sequence_perspective[i],
                    summarize_entry_kind_in_string_kind(block_source_sequence_perspective[i]->entity).c_str());
             }
        }

     printf ("block_statement_map.size() = %zu \n",block_statement_map.size());
     int counter = 0;
     for (map<a_source_sequence_entry_ptr,SgStatement*>::iterator i = block_statement_map.begin(); i != block_statement_map.end(); i++)
        {
#if 0
          printf ("   --- block_statement_map[%d]: first = %p second = %p \n",counter,i->first,i->second);
#else
          printf ("   --- block_statement_map[%d]: first = %p  entity = %p = %s second = %p = %s \n",counter,
               i->first,i->first->entity.ptr,summarize_entry_kind_in_string_kind(i->first->entity).c_str(),
               i->second,i->second->class_name().c_str());
#endif
          counter++;
        }

     printf ("result.size() = %zu \n",result.size());
     for (size_t i = 0; i < result.size(); i++)
        {
          printf ("   --- result[%zu] = %p \n",i,result[i]);
          if (result[i] != NULL)
             {
               printf ("   ---   --- result[%zu] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
             }
        }
#endif

     int rose_variable_declaration_counter = 0;

     vector<SgStatement*>::iterator location = result.begin();
     vector<a_statement_ptr>::iterator stmt_itr = block_list_perspective.begin();

#if DEBUG_SETUP_SWITCH_DATA
     printf ("before loop: rose_variable_declaration_counter = %d \n",rose_variable_declaration_counter);
     printf ("Start of for loop over the block_list_perspective (size = %zu) \n",block_list_perspective.size());
#endif

     int numberOfAddedCaseStatements  = 0;
     int numberOfExtraRoseStatements  = 0;
     int numberOfSkippedEdgStatements = 0;

     bool label_found_at_end_of_block_list_perspective = false;

#define USE_block_list_perspective 1

#if USE_block_list_perspective
  // for (size_t i = 0; i < block_list_perspective.size(); i++)
     size_t i = 0;
     vector<a_statement_ptr>::iterator block_list_perspective_itr = block_list_perspective.begin();
     while (block_list_perspective_itr != block_list_perspective.end())
#else
     for (size_t i = 0; i < block_source_sequence_perspective.size(); i++)
#endif
        {
#if USE_block_list_perspective
#if DEBUG_SETUP_SWITCH_DATA
          printf ("\n########### TOP of loop for i = %zu \n",i);
          printf ("   --- block_list_perspective[%zu] = %p \n",i,block_list_perspective[i]);
#endif
          a_statement_ptr edg_statement = block_list_perspective[i];

       // DQ (4/5/2021): Found a null pointer, debugging this.
          if (edg_statement == NULL)
             {
               printf ("Error: edg_statement == NULL: i = %zu block_list_perspective.size() = %zu \n",i,block_list_perspective.size());

               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
             }

       // This is to identify attributes (see test2020_14.c).
          if (edg_statement->kind == stmk_decl)
             {
#if DEBUG_SETUP_SWITCH_DATA
               printf ("Found stmk_decl statement: if there was not a ROSE statement generated from this then we need to increment numberOfSkippedEdgStatements \n");
               printf ("before increment: numberOfSkippedEdgStatements = %d \n",numberOfSkippedEdgStatements);
#endif
            // Check for an empty list if entities, if so then this is a list that didn't generate 
            // a ROSE statement (likely an gnu __attribute__(()) or something like that).
               an_il_entity_list_entry_ptr entities = edg_statement->variant.decl.entities;
               if (entities == NULL)
                  {
                 // Nothing was generated from this deg statement (stmk_decl) so we just skip it, are record it as skipped.
#if DEBUG_SETUP_SWITCH_DATA
                    printf ("Calling summarize_statement(): \n");
                    summarize_statement(edg_statement);
#endif
                    numberOfSkippedEdgStatements++;

#if DEBUG_SETUP_SWITCH_DATA
                    printf ("after increment: numberOfSkippedEdgStatements = %d \n",numberOfSkippedEdgStatements);
#endif
                  }
             }

          if (edg_statement->kind == stmk_label)
             {
#if DEBUG_SETUP_SWITCH_DATA
               printf ("Found a label: edg_statement = %p (increment past it to the next statement) \n",edg_statement);
#endif

            // Increment the count, and the iterator.
               i++;

               ROSE_ASSERT(block_list_perspective_itr != block_list_perspective.end());

            // Increment the count, and the iterator.
               block_list_perspective_itr++;

               numberOfSkippedEdgStatements++;

            // ROSE_ASSERT(block_list_perspective_itr != block_list_perspective.end());
               if (block_list_perspective_itr != block_list_perspective.end())
                  {
                    edg_statement = *block_list_perspective_itr;
                    ROSE_ASSERT(edg_statement != NULL);
                  }
                 else
                  {
#if DEBUG_SETUP_SWITCH_DATA
                    printf ("Calling summarize_statement(edg_statement = %p) \n",edg_statement);
                    summarize_statement(edg_statement);
                    printf ("Found case of stmk_label as the end of block_list_perspective: set edg_statement == NULL \n");
#endif
                    edg_statement = NULL;
                    label_found_at_end_of_block_list_perspective = true;
                  }
             }

          if (edg_statement != NULL)
             {
#if DEBUG_SETUP_SWITCH_DATA
            // printf ("   ---   --- block_list_perspective[%zu] = %p = %s \n",i,block_list_perspective[i],summarize_statement_in_string(block_list_perspective[i]));
               printf ("   ---   --- i = %zu edg_statement = %p = %s \n",i,edg_statement,summarize_statement_in_string(edg_statement));
#endif
            // if (block_list_perspective[i]->kind == stmk_switch_case)
               if (edg_statement->kind == stmk_switch_case)
                  {
                 // Add the associated switch case to the result vector.

#if 1
                 // vector<a_statement_ptr>::iterator stmt_itr = block_list_perspective.begin();
                 // rose_variable_declaration_counter = compute_number_of_declarations_before_switch_case_statement (stmt_itr,block_list_perspective);
#if DEBUG_SETUP_SWITCH_DATA
                    printf ("in loop: before computation: rose_variable_declaration_counter = %d numberOfAddedCaseStatements = %d \n",rose_variable_declaration_counter,numberOfAddedCaseStatements);
#endif
                 // int previous_count = rose_variable_declaration_counter + numberOfAddedCaseStatements;
                    int previous_count = 0; // rose_variable_declaration_counter;
                 // rose_variable_declaration_counter = compute_number_of_declarations_before_switch_case_statement (stmt_itr,block_list_perspective,previous_count);
                    rose_variable_declaration_counter = compute_number_of_declarations_before_switch_case_statement (stmt_itr,block_list_perspective,previous_count);
#endif
                    numberOfExtraRoseStatements += rose_variable_declaration_counter;
#if DEBUG_SETUP_SWITCH_DATA
                    printf ("in loop: after computation: rose_variable_declaration_counter = %d numberOfAddedCaseStatements = %d \n",rose_variable_declaration_counter,numberOfAddedCaseStatements);
#endif

                 // a_switch_case_entry_ptr edg_switch_case_info = p->variant.switch_case.extra_info;
                 // ROSE_ASSERT(edg_switch_case_info != NULL);
                 // switch_case_map.insert(pair<a_switch_case_entry_ptr,SgStatement*>(edg_switch_case_info,case_statement));

                 // a_switch_case_entry_ptr edg_switch_case_info = block_list_perspective[i]->variant.switch_case.extra_info;
                    a_switch_case_entry_ptr edg_switch_case_info = edg_statement->variant.switch_case.extra_info;
                    ROSE_ASSERT(edg_switch_case_info != NULL);

#if DEBUG_SETUP_SWITCH_DATA
                    printf ("   ---   --- handle case of stmk_switch_case \n");
#endif
                    size_t result_location_offset = (i-numberOfSkippedEdgStatements)+numberOfExtraRoseStatements;

#if DEBUG_SETUP_SWITCH_DATA
                    printf ("in loop: i = %zu rose_variable_declaration_counter = %d numberOfExtraRoseStatements = %d \n",i,rose_variable_declaration_counter,numberOfExtraRoseStatements);
                 // printf ("in loop: result vector offset = %zu \n",(i-numberOfSkippedEdgStatements)+numberOfExtraRoseStatements);
                    printf ("in loop: result_location_offset = %zu \n",result_location_offset);
#endif
                 // location = result.begin()+(i+numberOfAddedCaseStatements+rose_variable_declaration_counter);
                 // location = result.begin()+(i+rose_variable_declaration_counter);
                 // location = result.begin()+(i+numberOfExtraRoseStatements);
                 // Need to account for any statements that we skipped over in the EDG lost (numberOfSkippedEdgStatements).
                 // location = result.begin()+((i-numberOfSkippedEdgStatements)+numberOfExtraRoseStatements);
                    location = result.begin()+result_location_offset;

                    SgStatement* location_statement = *location;
                 // if (location == result.end() || ((i-numberOfSkippedEdgStatements)+numberOfExtraRoseStatements) >= result.size())
                    if (location == result.end() || result_location_offset >= result.size())
                       {
#if DEBUG_SETUP_SWITCH_DATA
                         printf ("in loop: location == result.end(): reset location_statement = NULL \n");
#endif
                         location_statement = NULL;
                       }
                 // ROSE_ASSERT(location_statement != NULL);

#if DEBUG_SETUP_SWITCH_DATA
                    printf ("in loop: (i-numberOfSkippedEdgStatements)+rose_variable_declaration_counter = %zu location_statement = %p \n",(i-numberOfSkippedEdgStatements)+rose_variable_declaration_counter,location_statement);
                    if (location_statement != NULL)
                       {
                         printf ("in loop: (i-numberOfSkippedEdgStatements)+rose_variable_declaration_counter = %zu location_statement = %p = %s \n",(i-numberOfSkippedEdgStatements)+rose_variable_declaration_counter,
                              location_statement,location_statement->class_name().c_str());
                       }
#endif
                 // if (location_statement != NULL && ((i-numberOfSkippedEdgStatements)+numberOfExtraRoseStatements) > 0)
                    if (location_statement != NULL && result_location_offset > 0)
                       {
                      // SgStatement* previousStatement = result[(i+numberOfExtraRoseStatements)-1];
                      // SgStatement* previousStatement = result[((i-numberOfSkippedEdgStatements)+numberOfExtraRoseStatements)-1];
                         SgStatement* previousStatement = result[result_location_offset-1];
                         ROSE_ASSERT(previousStatement != NULL);

                         SgPragmaDeclaration* pragmaDeclaration = isSgPragmaDeclaration(previousStatement);
                         if (pragmaDeclaration != NULL)
                            {
#if DEBUG_SETUP_SWITCH_DATA || 1
                              printf ("########### Set location for insertion to previous statement (can't have pragma before case) \n");
#endif
                           // location = result.begin()+((i + numberOfExtraRoseStatements) + 1);
                           // location = result.begin()+(((i-numberOfSkippedEdgStatements)+numberOfExtraRoseStatements) + 1);
                              location = result.begin()+(result_location_offset + 1);
                              numberOfExtraRoseStatements++;
                            }
                       }

                 // DQ (3/25/2021): This can be combined into a single branch.
                    if (switch_case_map.find(edg_switch_case_info) != switch_case_map.end())
                       {
                         SgStatement* switch_case = switch_case_map[edg_switch_case_info];
                         ROSE_ASSERT(switch_case != NULL);
#if DEBUG_SETUP_SWITCH_DATA
                         printf ("Found switch_case = %p = %s i = %zu \n",switch_case,switch_case->class_name().c_str(),i);
#endif
                      // Add to the result list, insert in front of result statement at index i.
                      // vector<SgStatement*>::iterator it = result.begin();
                      // vector<SgStatement*> tmp_result = result;
                      // result.insert (it+i,tmp_result.begin(),tmp_result.end());
                      // SgStatement** switch_case_ptr_ptr = &switch_case;
                      // result.insert (result.begin(),switch_case_ptr_ptr,switch_case_ptr_ptr);
#if DEBUG_SETUP_SWITCH_DATA || 0
                         printf ("switch_case = %p = %s \n",switch_case,switch_case->class_name().c_str());
                         printf (" --- result.size() = %zu \n",result.size());
                         printf (" --- i = %zu \n",i);
                         printf (" --- result_location_offset = %zu \n",result_location_offset);
#endif
                      // vector<SgStatement*>::iterator location = result.begin()+i;
                      // result.insert (location,switch_case);
                      // if (i+1 == result.size())
                         if (result_location_offset+1 == result.size())
                            {
#if DEBUG_SETUP_SWITCH_DATA
                              printf ("Found switch_case: Problem case: i+1 == result.size() \n");
#endif
                           // vector<SgStatement*>::iterator location = result.begin()+i;
                              result.insert (location,switch_case);
                            }
                           else
                            {
#if DEBUG_SETUP_SWITCH_DATA
                              printf ("Found switch_case: Simple case: i+1 != result.size() \n");
#endif
                           // if (i+1 < result.size())
                              if (result_location_offset+1 < result.size())
                                 {
                                // vector<SgStatement*>::iterator location = result.begin()+i;
                                   result.insert (location,switch_case);
                                 }
                                else
                                 {
#if DEBUG_SETUP_SWITCH_DATA
                                   printf ("Found switch_case: Problem case: i+1 > result.size() \n");
#endif
#if 0
                                // vector<SgStatement*>::iterator location = result.begin()+(i-1)+rose_variable_declaration_counter;
                                   vector<SgStatement*>::iterator location = result.begin()+(result_location_offset-1);
                                   result.insert (location,switch_case);
#else
                                   printf ("switch_case = %p = %s \n",switch_case,switch_case->class_name().c_str());
                                   printf (" --- result.size() = %zu \n",result.size());
                                   printf (" --- i = %zu \n",i);
                                   printf (" --- result_location_offset = %zu \n",result_location_offset);
#if 1
                                   printf ("Exiting as a test! \n");
                                   ROSE_ASSERT(false);
#endif
#endif
#if 0
                                   printf ("Exiting as a test! \n");
                                   ROSE_ASSERT(false);
#endif
                                 }
                            }
                       }
                      else
                       {
                      // The default case is never visited (not where the other case statements are visited) except for here (not clear why).
#if DEBUG_SETUP_SWITCH_DATA
                         printf ("edg_switch_case_info = %p not available in switch_case_map (likely a default case statement) \n",edg_switch_case_info);
#endif
#if 1
                      // a_statement_ptr edg_switch_statement = block_list_perspective[i]->variant.switch_case.switch_statement;
                         a_statement_ptr edg_switch_statement = edg_statement->variant.switch_case.switch_statement;
                         ROSE_ASSERT(edg_switch_statement != NULL);

                      // a_switch_case_entry_ptr edg_switch_case_info = block_list_perspective[i]->variant.switch_case.extra_info;
                         a_switch_case_entry_ptr edg_switch_case_info = edg_statement->variant.switch_case.extra_info;
                         ROSE_ASSERT(edg_switch_case_info != NULL);

                      // The stmk_switch_case statement pointing to this entry.
                      // a_statement_ptr edg_switch_case_statement = edg_switch_case_info->stmt;
#if 0
                         printf ("Calling summarize_statement() \n");
                         summarize_statement(edg_switch_statement);
#endif
#if 0
                         printf ("Calling summarize_switch_case() \n");
                         summarize_switch_case(edg_switch_case_info);
#endif
                      // The integer value associated with this case.  (Or the value of the start
                      // of the range if this is a GNU case range.)  NULL for a "default:" case.
                      // a_constant_ptr edg_case_value = edg_switch_case_info->case_value;
#if 0
                         printf ("Calling summarize_constant() \n");
                         summarize_constant(edg_case_value);
#endif
#if GNU_EXTENSIONS_ALLOWED
                      // If this is a GNU case range, the value of the end of the range; otherwise, NULL.
                      // a_constant_ptr edg_case_range_end = edg_switch_case_info->range_end;
#endif
                      // TRUE if this switch case can be reached by falling through to it from the code immediately preceding.
                      // bool reachable_by_fall_through = edg_switch_case_info->reachable_by_fall_through;
#if DEBUG_SETUP_SWITCH_DATA
                         printf ("case stmk_switch_case: EDG 6.x or greater: Build a list of case statements and insert them into the correct switch statements and into the correct sequence later \n");
#endif
                         SgStatement* case_statement = convert_switch_case(edg_switch_case_info);
                         ROSE_ASSERT(case_statement != NULL);
#if DEBUG_SETUP_SWITCH_DATA
                         printf ("case_statement = %p = %s \n",case_statement,case_statement->class_name().c_str());
                         printf (" --- result.size() = %zu \n",result.size());
                         printf (" --- i = %zu \n",i);
                         printf (" --- result_location_offset = %zu \n",result_location_offset);
#endif
                      // SgStatement* switch_case = switch_case_map[edg_switch_case_info];
                      // ROSE_ASSERT(switch_case != NULL);

                      // vector<SgStatement*>::iterator location = result.begin()+i;
                      // result.insert (location,case_statement);
                      // if (i+1 == result.size())
                         if (result_location_offset+1 == result.size())
                            {
#if DEBUG_SETUP_SWITCH_DATA
                              printf ("Found default_case: Problem case: i+1 == result.size() \n");
#endif
                           // vector<SgStatement*>::iterator location = result.begin()+i;
                              result.insert (location,case_statement);
                            }
                           else
                            {
#if DEBUG_SETUP_SWITCH_DATA
                              printf ("Found default_case: Simple case: i+1 != result.size() \n");
#endif
                           // vector<SgStatement*>::iterator location = result.begin()+i;
                           // result.insert (location,case_statement);
                           // if (i+1 < result.size())
                              if (result_location_offset+1 < result.size())
                                 {
                                // vector<SgStatement*>::iterator location = result.begin()+i;
                                   result.insert (location,case_statement);
                                 }
                                else
                                 {
#if DEBUG_SETUP_SWITCH_DATA
                                   printf ("Found default_case: Problem case: i+1 > result.size() \n");
#endif
#if 0
                                // vector<SgStatement*>::iterator location = result.begin()+(i-1)+rose_variable_declaration_counter;
                                   vector<SgStatement*>::iterator location = result.begin()+(result_location_offset-1);
                                   result.insert (location,case_statement);
#else
                                   printf ("case_statement = %p = %s \n",case_statement,case_statement->class_name().c_str());
                                   printf (" --- result.size() = %zu \n",result.size());
                                   printf (" --- i = %zu \n",i);
                                   printf (" --- result_location_offset = %zu \n",result_location_offset);
#if 1
                                   printf ("Exiting as a test! \n");
                                   ROSE_ASSERT(false);
#endif
#endif
#if 0
                                   printf ("Exiting as a test! \n");
                                   ROSE_ASSERT(false);
#endif
                                 }
                            }
#if 0
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
#endif
                       }

                    numberOfAddedCaseStatements++;

                 // Advance the stmt_itr to this point in the block_list_perspective.
#if DEBUG_SETUP_SWITCH_DATA
                    printf ("We need to advance the stmt_itr to this point in the block_list_perspective \n");
#endif
                 // while ((*stmt_itr) != block_list_perspective[i] && stmt_itr != block_list_perspective.end())
                    while ((*stmt_itr) != edg_statement && stmt_itr != block_list_perspective.end())
                       {
#if DEBUG_SETUP_SWITCH_DATA
                      // printf ("   --- block_list_perspective[%zu] = %p \n",i,block_list_perspective[i]);
                         printf ("   --- block_list_perspective[%zu] = %p \n",i,edg_statement);
                         printf ("   --- stmt_itr = %p \n",*stmt_itr);
#endif
#if 0
                      // DQ (4/4/2021): If this is a variable declaration, now many variables are there?
                         if ((*stmt_itr)->kind == stmk_decl)
                            {
                              printf ("   ---   --- --- found stmt_decl \n");
                            }
#endif
                         stmt_itr++;
                       }

                 // ROSE_ASSERT((*stmt_itr) == block_list_perspective[i]);
                    ROSE_ASSERT((*stmt_itr) == edg_statement);
#if DEBUG_SETUP_SWITCH_DATA
                    printf ("Increment past the current entry defined by block_list_perspective[%zu] \n",i);
#endif
                    ROSE_ASSERT(stmt_itr != block_list_perspective.end());

                 // DQ (4/4/2021): Increment past the current entry defined by block_list_perspective[i].
                    stmt_itr++;

#if DEBUG_SETUP_SWITCH_DATA
                    printf ("at base of loop: result.size() = %zu \n",result.size());
                    for (size_t i = 0; i < result.size(); i++)
                       {
                         printf ("   --- result[%zu] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
                       }
#endif
#if 0
                    printf ("Exiting as a test: add the associated siwtch case to the result vector \n"); 
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
#if DEBUG_SETUP_SWITCH_DATA
                 // printf ("block_list_perspective[i]->kind != stmk_switch_case) \n");
                    printf ("i = %zu edg_statement->kind != stmk_switch_case \n",i);
                    printf ("   --- stmt_itr = %p \n",*stmt_itr);
#endif

                    if ((*stmt_itr)->kind == stmk_decl)
                       {
#if DEBUG_SETUP_SWITCH_DATA
                         printf ("i = %zu edg_statement->kind != stmk_switch_case: found stmt_decl \n",i);
                         printf ("   --- stmt_itr = %p \n",*stmt_itr);
#endif
#if 0
                         printf ("Increment stmt_itr \n");
                         stmt_itr++;
#endif
                       }
                  }
             }
            else
             {
#if DEBUG_SETUP_SWITCH_DATA
            // printf ("   ---   --- block_list_perspective[%zu] == NULL \n",i);
               printf ("   ---   --- i = %zu edg_statement == NULL \n",i);
#endif

            // DQ (4/5/2021): Sometimes in C89 mode there is a compiler generated label at the end of 
            // the block_list_perspective list.  In this case I think we can ignore it.
               if (label_found_at_end_of_block_list_perspective == false)
                  {
#if 1
                 // printf ("Exiting as a test: block_list_perspective[%zu] == NULL \n",i); 
                    printf ("Exiting as a test: i = %zu edg_statement == NULL \n",i); 
                    ROSE_ASSERT(false);
#endif
                  }
             }

#if DEBUG_SETUP_SWITCH_DATA
          printf ("########### Base of loop for i = %zu \n",i);
#endif
       // DQ (4/5/2021): Added to support while loop, so that we can skip over some labels when using C89 mode (which addes labels for each break statement)
       // Increment the count, and the iterator.
          i++;

       // block_list_perspective_itr++;
          if (block_list_perspective_itr != block_list_perspective.end())
             {
               block_list_perspective_itr++;
             }
            else
             {
#if DEBUG_SETUP_SWITCH_DATA
               printf ("LAST ITERATION OF LOOP: i = %zu \n",i);
#endif
             }
        }
#endif

#if 0
        printf ("Exiting as a test! \n");
        ROSE_ASSERT(false);
#endif

#if 0
     if (block_statement_map.size() == 0)
        {
          printf ("ERROR: In setup_duffs_device_switch_data(): there should be entries in the block_statement_map (empty) \n");
          ROSE_ASSERT(false);
        }
#endif

  // DQ (12/29/2017): Added test as part of debugging.
     if (block_statement_map.empty() == false && block_source_sequence_perspective.empty() == true)
        {
          printf ("ERROR: In setup_duffs_device_switch_data(): If there are entries in block_statement_map there should be entries in the block_source_sequence_perspective (empty) \n");
          ROSE_ASSERT(false);
        }
   }

}
