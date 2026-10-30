
#include "edgRose.h"

#define DEBUG__parse_structlike 0

namespace EDG_ROSE_Translation {

SgDeclarationStatement* parse_structlike(
    SourceSequenceList& sse,
    a_type_ptr p,
    SgClassDeclaration::class_types t,
    bool forceTemplateClassDeclaration,
    a_template_ptr edg_template
) {
     ROSE_ASSERT(p != NULL);

#if DEBUG__parse_structlike
     printf("Enter parse_structlike():\n");
     printf("  p = %p\n", p);
     printf("  name = %s\n", p->source_corresp.name ? p->source_corresp.name : "null");
     printf("  abstract = %s\n", p->variant.class_struct_union.abstract ? "true" : "false");
     printf ("  forceTemplateClassDeclaration = %s\n", forceTemplateClassDeclaration ? "true" : "false");
     printf ("  is_unnamed(p)                    = %s \n",is_unnamed(p) ? "true" : "false");
     printf ("  t == SgClassDeclaration::e_union = %s \n",t == SgClassDeclaration::e_union ? "true" : "false");
#endif
#define DEBUG_PARSE_STRUCTLIKE 0

     testEdgRoseMaps();

     ROSE_ASSERT(internal_intermediate_scope != NULL);
     if (internal_intermediate_scope->symbol_table_size() != 0) {
       mprintf ("In parse_structlike(): TOP: internal_intermediate_scope->symbol_table_size() = %zu \n",internal_intermediate_scope->symbol_table_size());
     }

     ROSE_ASSERT(p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union || p->kind == tk_typeref);

     if (forceTemplateClassDeclaration == true) {
       ROSE_ASSERT(edg_template != NULL);
     }

     bool buildTemplateDeclaration   = false;
     bool buildTemplateInstantiation = false;
     bool forceTemplateDeclaration   = false;
     setTemplateOrTemplateInstantiationClassGeneration(p, forceTemplateDeclaration, buildTemplateDeclaration, buildTemplateInstantiation);
     if (forceTemplateClassDeclaration == true) {
       if (buildTemplateInstantiation == true && buildTemplateDeclaration == false) {
         buildTemplateDeclaration = true;
         buildTemplateInstantiation = false;
       }
     }
#if DEBUG__parse_structlike
     printf ("  forceTemplateDeclaration   = %s\n", forceTemplateClassDeclaration ? "true" : "false");
     printf ("  buildTemplateDeclaration   = %s\n", buildTemplateDeclaration ? "true" : "false");
     printf ("  buildTemplateInstantiation = %s\n", buildTemplateInstantiation ? "true" : "false");
#endif

     SgDeclarationStatement * result_declaration = NULL;
     if (buildTemplateDeclaration) {
       if (edg_template == NULL) {
         edg_template = p->variant.class_struct_union.extra_info->assoc_template;
         ROSE_ASSERT(edg_template != NULL);
       }
       ROSE_ASSERT(edg_template != NULL);
       result_declaration = convert_template_struct_primary(p, t, NULL, edg_template);
       ROSE_ASSERT(matching_defining_and_nondefining_variants(result_declaration) == true);
     } else {
       result_declaration = convert_struct_primary(p, t, NULL);
     }
     ROSE_ASSERT(result_declaration != NULL);

#if DEBUG__parse_structlike
     printf ("  result_declaration = %p = %s\n", result_declaration, result_declaration->class_name().c_str());
     printf ("    ->get_parent() = %p = %s\n", result_declaration->get_parent(), result_declaration->get_parent() ? result_declaration->get_parent()->class_name().c_str() : "");
     printf ("    ->get_scope()  = %p = %s\n", result_declaration->get_scope(), result_declaration->get_scope() ? result_declaration->get_scope()->class_name().c_str() : "");
#endif

     SgClassDeclaration * class_decl = isSgClassDeclaration(result_declaration);
     if (class_decl != NULL) {
       a_scope_ptr assoc_scope = p->variant.class_struct_union.extra_info->assoc_scope;

#if DEBUG__parse_structlike
       printf ("  assoc_scope = %p\n", assoc_scope);
       printf ("  class_decl->get_definition() = %p\n", class_decl->get_definition());
       printf ("  class_decl->get_name() = %s \n",class_decl->get_name().str());
       printf ("  scope_cache[assoc_scope] = %p\n", scope_cache[assoc_scope]);
#endif
       ROSE_ASSERT( !( assoc_scope != NULL ) || ( scope_cache.find(assoc_scope) != scope_cache.end() && scope_cache[assoc_scope] != NULL ) );
       ROSE_ASSERT( class_decl->get_definingDeclaration() == class_decl );
       ROSE_ASSERT( class_decl->get_definition() != NULL );

       if ( scope_cache[assoc_scope] != class_decl->get_definition() )
          {
            mprintf ("NOTE: Reset the scope_cache[assoc_scope] to class_decl->get_definition() \n");
            scope_cache[assoc_scope] = class_decl->get_definition();
          }
       ROSE_ASSERT( scope_cache[assoc_scope] == class_decl->get_definition() );
     }

     //// START check 1
     SgType * assoc_type = NULL;
     if (type_translation_cache.find(p) != type_translation_cache.end()) {
       assoc_type = type_translation_cache[p];
     } else if (template_type_translation_cache.find(p) != template_type_translation_cache.end()) {
       assoc_type = template_type_translation_cache[p];
     } else {
       ROSE_ABORT();
     }
     ROSE_ASSERT(assoc_type != NULL);

     SgClassType * class_assoc_type = isSgClassType(assoc_type);
     SgNonrealType * nonreal_assoc_type = isSgNonrealType(assoc_type);
     SgNonrealDecl * nonreal_decl = isSgNonrealDecl(result_declaration);

     ROSE_ASSERT( !(   class_decl != NULL ) || (  class_assoc_type != NULL) );
     ROSE_ASSERT( !( nonreal_decl != NULL ) || (nonreal_assoc_type != NULL) );

     if (class_assoc_type != NULL) {
       if (class_assoc_type->get_declaration()->variantT() == class_decl->variantT()) {
         class_assoc_type->get_declaration()->set_definingDeclaration(class_decl);
       } else {
         mprintf ("Note: class_assoc_type->get_declaration()->variantT() != class_decl->variantT() \n");
       }
     } else if (nonreal_assoc_type != NULL) {
       ROSE_ASSERT(isSgNonrealDecl(nonreal_assoc_type->get_declaration()));
     } else {
       ROSE_ABORT();
     }
     //// END check 2

     if (p->autonomous_primary_tag_decl == false) {
       ROSE_ASSERT(result_declaration != NULL);
       ROSE_ASSERT(result_declaration->get_scope() != NULL);
       if (class_decl != NULL) {
         class_decl->set_isAutonomousDeclaration(false);
       }
     } else {
       ROSE_ASSERT(class_decl != NULL);
       ROSE_ASSERT(class_decl->get_isAutonomousDeclaration() == true);
       class_decl->set_isAutonomousDeclaration(true);
     }

     SgClassDefinition* def = NULL;
     if (class_decl != NULL) {
       ROSE_ASSERT(class_decl->get_firstNondefiningDeclaration() != class_decl);
       ROSE_ASSERT(class_decl->get_firstNondefiningDeclaration() != NULL);
       push_entry_to_type_declaration_map(p,class_decl);

       fixup_scopes_of_earlier_declarations(type_declaration_map[p], topScopeStack()); // Fix scopes of earlier declarations that may not have had them correct

       class_decl->unsetForward();

       def = class_decl->get_definition();
       ROSE_ASSERT(def != NULL);

       class_decl->set_definingDeclaration(class_decl);
       def->set_isAbstract(p->variant.class_struct_union.abstract == true);
     }

     //// START check 2
     ROSE_ASSERT(result_declaration->get_scope() != NULL);
     ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end() || template_type_translation_cache.find(p) != template_type_translation_cache.end());
     SgClassType* classType = NULL;
     if (type_translation_cache.find(p) != type_translation_cache.end()) {
       ROSE_ASSERT(type_translation_cache[p] != NULL);
       classType = isSgClassType(type_translation_cache[p]);
     } else if (template_type_translation_cache.find(p) != template_type_translation_cache.end()) {
       ROSE_ASSERT(template_type_translation_cache[p] != NULL);
       classType = isSgClassType(template_type_translation_cache[p]);
     }
     ROSE_ASSERT(classType != NULL);
     ROSE_ASSERT (classType->get_declaration() != NULL);
     SgClassDeclaration* local_classDeclaration = isSgClassDeclaration(classType->get_declaration());
     ROSE_ASSERT (local_classDeclaration != NULL);
     ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() != NULL);
     ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() == local_classDeclaration);
     ROSE_ASSERT(class_decl->get_firstNondefiningDeclaration() != NULL);
     //// END check 2

     convert_base_class_list(p, def);
     pushScopeStack(def);
     ++sse;

     ROSE_ASSERT(p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union || p->kind == tk_typeref);

     if (is_unnamed(p) == false) {
       types_with_emitted_definitions[p] = true; // Creating a type definition for p
     }

     ROSE_ASSERT(internal_intermediate_scope != NULL);
     ROSE_ASSERT(internal_intermediate_scope->symbol_table_size() == 0);

     while (sse)
        {
          an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);
          SgDeclarationStatement* s = NULL;
          switch (kind)
             {
               case iek_field:
                  {
                    s = parse_field_declaration(sse);
                    appendStatement(s, def);
                    break;
                  }

               case iek_type:
                  {
                    s = parse_type_declaration(sse);
                    if (s != NULL) {
                      appendStatement(s, def);
                    }
                    break;
                  }

               case iek_routine:
                  {
                    bool forceTemplateDeclaration = forceTemplateClassDeclaration;
                    if (forceTemplateDeclaration == true)
                       {
                         ROSE_ASSERT(edg_template != NULL);
                       }
                    if (buildTemplateDeclaration == true)
                       {
                      // Save this sse entry as a declaration that would be edited away into a function prototype by EDG and so we need to add it to the AST later.
                         a_routine_ptr defining_declaration_in_template_class = (a_routine_ptr)(sse->entity.ptr);
                         ROSE_ASSERT (defining_declaration_in_template_class != NULL);

                         bool forceSecondaryDeclaration = false;
                         s = parse_routine(sse,forceTemplateDeclaration,edg_template,forceSecondaryDeclaration);
                         ROSE_ASSERT(s != NULL);

                      // Detect a defining declaration (which will be normalized by EDG to be a prototype).
                         if (s == s->get_definingDeclaration())
                            {
                              SgFunctionDeclaration* functionDeclaration = isSgFunctionDeclaration(s);
                              ROSE_ASSERT(functionDeclaration != NULL);
                              a_template_ptr associated_template_for_function = defining_declaration_in_template_class->assoc_template;
                              if (associated_template_for_function != NULL)
                                 {
                                   if (saved_defining_declarations_from_class_templates.find(associated_template_for_function) == saved_defining_declarations_from_class_templates.end())
                                      {
                                        saved_defining_declarations_from_class_templates[associated_template_for_function] = functionDeclaration;

                                     // DQ (8/30/2016): We need to mark this as an EDG normalization so that we can detect it as an exception 
                                     // to some simple attempts to filter the AST (e.g. for the Call Graph implementation which filters on only 
                                     // functions in the current directory).
                                        functionDeclaration->set_marked_as_edg_normalization(true);

                                        SgTemplateMemberFunctionDeclaration * tpl_method_decl = isSgTemplateMemberFunctionDeclaration(functionDeclaration);
                                        ROSE_ASSERT(tpl_method_decl != NULL);

                                        if (associated_template_for_function->text != NULL) {
                                          std::string edg_template_string = associated_template_for_function->text;
                                          tpl_method_decl->set_string(edg_template_string);
                                          tpl_method_decl->set_string_represents_function_body(true);
                                        }

                                     // DQ (1/11/2015): Reset the source position of the non-defining declaration to be that of the defining declaration so that the token mapping will work properly.
                                     // Note that this requirem,ent is due to now EDG has rewritten it's AST to place the defining template definition outside of the template class declaration.
                                        SgFunctionDeclaration* nondefiningFunctionDeclaration = isSgFunctionDeclaration(functionDeclaration->get_firstNondefiningDeclaration());
                                        ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);

                                     // Reset the source position of the first non-defining declaration that was modified by EDG (where it was previously a defining declaration).
                                        ROSE_ASSERT(nondefiningFunctionDeclaration->get_startOfConstruct() != NULL);
                                        ROSE_ASSERT(nondefiningFunctionDeclaration->get_endOfConstruct()   != NULL);
                                        *(nondefiningFunctionDeclaration->get_startOfConstruct()) = *(functionDeclaration->get_startOfConstruct());
                                        *(nondefiningFunctionDeclaration->get_endOfConstruct())   = *(functionDeclaration->get_endOfConstruct());
                                      }

                                   ROSE_ASSERT(s->get_firstNondefiningDeclaration() != NULL);

                                   s = s->get_firstNondefiningDeclaration();
                                   ROSE_ASSERT(s->get_definingDeclaration() != s->get_firstNondefiningDeclaration());
                                 }
                            }
                       }
                      else
                       {
                         bool forceSecondaryDeclaration = false;
                         s = parse_routine(sse,forceTemplateDeclaration,edg_template,forceSecondaryDeclaration);
                       }
                    appendStatement(s, def);
                    break;
                  }

               case iek_src_seq_secondary_decl:
                  {
                    an_il_entry_kind inner_kind = (an_il_entry_kind)(((a_src_seq_secondary_decl_ptr)(sse->entity.ptr))->entity.kind);
                    char* inner_ptr = ((a_src_seq_secondary_decl_ptr)(sse->entity.ptr))->entity.ptr;
                    ROSE_ASSERT(inner_ptr != NULL);
                    switch (inner_kind)
                       {
                         case iek_variable:
                            {
                              s = parse_secondary_declaration(sse);
                              appendStatement(s, def);
                              break;
                            }

                         case iek_type:
                            {
                              a_src_seq_secondary_decl* source_sequence_secondary_declaration = (a_src_seq_secondary_decl_ptr)(sse->entity.ptr);
                              ROSE_ASSERT(source_sequence_secondary_declaration != NULL);

                              s = convert_type_as_secondary_decl(sse);
                              ROSE_ASSERT(s != NULL);
                              ROSE_ASSERT(s->get_scope() != NULL);

                              if (s->get_scope() == def) {
                                appendStatement(s, def);
                              } else {
                                if (s->get_declarationModifier().isFriend() == true) {
                                   appendStatement(s, def);
                                }
                              }
                              ++sse;
                              break;
                            }

                         case iek_routine:
                            {
                              a_routine_ptr routinePtr = (a_routine_ptr)inner_ptr;
                              ROSE_ASSERT(routinePtr != NULL);

                              s = parse_secondary_declaration(sse);
                              appendStatement(s, def);

                              ROSE_ASSERT(routinePtr->type != NULL);
                              ROSE_ASSERT(routinePtr->type->variant.routine.extra_info != NULL);
                              break;
                            }

                         case iek_template:
                            {
                              s = parse_secondary_declaration(sse);
                              ROSE_ASSERT(s->get_declarationModifier().get_accessModifier().isDefault() == false);
                              appendStatement(s, def);
                              break;
                            }

                         default:
                            {
                              printf ("Error: subcase not handled in case iek_src_seq_secondary_decl: il_entry_kind_names[inner_kind] = %s \n",il_entry_kind_names[inner_kind]);
                              ROSE_ABORT();
                            }
                       }
                    ROSE_ASSERT(s != NULL);
                    break;
                  }

               case iek_macro:
                  {
                    s = parse_macro(sse);
                    if (s != NULL) {
                      appendStatement(s, def);
                    }
                    break;
                  }

               case iek_pragma:
                  {
                    s = parse_pragma(sse);
                    ROSE_ASSERT(s != NULL);
                    if (s->get_declarationModifier().get_accessModifier().isDefault()) {
                      s->get_declarationModifier().get_accessModifier().setPublic();
                    }
                    appendStatement(s, def);
                    break;
                  }

               case iek_template:
                  {
                    s = parse_template(sse);
                    ROSE_ASSERT(s != NULL);
                    ROSE_ASSERT(s->get_declarationModifier().get_accessModifier().isDefault() == false);

                    appendStatement(s, def);
                    break;
                  }

               case iek_using_decl:
                  {
                    s = parse_using_declaration(sse);
                    ROSE_ASSERT(s != NULL);

                    ROSE_ASSERT(s->get_declarationModifier().get_accessModifier().isDefault() == false);
                    appendStatement(s, def);
                    break;
                  }

               case iek_constant:
                  {
                    a_constant_ptr p = (a_constant_ptr)(sse->entity.ptr);
                    ++sse;

                    SgExpression * constant = convert_constant(p);
                    SgName name = p->source_corresp.name;
                    SgType* type = convert_type(p->type);
                    ROSE_ASSERT(type != NULL);

                    SgAssignInitializer* assignInitailizer = buildAssignInitializer_nfi(constant,type);
                    ROSE_ASSERT (assignInitailizer != NULL);
                    ROSE_ASSERT(constant->get_parent() != NULL);

                    SgScopeStatement* scope = def;
                    SgVariableDeclaration* constantVariableDeclaration = buildVariableDeclaration(name,type,assignInitailizer,scope);
                    ROSE_ASSERT(constantVariableDeclaration != NULL);

                    s = constantVariableDeclaration;

                    ROSE_ASSERT(s->get_declarationModifier().get_accessModifier().isDefault() == false);
                    ROSE_ASSERT(constantVariableDeclaration->get_variables().empty() == false);

                    SgInitializedName* constantField = *(constantVariableDeclaration->get_variables().begin());
                    ROSE_ASSERT(constantField != NULL);

                    constantVariableDeclaration->get_declarationModifier().get_storageModifier().setStatic();

                    SgType* nonconst_type = constantField->get_type();
                    ROSE_ASSERT(nonconst_type != NULL);

                    SgType* const_type = buildConstType(nonconst_type);
                    ROSE_ASSERT(const_type != NULL);
                    constantField->set_type(const_type);

                    appendStatement(constantVariableDeclaration, def);
                    break;
                  }

#if MICROSOFT_EXTENSIONS_ALLOWED
               case iek_ms_attribute:
                  {
                    SgMicrosoftAttributeDeclaration* result = parse_microsoft_attribute(sse);
                    ROSE_ASSERT(result != NULL);
                 // DQ (8/13/2020): Enforce that all member declarations are assigned an access.
                    ROSE_ASSERT(result->get_declarationModifier().get_accessModifier().isDefault() == false);
                    appendStatement(result, def);
                    break;
                  }
#endif

               case iek_static_assertion:
                  {
                    SgStaticAssertionDeclaration* result = parse_static_assertion_declaration(sse);
                    ROSE_ASSERT(result != NULL);
                    appendStatement(result, def);
                    break;
                  }

               case iek_variable:
                  {
                    SgVariableDeclaration* variable = parse_variable_declaration(sse);
                    ROSE_ASSERT(variable->get_declarationModifier().get_accessModifier().isDefault() == false);
                    appendStatement(variable, def);
                    break;
                  }

               // Keep last as it exit the parent loop using `goto`
               case iek_src_seq_end_of_construct: 
                  {
                    a_src_seq_end_of_construct_ptr edg_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);
                    ROSE_ASSERT(edg_end_of_construct != NULL);
                    check_end_of_construct(iek_type, p, sse); 
                    goto end;
                  }

               default:
                  {
                    printf("ERROR: parse_structlike(): kind = %s \n", il_entry_kind_names[kind]);
                    ROSE_ABORT();
                  }
             }
        } // End of WHILE loop over the class members.
end: ; // Label used to exit loop for `iek_src_seq_end_of_construct`

  ROSE_ASSERT(result_declaration->get_definingDeclaration() != NULL);
  ROSE_ASSERT(result_declaration->get_firstNondefiningDeclaration() != NULL);
  ROSE_ASSERT(result_declaration->get_definingDeclaration() == result_declaration);
  ROSE_ASSERT(result_declaration->get_firstNondefiningDeclaration() != result_declaration);
  ROSE_ASSERT(result_declaration->get_firstNondefiningDeclaration() != result_declaration->get_definingDeclaration());

  if (is_unnamed(p)) {
    ROSE_ASSERT(class_decl != NULL);
    class_decl->set_isUnNamed(true);
    isSgClassDeclaration(class_decl->get_firstNondefiningDeclaration())->set_isUnNamed(true);
    isSgClassDeclaration(class_decl->get_definingDeclaration())->set_isUnNamed(true);
  }

  if (buildTemplateInstantiation == true) {
    ROSE_ASSERT(class_decl != NULL);
    if (p->variant.class_struct_union.is_specialized == true) {
      ROSE_ASSERT(class_decl->get_specialization() == SgDeclarationStatement::e_specialization);
      ROSE_ASSERT(isSgClassDeclaration(class_decl->get_firstNondefiningDeclaration())->get_specialization() == SgDeclarationStatement::e_specialization);
    } else {
      ROSE_ASSERT(class_decl->get_file_info()->isCompilerGenerated() == true);
      ROSE_ASSERT(class_decl->get_firstNondefiningDeclaration()->get_file_info()->isCompilerGenerated() == true);
    }
  } else {
    ROSE_ASSERT(p->variant.class_struct_union.is_specialized == false);
  }

  popScopeStack();

  if (p->kind != tk_typeref) {
    set_file_info_for_corresp_and_end(def, p->source_corresp, sse);
  }

  if (p->kind != tk_typeref) {
    ++sse;
  }

  if (!is_unnamed(p)) {
    ROSE_ASSERT(types_with_emitted_definitions[p] == true);
    types_with_emitted_definitions[p] = false;
  }
  ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end() || template_type_translation_cache.find(p) != template_type_translation_cache.end());
  ROSE_ASSERT(isSgClassDeclaration(result_declaration->get_firstNondefiningDeclaration()) != NULL);

  testDefiningNonDefiningDeclarations(result_declaration->get_firstNondefiningDeclaration(), result_declaration, NULL, "parse_structlike()");

  testEdgRoseMaps();


  if (il_header.source_language != sl_C && is_unnamed(p)) {
    bool scope_is_class = isSgClassDefinition(result_declaration->get_scope());
    bool must_return_nullptr = (t != SgClassDeclaration::e_union) || !scope_is_class;
    result_declaration = must_return_nullptr ? nullptr : result_declaration;
  }

#if DEBUG__parse_structlike
  printf("  result_declaration = %p = %s\n", result_declaration, result_declaration->class_name().c_str());
  printf("LEAVE parse_structlike()\n");
#endif
       return result_declaration;
   }
}

