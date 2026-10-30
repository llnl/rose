
#include "edgRose.h"

SgTryStmt*
EDG_ROSE_Translation::convert_try_block_statement(SourceSequenceList& sse, a_statement_ptr p)
   {
  // Generate code for the indicated stmk_try_block statement, for a C++ exception-handling "try" block.

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("convert_try_block_statement\n");
#endif

     ROSE_ASSERT(p != NULL);

     SgStatement* body = p->variant.try_block->statement == NULL ?
                         set_generated_file_info_to_unparse(buildBasicBlock_nfi()) :
                         build_basic_block_if_multiple_statements(parse_statement(sse));
     ROSE_ASSERT(body != NULL);

     SgTryStmt* tryStatement = buildTryStmt(body);
     ROSE_ASSERT(tryStatement != NULL);

  // We have to set the file info for the internally built SgCatchStatementSeq IR node.
     tryStatement->set_catch_statement_seq_root(set_generated_file_info_to_unparse(tryStatement->get_catch_statement_seq_root()));

  // DQ (9/21/2013): The parent should be set else it will be an error later (specifically 
  // if we define a type in the enclosed scope and the mangled name handling will have to 
  // generate a patch to the global scope using the parent pointers (and scopes).
     ROSE_ASSERT(tryStatement->get_parent() == NULL);
     tryStatement->set_parent(topScopeStack());
     ROSE_ASSERT(tryStatement->get_parent() != NULL);

#if 0
     printf ("In convert_try_block_statement(): tryStatement->get_parent() = %p = %s \n",tryStatement->get_parent(),tryStatement->get_parent()->class_name().c_str());
#endif

     a_handler_ptr handler = NULL;

  // Put out each handler ("catch" clause).
     for (handler = p->variant.try_block->handlers; handler != NULL; handler = handler->next)
        {
#if 0
          printf ("$$$$$$$$$$$$ Found a try block handler \n");
#endif
          SgCatchOptionStmt* catchStatement = set_generated_file_info_to_unparse(buildCatchOptionStmt());
          ROSE_ASSERT (catchStatement != NULL);

          tryStatement->append_catch_statement(catchStatement);

#if DEBUG_PUSH_SCOPE
          printf ("In convert_try_block_statement(): push scope stack: class scope = %p = %s \n",catchStatement,catchStatement->class_name().c_str());
#endif

       // DQ (9/21/2013): Push the new scope onto the stack.
          pushScopeStack(catchStatement);

       // DQ (9/27/2012): Setting the parent to the try statement body.
       // catchStatement->set_parent(body);

          a_variable_ptr handler_var = handler->parameter;
       // set_output_position_for_stmt(&handler->catch_position);

          if (handler_var == NULL)
             {
            // A NULL parameter means the handler can catch anything.
#if 0
               printf ("$$$$$$$$$$$$ A NULL parameter means the handler can catch anything. \n");
#endif
            // DQ (9/19/2004): variables of type "..." don't get a name (at least in a catch statement)!
               SgVariableDeclaration* sageDecl = new SgVariableDeclaration();
               ROSE_ASSERT (sageDecl != NULL);

            // DQ (11/3/2012): Added setting default source position info.
               setSourcePosition(sageDecl);

            // DQ (9/27/2012): Setting the parent of the SgVariableDeclaration to the try statement body.
            // sageDecl->set_parent(catchStatement);

            // DQ (2/12/2012): Added support to set the source code position.
               set_generated_file_info_to_unparse(sageDecl);

               SgInitializedName* initializedName = buildInitializedName_nfi ( SgName(""), SgTypeEllipse::createType(), NULL);
               ROSE_ASSERT(initializedName != NULL);
               initializedName->set_declptr(sageDecl);

            // DQ (2/12/2012): Set the scope.
               initializedName->set_scope(catchStatement);

            // DQ (2/12/2012): Set the file info before calling append_variable, so that it will be copied to the SgVariableDefinition.
               set_generated_file_info_to_unparse(initializedName);

               SgInitializer* initializer = NULL;
               sageDecl->append_variable(initializedName,initializer);

            // DQ (2/12/2012): Set the source position on the SgVariableDefinition.
            // This aspect of the SageBuilder API should be fixed so this is less trouble.
               ROSE_ASSERT(initializedName->get_declptr() != NULL);
               SgVariableDefinition* variableDefinition = isSgVariableDefinition(initializedName->get_declptr());
               ROSE_ASSERT(variableDefinition != NULL);
               if (variableDefinition->get_startOfConstruct() == NULL)
                  {
#if 0
                    printf ("Setting the source position on the SgVariableDefinition. \n");
#endif
                    set_generated_file_info_to_unparse(variableDefinition);
                  }

               catchStatement->set_condition(sageDecl);
            // Liao, 10/29/2009, missing parent pointer for sageDecl, bug 375
               sageDecl->set_parent(catchStatement);
            // this special declaration has no defining declaration

#if 0
               printf ("In convert_try_block_statement(): sageDecl = %p setting firstNondefiningDeclaration to %p and definingDeclaration to %p \n",sageDecl,sageDecl,(void*)NULL);
#endif
               sageDecl->set_firstNondefiningDeclaration(sageDecl);
               sageDecl->set_definingDeclaration(NULL);

               ROSE_ASSERT(initializedName->get_scope() != NULL);
               ROSE_ASSERT(sageDecl->get_scope() != NULL);
             }
            else
             {
            // Handle the parameter.  It may be unnamed.
#if 0
               printf ("In convert_try_block_statement(): Calling summarize_variable() on handler->parameter \n");
               summarize_variable(handler_var);
#endif
               ROSE_ASSERT(handler_var->is_handler_param == true);

            // DQ (11/6/2014): Adding C++11 support to record the use of C++11 syntax (for the unparser).
               if (handler_var->has_direct_braced_initializer == true)
                  {
                    printf ("In convert_try_block_statement(): Need to mark as using C++11 braced initializer syntax \n");
                 // ROSE_ASSERT(false);
                  }
#if 0
            // SgVariableSymbol* variableSymbol = convert_variable_use(handler_var);
            // ROSE_ASSERT(variableSymbol != NULL);
               SgInitializer* variableInitializer = convert_var_initializer(handler_var);
               ROSE_ASSERT(variableInitializer != NULL);
#endif
#if 0
               an_il_entry_kind kind = (an_il_entry_kind) (sse->entity.kind);
               printf ("What is this kind = %s \n",summarize_entry_kind_in_string_kind(kind));
               if (kind == iek_statement)
                  {
                 // SgStatement* statement = parse_statement(sse);
                    std::vector<SgStatement*> statement = parse_statement(sse); 
                  }
                 else
                  {
                    printf ("Error: unsupported kind = %s \n",summarize_entry_kind_in_string_kind(kind));
                    ROSE_ASSERT(false);
                  }
#endif
#if 0
               ROSE_ASSERT(!"Can we use a_field_ptr here? NO");
#endif
            // Can we use a_field_ptr here? NO
            // a_field_ptr p = (a_field_ptr)(sse->entity.ptr);
               a_variable_ptr p = handler_var;
               ROSE_ASSERT(p != NULL);
               SgType* type = convert_type(p->type);

            // DQ (2/10/2012): Added assertion.
               ROSE_ASSERT(type != NULL);

            // Note that p->source_corresp.name can be NULL (result is an empty string).
            // SgName name = p->source_corresp.name;
            // SgName name = generateName(p);
               SgName name = "";
               if (p->source_corresp.name != NULL)
                  {
                    name = generateName(p);
                  }
                 else
                  {
                 // For test2004_34.C p->source_corresp.name == NULL, so we have to handle this case.
                 // Note that sometimes the name is not provided (left empty), and test2004_34.C.

                 // Get the name from the type
                 // SgNamedType* namedType = isSgNamedType(type);
                 // ROSE_ASSERT(namedType != NULL);

                 // DQ (2/10/2012): We want this to be an unnamed variable since the name of the type is what is used in the unparsing.
                 // name = namedType->get_name();
                    name = "";
                  }

#if 0
               printf ("In convert_try_block_statement(): calling buildVariableDeclaration_nfi() \n");
#endif
            // SgVariableDeclaration* decl = set_file_info_for_corresp(buildVariableDeclaration_nfi(p->source_corresp.name, type, NULL, topScopeStack()), p->source_corresp);
            // SgVariableDeclaration* decl = set_file_info_for_corresp(buildVariableDeclaration_nfi(p->source_corresp.name, type, NULL, catchStatement), p->source_corresp);
            // SgVariableDeclaration* decl = buildVariableDeclaration_nfi(p->source_corresp.name, type, NULL, catchStatement);
               SgVariableDeclaration* decl = buildVariableDeclaration_nfi(name, type, NULL, catchStatement);

               ROSE_ASSERT(decl != NULL);
               if (decl->get_parent() == NULL)
                  {
                 // printf ("Setting the parent of the decl to the catchStatement \n");
                    decl->set_parent(catchStatement);
                  }
                 else
                  {
                 // printf ("NOTE: decl->get_parent() was already set to decl->get_parent() = %p = %s \n",decl->get_parent(),decl->get_parent()->class_name().c_str());
                    if (decl->get_parent() != catchStatement)
                       {
                      // This will be reported as an error in the AST consistancy check later.
                         printf ("Error: decl->get_parent() != catchStatement (catchStatement = %p = %s) \n",catchStatement,catchStatement->class_name().c_str());
#if 1
                         ROSE_ASSERT(false);
#endif
                       }
                  }

               if (decl->get_scope() == NULL)
                  {
                 // This is required for test2011_175.C (but not test2011_176.C).
                 // printf ("Setting the scope of the declaration to be the catchStatement \n");
                    decl->set_scope(catchStatement);
                  }
               ROSE_ASSERT(decl->get_scope() != NULL);
#if 0
               printf ("decl->get_scope() = %p = %s \n",decl->get_scope(),decl->get_scope()->class_name().c_str());
#endif
               if (p->source_corresp.decl_position.seq == 0)
                  {
                 // There is no source position available, so mark this as compiler generated.
#if 0
                    printf ("There is no source position available, so mark this as compiler generated \n");
                    ROSE_ASSERT(decl->get_startOfConstruct() != NULL);
                    decl->get_startOfConstruct()->display("There is no source position available, so mark this as compiler generated: debug");
#endif
#if 0
                    ROSE_ASSERT(decl->get_file_info() == NULL);
                 // decl->get_file_info()->setSourcePositionUnavailableInFrontend();
                    decl->set_startOfConstruct(Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode());
                    decl->set_endOfConstruct(Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode());

#error "DEAD CODE!"

#if 1
#if 0
                    printf ("************ convert_try_block_statement(): Marking decl for output decl = %p = %s \n",decl,decl->class_name().c_str());
#endif
                    decl->get_startOfConstruct()->setOutputInCodeGeneration();
                    decl->get_endOfConstruct()->setOutputInCodeGeneration();
#else
                    printf ("************ convert_try_block_statement(): Skipping the marking of decl for output decl = %p = %s \n",decl,decl->class_name().c_str());
#endif

#error "DEAD CODE!"

#else
                    ROSE_ASSERT(decl->get_startOfConstruct() != NULL);
                    ROSE_ASSERT(decl->get_endOfConstruct() != NULL);
                    decl->get_startOfConstruct()->setCompilerGenerated();
                    decl->get_endOfConstruct()->setCompilerGenerated();
#endif
                  }
                 else
                  {
                    decl = set_file_info_for_corresp(decl,p->source_corresp);
                  }

#if 0
               printf ("In convert_try_block_statement(): edg_type = %p type = %p = %s Calling fixup_variable_or_field_declaration() \n",p->type,type,type->class_name().c_str());
#endif
               fixup_variable_or_field_declaration(decl, p->type);

               SgInitializedName* in = set_file_info_for_corresp(getFirstInitializedName(decl), p->source_corresp);
               ROSE_ASSERT(in != NULL);

            // DQ (2/12/2012): Added test.
               ROSE_ASSERT(getFirstInitializedName(decl)->get_startOfConstruct() != NULL);

               if (p->source_corresp.decl_position.seq == 0)
                  {
                 // There is no source position available, so mark this as compiler generated.
                 // printf ("There is no source position available, so mark this as compiler generated \n");
                 // ROSE_ASSERT(in->get_file_info() == NULL);
                    if (in->get_startOfConstruct() == NULL)
                       {
                         ROSE_ASSERT(in->get_endOfConstruct() == NULL);
                         in->set_startOfConstruct(Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode());
                       }
#if 0
                    printf ("************ convert_try_block_statement(): Marking SgInitializedName for output in = %p = %s \n",in,in->get_name().str());
                    in->get_startOfConstruct()->setOutputInCodeGeneration();

#error "DEAD CODE!"

                    if (in->get_endOfConstruct() == NULL)
                       {
                         in->set_endOfConstruct(Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode());
                       }
                    in->get_endOfConstruct()->setOutputInCodeGeneration();
#else
#if 0
                    printf ("************ convert_try_block_statement(): Skipping the marking SgInitializedName for output in = %p = %s \n",in,in->get_name().str());
#endif
#endif
                  }
               
               set_file_info_for_corresp(decl->get_definition(), p->source_corresp);
               ROSE_ASSERT(in != NULL);
               in->set_scope(catchStatement);
               ROSE_ASSERT(in->get_scope() != NULL);
               catchStatement->set_condition(decl);

            // decl->get_file_info()->display("In convert_try_block_statement()");

#if 0
            // Advance the source sequence entry...
               ++sse;
#else
            // DQ (12/9/2011): Condional advance of source sequence entry (there might be a better way).
            // Only advance the source sequence if there is a variable in the source (see test2011_175.C for
            // where no named variable exists and test2011_176.C for there such a name variable exists).
#if 0
               printf ("In convert_try_block_statement(): note conditional advance of source sequence entry depending on if a named variable was found. \n");
#endif
               if (p->source_corresp.name != NULL)
                  {
                 // Advance the source sequence entry...
                    ++sse;
                  }
#endif
             }


       // DQ (9/21/2013): If there are any types declared in this set of statements then we need to have a SgBasicBlock scope to put them into.
       // Then we can use the 2nd parameter to pass the existing SgBasicBlock scope.  Note that we do not in this case push the SgBasicBlock 
       // onto the scope stack (I think this is handled internally in the parse_statement function.
          SgBasicBlock* blockStatement = buildBasicBlock_nfi();
#if 0
#if DEBUG_PUSH_SCOPE
          printf ("In convert_try_block_statement(): (SgBasicBlock): push scope stack: class scope = %p = %s \n",blockStatement,blockStatement->class_name().c_str());
#endif
       // DQ (9/21/2013): Push the new scope onto the stack.
          pushScopeStack(blockStatement);
#endif
       // SgStatement* handleBody = handler->statement == NULL ? set_generated_file_info_to_unparse(buildBasicBlock_nfi()) : build_basic_block_if_multiple_statements(parse_statement(sse));
          SgStatement* handleBody = handler->statement == NULL ? set_generated_file_info_to_unparse(buildBasicBlock_nfi()) : build_basic_block_if_multiple_statements(parse_statement(sse,blockStatement));
          ROSE_ASSERT(handleBody != NULL);

#if 0
#if DEBUG_POP_SCOPE
          printf ("In convert_try_block_statement(): calling popScopeStack() (SgBasicBlock) \n");
#endif
       // DQ (9/21/2013): Push the new scope onto the stack.
          popScopeStack();
#endif

          catchStatement->set_body(handleBody);
          handleBody->set_parent(catchStatement);

#if DEBUG_POP_SCOPE
          printf ("In convert_try_block_statement(): calling popScopeStack() \n");
#endif
#if 1
       // DQ (9/21/2013): Push the new scope onto the stack.
          popScopeStack();
#endif
        }

     return tryStatement;
   }
