
#include "edgRose.h"

#define DEBUG_CONVERT_NONREAL_TYPE 0
#define WARN_CONVERT_NONREAL_TYPE  0

SgType* EDG_ROSE_Translation::convert_nonreal_type(a_type_ptr p, bool pointerToMember, bool from_template_parameter, const char * constant_name)
   {
     ROSE_ASSERT(p != NULL);

#if DEBUG_CONVERT_NONREAL_TYPE
     printf ("In convert_nonreal_type(p=%p): from_template_parameter = %s \n", p, from_template_parameter ? "true" : "false");
#endif

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("%s type (kind = %s) (name = %s)", (p == NULL ? "null " : ""), (p == NULL ? "null" : output_type_enum(p->kind).c_str()), (p == NULL ? "null " : p->source_corresp.name) );
     print_source_corresp(p->source_corresp);
     debug_print("\n");
#endif

     SgType* result = type_translation_cache.find(p) != type_translation_cache.end() ? type_translation_cache[p] : NULL;

#if DEBUG_CONVERT_NONREAL_TYPE
     printf ("In convert_nonreal_type(p=%p): result (from type_translation_cache) = %p \n",p,result);
#endif
#if 0
     printf ("In convert_nonreal_type(): Calling summarize_type() \n");
     summarize_type(p);
#endif

     if (result == NULL)
        {
#if DEBUG_CONVERT_NONREAL_TYPE
          printf("EDG_ROSE_Translation::convert_nonreal_type(...):\n");
          printf("  > p    = %p\n", p);
          printf("  > constant_name = %s\n", constant_name);
          printf("  name = %s\n", p->source_corresp.name);
          printf("  kind = %s\n", output_type_enum(p->kind).c_str());
#endif
          ROSE_ASSERT( nonreal_scope_for_template_parameters_stack.size() == templateDeclarationStack.size() );
//        ROSE_ASSERT( nonreal_scope_for_template_parameters_stack.size() > 0 ); // FIXME ROSE-1846

          // Retrieve the nonreal scope associated with the template being currently processed

          SgDeclarationScope * top_nrscope = NULL;
          if (nonreal_scope_for_template_parameters_stack.size() > 0) {
            a_template_ptr edg_template = templateDeclarationStack.back();
#if DEBUG_CONVERT_NONREAL_TYPE
            printf ("  edg_template = %p \n",edg_template);
#endif
            a_template_decl_ptr template_decl = edg_template ? edg_template->template_decl : NULL;
#if DEBUG_CONVERT_NONREAL_TYPE
            printf ("  template_decl = %p \n", template_decl);
#endif
            a_scope_ptr tpl_scope = template_decl ? template_decl->scope : NULL;
#if DEBUG_CONVERT_NONREAL_TYPE
            printf ("  tpl_scope = %p \n", tpl_scope);
#endif
            if (tpl_scope != NULL) {
              ROSE_ASSERT(nonreal_scope_cache.find(tpl_scope) != nonreal_scope_cache.end());

              top_nrscope = nonreal_scope_cache[tpl_scope];
              ROSE_ASSERT(top_nrscope != NULL);
            } else {
              top_nrscope = nonreal_scope_for_template_parameters_stack[nonreal_scope_for_template_parameters_stack.size()-1];
            }
            ROSE_ASSERT(top_nrscope == nonreal_scope_for_template_parameters_stack[nonreal_scope_for_template_parameters_stack.size()-1]);
          }

          // Resolve scope associated with this type

          a_scope_ptr edg_parent_scope = p->source_corresp.parent_scope;
#if DEBUG_CONVERT_NONREAL_TYPE
          printf("  edg_parent_scope = %p (%s)\n", edg_parent_scope, edg_parent_scope ? summarize_scope_in_string(edg_parent_scope) : "");
#endif

          bool valid_edg_parent_scope = edg_parent_scope != NULL && (edg_parent_scope->kind == sck_template_declaration || edg_parent_scope->kind == sck_class_struct_union);
          bool construct_nrscope = valid_edg_parent_scope;
          bool use_nrscope_stack = !valid_edg_parent_scope;
          bool save_in_nrscope_cache = valid_edg_parent_scope;
          bool attach_to_template_decl = false;
 
          SgDeclarationScope * nrscope = NULL;
          if (valid_edg_parent_scope && nonreal_scope_cache.find(edg_parent_scope) != nonreal_scope_cache.end()) {
            nrscope = nonreal_scope_cache[edg_parent_scope];
            ROSE_ASSERT(nrscope != NULL);
            construct_nrscope = false;
            use_nrscope_stack = false;
#if DEBUG_CONVERT_NONREAL_TYPE
            printf("  nrscope (from cache) = %p (%s)\n", nrscope, nrscope ? nrscope->class_name().c_str() : "");
#endif
          }

          SgTemplateClassDeclaration * tpldecl = NULL; // potential parent template

          a_symbol_ptr assoc_info = (a_symbol_ptr)p->source_corresp.assoc_info;
          // verify that not having a symbol means that this is an anonymous declaration (but not an actual template parameter)
          ROSE_ASSERT( !( assoc_info == NULL ) || ( p->source_corresp.name == NULL ) );

          ROSE_ASSERT( !( assoc_info != NULL ) || ( !assoc_info->is_error ) );
          ROSE_ASSERT( !( assoc_info != NULL ) || ( !assoc_info->is_nonreal_nested_type ) );
          ROSE_ASSERT( !( p->kind == tk_template_param && p->source_corresp.name != NULL ) || ( assoc_info != NULL ) );
          ROSE_ASSERT( !( p->kind == tk_template_param && p->source_corresp.name != NULL ) || ( assoc_info->is_template_param != assoc_info->is_class_member ) );

          if (assoc_info != NULL && assoc_info->is_class_member) {
            ROSE_ASSERT(assoc_info->parent.class_type);
#if DEBUG_CONVERT_NONREAL_TYPE
            printf("  parent_type (EDG) = %p\n", assoc_info->parent.class_type);
#endif
            SgType * parent_type = convert_type(assoc_info->parent.class_type, pointerToMember, from_template_parameter);
            ROSE_ASSERT(parent_type != NULL);
#if DEBUG_CONVERT_NONREAL_TYPE
            printf("  parent_type (ROSE) = %p (%s)\n", parent_type, parent_type ? parent_type->class_name().c_str() : "");
#endif

            SgNonrealType * parent_nrtype = isSgNonrealType(parent_type);
            SgClassType * parent_ctype = isSgClassType(parent_type);
            if (parent_nrtype != NULL) {
              // ROSE_ASSERT(assoc_info->is_nonreal_member); // Happens: `rebind` in `_Alloc::template rebind<_Tp>::other` (both _Tp and _Alloc are template parameter)

              SgNonrealDecl * nrdecl = isSgNonrealDecl(parent_nrtype->get_declaration());
              ROSE_ASSERT(nrdecl != NULL);
              if (nrscope == NULL) {
                nrscope = nrdecl->get_nonreal_decl_scope();
                ROSE_ASSERT(nrscope != NULL);
                construct_nrscope = false;
                use_nrscope_stack = false;
#if DEBUG_CONVERT_NONREAL_TYPE
                printf("  nrscope (from parent non-real declaration) = %p (%s)\n", nrscope, nrscope ? nrscope->class_name().c_str() : "");
#endif
              } else {
                ROSE_ASSERT(nrdecl->get_nonreal_decl_scope() != NULL);
                ROSE_ASSERT(nrscope == nrdecl->get_nonreal_decl_scope());
              }
            } else if (parent_ctype != NULL) {

              SgClassDeclaration * xdecl = isSgClassDeclaration(parent_ctype->get_declaration());
              ROSE_ASSERT(xdecl != NULL);

#if DEBUG_CONVERT_NONREAL_TYPE
              printf("  parent declaration = %p (%s) = %s\n", xdecl, xdecl ? xdecl->class_name().c_str() : "", xdecl ? xdecl->get_name().str() : "");
#endif
              tpldecl = isSgTemplateClassDeclaration(xdecl);
              if (tpldecl != NULL) {
#if DEBUG_CONVERT_NONREAL_TYPE
                printf("  Parent is a templated class: a non-real scope exists!\n");
#endif

                construct_nrscope = false;
                if (tpldecl->get_nonreal_decl_scope() != NULL) {
                  use_nrscope_stack = false;
                  if (nrscope == NULL) { 
                    nrscope = tpldecl->get_nonreal_decl_scope();
                    ROSE_ASSERT(nrscope != NULL);
                  } else {
                    ROSE_ASSERT(nrscope == tpldecl->get_nonreal_decl_scope());
                  }
                } else {
#if DEBUG_CONVERT_NONREAL_TYPE
                  printf("  Case when the non-real scope of the template declaration is not set\n");
#endif
                  attach_to_template_decl = true;
                  if (nrscope == NULL) {
                    // We use the non-real scope from the stack
                    use_nrscope_stack = true;
                  }
                }
              } else {
#if DEBUG_CONVERT_NONREAL_TYPE
                printf("  Case of a real parent type\n");
#endif

                // Case of a real parent type:
                //    template class declared in another class and instantiated using a template parameter from the current template
                if (nrscope == NULL) {
                  // we will use the non-real scope from the stack
                  construct_nrscope = false;
                  use_nrscope_stack = true;
                }
              }
#if DEBUG_CONVERT_NONREAL_TYPE
              printf("  nrscope (from parent class) = %p (%s)\n", nrscope, nrscope ? nrscope->class_name().c_str() : "");
#endif
            } else {
              ROSE_ASSERT(false);
            }
          }

          ROSE_ASSERT(!construct_nrscope || nrscope == NULL);
          ROSE_ASSERT(!construct_nrscope || valid_edg_parent_scope);
          ROSE_ASSERT(!use_nrscope_stack || nrscope == NULL);

          ROSE_ASSERT( !(nrscope == NULL) || (construct_nrscope != use_nrscope_stack) );

          ROSE_ASSERT( !(nrscope != NULL) || (!construct_nrscope && !use_nrscope_stack) );

          // TV (05/07/2018): Parent scope is a template declaration scope: create and register scope
          if (construct_nrscope) {
            nrscope = construct_nonreal_declaration_scope();
#if DEBUG_CONVERT_NONREAL_TYPE
            printf("  nrscope (new) = %p (%s)\n", nrscope, nrscope ? nrscope->class_name().c_str() : "");
#endif
          }

          // TV (05/04/2018): Sometime EDG sets the scope of template parameters "incorrectly (???)" to the global scope or enclosing namespace
          //                  It uses the last non-real scope constructed for a template declaration (in 'convert_template_prototype(ptr)' or 'parse_template(sse)') (stored in `nonreal_scope_for_template_parameters_stack`).
          if (use_nrscope_stack) {
            int32_t tpl_param_nesting_depth = 1;
            if (p->kind == tk_template_param && p->variant.template_param.extra_info != NULL) {
              tpl_param_nesting_depth = p->variant.template_param.extra_info->coordinates.depth;
            }
#if DEBUG_CONVERT_NONREAL_TYPE
            printf("  depth = %d\n", tpl_param_nesting_depth);
            printf("  stack = %zd\n", nonreal_scope_for_template_parameters_stack.size());
#endif
            if (tpl_param_nesting_depth == 0) {
#if DEBUG_CONVERT_NONREAL_TYPE
              printf("  case where the template parameter nesting depth is equal to 0 (should be greater or equal to 1).\n");
#if 0
              summarize_type(p);
#endif
#endif
#if DEBUG_CONVERT_NONREAL_TYPE
              printf("    set nesting depth to one\n");
#endif
              tpl_param_nesting_depth = 1;
            } else if (tpl_param_nesting_depth == -1) {
#if DEBUG_CONVERT_NONREAL_TYPE
              printf("  case where the template parameter nesting depth is equal to -1!!!\n");
#if 0
              summarize_type(p);
#endif
#endif
#if DEBUG_CONVERT_NONREAL_TYPE
              printf("   => the declared type of an auto typed variable. Return SgTemplateType\n");
#endif
              return new SgAutoType();
            } else if (tpl_param_nesting_depth > (int32_t)nonreal_scope_for_template_parameters_stack.size()) {
#if DEBUG_CONVERT_NONREAL_TYPE
              printf("  case where the template parameter nesting depth is greater than the size of the non-real scope stack\n");
#if 0
              summarize_type(p);
#endif
#endif
            }
            if (tpl_param_nesting_depth > 0 && tpl_param_nesting_depth <= (int32_t)nonreal_scope_for_template_parameters_stack.size()) {
              int pos = nonreal_scope_for_template_parameters_stack.size() - tpl_param_nesting_depth;
              nrscope = nonreal_scope_for_template_parameters_stack[pos];
              ROSE_ASSERT(nrscope != NULL);
#if DEBUG_CONVERT_NONREAL_TYPE
              printf("  using non-real scope from stack[%d] = %p to represent %p (%s)\n", pos, nrscope, edg_parent_scope, edg_parent_scope ? summarize_scope_in_string(edg_parent_scope) : "");
#endif
              attach_to_template_decl = false;
              save_in_nrscope_cache = false;
            } else {
#if DEBUG_CONVERT_NONREAL_TYPE
              printf("  invalid template parameter depth!\n");
#endif
              ROSE_ASSERT(false);
            }
          } else if (top_nrscope != NULL) {
            // Compare nrscope and top_nrscope
            bool nrscope_are_related = false;
            SgNode * tmp = nrscope;
            while (tmp != NULL) {
              if (tmp == top_nrscope) {
                nrscope_are_related = true;
                break;
              }
              tmp = tmp->get_parent();
              if (isSgGlobal(tmp))
                break;
            }
            if (!nrscope_are_related) {
              nrscope = top_nrscope;
              attach_to_template_decl = false;
              save_in_nrscope_cache = false;
            }
          }

          // TV: At this point if the parent scope from EDG is valid, we should have a non-real scope. It should either be matched in the cache, or we add it to the cache
          ROSE_ASSERT( !valid_edg_parent_scope || nrscope != NULL);

          // Add scope to cache if the parent scope from EDG was valid
          if (save_in_nrscope_cache) {

            if (nonreal_scope_cache.find(edg_parent_scope) == nonreal_scope_cache.end()) {
#if DEBUG_CONVERT_NONREAL_TYPE
              printf(" insert %p as nonreal-scope for %p\n", nrscope, edg_parent_scope);
#endif
              nonreal_scope_cache[edg_parent_scope] = nrscope;
            }
          }

          if (attach_to_template_decl) {
            ROSE_ASSERT(tpldecl != NULL);
            if (nrscope->get_parent() != NULL) {
#if DEBUG_CONVERT_NONREAL_TYPE
              printf("  Parent of non-real scope %p is already set to %p (%s)\n", nrscope, nrscope->get_parent(), nrscope->get_parent()->class_name().c_str());
#endif
              ROSE_ASSERT(nrscope->get_parent() == tpldecl);
            } else {
#if DEBUG_CONVERT_NONREAL_TYPE
              printf("  Attach non-real scope %p to template declaration %p (%s)\n", nrscope, tpldecl, tpldecl->get_name().str());
#endif

              tpldecl->set_nonreal_decl_scope(nrscope);
              nrscope->set_parent(tpldecl);
            }
          }

          // TV (05/23/2018): If we were not able to find a valid nonreal scope, we use the global non-real scope...
          if (nrscope == NULL) {
#if DEBUG_CONVERT_NONREAL_TYPE || 1
            printf("WARNING: In convert_nonreal_type(...): Incorrectly attach the declaration to the global non-real scope as we cannot figure the proper nonreal scope.\n");
#endif
            nrscope = global_nonreal_scope;
          }

          ROSE_ASSERT(nrscope != NULL);

          const char * name = p->source_corresp.name;
          if (name == NULL) {
            name = constant_name;
          }
          if (name == NULL) {
            char * name_ = new char[64];
            int l = sprintf(name_, "__template_type_parameter_%p", p);
            name_[l] = '\0';
            name = name_;
#if DEBUG_CONVERT_NONREAL_TYPE
            printf ("  use \"%s\" for the unnamed type %p\n", name, p);
#endif
          }

          SgNonrealDecl * nrdecl = SageBuilder::buildNonrealDecl(name, nrscope);
          ROSE_ASSERT(nrdecl != NULL);
          set_generated_file_info_to_unparse(nrdecl);
          result = nrdecl->get_type();
          nrdecl->set_parent(nrscope);

#if DEBUG_CONVERT_NONREAL_TYPE
          printf ("  is_class_member        = %s\n", (assoc_info != NULL && assoc_info->is_class_member) ? "true" : "false");
          printf ("  is_template_param      = %s\n", (assoc_info != NULL && assoc_info->is_template_param) ? "true" : "false");
          printf ("  is_nonreal_member      = %s\n", (assoc_info != NULL && assoc_info->is_nonreal_member) ? "true" : "false");
          printf ("  is_nonreal_nested_type = %s\n", (assoc_info != NULL && assoc_info->is_nonreal_nested_type) ? "true" : "false");
#endif

          if (assoc_info != NULL && assoc_info->is_class_member) {
            nrdecl->set_is_class_member(true);
          } else if (assoc_info != NULL && assoc_info->is_template_param) {
            nrdecl->set_is_template_param(true);
          }

          SgDeclarationScope * child_nrscope = nrdecl->get_nonreal_decl_scope();
          ROSE_ASSERT(child_nrscope != NULL);
 
#if DEBUG_CONVERT_NONREAL_TYPE
          printf ("  result = %p = %s \n",result,result->class_name().c_str());
#endif
          ROSE_ASSERT (result != NULL);
          type_translation_cache[p] = result;

       // TV (04/16/2018): Post-processing based on the type of type:
       //                     - template parameters
       //                     - members known to be non-real classes
       //                     - members that could be methods, variables, or classes
          switch (p->kind) {
            case tk_class:
            case tk_struct:
            case tk_union: {
              ROSE_ASSERT(p->variant.class_struct_union.is_nonreal_class);

              a_class_type_supplement_ptr extra_info = p->variant.class_struct_union.extra_info;
              if (extra_info != NULL) {
             // TV (04/16/2018): Insert non-real scope from EDG and ROSE in corresponding translation table
                a_scope_ptr assoc_scope = extra_info->assoc_scope;
                if (assoc_scope != NULL && (assoc_scope->kind == sck_template_declaration || assoc_scope->kind == sck_class_struct_union)) {
#if DEBUG_CONVERT_NONREAL_TYPE
                  printf(" insert %p as nonreal-scope for %p\n", child_nrscope, assoc_scope);
#endif
                  nonreal_scope_cache[assoc_scope] = child_nrscope; 
                } else {
#if WARN_CONVERT_NONREAL_TYPE
                  printf("WARNING: in `convert_nonreal_type`: scope associated with %p is invalid (EDG issue). scope = %p (%s)\n", p, assoc_scope, assoc_scope ? summarize_scope_in_string(assoc_scope) : "");
#endif
                }

                SgDeclarationStatement * assoc_tpl_decl_stmt = NULL;
                SgDeclarationScope * assoc_nonreal_decl_scope = NULL;
                if (extra_info->assoc_template != NULL) {
                  assoc_tpl_decl_stmt = getFirstNondefDeclForTemplate(extra_info->assoc_template);
                  if (assoc_tpl_decl_stmt == NULL && extra_info->assoc_template->prototype_instantiation.type != NULL) {
                    assoc_tpl_decl_stmt = convert_template_prototype(extra_info->assoc_template);
                  } else if (assoc_tpl_decl_stmt == NULL && extra_info->assoc_template->prototype_instantiation.type == NULL) {
                    SgNonrealDecl * assoc_nrdecl = SageBuilder::buildNonrealDecl(name, nrscope);
                    ROSE_ASSERT(assoc_nrdecl != NULL);
                    set_generated_file_info_to_unparse(assoc_nrdecl);
                    assoc_nrdecl->set_parent(nrscope);
                    assoc_nrdecl->set_is_nonreal_template(true);
                    assoc_tpl_decl_stmt = assoc_nrdecl;

                    setFirstNondefDeclForTemplate(extra_info->assoc_template, assoc_nrdecl);
                  }
                  ROSE_ASSERT(assoc_tpl_decl_stmt != NULL);
#if DEBUG_CONVERT_NONREAL_TYPE
                  printf("  assoc_tpl_decl_stmt = %p (%s)\n", assoc_tpl_decl_stmt, assoc_tpl_decl_stmt->class_name().c_str());
#endif
                  nrdecl->set_templateDeclaration(assoc_tpl_decl_stmt);

                  if (templateDeclarationStack.size() == 0 || templateDeclarationStack[templateDeclarationStack.size() - 1] != extra_info->assoc_template) {
                    assoc_nonreal_decl_scope = get_nonreal_declaration_scope_for_template(extra_info->assoc_template, "convert_nonreal_type");
                    ROSE_ASSERT(assoc_nonreal_decl_scope != NULL);
                    templateDeclarationStack.push_back(extra_info->assoc_template);
                  }
                }

                SgTemplateArgumentPtrList & tpl_args = nrdecl->get_tpl_args();

#if DEBUG_CONVERT_NONREAL_TYPE
                printf("  calls `parse_template_arguments`\n");
#endif
                a_template_arg_ptr tpl_arg = extra_info->template_arg_list;
                if (tpl_arg != NULL) {
#if DEBUG_CONVERT_NONREAL_TYPE
                  printf("    for template instantiation\n");
#endif
                  tpl_args = parse_template_arguments(tpl_arg, child_nrscope);
                }

                a_template_arg_ptr part_spec_tpl_arg = extra_info->partial_spec_template_arg_list;
                if (part_spec_tpl_arg != NULL) {
#if DEBUG_CONVERT_NONREAL_TYPE
                  printf("    for template specialization\n");
#endif
                  tpl_args = parse_template_arguments(part_spec_tpl_arg, child_nrscope);
                }

                if (assoc_nonreal_decl_scope != NULL) {
                  templateDeclarationStack.pop_back();
                  attach_nonreal_declaration_scope_to_template(extra_info->assoc_template, assoc_tpl_decl_stmt, assoc_nonreal_decl_scope, "convert_nonreal_type::assoc_template_declaration");
                }
              }
              break;
            }
            case tk_template_param: {
              a_template_param_type_supplement_ptr extra_info = p->variant.template_param.extra_info;
              if (extra_info != NULL) {
                if (extra_info->class_type != NULL) {
                  a_type_ptr class_type = extra_info->class_type;
                  if (class_type->kind != tk_class) {
                    summarize_type(class_type);
                    ROSE_ASSERT(false);
                  }

               // Save the associated class type as an "alias"
                  type_translation_cache[class_type] = result;
#if DEBUG_CONVERT_NONREAL_TYPE
                  printf("  %p (tpl) and %p (class type) both point to %p (rose)\n", p, extra_info->class_type, result);
#endif
                  a_class_type_supplement_ptr class_type_extra_info = class_type->variant.class_struct_union.extra_info;
                  if (class_type_extra_info != NULL) {
                    a_scope_ptr assoc_scope = class_type_extra_info->assoc_scope;
                    if (assoc_scope != NULL && (assoc_scope->kind == sck_template_declaration || assoc_scope->kind == sck_class_struct_union)) {
#if DEBUG_CONVERT_NONREAL_TYPE
                      printf(" insert %p as nonreal-scope for %p\n", child_nrscope, assoc_scope);
#endif
                      nonreal_scope_cache[assoc_scope] = child_nrscope;
                    }
                  }
                }

                if (extra_info->orig_nested_type != NULL) {
                  a_type_ptr orig_nested_type = extra_info->orig_nested_type;
                  if (orig_nested_type->kind != tk_class && orig_nested_type->kind != tk_struct && orig_nested_type->kind != tk_union) {
                    summarize_type(orig_nested_type);
                    ROSE_ASSERT(false);
                  }

               // Save the associated origin nested type as an "alias"
                  type_translation_cache[orig_nested_type] = result;
#if DEBUG_CONVERT_NONREAL_TYPE
                  printf("  %p (tpl) and %p (origin nested type) both point to %p (rose)\n", p, extra_info->orig_nested_type, result);
#endif
                  a_class_type_supplement_ptr orig_nested_type_extra_info = orig_nested_type->variant.class_struct_union.extra_info;
                  if (orig_nested_type_extra_info != NULL) {
                    a_scope_ptr assoc_scope = orig_nested_type_extra_info->assoc_scope;
                    if (assoc_scope != NULL && (assoc_scope->kind == sck_template_declaration || assoc_scope->kind == sck_class_struct_union)) {
#if DEBUG_CONVERT_NONREAL_TYPE
                      printf(" insert %p as nonreal-scope for %p\n", child_nrscope, assoc_scope);
#endif
                      nonreal_scope_cache[assoc_scope] = child_nrscope;
                    }
                  }
                }

                nrdecl->set_template_parameter_position(extra_info->coordinates.position);
                nrdecl->set_template_parameter_depth(extra_info->coordinates.depth);
              }
              break;
            }
            default: {
              ROSE_ASSERT(false);
            }
          }
       }

  // DQ (9/9/2012): Added assertion.
     ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());

     ROSE_ASSERT(type_translation_cache[p] != NULL);

#if DEBUG_CONVERT_NONREAL_TYPE
     printf ("********** \n");
     printf ("Leaving convert_nonreal_type(p=%p): result = %p = %s \n",p,result,result->class_name().c_str());
     printf ("********** \n");
#endif

     ROSE_ASSERT (result != NULL);
     return result;
   }
