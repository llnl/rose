
#include "edgRose.h"

namespace EDG_ROSE_Translation {

#define DEBUG_NEW_DELETE_SUPPORT 0

SgExpression* convert_expression__enk_new_delete(an_expr_node_ptr expr)
   {
     SgExpression* result = NULL;

     ROSE_ASSERT (expr != NULL);

     a_new_delete_supplement_ptr ndsp = expr->variant.new_delete;

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(ndsp != NULL);

     a_type_ptr                  type             = ndsp->type;
     a_type_ptr                  unqual_type      = NULL;
  // a_routine_ptr               routine          = ndsp->routine;
     an_expr_node_ptr            arg              = ndsp->arg;
  // a_boolean                   need_type_parens = false;

     bool need_global_specifier = false;

  // for new
     SgExprListExp* placement = NULL;
     SgType*        theType   = NULL;
     SgExpression*  theArg    = NULL;
     SgExpression*  theIdx    = NULL;
     SgArrayType*   theArray  = NULL;

  // for delete
     SgExpression* theExpr    = NULL;

  // DQ (6/11/2013): Added to support new location of array size (change in IR for EDG 4.5).
     an_expr_node_ptr number_of_elements = ndsp->number_of_elements;

  // DQ (6/11/2013): Commented out as part of move to EDG 4.7.
  // ROSE_ASSERT (arg != NULL);

  // DataRequiredForComputationOfSourcePostionInformation sourcePositionData(expr);

     unqual_type = skip_typerefs(type);

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(unqual_type != NULL);

#if DEBUG_NEW_DELETE_SUPPORT || 0
     printf ("In convert_expression__enk_new_delete(): calling summarize_new_delete_supplement() \n");
     summarize_new_delete_supplement(ndsp);
#endif
#if DEBUG_NEW_DELETE_SUPPORT || 0
     printf ("*** In convert_expression__enk_new_delete(): ndsp->routine = %p \n",ndsp->routine);
#endif

#if 0
     printf ("Calling convert_argument_list() \n");
     SgExprListExp* arguementList = convert_argument_list(ndsp,arg);
     printf ("DONE: Calling convert_argument_list() \n");
#endif

     SgFunctionDeclaration* newOrDeleteFunctionDeclaration = NULL;
     if (ndsp->routine != NULL)
        {
       // This only appears to be true for the new operator and only if the new 
       // operator is explicitly declared (either globally or in a class).
       // SgFunctionSymbol* functionSymbol = isSgFunctionSymbol(ndsp->routine->source_corresp.sg_symbol_ptr);
       // SgFunctionSymbol* functionSymbol = function_cache[ndsp->routine];

       // DQ (11/25/2011): Modified to support uniform handling of SgFunctionDeclaration and SgTemplateFunctionDeclaration IR nodes.
          SgSymbol* symbol = function_cache[ndsp->routine];
       // if (functionSymbol != NULL)
          if (symbol != NULL)
             {
            // SgMemberFunctionSymbol* memberFunctionSymbol = isSgMemberFunctionSymbol(functionSymbol);
               SgMemberFunctionSymbol* memberFunctionSymbol = isSgMemberFunctionSymbol(symbol);
               if (memberFunctionSymbol != NULL)
                  {
                    newOrDeleteFunctionDeclaration = memberFunctionSymbol->get_declaration();
                    ROSE_ASSERT(newOrDeleteFunctionDeclaration != NULL);
                 // DQ (3/7/2012): Added testing (debugging test2004_77.C).
                    ROSE_ASSERT(newOrDeleteFunctionDeclaration->get_symbol_from_symbol_table() != NULL);
                  }
                 else
                  {
                    SgFunctionSymbol* functionSymbol = isSgFunctionSymbol(symbol);
                    if (functionSymbol != NULL)
                       {
                         newOrDeleteFunctionDeclaration = functionSymbol->get_declaration();

                      // DQ (3/7/2012): Added testing (debugging test2004_77.C).
                         ROSE_ASSERT(newOrDeleteFunctionDeclaration->get_symbol_from_symbol_table() != NULL);
                       }
                      else
                       {
                         printf ("Case of SgTemplateSymbol not handled symbol! \n");

                         printf ("Error: default case not handled: symbol = %p = %s \n",symbol,symbol->class_name().c_str());
                         ROSE_ASSERT(false);
                       }
                  }
             }
        }

#if DEBUG_NEW_DELETE_SUPPORT || 0
     printf ("*** In convert_expression__enk_new_delete(): ndsp->is_new = %s \n",ndsp->is_new ? "true" : "false");
#endif

     if (ndsp->is_new == true)
        {
       // New.  The general form is
       //   :: new (arg2, arg3, ...) type(initializer)
       // Everything except "new" and the type is optional.

#if 0
       // Generate an empty constructor argument list
          SgExprListExp* placement = set_generated_file_info_to_unparse(buildExprListExp_nfi());
#if DEBUG_NEW_DELETE_SUPPORT
          printf ("In convert_expression__enk_new_delete(): ndsp->placement_new = %s \n",ndsp->placement_new ? "true" : "false");
#endif
       // if (ndsp->placement_new == true)
             {
            // DQ (11/17/2012): Make sure that we don't grap the expression from the source sequence list and 
            // put it into the placement arges if this is not a new expression using placement args.

               ROSE_ASSERT (arg != NULL);

            // Add any available arguments to the constructor argument list
               for (an_expr_node_ptr argument = arg; argument != NULL; argument = argument->next)
                  {
                 // bool isCompilerGeneratedConstantForUndefinedConstructor = ( (ndsp->routine != NULL) && (argument->kind == enk_constant) && (argument->variant.constant->source_corresp.decl_position.seq == 0) );
                    bool isCompilerGeneratedConstantForUndefinedConstructor = ( (ndsp->routine == NULL) && (argument->kind == enk_constant) && (argument->variant.constant->source_corresp.decl_position.seq == 0) );
#if DEBUG_NEW_DELETE_SUPPORT
                    printf ("(placement) isCompilerGeneratedConstantForUndefinedConstructor = %s \n",isCompilerGeneratedConstantForUndefinedConstructor ? "true" : "false");
#endif
                    if (isCompilerGeneratedConstantForUndefinedConstructor == false)
                       {
                         SgExpression* expression = set_generated_file_info_to_unparse(convert_expression(argument));
#if DEBUG_NEW_DELETE_SUPPORT
                         printf ("expression in placement (expression list) = %p = %s \n",expression,expression->class_name().c_str());
#endif
                         appendExpression(placement, expression);
                       }
                  }
             }
#else
       // printf ("Calling convert_argument_list() \n");
       // SgExprListExp* placement = convert_argument_list(ndsp,arg);
       // SgExprListExp* placement = arguementList;
          if (ndsp->placement_new == true)
             {
            // DQ (6/12/2013): Added assertion to debug EDG 4.7 work.
               ROSE_ASSERT (arg != NULL);

#if DEBUG_NEW_DELETE_SUPPORT || 0
               printf ("In convert_expression__enk_new_delete(): ndsp->placement_new == true: Calling convert_argument_list() \n");
#endif
               placement = convert_argument_list(ndsp,arg);

            // DQ (1/19/2020): added assertion for debugging.
               ROSE_ASSERT(placement != NULL);

#if DEBUG_NEW_DELETE_SUPPORT || 0
               printf ("DONE: In convert_expression__enk_new_delete(): ndsp->placement_new == true: Calling convert_argument_list(): size = %zu \n",placement->get_expressions().size());
#endif
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }
#endif

       // The type is easy to put out except when it is a variable-length array type.
#if DEBUG_NEW_DELETE_SUPPORT || 0
          printf ("*** In convert_expression__enk_new_delete(): unqual_type->size = %" PRIuPTR " (normal case if non-zero) \n",(size_t) unqual_type->size);
#endif
       // The size is the value of sizeof() for this type, or 0 if the type is incomplete.  Also 0 for typeref references, even
       // if the referenced type is not incomplete.  In GNU C mode, zero-length arrays and empty structs and unions can have 
       // size 0 and be complete at the same time (the bound_is_zero or is_empty_class flags must be set in those cases).
          if (unqual_type->size != 0)
             {
            // Normal case
               theType = convert_type(type);
               ROSE_ASSERT(theType != NULL);
             }
            else
             {
            // DQ (3/13/2012): Note that when size == 0 it can mean a number of things...not just an array of size zero...I think...
#if DEBUG_NEW_DELETE_SUPPORT || 0
               printf ("*** In convert_expression__enk_new_delete(): unqual_type->kind = %s \n",summarize_type_kind_in_string_kind(unqual_type->kind));
#endif
               switch (unqual_type->kind)
                  {
                    case tk_array:
                       {
                      // We should not be using a C-stype CAST!!!!
                      // theArray = (SgArrayType*) convert_type(unqual_type);

                      // reset the indexExpression later on (after it is built).
                         theArray = isSgArrayType(convert_type(unqual_type));
                         ROSE_ASSERT(theArray != NULL);

                      // DQ (5/17/2012): Must set the type of the array.
                      // theType = convert_type(type);
                      // theType = theArray;

                      // DQ (2/21/2012): If theArray is valid then theType should be NULL, so this is OK.
                      // DQ (11/23/2011): I don't see how this will pass, I expect we have not tested this case yet!
                      // ROSE_ASSERT(theType != NULL);

                      // Harder case: a variable-length array.  The first argument expression
                      // gives the size of the array, which is the number of elements times
                      // the size of each element.  Put out the first part of the type, the
                      // open bracket, the expression, and the close bracket.
                      // a_type_ptr       elem_type;

                      // a_targ_size_t    elem_size;

                      // an_expr_node_ptr num_elems_expr;
#if DEBUG_NEW_DELETE_SUPPORT || 0
                         printf ("In convert_expression__enk_new_delete(): unqual_type->kind = %s \n",summarize_type_kind_in_string_kind(unqual_type->kind));
#endif
                      // DQ (3/13/2012): For test2004_85.C this assertion falis with unqual_type->kind == tk_struct.
                      // check_assertion_str(unqual_type->kind == (a_type_kind)tk_array,"gen_new_delete: zero-sized type not array");
                         ROSE_ASSERT(unqual_type->kind == (a_type_kind)tk_array);
                         if (ndsp->placement_new == true)
                            {
                           // If this is a placement new operator then the first argument is used as the array length, and so we want to take that off of the placement list.
                              ROSE_ASSERT(placement != NULL);
                              ROSE_ASSERT(placement->get_expressions().empty() == false);

#if DEBUG_NEW_DELETE_SUPPORT || 0
                              printf ("In case of placement_new == true: use the first expression in the args list as the array size: placement.size() = %" PRIuPTR " \n",
                                   placement->get_expressions().size());
#endif
                              theIdx = *(placement->get_expressions().begin());
                              placement->get_expressions().erase(placement->get_expressions().begin());

#if DEBUG_NEW_DELETE_SUPPORT || 0
                              printf ("In case of placement_new == true: after removing the first element: placement.size() = %" PRIuPTR " \n",placement->get_expressions().size());
#endif
                           // I think that this is required for an array type, it will be reset by SgArrayType::createType().
                              theIdx->set_parent(NULL);
#if 0
                              printf ("Exiting as a test! \n");
                              ROSE_ASSERT(false);
#endif
                            }
                           else
                            {
                           // DQ (6/11/2013): Note that EDG 4.5 has changed the IR and the location of the expression for the array size.
                              ROSE_ASSERT(arg == NULL);
                              ROSE_ASSERT(number_of_elements != NULL);
                              theIdx = convert_expression(number_of_elements);
                            }

                      // DQ (2/21/2012): This is not used, so remove it.
                      // SgUnsignedLongVal* unsignedLongVal = buildUnsignedLongVal_nfi(elem_size,"");
                      // ROSE_ASSERT (unsignedLongVal != NULL);

                      // DQ (8/11/2010): Moved the construction of the SgArrayType from the top of this block.
                      // ROSE_ASSERT(theIdx != NULL);
                      // theArray = (a_SgArrayType_ptr )sage_gen_type(unqual_type); // reset the indexExpression later on

                      // DQ (8/11/2010): This is a problem for the global type table since the index needs to be used in 
                      // the mangled type name and the type has already been built without the index.  We have to unload
                      // the global type table and rebuild the type with a valid type.

                      // DQ (8/11/2010): Unload the global type table.
                      // printf ("Don't forget to unload the unused array type without the index. \n");

                         SgType* arrayBaseType = theArray->get_base_type();
                         ROSE_ASSERT(arrayBaseType != NULL);

                      // Delete the associated array type. But only if we unload it from the type table AND the mangled name table!
                      // It is simpler to just build a new type.
                      // delete theArray;
                         theArray = NULL;

                      // DQ (9/25/2011): If the array is identified to already exist, then the shared 
                      // array type will be returned and the idex expression deleted (as a side-effect).
                      // build a new array type with the proper index.
                         theArray = SgArrayType::createType( arrayBaseType, theIdx );
#if 0
                      // DQ (1/27/2015): debugging test2015_05.C.
                         printf ("Call to SgArrayType::createType(): theArray = %p arrayBaseType = %p = %s theIdx = %p = %s \n",
                              theArray,arrayBaseType,arrayBaseType->class_name().c_str(),theIdx,theIdx->class_name().c_str());
#endif
                      // DQ (9/25/2011): Because of the semantics of SgArrayType::createType() on the index expression, 
                      // we have to access the index expression via the array type, since theIdx may have been deleted.
                      // ROSE_ASSERT(theIdx->get_parent() != NULL);
                         ROSE_ASSERT(theArray != NULL);
                         ROSE_ASSERT(theArray->get_index() != NULL);
                         ROSE_ASSERT(theArray->get_index()->get_parent() != NULL);

                         ROSE_ASSERT(theArray->get_index()->get_startOfConstruct() != NULL);

                      // printf ("Built a new array type using new index: theArray = %p \n",theArray);
                         break;
                       }

                 // DQ (1/26/2013): Adding handling fo tk_class (detected as problem in ROSE compiling ROSE header files).
                 // We might have to add tk_union, but I would like to see an example of this as a test code.
                    case tk_class:

                    case tk_struct:
                       {
                      // DQ (3/13/2012): I think this is all that we need.
                         theType = convert_type(type);
                         ROSE_ASSERT(theType != NULL);
                         break;
                       }

                    default:
                       {
                         printf ("In convert_expression__enk_new_delete(): Default reached in switch unqual_type->kind = %s \n",summarize_type_kind_in_string_kind(unqual_type->kind));
                         ROSE_ASSERT(false);
                       }
                  }
             }

#if DEBUG_NEW_DELETE_SUPPORT || 0
          printf ("In convert_expression__enk_new_delete(): ndsp->dynamic_init = %p \n",ndsp->dynamic_init);
#endif
          if (ndsp->dynamic_init != NULL)
             {
            // The allocated entity gets initialized.

               ROSE_ASSERT(theType != NULL || theArray != NULL);
#if 1
#if 0
               printf ("+++++++++++++++++++++++ Calling convert_dynamic_initializer() \n");
#endif
               theArg = convert_dynamic_initializer(ndsp->dynamic_init,type);
#if 0
               printf ("+++++++++++++++++++++++ DONE: Calling convert_dynamic_initializer() theArg = %p = %s \n",theArg,theArg != NULL ? theArg->class_name().c_str() : "null");
#endif
               SgConstructorInitializer* constInit = isSgConstructorInitializer(theArg);
               if (constInit != NULL)
                  {
                    constInit->set_need_parenthesis_after_name(true);
                  }
                 else
                  {
#if DEBUG_NEW_DELETE_SUPPORT || 0
                    printf ("NOTE: In convert_expression__enk_new_delete(): (constInit != NULL): theArg = %p = %s \n",theArg,theArg != NULL ? theArg->class_name().c_str() : "null");
#endif
                  }
#else
            // SgConstructorInitializer* constInit = set_generated_file_info_to_unparse(SageBuilder::buildConstructorInitializer_nfi(NULL,NULL,constructorType,false,false,false,isSgClassType(constructorType) == NULL ? true : false));

#error "DEAD CODE!"

               SgType* constructorType = (theType  != NULL) ? theType : theArray;
               ROSE_ASSERT(constructorType != NULL);

            // SgConstructorInitializer* constInit = set_generated_file_info_to_unparse(SageBuilder::buildConstructorInitializer_nfi(NULL,NULL,constructorType,false,false,false,isSgClassType(constructorType) == NULL ? true : false));
            // SgConstructorInitializer* constInit = set_generated_file_info_to_unparse(SageBuilder::buildConstructorInitializer_nfi(NULL,placement,constructorType,false,false,false,isSgClassType(constructorType) == NULL ? true : false));
               SgConstructorInitializer* constInit = set_generated_file_info_to_unparse(SageBuilder::buildConstructorInitializer_nfi(NULL,arguementList,constructorType,false,false,false,isSgClassType(constructorType) == NULL ? true : false));
#if DEBUG_NEW_DELETE_SUPPORT
               printf ("After call to buildConstructorInitializer_nfi() 1b: constInit = %p \n",constInit);
#endif
               theArg = constInit;

            // DQ (11/17/2012): This way of using the SgConstructorInitializer requires the use of parenthesis.
               constInit->set_need_parenthesis_after_name(true);
#endif

#if 0
            // DQ (1/19/2019): Test the expresion type in the SgConstructorInitializer.
               if (constInit != NULL)
                  {
                    SgType* expression_type = constInit->get_expression_type();
                    if (expression_type != NULL)
                       {
                         printf ("In convert_expression__enk_new_delete(): (constInit != NULL): expression_type = %p = %s \n",expression_type,expression_type->class_name().c_str());
                       }
                  }
#endif
               ROSE_ASSERT (theArg != NULL);
             }
            else
             {
            // DQ (8/5/2006): I think we want to assert that this is valid.
               ROSE_ASSERT(theType != NULL || theArray != NULL);

               SgType* constructorType = (theType  != NULL) ? theType : theArray;
               ROSE_ASSERT(constructorType != NULL);

            // theArg = new SgConstructorInitializer(NULL,NULL,constructorType,false,false,false,isSgClassType(constructorType) == NULL ? true : false);
            // theArg = set_generated_file_info_to_unparse(SageBuilder::buildConstructorInitializer_nfi(NULL,NULL,constructorType,false,false,false,isSgClassType(constructorType) == NULL ? true : false));
               SgConstructorInitializer* constInit = set_generated_file_info_to_unparse(SageBuilder::buildConstructorInitializer_nfi(NULL,NULL,constructorType,false,false,false,isSgClassType(constructorType) == NULL ? true : false));
               ROSE_ASSERT (constInit != NULL);

#if DEBUG_NEW_DELETE_SUPPORT || 0
               printf ("After call to buildConstructorInitializer_nfi() 1: constInit = %p \n",constInit);
#endif
            // Since we set the file info after the IR node construction, we have to process the empty argument list.
               constInit->set_args(set_generated_file_info_to_unparse(constInit->get_args()));

               theArg = constInit;
               ROSE_ASSERT (theArg != NULL);

#if DEBUG_NEW_DELETE_SUPPORT
               printf ("In convert_expression__enk_new_delete(): theArg = new SgConstructorInitializer = %p \n",theArg);
#endif
            // DQ (3/3/2007): Added assertions
               ROSE_ASSERT( (theArg->get_startOfConstruct() == NULL && theArg->get_endOfConstruct() == NULL) || 
                            (theArg->get_startOfConstruct() != NULL && theArg->get_endOfConstruct() != NULL) );
             }

          if (theArg != NULL)
             {
            // DQ (3/3/2007): Added assertions
               ROSE_ASSERT( (theArg->get_startOfConstruct() == NULL && theArg->get_endOfConstruct() == NULL) || 
                            (theArg->get_startOfConstruct() != NULL && theArg->get_endOfConstruct() != NULL) );
            // sourcePositionData.initializeSourcePositionInformation(theArg);
             }

       // DQ (2/21/2012): Added source position informaction check.
          if (theArg != NULL)
             {
               ROSE_ASSERT (theArg->get_startOfConstruct() != NULL);
             }
        }
       else
        {
       // Delete.  The general form is
       //    :: delete [] expression
       // Everything except "delete" and the expression is optional.
       // theExpr = sage_gen_expr_with_parens(arg,NULL);

       // DQ (6/12/2013): Added assertion to debug EDG 4.7 work.
          ROSE_ASSERT (arg != NULL);

          theExpr = convert_expression(arg);

       // DQ (5/29/2004): This has not been set so it should still be NULL, check it
          ROSE_ASSERT (theArg == NULL);
        }

#if DEBUG_NEW_DELETE_SUPPORT || 0
     printf ("In convert_expression__enk_new_delete(): ndsp->is_new = %s \n",ndsp->is_new ? "true" : "false");
     printf ("In convert_expression__enk_new_delete(): theArg = %p = %s \n",theArg,(theArg != NULL) ? theArg->class_name().c_str() : "null");
#endif

     if (ndsp->is_new == true)
        {
       // Note: theArg will be NULL for the case of an array constructor initialization (which used the default constructor)
          SgConstructorInitializer *isCI = isSgConstructorInitializer(theArg);
       // SgInitializer *isCI = isSgInitializer(theArg);

          SgConstructorInitializer *emptyConstructorInitializer = NULL;

#if DEBUG_NEW_DELETE_SUPPORT
          printf ("In convert_expression__enk_new_delete(): isCI = %p \n",isCI);
#endif

       // DQ (1/19/2019): Add a test to see if theArg is always a valid pointer. If so then we can eliminate the branch below.
          ROSE_ASSERT(theArg != NULL);

       // DQ (1/19/2019): There can be a valid theArg, just that it might be a SgAggregateInitializer instead of a SgConstructorInitializer.
       // In this case we don't want to overwrite it in the argument list for the new operator. So only support this branch if we need to (if theArg is NULL)..
       // if (isCI == NULL)
       // if (theArg == NULL)
          if (isCI == NULL)
             {
#if DEBUG_NEW_DELETE_SUPPORT || 0
               printf ("In convert_expression__enk_new_delete(): Building a new constructor initializer \n");
#endif

            // emptyConstructorInitializer = new SgConstructorInitializer(NULL,NULL,SgTypeDefault::createType(),false,false,false,true);
            // SgConstructorInitializer* SageBuilder::buildConstructorInitializer(SgMemberFunctionDeclaration *declaration/* = NULL*/, SgExprListExp *args/* = NULL*/, 
            //      SgType *expression_type/* = NULL*/, bool need_name /*= false*/, bool need_qualifier /*= false*/, 
            //      bool need_parenthesis_after_name /*= false*/, bool associated_class_unknown /*= false*/);
            // emptyConstructorInitializer = SageBuilder::buildConstructorInitializer(NULL,NULL,SgTypeDefault::createType(),false,false,false,true);
               emptyConstructorInitializer = set_generated_file_info_to_unparse(SageBuilder::buildConstructorInitializer_nfi(NULL,NULL,SgTypeDefault::createType(),false,false,false,true));
               ROSE_ASSERT(emptyConstructorInitializer != NULL);

#if DEBUG_NEW_DELETE_SUPPORT || 0
               printf ("After call to buildConstructorInitializer_nfi() 1a: emptyConstructorInitializer = %p \n",emptyConstructorInitializer);
#endif
#if 0
               emptyConstructorInitializer = set_generated_file_info_to_unparse(emptyConstructorInitializer);
               ROSE_ASSERT(emptyConstructorInitializer != NULL);
#endif
            // sourcePositionData.initializeSourcePositionInformation(emptyConstructorInitializer);

            // DQ (2/12/2012): Set and then test the source code position info.
            // Since we set the file info after the IR node construction, we have to process the empty argument list.
               set_generated_file_info_to_unparse(emptyConstructorInitializer->get_args());
               ROSE_ASSERT(emptyConstructorInitializer->get_args()->get_startOfConstruct() != NULL);

#if DEBUG_NEW_DELETE_SUPPORT
               printf ("In convert_expression__enk_new_delete(): emptyConstructorInitializer = new SgConstructorInitializer = %p \n",emptyConstructorInitializer);
#endif
#if DEBUG_NEW_DELETE_SUPPORT
               printf ("In convert_expression__enk_new_delete(): ndsp->has_new_initializer = %s \n",ndsp->has_new_initializer ? "true" : "false");
#endif
               if (ndsp->has_new_initializer == true)
                  {
                 // DQ (11/17/2012): Make sure that we don't grab the expression from the source sequence list and 
                 // put it into the placement args if this is not a new expression using placement args.

                 // DQ (11/18/2012): Add the arg to the new SgConstructorInitializer.
                    emptyConstructorInitializer->get_args()->append_expression(theArg);

                 // This is to fix test2005_24.C
                    theArg->set_parent(emptyConstructorInitializer->get_args());

                 // DQ (11/17/2012): This way of using the SgConstructorInitializer requires the use of parenthesis.
                    emptyConstructorInitializer->set_need_parenthesis_after_name(true);

#if DEBUG_NEW_DELETE_SUPPORT
                    printf ("In convert_expression__enk_new_delete(): after being set: emptyConstructorInitializer->get_need_parenthesis_after_name() = %s \n",
                         emptyConstructorInitializer->get_need_parenthesis_after_name() ? "true" : "false");
#endif
                  }
                 else
                  {
                 // This is to fix test2005_24.C
                    theArg->set_parent(emptyConstructorInitializer->get_args());

                 // DQ (1/19/2019): I think we need to add theArg to the argument list of the emptyConstructorInitializer.
                    ROSE_ASSERT(emptyConstructorInitializer != NULL);
                    emptyConstructorInitializer->get_args()->append_expression(theArg);
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }

#if DEBUG_NEW_DELETE_SUPPORT
               printf ("In convert_expression__enk_new_delete(): emptyConstructorInitializer->get_need_parenthesis_after_name() = %s \n",
                    emptyConstructorInitializer->get_need_parenthesis_after_name() ? "true" : "false");
#endif
             }

       // XOR these pointer values!
          ROSE_ASSERT(isCI != NULL || emptyConstructorInitializer != NULL);
          ROSE_ASSERT(isCI == NULL || emptyConstructorInitializer == NULL);

#if DEBUG_NEW_DELETE_SUPPORT || 0
          printf ("In convert_expression__enk_new_delete(): theArray                       = %p \n",theArray);
          printf ("In convert_expression__enk_new_delete(): theType                        = %p = %s \n",theType,(theType != NULL) ? theType->class_name().c_str() : "NULL");
          printf ("In convert_expression__enk_new_delete(): placement                      = %p = %s \n",placement,(placement != NULL) ? placement->class_name().c_str() : "NULL");
          printf ("In convert_expression__enk_new_delete(): emptyConstructorInitializer    = %p \n",emptyConstructorInitializer);
          printf ("In convert_expression__enk_new_delete(): newOrDeleteFunctionDeclaration = %p \n",newOrDeleteFunctionDeclaration);
#endif
          if (theArray != NULL)
             {
               if (isCI != NULL)
                  {
                    result = SageBuilder::buildNewExp(theArray, placement, isCI, NULL, need_global_specifier, newOrDeleteFunctionDeclaration);
                  }
                 else
                  {
                    result = SageBuilder::buildNewExp(theArray, placement, emptyConstructorInitializer, NULL, need_global_specifier, newOrDeleteFunctionDeclaration);
                  }
             }
            else
             {
               if (isCI != NULL)
                  {
                    result = SageBuilder::buildNewExp(theType, placement, isCI, NULL, need_global_specifier, newOrDeleteFunctionDeclaration);
                  }
                 else
                  {
                    result = SageBuilder::buildNewExp(theType, placement, emptyConstructorInitializer, NULL, need_global_specifier, newOrDeleteFunctionDeclaration);
                  }
             }

          ROSE_ASSERT (isSgNewExp(result) != NULL);
        }
       else
        {
          result = SageBuilder::buildDeleteExp(theExpr, (bool) ndsp->array_delete, (bool) need_global_specifier, newOrDeleteFunctionDeclaration);
        }

     ROSE_ASSERT (result != NULL);

     ROSE_ASSERT (result->get_startOfConstruct() != NULL);

  // DQ (2/25/2012): Added test.
     if (theArg != NULL)
        {
          ROSE_ASSERT (theArg->get_parent() != NULL);
        }

#if 0
     printf ("Leaving convert_expression__enk_new_delete(): possible source of SgConstructorInitializer result = %p \n",result);
#endif

     return result;
   }
}
