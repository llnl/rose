
#include "edgRose.h"

std::tuple<SgFunctionType*, SgFunctionType*>
EDG_ROSE_Translation::convert_function_type(a_type_ptr p, bool withNondeducedTypes, bool pointerToMember)
   {
     ROSE_ASSERT(p->kind == tk_routine);

#if 0
     printf ("In convert_function_type(): p = %p pointerToMember = %s \n",p,pointerToMember ? "true" : "false");
#endif

#if 0 // RESULT_HANDLING_1
#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("process return type\n");
#endif

  // DQ (8/8/2014): For C++11 support of the new function return type syntax, we sometime 
  // have to process the function parameters before procesing the return type.

     SgType* return_type = convert_type(p->variant.routine.return_type);
#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("DONE: process return type\n");
#endif
#if 0
     printf ("In convert_function_type(): return_type = %p = %s \n",return_type,return_type->class_name().c_str());
#endif
#endif // RESULT_HANDLING_1

     a_routine_type_supplement_ptr sup = p->variant.routine.extra_info;
     a_param_type_ptr params           = sup->param_type_list;

#if 0
  // DQ (1/9/2014): Added debbung output (debugging test2014_29.c, but not useful).
     if (params == NULL)
        {
          printf ("WARNING: In convert_function_type(): sup->param_type_list == NULL \n");
          ROSE_ASSERT(false);
        }
#endif

  // DQ (12/30/2008): Use the build functions instead (below).
  // bool has_ellipsis = sup->has_ellipsis;
  // SgFunctionType* t = new SgFunctionType(return_type, has_ellipsis);

#if 0
     printf ("In convert_function_type(): calling summarize_type() \n");
     summarize_type(p);
#endif

  // DQ (8/19/2012): Removing redundant veriable function_type_data, but also make sure that p->kind == tk_routine.
  // a_type_ptr function_type_data = p;
  // ROSE_ASSERT(function_type_data != NULL);

  // DQ (12/17/2011): This is not always true.
  // ROSE_ASSERT(associatedFunction != NULL);


  // Get the extra information about the function type in the EDG AST.
  // a_routine_type_supplement_ptr extra_info = function_type_data->variant.routine.extra_info;
     a_routine_type_supplement_ptr extra_info = p->variant.routine.extra_info;
     ROSE_ASSERT(extra_info != NULL);

     a_routine_ptr associatedFunction = extra_info->assoc_routine;

#if 0
     printf ("In convert_function_type(): associatedFunction = %p \n",associatedFunction);
#endif

  // Note that now that this refactored code is not only used for typdefs of function types (function pointers), 
  // we can't assert anthign about the extra_info->assoc_routine data member in EDG.
  // Not suprising that there is no associated function since this is a typedef based on a function type and not a specific function.
  // a_routine_ptr associated_routine = extra_info->assoc_routine;
  // ROSE_ASSERT(associated_routine == NULL);

     a_type_ptr this_class = extra_info->this_class;

  // DQ (3/27/2012): I don't think we can assert this for all functions.
  // ROSE_ASSERT(this_class != NULL);

#if 0
     printf ("In convert_function_type(): this_class = %p \n",this_class);
#endif

     SgType* roseClass = NULL;
     if (this_class != NULL)
        {
#if 0
          printf ("In convert_function_type(): (this_class != NULL): calling convert_type(this_class = %p) \n",this_class);
#endif
          roseClass = convert_type(this_class);
#if 0
          printf ("DONE: In convert_function_type(): (this_class != NULL): calling convert_type(this_class = %p) \n",this_class);
#endif
#if 0
          printf("type = %p (%s)\n", type, type ? type->class_name().c_str() : "");
#endif
          ROSE_ASSERT(isSgClassType(roseClass) || isSgNonrealType(roseClass));
        }
       else
        {
       // DQ (12/6/2012): Added branch to help set the roseClass (not required to be valid to support generation of proper member function type.
          if (p->kind == tk_routine)
             {
            // If we can't identify the associated scope of the function then look for it in the extra_info->assoc_routine.
#if 0
               printf ("In convert_function_type(): (roseClass == NULL): associatedFunction = %p \n",associatedFunction);
#endif
               if (associatedFunction != NULL)
                  {
#if 0
                    printf ("In convert_function_type(): (roseClass == NULL): calling summarize_routine(): associatedFunction = %p \n",associatedFunction);
                    summarize_routine(associatedFunction);
#endif
#if DEBUG_GET_DECLARATION_SCOPE
                    printf ("In convert_function_type(): calling getDeclarationScope() \n");
#endif
                    SgScopeStatement*  scope           = getDeclarationScope(associatedFunction->source_corresp);
                    SgClassDefinition* classDefinition = isSgClassDefinition(scope);
                    if (classDefinition != NULL)
                       {
                         ROSE_ASSERT(classDefinition->get_declaration() != NULL);
                         roseClass = classDefinition->get_declaration()->get_type();
                         ROSE_ASSERT(roseClass != NULL);
                       }
                  }
             }
            else
             {
#if 0
               printf ("In convert_function_type(): (roseClass == NULL): Can't identify the scope of the type: calling summarize_type()\n");
               summarize_type(p);
#endif
             }

       // DQ (12/6/2012): This could be a non-member function so it is fine to not compute a valid associated roseClass
       // ROSE_ASSERT(roseClass != NULL);
        }

  // DQ (3/8/2012): Added support for specification of function type modifiers.
     unsigned int mfunc_specifier = setupFunctionTypeModifiers(extra_info);

  // ROSE_ASSERT(mfunc_specifier == 0);

  // SgPartialFunctionType* partialFunctionType = SgPartialFunctionType::createType(return_type);
  // SgType* associatedClass = NULL;
  // SgType* associatedClass = roseClass;

#if 0 // PARTIAL_FUNC_1
  // DQ (8/19/2012): I want to remove the use of the SgPartialFunctionType if possible.
     SgPartialFunctionType* partialFunctionType = new SgPartialFunctionType(return_type, false, associatedClass, mfunc_specifier);
     ROSE_ASSERT(partialFunctionType != NULL);

#if 0
     printf ("############## In convert_function_type(): building SgPartialFunctionType = %p ##############\n",partialFunctionType);
#endif
#endif // PARTIAL_FUNC_1

  // Add the parameter types to the SgPartialFunctionType
     vector<SgType*> param_types;
     for (a_param_type_ptr param = params; param != NULL; param = param->next)
        {
#ifdef DEBUG_CONVERSION
          indent_line(); debug_print("Processing function parameter types: case tk_routine\n");
#endif
#if 0
          printf ("In convert_function_type(): calling summarize_param_type() \n");
          summarize_param_type(param);
#endif
#if 0
          if (param->name != NULL)
             {
               printf ("######################### In convert_function_type(): param->name = %s \n",param->name);
             }
#endif
#if GNU_EXTENSIONS_ALLOWED
       // DQ (9/29/2012): Added detection of transparent unions
          if (param->is_transparent == true)
             {
            // For a parameter of union type, TRUE if the union is transparent.
            // This is a C only feature, transparent unions are not yet supported
               printf ("WARNING: This is a C only feature, transparent unions are not yet supported p = %p \n",p);
               ROSE_ASSERT(false);
             }

       // DQ (9/29/2012): Added detection of non-null pointers.
          if (param->nonnull == true)
             {
            // TRUE if this represents a parameter of pointer type that must be passed a non-NULL argument.
#ifdef DEBUG_CONVERSION
               printf ("WARNING: non-NULL argument specification is not yet supported p = %p \n",p);
#endif
            // ROSE_ASSERT(false);
             }
#endif
#if 0
          printf ("In convert_function_type(): (loop over parameters): calling convert_type() param->declared_type = %p \n",param->declared_type);
#endif
       // DQ (6/8/2012): Make sure that we don't insert NULL pointers (see test2012_99.C).
       // param_types.push_back(convert_type(param->declared_type));
       // ROSE_ASSERT(param->declared_type != NULL);
       // SgType* type = convert_type(param->declared_type);
          SgType* type = NULL;
          if (param->declared_type != NULL)
             {
               type = convert_type(param->declared_type);
             }
            else
             {
#ifdef DEBUG_CONVERSION
               printf ("WARNING: the param->declared_type == NULL (so using the param->type instead) (might miss some qualifiers) \n");
#endif
               ROSE_ASSERT(param->type != NULL);

            // Verify that we are not missing any qualifiers (not handled yet, if non-zero).
               ROSE_ASSERT(param->qualifiers == 0);

               type = convert_type(param->type);
             }

          ROSE_ASSERT(type != NULL);

#if 0
          printf ("type (tmp_routine->type) = %p = %s \n",type,type->class_name().c_str());
          printf ("type->unparseToString() = %s \n",type->unparseToString().c_str());
#endif
          param_types.push_back(type);

#ifdef DEBUG_CONVERSION
          indent_line(); debug_print("DONE: Processing function parameter types: case tk_routine\n");
#endif
        }

#if 0
     printf ("In convert_function_type(): Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

#if 0
     if (param_types.size() > 0)
        {
          printf ("part 1: param_types[0]->unparseToString() = %s \n",param_types[0]->unparseToString().c_str());
        }
#endif

  // DQ (10/28/2012): Fixing test2012_107.c (missing ellipsis in function parameter type).
     bool has_ellipsis = extra_info->has_ellipsis;

#if 0
     printf ("In convert_function_type(): has_ellipsis = %s \n",has_ellipsis ? "true" : "false");
#endif

     if (has_ellipsis == true)
        {
#if 0
          printf ("In convert_function_type(): detected ellipsis type parameter in function type \n");
#endif
          SgType* ellipsis_type = SgTypeEllipse::createType();
          param_types.push_back(ellipsis_type);
        }

     SgFunctionParameterTypeList* param_type_list = new SgFunctionParameterTypeList();

#if 0
     printf ("In convert_function_type(): Building a SgFunctionParameterTypeList = %p \n",param_type_list);
#endif

     param_type_list->get_arguments() = param_types;

#if 0
     if (param_types.size() > 0)
        {
          printf ("part 2: param_types[0]->unparseToString() = %s \n",param_types[0]->unparseToString().c_str());
       // printf ("param_type_list->unparseToString() = %s \n",param_type_list->unparseToString().c_str());
        }
#endif

  // DQ (12/30/2008): Use the build function so that the function type table will be updated.
  // SgFunctionType* t = buildFunctionType(return_type,param_type_list);

#if 0
     printf ("Undoing one of my changes to use SgPartialFunctionType instead of SgFunctionType in convert_type(case tk_routine) (take care of the function type table!) \n");
#endif

  // SgFunctionType* t = buildFunctionType(return_type,param_type_list);

#if 0
  // DQ (8/19/2012): I want to remove the use of the SgPartialFunctionType if possible.
     partialFunctionType->set_argument_list(param_type_list);
     param_type_list->set_parent(partialFunctionType);
  // functionType = t;
#endif

     bool is_class_member = false;
     if (associatedFunction != NULL)
        {
          is_class_member = associatedFunction->source_corresp.is_class_member;
        }
#if 0
     printf ("In convert_function_type(): is_class_member = %s \n",is_class_member ? "true" : "false");
#endif

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("process return type\n");
#endif

  // DQ (8/8/2014): Moved this to be handled after the processing of the function
  // parameters. For C++11 support of the new function return type syntax, we sometime 
  // have to process the function parameters before procesing the return type.
     SgType* return_type = nullptr;
     SgType* return_type_undeduced = nullptr;

     std::tie(return_type, return_type_undeduced) = convert_type2(p->variant.routine.return_type, withNondeducedTypes, false, false);

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("DONE: process return type\n");
#endif
#if 0
     printf ("In convert_function_type(): return_type = %p = %s \n",return_type,return_type->class_name().c_str());
#endif

#if 0
     printf ("After processing the return type: Calling summarize_type() \n");
     summarize_type(p->variant.routine.return_type);
#endif

  // DQ (3/8/2012): This appears to always be false (verify this).
  // ROSE_ASSERT(is_class_member == false);

  // DQ (12/17/2011): changed logic to distinguishing if this is associated with a member function (and thus we generate a SgMemberFunctionType).
     bool interpretAsMemberFunction = is_class_member;
     if (is_class_member == true && this_class == NULL)
        {
#ifdef DEBUG_CONVERSION
          printf ("WARNING: I think this may be an error in EDG, but we want to interpret this as a member function \n");
#endif
       // ROSE_ASSERT(false);
        }

  // DQ (12/18/2011): Force this to be treated as a member function when it is explictly communicated from outside this function.
     interpretAsMemberFunction |= pointerToMember;

#if 0
     printf ("In convert_function_type(): pointerToMember = %s interpretAsMemberFunction = %s \n",pointerToMember ? "true" : "false",interpretAsMemberFunction ? "true" : "false");
#endif

  // DQ (7/29/2014): I think this is true (if the pointer is sup is valid).
     ROSE_ASSERT(p->kind == tk_routine);

#if 0
  // DQ (7/28/2014): This appears to be a contributing problem to the handling of test2013_285.C.
  // DQ (7/28/2014): C++11 appears to require the additional logic to set this using these values.
     interpretAsMemberFunction |= (sup->assoc_routine_is_ctor || sup->assoc_routine_is_dtor);
#endif

#if 0
     printf ("In convert_function_type(): sup->assoc_routine_is_ctor = %s sup->assoc_routine_is_dtor = %s interpretAsMemberFunction = %s \n",
          sup->assoc_routine_is_ctor ? "true" : "false",sup->assoc_routine_is_dtor ? "true" : "false",interpretAsMemberFunction ? "true" : "false");
#endif

     SgFunctionType* functionType = nullptr;
     SgFunctionType* functionTypeUndeduced = nullptr;

  // if (this_class == NULL)
     if (interpretAsMemberFunction == false)
        {
#if 0
       // DQ (7/28/2014): This appears to be a contributing problem to the handling of test2013_285.C.
       // DQ (7/28/2014): This is to debug C++11 constructors that are marked as is_defaulted or is_deleted (C++11 syntax).
       // See Cxx11_tests/test2014_05.C for an example of where we are mistakenly computing a SgFunctionType for a 
       // SgMemberFunctionDeclaration (instead of a SgMemberFunctionType).
          ROSE_ASSERT(sup->assoc_routine_is_ctor == false);
          ROSE_ASSERT(sup->assoc_routine_is_dtor == false);
#endif

#if 0
       // This is a C or C++ non-member function
          functionType = SgFunctionType::createType(partialFunctionType);
#else
       // DQ (8/19/2012): Use a mechanism in SageBuilder to make sure that we handle the global function type table consistantly.
       // This will cause function types to be properly shared in the AST.
          functionType = buildFunctionType(return_type, param_type_list);

          if (return_type_undeduced)
          {
            // for now, we just create a copy of the parameter types
            //   without representing any deduced types...
            SgFunctionParameterTypeList* param_type_list2 = new SgFunctionParameterTypeList();

            param_type_list2->get_arguments() = param_types;
            functionTypeUndeduced = buildFunctionType(return_type_undeduced, param_type_list2);

            if (functionTypeUndeduced->get_argument_list() != param_type_list2)
              delete param_type_list2;
          }


#endif

#if 0
          if (param_types.size() > 0)
             {
               printf ("part 3: param_types[0]->unparseToString() = %s \n",param_types[0]->unparseToString().c_str());
             }
          printf ("functionType->unparseToString() = %s \n",functionType->unparseToString().c_str());
#endif

#if 0 // CHECK_EXTRA_INFO
       // a_routine_type_supplement_ptr sup = p_type->variant.routine.extra_info;
       // ROSE_ASSERT(sup != NULL);

       // DQ (1/18/2014): Mark function as noreturn (see test2014_33.c).
       // if (sup->does_not_return == true)
          if (extra_info->does_not_return == true)
             {
               ROSE_ASSERT(functionType != NULL);
            // functionType->setGnuAttributeNoReturn();

            // set_noreturn_attribute( a_type_ptr type, SgInitializedName* initializedName )

#if 0
               printf ("In convert_function_type(): Detected noreturn attribute on function: Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }
#endif // CHECK_EXTRA_INFO
        }
       else
        {
       // This is a C++ member function
       // functionType = SgMemberFunctionType::createType(partialFunctionType);
#if 0 // MEM_FUN_TYPE
          SgMemberFunctionType* memberFunctionType = SgMemberFunctionType::createType(partialFunctionType);
#else
       // DQ (8/19/2012): Use a mechanism in SageBuilder to make sure that we handle the global function type table consistantly.
       // I think this needs to be used in the generation of the member function type.
       // ROSE_ASSERT(associatedClass != NULL);
#if 0 // MEM_FUN_TYPE_2
#if 0
       // SgClassDefinition* associatedClassDefinition = getAssociatedClassDefinitionForMemberFunction(p,false);
       // ROSE_ASSERT(cache.find(this_class) != cache.end());
       // SgClassDeclaration* associatedClassDeclaration = cache[this_class];
       // SgClassDefinition* associatedClassDefinition = getAssociatedClassDefinitionForMemberFunction(p,false);
          ROSE_ASSERT(associatedClassDefinition != NULL);
#else
          SgClassDefinition* associatedClassDefinition = NULL;
          if (associatedClass != NULL)
             {
               SgClassType* associatedClassType = isSgClassType(associatedClass);
               ROSE_ASSERT(associatedClassType != NULL);
               SgClassDeclaration* associatedClassDeclaration = isSgClassDeclaration(associatedClassType->get_declaration());
               ROSE_ASSERT(associatedClassDeclaration != NULL);
               SgClassDeclaration* associatedDefiningClassDeclaration = isSgClassDeclaration(associatedClassDeclaration->get_definingDeclaration());
               ROSE_ASSERT(associatedDefiningClassDeclaration != NULL);
               associatedClassDefinition  = associatedDefiningClassDeclaration->get_definition();
               ROSE_ASSERT(associatedClassDefinition != NULL);
             }
#endif
#endif // MEM_FUN_TYPE_2
#if 0
       // Debugging code.
          if (roseClass == NULL)
             {
               printf ("In convert_function_type(): (roseClass == NULL): calling summarize_type() \n");
               summarize_type(p);

               if (p->kind == tk_routine)
                  {
                    ROSE_ASSERT(p->variant.routine.extra_info != NULL);
                    printf ("In convert_function_type(): (roseClass == NULL): calling summarize_routine() p->variant.routine.extra_info->assoc_routine = %p \n",p->variant.routine.extra_info->assoc_routine);
                    summarize_routine(p->variant.routine.extra_info->assoc_routine);
                  }
             }
#endif

#if 0
          printf ("Calling buildMemberFunctionType(): \n");
          printf (" --- return_type     = %p = %s \n",return_type,return_type->class_name().c_str());
          printf (" --- param_type_list = %p = %s \n",param_type_list,param_type_list->class_name().c_str());
          printf (" --- roseClass       = %p = %s \n",roseClass,roseClass->class_name().c_str());
          printf (" --- mfunc_specifier = %d \n",mfunc_specifier);
#endif
       // SgMemberFunctionType* memberFunctionType = buildMemberFunctionType(return_type, param_type_list, associatedClassDefinition, mfunc_specifier);
          SgMemberFunctionType* memberFunctionType = buildMemberFunctionType(return_type, param_type_list, roseClass, mfunc_specifier);
          ROSE_ASSERT(memberFunctionType != NULL);

#if 0
          printf ("DONE: Calling buildMemberFunctionType(): \n");
#endif

       // printf ("Error: new support using SageBuilder to build the member function type is not implemented yet! \n");
       // ROSE_ASSERT(false);
#endif // MEM_FUN_TYPE
          ROSE_ASSERT(memberFunctionType != NULL);

          if (return_type_undeduced)
          {
            // for now, we just create a copy of the parameter types
            //   without representing any deduced types...
            SgFunctionParameterTypeList* param_type_list2 = new SgFunctionParameterTypeList();

            param_type_list2->get_arguments() = param_types;
            functionTypeUndeduced = buildFunctionType(return_type_undeduced, param_type_list2);

            if (functionTypeUndeduced->get_argument_list() != param_type_list2)
              delete param_type_list2;
          }

          ROSE_ASSERT(sup != NULL);
          if (sup->qualifiers & TQ_CONST)
             {
#if 0
               printf ("##### Mark member function as const. \n");
#endif
               memberFunctionType->setConstFunc();
               // if (functionTypeUndeduced) functionTypeUndeduced->setConstFunc();
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
               memberFunctionType->setVolatileFunc();
               // \todo PP (9/24/25)
               //       should this be a SgMemberFunctionType where we can set
               //       the return type?
               // if (functionTypeUndeduced) functionTypeUndeduced->setVolatileFunc();
             }

       // DQ (3/27/2012): Added new assertion.  This fails on selected cases (e.g. test2009_04.C; I think for static functions).
       // ROSE_ASSERT(memberFunctionType->get_class_type() != NULL);
          if (memberFunctionType->get_class_type() == NULL)
             {
#ifdef DEBUG_CONVERSION
               printf ("WARNING: In convert_function_type(): case of memberFunctionType->get_class_type() == NULL \n");
#endif
             }

          functionType = memberFunctionType;
        }

     ROSE_ASSERT(functionType != NULL);

#if 0
     if (param_types.size() > 0)
        {
          printf ("part 4: param_types[0]->unparseToString() = %s \n",param_types[0]->unparseToString().c_str());
        }
     printf ("functionType->unparseToString() = %s \n",functionType->unparseToString().c_str());
#endif

#if 0
     printf ("functionType->get_argument_list() = %p \n",functionType->get_argument_list());
     printf ("param_type_list                   = %p \n",param_type_list);
#endif

  // DQ (12/5/2012): If we didn't use the SgFunctionParameterTypeList then we need to delete it.
     if (functionType->get_argument_list() != param_type_list)
        {
#if 0
          printf ("Deleting the SgFunctionParameterTypeList if we didn't use it. \n");
#endif
          delete param_type_list;
          param_type_list = NULL;
        }

     ROSE_ASSERT(functionType->get_argument_list() != NULL);

  // Are these preserved across the translation of SgPartialFunctionType to either SgFunctionType or a SgMemberFunctionType?
  // DQ (11/6/2011): Modifiers are not used for function types this way (the function holds the const modifier as part of 
  // its declaration modifier.  Modifiers are not used with function types.
  // functionType = isSgFunctionType(apply_qualifiers(functionType, sup->qualifiers, UPC_BLOCK_SIZE_NONE));

  // Temp code to test 2009_06.C
  // ROSE_ASSERT(isSgMemberFunctionType(functionType) != NULL);

  // This should have been converted to either a SgFunctionType or a SgMemberFunctionType
     ROSE_ASSERT(isSgPartialFunctionType(functionType) == NULL);

#if 0
     printf ("functionType (tmp_routine->type) = %p = %s \n",functionType,functionType->class_name().c_str());
     printf ("functionType->unparseToString() = %s \n",functionType->unparseToString().c_str());
#endif
#if 0
     printf ("Leaving convert_function_type(): functionType = %p = %s \n",functionType,functionType->class_name().c_str());
#endif

     return {functionType, functionTypeUndeduced};
   }
