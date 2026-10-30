
#include "edgRose.h"

SgTypedefDeclaration*
EDG_ROSE_Translation::convert_typedef(a_type_ptr p) 
   {
     ROSE_ASSERT(p->source_corresp.name != NULL);

     SgTypedefDeclaration* local_result = NULL;

  // SgName name = p->source_corresp.name;
     SgName name = generateName(p);

#if 0
     printf ("################################### \n");
     printf ("################################### \n");
     printf ("In convert_typedef(): name = %s \n",name.str());
     printf ("################################### \n");
     printf ("################################### \n");
#endif

#if 0
     printf ("In convert_typedef(): name = %s \n",name.str());
     if (type_translation_cache.find(p) != type_translation_cache.end())
        {
          printf ("In convert_typedef(): (top of convert_typedef()) type_translation_cache[p = %p] = %p = %s \n",p,type_translation_cache[p],type_translation_cache[p]->class_name().c_str());
        }
       else
        {
          printf ("In convert_typedef(): (top of convert_typedef()) type_translation_cache.find(p) == type_translation_cache.end() \n");
        }
#endif

     SgType* base_type = NULL;
     if (p->variant.typeref.type->kind == tk_routine)
        {
#if 0
          printf ("In convert_typedef(): Calling convert_function_type() as a typedef of a routine p = %p p->variant.typeref.type = %p \n",p,p->variant.typeref.type);
#endif
          std::tie(base_type, std::ignore) = convert_function_type(p->variant.typeref.type, false /*no undeduced*/, false);
#if 0
          printf ("DONE: In convert_typedef(): Calling convert_function_type() as a typedef of a routine p = %p p->variant.typeref.type = %p \n",p,p->variant.typeref.type);
#endif
          ROSE_ASSERT(base_type != NULL);
#if 0
          printf ("In convert_typedef(): Calling convert_type(): test 2: base_type = %p = %s \n",base_type,base_type->class_name().c_str());
#endif
        }
       else
        {
#if DEBUG_TYPEDEF_GEN
          printf ("In convert_typedef(): Calling convert_type() as a typedef something other than a routine p = %p p->variant.typeref.type = %p \n",p,p->variant.typeref.type);
#endif
#if 0
          printf ("In convert_typedef(): calling summarize_type() on base type \n");
          summarize_type(p->variant.typeref.type);
#endif

          base_type = convert_type(p->variant.typeref.type);
#if 0
          printf ("DONE: In convert_typedef(): Calling convert_type() as a typedef something other than a routine p = %p p->variant.typeref.type = %p \n",p,p->variant.typeref.type);
#endif
          ROSE_ASSERT(base_type != NULL);
#if 0
          printf ("In convert_typedef(): Calling convert_type(): test 3: base_type = %p = %s \n",base_type,base_type->class_name().c_str());
#endif
#if 0
       // DQ (10/14/2013): This is redundant source position information.
       // print_source_corresp(p->source_corresp); debug_print("\n");

          base_type->get_startOfConstruct()->display("In convert_typedef(): startOfConstruct");
          base_type->get_endOfConstruct()  ->display("In convert_typedef(): endOfConstruct");
#endif

       // DQ (10/14/2012): class/struct/union declarations that are output in a typedef should not be nested in declarations later.
          a_type_ptr possibleNestedType = p->variant.typeref.type;
          ROSE_ASSERT(possibleNestedType != NULL);
          bool possibleNestedTypeDeclaration = ( (possibleNestedType->kind == tk_class) || (possibleNestedType->kind == tk_struct) || (possibleNestedType->kind == tk_union) );
          if (possibleNestedTypeDeclaration == true)
             {
#if 0
               printf ("Mark this declaration as output via a typedef so it can't be marked for output later in a variable declaration possibleNestedType = %p \n",possibleNestedType);
#endif
               if (outputTypesSet.find(possibleNestedType) == outputTypesSet.end())
                  {
#if 0
                    printf ("Adding possibleNestedType = %p to outputTypesSet \n",possibleNestedType);
#endif
                    outputTypesSet.insert(possibleNestedType);
                  }
             }
        }

     ROSE_ASSERT(base_type != NULL);

#if 0
     printf ("In convert_typedef(): base_type = %p = %s \n",base_type,base_type->class_name().c_str());

     if (type_translation_cache.find(p) != type_translation_cache.end())
        {
          printf ("In convert_typedef(): (after generation of base type in convert_typedef()) type_translation_cache[p = %p] = %p = %s \n",p,type_translation_cache[p],type_translation_cache[p]->class_name().c_str());

          if (type_translation_cache[p] == base_type)
             {
               printf ("type_translation_cache[p] == base_type: = %p = %s \n",type_translation_cache[p],type_translation_cache[p]->class_name().c_str());
             }
            else
             {
               printf ("In convert_typedef(): type_translation_cache[p] = %p = %s  base_type = %p = %s \n",type_translation_cache[p],type_translation_cache[p]->class_name().c_str(),base_type,base_type->class_name().c_str());
             }
        }
       else
        {
          printf ("In convert_typedef(): (after generation of base type in convert_typedef()) type_translation_cache.find(p) == type_translation_cache.end() \n");
        }
#endif

     ROSE_ASSERT(base_type != NULL);

#if 0
     printf ("Before call to apply_qualifiers(): base_type = %p = %s \n",base_type,base_type->class_name().c_str());
#endif

     a_bit_field qualifiers = p->variant.typeref.qualifiers;
#if UPC_EXTENSIONS_ALLOWED
     ROSE_ASSERT(p->variant.typeref.extra_info != NULL);
     base_type = apply_qualifiers(base_type, qualifiers, p->variant.typeref.extra_info->upc_block_size);
#endif

     ROSE_ASSERT(base_type != NULL);

#if 0
     printf ("After call to apply_qualifiers(): base_type = %p = %s \n",base_type,base_type->class_name().c_str());
#endif

  // DQ (12/27/2019): Need to move these static variables outside the function so that they
  // can be reset after processing each source file.
#if 0
     printf ("MOVE static a_seq_number last_source_sequence_number outside of function so it can be reset after each file! \n");
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

  // DQ (7/17/2012): Add support for multiple typedef to be specified in a typedef declaration.
  // Only the EDG "X.decl_position.seq" can be matched to identify that a second variable 
  // should be associated with a previous typedef declaration.  Se we keep track of the 
  // last declaration sequence number.

  // DQ (12/28/2019): These are now namespace variables so that they can be reset for each file in processing multiple files.
  // static a_seq_number last_source_sequence_number = 0 /* default value */;
  // static SgTypedefDeclaration* last_decl          = NULL;

     bool addingTypedefToExistingTypedefDeclaration = false;

     a_seq_number current_source_sequence_number     = p->source_corresp.decl_position.seq;

#if DEBUG_TYPEDEF_GEN
     printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
     printf ("In convert_typedef(): convert_typedef_last_source_sequence_number = %u \n",convert_typedef_last_source_sequence_number);
     printf ("In convert_typedef(): current_source_sequence_number              = %u \n",current_source_sequence_number);
  // convert_typedef_last_decl = local_result;
     printf ("convert_typedef_last_decl = %p \n",convert_typedef_last_decl);
     if (convert_typedef_last_decl != NULL)
        {
          printf ("convert_typedef_last_decl = %p = %s base_type = %p = %s \n",convert_typedef_last_decl,
               convert_typedef_last_decl->class_name().c_str(),convert_typedef_last_decl->get_base_type(),
               convert_typedef_last_decl->get_base_type()->class_name().c_str());
        }
#endif

     if (current_source_sequence_number == convert_typedef_last_source_sequence_number)
        {
#if DEBUG_TYPEDEF_GEN
          printf ("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");
          printf ("In convert_typedef(): Adding a typedef to a previous typedef declaration \n");
#endif
          addingTypedefToExistingTypedefDeclaration = true;
#if DEBUG_TYPEDEF_GEN
          printf ("In convert_typedef(): part 1: calling buildTypedefDeclaration() \n");
#endif
          local_result = set_file_info_for_corresp(buildTypedefDeclaration_nfi(name, base_type), p->source_corresp);
          ROSE_ASSERT(local_result != NULL);

       // DQ (12/31/2015): This is now not always true (e.g. when we handle multiple variable in a singled declaration using the normalized mode.
       // DQ (12/29/2015): Before supporting multiple variable in the same declaration, I think this is alway true.
       // ROSE_ASSERT(last_decl == NULL);

       // DQ (4/4/2015): We want to have a better test for when we combine types defined in a typedef (see test2015_60.c and test2015_62.c).
          if (convert_typedef_last_decl != NULL)
             {
               ROSE_ASSERT(local_result != NULL);
               SgType* last_decl_base_type    = convert_typedef_last_decl->get_base_type();
               SgType* local_result_base_type = local_result->get_base_type();
               ROSE_ASSERT(last_decl_base_type != NULL);
               ROSE_ASSERT(local_result_base_type != NULL);
#if 0
            // DQ (6/6/2016): Tracing down use of incorrect typedef alias (inconsistant with original source code) \n");
               printf ("In convert_typedef(): Calling stripType() \n");
#endif
            // DQ (4/4/2015): We want to avoid striping typedefs.
            // SgType* stripped_type_last_decl_base_type    = last_decl_base_type->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE|SgType::STRIP_TYPEDEF_TYPE);
            // SgType* stripped_type_local_result_base_type = local_result_base_type->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE|SgType::STRIP_TYPEDEF_TYPE);
               SgType* stripped_type_last_decl_base_type    = last_decl_base_type->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_RVALUE_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE);
               SgType* stripped_type_local_result_base_type = local_result_base_type->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_RVALUE_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE);
               ROSE_ASSERT(stripped_type_last_decl_base_type != NULL);
               ROSE_ASSERT(stripped_type_local_result_base_type != NULL);

               if (stripped_type_last_decl_base_type == stripped_type_local_result_base_type)
                  {
#if 0
                    printf ("Identified the SAME types used in typedef declaration for multiple types \n");
#endif
                    local_result->set_isAssociatedWithDeclarationList(true);
                    convert_typedef_last_decl->set_isAssociatedWithDeclarationList(true);
                  }
                 else
                  {
#if 0
                    printf ("Identified the SEPERATE types used in typedef declaration for multiple types \n");
#endif
                  }
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }

#if 0
       // DQ (4/4/2015): This is now handled by a more complex logic to test if the base types are the same.
#if 0
       // DQ (4/4/2015): Commented this out (see test2015_60.c).
       // DQ (7/28/2012): Mark this as a declaration that is generated to support a declaration list.
          local_result->set_isAssociatedWithDeclarationList(true);
#endif

#if 0
       // DQ (4/4/2015): Commented this out (see test2015_60.c).
       // DQ (8/2/2012): This is now consistant with how variable declarations are handled.
          last_decl->set_isAssociatedWithDeclarationList(true);
#else
       // DQ (7/31/2012): Note that in the case of a variable declaration all of the associated declarations 
       // (including the first, are marked as a part of a declaration list).  I would like to see this be 
       // uniform, but I think that is a problem.  I would like to look into this if I didn't have to 
       // prepare a talk just now.  FIXME!
          printf ("Note that unlike variable declarations, we are not marking the last_decl as isAssociatedWithDeclarationList() (not clear if we should be) \n");
#endif
#endif
        }
       else
        {
          ROSE_ASSERT(base_type != NULL);

#if DEBUG_TYPEDEF_GEN
          printf ("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ \n");
          printf ("In convert_typedef(): part 2: calling buildTypedefDeclaration(): name = %s \n",name.str());
          printf ("   --- base_type                   = %p = %s \n",base_type,base_type->class_name().c_str());
          printf ("   --- p->variant.typeref.is_alias = %s \n",p->variant.typeref.is_alias ? "true" : "false");
#endif

          local_result = set_file_info_for_corresp(buildTypedefDeclaration_nfi(name, base_type), p->source_corresp);
        }

  // DQ (12/28/2019): Find the base declaration that is asociated with the current file (multi-file support).
     SgDeclarationStatement* base_type_declaration = NULL;
     a_type_ptr edg_node =  p->variant.typeref.type;
     ROSE_ASSERT(edg_node != NULL);

     if (type_translation_cache.find(edg_node) != type_translation_cache.end())
        {
          SgNode* rose_node = type_translation_cache[edg_node];
          ROSE_ASSERT(rose_node != NULL);
#if 0
          printf ("rose_node = %p = %s \n",rose_node,rose_node->class_name().c_str());
#endif
#if 0
          if (types_with_emitted_definitions.find(edg_node) != types_with_emitted_definitions.end())
             {
               bool value = types_with_emitted_definitions[edg_node];
               printf ("types_with_emitted_definitions[edg_node] = %s \n",value ? "true" : "false");
             }
#endif

          base_type_declaration = isSgDeclarationStatement(rose_node);
       // ROSE_ASSERT(base_type_declaration != NULL);
#if 0
          if (base_type_declaration != NULL)
             {
               printf ("base_type_declaration = %p = %s \n",base_type_declaration,base_type_declaration->class_name().c_str());
             }
#endif
          if (type_declaration_map.find(edg_node) != type_declaration_map.end())
             {
#if 0
               printf ("Found list of associated declarations for the edg_node (type) so that we select the defining declaration for this file (multi-file support \n");
#endif
               vector<SgDeclarationStatement*> declarationList = type_declaration_map[edg_node];
#if 0
               printf ("declarationList.size() = %zu \n",declarationList.size());
#endif
#if 0
               if (base_type_declaration != NULL)
                  {
                    printf("Note: In convert_typedef(): before being reset: \n");
                    printf (" --- base_type_declaration = %p \n",base_type_declaration);
                    printf (" --- base_type_declaration->get_definingDeclaration() = %p \n",base_type_declaration->get_definingDeclaration());
                  }
#endif
               base_type_declaration = declarationList[0];
               ROSE_ASSERT(base_type_declaration != NULL);
#if 0
            // I think we can assert this.
               if (base_type_declaration != base_type_declaration->get_definingDeclaration())
                  {
                    printf("Note: In convert_typedef(): base_type_declaration != base_type_declaration->get_definingDeclaration() \n");
                    printf (" --- base_type_declaration = %p \n",base_type_declaration);
                    printf (" --- base_type_declaration->get_definingDeclaration() = %p \n",base_type_declaration->get_definingDeclaration());
                  }
#endif
            // ROSE_ASSERT(base_type_declaration == base_type_declaration->get_definingDeclaration());

               SgDeclarationStatement* local_declaration = local_result->get_declaration();
#if 0
               if (local_declaration != NULL)
                  {
                    printf ("Before replacement: local_declaration = %p = %s \n",local_declaration,local_declaration->class_name().c_str());
                    ROSE_ASSERT(local_declaration->get_file_info() != NULL);
                    printf (" --- local_declaration->get_file_info()->get_filenameString() = %s \n",local_declaration->get_file_info()->get_filenameString().c_str());
                  }

               printf ("base_type_declaration = %p \n",base_type_declaration);
               if (base_type_declaration != NULL)
                  {
                    ROSE_ASSERT(base_type_declaration->get_file_info() != NULL);
                    printf ("base_type_declaration->get_file_info()->get_filenameString() = %s \n",base_type_declaration->get_file_info()->get_filenameString().c_str());
                  }
#endif
               bool resetDeclaration = false;
               if (local_declaration != NULL && base_type_declaration != NULL)
                  {
                    if (local_declaration->get_file_info()->get_file_id() >= 0 && base_type_declaration->get_file_info()->get_file_id() >= 0)
                       {
                         if (local_declaration->get_file_info()->get_filenameString() != base_type_declaration->get_file_info()->get_filenameString())
                            {
                              resetDeclaration = true;

                           // DQ (12/30/2019): This is an exception demonstrated by Cxx11_tests/test2016_23.C (avoid this case as a test in debugging).
                              SgTemplateClassDeclaration* templateClassDeclaration = isSgTemplateClassDeclaration(base_type_declaration);
                              if (templateClassDeclaration != NULL)
                                 {
#if 0
                                   printf ("Reset resetDeclaration = false for templateClassDeclaration = %p \n",templateClassDeclaration);
#endif
                                   resetDeclaration = false;
                                 }
                            }
                       }
                  }
#if 0
               printf ("resetDeclaration = %s \n",resetDeclaration ? "true" : "false");
#endif
            // DQ (12/29/2019): Commented out to debug failing case.
            // local_result->set_declaration(base_type_declaration);
               if (resetDeclaration == true)
                  {
#if DEBUG_TYPEDEF_GEN
                    printf ("base_type_declaration = %p = %s \n",base_type_declaration,base_type_declaration->class_name().c_str());
                    SgDeclarationStatement* local_declaration = local_result->get_declaration();
                    if (local_declaration != NULL)
                       {
                         printf ("Calling local_result->set_declaration(base_type_declaration): local_declaration = %p = %s \n",local_declaration,local_declaration->class_name().c_str());
                         ROSE_ASSERT(local_declaration->get_file_info() != NULL);
                         printf (" --- local_declaration->get_file_info()->get_filenameString() = %s \n",local_declaration->get_file_info()->get_filenameString().c_str());
                       }

                    printf ("base_type_declaration = %p \n",base_type_declaration);
                    if (base_type_declaration != NULL)
                       {
                         ROSE_ASSERT(base_type_declaration->get_file_info() != NULL);
                         printf ("base_type_declaration->get_file_info()->get_filenameString() = %s \n",base_type_declaration->get_file_info()->get_filenameString().c_str());
                       }
#endif
                    local_result->set_declaration(base_type_declaration);
                  }
#if 0
               printf ("local_result->get_declaration() = %p \n",local_result->get_declaration());
#endif
#if 0
               printf("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }

#if 0
          printf("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

#if 0
  // DQ (10/14/2013): This is redundant source position information.
     print_source_corresp(p->source_corresp); debug_print("\n");

     local_result->get_startOfConstruct()->display("In convert_typedef(): startOfConstruct");
     local_result->get_endOfConstruct()  ->display("In convert_typedef(): endOfConstruct");
#endif
#if 0
     printf ("Before calling fixup_typedef_declaration() local_result->get_parent() = %p \n",local_result->get_parent());
     printf ("We need to figure out when to set: typedef_stmt->get_typedefBaseTypeContainsDefiningDeclaration() \n");
     printf ("local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

#if DEBUG_TYPEDEF_GEN
     printf ("Calling local_result->set_typedefBaseTypeContainsDefiningDeclaration(false) \n");
#endif

  // This is set by default to "false".
     local_result->set_typedefBaseTypeContainsDefiningDeclaration(false);

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

#if DEBUG_TYPEDEF_GEN
     printf ("Before calling fixup_typedef_declaration(): local_result->get_declaration() = %p \n",local_result->get_declaration());
#endif

#if DEBUG_TYPEDEF_GEN
     printf ("Test #A1 --- local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

     fixup_typedef_declaration(local_result, p->variant.typeref.type);

#if DEBUG_TYPEDEF_GEN
     printf ("Test #A1.5 --- local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

  // SgTypedefType* tt = new SgTypedefType(local_result);
     SgTypedefType* tt = local_result->get_type();
     ROSE_ASSERT(tt != NULL);

     if (tt != local_result->get_type())
        {
          printf ("ERROR: typedef types are different: local_result->get_type() = %p and tt = %p \n",local_result->get_type(),tt);
          ROSE_ASSERT(false);
        }

#if 0
  // DQ (8/12/2012): Modified to avoid overwriting existing type.
  // type_translation_cache[p] = tt;
#else
  // DQ (8/12/2012): Added new code and test.
  // ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());
     if (type_translation_cache.find(p) != type_translation_cache.end())
        {
          ROSE_ASSERT(type_translation_cache[p] != NULL);
#if 0
          printf ("In convert_typedef(): (before reset) type_translation_cache[p = %p] = %p = %s \n",p,type_translation_cache[p],type_translation_cache[p]->class_name().c_str());
#endif
#if 0
       // It is not enough to only set the entry in the map with it is NULL (which actually should not be happening now), 
       // what we have to do is reset it to support later resetting the recorded type references that will be fixed up in 
       // AST post-processing.
          if (type_translation_cache[p] == NULL)
             {
               type_translation_cache[p] = tt;
             }
#else
       // Note: we could look at the SgModifierType and verify that this is an entry in the type_translation_cache that
       // should be reset (we could also use this opportunity to reset the SgModifierType (or even eliminate it in cases).
#if 0
          printf ("In convert_typedef(): RESET: Allow the type_translation_cache[p = %p] = %p = %s to be updated with tt = %p = %s \n",p,type_translation_cache[p],type_translation_cache[p]->class_name().c_str(),tt,tt->class_name().c_str());
#endif
#if 0
          printf ("In convert_typedef(): Adding to type_translation_cache: p = %p tt = %p = %s \n",p,tt,tt->class_name().c_str());
#endif
          type_translation_cache[p] = tt;
#endif
        }
       else
        {
#if 0
          printf ("In convert_typedef(): Adding to type_translation_cache: p = %p tt = %p = %s \n",p,tt,tt->class_name().c_str());
#endif
          ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());
          type_translation_cache[p] = tt;
        }

#if DEBUG_TYPEDEF_GEN
     printf ("Test #A2 --- local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif
#if 0
    printf ("In convert_typedef(): (after reset) type_translation_cache[p = %p] = %p = %s \n",p,type_translation_cache[p],type_translation_cache[p]->class_name().c_str());
#endif

     ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
     ROSE_ASSERT(type_translation_cache[p] != NULL);
#endif

  // DQ (3/3/2012): Added test.
     ROSE_ASSERT(local_result->get_definingDeclaration() == NULL);

  // DQ (3/15/2012): Added test.
     ROSE_ASSERT(local_result->get_firstNondefiningDeclaration() != NULL);

  // DQ (7/17/2012): If there is a second typedef for another typedef name (but from the same typedef declaration) 
  // then don't output the typedef's type definition twice.  When we want to support multiple typedef names in 
  // a single typedef then this will not be required.
     if (addingTypedefToExistingTypedefDeclaration == true && local_result->get_typedefBaseTypeContainsDefiningDeclaration() == true)
        {
#if DEBUG_TYPEDEF_GEN
          printf ("In convert_typedef(): Resetting the following declarations to force typedefBaseTypeContainsDefiningDeclaration == false \n");
#endif

#if 0
       // DQ (8/28/2020): Original code (commented out to fix C_tests/test2020_30.c.
          local_result->set_typedefBaseTypeContainsDefiningDeclaration(false);
          ROSE_ASSERT(local_result->get_typedefBaseTypeContainsDefiningDeclaration() == false);
#else
       // DQ (8/28/2020): When there are two typedefs on the same line it appears that that have the same source sequence number.
#if 0
          printf ("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");
          printf ("Skip resetting the typedefBaseTypeContainsDefiningDeclaration to false \n");
#endif
#endif
          ROSE_ASSERT(local_result->get_declaration() != NULL);
        }

#if DEBUG_TYPEDEF_GEN
     printf ("Test #A3 --- local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

  // DQ (7/17/2012): Update the saved version of the last_source_sequence_number (so that
  // we can detect where multiple typedef names are used in a single typedef declaration).
     convert_typedef_last_source_sequence_number = current_source_sequence_number;
     convert_typedef_last_decl = local_result;

  // DQ (7/17/2012): Don't invalidate the second typedef.  Later when we want to prpoerly 
  // support multiple typedef names in a single typedef declaration then we will want to do this.
     if (addingTypedefToExistingTypedefDeclaration == true)
        {
       // If we are adding a variable to an existing variable declaration, return 
       // NULL so that we will not add the variable declaration to the AST twice.
       // local_result = NULL;
        }

  // DQ (8/12/2012): Added test.
     ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());

#if DEBUG_TYPEDEF_GEN || 0
     printf ("Leaving convert_typedef(): type_translation_cache[p = %p] = %p = %s \n",p,type_translation_cache[p],type_translation_cache[p]->class_name().c_str());
     printf (" --- local_result = %p local_result->get_declaration() = %p \n",local_result,local_result->get_declaration());
     printf (" --- local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

#if DEBUG_TYPEDEF_GEN
     printf ("Test #1 --- local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

#if 0
// Dan and Liao 12/3/2012     
// We now enforce the parent to be set for base type's declaration
// The API has changed (last bool to indicate if typedef has a defining base type)
    SgType* b_type = local_result->get_base_type();
    if (b_type != NULL)
    {
      SgDeclarationStatement* b_decl = b_type->getAssociatedDeclaration ();
      if (b_decl != NULL)
      {
        SgDeclarationStatement* b_def_decl = b_decl->get_definingDeclaration();
        SgDeclarationStatement* b_nondef_decl = b_decl->get_firstNondefiningDeclaration();
        if (local_result->get_typedefBaseTypeContainsDefiningDeclaration())
        {
          ROSE_ASSERT (b_def_decl != NULL);
          ROSE_ASSERT (b_def_decl ->get_parent() != NULL);
        }
        else
        {
          ROSE_ASSERT (b_nondef_decl != NULL);
          ROSE_ASSERT (b_nondef_decl ->get_parent() != NULL);
        }
      }
    }
#endif

  // DQ (10/26/2015): Added test to detect simple cycle in types.
     ROSE_ASSERT(base_type != local_result->get_type());

#if DEBUG_TYPEDEF_GEN
     printf ("Test #2 --- local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

#if 0
  // SgTypedefType*  tmp_typedefType = isSgTypedefType(base_type);
     SgTypedefType*  tmp_typedefType = isSgTypedefType(local_result->get_type());
     ROSE_ASSERT(tmp_typedefType != NULL);

  // DQ (10/26/2015): Added test to detect simple cycle in types.
     SgModifierType* tmp_modifierType = isSgModifierType(tmp_typedefType->get_base_type());
     if (tmp_modifierType != NULL)
        {
          ROSE_ASSERT(tmp_typedefType != tmp_modifierType->get_base_type());
        }
#endif

#if DEBUG_TYPEDEF_GEN
     printf ("Test #3 --- local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

  // DQ (10/27/2015): Added test to detect more complex cycle in types.
     SgType* currentType = local_result->get_type();
     ROSE_ASSERT(currentType != NULL);

     if ( (isSgTypedefType(currentType) != NULL) &&
          (isSgModifierType(isSgTypedefType(currentType)->get_base_type()) != NULL) && 
          (isSgModifierType(isSgTypedefType(currentType)->get_base_type())->get_base_type() == currentType) )
        {
          printf ("Error: detected recusive typedef declaration \n");
          printf ("   --- typedef declaration = %p = %s \n",local_result,local_result->class_name().c_str());

          SgTypedefType*  typedefType  = isSgTypedefType(currentType);
          if (typedefType == NULL)
             {
               printf ("currentType not a SgTypedefType: currentType = %p = %s \n",currentType,currentType->class_name().c_str());
             }
          ROSE_ASSERT(typedefType != NULL);
#if DEBUG_TYPEDEF_GEN
          printf ("Test #4 --- local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif
          SgModifierType* modifierType = isSgModifierType(typedefType->get_base_type());
          if (modifierType == NULL)
             {
               printf ("typedefType->get_base_type() not a SgModifierType: typedefType->get_base_type() = %p = %s \n",typedefType->get_base_type(),typedefType->get_base_type()->class_name().c_str());
             }
          ROSE_ASSERT(modifierType != NULL);

          printf ("   --- typedef declaration type = %p = %s \n",typedefType,typedefType->class_name().c_str());
          printf ("   --- typedef declaration type->base_type = %p = %s \n",modifierType,modifierType->class_name().c_str());
        }

#if DEBUG_TYPEDEF_GEN
     printf ("Test #5 --- local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

     ROSE_ASSERT(isSgTypedefType(currentType) != NULL);
     ROSE_ASSERT( (isSgModifierType(isSgTypedefType(currentType)->get_base_type()) == NULL) || 
                  ( (isSgModifierType(isSgTypedefType(currentType)->get_base_type()) != NULL) && 
                    (isSgModifierType(isSgTypedefType(currentType)->get_base_type())->get_base_type() != currentType) ) );

#if DEBUG_TYPEDEF_GEN || 0
     printf ("Leaving convert_typedef(): local_result = %p = %s \n",local_result,local_result != NULL ? local_result->class_name().c_str() : "null");
     printf (" --- local_result = %p local_result->get_declaration() = %p \n",local_result,local_result->get_declaration());
     printf (" --- local_result->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",local_result->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

  // Note that this will sometimes be a NULL pointer (e.g. test2001_24.C)
     return local_result;
   }


SgDeclarationStatement*
EDG_ROSE_Translation::parse_type_declaration(SourceSequenceList& sse)
   {
     SgDeclarationStatement* result = NULL;

  // DQ (2/9/2013): The switch support requires a map of switch statement body entries (typically statements, sometimes declarations).
  // This map thus can't be a map between EDG a_statement_ptr and ROSE SgStatements, it must be something more gneral in EDG.
  // So we are using EDG's a_source_sequence_entry_ptr type for the map, and so we have to save this value to fill in the map 
  // at the base of this function.
     a_source_sequence_entry_ptr parse_type_declaration_function_source_sequence_entry = NULL;

     a_type_ptr p = (a_type_ptr)(sse->entity.ptr);

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("%stype declaration p = %p ", (p == NULL ? "null " : ""),p);
     if (p != NULL) { print_source_corresp(p->source_corresp); debug_print("\n"); }
#endif

  // DQ (6/9/2013): I think I can assert this.
     ROSE_ASSERT(p != NULL);
  // DQ (4/23/2016): Testing new suport for Sawer assertions.
  // ROSE_ASSERT(p == NULL);

     if (p == NULL)
        {
          return NULL;
        }

#ifdef DEBUG_CONVERSION
     ScopedIndent si(indent);
#endif

  // SgName name = p->source_corresp.name == NULL ? generate_name_for_type(p) : p->source_corresp.name;
  // printf ("In parse_type_declaration(): Calling generateName(p) \n");
     SgName name = generateName(p);
  // printf ("DONE: In parse_type_declaration(): Calling generateName(p) \n");

  // DQ (2/10/2013): Set the parse_statement_function_source_sequence_entry so that we can setup the switch support map if it is required (if we are in a switch).
     ROSE_ASSERT(sse->entity.ptr != NULL);
     parse_type_declaration_function_source_sequence_entry = ((a_type_ptr)(sse->entity.ptr))->source_corresp.source_sequence_entry;
     ROSE_ASSERT(parse_type_declaration_function_source_sequence_entry != NULL);

     a_type_kind kind = p->kind;

#if DEBUG_STRUCTURAL_DESIGN || 0
     printf ("************************************************************************************ \n");
     printf ("In parse_type_declaration(): summarize_type_kind_in_string_kind(kind = %d) = %s \n",kind,summarize_type_kind_in_string_kind(kind));
     printf ("************************************************************************************ \n");
#endif
#if 0
     printf ("In parse_type_declaration(): Calling summarize_type() \n");
     summarize_type(p);
#endif

#ifdef DEBUG_CONVERSION
     printf ("TOP of parse_type_declaration(): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

     switch (kind)
        {
          case tk_integer:
             {
               ROSE_ASSERT (p->variant.integer.enum_type == true);
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("enum\n");
#endif
#if 0
               printf ("##### In parse_type_declaration(): Building a defining EnumDeclaration: name = %s \n",name.str());
#endif
            // DQ (5/14/2018): Compute the scope seperately.
            // SgEnumDeclaration* decl = set_file_info_for_corresp(buildEnumDeclaration_nfi(name, topScopeStack()), p->source_corresp);
               SgScopeStatement* declarationScope = topScopeStack();
               ROSE_ASSERT(declarationScope != NULL);
#if 0
               printf ("declarationScope for enum (from topScopeStack()): declarationScope = %p = %s \n",declarationScope,declarationScope->class_name().c_str());
#endif
               a_scope_ptr edg_scope = p->source_corresp.parent_scope;

            // ROSE_ASSERT(edg_scope != NULL);
               if (edg_scope != NULL)
                  {
                    if (scope_cache.find(edg_scope) != scope_cache.end())
                       {
                         declarationScope = scope_cache[edg_scope];
                       }
                      else
                       {
                         printf ("ERROR: the associated scope has not been processed: edg_scope = %p \n",edg_scope);
                         ROSE_ASSERT(false);
                       }
                  }
                 else
                  {
#if 0
                    printf ("In parse_type_declaration(): case tk_integer: p->source_corresp.parent_scope == NULL \n");
#endif
                  }

#if 0
               printf ("declarationScope for enum: declarationScope = %p = %s \n",declarationScope,declarationScope->class_name().c_str());
#endif
               SgEnumDeclaration* decl = set_file_info_for_corresp(buildEnumDeclaration_nfi(name, declarationScope), p->source_corresp);
#if 0
               printf ("##### In parse_type_declaration(): DONE: Building a defining EnumDeclaration: decl = %p p->autonomous_primary_tag_decl = %s \n",decl,p->autonomous_primary_tag_decl ? "true" : "false");
#endif
               if (p->autonomous_primary_tag_decl == false)
                  {
#if 0
                 // DQ (4/3/2017): But to support this we need to build another nondefining declaration that is set to be marked 
                 // as the one used in the declaration containing the nested construct.
                    SgEnumDeclaration* nondefining_decl = set_file_info_for_corresp(buildNondefiningEnumDeclaration_nfi(name, topScopeStack()), p->source_corresp);
#if 0
                    printf ("##### In parse_type_declaration(): Building a nondefining EnumDeclaration: nondefining_decl = %p \n",nondefining_decl);
#endif
                 // This is the case of an enum declaration nested in another (e.g. typedef, variable declaration, etc.) declaration.
                    nondefining_decl->set_isAutonomousDeclaration(false);

                    if (is_unnamed(p))
                       {
#if 0
                         printf ("In parse_type_declaration(): Mark enum declaration = %p = %s as UN-NAMED \n",decl,decl->class_name().c_str());
#endif
                         nondefining_decl->set_isUnNamed(true);
                       }

                 // DQ (7/31/2012): Set the non-defining declaration
                    if (nondefining_decl->get_firstNondefiningDeclaration() == NULL)
                       {
                         printf ("nondefining_decl->get_firstNondefiningDeclaration() not yet! \n");
                         ROSE_ASSERT(false);
                       }

                 // DQ (7/31/2012): Set the defining declaration
                    if (nondefining_decl->get_definingDeclaration() == NULL)
                       {
                         printf ("nondefining_decl->get_definingDeclaration() not yet! \n");
                         ROSE_ASSERT(false);
                       }


                 // But set the defining declaration to true.
                    decl->set_isAutonomousDeclaration(true);
#else
                 // This is the case of an enum declaration nested in another (e.g. typedef, variable declaration, etc.) declaration.
                 // DQ (4/3/2017): However, since we need it to be output in a normalized form we can't mark it as autonomous.

                 // DQ (4/3/2017): Original code.
                    decl->set_isAutonomousDeclaration(false);
                 // decl->set_isAutonomousDeclaration(true);
#endif
                 // DQ (7/12/2012): Added test (this will be set later, for example when we see the variable declaration that it might be a part of).
                    ROSE_ASSERT(decl->get_parent() == NULL);
                  }
                 else
                  {
                 // This is a simple (typical) declaration (referenced directly in a scope) used to define a type.
                    decl->set_isAutonomousDeclaration(true);

                 // DQ (7/12/2012): Added test.
                    if (decl->get_parent() == NULL)
                       {
#if 0
                         printf ("Setting the SgEnumDeclaration parent to the current scope \n");
#endif
                         decl->set_parent(topScopeStack());
                       }
                    ROSE_ASSERT(decl->get_parent() != NULL);
                  }
#if 0
               printf ("In parse_type_declaration(): p->autonomous_primary_tag_decl = %s \n",p->autonomous_primary_tag_decl ? "true" : "false");
               printf ("   --- decl->get_isAutonomousDeclaration() = %s \n",decl->get_isAutonomousDeclaration() ? "true" : "false");
#endif
#if 0
               printf ("In parse_type_declaration(): case tk_integer: calling summarize_template() \n");
               summarize_type(p);
#endif
#if 0
               decl->get_file_info()->display("parse_type_declaration(): case tk_integer (enum declaration)");
#endif

#if 0
               SgGlobal* globalScope = isSgGlobal(decl->get_scope());
               if (globalScope != NULL)
               {
               }
#endif
               if (is_unnamed(p))
                  {
#if 0
                    printf ("In parse_type_declaration(): Mark enum declaration = %p = %s as UN-NAMED \n",decl,decl->class_name().c_str());
#endif
                    decl->set_isUnNamed(true);

                 // DQ (7/31/2012): Set the non-defining declaration
                    if (decl->get_firstNondefiningDeclaration() != NULL)
                       {
                         isSgEnumDeclaration(decl->get_firstNondefiningDeclaration())->set_isUnNamed(true);
                       }

                 // DQ (7/31/2012): Set the defining declaration
                    if (decl->get_definingDeclaration() != NULL)
                       {
                         isSgEnumDeclaration(decl->get_definingDeclaration())->set_isUnNamed(true);
                       }
                  }

               decl->set_definingDeclaration(decl);
               set_file_info_for_corresp(decl->get_firstNondefiningDeclaration(), p->source_corresp);

            // DQ (8/12/2012): Added test.
               ROSE_ASSERT(decl->get_type() != NULL);
#if 0
            // DQ (8/12/2012): Added test.
               ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());

               type_translation_cache[p] = decl->get_type();
#else
            // This codes accounts for type_translation_cache[p] already having been set previously.
               if (type_translation_cache.find(p) == type_translation_cache.end())
                  {
#if 0
                    printf ("In parse_type_declaration(): Adding to type_translation_cache: p = %p decl->get_type() = %p = %s \n",p,decl->get_type(),decl->get_type()->class_name().c_str());
#endif
                 // If not set then set it.
                    type_translation_cache[p] = decl->get_type();
#if 0
                    bool isInCache = isInTypeDeclarationMapAsDefiningDeclaration(p);
                    printf ("BEFORE push_entry_to_type_declaration_map(): isInCache = %s \n",isInCache ? "true" : "false");
#endif
#if 0
                    printf ("Pushing decl = %p to type_declaration_map using p = %p \n",decl,p);
#endif
                 // DQ (12/28/2019): Save the declaration from the current file, so that we don't retrieve it from the shared type.
                    push_entry_to_type_declaration_map(p,decl);
#if 0
                    isInCache = isInTypeDeclarationMapAsDefiningDeclaration(p);
                    printf ("AFTER push_entry_to_type_declaration_map(): isInCache = %s \n",isInCache ? "true" : "false");
#endif
                  }
                 else
                  {
                 // If previously set then make sure it was set to the same value.
                    if (type_translation_cache[p] != decl->get_type())
                       {
#ifdef DEBUG_CONVERSION
                         printf ("WARNING: resetting type_translation_cache[p] (previously set to different type value) \n");
                         printf ("   --- decl->get_type() = %p = %s \n",decl->get_type(),decl->get_type()->class_name().c_str());
                         printf ("   --- type_translation_cache[p = %p] = %p = %s \n",p,type_translation_cache[p],type_translation_cache[p]->class_name().c_str());
#endif
#if 0
                         printf ("In parse_type_declaration(): type_translation_cache[p] != decl->get_type(): Adding to type_translation_cache: p = %p decl->get_type() = %p = %s \n",
                              p,decl->get_type(),decl->get_type()->class_name().c_str());
#endif
                         type_translation_cache[p] = decl->get_type();
                       }
                    ROSE_ASSERT(type_translation_cache[p] == decl->get_type());
                  }
               ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
#endif

            // DQ (8/12/2012): Added test.
               ROSE_ASSERT(type_translation_cache[p] != NULL);

               ++sse;

            // Set this similar as we do for class/struct/union, to make sure no self-reference can lead to recursive defining declarations being generated.
               if (!is_unnamed(p))
                  {
#if 0
                    printf ("Setting types_with_emitted_definitions[p = %p] = true \n",p);
#endif
                    types_with_emitted_definitions[p] = true; // Creating a type definition for p
                  }
#if 0
               printf ("In parse_type_declaration(): Calling parse_enumerators() \n");
#endif
            // Parse enumerators
               parse_enumerators(sse, decl, p);
#if 0
               printf ("In parse_type_declaration(): DONE: Calling parse_enumerators() \n");
#endif
            // DQ (10/21/2012): Similarly to class/struct/union, mark this has not yet output, so that any typedef 
            // or variable can be allowed to have it as part of their embedded declaration.
            // DQ (1/11/2009): With the fix below, also put this in the list.
            // if (!is_unnamed(p))
            //      types_with_emitted_definitions[p] = true; // Creating a type definition for p
            // types_with_emitted_definitions[p] = true; // Creating a type definition for p
               if (!is_unnamed(p))
                  {
                    types_with_emitted_definitions[p] = false; // Creating a type definition for p
                  }
#if 0
               printf ("In parse_type_declaration(): is_unnamed(p)                  = %s \n",(is_unnamed(p) == true) ? "true" : "false");
               printf ("In parse_type_declaration(): p->autonomous_primary_tag_decl = %s \n",p->autonomous_primary_tag_decl ? "true" : "false");
               printf ("In parse_type_declaration(): types_with_emitted_definitions[p = %p] = %s \n",p,types_with_emitted_definitions[p] ? "true" : "false");
#endif
#if 0
            // DQ (7/12/2012): This is set above.
               if (p->autonomous_primary_tag_decl == false)
                  {
                 // This is the case of an enum declaration nested in another (e.g. typedef) declaration.
                    decl->set_isAutonomousDeclaration(false);

                 // DQ (7/12/2012): Added test.
                    ROSE_ASSERT(decl->get_parent() == NULL);
                  }
                 else
                  {
                 // This is a simple (typical) declaration (referenced directly in a scope) used to define a type.
                    decl->set_isAutonomousDeclaration(true);

                 // DQ (7/12/2012): Added test.
                    ROSE_ASSERT(decl->get_parent() != NULL);
                  }
#endif

            // DQ (8/12/2014): Adding C++11 support for enum base type.
               if (p->variant.integer.extra_info != NULL)
                  {
                 // if (p->variant.integer.extra_info->has_explicit_enum_base == true)
                    if (p->variant.integer.extra_info->base_type != NULL)
                       {
#if 0
                      // DQ (3/5/2017): Converted to use message logging.
                         mprintf ("Detected use of enum field base type specification! \n");
#endif
                         ROSE_ASSERT(p->variant.integer.has_explicit_enum_base == true);

                      // DQ (8/12/2014): I have not yet seend this case.
                         ROSE_ASSERT(p->variant.integer.explicitly_signed == false);

                         SgType* enum_type = convert_type(p->variant.integer.extra_info->base_type);
                         ROSE_ASSERT(enum_type != NULL);

                         decl->set_field_type(enum_type);
#if 0
                         printf ("Exiting as a test (specification of base type for enum fields) \n");
                         ROSE_ASSERT(false);
#endif
                       }
                  }

            // DQ (8/12/2014): Adding C++11 support for scoped enum declarations.
               if (p->variant.integer.is_scoped_enum == true)
                  {
                 // This is the case of a C++11 "enum class" construct.

                 // DQ (1/28/2017): Currently ROSE computes the scope of the enum values incorrectly in this 
                 // case (this will have to be fixed later).  The unparsed value is also an explicit cast of
                 // the enum integer value instead of the qulaified version of the enum name.  It is not clear 
                 // if EDG saves the name of the enum value so this can be furtehr investigated later to improve 
                 // the source-to-source behavior.

                    decl->set_isScopedEnum(true);
#if 0
                    printf ("enum declaration set as scoped enum (C++11 specific feature): decl = %p \n",decl);
                    printf ("   --- non-defining declaration: decl->get_firstNondefiningDeclaration() = %p \n",decl->get_firstNondefiningDeclaration());
#endif
                    SgEnumDeclaration* nondefiningEnumDeclaration = isSgEnumDeclaration(decl->get_firstNondefiningDeclaration());
                    ROSE_ASSERT(nondefiningEnumDeclaration != NULL);

                 // DQ (1/28/2017): Set the value to be consistant with the defining declaration.
                    nondefiningEnumDeclaration->set_isScopedEnum(true);

                    ROSE_ASSERT(nondefiningEnumDeclaration->get_isScopedEnum() == decl->get_isScopedEnum());
#if 0
                 // DQ (5/14/2018): I think the fix might be to just push the a scope for the enum declaration onto the scope stack.
                    SgScopeStatement* declarationScope = topScopeStack();
                    printf ("C++11 enum class scope: topScopeStack() = %p = %s \n",declarationScope,declarationScope->class_name().c_str());
#endif
                 // DQ (5/14/2018): Test if the internal_intermediate_scope has been used to accumulate enum value symbols for the
                 // C++11 enum class declaration.  Is so use the SgDeclarationScope and build a new one for the next function declaration.
                    if (internal_intermediate_scope != NULL)
                       {
                         if (internal_intermediate_scope->get_symbol_table()->size() > 0)
                            {
                           // Need to build a new SgDeclarationScope so this will not be absorbed into the next function declaration.

                           // Use the scope that is setup for all of the SgDeclarationScope IR nodes.
                              SgScopeStatement* originalScope = isSgScopeStatement(internal_intermediate_scope->get_parent());
                              ROSE_ASSERT(originalScope != NULL);
                              Sg_File_Info *startOfConstruct = new Sg_File_Info (originalScope->get_startOfConstruct()->get_filenameString());
                              ROSE_ASSERT(startOfConstruct != NULL);

                              SgEnumDeclaration* definingEnumDeclaration = isSgEnumDeclaration(decl->get_definingDeclaration());
                              ROSE_ASSERT(definingEnumDeclaration != NULL);

                              internal_intermediate_scope->set_parent(definingEnumDeclaration);
                              ROSE_ASSERT(internal_intermediate_scope->get_parent() != NULL);
                              decl->set_declarationScope(internal_intermediate_scope);

                              internal_intermediate_scope = NULL;

                              internal_intermediate_scope = new SgDeclarationScope(startOfConstruct);
                              ROSE_ASSERT(internal_intermediate_scope != NULL);

                           // We need this to have a valid parent, and global scope might be all that is available.
                           // Or maybe we need to fixup the get_scope() function to detect the case of a SgDeclarationScope and return NULL (instead of an error).
                           // internal_intermediate_scope->set_parent(originalScope);
                           // internal_intermediate_scope->set_parent(decl);

                           // Set the new SgDeclarationScope pointed to by internal_intermediate_scope.
                              internal_intermediate_scope->set_parent(originalScope);
                              ROSE_ASSERT(internal_intermediate_scope->get_parent() != NULL);
                            }
                       }
                  }
               result = decl;
               break;
             }

          case tk_typeref:
             {
               ++sse;
#if GNU_EXTENSIONS_ALLOWED
               // TODO EDG 6.5 defines that in il_def.h at line 9204
               bool is_typeof = p->variant.typeref.kind == trk_is_typeof_with_expression || p->variant.typeref.kind == trk_is_typeof_with_type_operand;
               if (is_typeof)
                  {
                    result = NULL;
                    a_statement_ptr edg_statement = (a_statement_ptr)(sse->entity.ptr);
                    if (p->variant.typeref.kind == trk_is_typeof_with_expression)
                       {
                         if (statement_scope_cache.find(edg_statement) != statement_scope_cache.end())
                            {
                              SgScopeStatement* scope = statement_scope_cache[edg_statement];
                              ROSE_ASSERT(scope != NULL);
                              SgBasicBlock* block = isSgBasicBlock(scope);
                              ROSE_ASSERT(block != NULL);
                              SgStatementExpression* statementExpression = isSgStatementExpression(block->get_parent());
                              ROSE_ASSERT(statementExpression != NULL);
                              if (statementExpression != NULL)
                                 {
                                   result = NULL;
                                   SgType* result_type = SageBuilder::buildTypeOfType(statementExpression,NULL);
                                   ROSE_ASSERT(result_type != NULL);

                                   ROSE_ASSERT(statementExpression->get_parent() != NULL);
                                 }
                            }
                           else
                            {
                              SgBasicBlock* existingBasicBlock = NULL;
                              std::vector<SgStatement*> statement_list = parse_statement(sse,existingBasicBlock);
                              if (statement_scope_cache.find(edg_statement) != statement_scope_cache.end())
                                 {
                                   SgScopeStatement* scope = statement_scope_cache[edg_statement];
                                   ROSE_ASSERT(scope != NULL);
                                   SgBasicBlock* block = isSgBasicBlock(scope);
                                   ROSE_ASSERT(block != NULL);
                                   SgStatementExpression* statementExpression = new SgStatementExpression(block);
                                   ROSE_ASSERT(statementExpression != NULL);
                                   setSourcePosition(statementExpression);
                                   statementExpression->get_statement()->set_parent(statementExpression);

                                   statement_expression_bodies.insert(edg_statement);
                                   SgType* result_type = SageBuilder::buildTypeOfType(statementExpression,NULL);
                                   ROSE_ASSERT(result_type != NULL);
                                // Use the EDG typeref node as the key.
                                   if (edg_type_to_rose_typeof_type_map.find(p) == edg_type_to_rose_typeof_type_map.end())
                                      {
                                     // DQ (3/6/2017): Added support for message logging to control output from ROSE tools.
                                        mprintf ("Put the SgTypeOfType into the edg_type_to_rose_typeof_type_map: key = %p result_type = %p \n",p,result_type);

                                        SgTypeOfType* result_typeOfType = isSgTypeOfType(result_type);
                                        ROSE_ASSERT(result_typeOfType != NULL);
                                        edg_type_to_rose_typeof_type_map[p] = result_typeOfType;
                                      }
                                     else
                                      {
                                        printf ("ERROR: we should have reused an existing typeof operator! \n");
                                        ROSE_ASSERT(false);
                                      }
                                   ROSE_ASSERT(statementExpression->get_parent() != NULL);
                                 }
                               ++sse;
                            }
                       }
                      else
                       {
                      // If we have a type operand then we can use it.
                         printf ("ERROR: This type operand to the typeof operator should have been processed already! \n");
                         ROSE_ASSERT(false);
                       }
                    SgBasicBlock* existingBasicBlock = NULL;
                    std::vector<SgStatement*> statement_list = parse_statement(sse,existingBasicBlock);
                    if (statement_list.size() == 1)
                       {
                         ROSE_ASSERT(statement_list[0] != NULL);
                         result = isSgDeclarationStatement(statement_list[0]);
                         ROSE_ASSERT(result != NULL);
                       }
                      else
                       {
                         ROSE_ASSERT(statement_list.size() == 0);
                       }
                  }
                 else
#endif
                  {
                    ROSE_ASSERT(p->source_corresp.name != NULL);
                    SgTypedefDeclaration* typedefDeclaration = convert_typedef(p);
                    ROSE_ASSERT(typedefDeclaration != NULL);
                    result = typedefDeclaration;
                    ROSE_ASSERT(typedefDeclaration->get_base_type() != NULL);
                    SgNamedType * namedType = isSgNamedType(typedefDeclaration->get_base_type());
                    if (namedType != NULL)
                       {
                         SgDeclarationStatement* associatedDeclaration = namedType->get_declaration();
                         ROSE_ASSERT(associatedDeclaration != NULL);
                         ROSE_ASSERT(associatedDeclaration->get_firstNondefiningDeclaration() != NULL);
                       }

                 // DQ (2/28/2012): The defining declaration is always NULL for a typedef (I think the firstNondefining declaration is the only version allowed).
                    ROSE_ASSERT(typedefDeclaration->get_definingDeclaration() == NULL);
                    ROSE_ASSERT(typedefDeclaration->get_firstNondefiningDeclaration() != NULL);
                    SgScopeStatement* currentScope = topScopeStack();
                    ROSE_ASSERT(currentScope != NULL);
                    typedefDeclaration->set_parent(currentScope);
                    typedefDeclaration->get_firstNondefiningDeclaration()->set_parent(currentScope);

                    ROSE_ASSERT(typedefDeclaration->get_parent() != NULL);

                 // DQ (8/12/2012): Added tests.
                    ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
                    ROSE_ASSERT(type_translation_cache[p] != NULL);
                  }
               break;
             }

          case tk_class:  
             {
#if 0
               printf ("In parse_type_declaration(): case tk_class: calling parse_structlike(p = %p, SgClassDeclaration::e_class) \n",p);
#endif
               result = parse_structlike(sse, p, SgClassDeclaration::e_class);
#if 0
               printf ("DONE: In parse_type_declaration(): case tk_class: DONE calling parse_structlike(p = %p, SgClassDeclaration::e_class) result = %p \n",p,result);
#endif
               break;
             }

          case tk_struct:
             {
#if 0
               printf ("In parse_type_declaration(): case tk_struct: calling parse_structlike(p = %p, SgClassDeclaration::e_struct) \n",p);
#endif
#if 0
               printf ("In parse_type_declaration(): tk_struct: Calling summarize_type() \n");
               summarize_type(p);
#endif
            // DQ (6/9/2013): I can't see how to determine when to generate the primary declaration 
            // and when we just want the secondary declaration.  This is an issue for test2013_198.C, 
            // I have had to build a test in the SageInterface::appendStatement to make sure the 
            // declaration can never be inserted twice into the same scope.
               result = parse_structlike(sse, p, SgClassDeclaration::e_struct);

// #ifdef DEBUG_CONVERSION
#if (defined(DEBUG_CONVERSION) && 1)
               printf ("In parse_type_declaration(): case tk_struct: isInTypeTranslationCache(p = %p) = %s \n",p,isInTypeTranslationCache(p) ? "true" : "false");
               printf ("In parse_type_declaration(): case tk_struct: isInTypeDeclarationMap(p = %p)   = %s \n",p,isInTypeDeclarationMap(p) ? "true" : "false");
#endif
#if 0
               printf ("DONE: In parse_type_declaration(): case tk_struct: DONE calling parse_structlike(p = %p, SgClassDeclaration::e_struct) result = %p \n",p,result);
#endif
               break;
             }

          case tk_union:
             {
#if 0
               printf ("In parse_type_declaration(): case tk_union: calling parse_structlike(p = %p, SgClassDeclaration::e_union) \n",p);
#endif
               result = parse_structlike(sse, p, SgClassDeclaration::e_union);
#if 0
               printf ("DONE: In parse_type_declaration(): case tk_union: DONE calling parse_structlike(p = %p, SgClassDeclaration::e_union) result = %p \n",p,result);
#endif
               break;
             }

           default:
             {
               printf ("Case not handled! kind = %s \n",output_type_enum(kind).c_str());
               ROSE_ASSERT(false);
               break;
             }
        }

  // DQ (4/26/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

// #if GNU_EXTENSIONS_ALLOWED
  // DQ (8/16/2014): These functions are now called as part of an API supporting both GNU and Microsoft attrivutes.
  // DQ (1/3/2009): Setup the GNU attributes if present in the source code
     if (result != NULL)
        {
       // This should be called for enum declarations, typedef declarations and class declarations (including unions and structs).
#if 0
          printf ("##### In parse_type_declaration(): Calling set_type_attributes() \n");
#endif
          set_type_attributes(p,result);
#if 0
          printf ("##### DONE: In parse_type_declaration(): Calling set_type_attributes() \n");
#endif
        }
// #endif

#if 0
  // DQ (1/11/2009): I think we can enforce this now that case tk_integer should return a valid declaration.
     printf ("In parse_type_declaration(): result = %p \n",result);
  // ROSE_ASSERT(result != NULL);

  // DQ (2/27/2012): Tracing down the problem with parent pointers noticed by Robb.
     if (result != NULL)
        {
          printf ("In parse_type_declaration(): result->get_parent() = %p \n",result->get_parent());
          if (result->get_parent() != NULL)
             {
               printf ("In parse_type_declaration(): result->get_parent() (class name) = %s \n",result->get_parent()->class_name().c_str());

            // DQ (2/27/2012): We want to set this later when we see how it is used.
            // result->set_parent(NULL);
             }
        }
#endif

  // DQ (3/29/2015): Test this only if we processed the statment (when result != NULL, see test2015_51.c, processing of typeof type declaration).
     if (result != NULL)
        {
       // DQ (8/12/2012): Added tests.
       // ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
       // ROSE_ASSERT(type_translation_cache[p] != NULL);
          ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end() || template_type_translation_cache.find(p) != template_type_translation_cache.end());
       // ROSE_ASSERT(type_translation_cache[p] != NULL);
          ROSE_ASSERT(type_translation_cache.find(p)          == type_translation_cache.end()          || type_translation_cache[p] != NULL);
          ROSE_ASSERT(template_type_translation_cache.find(p) == template_type_translation_cache.end() || template_type_translation_cache[p] != NULL);
        }

  // DQ (2/10/2013): Adding support for switch statement processing (see test2013_61.C).
  // I think we might not need this since the parse_statement function appears to pick up the case of a SgTypedefStatement (at least).
     if (result != NULL)
        {
          updateSwitchSupportMap(parse_type_declaration_function_source_sequence_entry,result);

       // DQ (2/16/2013): Added support for access modifiers.
          unsigned int access_level = p->source_corresp.access;
          set_access_modifier(result,access_level);
#if DEBUG_STRUCTURAL_DESIGN || 0
          printf ("************************************************************************************ \n");
          printf ("Leaving parse_type_declaration(): result = %p = %s: access = %s \n",result,result->class_name().c_str(),result->get_declarationModifier().get_accessModifier().displayString().c_str());
          printf ("************************************************************************************ \n");
#endif
#if 0
          printf ("Source position for defining non-template class declaration: \n");
          result->get_startOfConstruct()->display("Source position for defining non-template class declaration: START: debug");
          result->get_endOfConstruct  ()->display("Source position for defining non-template class declaration: END: debug");
#endif
#if 0
       // DQ (4/23/2017): Debugging Microsoft extensions.
          if (result->get_parent() == NULL)
             {
            // I think it OK to be NULL at this point when it is nested in a typedef, the typedef has to be 
            // built (next) and then the parent for the nested type (e.g. enum) can be set.
               printf ("WARNING: parent of result not set! (may be OK!) \n");
            // ROSE_ASSERT(false);
             }
#endif
        }
       else
        {
#if DEBUG_STRUCTURAL_DESIGN || 0
          printf ("************************************************************************************ \n");
          printf ("Leaving parse_type_declaration(): result = NULL \n");
          printf ("************************************************************************************ \n");
#endif
        }

#if 0
  // DQ (7/7/2019): Note that for C langauge support all types are in global scope (same for functions).
  // DQ (7/5/2019): Added error checking.
     if (result != NULL && is_unnamed(p) == true)
        {
       // Note that un-named constructs can't be name qualified, so there scopes should always match that of the computed topScopeStack().
#if 0
          printf ("In parse_type_declaration(): If this is un-named then the scope should match the topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif
          SgScopeStatement* resultScope          = result->get_scope();
          SgScopeStatement* topOfScopeStackScope = topScopeStack();
          if (result != NULL && resultScope != topOfScopeStackScope)
             {
            // printf ("Error: for un-named construct, resultScope != topOfScopeStackScope \n");
               printf ("Warning: for un-named construct = %p = %s, resultScope != topOfScopeStackScope \n",result,result->class_name().c_str());
               printf (" --- resultScope          = %p = %s \n",resultScope,resultScope->class_name().c_str());
               printf (" --- topOfScopeStackScope = %p = %s \n",topOfScopeStackScope,topOfScopeStackScope->class_name().c_str());
#if 0
               result->get_file_info()->display("for un-named construct, resultScope != topOfScopeStackScope: debug");
#endif
            // ROSE_ASSERT(false);
             }
        }
#endif


  // This can be a NULL value, for example un-named unions apparently return NULL
     return result;
   }
