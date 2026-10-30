
#include "edgRose.h"

namespace
{
  template <class Map>
  typename Map::mapped_type
  lookup(const Map& m, const typename Map::key_type& key, typename Map::mapped_type alt)
  {
    auto const pos = m.find(key);
    if (pos != m.end())
      return pos->second;

    return alt;
  }

  template <class Map>
  void
  recordIfNotNull(Map& m, const typename Map::key_type& key, typename Map::mapped_type val)
  {
    if (val == nullptr)
      return;

    /*auto res=*/ m.emplace(key, val);
  }
}

std::tuple<SgType*, SgType*>
EDG_ROSE_Translation::convert_type2(a_type_ptr p, bool withNondeducedTypes, bool pointerToMember, bool from_template_parameter )
   {
  // DQ (12/18/2011): Added additional parameter to force functions to be handled as member function when they are a pointer to member type.

  // DQ (8/12/2012): Added test.
     ASSERT_not_null(p);

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("convert_type(): %stype (kind = %s pointerToMember = %s p = %p from_template_parameter = %s) ",
          (p == NULL ? "null " : ""), (p == NULL ? "null" : output_type_enum(p->kind).c_str()),
          pointerToMember ? "true" : "false", p, from_template_parameter ? "true" : "false");
     print_source_corresp(p->source_corresp);
     debug_print("\n");
#endif

#if 0
     printf ("In convert_type(): p = %p pointerToMember = %s from_template_parameter = %s \n",p,pointerToMember ? "true" : "false",from_template_parameter ? "true" : "false");
#endif
#if 0
     printf ("In convert_type(): TOP: p = %p type_translation_cache.find(p) != type_translation_cache.end() = %s \n",
          p,type_translation_cache.find(p) != type_translation_cache.end() ? "true" : "false");
#endif

#if 0
     printf ("In convert_type(): p = %p \n",p);
#if 0
     summarize_type(p);
#endif
#endif

  // DQ (8/12/2012): I don't like to return NULL pointers (unclear semantic significance).
  // if (p == NULL) return NULL;

  // DQ (9/9/2012): Test the maps that we use internally (especially the new tests on the type_translation_cache).
     testEdgRoseMaps();

  // DQ (8/12/2012): We can share ROSE SgTypes so if we detect that we have built this previously then return a pointer to it.
  // This is a possibly significant change to the existing type handling.
     SgType* previouslyBuiltType = NULL;
     auto const type_translation_cache_pos = type_translation_cache.find(p);
     if (type_translation_cache_pos != type_translation_cache.end())
        {
          previouslyBuiltType = type_translation_cache_pos->second;
          ROSE_ASSERT(previouslyBuiltType != NULL);
#if 0
          printf ("############ In convert_type(): Found a previously generated type, so use this type (might be a problem if the type had side-effects of processing declarations) p = %p previouslyBuiltType = %p = %s \n",
               p,previouslyBuiltType,previouslyBuiltType->class_name().c_str());
#endif
        }
       else
        {
          if (template_type_translation_cache.find(p) != template_type_translation_cache.end())
             {
               previouslyBuiltType = template_type_translation_cache[p];
               ROSE_ASSERT(previouslyBuiltType != NULL);
#if 0
               printf ("############ In convert_type(): Found a previously generated template type, so use this type (might be a problem if the type had side-effects of processing declarations) p = %p previouslyBuiltType = %p = %s \n",
                    p,previouslyBuiltType,previouslyBuiltType->class_name().c_str());
#endif
             }
        }

  // DQ (8/12/2012): I don't like this construction of building a NULL pointer into the type_translation_cache
  // at index p and then filling it in later.  This results in code that is hard to test (e.g. in recursive calls).
  // Instead build this as a NULL pointer and enforce that it is non-NULL at the end of the function.  Then put it
  // into the type_translation_cache map at as early an opportunity as possible.
     SgType* result                 = previouslyBuiltType;
     SgType* resultWithoutDeduction = lookup(typesWithoutDeduction, p, nullptr);

  // DQ (1/2/2012): Refactored code to make sure that the generation of the name used in the function is computed once.
  // For template instantiations the name will be modified to carry the template arguments.
     SgName name = p->source_corresp.name;

#if 0
     printf ("In convert_type(): p = %p result (from type_translation_cache) = %p kind = %s \n",p,result,summarize_type_kind_in_string_kind(p->kind));
#endif

#if 0
  // DQ (4/21/2018): Looking for tyes declared in function parameters.
     if (p->declared_in_function_prototype == true)
        {
       // These are types that we need to associate with SgDeclarationScope IR nodes infunction prototypes.
          printf ("@@@@@@@@@@@@@ In convert_type(): p = %p name = %s Found a type declared_in_function_prototype == true \n",p,name.str());

          printf ("   --- internal_intermediate_scope = %p \n",internal_intermediate_scope);
          if (internal_intermediate_scope != NULL)
               printf ("   --- internal_intermediate_scope = %p = %s \n",internal_intermediate_scope,internal_intermediate_scope->class_name().c_str());
             }

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
#endif

#if 0
     printf ("In convert_type() (top): Calling summarize_type() \n");
     summarize_type(p);
#endif

    // DQ (9/28/2012): Added support for builtin_va_list.
       if (p->is_builtin_va_list == true || p->is_builtin_va_list_from_cstdarg == true)
          {
         // Liao 10/26/2012, since SKIP_RETURNING_PREVIOUSLY_BUILT_TYPE is fixed to be 1. We may encounter the same builtin_val_list type more than once.
            if (type_translation_cache.find(p) != type_translation_cache.end())
             {
#ifdef DEBUG_CONVERSION
               printf ("Warning: found builtin_va_list which is already in type_translation_cache! \n");
#endif
            // ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());
               ROSE_ASSERT(result == previouslyBuiltType);
               ROSE_ASSERT(result != NULL);
               return { result, resultWithoutDeduction };
             }
            else
             {
               string type_name = p->source_corresp.name;
#if 0
               printf ("In convert_type(): Calling buildTypedefDeclaration(): base type type_name = %s \n",type_name.c_str());
#endif
               SgType* base_type = new SgTypeDefault();
               SgScopeStatement* scope = getGlobalScopeFromScopeStack();
               SgTypedefDeclaration* typedefDeclaration_for_builtin_va_list = buildTypedefDeclaration_nfi(name,base_type,scope);
               ROSE_ASSERT(typedefDeclaration_for_builtin_va_list != NULL);
               ROSE_ASSERT(typedefDeclaration_for_builtin_va_list->get_type() != NULL);

               type_translation_cache[p] = typedefDeclaration_for_builtin_va_list->get_type();
               ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
               ROSE_ASSERT(type_translation_cache[p] != NULL);

               return { typedefDeclaration_for_builtin_va_list->get_type(),
                        resultWithoutDeduction
                      };
             }
#if 0
          printf ("Found a builtin_va_list or maybe one from cstdarg \n");
          ROSE_ASSERT(false);
#endif
        }

  // DQ (12/12/2011): Fixed type ";" at end of if statement (statement is meaningless, so remove it).
  // DQ (11/29/2011): Added to support reuse of types (not previously present).
  // if (result == NULL);

        {
#ifdef DEBUG_CONVERSION
          ScopedIndent si(indent);
#endif
          a_type_kind kind = p->kind;

          if (result != NULL)
             {
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("already translated as SgType* = %p = %s\n",result,result->class_name().c_str());
#endif
            // If it was already processed then check that it is in the type cache...
#if 0
               printf ("Checking the type_translation_cache using p = %p result = %p = %s (goto end) \n",p,result,result->class_name().c_str());
#endif
               ROSE_ASSERT(
                   ( type_translation_cache.find(p) != type_translation_cache.end() && type_translation_cache[p] != NULL ) ||
                   ( template_type_translation_cache.find(p) != template_type_translation_cache.end() && template_type_translation_cache[p] != NULL )
               );

            // DQ (2/13/2016): However, if this is a SgArrayType that was marked as a VLA and not
            // processed as part of the defining function where it appeared (e.g. in a function prototype),
            // then we need to process it again.
            // goto end;
               SgArrayType* arrayType = isSgArrayType(result);
#if 0
               printf ("arrayType = %p \n",arrayType);
               if (arrayType != NULL)
                  {
                    printf ("   --- arrayType->get_is_variable_length_array() = %s \n",arrayType->get_is_variable_length_array() ? "true" : "false");
                    printf ("   --- arrayType->get_index()                    = %p = %s \n",arrayType->get_index(),arrayType->get_index()->class_name().c_str());

                    SgArrayType* baseTypeArrayType = isSgArrayType(arrayType->get_base_type());
                    if (baseTypeArrayType != NULL)
                       {
                         printf ("   --- --- arrayType->get_base_type()                = %p = %s \n",baseTypeArrayType,baseTypeArrayType->class_name().c_str());
                         printf ("   --- --- arrayType->get_is_variable_length_array() = %s \n",baseTypeArrayType->get_is_variable_length_array() ? "true" : "false");
                         printf ("   --- --- arrayType->get_index()                    = %p = %s \n",baseTypeArrayType->get_index(),baseTypeArrayType->get_index()->class_name().c_str());
                       }
                  }
#endif

            // DQ (2/11/2018): Look for the case of a VLA that is part of a multi-dimensional array.
               bool is_vla_array_type = false;
               SgArrayType* tmp_arrayType = arrayType;
               int dimension = 0;
               while (tmp_arrayType != NULL)
                  {
                    if (tmp_arrayType->get_is_variable_length_array() == true)
                       {
#if 0
                         printf ("   --- --- dimension = %d Found VLA inside of array type! \n",dimension);
#endif
                         is_vla_array_type = true;
                       }
                      else
                       {
#if 0
                         printf ("   --- --- dimension = %d Not a VLA dimension for this array type! \n",dimension);
#endif
                       }

                    dimension++;
                    tmp_arrayType = isSgArrayType(tmp_arrayType->get_base_type());
                  }
#if 0
               printf ("In convert_type(): is_vla_array_type = %s \n",is_vla_array_type ? "true" : "false");
#endif
            // The case of isSgNullExpression(arrayType->get_index()) != NULL is for the arrays used in function prototypes to be reset.
            // It might be that we want an new array type to be built each time.  We certainly don't want the array types shared across
            // multiple defining functions parameter lists to be reset.
            // if (arrayType != NULL && arrayType->get_is_variable_length_array() == true && isSgNullExpression(arrayType->get_index()) != NULL)
            // if (arrayType != NULL && arrayType->get_is_variable_length_array() == true)
               if (arrayType != NULL && is_vla_array_type == true)
                  {
#if 0
                    printf ("In convert_type(): found previously built VLA array type that need to have it's demension setup: result = %p = %s \n",result,result->class_name().c_str());
                    printf ("   --- global_innermost_function_scope = %p \n",global_innermost_function_scope);
#endif
                 // ROSE_ASSERT(false);
                  }
                 else
                  {
#if 0
                    printf ("In convert_type(): Jump to the end of the function! \n");
#endif
                    goto end;
                  }
             }

          {
            bool interpret_as_nonreal_type = false;
            bool use_orig_nested_type = false;
            if (kind == tk_class || kind == tk_struct || kind == tk_union) {
              interpret_as_nonreal_type = p->variant.class_struct_union.is_nonreal_class;
              interpret_as_nonreal_type &= from_template_parameter || p->variant.class_struct_union.is_template_class || p->variant.class_struct_union.proxy_class;
              interpret_as_nonreal_type &= !p->variant.class_struct_union.is_prototype_instantiation;
            } else if (kind == tk_template_param) {
              a_template_param_type_supplement_ptr extra_info = p->variant.template_param.extra_info;
              use_orig_nested_type = extra_info != NULL && extra_info->orig_nested_type != NULL;
              interpret_as_nonreal_type = !use_orig_nested_type;
            }
            if (interpret_as_nonreal_type) {
#if 0
              printf("In convert_type(p=%p): interpreted as a non-real type\n", p);
#  if 0
              summarize_type(p);
#  endif
#endif
              result = convert_nonreal_type(p, pointerToMember, from_template_parameter, NULL);

#if 0
              printf ("In convert_type(): interpret_as_nonreal_type == true: Jump to the end of the function! result = %p = %s \n",result,result->class_name().c_str());
#endif
              goto end;
            } else if (use_orig_nested_type) {
#if 0
              printf("In convert_type(p=%p): using original nested type for translation\n", p);
#  if 0
              summarize_type(p);
#  endif
#endif
              // PP (9/24/25): \todo call convert_type2?
              result = convert_type(p->variant.template_param.extra_info->orig_nested_type, pointerToMember, from_template_parameter);
              type_translation_cache[p] = result;
#if 0
              printf ("In convert_type(): use_orig_nested_type == true: Jump to the end of the function! result = %p = %s \n",result,result->class_name().c_str());
#endif
              goto end;
            } else {
#if 0
              printf("In convert_type(p=%p): NOT interpreted as a non-real type\n", p);
#  if 0
              summarize_type(p);
#  endif
#endif
            }
          }

#if 0
          printf ("################## In convert_type(): kind = %s \n",summarize_type_kind_in_string(p));
#endif
          switch (kind)
             {
            // Handle the primative types directly (this is simple since they are shared internally).
               case tk_void:
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("void\n");
#endif
                    result = buildVoidType();
                    break;

               case tk_integer:
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("integer\n");
#endif
                    result = convert_integer_type(p);
                    break;

               case tk_float:
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("float\n");
#endif
                    result = convert_float_type(p);
                    break;

               case tk_imaginary:
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("imaginary\n");
#endif
                    result = convert_float_type(p);
                    break;

               case tk_complex:
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("complex\n");
#endif
                    result = convert_float_type(p);
                    break;

            // Pointer types are shared, but modifiers are significant.
               case tk_pointer:
                  {
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("build base type for: %s\n", (p->variant.pointer.is_reference ? "reference" : "pointer"));
#endif
                    SgType* base_type = nullptr;
                    SgType* base_type_undeduced = nullptr;

                    std::tie(base_type, base_type_undeduced) = convert_type2(p->variant.pointer.type, withNondeducedTypes, false, false);

                 // indent_line(); debug_print("DONE: convert_type()\n");

                    if (p->variant.pointer.is_reference)
                       {
                      // result = buildReferenceType(base_type);
                         if (p->variant.pointer.is_rvalue_reference)
                            {
#if 0
                              printf ("In convert_type(): case tk_pointer: Need to build Rvalue reference type \n");
#endif
                              result = buildRvalueReferenceType(base_type);
                              resultWithoutDeduction = base_type_undeduced ? buildRvalueReferenceType(base_type_undeduced) : nullptr;
                            }
                           else
                            {
                              result = buildReferenceType(base_type);
                              resultWithoutDeduction = base_type_undeduced ? buildReferenceType(base_type_undeduced) : nullptr;
                            }
                       }
                      else
                       {
                      // indent_line(); debug_print("Calling buildPointerType()\n");
                         result = buildPointerType(base_type);
                         resultWithoutDeduction = base_type_undeduced ? buildPointerType(base_type_undeduced) : nullptr;
                      // indent_line(); debug_print("DONE: Calling buildPointerType()\n");
                       }
                    break;
                  }

            // Function types will be shared where function can use the same function type (though modifiers make this significant, like pointers)
               case tk_routine:
                  {
                 // Note that this is not a SgFunctionType (or SgMemberFunctionType), but the return type of the function.
                 // Since we don't at this point have information to know if this is a member function or now, we can't
                 // know to generate a SgFunctionType or a SgMemberFunctionType, so we need to just return the return type
                 // directly and let the calling function (which has access to the routine pointer) figure it out.

                 // Use the SgPartialFunctionType to support this operation, and then construct either a SgFunctionType or
                 // a SgMemberFunctionType later (in the calling routine).

                 // Type of this routine.  Points to a type entry with kind == tk_routine (or to a typeref
                 // that refers to such a type), which gives the return type and parameter information.

#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("routine type\n");
#endif
#if 0
                    printf ("In convert_type(): case tk_routine: calling convert_function_type() \n");
#endif
                 // DQ (1/16/2009): Refactored code to localize conversion of EDG function type to ROSE function types.
                 // result = convert_function_type(p); // p->variant.typeref.type
                    std::tie(result, resultWithoutDeduction) = convert_function_type(p, withNondeducedTypes, pointerToMember);

                 // DQ (12/17/2011): This does not appear to be used.
                 // a_routine_type_supplement_ptr sup = p->variant.routine.extra_info;
                 // ROSE_ASSERT(sup != NULL);

                 // Are these preserved across the translation of SgPartialFunctionType to either SgFunctionType or a SgMemberFunctionType?
                 // DQ (11/6/2011): We don't use SgModifiers on the function type to support const functions.  Such information is held in
                 // the function declaration modifiers.
                 // result = apply_qualifiers(result, sup->qualifiers, UPC_BLOCK_SIZE_NONE);
#if 0
                    printf ("In convert_type(): case tk_routine: DONE: calling convert_function_type(): result = %p = %s \n",result,result->class_name().c_str());
#endif
                 // indent_line(); debug_print("Leaving convert_type: case tk_routine\n");
                    ROSE_ASSERT (result != NULL);
                    break;
                  }

               case tk_array:
                  {
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("array type\n");
#endif
                 // DQ (2/12/2016): Adding support for variable length arrays.
                    bool is_variable_length_array = false;
#if 0
                    printf ("In convert_type(): case tk_array: Build base type for array: p = %p \n",p);
#endif
#if 0
                 // DQ (3/2/2017): This is part of testing to debug ROSE Matrix testing (test2017_04.C).
                    printf ("In convert_type(): case tk_array: p->variant.array.is_template_dependent_size_array = %s \n",p->variant.array.is_template_dependent_size_array ? "true" : "false");
#endif
                    SgType* base_type = convert_type(p->variant.array.element_type);
#if 0
                    printf ("In convert_type(): case tk_array: base_type = %p = %s \n",base_type,base_type->class_name().c_str());
#endif
                    SgExpression* size = NULL;
                    if (p->variant.array.is_template_dependent_size_array)
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("- size is template-dependent\n");
#endif
#if 0
                         printf ("In convert_type(): case tk_array: p->variant.array.is_template_dependent_size_array == true: p->variant.array.variant.element_count_constant = %p \n",p->variant.array.variant.element_count_constant);
                         printf ("In convert_type(): case tk_array: p->variant.array.is_template_dependent_size_array == true: p->variant.array.is_variable_size_array = %s \n",p->variant.array.is_variable_size_array ? "true" : "false");
#endif
                         if (p->variant.array.variant.element_count_constant != NULL)
                            {
                              size = convert_constant(p->variant.array.variant.element_count_constant);
                            }
                           else
                            {
                           // NULL to indicate that the number of elements is template-dependent but we know nothing about its value.
                           // Since this is part of a template it will not presently be unparsed, until we unparse templates directly.
                              size = buildIntVal_nfi(0,"");
                            }
                       }
                      else
                       {
                         if (p->variant.array.is_variable_size_array)
                            {
#ifdef DEBUG_CONVERSION
                              indent_line(); debug_print("- size is variable\n");
#endif
                              is_variable_length_array = true;
#if 0
                              printf ("@@@@@@@@@@@@@@@@@@@@@@@@ Note: we may be reprocessing a previously built array type that was a VLA with for which we could not compute the dimention info: result = %p \n",result);
#endif
#if 0
                              printf ("In convert_type(): case tk_array: Calling summarize_type() \n");
                              summarize_type(p);
#endif
#if 0
                              printf ("p->variant.array.variant.element_count_expr = %p \n",p->variant.array.variant.element_count_expr);
#endif
                           // DQ (10/7/2012): Note that for VLAs the index expression is stored in the function scope (and p->variant.array.variant.element_count_expr == NULL).
                              if (p->variant.array.variant.element_count_expr == NULL)
                                 {
                                // DQ (2/11/2016): This is not working.
                                // a_vla_dimension_ptr find_vla_dimension_in_current_function(a_type_ptr  array_type)
                                // a_vla_dimension_ptr  vla_dim = find_vla_dimension(p);
                                // a_vla_dimension_ptr  vla_dim = find_vla_dimension_in_current_function(p);
                                // ROSE_ASSERT(vla_dim != NULL);
#if 0
                                   printf ("@@@@@@@@@@@@@@@@@@@@@@@@ In convert_type(): case tk_array: global_innermost_function_scope = %p \n",global_innermost_function_scope);
#endif
                                // ROSE_ASSERT(global_innermost_function_scope != NULL);
                                   if (global_innermost_function_scope != NULL)
                                      {
#if 0
                                        printf ("In convert_type(): case tk_array: global_innermost_function_scope->vla_dimensions = %p \n",global_innermost_function_scope->vla_dimensions);
#endif
#if 0
                                        printf ("In convert_type(): case tk_array: global_innermost_function_scope: Calling summarize_scope() \n");
                                        summarize_scope(global_innermost_function_scope);
#endif
#if 0
                                        printf ("In convert_type(): case tk_array: p->variant.array.is_variable_size_array == true: Calling summarize_type() \n");
                                        summarize_type(p);
#endif
                                        a_vla_dimension_ptr vla_entry = global_innermost_function_scope->vla_dimensions;
#if 0
                                     // DQ (1/22/2018): Adding warning for vla in function prototypes.
                                        if (vla_entry == NULL)
                                           {
                                             printf ("WARNING: In convert_type(): global_innermost_function_scope->vla_dimensions == NULL \n");
                                           }
                                     // ROSE_ASSERT(vla_entry != NULL);
#endif
                                     // printf ("In convert_type(): p = %p \n",p);
                                     // printf ("In convert_type(): detected vla edg_scope->vla_dimensions = %p \n",vla_entry);

                                        bool found_type_in_vla_list = false;
                                        while (vla_entry != NULL)
                                           {
#if 0
                                             printf ("In convert_type(): case tk_array: p->variant.array.is_variable_size_array == true: vla_entry != NULL: Calling summarize_vla_dimension() \n");
                                             summarize_vla_dimension(vla_entry);
#endif
                                          // Note that we might have to lookup the associated SgArrayType in the type maps (e.g. for multi-dimensional arrays buried in an outer SgArrayType).
                                             ROSE_ASSERT(vla_entry->type != NULL);
#if 0
                                             printf ("In convert_type(): p = %p vla_entry->type = %p \n",p,vla_entry->type);
                                             printf ("   --- p               = %s \n",summarize_type_kind_in_string(p));
                                             printf ("   --- vla_entry->type = %s \n",summarize_type_kind_in_string(vla_entry->type));
#endif
#ifdef DEBUG_CONVERSION
                                             if (p != vla_entry->type)
                                                {
                                                  printf ("Note: (p == vla_entry->type) == false in VLA dimension processing (previously only true cases were handled) \n");
                                                }
#endif
                                          // DQ (2/16/2016): This is what makes sure that the variables used in multiple
                                          // dimensions are assigned to each dimension correctly (see test2016_14.c).
                                          // DQ (2/14/2016): I don't think we need or want this conditional predicate.
                                          // if (p == vla_entry->type)
                                             if (p == vla_entry->type)
                                                {
#if 0
                                                  printf ("p == vla_entry->type: vla_entry->dimension_expr = %p \n",vla_entry->dimension_expr);
                                                  printf ("     ---- vla: original_dimension = %p \n",vla_entry->original_dimension);
                                                  printf ("     ---- vla: in_prototype_scope = %s \n",vla_entry->in_prototype_scope ? "true" : "false");
#endif
#if 0
                                                  printf ("In convert_type(): case tk_array: p->variant.array.is_variable_size_array == true: vla_entry != NULL: p == vla_entry->type: Calling summarize_vla_dimension() \n");
                                                  summarize_vla_dimension(vla_entry);
#endif
                                                  found_type_in_vla_list = true;
                                                  SgExpression* arrayDimensionExpression = convert_expression(vla_entry->dimension_expr);
                                                  ROSE_ASSERT(arrayDimensionExpression != NULL);

                                                  size = arrayDimensionExpression;
                                                }

                                             vla_entry = vla_entry->next;
                                           }

                                     // DQ (2/16/2016): Not all VLA will have an entry in the scope (e.g. VLA types in the function parameter list of a function prototype).
                                        if (found_type_in_vla_list == false)
                                           {
                                          // This is for a VLA type in a function prototype so there was no vla dimension entry for this in the scope.
#if 0
                                             printf ("In convert_type(): case tk_array: global_innermost_function_scope: error: found_type_in_vla_list == false: Calling summarize_scope() \n");
                                             summarize_scope(global_innermost_function_scope);
#endif
                                             ROSE_ASSERT(size == NULL);
                                             size = set_generated_file_info_to_unparse(buildNullExpression_nfi());
                                             found_type_in_vla_list = true;
                                           }
                                        ROSE_ASSERT(found_type_in_vla_list == true);

                                        ROSE_ASSERT(size != NULL);
#if 0
                                        printf ("In convert_type(): found VLA dimension: size = %p = %s = %s \n",size,size->class_name().c_str(),SageInterface::get_name(size).c_str());
#endif
#if 0
                                        printf ("Exiting as a test! \n");
                                        ROSE_ASSERT(false);
#endif
                                      }
                                     else
                                      {
                                     // DQ (2/11/2016): If this is a VLA and has_assoc_vla_dimension == false then the dimention is not defined and should be canonicalized to "*".
                                        if (p->variant.array.has_assoc_vla_dimension == false)
                                           {
                                          // The array dimention size should be represented by "*" in this case.
#ifdef DEBUG_CONVERSION
                                             indent_line(); debug_print("- case of VLA size is * (p->variant.array.has_assoc_vla_dimension == false) \n");
#endif
#if 0
                                             printf ("In convert_type() (p->variant.array.is_variable_size_array == true && p->variant.array.variant.element_count_expr == NULL && p->variant.array.has_assoc_vla_dimension == false): Calling summarize_type() \n");
                                             summarize_type(p);
#endif
                                             size = set_generated_file_info_to_unparse(buildNullExpression_nfi());
                                           }
                                          else
                                           {
#ifdef DEBUG_CONVERSION
                                             indent_line(); debug_print("- case of VLA size is * (p->variant.array.has_assoc_vla_dimension == true) \n");
#endif
#if 0
                                             printf ("In convert_type() (p->variant.array.is_variable_size_array == true && p->variant.array.variant.element_count_expr == NULL && p->variant.array.has_assoc_vla_dimension == true): Calling summarize_type() \n");
                                             summarize_type(p);
#endif
                                             size = set_generated_file_info_to_unparse(buildNullExpression_nfi());
                                           }

                                        ROSE_ASSERT(size != NULL);
                                      }
                                 }
                                else
                                 {
                                   ROSE_ASSERT(p->variant.array.variant.element_count_expr != NULL);
                                   size = convert_expression(p->variant.array.variant.element_count_expr);
                                 }
#if 0
                              printf ("In convert_type(): case tk_array: array size is defined as: size = %p = %s \n",size,size->class_name().c_str());
#endif
                            }
                           else
                            {
                              if (p->variant.array.variant.number_of_elements == 0 && !p->variant.array.bound_is_zero)
                                 {
                                // DQ (6/11/2013): Note that this code is visited for "X *x5 = new X[i+j];" now that EDG 4.5 has
                                // changed where the arrsy size expression is located (now in the a_new_delete_supplement type).
#ifdef DEBUG_CONVERSION
                                   indent_line(); debug_print("- size is blank\n");
#endif
#if 0
                                   printf ("In convert_type() (p->variant.array.variant.number_of_elements == 0 && !p->variant.array.bound_is_zero): Calling summarize_type() \n");
                                   summarize_type(p);
#endif
                                   size = set_generated_file_info_to_unparse(buildNullExpression_nfi());
                                 }
                                else
                                 {
#ifdef DEBUG_CONVERSION
                                   indent_line(); debug_print("- size is known\n");
#endif
#if 0
                                   printf ("In convert_type(): case tk_array: (p->variant.array.variant.number_of_elements == 0 && !p->variant.array.bound_is_zero) == false: Calling summarize_type() \n");
                                   summarize_type(p);
#endif
                                   if (p->variant.array.bound_constant == NULL)
                                      {
#ifdef DEBUG_CONVERSION
                                        printf ("Note that p->variant.array.bound_constant == NULL, this is OK (convert_constant() will build a SgNullExpression) \n");
#endif
#if 0
                                        printf ("In convert_type(): case tk_array: p->variant.array.bound_constant == NULL: Calling summarize_type() \n");
                                        summarize_type(p);
#endif
                                      }

                                   if (p->variant.array.bound_constant == NULL)
                                      {
                                     // I think that p->variant.array.variant.number_of_elements should be greater than zero, but not certain.

                                     // DQ (9/28/2022): Remove this compiler warning (comparison of unsigned expression >= 0 is always true).
                                     // ROSE_ASSERT(p->variant.array.variant.number_of_elements >= 0);

                                     // This should be an SgUnsignedLongVal
                                        size = set_generated_file_info_to_unparse(buildUnsignedLongVal_nfi(p->variant.array.variant.number_of_elements,""));
                                      }
                                     else
                                      {
                                        size = convert_constant(p->variant.array.bound_constant);
                                      }
                                 }
                            }
                       }

                    ROSE_ASSERT (size != NULL);
                    SgArrayType * arrayType = buildArrayType(base_type, size);
                    arrayType->set_number_of_elements(p->variant.array.variant.number_of_elements);
                    result = apply_qualifiers(arrayType, p->variant.array.qualifiers, UPC_BLOCK_SIZE_NONE);

                 // DQ (2/12/2016): Adding support for variable length arrays.
                    if (is_variable_length_array == true)
                       {
                         ROSE_ASSERT(arrayType != NULL);
                         arrayType->set_is_variable_length_array(true);
#if 0
                         printf ("In convert_type(): case tk_array: result is marked as a VLA \n");
#endif
                       }
#if 0
                    printf ("In convert_type(): case tk_array: p = %p result = %p = %s \n",p,result,result->class_name().c_str());
#endif
                 // DQ (7/25/2012): Enforce this if possible.
                    ROSE_ASSERT(size->get_parent() != NULL);

                    break;
                  }

            // DQ (1/11/2009): These declarations need not have been defined before they are used (we just need to build nondefining declarations for them)
               case tk_class:
                  {
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("class needing declaration\n");
#endif
                 // if (!result) ROSE_ASSERT (!"Should already have been translated");

#if 0
                    printf("In convert_type(): tk_class: abstract = %s\n", p->variant.class_struct_union.abstract ? "true" : "false");
#endif
#if 0
                    printf("In convert_type(): tk_class: is_nonreal_class = %s\n", p->variant.class_struct_union.is_nonreal_class ? "true" : "false");
#endif
#if 0
                    printf ("In convert_type(): tk_class: TOP: type_translation_cache.find(p) != type_translation_cache.end() = %s \n",
                         type_translation_cache.find(p) != type_translation_cache.end() ? "true" : "false");
#endif
#if 0
                    printf ("In convert_type(): tk_class: Calling summarize_type() \n");
                    summarize_type(p);
#endif
#if 0
                    printf ("In convert_type(): case tk_class: p->variant.class_struct_union.is_template_class = %s \n",p->variant.class_struct_union.is_template_class ? "true" : "false");
#endif
                    bool buildTemplateDeclaration   = false;
                    bool buildTemplateInstantiation = false;
                    bool forceTemplateDeclaration   = false;
                    setTemplateOrTemplateInstantiationClassGeneration(p,forceTemplateDeclaration,buildTemplateDeclaration,buildTemplateInstantiation);
#if 0
                    printf ("In convert_type(): buildTemplateDeclaration   = %s \n",buildTemplateDeclaration ? "true" : "false");
                    printf ("In convert_type(): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
#endif

                 // DQ (5/10/2012): We can not assert this (see test2005_12.C).
                 // DQ (5/8/2012): We want to detect if this can be asserted (testing).
                 // ROSE_ASSERT(buildTemplateDeclaration == false);
#if 0
                 // DQ (12/21/2011): We don't want to handle this by forcing declarations to be template declarations (see test2011_198.C).
                    if (templateDeclarationStack.empty() == false)
                       {
                      // If we are procesing templates then this should be a template declaration (and not an instantiation).
                         buildTemplateDeclaration   = true;
                         buildTemplateInstantiation = false;

                         ROSE_ASSERT(buildTemplateDeclaration   == true);
                         ROSE_ASSERT(buildTemplateInstantiation == false);
                       }
                    if (buildTemplateInstantiation == true)
                       {
                         ROSE_ASSERT(buildTemplateDeclaration == false);
                       }
                    if (buildTemplateDeclaration == true)
                       {
                         ROSE_ASSERT(buildTemplateInstantiation == false);
                       }
#endif
#if 0
                    printf ("In convert_type(): (reset) buildTemplateDeclaration   = %s \n",buildTemplateDeclaration ? "true" : "false");
                    printf ("In convert_type(): (reset) buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
#endif
                 // DQ (12/27/2011): Even when we see a type in variable declaration, we may not have seen the primary
                 // declaration and yet still have to build it (e.g. in the case of a template instantiation). So
                 // building the seconary is not enough.
                 // SgDeclarationStatement* tempDecl = convert_struct_secondary(p, SgClassDeclaration::e_class); // Build first nondefining declaration for type references to point to.

                 // Should this be refactored to apply equally to classes, structs, and unions?
                 // SgDeclarationStatement* tempDecl = NULL;
                    SgClassDeclaration* tempDecl = NULL;

                 // DQ (1/1/2012): This should be clearer code.
                 // if (p->variant.class_struct_union.is_template_class == true)
                    if (buildTemplateDeclaration == true || buildTemplateInstantiation == true)
                       {
                      // This should be built as a template class instantiation (this is the secondary (non-defining declaration).
                      // In the case of a template instantiation we always want to build the associated defining template instantiation
                      // as well (if the template declaration is available).
#if 0
                         printf ("In convert_type(): tk_class: (buildTemplateDeclaration == true || buildTemplateInstantiation == true): Calling summarize_type() \n");
                         summarize_type(p);
#endif
#if 0
                         printf ("In convert_type(): tk_class: calling convert_struct_secondary(p=%p) \n",p);
#endif
                      // This returns either a SgTemplateClassDeclaration or a SgTemplateInstantiationDecl.
                      // Build first nondefining declaration for type references to point to.

                      // DQ (9/28/2022): Remove this compiler warning (unused parameter).
                      // SgDeclarationStatement * decl_stmt = convert_struct_secondary(p, SgClassDeclaration::e_class, from_template_parameter);
                         SgDeclarationStatement * decl_stmt = convert_struct_secondary(p, SgClassDeclaration::e_class);
                         ROSE_ASSERT(decl_stmt != NULL);
                         SgNonrealDecl * nrdecl = isSgNonrealDecl(decl_stmt);
                         if (nrdecl != NULL) {
                           result = nrdecl->get_type();
                           ROSE_ASSERT(result != NULL);
#if 0
                        // DQ (12/20/2019): Debugging Cxx11_tests/test2019_505.C
                           printf ("Call to convert_struct_secondary() returned SgNonrealDecl: nrdecl = %p nrdecl->get_type() = %p \n",nrdecl,result);
#endif
                           break;
                         }
                         tempDecl = isSgClassDeclaration(decl_stmt);
                         ROSE_ASSERT(tempDecl != NULL);
#if 0
                         printf ("DONE: In convert_type(): tk_class: calling convert_struct_secondary(p=%p) \n",p);
                         SgScopeStatement* tmp_scope = tempDecl->get_scope();
                         printf ("In convert_type(): tk_class: tempDecl->get_scope() = %p = %s \n",tmp_scope,tmp_scope->class_name().c_str());
#endif
                      // DQ (9/8/2012): I think this should be NULL.
                      // ROSE_ASSERT(tempDecl->get_definingDeclaration() == NULL);
                         if (tempDecl->get_definingDeclaration() == NULL)
                            {
#ifdef DEBUG_CONVERSION
                              printf ("WARNING: we might be building a 2nd defining declaration below, this would be a problem! \n");
#endif
                            }

                      // DQ (12/27/2011): Added new assertion.
                      // ROSE_ASSERT(buildTemplateDeclaration ? tempDecl->get_scope()->lookup_template_class_symbol(p->source_corresp.name) != NULL : tempDecl->get_scope()->lookup_nontemplate_class_symbol(p->source_corresp.name) != NULL );
                      // ROSE_ASSERT(buildTemplateDeclaration ? tempDecl->get_scope()->lookup_template_class_symbol(name) != NULL : tempDecl->get_scope()->lookup_nontemplate_class_symbol(name) != NULL );
                      // ROSE_ASSERT(buildTemplateDeclaration ? tempDecl->get_scope()->lookup_template_class_symbol(tempDecl->get_name()) != NULL : tempDecl->get_scope()->lookup_nontemplate_class_symbol(tempDecl->get_name()) != NULL );
                      // ROSE_ASSERT(buildTemplateDeclaration ? tempDecl->get_scope()->lookup_template_class_symbol(tempDecl->get_name()) != NULL : tempDecl->get_scope()->lookup_nontemplate_class_symbol(tempDecl->get_name()) != NULL );

                      // DQ (8/22/2012): lookup_nontemplate_class_symbol() needs the template arguments, if they exist (shared between efining and non-defining declarations).
                      // This will allow subtle cases of the same name for a template class to be disambiguated when they differe only in the name qualification of their template arguments.
                         SgTemplateInstantiationDecl* templateClassInstantiation = isSgTemplateInstantiationDecl(tempDecl);
                         SgTemplateArgumentPtrList* templateArgumentsList = NULL;
                         if (templateClassInstantiation != NULL)
                            {
                              templateArgumentsList = &(templateClassInstantiation->get_templateArguments());
                            }

                      // DQ (9/9/2012): Setup the template parameters (required to disambiguate templates when the names are not enough). See test2012_209.C
                         SgTemplateClassDeclaration* templateClassDeclaration = isSgTemplateClassDeclaration(tempDecl);
                         SgTemplateParameterPtrList* templateParameterList = NULL;
                         if (templateClassDeclaration != NULL)
                            {
                              templateParameterList = &(templateClassDeclaration->get_templateParameters());
                            }

                      // DQ (9/11/2012): Adding support for partial specialization.
                         SgTemplateArgumentPtrList* templateSpecializationArgumentList = NULL;
                      // a_template_arg_ptr partial_spec_template_arg_list = p->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
                      // if (templateClassDeclaration != NULL && partial_spec_template_arg_list != NULL)
                         if (templateClassDeclaration != NULL)
                            {
                           // templateSpecializationArgumentList = parse_template_parameters(partial_spec_template_arg_list,scopeOfDeclaration,NULL);
                              templateSpecializationArgumentList = &(templateClassDeclaration->get_templateSpecializationArguments());
                            }

                      // bool nontemplate_class_symbol_found = (buildTemplateDeclaration == false) ? tempDecl->get_scope()->lookup_nontemplate_class_symbol(tempDecl->get_name(),templateArgumentsList) : false;
                         ROSE_ASSERT(buildTemplateDeclaration ? tempDecl->get_scope()->lookup_template_class_symbol(tempDecl->get_name(),templateParameterList,templateSpecializationArgumentList) != NULL :
                                                                tempDecl->get_scope()->lookup_nontemplate_class_symbol(tempDecl->get_name(),templateArgumentsList) != NULL );

#ifdef DEBUG_CONVERSION
                         printf ("In convert_type(): tk_class: tempDecl = %p = %s \n",tempDecl,tempDecl->class_name().c_str());
#endif
#if 0
                         printf ("In convert_type(): tk_class: buildTemplateDeclaration = %s \n",buildTemplateDeclaration ? "true" : "false");
#endif
                         if (buildTemplateDeclaration == true)
                            {
                           // This is a templated class and it will be seen in the source code so no special handling is required as in
                           // the case of a template instantiation (which is instantiated by the front-end thus not in the source code).

                              SgTemplateClassDeclaration* nondefiningTemplateClassDeclaration = isSgTemplateClassDeclaration(tempDecl);
                              ROSE_ASSERT(nondefiningTemplateClassDeclaration != NULL);

#if 0
                              printf ("In convert_type(): tk_class: not clear what to pass as a EDG template: Calling summarize_type() \n");
                              summarize_type(p);
#endif
                           // DQ (4/8/2012): Unclear what to pass as a EDG template.
                           // SgTemplateClassDeclaration* definingTemplateClassDeclaration = convert_template_struct_primary(p, SgClassDeclaration::e_class,nondefiningTemplateClassDeclaration);
                           // SgTemplateClassDeclaration* definingTemplateClassDeclaration = convert_template_struct_primary(p, SgClassDeclaration::e_class,nondefiningTemplateClassDeclaration,NULL);

                           // DQ (4/27/2012): Added tests.
                              ROSE_ASSERT(p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union);
                              ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);

                           // DQ (8/26/2012): This appears to only have for template type (SgTemplateType) IR nodes uses as template parameters.
                           // Data members referenced have to be assembled into classes that are incomplete classes used to support the consistent
                           // declaration in ROSE (but representative of only the interface requirements for any type that would be used as such
                           // a template parameter).  In this case the pointer p->variant.class_struct_union.extra_info->assoc_template can be NULL.
                           // This should perhaps trigger a flag in the ROSE SgClassDeclaration to indicate that this class only represents a
                           // minimal interface requirement for the use of a type (class, struct, etc.) that would be used as a peramter.
                              if (p->variant.class_struct_union.extra_info->assoc_template == NULL)
                                 {
#if 0
                                   printf ("In convert_type(): p->variant.class_struct_union.extra_info->assoc_template == NULL: Calling summarize_type() \n");
                                   summarize_type(p);
#endif
#ifdef DEBUG_CONVERSION
                                   printf ("MARK THIS CLASS AS A MINIMAL INTERFACE CLASS USED! (needs ROSE IR extension to be supported) \n");
#endif
                                   ROSE_ASSERT(p->variant.class_struct_union.is_nonreal_class == true);

                                   nondefiningTemplateClassDeclaration->set_isRepresentingTemplateParameterInTemplateDeclaration(true);
                                 }
                                else
                                 {
                                   ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template != NULL);
                                   ROSE_ASSERT(p->variant.class_struct_union.is_template_class == true);
                                 }
                           // ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template != NULL);
                           // ROSE_ASSERT(p->variant.class_struct_union.is_template_class == true);

                           // DQ (4/27/2012): Build the edg template paremeter.
                              a_template_ptr edg_template = p->variant.class_struct_union.extra_info->assoc_template;
#if 0
                           // DQ (8/26/2012): This fails for non-real classes that don't have associated templates.
                              ROSE_ASSERT(edg_template != NULL);
#endif
                           // DQ (8/26/2012): This fails for non-real classes that don't have associated templates (fixed to be a conditional).
                              if (edg_template != NULL)
                                 {
                                // DQ (4/27/2012): Don't use just any non-defining declaration, we want the first one.
                                   SgDeclarationStatement* first_nondefiningDeclaration = nondefiningTemplateClassDeclaration->get_firstNondefiningDeclaration();
                                   ROSE_ASSERT(first_nondefiningDeclaration != NULL);
                                   SgTemplateClassDeclaration* first_nondefiningTemplateClassDeclaration = isSgTemplateClassDeclaration(first_nondefiningDeclaration);
                                   ROSE_ASSERT(first_nondefiningTemplateClassDeclaration != NULL);

                                // DQ (9/8/2012): I think these should be NULL (fails for test2005_12.C).
                                   if (first_nondefiningTemplateClassDeclaration->get_definingDeclaration() != NULL)
                                      {
#ifdef DEBUG_CONVERSION
                                         printf ("WARNING: We might be building a second defining declartion for a template declaration: first_nondefiningTemplateClassDeclaration->get_definingDeclaration() = %p \n",first_nondefiningTemplateClassDeclaration->get_definingDeclaration());
                                         first_nondefiningTemplateClassDeclaration->get_definingDeclaration()->get_startOfConstruct()->display("WARNING: We might be building a second defining declartion for a template declaration: debug");
#endif
                                      }
                                // ROSE_ASSERT(first_nondefiningTemplateClassDeclaration->get_definingDeclaration() == NULL);
                                   if (nondefiningTemplateClassDeclaration->get_definingDeclaration() != NULL)
                                      {
#ifdef DEBUG_CONVERSION
                                         printf ("WARNING: We might be building a second defining declartion for a template declaration: nondefiningTemplateClassDeclaration->get_definingDeclaration() = %p \n",nondefiningTemplateClassDeclaration->get_definingDeclaration());
                                         nondefiningTemplateClassDeclaration->get_definingDeclaration()->get_startOfConstruct()->display("WARNING: We might be building a second defining declartion for a template declaration: debug");
#endif
                                      }
                                // ROSE_ASSERT(nondefiningTemplateClassDeclaration->get_definingDeclaration() == NULL);

                                // DQ (4/27/2012): Pass the edg_template paremeter to the convert_template_struct_primary() function.
                                // SgTemplateClassDeclaration* definingTemplateClassDeclaration = convert_template_struct_primary(p, SgClassDeclaration::e_class,nondefiningTemplateClassDeclaration,edg_template);
                                   SgTemplateClassDeclaration* definingTemplateClassDeclaration = convert_template_struct_primary(p, SgClassDeclaration::e_class,first_nondefiningTemplateClassDeclaration,edg_template);
                                   ROSE_ASSERT(definingTemplateClassDeclaration != NULL);

                                // DQ (3/3/2012): Added test for matching variants.
                                   ROSE_ASSERT(matching_defining_and_nondefining_variants(nondefiningTemplateClassDeclaration) == true);
#if 0
                                // DQ (9/9/2012): It appears that the template parameter list is not set properly (frequently zero, at least).
                                   printf ("In convert_type(): first_nondefiningTemplateClassDeclaration->get_templateParameters().size() = %" PRIuPTR " \n",first_nondefiningTemplateClassDeclaration->get_templateParameters().size());
                                   printf ("In convert_type(): definingTemplateClassDeclaration->get_templateParameters().size()          = %" PRIuPTR " \n",definingTemplateClassDeclaration->get_templateParameters().size());
#endif
                                // DQ (9/9/2012): It appears that the template parameter list is not set properly (frequently zero, at least).
                                // ROSE_ASSERT(first_nondefiningTemplateClassDeclaration->get_templateParameters().size() > 0);
                                // ROSE_ASSERT(definingTemplateClassDeclaration->get_templateParameters().size() > 0);
                                 }
                                else
                                 {
                                // DQ (8/26/2012): This path is taken for non real classes (used to represent template parameters internally (nested) in template declarations).
#ifdef DEBUG_CONVERSION
                                   printf ("Nonreal classes are marked and don't have an associated template declaration (where they are built as template instantations). \n");
#endif
                                 }
                            }
                           else
                            {
                           // This is the case of a template class instantiation, and we need to generate the declaration (to build the
                           // type reference) AND build an AST for its class definition.  So that we can see all of the class data members
                           // and member functions.  See test2012_01.C for an example.
#if 0
                              printf ("In convert_type(): tk_class: buildTemplateDeclaration == false: buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
#endif
                              ROSE_ASSERT(buildTemplateInstantiation == true);
#if 0
                              printf ("In convert_type(): buildTemplateInstantiation == true: (could be instantiation of templated typedef): p = %p \n",p);
                              summarize_type(p);
#endif
                              SgTemplateInstantiationDecl* nondefiningTemplateInstantiation = isSgTemplateInstantiationDecl(tempDecl);
                              ROSE_ASSERT(nondefiningTemplateInstantiation != NULL);

                           // DQ (9/8/2012): I think this should be NULL (before we build the defining declaration).
                           // ROSE_ASSERT(tempDecl->get_definingDeclaration() == NULL);

                           // SgClassDeclaration* definingDeclaration = convert_struct_primary(p, SgClassDeclaration::e_class, nondefiningTemplateInstantiation);
                              SgDeclarationStatement* definingDeclaration = NULL;
                              if (tempDecl->get_definingDeclaration() == NULL)
                                 {
#if 0
                                   printf ("In convert_type(): No defining declaration exists; so building one now \n");
#endif
                                   definingDeclaration = convert_struct_primary(p, SgClassDeclaration::e_class, nondefiningTemplateInstantiation);
#if 0
                                   printf ("DONE: In convert_type(): No defining declaration exists; so building one now \n");
#endif
                                 }
                                else
                                 {
#if 0
                                   printf ("In convert_type(): Using the existing (previously built) defining declaration \n");
#endif
                                   definingDeclaration = tempDecl->get_definingDeclaration();
                                 }

                              ROSE_ASSERT(definingDeclaration != NULL);
                              SgTemplateInstantiationDecl* definingTemplateInstantiation = isSgTemplateInstantiationDecl(definingDeclaration);
                              ROSE_ASSERT(definingTemplateInstantiation != NULL);
                              ROSE_ASSERT(definingTemplateInstantiation->get_templateDeclaration() != NULL);

                           // DQ (1/2/2012): Added test.
                              ROSE_ASSERT(nondefiningTemplateInstantiation->get_templateDeclaration() != NULL);

                           // DQ (1/1/2012): Refactored some of this code to a seperate function.
                              if (nondefiningTemplateInstantiation->get_definingDeclaration() != definingTemplateInstantiation)
                                 {
                                // DQ (2/11/2012): This fails for test2004_35.C (STL test code).  I am not sure if that is a problem or not.
#ifdef DEBUG_CONVERSION
                                   printf ("WARNING: nondefiningTemplateInstantiation->get_definingDeclaration() = %p != definingTemplateInstantiation = %p \n",nondefiningTemplateInstantiation->get_definingDeclaration(),definingTemplateInstantiation);
#endif
                                 }
                           // ROSE_ASSERT(nondefiningTemplateInstantiation->get_definingDeclaration() == definingTemplateInstantiation);

                           // process_template_arguments(p,nondefiningTemplateInstantiation);
                           // ROSE_ASSERT(nondefiningTemplateInstantiation->get_templateArguments().empty() == true);
                           // process_template_arguments(p,nondefiningTemplateInstantiation,templateArgumentsList);

                           // DQ (5/17/2014): Some boost code such as:
                           //     template<typename F, typename has_apply_ = typename aux::has_apply<F>::type> struct apply_wrap0 : F::template apply<  > {};
                           // can define instantiations that have an empty argument list (see apply_wrap.hpp in Boost version 1.50).
                           // ROSE_ASSERT(nondefiningTemplateInstantiation->get_templateArguments().empty() == false);
                              if (nondefiningTemplateInstantiation->get_templateArguments().empty() == true)
                                 {
                                   mprintf ("WARNING: template instantiation with empty argument list (allowed) \n");
                                 }

                              ROSE_ASSERT(nondefiningTemplateInstantiation->get_templateName().is_null() == false);
                              ROSE_ASSERT(isSgTemplateInstantiationDecl(nondefiningTemplateInstantiation->get_firstNondefiningDeclaration())->get_templateName().is_null() == false);
                              if (nondefiningTemplateInstantiation->get_definingDeclaration() == NULL)
                                 {
#ifdef DEBUG_CONVERSION
                                   printf ("NOTE: Setting the defining template declaration in the nondefiningTemplateInstantiation \n");
#endif
                                   ROSE_ASSERT(definingTemplateInstantiation != NULL);
                                   nondefiningTemplateInstantiation->set_definingDeclaration(definingTemplateInstantiation);
                                 }
                              ROSE_ASSERT(nondefiningTemplateInstantiation->get_definingDeclaration() != NULL);
                              ROSE_ASSERT(isSgTemplateInstantiationDecl(nondefiningTemplateInstantiation->get_definingDeclaration()) != NULL);
                              ROSE_ASSERT(isSgTemplateInstantiationDecl(nondefiningTemplateInstantiation->get_definingDeclaration())->get_templateName().is_null() == false);

#ifdef DEBUG_CONVERSION
                              printf ("************************************************************************* \n");
                              printf ("*** Must process the instantiated template class definition name = %s \n",name.str());
                              printf ("************************************************************************ \n");
#endif
                            }
                      // Need to put the new defining declaration into the defining declaration cache (I think this is done already).
                      // ROSE_ASSERT(types_with_emitted_definitions[p] == true);
                         ROSE_ASSERT(isInTypeDeclarationMap(p));
#if 0
                         printf ("Exiting as a test after building the primary template instantiation (only for template instantiations). \n");
                         ROSE_ASSERT(false);
#endif
                       }
                      else
                       {
                      // DQ (7/18/2017): Added support to specify this as a class being built from a template parameter
                      // so that it could be built in the internal_intermediate_scope and then moved to a scope in the
                      // declaration (new concept of declaration scope); this is instead of being placed in the global
                      // scope as EDG lists it incorrectly. See test2017_44.C and test2017_47.C.
                      // This is the non-template non-template instantiation case, we only want to generate a reference
                      // to the class (from which to construct a SgClassType).  Build first nondefining declaration for
                      // type references to point to.
                      // tempDecl = convert_struct_secondary(p, SgClassDeclaration::e_class);
#if 0
                         printf ("In convert_type(): calling convert_struct_secondary() \n");
#endif
                      // DQ (9/28/2022): Remove this compiler warning (unused parameter).
                      // SgDeclarationStatement * decl_stmt = convert_struct_secondary(p, SgClassDeclaration::e_class, from_template_parameter);
                         SgDeclarationStatement * decl_stmt = convert_struct_secondary(p, SgClassDeclaration::e_class);
                         ROSE_ASSERT(decl_stmt != NULL);
                         SgNonrealDecl * nrdecl = isSgNonrealDecl(decl_stmt);
                         if (nrdecl != NULL) {
                           result = nrdecl->get_type();
                           ROSE_ASSERT(result != NULL);
#if 1
                        // DQ (12/20/2019): Debugging Cxx11_tests/test2019_505.C
                           printf ("Call to convert_struct_secondary() returned SgNonrealDecl (part 2): nrdecl = %p nrdecl->get_type() = %p \n",nrdecl,result);
#endif
                           break;
                         }
                         tempDecl = isSgClassDeclaration(decl_stmt);
                         ROSE_ASSERT(tempDecl != NULL);

#if 0
                         tempDecl->get_startOfConstruct()->display("In convert_type(): case tk_class: startOfConstruct");
                         tempDecl->get_endOfConstruct()  ->display("In convert_type(): case tk_class: endOfConstruct");
#endif
                       }

                 // TV (09/16/2018): (ROSE-1377) this is required after this point. Happened in CompileTests/RoseExample_tests/testRoseHeaders_06.C while working on non-real declaration
                    if (type_translation_cache.find(p) == type_translation_cache.end()) {
#if 0
                      printf("In convert_type():\n");
                      printf("  -- p = %p\n", p);
                      printf("  -- result = %p (%s)\n", result, result ? result->class_name().c_str() : "");
                      printf("  -- tempDecl = %p (%s)\n", tempDecl, tempDecl ? tempDecl->class_name().c_str() : "");
                      printf("     -- tempDecl->get_type() = %p (%s)\n", tempDecl->get_type(), tempDecl->get_type() ? tempDecl->get_type()->class_name().c_str() : "");
#endif
                      if (result == NULL && isSgTemplateClassDeclaration(tempDecl) != NULL) {
                        ROSE_ASSERT(tempDecl->get_type() != NULL);
                        type_translation_cache[p] = tempDecl->get_type();
                      }
                    }
                    ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());

                 // DQ (8/12/2012): Note that "result" may not have been updated after the class was built that put the class type (and any other types) into
                 // the type_translation_cache.  I think this is OK, but it should be updated at some point in this function.
                    if (result == NULL)
                       {
                         ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());

                         result = type_translation_cache[p];
                         ROSE_ASSERT(result != NULL);
                       }
                    ROSE_ASSERT(result != NULL);

                 // The type is only put into the cache if it is NOT a template declaration (templates don't generate types).
                    if (isSgTemplateClassDeclaration(tempDecl) == NULL)
                       {
                      // DQ (9/9/2012): Added assertion.
                         ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());

                         ROSE_ASSERT(type_translation_cache[p] != NULL);
                         ROSE_ASSERT(result != NULL);
                       }
                      else
                       {
                      // DQ (9/9/2012): Added assertion.
                         ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());

                      // Template declarations don't generate types.
                         if (type_translation_cache[p] != NULL)
                            {
#ifdef DEBUG_CONVERSION
                              printf ("In convert_type(): type_translation_cache[p] = %p = %s \n",type_translation_cache[p],type_translation_cache[p]->class_name().c_str());
#endif
                            }
                      // DQ (4/27/2012): I think it is OK for template class declarations to generate types (in fact any template...).
                      // ROSE_ASSERT(type_translation_cache[p] == NULL);
                      // ROSE_ASSERT(result == NULL);
                         ROSE_ASSERT(type_translation_cache[p] != NULL);
                         ROSE_ASSERT(result != NULL);
#if 0
                         printf ("In convert_type(): case tk_class: result = %p = %s = %s result->get_parent() = %p \n",result,result->class_name().c_str(),SageInterface::get_name(result).c_str(),result->get_parent());
                         printf ("In convert_type(): case tk_class: tempDecl = %p tempDecl->get_definingDeclaration() = %p tempDecl->get_firstNondefiningDeclaration() = %p \n",
                              tempDecl,tempDecl->get_definingDeclaration(),tempDecl->get_firstNondefiningDeclaration());
                         printf ("In convert_type(): case tk_class: tempDecl->get_parent() = %p \n",tempDecl->get_parent());
#endif
                      // DQ (12/31/2012): added assertion.
                      // ROSE_ASSERT(tempDecl->get_parent() != NULL);
                       }

                 // DQ (11/20/2011): I think this might be for this assertion to be false, it is an
                 // assertion that I think we can't support, but add a comment when it is violated (see test2011_155.C).
                 // ROSE_ASSERT(tempDecl->isForward() == false);
                    if (tempDecl->isForward() == true)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("Warning: tempDecl is marked as a forward declaration \n");
#endif
                       }

                    ROSE_ASSERT(result != NULL);
                 // if (!result) ROSE_ASSERT (!"Should already have been translated");

                 // DQ (3/3/2012): Added test for matching variants.
                    ROSE_ASSERT(matching_defining_and_nondefining_variants(tempDecl) == true);
#if 0
                    printf ("In convert_type(): case tk_class: result = %p = %s = %s \n",result,result->class_name().c_str(),SageInterface::get_name(result).c_str());
#endif
#if 0
                    SgScopeStatement* tmp_scope = tempDecl->get_scope();
                    ROSE_ASSERT(tmp_scope != NULL);
                    printf ("In convert_type(): case tk_class: tmp_scope = %p = %s \n",tmp_scope,tmp_scope->class_name().c_str());
#endif
#if 0
                    printf ("In convert_type(): tk_class: BOTTOM: type_translation_cache.find(p) != type_translation_cache.end() = %s \n",
                         type_translation_cache.find(p) != type_translation_cache.end() ? "true" : "false");
#endif
                    break;
                  }

               case tk_struct:
                  {
                 // This case needs to be unified with the tk_class case.
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("struct needing declaration\n");
#endif
#if 0
                    printf ("In convert_type(p=%p): tk_struct\n", p);
#endif
#if 0
                    printf ("In convert_type(): tk_struct: Calling summarize_type() \n");
                    summarize_type(p);
#endif
                    ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());
                    ROSE_ASSERT(template_type_translation_cache.find(p) == template_type_translation_cache.end());

                 // DQ (9/28/2022): Remove this compiler warning (unused parameter).
                 // SgDeclarationStatement * decl_stmt = convert_struct_secondary(p, SgClassDeclaration::e_struct, from_template_parameter);
                    SgDeclarationStatement * decl_stmt = convert_struct_secondary(p, SgClassDeclaration::e_struct);
                    ROSE_ASSERT(decl_stmt != NULL);
                    ROSE_ASSERT(matching_defining_and_nondefining_variants(decl_stmt) == true);

                    if (isSgTemplateClassDeclaration(decl_stmt)) {
                      ROSE_ASSERT(template_type_translation_cache.find(p) != template_type_translation_cache.end());
                      ROSE_ASSERT(template_type_translation_cache[p] != NULL);

                      result = template_type_translation_cache[p];
                    } else {
                      ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
                      ROSE_ASSERT(type_translation_cache[p] != NULL);

                      result = type_translation_cache[p];
                    }

#if 0
                    printf ("In convert_type(): tk_struct: BOTTOM: type_translation_cache.find(p) != type_translation_cache.end() = %s \n",
                         type_translation_cache.find(p) != type_translation_cache.end() ? "true" : "false");
#endif
                    break;
                  }

               case tk_union:
                  {
                 // This case needs to be unified with the tk_class case.
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("union needing declaration\n");
#endif
#if 0
                    printf ("In convert_type(p=%p): tk_union\n", p);
#endif
#if 0
                    printf ("In convert_type(): tk_union: Calling summarize_type() \n");
                    summarize_type(p);
#endif
                    ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());
                    ROSE_ASSERT(template_type_translation_cache.find(p) == template_type_translation_cache.end());

#if 0
                 // DQ (7/1/2019): Check and see if we have seen this type and/or defining declaration yet).
                    if (type_translation_cache.find(p) == type_translation_cache.end())
                      {
                     // We have not yet seen this type (so neither the non-defining nor defining declaration).

                        printf ("We have not yet seen this type (so neither the non-defining nor defining declaration) \n");
#if 0
                        printf ("In In convert_type(): tk_union: loop to iek_src_seq_end_of_construct entry_kind = %p = %s \n",
                             sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                     // We might want to call:
                     // SgDeclarationStatement* parse_structlike(SourceSequenceList& sse, a_type_ptr p, SgClassDeclaration::class_types t, bool forceTemplateClassDeclaration = false, a_template_ptr edg_template = NULL);
                     // SgDeclarationStatement* convert_struct_primary (a_type_ptr p, SgClassDeclaration::class_types t, SgClassDeclaration* nonDefiningDecl);
                     // Or maybe it should be process in:
                     // SgStatement* convert_condition(an_expr_node_ptr e, SourceSequenceList& sse, a_statement_ptr target_statement_after_conditional = NULL);
#if 0
                        SgClassDeclaration* nonDefiningDecl = NULL;
                        SgDeclarationStatement * defining_decl_stmt = convert_struct_primary(p, SgClassDeclaration::e_union, nonDefiningDecl);
#endif
#if 0
                        printf ("Exiting as a test! \n");
                        ROSE_ASSERT(false);
#endif
                      }
#endif

                 // DQ (7/1/2019): In the case of test2019_01.c we should be likely building a primary declaration here.
                 // No, the secondary declaration is what we want as long as we have handled the primary declaration where
                 // it appears within a conditional.

                 // DQ (9/28/2022): Remove this compiler warning (unused parameter).
                 // SgDeclarationStatement * decl_stmt = convert_struct_secondary(p, SgClassDeclaration::e_union, from_template_parameter);
                    SgDeclarationStatement * decl_stmt = convert_struct_secondary(p, SgClassDeclaration::e_union);
                    ROSE_ASSERT(decl_stmt != NULL);
                    ROSE_ASSERT(matching_defining_and_nondefining_variants(decl_stmt) == true);

                    if (isSgTemplateClassDeclaration(decl_stmt)) {
                      ROSE_ASSERT(template_type_translation_cache.find(p) != template_type_translation_cache.end());
                      ROSE_ASSERT(template_type_translation_cache[p] != NULL);

                      result = template_type_translation_cache[p];
                    } else {
                      ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
                      ROSE_ASSERT(type_translation_cache[p] != NULL);

                      result = type_translation_cache[p];
                    }
#if 0
                    printf ("In convert_type() (case tk_union): tempDecl = %p = %s \n",tempDecl,tempDecl->class_name().c_str());
                    SgScopeStatement* class_scope = tempDecl->get_scope();
                    printf ("   --- class_scope = %p tempDecl->get_scope() = %s \n",class_scope,class_scope->class_name().c_str());
#endif
                 // DQ (4/22/2018): Adding support to detect when types build in the function prototype are to be put into the SgDeclarationScope.
                    if (p->declared_in_function_prototype == true)
                       {
#if 0
                         printf ("In convert_type() (case tk_union): p->declared_in_function_prototype == true: result = %p = %s \n",result,result->class_name().c_str());
#endif
                         SgScopeStatement* scope = decl_stmt->get_scope();
#if 0
                         printf ("   --- scope = %p \n",scope);
#endif
                         if (scope != NULL)
                            {
                           // DQ (4/22/2018): Note that this step might have to be handled on the defining function as well.
#if 0
                              printf ("   --- scope = %p = %s \n",scope,scope->class_name().c_str());
#endif
                           // Find the associated symbol in the scope and remove it, so that we can place it into the internal_intermediate_scope.
                           // SgSymbol* search_for_symbol_from_symbol_table();
                              SgSymbol* symbol = decl_stmt->search_for_symbol_from_symbol_table();
                              ROSE_ASSERT(symbol != NULL);

                              SgClassSymbol* classSymbol = isSgClassSymbol(symbol);
                              ROSE_ASSERT(classSymbol != NULL);

                           // Should this be the mangled name?
                              SgName name = classSymbol->get_name();
#if 0
                              printf ("In convert_type() (case tk_union): calling scope->remove_symbol(): scope = %p = %s classSymbol->get_name() = %s \n",scope,scope->class_name().c_str(),name.str());
#endif
                              scope->remove_symbol(classSymbol);

                              ROSE_ASSERT(internal_intermediate_scope != NULL);
                              internal_intermediate_scope->insert_symbol(name,classSymbol);
                            }


                         ROSE_ASSERT(internal_intermediate_scope != NULL);
#if 0
                         printf ("Reset the scope and parent to the SgDeclarationScope: internal_intermediate_scope = %p = %s \n",internal_intermediate_scope,internal_intermediate_scope->class_name().c_str());
#endif
                         decl_stmt->set_scope(internal_intermediate_scope);
                         decl_stmt->set_parent(internal_intermediate_scope);

                      // We don't know the current function prototype until later.
                      // internal_intermediate_scope->set_parent(currentFunctionPrototype);
#if 0
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                       }

                    break;
                  }

               case tk_typeref:
                  {
                 // DQ (8/2/2014): Note that C++11 decltype() support is included in the case tk_typeref.
#if 0
                    printf ("\n************************************************************** \n");
                    printf ("************************************************************** \n");
                    printf ("In convert_type(): case tk_typeref: p = %p \n",p);
                    printf ("************************************************************** \n");
                    printf ("************************************************************** \n\n");
#endif
                 // Use of a typedef, i.e., a type equivalent to another type; also used to add type qualifiers (const or volatile) to a type.

#ifdef DEBUG_CONVERSION
                    ROSE_ASSERT(p->variant.typeref.extra_info != NULL);
                    indent_line(); debug_print("typeref with block size %d\n", (int)(p->variant.typeref.extra_info->upc_block_size));
#endif

                 // DQ (3/3/2018): I think that we can assert this now.
                    ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());

                    if (type_translation_cache.find(p) == type_translation_cache.end())
                       {
                      // DQ (6/6/2016): Actually, it is just that the type that can be in a template instantiation has not been built yet (see test2016_46.C for an example).

                      // This branch is at least taken for test2012_190.C.
                      // printf ("ERROR: In convert_type(): tk_typeref: How can we be referencing a type that does not exist in the type_translation_cache? p = %p \n",p);
                      // ROSE_ASSERT(false);

                      // DQ (2/19/2018): Uncommented this to test template alias handling.
#if 0
                         printf ("******************************************************************************************************************************************* \n");
                         printf ("WARNING: This goes too far and resolves the base type when it should be returning the typedef type (p = %p = %s) \n",p,p->source_corresp.name);
                         printf ("******************************************************************************************************************************************* \n");
                      // ROSE_ASSERT(false);
#endif
#if 0
                         printf ("In convert_type(): tk_typeref: type_translation_cache.find(p) == type_translation_cache.end(): Calling summarize_type() \n");
                         summarize_type(p);
#endif
#if 0
                         printf ("In convert_type(): case tk_typeref: p = %p Calling convert_type on p->variant.typeref.type = %p \n",p,p->variant.typeref.type);
#endif
#if 0
                      // ROSE_ASSERT(p->source_corresp.name != NULL);
                         string typeref_name;
                         if (p->source_corresp.name != NULL)
                            {
                              typeref_name = p->source_corresp.name;
                            }
                         printf ("\n********************************************** \n");
                         printf ("In convert_type(): tk_typeref: setup base_type: typeref_name = %s \n",typeref_name.c_str());
                         printf ("********************************************** \n");
#endif
                      // DQ (8/12/2012): Because we only explore the rest of the EDG AST by generating non-definging declarations,
                      // we never generate a type for p and instead return the base type and use that as an entry for p in the
                      // type_translation_cache map. This is a problem for test2012_190.C.  We should instead build a SgTypedefDeclaration
                      // so that we can generate a SgTypedefType directly.  But then we would not have a location to put the typedef
                      // declaration because we have not seen the class scope where it belongs (if it is part of a class instantiation).
                      // Though I think requesting the scope should trigger the defining declaration to be generated, as I recall.
                      // result = convert_type(p->variant.typeref.type);
                         SgType* base_type = nullptr;
                         SgType* base_undeduced_type = nullptr;

                         std::tie(base_type, base_undeduced_type) = convert_type2(p->variant.typeref.type, withNondeducedTypes, false, false);

                      // DQ (3/4/2018): I think we can assert this.
                         ROSE_ASSERT(base_type != NULL);
#if 0
                         printf ("************************************************************** \n");
                         printf ("In convert_type(): tk_typeref: DONE: setup base_type = %p = %s \n",base_type,base_type->class_name().c_str());
                         printf ("   --  base_type = %s \n",base_type->unparseToString().c_str());
                         printf ("************************************************************** \n\n");
#endif
#if 0
                         printf ("In convert_type(): case tk_typeref: p = %p DONE: Calling convert_type on p->variant.typeref.type = %p \n",p,p->variant.typeref.type);
                         printf ("In convert_type(): case tk_typeref: base_type = %p = %s \n",base_type,base_type->class_name().c_str());
#endif

                      // DQ (3/3/2018): Reorganize how we handle type references.
                         enum kinds_of_type_references
                            {
                              e_unknown_type_reference = 0,    // Error condition.
                              e_modifier_type_reference,         // This is a modifier applied to a type.
                              e_typeof_type_reference,         // This is a typeof operator applied to a type or expression.
                              e_decltype_type_reference,       // This is a decltype operator to a declaration???.
                              e_template_alias_type_reference, // This is a C++11 specific template alias (templated typedef)

                              e_deduce_auto_type,
                              e_decltype_auto_type_reference,
                              e_last_type_reference            // Error condition
                            };

                         kinds_of_type_references kind = e_unknown_type_reference;
                         // TODO EDG 6.5 defines that in il_def.h at line 9204
                         switch (p->variant.typeref.kind) {
                           case trk_none:                         kind = e_modifier_type_reference;       break;
                           case trk_is_decltype:                  kind = e_decltype_type_reference;       break;
                           case trk_is_deduced_decltype_auto:     kind = e_decltype_auto_type_reference;  break;
                           case trk_is_deduced_auto:              kind = e_deduce_auto_type;              break;
                           case trk_is_deduced_class:             kind = e_modifier_type_reference;       break; // C++17
                           case trk_is_typeof_with_expression:    kind = e_typeof_type_reference;         break;
                           case trk_is_typeof_with_type_operand:  kind = e_typeof_type_reference;         break;
                           case trk_is_alias:                     kind = e_modifier_type_reference;       break;
                           case trk_is_template_alias:            kind = e_template_alias_type_reference; break;
                           case trk_for_type_attributes:          kind = e_modifier_type_reference;       break;
                           case trk_is_underlying_type:           kind = e_modifier_type_reference;       break;
                           case trk_bases:                ROSE_ABORT();
                           case trk_direct_bases:         ROSE_ABORT();
                           case trk_add_lvalue_reference: ROSE_ABORT();
                           case trk_add_pointer:          ROSE_ABORT();
                           case trk_add_rvalue_reference: ROSE_ABORT();
                           case trk_decay:                ROSE_ABORT();
                           case trk_make_signed:          ROSE_ABORT();
                           case trk_make_unsigned:        ROSE_ABORT();
                           case trk_remove_all_extents:   ROSE_ABORT();
                           case trk_remove_const:         ROSE_ABORT();
                           case trk_remove_cv:            ROSE_ABORT();
                           case trk_remove_cvref:         ROSE_ABORT();
                           case trk_remove_extent:        ROSE_ABORT();
                           case trk_remove_pointer:       ROSE_ABORT();
                           case trk_remove_reference_t:   ROSE_ABORT();
                           case trk_remove_restrict:      ROSE_ABORT();
                           case trk_remove_volatile:      ROSE_ABORT();
                           case trk_remove_reference:     ROSE_ABORT();
                           default:
                               ROSE_ABORT();
                         }
                         switch (kind)
                            {
                              case e_unknown_type_reference:
                                 {
                                   printf ("Error: unknown_type_reference is not an allowed case \n");
                                   ROSE_ASSERT(false);
                                   break;
                                 }

                              case e_modifier_type_reference:
                              case e_deduce_auto_type:
                                 {
                                   a_bit_field qualifiers = p->variant.typeref.qualifiers;

                                   // PP (9/18/25):
                                   // Only use SgModifier when the type modifies (qualifiers != 0)
                                   // or when the type needs fixup (kind == e_modifier_type_reference) && (qualifiers == 0)
                                   if ((kind == e_modifier_type_reference) || (qualifiers != 0))
                                   {
                                     SgModifierType * modifierType = new SgModifierType(base_type);
                                     ROSE_ASSERT(modifierType != NULL);
                                     ROSE_ASSERT(modifierType != base_type);

                                     if (qualifiers & TQ_CONST) {
                                       modifierType->get_typeModifier().get_constVolatileModifier().setConst();
                                     }
                                     if (qualifiers & TQ_VOLATILE) {
                                       modifierType->get_typeModifier().get_constVolatileModifier().setVolatile();
                                     }
                                     if (qualifiers & TQ_RESTRICT) {
                                       modifierType->get_typeModifier().setRestrict();
                                     }

#if UPC_EXTENSIONS_ALLOWED
                                     if (qualifiers & TQ_UPC_SHARED)
                                        {
                                          modifierType->get_typeModifier().get_upcModifier().set_isShared(true);

                                          int block_size = p->variant.typeref.extra_info->upc_block_size;
                                          if (block_size == UPC_BLOCK_SIZE_NONE || block_size == 1) {
                                            modifierType->get_typeModifier().get_upcModifier().set_layout(1);
                                          } else if (block_size == UPC_BLOCK_SIZE_INDEFINITE) {
                                            modifierType->get_typeModifier().get_upcModifier().set_layout(0);
                                          } else if (block_size == UPC_BLOCK_SIZE_BLOCK) {
                                            modifierType->get_typeModifier().get_upcModifier().set_layout(-2); // [*] layout
                                          } else {
                                            modifierType->get_typeModifier().get_upcModifier().set_layout(block_size); // [block_size] layout
                                          }
                                        }
                                     if (qualifiers & TQ_UPC_STRICT)  modifierType->get_typeModifier().get_upcModifier().set_modifier(SgUPC_AccessModifier::e_upc_strict);
                                     if (qualifiers & TQ_UPC_RELAXED) modifierType->get_typeModifier().get_upcModifier().set_modifier(SgUPC_AccessModifier::e_upc_relaxed);
#endif
                                     SgModifierType * result2 = SgModifierType::insertModifierTypeIntoTypeTable(modifierType);
                                     if (modifierType != result2) {
                                       delete modifierType;
                                     }

                                     ASSERT_not_null(result2);
                                  // If there were not any qualifiers, then why is this a typeref
                                  //   (unless it is a decltype or typeof operator).
                                     if (qualifiers == 0) {
                                        ROSE_ASSERT(result2 != result2->get_base_type());
                                        result2->set_frontend_type_reference((char*)p);
                                     }

                                     result = result2;
                                   }
                                   else {
                                     result = base_type;
                                   }

                                   ASSERT_not_null(result);

                                // handle auto
                                //   after the main type has been built, handle the non deduced type (if requested).
                                   if (withNondeducedTypes)
                                   {
                                     if (kind == e_deduce_auto_type)
                                     {
                                       resultWithoutDeduction = buildAutoType();
                                     }
                                     else if (base_undeduced_type && (qualifiers != 0))
                                     {
                                       SgModifierType* result_modifier = isSgModifierType(result);
                                       ROSE_ASSERT(result_modifier != NULL);
                                       SgModifierType* undeducedModifier = new SgModifierType(base_undeduced_type);

                                       undeducedModifier->get_typeModifier() = result_modifier->get_typeModifier();

                                       resultWithoutDeduction = undeducedModifier;
                                     }
                                     else
                                     {
                                       // resultWithoutDeduction = nullptr;
                                     }
                                   }

                                   break;
                                 }

                              case e_typeof_type_reference:
                                 {
#define DEBUG_TYPEOF 0
                                   SgScopeStatement* scope = topScopeStack();
                                   ROSE_ASSERT(scope->get_parent() != NULL);

                                   bool typeofOperatorPreviouslyBuilt = false;
                                   if (edg_type_to_rose_typeof_type_map.find(p) != edg_type_to_rose_typeof_type_map.end())
                                      {
                                        typeofOperatorPreviouslyBuilt = true;

                                        result = edg_type_to_rose_typeof_type_map[p];
                                        ROSE_ASSERT(result != NULL);
                                      }

                                // The type was created by a typeof operator (a GNU C extension).
                                   a_typeref_type_supplement_ptr extra_info = p->variant.typeref.extra_info;
                                   ROSE_ASSERT(extra_info != NULL);
                                   if (typeofOperatorPreviouslyBuilt == false)
                                      {
                                       SgExpression* typeof_arg_expression = NULL;
                                       if (extra_info->expr != NULL)
                                          {
                                            typeof_arg_expression = convert_expression(extra_info->expr);
                                            ROSE_ASSERT(typeof_arg_expression != NULL);
                                          }
                                       SgType* typeof_arg_type = result;
                                       if (extra_info->operator_type_arg != NULL)
                                          {
                                            typeof_arg_type = convert_type(extra_info->operator_type_arg);
                                            ROSE_ASSERT(typeof_arg_type != NULL);
                                          }
                                         else
                                          {
                                            if (p->variant.typeref.type != NULL)
                                               {
                                                 typeof_arg_type = convert_type(p->variant.typeref.type);
                                                 ROSE_ASSERT(typeof_arg_type != NULL);
                                               }
                                          }

                                       if (typeof_arg_expression != NULL)
                                          {
                                            result = SageBuilder::buildTypeOfType(typeof_arg_expression,typeof_arg_type);
                                          }
                                         else
                                          {
                                            result = SageBuilder::buildTypeOfType(typeof_arg_expression,typeof_arg_type);
                                          }
                                      }
                                   ROSE_ASSERT(result != NULL);
                                   break;
                                 }


                              case e_decltype_type_reference:
                              case e_decltype_auto_type_reference:
                                 {
                                   ASSERT_not_null(base_type);
                                   ROSE_ASSERT(p->variant.typeref.extra_info != NULL);
                                   an_expr_node_ptr decltype_arg = p->variant.typeref.extra_info->expr;
                                   if (decltype_arg != NULL)
                                      {
                                        SgExpression* decltype_arg_expression = convert_expression(decltype_arg);
                                        ASSERT_not_null(decltype_arg_expression);

                                        result = SageBuilder::buildDeclType(decltype_arg_expression,base_type);
                                        ROSE_ASSERT(decltype_arg_expression->get_parent() != NULL);
                                      }
                                     else
                                      {
                                        // PP (9/24/25)
                                        // \todo SgTypeExpression(SgAutoType()) seems to work
                                        //       but should we use just SgVoidExpr()?
                                        SgType*       autoType = buildAutoType();
                                        SgExpression* autoExpr = buildTypeExpression(autoType);

                                        result = base_type;
                                        resultWithoutDeduction = SageBuilder::buildDeclType(autoExpr,base_type);
                                      }
                                   ASSERT_not_null(result);
                                   break;
                                 }

                              case e_template_alias_type_reference:
                                 {
                                   SgName name = p->source_corresp.name;
                                   SgType* rose_base_type = base_type;
                                   ROSE_ASSERT(rose_base_type != NULL);
                                   bool has_defining_base = false;

                                // This needs to be found using a lookup into the template declaration map using p->variant.typeref.extra_info->assoc_template.
                                   SgTemplateTypedefDeclaration* templateTypedefDeclaration = NULL;
                                   a_template_ptr edg_template = p->variant.typeref.extra_info->assoc_template;
                                   ROSE_ASSERT(edg_template != NULL);
                                   if (hasFirstNondefDeclForTemplate(edg_template))
                                      {
                                        SgDeclarationStatement* nondefiningDeclaration = getFirstNondefDeclForTemplate(edg_template);
                                        ROSE_ASSERT(nondefiningDeclaration == nondefiningDeclaration->get_firstNondefiningDeclaration());
                                        templateTypedefDeclaration = isSgTemplateTypedefDeclaration(nondefiningDeclaration);
                                        ROSE_ASSERT(templateTypedefDeclaration != NULL);
                                      }
                                     else
                                      {
                                        ROSE_ASSERT(!hasFirstNondefDeclForTemplate(edg_template));
                                        ROSE_ASSERT(!hasDefiningDeclForTemplate(edg_template));
                                        SgDeclarationStatement* templateDeclaration = convert_template_prototype(edg_template);
                                        ROSE_ASSERT(templateDeclaration != NULL);

                                        templateTypedefDeclaration = isSgTemplateTypedefDeclaration(templateDeclaration);
                                        ROSE_ASSERT(templateTypedefDeclaration != NULL);
                                      }

                                   ROSE_ASSERT(hasFirstNondefDeclForTemplate(edg_template));
                                   ROSE_ASSERT(templateTypedefDeclaration != NULL);

                                   SgDeclarationStatement* nondefiningDeclaration = getFirstNondefDeclForTemplate(edg_template);
                                   ROSE_ASSERT(nondefiningDeclaration == nondefiningDeclaration->get_firstNondefiningDeclaration());
                                   templateTypedefDeclaration = isSgTemplateTypedefDeclaration(nondefiningDeclaration);
                                   ROSE_ASSERT(templateTypedefDeclaration != NULL);

                                   SgScopeStatement* scope = templateTypedefDeclaration->get_scope();
                                   ROSE_ASSERT(scope != NULL);

                                   bool buildTemplateInstantiation = true;
                                   a_template_arg_ptr templateArgs = p->variant.typeref.extra_info->template_arg_list;
                                   SgTemplateArgumentPtrList templateArgumentsList;
                                   if (buildTemplateInstantiation == true && templateArgs != NULL)
                                      {
                                        templateArgumentsList = parse_template_arguments(templateArgs,scope);
                                      }
                                     else
                                      {
                                        printf ("ERROR: This should have been a request for a templated instantiation (of a templated typedef) \n");
                                        ROSE_ASSERT(false);
                                      }

                                   SgTemplateInstantiationTypedefDeclaration * td_decl = SageBuilder::buildTemplateInstantiationTypedefDeclaration_nfi(
                                       name, rose_base_type, scope, has_defining_base, templateTypedefDeclaration, templateArgumentsList
                                   );
                                   ROSE_ASSERT(td_decl != NULL);

                                   for (SgTemplateArgumentPtrList::iterator it = templateArgumentsList.begin(); it != templateArgumentsList.end(); it ++) {
                                     SgNode * parent = (*it)->get_parent();
#if WARNING_FOR_NONREAL_DEVEL
                                     if (isSgScopeStatement(parent)) {
                                       printf("WARNING: template template argument = %p has an invalid parent = %p (%s). Reset!\n", *it, parent, parent->class_name().c_str());
                                     }
#endif
                                     if ((parent == NULL) || isSgScopeStatement(parent)) {
                                       (*it)->set_parent(td_decl);
                                     } else {
                                       SgDeclarationStatement * declstmt_parent = isSgDeclarationStatement(parent);
#if WARNING_FOR_NONREAL_DEVEL
                                       if (declstmt_parent == NULL) {
                                         printf("ERROR: Unexpected for template argument: parent = %p (%s)\n", parent, parent->class_name().c_str());
                                       }
#endif
                                       ROSE_ASSERT(declstmt_parent != NULL);
                                       ROSE_ASSERT(declstmt_parent->get_firstNondefiningDeclaration() == td_decl->get_firstNondefiningDeclaration());
                                     }
                                   }
                                   result = td_decl->get_type();
                                   ROSE_ASSERT(result != NULL);
                                   break;
                                 }

                              case e_last_type_reference:
                                 {
                                   printf ("Error: last_type_reference is not an allowed case \n");
                                   ROSE_ASSERT(false);
                                   break;
                                 }

                              default:
                                 {
                                   printf ("Error: default reached in switch over type reference kinds \n");
                                   ROSE_ASSERT(false);
                                   break;
                                 }
                            }

                         ROSE_ASSERT(result != NULL);
                         ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());
                         type_translation_cache[p] = result;

                         ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
                         ROSE_ASSERT(type_translation_cache[p] != NULL);
                       }

                    ROSE_ASSERT(result != NULL);

                    break;
                  }

            // DQ (1/11/2009): Not clear if we will eventually have to implement this case!
               case tk_ptr_to_member:
                  {
                 // printf ("Case of tk_ptr_to_member: calling convert_ptr_to_member_type() \n");
                    result = convert_ptr_to_member_type(p);
                 // printf ("DONE: Case of tk_ptr_to_member: calling convert_ptr_to_member_type() \n");
                    ROSE_ASSERT(result != NULL);

                 // DQ (12/18/2011): Case we assert this?
                    SgPointerMemberType* pointerMemberType = isSgPointerMemberType(result);
                    ROSE_ASSERT(pointerMemberType != NULL);
                    ROSE_ASSERT(pointerMemberType->get_base_type() != NULL);

                 // DQ (12/18/2011): test2011_183.C demonstrates where this need not be a member function (reference to integer member data).
                 // ROSE_ASSERT(isSgMemberFunctionType(pointerMemberType->get_base_type()) != NULL);
                    break;
                  }

               case tk_template_param:
                  {
                    ROSE_ASSERT(false); // Should be caught before switch starts...
                    break;
                  }

#if GNU_VECTOR_TYPES_ALLOWED
            // DQ (7/27/2014): Added EDG 4.9 specific members.
               case tk_vector:
                  {
                 // GNU vector types.
#if 0
                    printf ("In convert_type(): Detected a vector type (temporarily converted to double type) \n");
#endif
                 // DQ (9/12/2016): For temporary debugging, build this as a double type (need to add SgVectorType to ROSE).
                    result = buildDoubleType();

                 // DQ (12/17/2016): Reduced the number of these messages.
                    static int counter = 0;
                    if (counter++ % 50 == 0)
                       {
                      // DQ (3/5/2017): Fixed to use mlog message logging.
                         mprintf ("In convert_type(): case tk_vector: still not correctly implemented! \n");
                       }
                    break;
                  }
#endif
            // DQ (7/27/2014): Added EDG 4.9 specific members.
               case tk_nullptr:
                  {
                 // Type of the C++ or C++/CLI nullptr and __nullptr keywords.  There are two nullptr types that have
                 // slightly different semantics: the managed nullptr type is the type of the nullptr keyword
                 // in C++/CLI, and the standard nullptr type (std::nullptr_t) is the type of the non-C++/CLI
                 // nullptr keyword and the __nullptr keyword in Microsoft mode (both C++/CLI and native).  See
                 // is_managed_nullptr_type and is_standard_nullptr_type in types.c for details.

#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("nullptr type\n");
#endif
#if 0
                    printf ("In convert_type() (case tk_nullptr): Calling summarize_type() \n");
                    summarize_type(p);
#endif
                 // DQ (7/27/2014): Make this a pointer to void (but it might be that we need a new type IR node in ROSE to support nullptr_t).
                 // result = buildPointerType(buildVoidType());
                    result = buildNullptrType();
#if 0
                    printf ("In convert_type() (case tk_nullptr): result = %p = %s result->get_parent() = %p \n",result,result->class_name().c_str(),result->get_parent());
#endif
                    break;
                  }

            // DQ (10/16/2016): Found new case in EDG AST?
               case tk_error:
                 {
                    printf ("Case not implemented for tk_error \n");
                    ROSE_ASSERT(false);

                    break;
                 }


            // DQ (12/29/2008): Added cases not handled (these are OK to default to error as in default clause, I think).
            // case tk_error:
#if FIXED_POINT_ALLOWED
               case tk_fixed_point:
#endif
               case tk_unknown:
                  {
                    printf ("Case not handled! kind = %s \n",output_type_enum(kind).c_str());
                    ROSE_ASSERT(false);
                    break;
                  }

               default:
                  {
                    ERROR_MESSAGE(stderr, "In convert_type(): default reached in switch: Bad type kind %d = %s \n", kind,output_type_enum(kind).c_str());
                    ROSE_ASSERT(false);
                  }
             }

// Note label
end: ;
        }

  // indent_line(); debug_print("Leaving convert_type\n");

  // DQ (12/12/2011): I would like to not return NULL (propagates errors too far).
     ROSE_ASSERT (result != NULL);

#if 0
     printf ("Leaving convert_type(): Calling testEdgRoseMaps(): part 1 \n");
#endif

  // DQ (9/9/2012): Test the maps that we use internally (especially the new tests on the type_translation_cache).
     testEdgRoseMaps();

  // DQ (8/12/2012): Added tests (unclear if I can enforce this since other child function calls may have already updated the map).
  // In general, I will trade uniformity (of the update being done in one place) with the map being updated as early as possible
  // as new types are built (this is because multiple types may be constructed).
  // ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());

  // DQ (8/12/2012): Now that we no longer build a reference to a NULL pointer in the type_translation_cache,
  // we have to update the map explicitly.
     if (type_translation_cache.find(p) == type_translation_cache.end())
        {
#if 0
          printf ("Leaving convert_type(): Adding to type_translation_cache: p = %p result = %p = %s \n",p,result,result->class_name().c_str());
#endif
          type_translation_cache[p] = result;
        }

  // DQ (8/12/2012): Added tests.
     ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
     ROSE_ASSERT(type_translation_cache[p] != NULL);

     recordIfNotNull(typesWithoutDeduction, p, resultWithoutDeduction);


#if 0
     printf ("Leaving convert_type(): Calling testEdgRoseMaps(): part 2 \n");
#endif

  // DQ (8/12/2012): Test the maps that we use internally (especially the new tests on the type_translation_cache).
     testEdgRoseMaps();

#if 0
     printf ("Leaving convert_type(): DONE: Calling testEdgRoseMaps() \n");
#endif

// #if 0
#if DEBUG_IS_TEMPLATE_DECLARATION || 0
     printf ("Leaving convert_type(): p = %p = %s result = %p = %s \n",p,summarize_type_kind_in_string(p),result,result->class_name().c_str());
     if (isSgFunctionType(result) != NULL)
        {
           SgFunctionType* functionType = isSgFunctionType(result);
           SgType* return_type = functionType->get_return_type();

           printf ("Leaving convert_type(): result = %p = %s functionType->get_return_type() = %p = %s \n",functionType,functionType->class_name().c_str(),return_type,return_type->class_name().c_str());
        }
#endif

  // DQ (3/28/2015): Testing for corruption in return value.
     ROSE_ASSERT(result != NULL);

#if 0
     if (isSgFunctionType(result) != NULL)
        {
          printf ("Leaving convert_type(): p = %p = %s result = %p = %s \n",p,summarize_type_kind_in_string(p),result,result->class_name().c_str());
        }
#endif

#if 0
     printf ("Leaving convert_type(): tk_class: p = %p type_translation_cache.find(p) != type_translation_cache.end() = %s \n",
          p,type_translation_cache.find(p) != type_translation_cache.end() ? "true" : "false");
#endif
#if 0
     printf ("\n************************************************************** \n");
  // printf ("Leaving convert_type(): result = %p = %s \n",result,result->class_name().c_str());
     printf ("Leaving convert_type(): result = %p = %s = %s \n",result,result->class_name().c_str(),result->unparseToString().c_str());
     printf ("************************************************************** \n");
     printf ("************************************************************** \n\n");
#endif

     SageInterface::detectCycleInType(result, "convert_type() before return");

     return { result, resultWithoutDeduction };
   }


SgType*
EDG_ROSE_Translation::convert_type(a_type_ptr p, bool pointerToMember, bool from_template_parameter)
   {
     SgType* result = nullptr;

     std::tie(result, std::ignore) = convert_type2( p,
                                                    false /* do not generate types for deduced types */,
                                                    pointerToMember,
                                                    from_template_parameter
                                                  );
     return result;
   }
