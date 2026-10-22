
#include "edgRose.h"

#define DEBUG_CONVERT_CST_TPL_PARAM 0

SgExpression* EDG_ROSE_Translation::convert_constant_template_parameter( a_constant_ptr e )
   {
  // When a constant is marked as a template parameter it may have one of several kinds (front end only except when PROTOTYPE_INSTANTIATIONS_IN_IL is TRUE).

  // a_constant_ptr template_parameter_constant = e->variant.template_param;

     SgExpression* result = NULL;

     ROSE_ASSERT(e != NULL);

  // DQ (1/2/2012): Refactoring to generate common name used in this function.
     SgName name = e->source_corresp.name;

#if DEBUG_CONVERT_CST_TPL_PARAM
     printf ("In convert_constant_template_parameter():\n");
     printf ("  e    = %p\n", e);
     printf ("  name = %s\n", name.str());
#endif

     ROSE_ASSERT(e->kind == ck_template_param);

     a_template_param_constant_kind template_parameter_kind = e->variant.template_param.kind;

     switch (template_parameter_kind)
        {
          case tpck_param:
             {
            // The template param constant represents a simple non-type template parameter, e.g., for I in the following:
            //      template <int I> class A { int a[I]; };
            // This is the most common and obvious case.
            // Another case is (more complex) is: 
            //    template <void (foo::*M)()> void test() {}
            // and: 
            //    template <int foo::*M> void test() {}
            // Here, M is a tpck_param, but the two differ in the type associated with the template parameter.

               const char * aname = e->source_corresp.name;
               char * noname = new char[64];
               const char * pattern = "__unnamed_template_type_parameter_depth_%d_position_%d";
               int l = sprintf(noname, pattern, e->variant.template_param.variant.coordinates.depth, e->variant.template_param.variant.coordinates.position);
               noname[l] = '\0';
               
               if (aname == NULL) {
                 aname = noname;
                 noname = NULL;
               }
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("  aname  = %s\n", aname ? aname : "");
               printf ("  noname = %s\n", noname ? noname : "");
#endif

               a_scope_ptr edg_scope = e->source_corresp.parent_scope;
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("  edg_scope = %p \n", edg_scope);
#endif
               if (edg_scope == NULL) {
                 a_template_ptr edg_template = templateDeclarationStack.back();
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf ("  edg_template = %p \n",edg_template);
#endif
                 a_template_decl_ptr template_decl = edg_template ? edg_template->template_decl : NULL;
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf ("  template_decl = %p \n", template_decl);
#endif
                 edg_scope = template_decl ? template_decl->scope : NULL;
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf ("  edg_scope = %p \n", edg_scope);
#endif
               }

               if (edg_scope == NULL) {
                 ROSE_ASSERT(nonreal_scope_cache.find(edg_scope) == nonreal_scope_cache.end());
               } else {
                 ROSE_ASSERT(edg_scope != NULL);
                 ROSE_ASSERT(edg_scope->kind == sck_template_declaration);
               }

               SgDeclarationScope * nrscope = NULL;
               if (nonreal_scope_cache.find(edg_scope) != nonreal_scope_cache.end()) {
                 nrscope = nonreal_scope_cache[edg_scope];
               } else {
                 // Last ditch solution... TODO use assoc type!!!
#if 0
                 printf ("Last ditch solution... TODO use assoc type!!! \n");
#endif
                 ROSE_ASSERT(nonreal_scope_for_template_parameters_stack.size() > 0);
                 nrscope = nonreal_scope_for_template_parameters_stack.back();
                 ROSE_ASSERT(nrscope != NULL);
               }
               ROSE_ASSERT(nrscope != NULL);
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("  nrscope = %p\n", nrscope);
#endif

               SgVariableSymbol * vsym = nrscope->lookup_variable_symbol(aname);
               if (vsym == NULL) {
                 a_type_ptr edg_type = e->type;
                 ROSE_ASSERT(edg_type != NULL);
                 SgType* type = convert_type(edg_type);
                 ROSE_ASSERT(type != NULL);

                 SgInitializedName * init_name = set_file_info_for_corresp(buildInitializedName_nfi(aname,type,NULL), e->source_corresp);
                 init_name->set_scope(nrscope);
                 init_name->set_parent(nrscope);
                 vsym = new SgVariableSymbol(init_name);

                 nrscope->insert_symbol(aname, vsym);
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf ("  init_name = %p\n", init_name);
#endif
               }
               ROSE_ASSERT(vsym != NULL);
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("  vsym = %p\n", vsym);
#endif

               result = set_file_info_for_corresp(SageBuilder::buildVarRefExp_nfi(vsym), e->source_corresp);

               if (noname != NULL)
                 delete [] noname;
               else
                 delete [] aname;
#if 0
               printf ("In convert_constant_template_parameter(): case tpck_param: result = %p = %s type = %p = %s \n",result,result->class_name().c_str(),templateParameterType,templateParameterType->class_name().c_str());
#endif

               break;
             }

          case tpck_expression:
             {
            // The template param constant represents an expression, e.g., for I+1 in the following:
            //    template <int I> class A { static char s[I+1]; };
            //    template <int I> char A<I>::s[I+1] = { 0 };
#if 0
               printf ("case of tpck_expression \n");
#endif
#if 0
               printf ("In convert_constant_template_parameter(): case tpck_expression: Calling summarize_constant() \n");
               summarize_constant(e);
#endif
            // Expression node representing a constant value in terms of an expression involving one or more
            // ck_template_param constants -- e.g., if "I" is a template param constant (of kind tpck_param),
            // "I+1" is also a template param constant (of kind tpck_expression).
               an_expr_node_ptr expression = e->variant.template_param.variant.expr;
               if (expression != NULL)
                  {
                    result = convert_expression(expression);
                  }
                 else
                  {
#if WARNING_FOR_NONREAL_DEVEL
                    printf ("ERROR: In convert_constant_template_parameter(): case tpck_expression: no associated expression. It seems to happen for reference to const variables used a template argument (replace by SgIntValue(42) )\n");
#endif
                    result = set_file_info_for_corresp(buildIntVal_nfi(42,"42"),e->source_corresp);
                  }
               break;
             }

          case tpck_member:
             {
            // The template param constant represents a member of a tk_template_param class, e.g., for T::k in the following:
            //    template <class T> class A { int a[T::k]; };
            // Represents the "value" of the member; see also tpck_address.

            // DQ (2/24/2013): However, it appears that in practice this can stand for a template member function and when it does it
            // is not clear when it is a member function.  So we need to decide how to build this, either as a SgVarRef, or a SgFunctionRef.
            // Also it is common that the scope of the reference has not been process before so it si not in the scope _cache and so 
            // we can't search for the name to decide if it is a data member of member function.  Also, the associated scope via the 
            // type is also empty (a template instantiation).  So we don't have anything to help figure this out, it seems.
            // *** Maybe we could find the associated template declaration ans search that? ***
#if 0
               printf ("template constant support for variable reference name = %s \n",name.str());
#endif
               a_scope_ptr edg_scope = e->source_corresp.parent_scope;
               ROSE_ASSERT(edg_scope != NULL);
               ROSE_ASSERT(edg_scope->kind == sck_class_struct_union);

               SgDeclarationScope * nrscope = NULL;
               if (nonreal_scope_cache.find(edg_scope) != nonreal_scope_cache.end()) {
                 nrscope = nonreal_scope_cache[edg_scope];
               } else {
                 a_type_ptr edg_assoc_type = edg_scope->variant.assoc_type;
                 ROSE_ASSERT(edg_assoc_type != NULL);
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf("  edg_assoc_type = %p\n", edg_assoc_type);
#endif
                 SgType * rose_assoc_type = convert_type(edg_assoc_type);
                 ROSE_ASSERT(rose_assoc_type != NULL);
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf("  rose_assoc_type = %p (%s)\n", rose_assoc_type, rose_assoc_type ? rose_assoc_type->class_name().c_str() : "");
#endif
                 if (nonreal_scope_cache.find(edg_scope) != nonreal_scope_cache.end()) {
                   nrscope = nonreal_scope_cache[edg_scope];
                 } else {
                   SgNamedType * ntype = isSgNamedType(rose_assoc_type);
                   ROSE_ASSERT(ntype != NULL);

                   SgDeclarationStatement * declstmt = ntype->get_declaration();
                   ROSE_ASSERT(declstmt != NULL);
#if DEBUG_CONVERT_CST_TPL_PARAM
                   printf("  declstmt = %p (%s)\n", declstmt, declstmt ? declstmt->class_name().c_str() : "");
#endif
                   SgTemplateClassDeclaration * tcdecl = isSgTemplateClassDeclaration(declstmt);
                   if (tcdecl != NULL) {
                     nrscope = tcdecl->get_nonreal_decl_scope();
                   }
                   if (nrscope == NULL) {
                     // Last ditch solution: Seen in ROSE-1998
                     ROSE_ASSERT(nonreal_scope_for_template_parameters_stack.size() > 0);
                     nrscope = nonreal_scope_for_template_parameters_stack.back();
                     ROSE_ASSERT(nrscope != NULL);
                   }
                 }
               }
               ROSE_ASSERT(nrscope != NULL);

               SgNonrealSymbol * nrsym = isSgNonrealSymbol(nrscope->lookup_symbol(name));
               if (nrsym == NULL) {
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf("  symbol not found build nonreal decl\n");
#endif
                 SgNonrealDecl * nrdecl = SageBuilder::buildNonrealDecl(name, nrscope);
                 ROSE_ASSERT(nrdecl != NULL);
                 set_generated_file_info_to_unparse(nrdecl);
                 nrdecl->set_parent(nrscope);

                 nrdecl->set_is_class_member(true);

                 nrsym = isSgNonrealSymbol(nrdecl->get_symbol_from_symbol_table());
               } else {
                 SgNonrealDecl * nrdecl = nrsym->get_declaration();
                 ROSE_ASSERT(nrdecl != NULL);

                 ROSE_ASSERT(nrdecl->get_is_class_member());
               }
               ROSE_ASSERT(nrsym != NULL);

               result = SageBuilder::buildNonrealRefExp_nfi(nrsym);
               result = set_file_info_for_corresp(result, e->source_corresp);

               break;
             }

          case tpck_unknown_function:
             {
            // Represents the address of an unknown function called in a template context where overload
            // resolution cannot be done (e.g., because the argument types involve template parameters).
            // "address" really means "an rvalue for the function," which has unknown type and might therefore be a
            // pointer or a pointer to member.

            // Note that EDG has very littel information about this function (only the name, no scope information, ect.

#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("In convert_constant_template_parameter(): case of tpck_unknown_function: name = %s \n",e->source_corresp.name);
#endif
            // DQ (2/24/2013): Refactored this code to a seperate function so that it could be called elsewhere for similar template handling.
               result = buildSpeculativeFunctionRefExp(e);

            // ROSE_ASSERT(!"case of tpck_unknown_function \n");

               break;
             }
          case tpck_address:
             {
            // Used, pointing to a tpck_member constant, to indicate the address of the indicated member.
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("In convert_constant_template_parameter(): case of tpck_address \n");
#endif
               ROSE_ASSERT(!"case of tpck_address \n");

               break;
             }

          case tpck_sizeof:
             {
            // The template param constant represents the sizeof operator applied to a type or expression that
            // contains a template parameter type.
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("In convert_constant_template_parameter(): case of tpck_sizeof \n");
#endif
            // If there is no type, then the sizeof operator is being applied to an expression.
               if (e->variant.template_param.variant.templ_sizeof.type != NULL)
                  {
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("- of type %p\n", e->variant.template_param.variant.templ_sizeof.type);
                    ScopedIndent si(indent);
#endif

                    SgType* type = convert_type(e->variant.template_param.variant.templ_sizeof.type);
                    ROSE_ASSERT(type != NULL);

#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("-> %s\n", type->class_name().c_str());
#endif
                    result = set_file_info_for_corresp(buildSizeOfOp_nfi(type),e->source_corresp);
                  }
                 else
                  {
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("- of expression %p\n", e->variant.template_param.variant.templ_sizeof.expr);
                    ScopedIndent si(indent);
#endif

                    SgExpression* expr = convert_expression(e->variant.template_param.variant.templ_sizeof.expr);
                    ROSE_ASSERT(expr != NULL);

#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("-> %s\n", expr->class_name().c_str());
#endif
                    result = set_file_info_for_corresp(buildSizeOfOp_nfi(expr),e->source_corresp);
                  }
            // ROSE_ASSERT(!"case of tpck_sizeof \n");

               break;
             }

          case tpck_alignof:
             {
            // The template param constant represents the __ALIGNOF__ operator applied to a type or
            // expression that contains a template parameter type.
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("In convert_constant_template_parameter(): case of tpck_alignof (not clear where this is coming from in Boost header files) \n");
#endif
#if 0
               printf ("In convert_constant_template_parameter(): case tpck_alignof: Calling summarize_constant() \n");
               summarize_constant(e);
#endif
               ROSE_ASSERT(e->variant.template_param.variant.templ_sizeof.type != NULL);

               SgType* type = convert_type(e->variant.template_param.variant.templ_sizeof.type);
               ROSE_ASSERT(type != NULL);
               result = buildAlignOfOp_nfi(type);

               break;
             }

          case tpck_uuidof:
             {
            // The template param constant represents the Microsoft __uuidof operator applied to a type
            // that contains a template parameter type.  It represents the address of the implied structure.
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("In convert_constant_template_parameter(): case of tpck_uuidof \n");
#endif
               ROSE_ASSERT(!"case of tpck_uuidof \n");

               break;
             }

          case tpck_typeid:
             {
            // The template param constant represents the typeid operator applied to a type or expression that
            // contains a template parameter type.  It represents the address of the implied std::type_info
            // structure.  (Only used when typeid is used for a nontype template argument; e.g. "X<&typeid(Y)>".)
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("In convert_constant_template_parameter(): case of tpck_typeid \n");
#endif
               ROSE_ASSERT(!"case of tpck_typeid \n");

               break;
             }

          case tpck_template_ref:
             {
            // The template param constant provides the address of an unknown function template, and a set of
            // explicit template arguments for that template. Very similar to tpck_unknown_function, and likewise
            // represents an rvalue for the function, which has unknown type and might therefore be a pointer or a
            // pointer to member.

#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("In convert_constant_template_parameter(): case of tpck_template_ref\n");
#endif

               a_scope_ptr edg_scope = e->source_corresp.parent_scope;
               if (edg_scope == NULL) {
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf ("  missing parent scope!\n");
#endif
                 a_template_ptr edg_template = templateDeclarationStack.back();
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf ("  edg_template = %p \n",edg_template);
#endif
                 a_template_decl_ptr template_decl = edg_template ? edg_template->template_decl : NULL;
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf ("  template_decl = %p \n", template_decl);
#endif
                 edg_scope = template_decl ? template_decl->scope : NULL;
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf ("  edg_scope = %p \n", edg_scope);
#endif
               }
               ROSE_ASSERT(edg_scope != NULL);
               ROSE_ASSERT(edg_scope->kind == sck_template_declaration);

               SgDeclarationScope * nrscope = NULL;
               if (nonreal_scope_cache.find(edg_scope) != nonreal_scope_cache.end()) {
                 nrscope = nonreal_scope_cache[edg_scope];
               } else {
                 // Last ditch solution... TODO use assoc type!!!
                 ROSE_ASSERT(nonreal_scope_for_template_parameters_stack.size() > 0);
                 nrscope = nonreal_scope_for_template_parameters_stack.back();
                 ROSE_ASSERT(nrscope != NULL);
               }
               ROSE_ASSERT(nrscope != NULL);
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("  nrscope = %p (%s)\n", nrscope, nrscope->class_name().c_str());
#endif

               SgName name = e->variant.template_param.variant.template_ref.con->source_corresp.name;
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("  name = %s\n", name.str());
#endif

               SgScopeStatement* tpl_scope = getDeclarationScope(e->variant.template_param.variant.template_ref.con->source_corresp);          
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("  tpl_scope = %p (%s) \n", tpl_scope, tpl_scope ? tpl_scope->class_name().c_str() : "");
#endif
               ROSE_ASSERT( tpl_scope != NULL );

               SgFunctionSymbol * tplFunctionSymbol = lookupFunctionSymbolInParentScopes(name, tpl_scope);
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("  tplFunctionSymbol = %p (%s) \n", tplFunctionSymbol, tplFunctionSymbol ? tplFunctionSymbol->class_name().c_str() : "");
#endif
#if WARNING_FOR_NONREAL_DEVEL
               if (tplFunctionSymbol == NULL) {
                 printf ("WARNING: In convert_constant_template_parameter(): case of tpck_template_ref: Cannot find symbol for associated template.\n");
                 printf ("  nrscope = %p (%s)\n", nrscope, nrscope->class_name().c_str());
                 printf ("  name = %s\n", name.str());
                 printf ("  tpl_scope = %p (%s) \n", tpl_scope, tpl_scope ? tpl_scope->class_name().c_str() : "");
               }
#endif

               SgFunctionDeclaration * tplFunctionDecl = tplFunctionSymbol ? tplFunctionSymbol->get_declaration() : NULL;
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("  tplFunctionDecl = %p (%s) \n", tplFunctionDecl, tplFunctionDecl ? tplFunctionDecl->class_name().c_str() : "");
#endif
               ROSE_ASSERT( !(tplFunctionSymbol != NULL) || (tplFunctionDecl != NULL) );

               SgTemplateArgumentPtrList templateArgumentList = parse_template_arguments(e->variant.template_param.variant.template_ref.arg_list, nrscope);

               SgNonrealSymbol * nrsym = isSgNonrealSymbol(nrscope->lookup_symbol(name, NULL, &templateArgumentList));
               if (nrsym == NULL) {
#if DEBUG_CONVERT_CST_TPL_PARAM
                 printf("  symbol not found build nonreal decl\n");
#endif
                 SgNonrealDecl * nrdecl = SageBuilder::buildNonrealDecl(name, nrscope);
                 ROSE_ASSERT(nrdecl != NULL);
                 set_generated_file_info_to_unparse(nrdecl);
                 nrdecl->set_parent(nrscope);

                 nrdecl->set_is_nonreal_function(true);
                 nrdecl->set_is_class_member(e->variant.template_param.variant.template_ref.con->source_corresp.is_class_member);

                 nrdecl->set_templateDeclaration(tplFunctionDecl);

                 nrdecl->get_tpl_args() = templateArgumentList;

                 nrsym = isSgNonrealSymbol(nrdecl->get_symbol_from_symbol_table());
               } else {
                 SgNonrealDecl * nrdecl = nrsym->get_declaration();
                 ROSE_ASSERT(nrdecl != NULL);

                 ROSE_ASSERT(nrdecl->get_is_nonreal_function());
                 ROSE_ASSERT(nrdecl->get_is_class_member() == e->variant.template_param.variant.template_ref.con->source_corresp.is_class_member);
               }
               ROSE_ASSERT(nrsym != NULL);

               result = SageBuilder::buildNonrealRefExp_nfi(nrsym);
               result = set_file_info_for_corresp(result, e->source_corresp);

               break;
             }

          case tpck_noexcept:
             {
            // The template param constant represents the noexcept operator applied to an expression that
            // contains a template parameter type.
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("In convert_constant_template_parameter(): case tpck_noexcept: \n");
#endif
               SgExpression* expr = convert_expression(e->variant.template_param.variant.templ_sizeof.expr);
               ROSE_ASSERT(expr != NULL);

               result = set_file_info_for_corresp(buildNoexceptOp_nfi(expr),e->source_corresp);

               break;
             }

       // DQ (3/23/2014): Added new case for EDG 4.8.
          case tpck_destructor:
             {
            // The template param constant represents a destructor of a nonreal class.

            // I think we might just have to build a reference to the member function (might have to 
            // build the member function) for the template.
            // a_type_ptr template_class_type = e->variant.template_param.variant.destructor.type;

#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("In convert_constant_template_parameter(): case of tpck_destructor: name = %s \n",e->source_corresp.name);
#endif
               result = buildSpeculativeFunctionRefExp(e);
               set_file_info_for_corresp(result,e->source_corresp);
               break;
             }

       // DQ (7/23/2020): Added support for EDG 6.0
          case tpck_integer_pack:
             {
            // The template param constant represents the typeid operator applied to a type or expression that
            // contains a template parameter type.  It represents the address of the implied std::type_info
            // structure.  (Only used when typeid is used for a nontype template argument; e.g. "X<&typeid(Y)>".)
#if DEBUG_CONVERT_CST_TPL_PARAM
               printf ("In convert_constant_template_parameter(): case of tpck_integer_pack \n");
#endif
               SgExpression* expr = convert_constant(e->variant.template_param.variant.bound);
               ROSE_ASSERT(expr != NULL);

            // result = expr;
               result = set_file_info_for_enumerator(expr,e);

            // ROSE_ASSERT(!"case of tpck_integer_pack \n");

               break;
             }

#if ROSE_EDG_MAJOR_VERSION_NUMBER >= 7
          case tpck_dependent_constant:
             {
            // EDG 7.0: a non-dependent constant wrapped so that it is treated as template-dependent
            // (seen in libstdc++'s <tr1/memory> with GCC 11). Convert the wrapped constant.
               a_constant_ptr wrapped = e->variant.template_param.variant.constant;
               ROSE_ASSERT(wrapped != NULL);
               result = convert_constant(wrapped);
               ROSE_ASSERT(result != NULL);
               result = set_file_info_for_corresp(result,e->source_corresp);
               break;
             }

          case tpck_concat_string_literals:
             {
            // EDG 7.0: concatenation of string literals of which at least one is dependent. ROSE has no
            // node for the concatenation; join the pieces that are plain string literals.
               std::string joined;
               bool all_strings = true;
               SgExpression* first = NULL;
               for (a_constant_ptr piece = e->variant.template_param.variant.string_literal_list; piece != NULL; piece = piece->next)
                  {
                    SgExpression* piece_expr = convert_constant(piece);
                    ROSE_ASSERT(piece_expr != NULL);
                    if (first == NULL) first = piece_expr;
                    if (SgStringVal* sv = isSgStringVal(piece_expr))
                         joined += sv->get_value();
                    else
                         all_strings = false;
                  }
               ROSE_ASSERT(first != NULL);
               if (all_strings)
                    result = buildStringVal_nfi(joined);
               else
                  {
                    printf ("WARNING: In convert_constant_template_parameter(): tpck_concat_string_literals with a non-literal piece; using the first piece \n");
                    result = first;
                  }
               result = set_file_info_for_corresp(result,e->source_corresp);
               break;
             }

          case tpck_datasizeof:
             {
            // EDG 7.0: __datasizeof (size without tail padding). ROSE has no such operator; represent it as
            // sizeof of the same type or expression (the only difference is for types with tail padding).
               if (e->variant.template_param.variant.templ_sizeof.type != NULL)
                  {
                    SgType* type = convert_type(e->variant.template_param.variant.templ_sizeof.type);
                    ROSE_ASSERT(type != NULL);
                    result = set_file_info_for_corresp(buildSizeOfOp_nfi(type),e->source_corresp);
                  }
                 else
                  {
                    SgExpression* expr = convert_expression(e->variant.template_param.variant.templ_sizeof.expr);
                    ROSE_ASSERT(expr != NULL);
                    result = set_file_info_for_corresp(buildSizeOfOp_nfi(expr),e->source_corresp);
                  }
               break;
             }
#endif

          default:
             {
               printf ("Error: default reached template_parameter_kind = %d \n",template_parameter_kind);
               ROSE_ASSERT(false);
             }      
        }

     ROSE_ASSERT(result != NULL);

  // DQ (7/24/2012): Added test. This will be set later when the parent if built (bottom up; so we can't test this here).
  // ROSE_ASSERT(result->get_parent() != NULL);

#if 0
     printf ("In convert_constant_template_parameter(): result = %p = %s \n",result,result->class_name().c_str());
#endif

     return result;
   }
