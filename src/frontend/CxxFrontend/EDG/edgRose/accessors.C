
#include "edgRose.h"

namespace EDG_ROSE_Translation {

void putFunctionInTemplateMap(a_routine_ptr p, a_template_ptr edg_template, SgFunctionDeclaration* nondefiningFunctionDeclaration, SgFunctionDeclaration* definingFunctionDeclaration)
   {
     ROSE_ASSERT(p != NULL);
     assert(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() == nondefiningFunctionDeclaration);

     if (nondefiningFunctionDeclaration != NULL)
        {
       // DQ (4/9/2012): Add this to the template cache
          if (edg_template != NULL)
             {
               if (!hasFirstNondefDeclForTemplate(edg_template))
                  {
                 // DQ (8/15/2013): Added assertions to support more precise template handling.
                    ROSE_ASSERT(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
                    if (nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() != nondefiningFunctionDeclaration)
                       {
#if DEBUG_NONDEFINING_TEMPLATE_DECLARATION_MAP
                         printf ("WARNING: nondefiningFunctionDeclaration = %p != nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() = %p = %s = %s and first non-defining declaration was not previously added to the cache \n",
                              nondefiningFunctionDeclaration,nondefiningFunctionDeclaration->get_firstNondefiningDeclaration(),nondefiningFunctionDeclaration->class_name().c_str(),nondefiningFunctionDeclaration->get_name().str());
#endif
                       }
#if DEBUG_NONDEFINING_TEMPLATE_DECLARATION_MAP
                    printf ("In putFunctionInTemplateMap(): Adding nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() = %p = %s to first_nondefining_declarations_for_templates[kep = %p] \n",
                         nondefiningFunctionDeclaration->get_firstNondefiningDeclaration(),nondefiningFunctionDeclaration->get_firstNondefiningDeclaration()->class_name().c_str(),edg_template);
#endif
                    setFirstNondefDeclForTemplate(edg_template, isSgFunctionDeclaration(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration()));
                  }
             }
            else
             {
            // It appears that this case can happen when called from convert_routine_prototype() (e.g. test2005_06.C)
#ifdef DEBUG_CONVERSION
               printf ("In putFunctionInTemplateMap(): edg_template == NULL (might be an error) \n");
#endif
             }

       // DQ (8/10/2012): Add function declarations to new template function map (using a_routine_ptr value instead of a_template_ptr values.
          if (first_nondefining_declarations_for_template_functions.find(p) == first_nondefining_declarations_for_template_functions.end())
             {
            // DQ (8/15/2013): Added assertions to support more precise template handling.  But there is no requirement 
            // that nondefiningFunctionDeclaration be the first nondefining declaration, so this is OK (I think).
               ROSE_ASSERT(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
               if (nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() != nondefiningFunctionDeclaration)
                  {
#if DEBUG_NONDEFINING_TEMPLATE_DECLARATION_MAP
                    printf ("WARNING: nondefiningFunctionDeclaration = %p = %s nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() = %p \n",
                         nondefiningFunctionDeclaration,nondefiningFunctionDeclaration->class_name().c_str(),nondefiningFunctionDeclaration->get_firstNondefiningDeclaration());
#endif
                  }

#if DEBUG_NONDEFINING_TEMPLATE_DECLARATION_MAP
               printf ("In putFunctionInTemplateMap(): Adding nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() = %p = %s to first_nondefining_declarations_for_templates[kep = %p] \n",
                  nondefiningFunctionDeclaration->get_firstNondefiningDeclaration(),nondefiningFunctionDeclaration->get_firstNondefiningDeclaration()->class_name().c_str(),p);
#endif
               first_nondefining_declarations_for_template_functions[p] = isSgFunctionDeclaration(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration());
             }
        }

     if (definingFunctionDeclaration != NULL)
        {
       // DQ (5/22/2013): This assert fails for test2013_180.C (non-template friend function declared in templated class).
       // Maybe it should have been represented as a template instantiation?
       // This assertion is only for the defining functions.
          if (edg_template != NULL)
             {
            // DQ (4/9/2012): Add this to the template cache
               if (!hasDefiningDeclForTemplate(edg_template))
                  {
#if 0
                    printf ("Adding definingFunctionDeclaration = %p to defining_declarations_for_templates[edg_template = %p] \n",definingFunctionDeclaration,edg_template);
#endif
                 // DQ (8/15/2013): Added assertions to support more precise template handling.
                    ROSE_ASSERT(definingFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
                    ROSE_ASSERT(definingFunctionDeclaration->get_firstNondefiningDeclaration() != definingFunctionDeclaration);
                    ROSE_ASSERT(definingFunctionDeclaration->get_definingDeclaration() != NULL);
                    ROSE_ASSERT(definingFunctionDeclaration->get_definingDeclaration() == definingFunctionDeclaration);

                    setDefiningDeclForTemplate(edg_template, definingFunctionDeclaration);
                  }

            // DQ (8/10/2012): Add function declarations to new template function map (using a_routine_ptr value instead of a_template_ptr values.
               if (defining_declarations_for_template_functions.find(p) == defining_declarations_for_template_functions.end())
                  {
                 // DQ (8/15/2013): Added assertions to support more precise template handling.
                    ROSE_ASSERT(definingFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
                    ROSE_ASSERT(definingFunctionDeclaration->get_firstNondefiningDeclaration() != definingFunctionDeclaration);
                    ROSE_ASSERT(definingFunctionDeclaration->get_definingDeclaration() != NULL);
                    ROSE_ASSERT(definingFunctionDeclaration->get_definingDeclaration() == definingFunctionDeclaration);

                    defining_declarations_for_template_functions[p] = definingFunctionDeclaration;
                  }
             }
            else
             {
#ifdef DEBUG_CONVERSION
               printf ("WARNING: In putFunctionInTemplateMap(): edg_template == NULL: case of SgTemplateDeclaration without an associated edg_template \n");
#endif
             }
        }

     if (edg_template != NULL)
        {
          ROSE_ASSERT( hasFirstNondefDeclForTemplate(edg_template) || hasDefiningDeclForTemplate(edg_template) );
        }
   }

std::string get_name_for_enumerator(a_type_ptr enum_type, an_integer_value value)
   {
     ROSE_ASSERT (enum_type->kind == tk_integer && enum_type->variant.integer.enum_type);

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("get name for enumerator '%p' \n", enum_type);
#endif

     for (a_constant_ptr enumerators = enum_type->variant.integer.enum_info.constant_list; enumerators != NULL; enumerators = enumerators->next)
        {
       // DQ (2/8/2011): Added assert for what I think is true. There should be no hidden expression trees from constant folded expressions.
          if (enumerators->expr != NULL)
             {
#ifdef DEBUG_CONVERSION
               printf ("WARNING: In get_name_for_enumerator(): enumerators->expr != NULL (may be missing hidden expression trees) \n");
#endif
             }

#if 0
          printf ("In get_name_for_enumerator(): calling summarize_type() \n");
          summarize_type(enum_type);
#endif
#if 0
          printf ("In get_name_for_enumerator(): calling summarize_constant() \n");
          summarize_constant(enumerators);
#endif

       // DQ (1/28/2017): The case of a strongly typed enum (C++11 feature) will have the enum constant marked as ck_error.
          if (enumerators->kind != ck_error)
             {
               ROSE_ASSERT (enumerators->kind == ck_integer);
               if (value == enumerators->variant.integer_value)
                  {
                    return enumerators->source_corresp.name;
                  }
             }
        }
#if 0
     printf ("In get_name_for_enumerator(): calling summarize_type() \n");
     summarize_type(enum_type);
#endif
#if 0
     printf ("WARNING: This enumerator does not have a representative entry in the enum type (but it appears this is allowed). \n");
#endif
  // DQ (8/31/2012): Oddly enough, tis appears to be allowed, in which case there is no associated name (just the value).
  // See test2012_202.C for an example of this.
     
     
  // Likely this should be represented as a cast...
     SgName alternativeName;
     SgName enumClassName = generateName(enum_type);

  // DQ (11/5/2012): Make the type of cast depending on the source language.
  // This is addressing test2012_167.c
  // Note that this should maybe be constructed in the unparser.  We are not addressing
  // if the enum type even has a name.
  // Make this look like a cast (must be output as a cast to be compiled as source code).
     if (il_header.source_language == sl_C)
        {
       // Handling for C just uses the numbr value.
          alternativeName = "(" + StringUtility::numberToString((long long)value) + ")";
        }
       else
        {
       // C++ style cast...(original code)
          alternativeName = enumClassName + "(" + StringUtility::numberToString((long long)value) + ")";
        }

#if 0
     printf ("In get_name_for_enumerator(): Generating an alternative name: alternativeName = %s \n",alternativeName.str());
#endif

     return alternativeName;
   }

SgType* getType(SgDeclarationStatement* declaration)
   {
     SgType* type = NULL;

     if (declaration == NULL)
        {
          printf ("In getType(): Detected declaration == NULL (return type == NULL) \n");
          return type;
        }

     ROSE_ASSERT(declaration != NULL);

     switch(declaration->variantT())
        {
          case V_SgClassDeclaration:
          case V_SgTemplateClassDeclaration:
          case V_SgTemplateInstantiationDecl:
             {
               type = isSgClassDeclaration(declaration)->get_type();
               ROSE_ASSERT(type != NULL);
               break;
             }

          case V_SgFunctionDeclaration:
          case V_SgMemberFunctionDeclaration:
          case V_SgTemplateFunctionDeclaration:
          case V_SgTemplateMemberFunctionDeclaration:
          case V_SgTemplateInstantiationFunctionDecl:
          case V_SgTemplateInstantiationMemberFunctionDecl:
             {
               type = isSgFunctionDeclaration(declaration)->get_type();
               ROSE_ASSERT(type != NULL);
               break;
             }

          case V_SgEnumDeclaration:
             {
               type = isSgEnumDeclaration(declaration)->get_type();
               ROSE_ASSERT(type != NULL);
               break;
             }

       // DQ (5/17/2018): Added case for SgTemplateTypedefDeclaration (see test2018_98.C).
       // Added SgTemplateInstantiationTypedefDeclaration as well.
          case V_SgTemplateTypedefDeclaration:
          case V_SgTemplateInstantiationTypedefDeclaration:

          case V_SgTypedefDeclaration:
             {
               type = isSgTypedefDeclaration(declaration)->get_type();
               ROSE_ASSERT(type != NULL);
               break;
             }

          case V_SgVariableDeclaration:
          case V_SgTemplateVariableDeclaration:
          case V_SgTemplateVariableInstantiation:
             {
               SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(declaration);
               ROSE_ASSERT(variableDeclaration != NULL);
               SgInitializedName* initializedName = getFirstInitializedName(variableDeclaration);
               ROSE_ASSERT(initializedName != NULL);
               type = initializedName->get_type();
               ROSE_ASSERT(type != NULL);
               break;
             }

          case V_SgNamespaceAliasDeclarationStatement:
          case V_SgNamespaceDeclarationStatement:
             {
            // A namespace does not have a type...
               type = NULL;
               break;
             }

          default:
             {
               type = NULL;

               printf ("Error: getType(): default reached: declaration = %p = %s \n",declaration,declaration->class_name().c_str());

               ROSE_ASSERT(type != NULL);
             }
        }

  // ROSE_ASSERT(type != NULL);

     return type;
   }

SgDeclarationStatement* get_declaration_of_unnamed_type(SgType* t)
   {
     SgDeclarationStatement* result = NULL;

#ifdef DEBUG_CONVERSION
  // DQ (10/27/2015): Added debugging output to get_declaration_of_unnamed_type().
     indent_line(); debug_print("-- declaration_of_unnamed_type \n");
#endif

#if 0
     printf ("In get_declaration_of_unnamed_type() t = %p = %s \n",t,t != NULL ? t->class_name().c_str() : "null");
#endif

  // This is a little be longer, but it avoids a switch that uses just two
  // cases of 500+ and permits compilation using the "-Wswitch-enum" option.

     SgNamedType* namedType = isSgNamedType(t);
     if (namedType != NULL)
        {
          SgEnumType*  enumType  = isSgEnumType(t);
          if (enumType != NULL)
             {
               result = enumType->get_declaration();
             }
            else
             {
               SgClassType* classType = isSgClassType(t);
               if (classType != NULL)
                  {
                    result = classType->get_declaration();
                  }
                 else
                  {
                 // DQ (10/21/2012): Added support for SgTypedefType (see test2010_08.c).
                    SgTypedefType* typedefType = isSgTypedefType(t);
                    if (typedefType != NULL)
                       {
                         result = typedefType->get_declaration();
                       }
                      else
                       {
                         SgNonrealType * nrtype = isSgNonrealType(t);
                         if (nrtype != NULL)
                            {
                              result = nrtype->get_declaration();
                            }
                           else
                            {
                              ROSE_ASSERT (!"Bad type in get_declaration_of_unnamed_type");
                              ROSE_ASSERT(false);
                            }
                       }
                  }
             }
        }
       else
        {
       // DQ (10/21/2012): Added support for SgModifierType (see test2012_37.c).
          SgModifierType* modifierType = isSgModifierType(t);
          if (modifierType != NULL)
             {
               SgType* type = modifierType->get_base_type();
               ROSE_ASSERT(type != NULL);
               result = get_declaration_of_unnamed_type(type);
             }
        }

  // DQ (7/9/2012): This function can now return NULL.
     if (result != NULL && result->get_definingDeclaration() != NULL)
        {
          result = result->get_definingDeclaration();

       // DQ (11/12/2021): Can we assert this?
          ROSE_ASSERT(result != NULL);
#if 0
          printf ("In get_declaration_of_unnamed_type(): result != NULL: calling result->get_definingDeclaration(): result = %p = %s \n",result,result->class_name().c_str());
#endif
        }

     return result;
   }

a_type_ptr getBaseTypeOfEdgArrayOrPointerType(a_type_ptr type)
   {
  // DQ (7/16/2012): If this is an aray or pointer, then we have to strip this away to find the base type.

     ROSE_ASSERT(type != NULL);

  // DQ (8/12/2012): Use a return variable to amke the implementation cleaner (don't reset 
  // the values of input parameters (even if it was pass by value and caused no side-effect).
     a_type_ptr return_type = type;

  // DQ (10/21/2012): We need to also look past tk_typeref (see test2012_63.c).
     if (type->kind == tk_array || type->kind == tk_pointer || type->kind == tk_typeref)
        {
#if 0
          printf ("Resolving the base type of the input type = %p \n",type);
#endif
          a_type_ptr base_type = type;
          while (base_type->kind == tk_array || base_type->kind == tk_pointer || base_type->kind == tk_typeref)
             {
#if 0
               printf ("finding the base type base_type = %p \n",base_type);
#endif
               if (base_type->kind == tk_array)
                  {
                    base_type = base_type->variant.array.element_type;
                  }
                 else
                  {
                    if (base_type->kind == tk_pointer)
                       {
                         base_type = base_type->variant.pointer.type;
                       }
                      else
                       {
                         if (base_type->kind == tk_typeref)
                            {
                              base_type = base_type->variant.typeref.type;
                            }
                       }
                  }
             }
          return_type = base_type;
        }
     return return_type;
   }

SgScopeStatement* getAssociatedClassDefinitionForMemberFunction(a_routine_ptr p, bool forceTemplateDeclaration )
   {
  // This function returns the scope for a member function, before the construction of the member 
  // fucntion and so that the member function can be constructed.

  // A SgTemplateClassDeclaration stores a SgTemplateClassDefinition, while a  SgClassDeclaration stores a SgClassDefinition.
  // But a SgTemplateClassDefinition is derived from the SgClassDefinition, so we can use a pointer of type SgClassDefinition
  // as the return type for this function.

     ROSE_ASSERT (p != NULL);

     SgScopeStatement* associatedClassDefinition = NULL;

  // setDefiningAndFirstNondefiningDeclarations(p,buildTemplateDeclaration,buildTemplateInstantiation,forceTemplateDeclaration,first_nondefining_declaration,return_defining_declaration,edg_template);
     SgFunctionDeclaration* first_nondefining_declaration = NULL;
     SgFunctionDeclaration* return_defining_declaration   = NULL;
     a_template_ptr         edg_template                  = NULL;

  // This will set the edg_template, but we don't care about that for this use.
     setDefiningAndFirstNondefiningDeclarations(p,forceTemplateDeclaration,first_nondefining_declaration,return_defining_declaration,edg_template);

     ROSE_ASSERT(p->source_corresp.is_class_member == true);

     if (first_nondefining_declaration != NULL)
        {
          associatedClassDefinition = first_nondefining_declaration->get_scope();
          ROSE_ASSERT(associatedClassDefinition != NULL);
        }
       else
        {
          a_scope_ptr parentScope = p->source_corresp.parent_scope;
          ROSE_ASSERT(parentScope != NULL);

          if (nonreal_scope_cache.find(parentScope) != nonreal_scope_cache.end()) {
            associatedClassDefinition = nonreal_scope_cache[parentScope];
          } else if (scope_cache.find(parentScope) != scope_cache.end()) {
            associatedClassDefinition = scope_cache[parentScope];
          } else {
            associatedClassDefinition = topScopeStack();
          }
          ROSE_ASSERT(associatedClassDefinition != NULL);
        }

     ROSE_ASSERT(associatedClassDefinition != NULL);

     return associatedClassDefinition;
   }

void setTemplateOrTemplateInstantiationClassGeneration(a_type_ptr p, bool forceTemplateDeclaration, bool & buildTemplateDeclaration, bool & buildTemplateInstantiation)
   {
  // See documentation for similar function: setTemplateOrTemplateInstantiationFunctionGeneration().

#define DEBUG_SET_TEMPLATE_OR_INSTANTIATION_CLASS_DECLARATION 0

#if DEBUG_SET_TEMPLATE_OR_INSTANTIATION_CLASS_DECLARATION
     printf ("In setTemplateOrTemplateInstantiationClassGeneration(): Before setting buildTemplateDeclaration: forceTemplateDeclaration = %s \n",forceTemplateDeclaration ? "true" : "false");
#endif

  // DQ (12/18/2011): Allow us to force declaration to be template declarations from the function parameter list).
     buildTemplateDeclaration   = isTemplateDeclaration(p) || forceTemplateDeclaration;

#if DEBUG_SET_TEMPLATE_OR_INSTANTIATION_CLASS_DECLARATION
     printf ("In setTemplateOrTemplateInstantiationClassGeneration(): After setting: buildTemplateDeclaration = %s \n",buildTemplateDeclaration ? "true" : "false");
#endif

  // buildTemplateInstantiation = isTemplateInstantiation(p);
     buildTemplateInstantiation = (buildTemplateDeclaration == false) ? isTemplateInstantiation(p) : false;

#if DEBUG_SET_TEMPLATE_OR_INSTANTIATION_CLASS_DECLARATION
     printf ("In setTemplateOrTemplateInstantiationClassGeneration(): Before handleAsClass: forceTemplateDeclaration   = %s \n",forceTemplateDeclaration   ? "true" : "false");
     printf ("In setTemplateOrTemplateInstantiationClassGeneration(): Before handleAsClass: buildTemplateDeclaration   = %s \n",buildTemplateDeclaration   ? "true" : "false");
     printf ("In setTemplateOrTemplateInstantiationClassGeneration(): Before handleAsClass: buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
#endif

  // This applies to both classes and functions (and also variables, I think!).
     bool handleAsClass = (p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union);
     if (handleAsClass && p->variant.class_struct_union.is_specialized == true)
        {
       // DQ (4/26/2012): If this is a specialization then we want to have it be built as a template instantiation in ROSE.

#if DEBUG_SET_TEMPLATE_OR_INSTANTIATION_CLASS_DECLARATION
// #ifdef DEBUG_CONVERSION
          printf ("Since this is a specialization of a template (even if there are no template arguments for where the specialization is of the member function's class) \n");
          printf ("  --- Note: p->variant.class_struct_union.specialized_with_old_syntax = %s \n",p->variant.class_struct_union.specialized_with_old_syntax ? "true" : "false");
          printf ("  --- Note: p->variant.class_struct_union.is_in_class_specialization  = %s \n",p->variant.class_struct_union.is_in_class_specialization  ? "true" : "false");
#endif
          buildTemplateDeclaration   = false;
          buildTemplateInstantiation = true;
        }

     errorCheckingOfTemplateOrTemplateInstantiationModes(buildTemplateDeclaration, buildTemplateInstantiation);

#if 1
  // DQ (5/18/2014): Added code to reset buildTemplateInstantiation when the previous associated declaration 
  // was declared as a template instantiation.  This happens for Boost examples and is a problem that leads 
  // to a later inconsistancy in ROSE.
     if (buildTemplateDeclaration == false && buildTemplateInstantiation == false)
        {
       // DQ (5/18/2014): It appears that in Boost there are instances where we have a template instantiation matching 
       // the current name but we may have an EDG type that will evaluate to a non-template instantiation.
#if DEBUG_GET_DECLARATION_SCOPE
          printf ("In setTemplateOrTemplateInstantiationClassGeneration(): calling getDeclarationScope() \n");
#endif
          SgScopeStatement* scopeOfDeclaration = getDeclarationScope(p->source_corresp);
          ROSE_ASSERT(scopeOfDeclaration != NULL);

       // DQ (8/12/2014): Added C++11 support for templated typerefs (see Cxx11_tests/test2014_38.C).
       // ROSE_ASSERT(p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union);

       // DQ (8/12/2014): Added C++11 support for templated typerefs (see Cxx11_tests/test2014_38.C).
       // a_template_arg_ptr templateArgs = p->variant.class_struct_union.extra_info->template_arg_list;
          a_template_arg_ptr templateArgs = NULL;
          if (p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union)
             {
               templateArgs = p->variant.class_struct_union.extra_info->template_arg_list;
             }

          SgTemplateArgumentPtrList templateArgumentsList;
          if (templateArgs != NULL)
             {
#if DEBUG_SET_TEMPLATE_OR_INSTANTIATION_CLASS_DECLARATION || 0
               printf ("In setTemplateOrTemplateInstantiationClassGeneration(): Calling parse_template_arguments to support templateArgumentList \n");
#endif
#if 0
               a_class_type_supplement_ptr classTypeSupplement = p->variant.class_struct_union.extra_info;
               ROSE_ASSERT((p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union) && classTypeSupplement != NULL);
               printf ("In setTemplateOrTemplateInstantiationClassGeneration(): classTypeSupplement->min_template_arguments = %d \n",classTypeSupplement->min_template_arguments);
#endif
               templateArgumentsList = parse_template_arguments(templateArgs,scopeOfDeclaration);
#if DEBUG_SET_TEMPLATE_OR_INSTANTIATION_CLASS_DECLARATION
               printf ("In setTemplateOrTemplateInstantiationClassGeneration(): DONE: Calling parse_template_arguments to support templateArgumentList \n");
#endif
             }

          SgName nameWithOutTemplateArguments = generateName(p);
       // SgName nameWithTemplateArguments    = appendTemplateArgumentsToName(nameWithOutTemplateArguments,templateArgumentsList);
          SgName nameWithTemplateArguments    = nameWithOutTemplateArguments;
#if 0
          printf ("In setTemplateOrTemplateInstantiationClassGeneration(): nameWithOutTemplateArguments = %s nameWithTemplateArguments = %s \n",nameWithOutTemplateArguments.str(),nameWithTemplateArguments.str());
#endif
          SgClassSymbol* mysymbol = scopeOfDeclaration->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList);
          if (mysymbol != NULL)
             {
               ROSE_ASSERT(mysymbol->get_declaration() != NULL);
               if (isSgTemplateInstantiationDecl(mysymbol->get_declaration()) != NULL)
                  {
#if 0
                    printf ("Reset buildTemplateInstantiation based on having identified SgTemplateInstantiationDecl in target scope \n");
#endif
                    buildTemplateInstantiation = true;
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
             }
        }
#endif

#if DEBUG_IS_TEMPLATE_DECLARATION || DEBUG_SET_TEMPLATE_OR_INSTANTIATION_CLASS_DECLARATION
     printf ("Leaving setTemplateOrTemplateInstantiationClassGeneration(): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
     printf ("Leaving setTemplateOrTemplateInstantiationClassGeneration(): buildTemplateDeclaration   = %s \n",buildTemplateDeclaration ? "true" : "false");
#endif
   }

void setTemplateOrTemplateInstantiationFunctionGeneration(a_routine_ptr p, bool forceTemplateDeclaration, bool & buildTemplateDeclaration, bool & buildTemplateInstantiation)
   {
  // This function determines if an EDG routine is to be build as a template declaration or a template 
  // instantiation function (member or non-member function). A value of "false" to both variables implies 
  // that it is a normal function or member function.  This function captures all logic and testing for
  // this subject and is called by both the convert_routine() and convert_routine_prototype() functions.
  // Note that all functions are generated by either of these two convert_*() functions.

#if 0
     printf ("In setTemplateOrTemplateInstantiationFunctionGeneration(): (routine) p = %p \n",p);
#endif

#if 0
     printf ("In setTemplateOrTemplateInstantiationFunctionGeneration(): calling isTemplateDeclaration(): (routine) p = %p \n",p);
#endif

  // DQ (12/18/2011): Allow us to force declaration to be template declarations from the function parameter list).
     buildTemplateDeclaration   = isTemplateDeclaration(p) || forceTemplateDeclaration;

#if 0
     printf ("In setTemplateOrTemplateInstantiationFunctionGeneration(): calling isTemplateInstantiation(): (routine) p = %p \n",p);
#endif

     buildTemplateInstantiation = (buildTemplateDeclaration == false) ? isTemplateInstantiation(p) : false;

#if 0
     printf ("In setTemplateOrTemplateInstantiationFunctionGeneration(): (before special case handling): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
     printf ("In setTemplateOrTemplateInstantiationFunctionGeneration(): (before special case handling): buildTemplateDeclaration   = %s \n",buildTemplateDeclaration ? "true" : "false");
#endif

  // DQ (9/6/2014): Note that the same a_routine_ptr can be used to build both a template instantiation and also the 
  // associated template declaration.  When this happends it is forced using forceTemplateDeclaration and we have to 
  // allow this to ignore the p->is_specialized flag in this case.
  // This applies to both classes and functions (and also variables, I think!).
     if (p->is_specialized == true && forceTemplateDeclaration == false)
        {
       // DQ (4/26/2012): If this is a specialization then we want to have it be built as a template instantiation in ROSE.

#ifdef DEBUG_CONVERSION
          printf ("In setTemplateOrTemplateInstantiationFunctionGeneration(): Since this is a specialization of a function template (even if there are no template arguments for where the specialization is of the member function's class) \n");

          printf ("   --- Note: p->specialized_with_old_syntax = %s \n",p->specialized_with_old_syntax ? "true" : "false");
          printf ("   --- Note: p->is_in_class_specialization  = %s \n",p->is_in_class_specialization  ? "true" : "false");
#endif
          buildTemplateDeclaration   = false;
          buildTemplateInstantiation = true;

       // DQ (3/2/2019): Can we assert this?
          ROSE_ASSERT(p->is_template_function == true);
        }

  // DQ (5/15/2012): Added a new test to force generation of template instantiation mode (to support explicit template instantiation (see test2012_84.C).
     if (buildTemplateDeclaration == false && buildTemplateInstantiation == false && p->assoc_template != NULL)
        {
#ifdef DEBUG_CONVERSION
          printf ("In setTemplateOrTemplateInstantiationFunctionGeneration(): Even though we have forceTemplateDeclaration = %s, setting buildTemplateInstantiation = true \n",forceTemplateDeclaration ? "true" : "false");
#endif
          buildTemplateInstantiation = true;

       // DQ (3/2/2019): Can we assert this?
          ROSE_ASSERT(p->is_template_function == true);
        }

#if 0
  // DQ (5/21/2013): Added to test for friend function defined in class, but which is not a template.
     if (buildTemplateDeclaration == false && buildTemplateInstantiation == true && p->assoc_template == NULL && p->defined_in_friend_decl == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("In setTemplateOrTemplateInstantiationFunctionGeneration(): Even though we have forceTemplateDeclaration = %s, setting buildTemplateInstantiation = true (where p->defined_in_friend_decl == true) \n",
               forceTemplateDeclaration ? "true" : "false");
#endif
          buildTemplateInstantiation = false;
        }
#endif

  // DQ (3/2/2019): Can we assert this?
     if (buildTemplateInstantiation == true)
       {
         if (p->is_template_function == false)
            {
#if 0
              printf ("We got this function wrong: it is not a template instantiation, it is just a regular function \n");
#endif
           // Only change the status of buildTemplateInstantiation if we already decided that buildTemplateDeclaration == false.
              if (buildTemplateDeclaration == false)
                 {
                   buildTemplateInstantiation = false;
                 }
            }
      // ROSE_ASSERT(p->is_template_function == false);
       }
      else
       {
         if (buildTemplateDeclaration == false)
            {
              ROSE_ASSERT(p->is_template_function == false);
            }
       }

  // Error checking (for consistancy).
     errorCheckingOfTemplateOrTemplateInstantiationModes(buildTemplateDeclaration, buildTemplateInstantiation);

// #if 0
#if DEBUG_IS_TEMPLATE_DECLARATION || 0
     printf ("Leaving setTemplateOrTemplateInstantiationFunctionGeneration(): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
     printf ("Leaving setTemplateOrTemplateInstantiationFunctionGeneration(): buildTemplateDeclaration   = %s \n",buildTemplateDeclaration ? "true" : "false");
#endif
   }

SgDeclarationStatement* getTemplateDeclaration(a_type_ptr p)
   {
  // DQ (4/21/2012): Added test.
     ROSE_ASSERT(p != NULL);

     ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);

     a_template_ptr associatedTemplateDeclaration = p->variant.class_struct_union.extra_info->assoc_template;
     ROSE_ASSERT(associatedTemplateDeclaration != NULL);

     a_type_ptr instantiation_type = associatedTemplateDeclaration->prototype_instantiation.type;
     if (instantiation_type != NULL && instantiation_type != p) {
       p = instantiation_type;
     }

     SgDeclarationStatement * decl_stmt = getFirstNondefDeclForTemplate(associatedTemplateDeclaration);

#if 0
     printf ("$$$$$ Looking for the associated templateDeclaration = %p (EDG associatedTemplateDeclaration = %p) \n",templateDeclaration,associatedTemplateDeclaration);
#endif

     if (decl_stmt == NULL) {

       ROSE_ASSERT(p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union);
       ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template != NULL);

       decl_stmt = convert_template_prototype(p->variant.class_struct_union.extra_info->assoc_template);
       ROSE_ASSERT(decl_stmt != NULL);

       SgTemplateClassDeclaration * templateDeclaration = isSgTemplateClassDeclaration(decl_stmt);
       if (templateDeclaration != NULL) {
         SgTemplateClassDeclaration* firstDefining_templateClassDeclaration = isSgTemplateClassDeclaration(templateDeclaration->get_firstNondefiningDeclaration());
         ROSE_ASSERT(firstDefining_templateClassDeclaration != NULL);

         setFirstNondefDeclForTemplate(associatedTemplateDeclaration, firstDefining_templateClassDeclaration);

      // DQ (9/9/2012): Verify these are for the name named templates (though template arguments could still make them different).
         ROSE_ASSERT(firstDefining_templateClassDeclaration->get_name() == templateDeclaration->get_name());

         SgTemplateClassDeclaration * new_defining_templateClassDeclaration = convert_template_struct_primary(p, SgClassDeclaration::e_class, firstDefining_templateClassDeclaration, associatedTemplateDeclaration);
         ROSE_ASSERT(new_defining_templateClassDeclaration != NULL);

         ROSE_ASSERT(hasDefiningDeclForTemplate(associatedTemplateDeclaration));
       }
     }
     ROSE_ASSERT(decl_stmt != NULL);

     return decl_stmt;
   }

void set_access_modifier(SgDeclarationStatement* declarationStatement, unsigned int access_modifiers)
   {
  // DQ (2/16/2013): Adding support for access modifiers for declarations in classes.

#if 0
     printf ("In set_access_modifier(): declarationStatement = %p = %s name = %s access_modifiers = %u \n",
          declarationStatement,declarationStatement->class_name().c_str(),SageInterface::get_name(declarationStatement).c_str(),access_modifiers);
#endif

// DQ (2/17/2013): This is a mechanism to disable yesterday's fix for class member access marking.
// With this in place we fail the outlining and OMP lowering tests from Liao's work. Since those are
// the only failing tests and I would like to get this work checked in so that Liao can fix the 
// outlined functions to be friend functions of the classes whose non-public data members are used
// this turns off this fix and yet makes it easy to reactivate as a testing/development 2nd step.
#define USE_CLASS_MEMBER_ACCESS_FIX 1

#if USE_CLASS_MEMBER_ACCESS_FIX


#if 0
     printf ("In set_access_modifier(): access_modifiers = %d \n",access_modifiers);
     if (declarationStatement == declarationStatement->get_definingDeclaration())
        {
          printf ("declarationStatement = %p is the DEFINING declaration \n",declarationStatement);
        }
       else
        {
          printf ("declarationStatement = %p is the NON-defining declaration \n",declarationStatement);
        }
     declarationStatement->get_declarationModifier().get_accessModifier().display("In set_access_modifier()");
#endif

  // DQ (8/11/2020): debugging Cxx_tests/test2020_28.C (incores marking of access for extern "C" friend function.
     ROSE_ASSERT(declarationStatement->get_declarationModifier().get_accessModifier().isDefault() == false || declarationStatement->get_declarationModifier().get_accessModifier().isPublic() == false);

#if 0
     if (access_modifiers != as_public)
        {
          SgDeclarationModifier & declarationModifier = declarationStatement->get_declarationModifier();
          declarationModifier.unsetDefault();

       // DQ (8/11/2020): debugging Cxx_tests/test2020_28.C (incores marking of access for extern "C" friend function.
          ROSE_ASSERT(declarationStatement->get_declarationModifier().get_accessModifier().isDefault() == false || declarationStatement->get_declarationModifier().get_accessModifier().isPublic() == false);

#error "DEAD CODE!"

#if 0
          printf ("In set_access_modifier(): access_modifiers != as_public \n");
#endif
       // set access modifier bits
          switch (access_modifiers)
             {
               case as_public:
#if 0
                    printf ("In set_access_modifier(): declarationStatement = %p = %s: Mark as public \n",declarationStatement,declarationStatement->class_name().c_str());
#endif
                    declarationModifier.get_accessModifier().setPublic();
                    break;

#error "DEAD CODE!"

               case as_protected:
#if 0
                    printf ("In set_access_modifier(): declarationStatement = %p = %s: Mark as protected \n",declarationStatement,declarationStatement->class_name().c_str());
#endif
                    declarationModifier.get_accessModifier().setProtected();
                    break;

#error "DEAD CODE!"

               case as_private:
#if 0
                    printf ("In set_access_modifier(): declarationStatement = %p = %s: Mark as private \n",declarationStatement,declarationStatement->class_name().c_str());
#endif
                    declarationModifier.get_accessModifier().setPrivate();
                    break;

#error "DEAD CODE!"

               default:
                    printf ("Error in set_access_modifier(): default reached in switch() \n");
                    ROSE_ASSERT (false);
             }
        }
       else
        {
#error "DEAD CODE!"

       // default is public access
          declarationStatement->get_declarationModifier().unsetDefault();
          declarationStatement->get_declarationModifier().get_accessModifier().setPublic();
        }
#else

  // DQ (8/12/2020): New version of code to support e_default != e_public in access_modifier_enum.

     SgDeclarationModifier & declarationModifier = declarationStatement->get_declarationModifier();
     declarationModifier.unsetDefault();

  // DQ (8/11/2020): debugging Cxx_tests/test2020_28.C (incores marking of access for extern "C" friend function.
     ROSE_ASSERT(declarationStatement->get_declarationModifier().get_accessModifier().isDefault() == false || declarationStatement->get_declarationModifier().get_accessModifier().isPublic() == false);

#if 0
     printf ("In set_access_modifier(): access_modifiers != as_public \n");
#endif
  // set access modifier bits
     switch (access_modifiers)
        {
       // as_public is the value 0, which means that there is no setting in EDG for when the access modifier should not be used.
          case as_public:
#if 0
               printf ("In set_access_modifier(): declarationStatement = %p = %s: Mark as public \n",declarationStatement,declarationStatement->class_name().c_str());
#endif
               declarationModifier.get_accessModifier().setPublic();
               break;

          case as_protected:
#if 0
               printf ("In set_access_modifier(): declarationStatement = %p = %s: Mark as protected \n",declarationStatement,declarationStatement->class_name().c_str());
#endif
               declarationModifier.get_accessModifier().setProtected();
               break;

          case as_private:
#if 0
               printf ("In set_access_modifier(): declarationStatement = %p = %s: Mark as private \n",declarationStatement,declarationStatement->class_name().c_str());
#endif
               declarationModifier.get_accessModifier().setPrivate();
               break;

          default:
               printf ("Error in set_access_modifier(): default reached in switch() \n");
               ROSE_ASSERT (false);
        }
#endif


#if 0
     printf ("Leaving set_access_modifier(): access_modifiers = %d \n",access_modifiers);
     declarationStatement->get_declarationModifier().get_accessModifier().display("Leaving set_access_modifier()");
#endif

  // DQ (8/11/2020): debugging Cxx_tests/test2020_28.C (incores marking of access for extern "C" friend function.
     ROSE_ASSERT(declarationStatement->get_declarationModifier().get_accessModifier().isDefault() == false || declarationStatement->get_declarationModifier().get_accessModifier().isPublic() == false);

#endif
   }

void setTemplateFunctionDeclaration (a_routine_ptr p, SgFunctionDeclaration* functionDeclaration )
   {
  // DQ (1/2/2012): This sets the template function inside of the template instantiation function (for both functions and member functions).

     SgTemplateInstantiationMemberFunctionDecl* templateInstantiationMemberFunction = isSgTemplateInstantiationMemberFunctionDecl(functionDeclaration);
     SgTemplateInstantiationFunctionDecl*       templateInstantiationFunction       = isSgTemplateInstantiationFunctionDecl(functionDeclaration);

#if 0
     printf ("In setTemplateFunctionDeclaration(a_routine_ptr p = %p functionDeclaration = %p = %s \n",p,functionDeclaration,functionDeclaration->class_name().c_str());
     functionDeclaration->get_file_info()->display("In setTemplateFunctionDeclaration(): debug");
#endif

#if 0
     printf ("In setTemplateFunctionDeclaration(): calling summarize_rountine(): p = %p \n",p);
     summarize_routine(p);
#endif

     ROSE_ASSERT(functionDeclaration->get_scope() != NULL);

#if 0
     printf ("In setTemplateFunctionDeclaration(): functionDeclaration = %p = %s = %s \n",functionDeclaration,functionDeclaration->class_name().c_str(),functionDeclaration->get_name().str());
     printf ("   --- functionDeclaration->get_scope()               = %p = %s \n",functionDeclaration->get_scope(),functionDeclaration->get_scope()->class_name().c_str());
     if (templateInstantiationFunction != NULL)
          printf ("   --- functionDeclaration->get_templateDeclaration() = %p \n",templateInstantiationFunction->get_templateDeclaration());
#endif

  // Check that ONLY ONE of these is a non-NULL pointer.
     if (templateInstantiationFunction == NULL && templateInstantiationMemberFunction == NULL)
        {
          ROSE_ASSERT(functionDeclaration != NULL);
          printf ("Error: what is the input functionDeclaration = %p = %s \n",functionDeclaration,functionDeclaration->class_name().c_str());
        }
     ROSE_ASSERT(templateInstantiationFunction != NULL || templateInstantiationMemberFunction != NULL);
     ROSE_ASSERT(templateInstantiationFunction == NULL || templateInstantiationMemberFunction == NULL);

     ROSE_ASSERT(p != NULL);

#if 0
     printf ("In setTemplateFunctionDeclaration(): p->assoc_template = %p \n",p->assoc_template);
#endif

  // Get the associated template declaration from the template declaration cache.
  // DQ (4/30/2012): In test2012_58.C - handle case where p->assoc_template == NULL for friend function in template class.
  // ROSE_ASSERT(p->assoc_template != NULL);
     if (p->assoc_template != NULL)
        {
       // DQ (11/3/2012): I just noticed this potential bug...check it out later...
#if 0
          printf ("In setTemplateFunctionDeclaration(): Should we be looking up the function template in first_nondefining_declarations_for_template_functions instead of first_nondefining_declarations_for_templates??? \n");
#endif

       // DQ (3/13/2012): This assertion fails for test2004_85.C (Note that 735 other tests pass fine, so I 
       // can't tell if this is a real problem or not (fails in basic_string.h)). Make it a warning for now.
       // I think that where this is a nested templated function of a non-template class in a templated class 
       // then we might not have seen the original template previously (it is at least a rather complex case).
       // DQ (12/29/2011): Modified to avoid insertion of NULL pointer into cache.
          if (!hasFirstNondefDeclForTemplate(p->assoc_template))
             {
            // DQ (4/6/2014): This happens rarely, but on larger scale use of more complex templates. 
            // So it is something we have to accept for now.
#ifdef DEBUG_CONVERSION
               printf ("WARNING: Failing test for p->assoc_template = %p in first_nondefining_declarations_for_templates list \n",p->assoc_template);
#endif
             }
          SgDeclarationStatement* templateDeclaration = NULL;
          if (hasFirstNondefDeclForTemplate(p->assoc_template))
             {
               templateDeclaration = getFirstNondefDeclForTemplate(p->assoc_template);
             }

          if (templateDeclaration == NULL)
             {
            // Can we generate it here?  We at least have a valid p->assoc_template.
#if 0
               printf ("In setTemplateFunctionDeclaration(): Since we have not seen the template declaration yet, and we appear to need it, we will try to generate it directly using p->assoc_template = %p \n",p->assoc_template);
#endif
               SgDeclarationStatement* missingTemplateDeclaration = convert_template_prototype(p->assoc_template);
#if 0
               printf ("DONE: In setTemplateFunctionDeclaration(): Since we have not seen the template declaration yet, and we appear to need it, we will try to generate it directly using p->assoc_template = %p \n",p->assoc_template);
#endif
               ROSE_ASSERT(missingTemplateDeclaration != NULL);

               templateDeclaration = missingTemplateDeclaration;
             }

       // DQ (3/13/2012): Commented out the test since it appears that in some cases the template will not have been processed yet.
          ROSE_ASSERT(templateDeclaration != NULL);

          if (templateDeclaration != NULL)
             {
            // If we have a valid templateDeclaration then we can use it to set the reference to it in the template instantiation.
               if (templateInstantiationFunction != NULL)
                  {
                    SgTemplateFunctionDeclaration* templateFunctionDeclaration = isSgTemplateFunctionDeclaration(templateDeclaration);
                    ROSE_ASSERT(templateFunctionDeclaration != NULL);
                    templateInstantiationFunction->set_templateDeclaration(templateFunctionDeclaration);
                  }
                 else
                  {
                    SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(templateDeclaration);
                    ROSE_ASSERT(templateMemberFunctionDeclaration != NULL);

                    if (templateInstantiationMemberFunction == NULL)
                       {
                         ROSE_ASSERT(functionDeclaration != NULL);
                         printf ("Error: what is the input functionDeclaration = %p = %s \n",functionDeclaration,functionDeclaration->class_name().c_str());
                       }
                    ROSE_ASSERT(templateInstantiationMemberFunction != NULL);
                    templateInstantiationMemberFunction->set_templateDeclaration(templateMemberFunctionDeclaration);
                  }
             }
            else
             {
            // DQ (3/13/2012): But we don't always have it, or maybe we should process it here???  See test2004_85.C.
               printf ("WARNING: could not identify a previously generated template for function name = %s to initialize the reference to templateDeclaration in the template instantiation \n",functionDeclaration->get_name().str());
             }
        }
       else
        {
#ifdef DEBUG_CONVERSION
          printf ("WARNING: In setTemplateFunctionDeclaration(a_routine_ptr p = %p): p->assoc_template == NULL \n",p);
#endif

       // DQ (11/3/2012): I want to check this out later...but back to source position details for now...
#if 0
          printf ("I think we should be able to find the function template using p = %p as a key instead of p->assoc_template (which is NULL) \n",p);
#endif
          SgScopeStatement* scope = functionDeclaration->get_scope();
          ROSE_ASSERT(scope != NULL);
#if 0
          scope->get_symbol_table()->print("In setTemplateFunctionDeclaration()");
#endif
#if 0
          printf ("In setTemplateFunctionDeclaration(): lookup symbol in associated scope: functionDeclaration->get_name() = %s \n",functionDeclaration->get_name().str());
          printf ("In setTemplateFunctionDeclaration(): lookup symbol in associated scope: functionDeclaration->get_type() = %p = %s \n",functionDeclaration->get_type(),functionDeclaration->get_type()->class_name().c_str());          
#endif
       // ROSE_ASSERT(scope->lookup_template_function_symbol(functionDeclaration->get_name(),functionDeclaration->get_type()) != NULL);
          ROSE_ASSERT(scope->symbol_exists(functionDeclaration->get_name()) == true);

       // DQ (8/22/2013): Changed this to use eather: lookup_template_function_symbol() or lookup_template_member_function_symbol().
       // DQ (8/16/2013): Modified to reflect new API.
       // SgSymbol* symbol = scope->lookup_symbol(functionDeclaration->get_name());
       // SgSymbol* symbol = scope->lookup_symbol(functionDeclaration->get_name(),NULL,NULL);
       // SgSymbol* symbol = (templateInstantiationFunction != NULL) ? scope->lookup_template_function_symbol(functionDeclaration->get_name(),NULL,NULL) : scope->lookup_template_member_function_symbol(functionDeclaration->get_name(),NULL,NULL);
          SgSymbol* symbol = NULL;
          SgType* functionType = NULL;
          if (templateInstantiationFunction != NULL)
             {
            // SgType* functionType = templateInstantiationFunction->get_type();
            // ROSE_ASSERT(functionType != NULL);
#if 0
            // DQ (8/28/2014): OLD VERSION OF CODE (lookup_template_function_symbol needs the name without template arguments, and without the type (which is not known at this point)).
               symbol = scope->lookup_template_function_symbol(functionDeclaration->get_name(),functionType,NULL);
               ROSE_ASSERT(symbol != NULL);
#else
            // DQ (8/28/2014): New version of code.
#if 0
               printf ("In setTemplateFunctionDeclaration(): lookup symbol in associated scope: functionDeclaration->get_name() = %s \n",functionDeclaration->get_name().str());
#endif
            // This is the name with template arguments.
            // SgName name = templateInstantiationFunction->get_name();
            // printf ("   --- name = %s \n",name.str());

            // This is the name without template arguments.
               SgName nameWithOutTemplateArguments = templateInstantiationFunction->get_templateName();
#if 0
               printf ("   --- nameWithOutTemplateArguments = %s \n",nameWithOutTemplateArguments.str());
#endif
            // This is the function type of the template instantiation (and not useful in fidning the associated template).
            // SgFunctionType* functionType = templateInstantiationFunction->get_type();
            // printf ("   --- functionType = %p = %s \n",functionType,functionType->class_name().c_str());

            // DQ (12/29/2014): Get the associated template arguments so that we can lookup the correct SgTemplateFunctionSymbol
            // (and avoid finding a non-template function symbol by mistake).  See test2014_242.C.
            // SgTemplateArgumentPtrList & templateArgumentList = templateInstantiationFunction->get_templateArguments();
            // ROSE_ASSERT(templateArgumentList);

            // SgTemplateClassSymbol* lookupTemplateClassSymbolInParentScopes (const SgName &  name, SgTemplateParameterPtrList* templateParameterList, SgTemplateArgumentPtrList* templateArgumentList, SgScopeStatement *cscope = NULL);
            // SgTemplateSymbol* templateSymbol = associated_scope->lookup_template_function_symbol(name,functionType);
               SgFunctionSymbol* functionSymbol = scope->lookup_function_symbol(nameWithOutTemplateArguments);
               ROSE_ASSERT(functionSymbol != NULL);
               SgTemplateFunctionSymbol* templateSymbol = isSgTemplateFunctionSymbol(functionSymbol);

            // DQ (8/29/2014): This fails for ROSE compiling the ROSE header files (testRoseHeaders_01.C).
            // ROSE_ASSERT(templateSymbol != NULL);
               if (templateSymbol == NULL)
                  {
#if 0
                    printf ("In setTemplateFunctionDeclaration(): templateSymbol == NULL: calling summarize_rountine(): p = %p \n",p);
                    summarize_routine(p);
#endif
#if 0
                    mprintf ("WARNING: In setTemplateFunctionDeclaration(): Found SgFunctionSymbol instead of SgTemplateFunctionSymbol: functionSymbol = %p = %s \n",functionSymbol,functionSymbol->class_name().c_str());
#endif
                    SgType* templateInstantiationFunctionType = templateInstantiationFunction->get_type();
                    ROSE_ASSERT(templateInstantiationFunctionType != NULL);

                 // DQ (12/29/2014): This does not work when p->assoc_template == NULL.
                 // SgTemplateParameterPtrList templateParameterList = parse_template_parameters(p,scope,NULL);
                 // printf ("In setTemplateFunctionDeclaration(): templateParameterList.size() = %zu \n",templateParameterList.size());
#if 0
                    mprintf ("   --- nameWithOutTemplateArguments                                     = %s \n",nameWithOutTemplateArguments.str());
                    mprintf ("   --- templateInstantiationFunction                                    = %p \n",templateInstantiationFunction);
                    mprintf ("   --- templateInstantiationFunction->get_firstNondefiningDeclaration() = %p \n",templateInstantiationFunction->get_firstNondefiningDeclaration());
                    mprintf ("   --- templateInstantiationFunction->get_definingDeclaration()         = %p \n",templateInstantiationFunction->get_definingDeclaration());
                    mprintf ("   --- templateInstantiationFunction->get_scope()                       = %p = %s \n",templateInstantiationFunction->get_scope(),templateInstantiationFunction->get_scope()->class_name().c_str());
                    mprintf ("   --- scope                                                            = %p = %s \n",scope,scope->class_name().c_str());
                    mprintf ("   --- templateInstantiationFunction->get_parent()                      = %p = %s \n",templateInstantiationFunction->get_parent(),templateInstantiationFunction->get_parent()->class_name().c_str());
#endif
                    a_scope_ptr edg_parent_scope = p->source_corresp.parent_scope;
                    ROSE_ASSERT(edg_parent_scope != NULL);
#if 0
                    mprintf ("   --- edg_parent_scope = %p \n",edg_parent_scope);
#endif
#if 0
                    SgScopeStatement* rose_parent_scope = NULL;
                    if (scope_cache.find(edg_parent_scope) != scope_cache.end())
                       {
                         rose_parent_scope = scope_cache[edg_parent_scope];
                         mprintf ("   --- rose_parent_scope = %p = %s \n",rose_parent_scope,rose_parent_scope->class_name().c_str());
                       }
#endif

#if 0
                 // SgScopeStatement* top_of_template_declaration_scack = NULL;
                    if (templateDeclarationStack.empty() == false)
                       {
                         a_template_ptr edg_template = templateDeclarationStack.back();
                         mprintf ("   --- edg_template = %p \n",edg_template);
                       }
#endif

#if 0
                    if (first_nondefining_declarations_for_template_functions.find(p) != first_nondefining_declarations_for_template_functions.end())
                       {
                      // SgTemplateFunctionDeclaration* templateFunctionDeclaration = first_nondefining_declarations_for_template_functions[p];
                         SgFunctionDeclaration* templateFunctionDeclaration = first_nondefining_declarations_for_template_functions[p];
                         printf ("Found template declaration in first_nondefining_declarations_for_template_functions map! templateFunctionDeclaration = %p \n",templateFunctionDeclaration);                         
                       }
                      else
                       {
                         mprintf ("Not found in first_nondefining_declarations_for_template_functions map \n");
                       }
#endif

#if 0
                    if (defining_declarations_for_template_functions.find(p) != defining_declarations_for_template_functions.end())
                       {
                      // SgTemplateFunctionDeclaration* templateFunctionDeclaration = defining_declarations_for_template_functions[p];
                         SgFunctionDeclaration* templateFunctionDeclaration = defining_declarations_for_template_functions[p];
                         printf ("Found template declaration in defining_declarations_for_template_functions map! templateFunctionDeclaration = %p \n",templateFunctionDeclaration);                         
                       }
                      else
                       {
                         mprintf ("Not found in defining_declarations_for_template_functions map \n");
                       }
#endif

#if 0
                    if (templateInstantiationFunction != NULL)
                       {
                         SgTemplateInstantiationFunctionDecl* local_templateInstantiationFunction = isSgTemplateInstantiationFunctionDecl(templateInstantiationFunction->get_firstNondefiningDeclaration());
                         ROSE_ASSERT(local_templateInstantiationFunction != NULL);
                         mprintf ("local_templateInstantiationFunction->get_templateDeclaration() = %p \n",local_templateInstantiationFunction->get_templateDeclaration());
                         SgTemplateFunctionDeclaration* templateFunctionDeclaration = isSgTemplateFunctionDeclaration(local_templateInstantiationFunction->get_templateDeclaration());
                      // templateInstantiationFunction->set_templateDeclaration(templateFunctionDeclaration);
                         if (templateFunctionDeclaration == NULL)
                            {
                              mprintf ("\n\n########## SgTemplateFunctionDeclaration not found for SgTemplateInstantiationFunctionDecl \n\n\n");
                            }
                     // ROSE_ASSERT(templateFunctionDeclaration != NULL);
                       }
                      else
                       {
                         SgTemplateInstantiationMemberFunctionDecl* local_templateInstantiationMemberFunction = 
                              isSgTemplateInstantiationMemberFunctionDecl(templateInstantiationMemberFunction->get_firstNondefiningDeclaration());

                         SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(local_templateInstantiationMemberFunction->get_templateDeclaration());
                         if (templateMemberFunctionDeclaration == NULL)
                            {
                              printf ("\n\n########## SgTemplateMemberFunctionDeclaration not found for SgTemplateInstantiationMemberFunctionDecl \n\n\n");
                            }
                      // ROSE_ASSERT(templateMemberFunctionDeclaration != NULL);
                       }
#endif


                 // SgTemplateFunctionSymbol* functionTemplateSymbol = NULL;
                 // functionTemplateSymbol = scope->lookup_template_function_symbol(nameWithOutTemplateArguments,&templateArgumentList);
                 // SgTemplateFunctionSymbol* functionTemplateSymbol = scope->lookup_template_function_symbol(nameWithOutTemplateArguments,templateInstantiationFunctionType,&templateParameterList);

                 // printf ("In setTemplateFunctionDeclaration(): Reset the templateSymbol: functionTemplateSymbol = %p \n",functionTemplateSymbol);

                 // templateSymbol = functionTemplateSymbol;
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
#if 0
               ROSE_ASSERT(templateSymbol->get_declaration() != NULL);
               SgTemplateFunctionDeclaration* templateFunctionDeclaration = isSgTemplateFunctionDeclaration(templateSymbol->get_declaration());
               ROSE_ASSERT(templateFunctionDeclaration != NULL);
#endif
            // setTemplateFunctionDeclaration (p,templateFunctionInstantiationDeclaration);
            // ROSE_ASSERT(templateFunctionInstantiationDeclaration->get_template() == NULL);
            // templateFunctionInstantiationDeclaration->set_template(templateFunctionDeclaration);
            // ROSE_ASSERT(templateFunctionInstantiationDeclaration->get_template() != NULL);

            // symbol = templateSymbol;
               symbol = functionSymbol;
#endif
             }
            else
             {
            // SgType* functionType = templateInstantiationMemberFunction->get_type();
            // ROSE_ASSERT(functionType != NULL);

            // DQ (8/22/2013): In cases where the member function is an added part of a class specialization then
            // there is no template member function to find and we will return a NULL pointer (see test2004_69.C).
            // This is a new development because we have not previously had the new level of precission in symbol 
            // lookup that we have now, and so previously we had found the symbol for the template member function 
            // instantiation when we were looking for the symbol from the template member function declaration.
               symbol = scope->lookup_template_member_function_symbol(functionDeclaration->get_name(),functionType,NULL);
            // ROSE_ASSERT(symbol != NULL);
             }
       // ROSE_ASSERT(symbol != NULL);

          SgFunctionSymbol* functionSymbol = isSgFunctionSymbol(symbol);
          if (symbol != NULL && functionSymbol == NULL)
             {
               printf ("ERROR: In setTemplateFunctionDeclaration(): lookup symbol in associated scope: symbol = %p = %s \n",symbol,symbol->class_name().c_str());
               printf ("ERROR: In setTemplateFunctionDeclaration(): lookup symbol in associated scope: functionDeclaration->get_name() = %s \n",functionDeclaration->get_name().str());
               printf ("ERROR: In setTemplateFunctionDeclaration(): lookup symbol in associated scope: scope = %p = %s \n",scope,scope->class_name().c_str());          
               printf ("ERROR: In setTemplateFunctionDeclaration(): lookup symbol in associated scope: functionDeclaration->get_type() = %p = %s \n",functionDeclaration->get_type(),functionDeclaration->get_type()->class_name().c_str());          
             }
       // ROSE_ASSERT(functionSymbol != NULL);

#ifdef DEBUG_CONVERSION
          printf ("WARNING: In setTemplateFunctionDeclaration(): We might not select the correct template declaration where more than one might exist with the same name. \n");
#endif
#if 0
          printf ("In setTemplateFunctionDeclaration(): templateInstantiationFunction = %p symbol = %p = %s \n",templateInstantiationFunction,symbol,symbol->class_name().c_str());
#endif
          if (templateInstantiationFunction != NULL)
             {
               SgTemplateFunctionDeclaration* templateFunctionDeclaration = isSgTemplateFunctionDeclaration(functionSymbol->get_declaration());
#if 0
               printf ("In setTemplateFunctionDeclaration(): functionSymbol->get_declaration() = %p = %s \n",functionSymbol->get_declaration(),functionSymbol->get_declaration()->class_name().c_str());
               printf ("In setTemplateFunctionDeclaration(): templateFunctionDeclaration = %p \n",templateFunctionDeclaration);
#endif
            // DQ (1/22/2013): In ROSE compiling ROSE there are templates (e.g. in src/roseSupport/rangemap.h:384) 
            // that will sometimes not have an associated template declaration.  We have to allow this, I think.
            // ROSE_ASSERT(templateFunctionDeclaration != NULL);
               if (templateFunctionDeclaration != NULL)
                  {
                    templateInstantiationFunction->set_templateDeclaration(templateFunctionDeclaration);
                    ROSE_ASSERT(templateInstantiationFunction->get_templateDeclaration() != NULL);
                  }
                 else
                  {
// #ifdef DEBUG_CONVERSION
#if 0
                    mprintf ("In setTemplateFunctionDeclaration(): templateInstantiationFunction != NULL: could not find an associated template declaration for this function \n");
#endif
                  }
             }
            else
             {
               if (templateInstantiationMemberFunction != NULL)
                  {
                 // SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(functionSymbol->get_declaration());
                    SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = (functionSymbol != NULL) ? isSgTemplateMemberFunctionDeclaration(functionSymbol->get_declaration()) : NULL;

                 // ROSE_ASSERT(templateMemberFunctionDeclaration != NULL);
                    if (templateMemberFunctionDeclaration != NULL)
                       {
                         templateInstantiationMemberFunction->set_templateDeclaration(templateMemberFunctionDeclaration);
                       }
                      else
                       {
                      // DQ (8/22/2013): This case is more likely and is demonstrated by test2004_69.C.
// #ifdef DEBUG_CONVERSION
#if 0
                         printf ("In setTemplateFunctionDeclaration(): templateInstantiationMemberFunction != NULL: could not find an associated template declaration for this function \n");
#endif
                       }
                  }
                 else
                  {
                 // Note that one of these had to be true.
                    ROSE_ASSERT(templateInstantiationFunction != NULL || templateInstantiationMemberFunction != NULL);

                    printf ("Error: In setTemplateFunctionDeclaration(): not sure what do do for this case! \n");
                    ROSE_ASSERT(false);
                  }

            // ROSE_ASSERT(templateInstantiationMemberFunction->get_templateDeclaration() != NULL);
               if (templateInstantiationMemberFunction->get_templateDeclaration() == NULL)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("WARNING: In setTemplateFunctionDeclaration(): templateInstantiationMemberFunction->get_templateDeclaration() == NULL: templateInstantiationMemberFunction = %p = %s \n",
                         templateInstantiationMemberFunction,templateInstantiationMemberFunction->get_name().str());
                    printf ("--- definingDeclaration()         = %p \n",templateInstantiationMemberFunction->get_definingDeclaration());
                    printf ("--- firstNondefiningDeclaration() = %p \n",templateInstantiationMemberFunction->get_firstNondefiningDeclaration());
#endif
#ifdef DEBUG_CONVERSION
                    templateInstantiationMemberFunction->get_file_info()->display("templateInstantiationMemberFunction: debug");
#endif
                  }
            // ROSE_ASSERT(templateInstantiationMemberFunction->get_templateDeclaration() != NULL);
             }

#if 0
          printf ("Exiting as a test in setTemplateFunctionDeclaration() \n");
          ROSE_ASSERT(false);
#endif
#if 0
       // DQ (12/28/2012): Activate this code to test test2012_277.C.
       // DQ (11/3/2012): I think it is likely that "p->assoc_template == NULL" because "p" is the associated template and so all
       // we need to do to get the associated SgTemplateFunctionDeclaration for this template instantiation is to look it up in
       // the cache using "p" as the key.
       // ROSE_ASSERT(first_nondefining_declarations_for_template_functions.find(p) != first_nondefining_declarations_for_template_functions.end());
       // and maybe...
          ROSE_ASSERT(defining_declarations_for_template_functions.find(p) != defining_declarations_for_template_functions.end());

          SgDeclarationStatement* templateDeclaration = NULL;

          templateDeclaration = first_nondefining_declarations_for_template_functions[p];
          ROSE_ASSERT(templateDeclaration != NULL);
#endif
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
     
#if 0
     printf ("Leaving setTemplateFunctionDeclaration(a_routine_ptr p = %p functionDeclaration = %p = %s \n",p,functionDeclaration,functionDeclaration->class_name().c_str());
  // functionDeclaration->get_file_info()->display("In setTemplateFunctionDeclaration(): debug");
#endif
   }

void setDefiningAndFirstNondefiningDeclarations (
   a_routine_ptr p, 
// bool buildTemplateDeclaration, 
// bool buildTemplateInstantiation, 
   bool forceTemplateDeclaration,
   SgFunctionDeclaration* & first_nondefining_declaration, 
   SgFunctionDeclaration* & return_defining_declaration, 
   a_template_ptr & edg_template )
   {
  // Call the standard function for settin up the values of buildTemplateDeclaration and buildTemplateInstantiation.
     bool buildTemplateDeclaration   = false;
     bool buildTemplateInstantiation = false;
  // bool forceTemplateDeclaration   = false;

     ROSE_ASSERT(p != NULL);

  // DQ (1/27/2013): I think we can assert this here.
     ROSE_ASSERT(first_nondefining_declaration == NULL);
     ROSE_ASSERT(return_defining_declaration == NULL);

#if 0
     printf ("In setDefiningAndFirstNondefiningDeclarations(): calling setTemplateOrTemplateInstantiationFunctionGeneration(): p = %p \n",p);
#endif

     setTemplateOrTemplateInstantiationFunctionGeneration(p,forceTemplateDeclaration,buildTemplateDeclaration,buildTemplateInstantiation);

#if 0
     printf ("In setDefiningAndFirstNondefiningDeclarations(): p = %p forceTemplateDeclaration = %s \n",p,forceTemplateDeclaration ? "true" : "false");
#endif

     if (buildTemplateDeclaration == true)
        {
          if (forceTemplateDeclaration == true)
             {
            // This this case we should have passed the template into this function (we have to modify the API).
               if (edg_template == NULL)
                  {
#if 0
                    printf ("In setDefiningAndFirstNondefiningDeclarations(): edg_template not set (case forceTemplateDeclaration == true), but required, using value in p->assoc_template = %p \n",p->assoc_template);
#endif
#if 1
                    edg_template = p->assoc_template;

                    if (edg_template == NULL)
                       {
                      // See test2012_56.C for an example.
#ifdef DEBUG_CONVERSION
                         printf ("WARNING: p->assoc_template was also NULL \n");
#endif
                       }
#else

#error "DEAD CODE!"

                    if (edg_template != p->assoc_template)
                       {
                         ROSE_ASSERT(edg_template == NULL);
                         edg_template = p->assoc_template;
                       }
                    ROSE_ASSERT(edg_template != NULL);
#endif
                  }
           // ROSE_ASSERT(edg_template != NULL);
             }
            else
             {
               if (edg_template != NULL)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("Note that edg_template is already set (not typical but OK) edg_template = %p \n",edg_template);
#endif
                  }
            // ROSE_ASSERT(edg_template == NULL);
               ROSE_ASSERT(edg_template == NULL || edg_template == p->assoc_template);

#ifdef DEBUG_CONVERSION
               printf ("In setDefiningAndFirstNondefiningDeclarations(): edg_template not set (case forceTemplateDeclaration == false), but required, using value in p->assoc_template = %p \n",p->assoc_template);
#endif
               edg_template = p->assoc_template;
             }

       // DQ (8/10/2012): I think it does not make any different what the edg_template is, since we need
       // to be using the a_routine_ptr variable "p" as the hash key to look up the template function.

#if 0
          printf ("In setDefiningAndFirstNondefiningDeclarations(): edg_template = %p (used to index EDG/ROSE maps) \n",edg_template);
#endif
          if (edg_template != NULL)
             {
               if (hasFirstNondefDeclForTemplate(edg_template))
                  {
                 // DQ (4/12/2012): I would like to have the first_nondefining_declaration NOT already set if we set it below (is this reasonable).
                    ROSE_ASSERT(first_nondefining_declaration == NULL);

                 // This test protects the map from having a NULL pointer being added.
                    SgDeclarationStatement* templateDeclaration = getFirstNondefDeclForTemplate(edg_template);
                    ROSE_ASSERT(templateDeclaration != NULL);
                    first_nondefining_declaration = isSgFunctionDeclaration(templateDeclaration);
                    ROSE_ASSERT(first_nondefining_declaration != NULL);
#if 0
                    printf ("Setting first_nondefining_declaration = %p = %s from first_nondefining_declarations_for_templates EDG/ROSE map \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
#endif
#if 0
                 // It makes no sense to compare types and symbols (I think there is a silly error in what I wanted to test).

                 // DQ (4/25/2012): Error checking...next step would be to put the testing into the testEdgRoseMaps() function.
                    if (p->source_corresp.is_class_member == true && first_nondefining_declaration != NULL)
                       {
                      // DQ (4/25/2012): Make sure that the nondefiningTemplateMemberFunctionDeclaration has the correct type of symbol (SgTemplateMemberFunctionSymbol).
                         if (isSgTemplateMemberFunctionSymbol(first_nondefining_declaration->get_type()) == NULL)
                            {
                              printf ("first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
                              printf ("first_nondefining_declaration->get_type() = %p = %s \n",first_nondefining_declaration->get_type(),first_nondefining_declaration->get_type()->class_name().c_str());
                            }
                         ROSE_ASSERT(isSgTemplateMemberFunctionSymbol(first_nondefining_declaration->get_type()) != NULL);
                       }
#endif
                  }

            // DQ (8/10/2012): Use the identification of the template as a trigger to lookup the function in a new template 
            // function map (using a_routine_ptr p as the hash key).  This solves a problem identified in test2012_174.C.
               if (first_nondefining_declarations_for_template_functions.find(p) != first_nondefining_declarations_for_template_functions.end())
                  {
                    first_nondefining_declaration = first_nondefining_declarations_for_template_functions[p];

                  // DQ (1/27/2013): Added early test for code failing in buildDefiningFunctionDeclaration_T().
                  // verifyFunctionHasSymbolInAssociatedScope(first_nondefining_declaration);
                  }
                else
                 {
                // For now at least return the same function as before (important for test2012_174.C).
                   first_nondefining_declaration = NULL;
                 }

               if (hasDefiningDeclForTemplate(edg_template))
                  {
                 // DQ (4/12/2012): I would like to have the return_defining_declaration NOT already set if we set it below (is this reasonable).
                    ROSE_ASSERT(return_defining_declaration == NULL);

                 // This test protects the map from having a NULL pointer being added.
                    SgDeclarationStatement* templateDeclaration = getDefiningDeclForTemplate(edg_template);
                    ROSE_ASSERT(templateDeclaration != NULL);
                    return_defining_declaration = isSgFunctionDeclaration(templateDeclaration);
                    ROSE_ASSERT(return_defining_declaration != NULL);
                  }

            // DQ (8/10/2012): Use the identification of the template as a trigger to lookup the function in a new template 
            // function map (using a_routine_ptr p as the hash key).  This solves a problem identified in test2012_174.C.
               if (defining_declarations_for_template_functions.find(p) != defining_declarations_for_template_functions.end())
                  {
                    return_defining_declaration = defining_declarations_for_template_functions[p];
                  }
                 else
                  {
                 // For now at least return the same function as before (important for test2012_174.C).
                    return_defining_declaration = NULL;
                  }
             }
#if 0
          printf ("In setDefiningAndFirstNondefiningDeclarations(): Is this EDG routine associated with a ROSE function in the first_nondefining_declarations_for_functions EDG/ROSE map: %s \n",
               first_nondefining_declarations_for_functions.find(p) != first_nondefining_declarations_for_functions.end() ? "true" : "false");
#endif
        }
       else
        {
          if (first_nondefining_declarations_for_functions.find(p) != first_nondefining_declarations_for_functions.end())
             {
            // The conditional test (above) protects the map from having a NULL pointer being added.
               first_nondefining_declaration = first_nondefining_declarations_for_functions[p];
               ROSE_ASSERT(first_nondefining_declaration != NULL);
               ROSE_ASSERT(first_nondefining_declaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);
#if 0
               printf ("Setting first_nondefining_declaration = %p = %s from first_nondefining_declarations_for_functions EDG/ROSE map \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
#endif
            // DQ (1/27/2013): Added early test for code failing in buildDefiningFunctionDeclaration_T().
            // verifyFunctionHasSymbolInAssociatedScope(first_nondefining_declaration);
             }

          if (defining_declarations_for_functions.find(p) != defining_declarations_for_functions.end())
             {
            // This test protects the map from having a NULL pointer being added.
               return_defining_declaration = defining_declarations_for_functions[p];
               ROSE_ASSERT(return_defining_declaration != NULL);
             }

          if (p->assoc_template != NULL)
             {
#if 0
               printf ("In setDefiningAndFirstNondefiningDeclarations(): Is this EDG routine associated with a ROSE function in the first_nondefining_declarations_for_templates EDG/ROSE map: %s \n",
                    first_nondefining_declarations_for_templates.find(p->assoc_template) != first_nondefining_declarations_for_templates.end() ? "true" : "false");
#endif
             }
        }

     if (first_nondefining_declaration != NULL)
        {
#if 0
          printf ("In setDefiningAndFirstNondefiningDeclarations(): (first_nondefining_declaration != NULL): template parameter list and template argument list is not available (can't verify correctness) \n");
#endif
       // DQ (8/7/2013): Adding support for template functions overloaded using template parameters.
       // DQ (1/27/2013): Added early test for code failing in buildDefiningFunctionDeclaration_T().
       // verifyFunctionHasSymbolInAssociatedScope(first_nondefining_declaration);

          SgTemplateArgumentPtrList*  templateArgumentList = getTemplateArgumentList(first_nondefining_declaration);

       // DQ (8/12/2013): Added templateParameterList.
       // verifyFunctionHasSymbolInAssociatedScope(first_nondefining_declaration,NULL,templateArgumentList);
          SgTemplateParameterPtrList* templateParameterList = getTemplateParameterList(first_nondefining_declaration);
          verifyFunctionHasSymbolInAssociatedScope(first_nondefining_declaration,templateParameterList,templateArgumentList);
#if 0
          printf ("In setDefiningAndFirstNondefiningDeclarations(): WARNING: (first_nondefining_declaration!= NULL) call to verifyFunctionHasSymbolInAssociatedScope() not passing template arguments! \n");
#endif
        }

     if (return_defining_declaration != NULL)
        {
#if 0
          printf ("In setDefiningAndFirstNondefiningDeclarations(): (return_defining_declaration != NULL): template parameter list and template argument list is not available (can't verify correctness) \n");
#endif
       // DQ (8/7/2013): Adding support for template functions overloaded using template parameters.
       // DQ (1/27/2013): Added early test for code failing in buildDefiningFunctionDeclaration_T().
       // verifyFunctionHasSymbolInAssociatedScope(return_defining_declaration);

          SgTemplateArgumentPtrList* templateArgumentList = getTemplateArgumentList(return_defining_declaration);

       // DQ (8/12/2013): Added templateParameterList.
       // verifyFunctionHasSymbolInAssociatedScope(return_defining_declaration,NULL,templateArgumentList);
          SgTemplateParameterPtrList* templateParameterList = getTemplateParameterList(first_nondefining_declaration);
          verifyFunctionHasSymbolInAssociatedScope(return_defining_declaration,templateParameterList,templateArgumentList);
#if 0
          printf ("In setDefiningAndFirstNondefiningDeclarations(): WARNING: (return_defining_declaration != NULL) call to verifyFunctionHasSymbolInAssociatedScope() not passing template arguments! \n");
#endif
        }

#if 0
     printf ("At base of setDefiningAndFirstNondefiningDeclarations(): first_nondefining_declaration = %p return_defining_declaration = %p edg_template = %p \n",first_nondefining_declaration,return_defining_declaration,edg_template);
#endif

     if (first_nondefining_declaration != NULL && first_nondefining_declaration->get_firstNondefiningDeclaration() != first_nondefining_declaration)
        {
       // This is an error.
          printf ("Error: In setDefiningAndFirstNondefiningDeclarations(): (first_nondefining_declaration != NULL && first_nondefining_declaration->get_firstNondefiningDeclaration() != first_nondefining_declaration) \n");
          printf ("   --- first_nondefining_declaration = %p = %s = %s \n",first_nondefining_declaration,
               first_nondefining_declaration->class_name().c_str(),first_nondefining_declaration->get_name().str());

          printf ("   --- first_nondefining_declaration->get_mangled_name() = %s \n",first_nondefining_declaration->get_mangled_name().str());

          ROSE_ASSERT(isSgFunctionDeclaration(first_nondefining_declaration->get_firstNondefiningDeclaration()) != NULL);
          printf ("   --- first_nondefining_declaration->get_firstNondefiningDeclaration() = %p = %s = %s \n",first_nondefining_declaration->get_firstNondefiningDeclaration(),
               first_nondefining_declaration->get_firstNondefiningDeclaration()->class_name().c_str(),
               isSgFunctionDeclaration(first_nondefining_declaration->get_firstNondefiningDeclaration())->get_name().str());
          printf ("   --- first_nondefining_declaration->get_firstNondefiningDeclaration()->get_mangled_name() = %s \n",first_nondefining_declaration->get_firstNondefiningDeclaration()->get_mangled_name().str());
#if 0
          printf ("In setDefiningAndFirstNondefiningDeclarations(): calling summarize_routine() before checking for if this has previously been translated into return_defining_declaration!\n");
          summarize_routine(p);
#endif
        }
     ROSE_ASSERT(first_nondefining_declaration == NULL || first_nondefining_declaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);

#if 0
     printf ("Leaving setDefiningAndFirstNondefiningDeclarations(): first_nondefining_declaration = %p return_defining_declaration = %p edg_template = %p \n",first_nondefining_declaration,return_defining_declaration,edg_template);
#endif
   }

void setTemplateArgumentParents(SgTemplateArgumentPtrList & templateArgumentsList, SgDeclarationStatement* nondefining_templateInstantiation)
   {
  // DQ (7/25/2012): The parent should point to the SgTemplateInstantiationDecl, not the SgGlobal.

     ROSE_ASSERT(nondefining_templateInstantiation != NULL);

  // DQ (9/13/2012): This is the new test in SageBuilder: the parents should have already been set (test this).
     SageBuilder::testTemplateArgumentParents(nondefining_templateInstantiation);
  // setTemplateArgumentParents(nondefining_templateInstantiation);

#if 0
     for (size_t i = 0; i < templateArgumentsList.size(); i++)
        {
       // DQ (7/25/2012): I don't know if we can assert these conditions.
          ROSE_ASSERT(templateArgumentsList[i]->get_parent() != NULL);
          ROSE_ASSERT(templateArgumentsList[i]->get_parent() != nondefining_templateInstantiation);

          templateArgumentsList[i]->set_parent(nondefining_templateInstantiation);
        }
#endif
   }

void set_noreturn_attribute( a_type_ptr type, SgInitializedName* initializedName )
   {
  // DQ (1/18/2014): Refactored this function.
  // NOTE: This might be better to put onto the function type.

  // DQ (1/6/2014): Adding support for noreturn GNU attribute.
     if (type != NULL)
        {
#if 0
          printf ("************ In set_noreturn_attribute() (support for noreturn attribute): initializedName->get_name() = %s \n",initializedName->get_name().str());
#endif
#if 0
          printf ("In set_noreturn_attribute(): Calling summarize_type() on parameter (type): type = %p name = %s \n",type,initializedName->get_name().str());
          summarize_type(type);
#endif

#if 0
          printf ("is_pointer_type(type) = %s \n",is_pointer_type(type) ? "true" : "false");
#endif
       // DQ (1/6/2014): Added support for detecting the use of GNU noreturn attribute when used 
       // with function pointer types function parameters.
          if (is_pointer_type(type) == true)
             {
#if 0
               printf ("type_pointed_to(type) = %p \n",type_pointed_to(type));
#endif
            // printf ("is_function_type(type_pointed_to(var->type)) = %s \n",is_function_type(type_pointed_to(var->type)) ? "true" : "false");
               a_type_ptr base_type = type_pointed_to(type);
               if (base_type != NULL)
                  {
                    a_type_ptr typeref_base_type = skip_typerefs(base_type);
#if 0
                    printf ("typeref_base_type = %p \n",typeref_base_type);
#endif
                    if (typeref_base_type != NULL)
                       {
                         bool isFunctionType = is_function_type(typeref_base_type);
#if 0
                         printf ("isFunctionType = %s \n",isFunctionType ? "true" : "false");
#endif
                         if (isFunctionType == true)
                            {
#if 0
                              printf ("Detected a function type! \n");
#endif
#if 0
                              printf ("Calling summarize_type(typeref_base_type) \n");
                              summarize_type(typeref_base_type);
#endif
                              ROSE_ASSERT(typeref_base_type->kind == tk_routine);
                              ROSE_ASSERT(typeref_base_type->variant.routine.extra_info != NULL);
                              if (typeref_base_type->variant.routine.extra_info->does_not_return == true)
                                 {
                                   ROSE_ASSERT(initializedName != NULL);
                                   initializedName->setGnuAttributeNoReturn();
#if 0
                                   printf ("Detected noreturn attribute on function pointer: Exiting as a test! \n");
                                   ROSE_ASSERT(false);
#endif
                                 }
                            }
                           else
                            {
#if 0
                              printf ("Calling summarize_type(typeref_base_type) \n");
                              summarize_type(typeref_base_type);
#endif
                            }
                       }
                      else
                       {
#if 0
                         printf ("Calling summarize_type(base_type): What is this? \n");
                         summarize_type(base_type);
#endif
                       }
                  }
             }
        }
   }

void set_linkage(a_routine_ptr p, SgFunctionDeclaration* result)
   {
  // DQ (7/29/2012): We need to correctly mark declarations as extern "C" when they are declared that way.
  // Note that extern "C" is captured by the lexer for comments and CPP directives, but it is not inserted
  // correctly if the opening and cloasing braces are on the same line; see test2005_15.C as an example.
  // However, if the declarations are correctly marked as extern "C" then there is not a problem (the unparser
  // will still output the correct code and the redundant extern "C" {} will be output as well (this should 
  // be fixed later)).
     bool is_definition               = false;

#if 0
     printf ("In set_linkage(): innermost_function_scope                                           = %p \n",innermost_function_scope);
     printf ("In set_linkage(): global_innermost_function_scope                                    = %p \n",global_innermost_function_scope);
     printf ("In set_linkage(): il_header.source_language == sl_Cplusplus                          = %s \n",il_header.source_language == sl_Cplusplus ? "true" : "false");
     printf ("In set_linkage(): p->source_corresp.name_linkage ==(a_name_linkage_kind)nlk_external = %s \n",p->source_corresp.name_linkage ==(a_name_linkage_kind)nlk_external ? "true" : "false");
#endif

  // DQ (1/22/2013): I would like to not use the global_innermost_function_scope as a predicate in this statement.
  // See test2013_25.C for where this was causing the extern "C" markings on a function to be dropped.
  // if (il_header.source_language == sl_Cplusplus && p->source_corresp.name_linkage == (a_name_linkage_kind) nlk_external && global_innermost_function_scope == NULL) 
     if (il_header.source_language == sl_Cplusplus && p->source_corresp.name_linkage == (a_name_linkage_kind) nlk_external) 
        {
       // Inside a function, this is not allowed, and can only have come from an extern "C" { ... } wrapped around the function.
       // markAsExternC = true;
#if 0
          printf ("In set_linkage(): marking as extern C result = %p = %s \n",result,result->get_name().str());
#endif
       // For a definition, use the form
       //    extern "C" { int i; };
       // because simply
       //    extern "C" int i;
       // is no longer a definition. */
          if (is_definition) 
             {
            // need_extern_C_closing_brace = TRUE;
            // markAsNeedBrace = true;
             }

          result->get_declarationModifier().get_storageModifier().setExtern();
          result->set_linkage("C");
          result->setExternBrace();
        }

#if 0
  // printf ("In set_linkage(a_routine_ptr,SgFunctionDeclaration): markAsExternC               = %s \n",markAsExternC ? "true" : "false");
  // printf ("In set_linkage(a_routine_ptr,SgFunctionDeclaration): need_extern_C_closing_brace = %s \n",need_extern_C_closing_brace ? "true" : "false");
  // printf ("In set_linkage(a_routine_ptr,SgFunctionDeclaration): markAsNeedBrace             = %s \n",markAsNeedBrace ? "true" : "false");
     printf ("In set_linkage(a_routine_ptr,SgFunctionDeclaration): is_definition               = %s \n",is_definition ? "true" : "false");
#endif
#if 0
     printf ("In set_linkage(a_routine_ptr,SgFunctionDeclaration): result->get_declarationModifier().get_storageModifier().isExtern() = %s \n",result->get_declarationModifier().get_storageModifier().isExtern() ? "true" : "false");
     printf ("In set_linkage(a_routine_ptr,SgFunctionDeclaration): result->get_linkage().empty() = %s \n",result->get_linkage().empty() ? "true" : "false");
     printf ("In set_linkage(a_routine_ptr,SgFunctionDeclaration): result->get_linkage()         = %s \n",result->get_linkage().c_str());
     printf ("In set_linkage(a_routine_ptr,SgFunctionDeclaration): result->isExternBrace()       = %s \n",result->isExternBrace() ? "true" : "false");
#endif
   }

SgScopeStatement* getDeclarationScope ( a_source_correspondence & scp , bool expect_nonreal_scope )
   {
  // DQ (7/6/2019): This function is call to determine the scope associated with each declaration as it is constructed.

  // DQ (7/6/2019): If we are supporting C then this function should return the topScopeStack()
  // If not then unions and struct types will be assgined global scope, which is an error.
  // for C++, because of name qualification, the solution below appears to be correct, and we 
  // cannot use the topScopeStack().

     SgScopeStatement* scopeOfDeclaration = NULL;

#if 1
  // DQ (7/6/2019): For C language support we can use the topScopeStack(), because name qualification is 
  // not a part of the C language.  Only with name qualificaiton can the declaration scope be different.
  // Note: the variable "il_header.source_language" can be only either "sl_C" or "sl_Cplusplus".
     if (il_header.source_language == sl_C)
        {
          scopeOfDeclaration = topScopeStack();
#if 0
          printf ("In getDeclarationScope(): (il_header.source_language == sl_C): using topScopeStack(): scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

       // DQ (7/7/2019): Added support tohandle the special case of function prototypes.
       // DQ (4/8/2018): Look for prototype scopes so that we can handle them more accurately.
       // ROSE_ASSERT(scp.parent_scope != NULL);
          if (scp.parent_scope != NULL && scp.parent_scope->kind == sck_func_prototype)
             {
#if DEBUG_GET_DECLARATION_SCOPE
               printf ("$$$$$$$$$$$$$$$$$$$$$$$ In getDeclarationScope(): (il_header.source_language == sl_C): This is a function prototype scope! \n");
#endif

#if 0
            // ROSE_ASSERT(scp.is_local_to_function == true);
               if (scp.is_local_to_function == true)
                  {
                    ROSE_ASSERT(internal_intermediate_scope != NULL);
                    ROSE_ASSERT(isSgDeclarationScope(internal_intermediate_scope) != NULL);

#if DEBUG_GET_DECLARATION_SCOPE
                    printf ("Leaving getDeclarationScope(): (il_header.source_language == sl_C): setting scopeOfDeclaration to internal_intermediate_scope = %p = %s \n",internal_intermediate_scope,internal_intermediate_scope->class_name().c_str());
#endif
                    scopeOfDeclaration = internal_intermediate_scope;
                 }
#endif
            // DQ (7/7/2019): Setup an entry for the parent scope of the types in the function prototype.
               if (scope_cache.find(scp.parent_scope) == scope_cache.end())
                  {
                    SgScopeStatement* declarationScope = getGlobalScopeFromScopeStack();
                    scope_cache[scp.parent_scope] = declarationScope;
                  }

#if 0
               printf ("Found a function prototype scope: exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }

       // DQ (7/7/2019): Added this so that we could support types (which should not have the scope of the topScopeStack()).
          if (scope_cache.find(scp.parent_scope) != scope_cache.end())
             {
               scopeOfDeclaration = scope_cache[scp.parent_scope];

#if DEBUG_GET_DECLARATION_SCOPE
               printf ("$$$$$$$$$$$$$$$$$$$$$$$ In getDeclarationScope(): (il_header.source_language == sl_C): found valid scope from cache: scopeOfDeclaration = %p \n",scopeOfDeclaration);
#endif
             }
            else
             {
#if 0
               printf ("Leaving getDeclarationScope(): (il_header.source_language == sl_C): scope not found in scope_cache! \n");
#endif
             }

#if DEBUG_GET_DECLARATION_SCOPE
          printf ("Leaving getDeclarationScope(): (il_header.source_language == sl_C): scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif
          return scopeOfDeclaration;
        }
#else
     printf ("In getDeclarationScope(): Commented out use of C language specific topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif

  // DQ (7/19/2017): Note that I think the is_nonreal_class variable is not used in getDeclarationScope().

  // Many C++ declarations use name qualification to be structurally in one scope 
  // while they are logically a part of another scope (e.g. member functions can 
  // be defined outside of their class).  This function provides the support to
  // identify the logical scope (not the structural scope) of any declaration.
  // if it is not found in the scope_cache map then it is assumed to be the structural
  // scope (the current scope in the processing of EDG's source sequence list).

#if DEBUG_GET_DECLARATION_SCOPE || 0
     printf ("\nIn TOP of getDeclarationScope():\n");
     printf (" - scp.parent_scope = %p (%s)\n",scp.parent_scope, scp.parent_scope ? summarize_scope_in_string(scp.parent_scope) : "");
     printf (" - expect_nonreal_scope = %s\n", expect_nonreal_scope ? "true" : "false");
     printf (" - scp.name = %s \n",scp.name);
#endif
#if 0
     printf ("In getDeclarationScope(): calling summarize_source_correspondence() \n");
     summarize_source_correspondence(scp);
#endif

  // In the case of variabel references in function scopes that have not yet been defined (built in ROSE) 
  // we want to return NULL fom this function so that we can avoid adding the associated variable reference 
  // to the incorrect scope.
     bool returnNull = false;

     if (scp.parent_scope != NULL)
        {
          expect_nonreal_scope = expect_nonreal_scope || (scp.parent_scope->kind == sck_template_declaration);
#if 0
          printf ("expect_nonreal_scope = %s \n",expect_nonreal_scope ? "true" : "false");
#endif
          if (expect_nonreal_scope) 
             {
               if (nonreal_scope_cache.find(scp.parent_scope) != nonreal_scope_cache.end()) 
                  {
                 // return nonreal_scope_cache[scp.parent_scope];
                    SgScopeStatement* nonreal_scope = nonreal_scope_cache[scp.parent_scope];
#if 0
                    printf ("Returning from nonreal_scope_cache: nonreal_scope = %p = %s \n",nonreal_scope,nonreal_scope->class_name().c_str());
#endif
                    return nonreal_scope;
               // } else {
               //   return NULL;
                  }
                 else
                  {
#if 0
                    printf ("Check the nonreal_scope_cache: \n");
                 // std::map<a_scope_ptr, SgDeclarationScope*> nonreal_scope_cache;
                    std::map<a_scope_ptr, SgDeclarationScope*>::iterator i = nonreal_scope_cache.begin();
                    while (i != nonreal_scope_cache.end())
                       {
                         a_scope_ptr         edg_scope        = i->first;
                         SgDeclarationScope* rose_declaration = i->second;

                         ROSE_ASSERT(edg_scope != NULL);
                         ROSE_ASSERT(rose_declaration != NULL);

                         printf (" --- edg_scope        = %p \n",edg_scope);
                         printf (" --- calling summarize_scope() \n");
                         summarize_scope(edg_scope);
                         printf (" --- rose_declaration = %p = %s name = %s \n",rose_declaration,rose_declaration->class_name().c_str(),SageInterface::get_name(rose_declaration).c_str());
                         printf (" --- --- rose_declaration->get_symbol_table(): \n");
                         rose_declaration->get_symbol_table()->print("rose_declaration->get_symbol_table()");

                         i++;
                       }

                    printf ("DONE: Check the nonreal_scope_cache: \n");
#endif
                  }
             }

       // DQ (7/5/2017): Debugging info.
#if DEBUG_GET_DECLARATION_SCOPE
          SgScopeStatement* tmp_scopeOfDeclaration = topScopeStack();
          printf ("In getDeclarationScope(): topScopeStack(): scopeOfDeclaration = %p = %s \n",tmp_scopeOfDeclaration,tmp_scopeOfDeclaration->class_name().c_str());
#endif

#if DEBUG_GET_DECLARATION_SCOPE && 0
          printf ("In getDeclarationScope(): calling summarize_scope() for scp.parent_scope = %p \n",scp.parent_scope);
          summarize_scope(scp.parent_scope);
#endif

#if 0
       // DQ (4/8/2018): Look for prototype scopes so that we can handle them more accurately.
          if (scp.parent_scope->kind == sck_func_prototype)
             {
#if 0
               printf ("In getDeclarationScope(): This is a function prototype scope! \n");
#endif
               ROSE_ASSERT(scp.is_local_to_function == true);
               ROSE_ASSERT(internal_intermediate_scope != NULL);
               ROSE_ASSERT(isSgDeclarationScope(internal_intermediate_scope) != NULL);
#if 0
               printf ("internal_intermediate_scope = %p = %s \n",internal_intermediate_scope,internal_intermediate_scope->class_name().c_str());
#endif
               tmp_scopeOfDeclaration = internal_intermediate_scope;
#if 0
               printf ("Found a function prototype scope: exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }
#endif

       // For situations where it makes a difference we store the scope in a map (e.g. where it is not the current scope).

#if 0
          printf ("Step 1: scp.parent_scope = %p scp.name = %s scope_cache.find(scp.parent_scope) != scope_cache.end() = %s \n",
               scp.parent_scope,scp.name,(scope_cache.find(scp.parent_scope) != scope_cache.end()) ? "true" : "false");
#endif

       // DQ (12/28/2011): If the scope is present in the cache, then it should be non-NULL.
       // ROSE_ASSERT(scope_cache.find(scp.parent_scope) != scope_cache.end());
          if (scope_cache.find(scp.parent_scope) != scope_cache.end())
             {
               if (scope_cache[scp.parent_scope] == NULL)
                  {
#if DEBUG_GET_DECLARATION_SCOPE
                    printf ("Error: null entry in scope_cache --- scope_cache[scp.parent_scope] == NULL: scp.parent_scope = %p \n",scp.parent_scope);
#endif
#if 0
                    a_scope_ptr parent_scope = scp.parent_scope->parent;
                    while (parent_scope != NULL)
                       {
                         printf ("   This is the chain of parent scopes: edg parent_scope = %s \n",summarize_scope_in_string(parent_scope));
                         parent_scope = parent_scope->parent;
                       }
#endif
                  }

               if (scp.parent_scope->kind == sck_block)
                  {
                 // DQ (10/31/2012): In this case we can use the new statement_scope_cache 
                 // to find the ROSE scope from the edg sck_block statement.
#if DEBUG_GET_DECLARATION_SCOPE
                    printf ("Allow this case of sck_block (use the statement_scope_cache to retrieve the ROSE scope) \n");
#endif
#if 0
                    printf ("In getDeclarationScope(): calling summarize_scope() for sck_block \n");
                    summarize_scope(scp.parent_scope);
#endif
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
                    ROSE_ASSERT(scope_cache[scp.parent_scope] != NULL);
                  }
             }

#if 0
          printf ("Step 2: scp.parent_scope = %p scp.name = %s scope_cache.find(scp.parent_scope) != scope_cache.end() = %s \n",
               scp.parent_scope,scp.name,(scope_cache.find(scp.parent_scope) != scope_cache.end()) ? "true" : "false");
#endif
       // DQ (3/11/2012): check that this is not already in the list before we add it (or
       // verify it is NULL if it is supposed to be in the list but just not initialized yet).
       // ROSE_ASSERT (scope_cache.find(scp.parent_scope) != scope_cache.end());
          if (scope_cache.find(scp.parent_scope) != scope_cache.end())
             {
               scopeOfDeclaration = scope_cache[scp.parent_scope];
#if DEBUG_GET_DECLARATION_SCOPE
               printf ("In getDeclarationScope(): found valid scope from cache: scp.parent_scope = %p scopeOfDeclaration = %p \n",scp.parent_scope,scopeOfDeclaration);
#endif
               if (scopeOfDeclaration == NULL)
                  {
#if 0
                    printf ("We have not seen this edg scope previously, what is it? \n");
                    printf ("   This is an edg scope = %s \n",summarize_scope_in_string(scp.parent_scope));
#endif
#if 0
                    a_scope_ptr parent_scope = scp.parent_scope->parent;
                    while (parent_scope != NULL)
                       {
                         printf ("   This is the chain of parent scopes: edg parent_scope = %s \n",summarize_scope_in_string(parent_scope));
                         parent_scope = parent_scope->parent;
                       }
#endif
                  }
                 else
                  {
#if 0
                    printf ("In getDeclarationScope(): scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
                    scopeOfDeclaration->get_file_info()->display("In getDeclarationScope(): scopeOfDeclaration");
#endif
#if 0
                    printf ("In getDeclarationScope(): calling SageInterface::recursivePrintCurrentAndParent() for scopeOfDeclaration = %p = %s \n",
                         scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
                    SageInterface::recursivePrintCurrentAndParent(scopeOfDeclaration);
#endif
                  }
#if 0
               printf ("Step A: scp.parent_scope = %p scp.name = %s scope_cache.find(scp.parent_scope) != scope_cache.end() = %s \n",
                    scp.parent_scope,scp.name,(scope_cache.find(scp.parent_scope) != scope_cache.end()) ? "true" : "false");
#endif
             }
            else
             {
            // DQ (3/12/2012): I would like to make this an error if possible, but it happens for STL (see test2012_07.C).
#if DEBUG_GET_DECLARATION_SCOPE
               printf ("WARNING: In getDeclarationScope(): we can't assume scope_cache.find(scp.parent_scope = %p) != scope_cache.end() \n",scp.parent_scope);
#endif
#if 0
               printf ("In getDeclarationScope(): Calling summarize_scope() \n");
               summarize_scope(scp.parent_scope);
#endif
#if DEBUG_GET_DECLARATION_SCOPE
               printf ("scp.parent_scope->assoc_block = %p \n",scp.parent_scope->assoc_block);
#endif
            // DQ (1/31/2018): Add the support to use the assoc_scope as an alternative.
            // a_scope_ptr associated_block_scope = scp.parent_scope->assoc_block != NULL ? scp.parent_scope->assoc_block->assoc_scope : NULL;
               a_statement_ptr associated_block_statement = scp.parent_scope->assoc_block;
#if 0
               printf ("associated_block_statement = %p \n",associated_block_statement);
#endif
               if (associated_block_statement != NULL)
                  {
                 // DQ (1/31/2018): Note that the ROSE scope will be found in the statement_scope_cache if it is a stmk_block.
                    ROSE_ASSERT(associated_block_statement != NULL);
                    ROSE_ASSERT(associated_block_statement->kind == stmk_block);

                 // DQ (1/31/2018): We can't assert this since it fails for test2006_54.C.
                 // ROSE_ASSERT(statement_scope_cache.find(associated_block_statement) != statement_scope_cache.end());

                 // DQ (1/31/2018): Add the support to use the assoc_scope as an alternative.
                    if (statement_scope_cache.find(associated_block_statement) != statement_scope_cache.end())
                       {
                         scopeOfDeclaration = statement_scope_cache[associated_block_statement];
#if 0
                         printf ("Found scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif
#if 0
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                         ROSE_ASSERT(scopeOfDeclaration != NULL);
#if 0
                         printf ("############# Returning from getDeclarationScope() with scopeOfDeclaration \n");
#endif
                         return scopeOfDeclaration;
                       }
                      else
                       {
#if 0
                         printf ("Note: identified scope is not present in statement_scope_cache \n");
#endif
                       }
                  }
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
            // In this case the EDG scope has not been processed previously, now would be a good time to do this.
            // This happens for class template instantiations.
#if DEBUG_GET_DECLARATION_SCOPE
               printf ("NOTE: In getDeclarationScope(): processing the type to get the scope \n");
#endif
            // This is the case when triggered by a function reference, other cases might require different code (e.g. member variable reference).
               a_type_ptr edg_type = NULL;
               if (scp.parent_scope->kind == sck_func_prototype     || scp.parent_scope->kind == sck_class_struct_union || 
                   scp.parent_scope->kind == sck_class_reactivation || scp.parent_scope->kind == sck_enum)
                  {
#if DEBUG_GET_DECLARATION_SCOPE
                    printf ("Setting the edg_type using scp.parent_scope->variant.assoc_type = %p \n",scp.parent_scope->variant.assoc_type);
#endif
                    edg_type = scp.parent_scope->variant.assoc_type;
                  }
            // ROSE_ASSERT(edg_type != NULL);

               if (edg_type != NULL)
                  {
#if 0
                 // DQ (4/24/2012): This type may not have been processed yet.
                    SgType* associated_rose_type = type_translation_cache[edg_type];
                    ROSE_ASSERT(associated_rose_type != NULL);
                    printf ("associated_rose_type = %p = %s \n",associated_rose_type,associated_rose_type->class_name().c_str());

                    ROSE_ASSERT(type_translation_cache.find(edg_type) != type_translation_cache.end());
#endif
                    SgClassDeclaration* definingClassDeclaration         = NULL;
                    SgClassDeclaration* firstNondefiningClassDeclaration = NULL;

                    SgScopeStatement* declarationScope = NULL;
#if 0
                    printf ("Check if we have processed this previously: type_declaration_map.find(edg_type) == type_declaration_map.end() = %s \n",
                         (type_declaration_map.find(edg_type) == type_declaration_map.end()) ? "true" : "false");
#endif
                    if (type_declaration_map.find(edg_type) == type_declaration_map.end())
                       {
#if DEBUG_GET_DECLARATION_SCOPE
                         printf ("@@@@@@@@@@@@@@ In getDeclarationScope(): type (edg_type = %p) not seen previously, processing it now \n",edg_type);
#endif
                      // DQ (11/12/2012): Check if this is a function type, if so then we could be processing function arguments of this 
                      // type and not want to start the recursive decent of no processing the function type for the arguments of the 
                      // function type that we are processing... (see test2012_158.c).
#if 0
                         printf ("In getDeclarationScope(): (edg_type = %p = %s) Always process the true branch: previously (edg_type->kind != tk_routine) \n",
                              edg_type,summarize_symbol_kind_in_string_kind(edg_type->kind));
#endif
                      // DQ (9/6/2020): This condition is not required, but I can't see that it provides much insight into RC-107.
                      // if (edg_type->kind != tk_routine)
                         if (true)
                            {
#if FIX_FOR_RC_107
                           // DQ(9/6/2020): This is the fix for RC-107, but it breaks Cxx11_tests/test2020_94.C (which is representative 
                           // of the failing tests in Cxx_tests directory if this fix is used.
#define DEBUG_REDUNDANT_TRANSLATION 1
                           // DQ (9/4/2020): Search the ROSE AST for the associated scope before possiblely retranslating the EDG AST redundantly.
                           // This is a fix for Cxx11_tests/test2020_91.C and test2020_92.C (simpler) and test2020_92.C.
                              a_scope_ptr parent_scope = scp.parent_scope;
                              ROSE_ASSERT(parent_scope != NULL);
#if DEBUG_REDUNDANT_TRANSLATION
                              printf ("First level of parent scope = %p \n",parent_scope);
#endif
                              parent_scope = parent_scope->parent;
                              ROSE_ASSERT(parent_scope != NULL);
#if DEBUG_REDUNDANT_TRANSLATION
                              printf ("Second level of parent scope = %p \n",parent_scope);
#endif
                              if (scope_cache.find(parent_scope) != scope_cache.end())
                                 {
                                // Check if the edg scope for the parent has been processed previously and if so we can use the ROSE lookup to find the associated scope.
                                   SgScopeStatement* rose_parent_scope = scope_cache[parent_scope];
                                   ROSE_ASSERT(rose_parent_scope != NULL);
#if DEBUG_REDUNDANT_TRANSLATION
                                   printf ("rose_parent_scope = %p = %s \n",rose_parent_scope,rose_parent_scope->class_name().c_str());
#endif
                                // SgName name_of_construct = scp.name;
                                   SgName name_of_construct = edg_type->source_corresp.name;
#if DEBUG_REDUNDANT_TRANSLATION
                                   printf ("name_of_construct = %s \n",name_of_construct.str());
#endif
#if DEBUG_REDUNDANT_TRANSLATION && 0
                                   printf ("Print out symbol table: rose_parent_scope = %p = %s \n",rose_parent_scope,rose_parent_scope->class_name().c_str());
                                   rose_parent_scope->get_symbol_table()->print("rose_parent_scope");
#endif
                                   SgSymbol* symbol = rose_parent_scope->lookup_symbol(name_of_construct);
                                // ROSE_ASSERT(symbol != NULL);

                                   if (symbol != NULL)
                                      {
#if DEBUG_REDUNDANT_TRANSLATION
                                        printf ("symbol = %p = %s \n",symbol,symbol->class_name().c_str());
#endif
                                        SgTemplateClassSymbol* templateClassSymbol = isSgTemplateClassSymbol(symbol);
                                        ROSE_ASSERT(templateClassSymbol != NULL);
#if DEBUG_REDUNDANT_TRANSLATION
                                        printf ("templateClassSymbol = %p = %s \n",templateClassSymbol,templateClassSymbol->class_name().c_str());
#endif
                                        SgDeclarationStatement* declaration = templateClassSymbol->get_declaration();
                                        ROSE_ASSERT(declaration != NULL);
#if DEBUG_REDUNDANT_TRANSLATION
                                        printf ("declaration = %p = %s \n",declaration,declaration->class_name().c_str());
#endif
                                        SgClassDeclaration* classDeclaration = isSgClassDeclaration(declaration);
                                        ROSE_ASSERT(classDeclaration != NULL);
#if DEBUG_REDUNDANT_TRANSLATION
                                        printf ("classDeclaration = %p = %s \n",classDeclaration,classDeclaration->class_name().c_str());
#endif
                                        SgClassDeclaration* definingClassDeclaration = isSgClassDeclaration(classDeclaration->get_definingDeclaration());
                                     // ROSE_ASSERT(definingClassDeclaration != NULL);
                                        if (definingClassDeclaration != NULL)
                                           {
#if DEBUG_REDUNDANT_TRANSLATION
                                             printf ("definingClassDeclaration = %p = %s \n",definingClassDeclaration,definingClassDeclaration->class_name().c_str());
#endif
                                             SgClassDefinition* classDefinition = definingClassDeclaration->get_definition();
                                             ROSE_ASSERT(classDefinition != NULL);
#if DEBUG_REDUNDANT_TRANSLATION
                                             printf ("classDefinition = %p = %s \n",classDefinition,classDefinition->class_name().c_str());
#endif
                                             SgTemplateClassDefinition* templateClassDefinition = isSgTemplateClassDefinition(classDefinition);
                                             ROSE_ASSERT(templateClassDefinition != NULL);
#if DEBUG_REDUNDANT_TRANSLATION
                                             printf ("Leaving getDeclarationScope(): using ROSE AST: templateClassDefinition = %p = %s \n",templateClassDefinition,templateClassDefinition->class_name().c_str());
#endif
#if 1
                                             printf ("Leaving getDeclarationScope(): using ROSE AST: scope_cache.find(scp.parent_scope) != scope_cache.end() = %s \n",
                                                  (scope_cache.find(scp.parent_scope) != scope_cache.end()) ? "true" : "false");
#endif
#if 0
                                             printf ("Exiting as a test! \n");
                                             ROSE_ASSERT(false);
#endif
                                             return templateClassDefinition;
                                           }
                                      }
                                 }
                                else
                                 {
                                // If the edg type associated with scope has not be processed then we need to do so.
#if 0
                                   printf ("Exiting as a test! \n");
                                   ROSE_ASSERT(false);
#endif
                                 }
#endif

#if 1
#if 0
                              printf ("In getDeclarationScope(): calling convert_type(): edg_type = %p \n",edg_type);
#endif
                           // DQ (9/6/2020): Calling this function causes some problems, but may fix some others (e.g. RC-107), 
                           // and this is confusing at this point.
                              SgType* parentscopeType = convert_type(edg_type);
                              ROSE_ASSERT(parentscopeType != NULL);
#else
                              printf ("In getDeclarationScope(): skip calling convert_type(): edg_type = %p \n",edg_type);
#endif
#if 0
                              printf ("After processing check if have in in the cache: edg_type = %p type_declaration_map.find(edg_type) == type_declaration_map.end() = %s \n",
                                      edg_type,(type_declaration_map.find(edg_type) == type_declaration_map.end()) ? "true" : "false");
#endif
#if 0
                              a_scope_ptr assoc_scope = edg_type->assoc_scope;
                              printf ("After processing check if have in in the cache: edg_scope = %p type_declaration_map.find(edg_scope) == type_declaration_map.end() = %s \n",
                                      edg_scope,(nonreal_scope_cache.find(assoc_scope) == nonreal_scope_cache.end()) ? "true" : "false");
#endif
#if 0
                              printf ("After processing check if have in in the cache: calling summarize_type(edg_type = %p) \n",edg_type);
                              summarize_type(edg_type);
#endif
                           // ROSE_ASSERT((type_declaration_map.find(edg_type) != type_declaration_map.end()) == true);

                            }
                           else
                            {
                           // DQ (3/6/2017): Added support for message logging to control output from ROSE tools.
                           // DQ (11/21/2012): I don't know if we can always do this. What if there is a function type defined in the 
                           // functions argument type, is this possible in C?
#if 0
                              mprintf ("Skipping the processing of function types not previously see in the getDeclarationScope() function \n");
#endif
#if 0
                              printf ("Skipping the processing of function types not previously see in the getDeclarationScope() function \n");
#endif
#if 0
                              printf ("In getDeclarationScope(): calling summarize_scope() \n");
                              summarize_scope(scp.parent_scope);
#endif
                            }
#if DEBUG_GET_DECLARATION_SCOPE
                         printf ("DONE: @@@@@@@@@@@@@@ In getDeclarationScope(): type (edg_type = %p) not seen previously, processing it now \n",edg_type);
#endif
                       }

                 // ROSE_ASSERT(type_declaration_map.find(edg_type) != type_declaration_map.end());
                    if (type_declaration_map.find(edg_type) != type_declaration_map.end())
                       {
                         std::vector<SgDeclarationStatement*> & vectorOfDeclarations = type_declaration_map[edg_type];
#if DEBUG_GET_DECLARATION_SCOPE
                         printf ("In getDeclarationScope(): vectorOfDeclarations.size() = %" PRIuPTR " \n",vectorOfDeclarations.size());
#endif
                         for (size_t i = 0; i < vectorOfDeclarations.size(); i++)
                            {
                              ROSE_ASSERT(vectorOfDeclarations[i] != NULL);
                              SgClassDeclaration* classDeclaration = isSgClassDeclaration(vectorOfDeclarations[i]);
                              ROSE_ASSERT(classDeclaration != NULL);

                              if (definingClassDeclaration == NULL && classDeclaration->get_definition() != NULL)
                                 {
                                   definingClassDeclaration = classDeclaration;
#if DEBUG_GET_DECLARATION_SCOPE
                                   printf ("Setting the definingClassDeclaration = %p name = %s \n",definingClassDeclaration,definingClassDeclaration->get_name().str());
                                   printf ("   --- definingClassDeclaration      = %p = %s \n",definingClassDeclaration,definingClassDeclaration->class_name().c_str());
                                   printf ("   --- definingClassDeclaration      = %s \n",definingClassDeclaration->unparseNameToString().c_str());
#endif
                                 }

                              if (firstNondefiningClassDeclaration == NULL && classDeclaration->get_definition() == NULL)
                                 {
                                // DQ (9/21/2012): Modified to avoid pointing to any secondary declaration that in not the firstNondefiningDeclaration.
                                // firstNondefiningClassDeclaration = classDeclaration;
                                   ROSE_ASSERT(classDeclaration->get_firstNondefiningDeclaration() != NULL);
                                   firstNondefiningClassDeclaration = isSgClassDeclaration(classDeclaration->get_firstNondefiningDeclaration());
                                   ROSE_ASSERT(firstNondefiningClassDeclaration == firstNondefiningClassDeclaration->get_firstNondefiningDeclaration());
#if DEBUG_GET_DECLARATION_SCOPE
                                   printf ("Setting the firstNondefiningClassDeclaration = %p name = %s \n",firstNondefiningClassDeclaration,firstNondefiningClassDeclaration->get_name().str());
                                   printf ("   --- firstNondefiningClassDeclaration      = %p = %s \n",firstNondefiningClassDeclaration,firstNondefiningClassDeclaration->class_name().c_str());
                                   printf ("   --- firstNondefiningClassDeclaration      = %s \n",firstNondefiningClassDeclaration->unparseNameToString().c_str());
#endif
                                 }
                            }

                         ROSE_ASSERT(firstNondefiningClassDeclaration != NULL);
                      // ROSE_ASSERT(definingClassDeclaration == NULL);
                         if (definingClassDeclaration == NULL)
                            {
                           // If we have not yet processed the defining declaration then build one.
                           // SgClassDeclaration * buildClassDeclaration_nfi(const SgName& name, SgClassDeclaration::class_types kind, SgScopeStatement* scope, SgClassDeclaration* nonDefiningDecl, bool buildTemplateInstantiation = false);
                              SgName                          name                       = firstNondefiningClassDeclaration->get_name();
                              SgClassDeclaration::class_types kind                       = firstNondefiningClassDeclaration->get_class_type();

                           // DQ (4/28/2012): This is the scope of the firstNondefiningClassDeclaration.
                              SgScopeStatement*               scope                      = firstNondefiningClassDeclaration->get_scope();
                              ROSE_ASSERT(scope != NULL);
#if DEBUG_GET_DECLARATION_SCOPE
                              printf ("In getDeclarationScope(): building a new defining SgClassDeclaration firstNondefiningClassDeclaration = %p = %s name = %s \n",
                                   firstNondefiningClassDeclaration,firstNondefiningClassDeclaration->class_name().c_str(),name.str());
                              printf ("In getDeclarationScope(): scope = %p = %s \n",scope,scope->class_name().c_str());
#endif
                           // DQ (4/24/2012): Make the request to build a template instantiation based on the type of IR node used in the non-defining declaration.
                           // bool buildTemplateInstantiation = true;
                           // bool buildTemplateInstantiation = (isSgTemplateInstantiationDecl(firstNondefiningDeclaration) != NULL) ? true : false;
                           // bool buildTemplateInstantiation = (isSgTemplateInstantiationDefn(scope) != NULL) ? true : false;
                              bool buildTemplateInstantiation = (isSgTemplateInstantiationDecl(firstNondefiningClassDeclaration) != NULL) ? true : false;
#if DEBUG_GET_DECLARATION_SCOPE
                              printf ("In getDeclarationScope(): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");

                              printf ("************************************************************************* \n");
                              printf ("BUILDING PREVIOUSLY UNSEEN DEFINING DECLARATION FOR PARENT OF DECLARATION \n");
                              printf ("************************************************************************* \n");
#endif
                              if (isSgTemplateClassDeclaration(firstNondefiningClassDeclaration) != NULL)
                                 {
                                // Build this as a template declaration
#if DEBUG_GET_DECLARATION_SCOPE
                                   printf ("In getDeclarationScope(): Build this AS a template declaration \n");
#endif
#if DEBUG_GET_DECLARATION_SCOPE
                                   printf ("In getDeclarationScope(): Calling parse_template_parameters() \n");
#endif
                                // DQ (9/10/2012): Setup the template parameters (required to disambiguate templates when the names are not enough). See test2012_209.C
                                   SgTemplateParameterPtrList templateParameterList = parse_template_parameters(edg_type,scope,NULL);
#if DEBUG_GET_DECLARATION_SCOPE
                                   printf ("DONE: In getDeclarationScope(): Calling parse_template_parameters() \n");
#endif
                                // DQ (9/11/2012): Setup the template specialization arguments (these also have to be passed to the SageBuilder API (since they are also needed to disambiguate template names).
                                // a_template_arg_ptr templateSpecializationArgs = edg_type->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
                                // This is the list of template arguments on which the list of "actual" arguments is based.
                                   a_template_arg_ptr templateSpecializationArgs        = edg_type->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
                                // This is the list of "actual" template arguments.
                                   a_template_arg_ptr templateSpecializationArgs_actual = edg_type->variant.class_struct_union.extra_info->template_arg_list;
                                   SgTemplateArgumentPtrList templateSpecializationArgumentList;
                                   if (templateSpecializationArgs != NULL)
                                      {
                                     // DQ (9/12/2012): This is tricky: If this is a partial specialization then the "actual" template arguments are in 
                                     // the p->variant.class_struct_union.extra_info->template_arg_list and the associated names of the template arguments 
                                     // are in the p->variant.class_struct_union.extra_info->partial_spec_template_arg_list list.  This is because the names
                                     // need not match that of where they are defined in an outer template.
                                        ROSE_ASSERT(templateSpecializationArgs_actual != NULL);
                                     // templateSpecializationArgumentList = parse_template_arguments(templateSpecializationArgs,scopeOfDeclaration);
#if 0
                                        printf ("In getDeclarationScope(): calling parse_template_arguments() \n");
#endif
#if 0
#if 0
                                     // DQ (2/11/2019): Adding support to track the min_template_arguments.
                                        a_class_type_supplement_ptr classTypeSupplement = p->variant.class_struct_union.extra_info;
                                        ROSE_ASSERT((p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union) && classTypeSupplement != NULL);
                                        printf ("In getDeclarationScope(): classTypeSupplement->min_template_arguments = %d \n",classTypeSupplement->min_template_arguments);
#endif
#endif
                                        templateSpecializationArgumentList = parse_template_arguments(templateSpecializationArgs_actual,scopeOfDeclaration);
                                      }

                                   ROSE_ASSERT(buildTemplateInstantiation == false);
                                   SgTemplateClassDeclaration* firstNondefiningTemplateClassDeclaration = isSgTemplateClassDeclaration(firstNondefiningClassDeclaration);
                                   ROSE_ASSERT(firstNondefiningTemplateClassDeclaration != NULL);
                                // definingClassDeclaration = buildTemplateClassDeclaration_nfi(name,kind,scope,firstNondefiningTemplateClassDeclaration);
                                   definingClassDeclaration = buildTemplateClassDeclaration_nfi(name,kind,scope,firstNondefiningTemplateClassDeclaration,&templateParameterList,&templateSpecializationArgumentList);

                                   for (SgTemplateParameterPtrList::iterator it = templateParameterList.begin(); it != templateParameterList.end(); it++) {
                                     if ((*it)->get_parent() != NULL) {
                                       SgDeclarationStatement * declstmt_parent = isSgDeclarationStatement((*it)->get_parent());
                                       ROSE_ASSERT(declstmt_parent != NULL);
                                       ROSE_ASSERT(declstmt_parent->get_firstNondefiningDeclaration() == definingClassDeclaration->get_firstNondefiningDeclaration());
                                     } else {
                                       (*it)->set_parent(definingClassDeclaration);
                                     }
                                   }
#if 0
                                // DQ (9/9/2012): It appears that the template parameter list is not set properly (frequently zero, at least).
                                   printf ("In getDeclarationScope(): firstNondefiningTemplateClassDeclaration->get_templateParameters().size() = %" PRIuPTR " \n",firstNondefiningTemplateClassDeclaration->get_templateParameters().size());
#endif
                                // DQ (9/9/2012): It appears that the template parameter list is not set properly (frequently zero, at least).
                                // ROSE_ASSERT(firstNondefiningTemplateClassDeclaration->get_templateParameters().size() > 0);
                                 }
                                else
                                 {
                                // DQ (6/6/2012): We need to provide the template arguments.
#if DEBUG_GET_DECLARATION_SCOPE
                                   printf ("In getDeclarationScope(): Build this as a NON template declaration \n");
#endif
#if 0
                                // DQ (1/2/2012): The templateArgumentsList is computed once and used twice (1st to fixup the name, 2nd to append template arguments to the template instantiation).
                                   ROSE_ASSERT(edg_type->variant.class_struct_union.extra_info != NULL);
                                   a_template_arg_ptr templateArgs = edg_type->variant.class_struct_union.extra_info->template_arg_list;
                                   SgTemplateArgumentPtrList templateArgumentsList;
                                   if (templateArgs != NULL)
                                      {
                                     // DQ (6/6/2012): This fails for test2005_35.C
                                        ROSE_ASSERT(scopeOfDeclaration != NULL);
#error "DEAD CODE!"
                                     // DQ (6/6/2012): This fails for test2005_35.C
                                        templateArgumentsList = parse_template_arguments(templateArgs,scopeOfDeclaration);

                                     // DQ (6/6/2012): Make sure that this is valid (might not always be true).
                                        ROSE_ASSERT(templateArgumentsList.size() > 0);
                                      }
#else
                                // DQ (6/7/2012): We need to make sure that we are processing a template instantiation.
                                // DQ (6/6/2012): We need to provide the template arguments.
                                // ROSE_ASSERT(isSgTemplateInstantiationDecl(firstNondefiningClassDeclaration) != NULL);
                                // SgTemplateArgumentPtrList templateArgumentsList = isSgTemplateInstantiationDecl(firstNondefiningClassDeclaration)->get_templateArguments();
                                   SgTemplateArgumentPtrList templateArgumentsList;
                                // if (isSgTemplateInstantiationDecl(firstNondefiningClassDeclaration) != NULL)
                                   if (buildTemplateInstantiation == true)
                                      {
                                     // DQ (8/25/2012): We need to reset the name to be without template arguments so that we don't add template arguments redundantly.
                                        name = isSgTemplateInstantiationDecl(firstNondefiningClassDeclaration)->get_templateName();
#if DEBUG_GET_DECLARATION_SCOPE
                                        printf ("In getDeclarationScope(): Reset name = %s \n",name.str());
#endif
                                        ROSE_ASSERT(isSgTemplateInstantiationDecl(firstNondefiningClassDeclaration) != NULL);
                                        templateArgumentsList = isSgTemplateInstantiationDecl(firstNondefiningClassDeclaration)->get_templateArguments();

#if DEBUG_GET_DECLARATION_SCOPE
                                        printf ("In getDeclarationScope(): firstNondefiningClassDeclaration                      = %p = %s \n",
                                             firstNondefiningClassDeclaration,firstNondefiningClassDeclaration->class_name().c_str());
                                        printf ("In getDeclarationScope(): firstNondefiningClassDeclaration->unparseNameToString = %s \n",firstNondefiningClassDeclaration->unparseNameToString().c_str());
#endif
                                     // DQ (7/25/2012): The parent should point to the SgTemplateInstantiationDecl, not the SgGlobal.
                                        for (size_t i = 0; i < templateArgumentsList.size(); i++)
                                           {
                                          // DQ (7/25/2012): I don't know if we can assert these conditions.
                                             ROSE_ASSERT(templateArgumentsList[i]->get_parent() != NULL);
                                             if (templateArgumentsList[i]->get_parent() != firstNondefiningClassDeclaration)
                                                {
#ifdef DEBUG_CONVERSION
                                                  printf ("firstNondefiningClassDeclaration                                          = %p = %s \n",firstNondefiningClassDeclaration,firstNondefiningClassDeclaration->class_name().c_str());
                                                  printf ("templateArgumentsList[i]->get_parent()                                    = %p = %s \n",templateArgumentsList[i]->get_parent(),templateArgumentsList[i]->get_parent()->class_name().c_str());
#endif
                                                  SgTemplateInstantiationDecl* templateInstantiationDecl = isSgTemplateInstantiationDecl(templateArgumentsList[i]->get_parent());
                                                  if (templateInstantiationDecl != NULL)
                                                     {
                                                       printf ("templateArgumentsList[i]->get_parent()->get_firstNondefiningDeclaration() = %p \n",templateInstantiationDecl->get_firstNondefiningDeclaration());
                                                       printf ("templateArgumentsList[i]->get_parent()->get_definingDeclaration()         = %p \n",templateInstantiationDecl->get_definingDeclaration());
                                                     }
                                                }
                                          // DQ (6/1/2016): This can also be a SgTemplateClassDeclaration (see test2016_34.C).
                                          // ROSE_ASSERT(templateArgumentsList[i]->get_parent() == firstNondefiningClassDeclaration);

                                          // templateArgumentsList[i]->set_parent(firstNondefiningClassDeclaration);
                                           }
                                      }
#endif

                                // DQ (6/6/2012): We need to provide the template arguments.
                                // definingClassDeclaration = buildClassDeclaration_nfi(name,kind,scope,firstNondefiningClassDeclaration,buildTemplateInstantiation);
                                // definingClassDeclaration = buildClassDeclaration_nfi(name,kind,scope,firstNondefiningClassDeclaration,buildTemplateInstantiation,&templateArgumentsList);
                                   if (firstNondefiningClassDeclaration->get_definingDeclaration() == NULL)
                                      {
#if 0
                                        printf ("In getDeclarationScope(): There is not associated class definition from a class definition, so build one: name = %s \n",name.str());
                                        printf ("   --- name                                         = %s \n",name.str());
                                        printf ("   --- firstNondefiningClassDeclaration->get_name() = %s \n",firstNondefiningClassDeclaration->get_name().str());
                                        printf ("   --- buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
                                        printf ("   --- Output the template arguments (size = %zu): \n",templateArgumentsList.size());
                                        for (size_t i = 0; i < templateArgumentsList.size(); i++)
                                           {
                                             printf ("   --- --- templateArgumentsList[%zu] = %p \n",i,templateArgumentsList[i]);
                                             printf ("   --- --- name = %s \n",SageBuilder::unparseTemplateArgumentToString(templateArgumentsList[i]).str());
                                           }

                                        SgTemplateInstantiationDecl* templateInstantiationDecl = isSgTemplateInstantiationDecl(firstNondefiningClassDeclaration);
                                        if (templateInstantiationDecl != NULL)
                                           {
                                             for (size_t i = 0; i < templateInstantiationDecl->get_templateArguments().size(); i++)
                                                {
                                                  printf ("   --- --- templateInstantiationDecl->get_templateArguments()[%zu] = %p \n",i,templateInstantiationDecl->get_templateArguments()[i]);
                                                }
                                           }
                                        SgName nameWithoutTemplateArguments = name;
                                        SgName nameWithTemplateArguments    = nameWithoutTemplateArguments;
                                        if (buildTemplateInstantiation == true)
                                           {
                                             nameWithTemplateArguments = appendTemplateArgumentsToName(nameWithoutTemplateArguments,templateArgumentsList);
                                           }
                                        printf ("   --- --- After appendTemplateArgumentsToName(): nameWithTemplateArguments = %s \n",nameWithTemplateArguments.str());
                                        if (firstNondefiningClassDeclaration->get_name() != nameWithTemplateArguments)
                                           {
                                             printf ("In getDeclarationScope(): Error: names don't match: \n");
                                             printf ("   --- firstNondefiningClassDeclaration           = %p = %s \n",firstNondefiningClassDeclaration,firstNondefiningClassDeclaration->class_name().c_str());
                                             printf ("   --- firstNondefiningClassDeclaration->get_name()            = %s \n",firstNondefiningClassDeclaration->get_name().str());
                                             printf ("   --- nameWithTemplateArguments                               = %s \n",nameWithTemplateArguments.str());
                                             printf ("   --- firstNondefiningClassDeclaration->unparseNameToString() = %s \n",firstNondefiningClassDeclaration->unparseNameToString().c_str());
                                           }
                                        ROSE_ASSERT (firstNondefiningClassDeclaration->get_name() == nameWithTemplateArguments);
#endif
#if 0
                                        printf ("In getDeclarationScope(): Calling summarize_type() \n");
                                        summarize_type(edg_type);
#endif
                                        definingClassDeclaration = buildClassDeclaration_nfi(name,kind,scope,firstNondefiningClassDeclaration,buildTemplateInstantiation,&templateArgumentsList);
                                        ROSE_ASSERT(definingClassDeclaration != NULL);
#if DEBUG_GET_DECLARATION_SCOPE
                                        printf ("In getDeclarationScope(): definingClassDeclaration     = %p = %s \n",definingClassDeclaration,definingClassDeclaration->class_name().c_str());
                                        printf ("   --- name                                            = %s \n",name.str());
                                        printf ("   --- definingClassDeclaration->get_name()            = %s \n",definingClassDeclaration->get_name().str());
                                        printf ("   --- definingClassDeclaration->unparseNameToString() = %s \n",definingClassDeclaration->unparseNameToString().c_str());
#endif
                                        if (is_unnamed(edg_type) == true)
                                           {
#if DEBUG_GET_DECLARATION_SCOPE
                                             printf ("Found definingClassDeclaration = %p that should be marked as un-named name = %s \n",definingClassDeclaration,name.str());
#endif
                                             definingClassDeclaration->set_isUnNamed(true);

                                          // DQ (2/5/2015): Set the non-defining declaration
                                             if (definingClassDeclaration->get_firstNondefiningDeclaration() != NULL)
                                                {
                                                  isSgClassDeclaration(definingClassDeclaration->get_firstNondefiningDeclaration())->set_isUnNamed(true);
                                                }
                                           }
#if DEBUG_GET_DECLARATION_SCOPE
                                        printf ("In getDeclarationScope(): definingClassDeclaration = %p = %s \n",definingClassDeclaration,definingClassDeclaration->class_name().c_str());
#endif
                                      }
                                     else
                                      {
#ifdef DEBUG_CONVERSION
                                        printf ("In getDeclarationScope(): reuse the existing class definition, from the previously built class definition = %p \n",firstNondefiningClassDeclaration->get_definingDeclaration());
#endif
                                        definingClassDeclaration = isSgClassDeclaration(firstNondefiningClassDeclaration->get_definingDeclaration());
                                        ROSE_ASSERT(definingClassDeclaration != NULL);
                                      }
                                 }
                              ROSE_ASSERT(definingClassDeclaration != NULL);
#if DEBUG_GET_DECLARATION_SCOPE
                              printf ("******************************************************************************************************************** \n");
                              printf ("DONE: BUILDING PREVIOUSLY UNSEEN DEFINING DECLARATION FOR PARENT OF DECLARATION (definingClassDeclaration = %p = %s) \n",definingClassDeclaration,definingClassDeclaration->class_name().c_str());
                              printf ("******************************************************************************************************************** \n");
#endif
                           // Check that these are consistant (same).
                              if (definingClassDeclaration->variant() != firstNondefiningClassDeclaration->variant())
                                 {
                                   printf ("definingClassDeclaration         = %p = %s \n",definingClassDeclaration,definingClassDeclaration->class_name().c_str());
                                   printf ("firstNondefiningClassDeclaration = %p = %s \n",firstNondefiningClassDeclaration,firstNondefiningClassDeclaration->class_name().c_str());
                                 }
                              ROSE_ASSERT(definingClassDeclaration->variant() == firstNondefiningClassDeclaration->variant());
                            }

                      // If this does not work then we could look for the parent scope of the parent scope?
#if 0
                         SgScopeStatement* declarationScope = getGlobalScopeFromScopeStack();
#else
#ifdef DEBUG_CONVERSION
                         printf ("Setting the parent of the new defining template instantiation declaration to be global scope (not clear what else would work)\n");
#endif
                         ROSE_ASSERT(scp.parent_scope != NULL);
                         a_scope_ptr parentScopeOfParentScope = scp.parent_scope->parent;

                      // DQ (10/17/2012): Assert this only where it is used in the false branch below.
                      // ROSE_ASSERT(parentScopeOfParentScope != NULL);

                         if (scope_cache.find(scp.parent_scope) == scope_cache.end())
                            {
                           // This function should be refactored to make it possible to be called recursively.
                           // DQ (4/23/2012): If this scope is not known then we have to recurseively call a function to generate the scope for each case (until we reach global scope).
#ifdef DEBUG_CONVERSION
                              printf ("Error scope parentScopeOfParentScope = %p not previously seen (set to global scope) \n",parentScopeOfParentScope);
#endif

                           // ROSE_ASSERT(false);
                              declarationScope = getGlobalScopeFromScopeStack();
                            }
                           else
                            {
                              ROSE_ASSERT(parentScopeOfParentScope != NULL);
                              declarationScope = scope_cache[parentScopeOfParentScope];
                            }
                      // ROSE_ASSERT(scope_cache.find(scp.parent_scope) != scope_cache.end());
#endif
                       }
                      else
                       {
                      // DQ (11/21/2012): This is the work to fix test2012_158.c.
                      // If this is a class declared first in a function's parameter list then it's scope is global scope for now.
                      // This is likely fine for C, but not quite good enough for C++.  Note that this code used to be an error
                      // but the consequence was that we could have infinite recursion.  So this is a compromize at present.
                      // As an alternative to global scope (which si fine for C, C++ might require this to be in the current scope.
#if DEBUG_GET_DECLARATION_SCOPE
                         printf ("WARNING: edg_type = %p has not been previously processed (using global scope) \n",edg_type);
#endif
                      // declarationScope = getGlobalScopeFromScopeStack();
                         declarationScope = getGlobalScopeFromScopeStack();
#if 0
                         printf ("In getDeclarationScope(): using getGlobalScopeFromScopeStack(): declarationScope = %p = %s \n",declarationScope,declarationScope->class_name().c_str());
#endif
                      // ROSE_ASSERT(false);
                       }
#if DEBUG_GET_DECLARATION_SCOPE
                    printf ("In getDeclarationScope(): definingClassDeclaration = %p \n",definingClassDeclaration);
#endif
                 // ROSE_ASSERT(definingDeclaration != NULL);
                    if (definingClassDeclaration != NULL)
                       {
                         ROSE_ASSERT(declarationScope != NULL);
                         definingClassDeclaration->set_parent(declarationScope);

                         ROSE_ASSERT(definingClassDeclaration->get_parent() != NULL);
#if DEBUG_GET_DECLARATION_SCOPE
                         printf ("In getDeclarationScope(): (definingClassDeclaration != NULL): scope_cache.find(scp.parent_scope = %p) == scope_cache.end() = %s \n",scp.parent_scope,scope_cache.find(scp.parent_scope) == scope_cache.end() ? "true" : "false");
#endif
                         scope_cache[scp.parent_scope] = definingClassDeclaration->get_definition();
                         ROSE_ASSERT(scope_cache[scp.parent_scope] != NULL);
#if 0
                      // DQ (7/7/2019): Debugging C language support of function prototype scopes (see test2019_11.c).
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                       }
#if 0
                    printf ("In getDeclarationScope(): scope_cache.find(scp.parent_scope = %p) == scope_cache.end() = %s \n",scp.parent_scope,scope_cache.find(scp.parent_scope) == scope_cache.end() ? "true" : "false");
#endif
                 // DQ (11/21/2012): If this is not yet in the scope cache then place it there.  Note that we are 
                 // frequently assigning a mapping of an EDG pointer to a sck_func_prototype to the ROSE SgGlobal 
                 // scope.   We might want ot have a specific ROSE IR node for the scope of variables and types in
                 // the parameter list of a function prototype.  This has been discussed but not implemented yet.
                    if (scope_cache.find(scp.parent_scope) == scope_cache.end())
                       {
                         scope_cache[scp.parent_scope] = declarationScope;
#if 0
                      // DQ (7/7/2019): Debugging C language support of function prototype scopes (see test2019_11.c).
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                       }

                    scopeOfDeclaration = scope_cache[scp.parent_scope];
                    ROSE_ASSERT(scopeOfDeclaration != NULL);
                  }
                 else
                  {
                 // This can't be the global scope since that is the first scope processed and so it is already present in the scope_cache.
                 // I think this is typically a sck_block that are not added to the scope_cache.  But it can (rarely) be a namespace that 
                 // we have not yet processed.  If so then we could build a namespace definition for use by just this edg_scope.

                    if (scp.parent_scope->kind == sck_block)
                       {
                      // This is the typical case and we need not do anything (since we return the current scope on the stack).
                       }
                      else
                       {
                      // In this case the scope may be asociated with a namespace that has not yet been processed.  EDG can insert
                      // code ahead of the namespace for template instantiations that may reference namespace specific classes.
                      // This happens in the procesing of the ROSE header files (called by the haskell_port project).
#if DEBUG_GET_DECLARATION_SCOPE
                         printf ("In getDeclarationScope(): This is a reference to a scope (scp.parent_scope = %p) that has not yet been processed (and not a class which would have caused it to be just constructed) \n",scp.parent_scope);
#endif
#if 0
                         printf ("In getDeclarationScope(): Calling summarize_scope() \n");
                         summarize_scope(scp.parent_scope);
#endif
                         if (scp.parent_scope->kind == sck_namespace)
                            {
                              a_namespace_ptr edg_namespace = scp.parent_scope->variant.assoc_namespace;
                              ROSE_ASSERT(edg_namespace != NULL);

                           // DQ (7/9/2014): This is a problem when edg_namespace->source_corresp.name == NULL (see test2014_85.C).
                           // string namespaceName = edg_namespace->source_corresp.name;
                              string namespaceName = edg_namespace->source_corresp.name != NULL ? edg_namespace->source_corresp.name : "";
#ifdef DEBUG_CONVERSION
                              printf ("In getDeclarationScope(): Processing a previously unseen namespace: edg_namespace = %p namespaceName = %s \n",edg_namespace,namespaceName.c_str());
#endif
#if 0
                              SgScopeStatement* current_scope = topScopeStack();
                              printf ("In getDeclarationScope(): (scp.parent_scope->kind == sck_namespace): But not a previously seen scope: current_scope = %p = %s \n",current_scope,current_scope->class_name().c_str());
#endif
#if 0
                              printf ("************************* Calling construct_namespace() ******************************************************* \n");
#endif
                              SgNamespaceDeclarationStatement* namespaceDeclaration = construct_namespace(edg_namespace);
#if 0
                              printf ("************************* DONE: Calling construct_namespace() ******************************************************* \n");
#endif
#if 0
                              printf ("In getDeclarationScope(): (scp.parent_scope->kind == sck_namespace): namespaceDeclaration->get_scope() = %p = %s \n",namespaceDeclaration->get_scope(),namespaceDeclaration->get_scope()->class_name().c_str());
#endif
                              ROSE_ASSERT(namespaceDeclaration != NULL);

#ifdef DEBUG_CONVERSION
// #if 0
                              printf ("In getDeclarationScope(): Unattached SgNamespaceDeclaration generated from reference to non-previously seen namespace: namespaceDeclaration = %p = %s \n",namespaceDeclaration,namespaceDeclaration->get_name().str());
#endif
                              scopeOfDeclaration = namespaceDeclaration->get_definition();
                              ROSE_ASSERT(scopeOfDeclaration != NULL);
                            }
                           else
                            {
#if 0
                              printf ("In getDeclarationScope(): Calling summarize_scope() what is this? \n");
                              summarize_scope(scp.parent_scope);
#endif
                              if (scp.parent_scope->kind == sck_template_declaration)
                                 {
                                // DQ (9/1/2012): This is the case of template template parameters as in "template<template<class T, class U> class V> class B {};" (see test2012_199.C).
#if 0
                                   printf ("In getDeclarationScope(): This is a scope kind = sck_template_declaration: scp.parent_scope = %p \n",scp.parent_scope);
#endif
                                // This case causes scopeOfDeclaration == NULL (which sets the scopeOfDeclaration to the value of topScopeStack()).
                                 }
                                else
                                 {
#if 0
                                   printf ("In getDeclarationScope(): Calling summarize_scope() topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
                                   summarize_scope(scp.parent_scope);
#endif
                                   if (scp.parent_scope->kind == sck_condition)
                                      {
                                     // This case happens for test2014_140.C (where an function parameter type references a name 
                                     // qualified variable in a namespace that has not yet been processed.  This is possible because 
                                     // the function type is processed for both its declared and non-declared types (and the 
                                     // non-declared type has the name qualified variable (e.g. for an array size in a type)).

                                        SgScopeStatement* scope = topScopeStack();
                                        if (isSgWhileStmt(scope) != NULL || isSgSwitchStatement(scope) != NULL || isSgForStatement(scope) != NULL || isSgDoWhileStmt(scope) != NULL || isSgIfStmt(scope) != NULL)
                                           {
                                          // This is a part of a declaration of a variable in a predicate in a few types of C++ constructs (only allowed in C++).
                                             scopeOfDeclaration = topScopeStack();
                                           }
                                          else
                                           if (isSgBasicBlock(scope) != NULL)
                                           {
                                          // The declaration of a C++17 init-statement ("if (int x = f(); x)") also lives in the EDG condition
                                          // scope; it is converted into the enclosing generated block built by parse_condition_init_statement().
                                             scopeOfDeclaration = scope;
                                           }
                                          else
                                           {
                                             printf ("In getDeclarationScope(): (scp.parent_scope->kind == sck_condition): But not a recognized scope for a conditional test, so exit as a test! scope = %p = %s \n",scope,scope->class_name().c_str());

                                             scopeOfDeclaration = scope;

#if 0
                                             printf ("Exiting since it is unclear what scope to use! \n");
                                             ROSE_ASSERT(false);
#else
                                          // DQ (2/11/2013): Try this (required for processing "rose.h".
                                             printf ("Using the current scope as a test! \n");
#endif
                                           }
                                      }
                                     else
                                      {
#if 0
                                        printf ("In getDeclarationScope(): Calling summarize_scope() topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
                                        summarize_scope(scp.parent_scope);
#endif
#if 0
                                     // DQ (2/15/2016): I don't think we want the added complexity of this function 
                                     // (this is handled directly where this function is called and is a better implementation).
                                     // DQ (2/13/2016): Adding support for VLA types in function parameter lists.
                                        if (scp.parent_scope->kind == sck_function)
                                           {
                                          // This happens for test2016_02.c (the use of VLA types in function parameter lists.

                                             SgScopeStatement* scope = topScopeStack();

                                             returnNull = true;
                                             scopeOfDeclaration = NULL;

                                             printf ("For the case of scp.parent_scope->kind == sck_function: returning scope = %p = %s \n",scope,scope->class_name().c_str());

                                             printf ("WARNING: I think this is the wrong scope it would be better to not ask for the scope from this function \n");
                                           }
#endif
#if 0
                                        printf ("In getDeclarationScope(): Unclear what this is so exit as a test! \n");
                                        ROSE_ASSERT(false);
#endif
                                      }
                                 }
                            }
                       }
                  }
#if DEBUG_GET_DECLARATION_SCOPE
               printf ("Make this an error to not find the edg scope of the current scp name = %s \n",scp.name);
#endif
#if 0
               ROSE_ASSERT(false);
#endif
             }
        }
       else
        {
       // DQ (10/31/2012): When the scp.parent_scope == NULL we still need to identify an appropriate scope for the declaration.
#if 0
          printf ("In getDeclarationScope(): Calling summarize_source_correspondence() what is this? \n");
          summarize_source_correspondence(scp);
#endif
          if (scp.source_sequence_entry == NULL)
             {
            // This fails for the Cxx11 test2012_02.C and appers to be speciifc to the lambda function support.
            // Perhaps it is just that the generated function declaration is not in the source sequence list.
#if DEBUG_GET_DECLARATION_SCOPE
               printf ("WARNING: In getDeclarationScope(): scp.source_sequence_entry == NULL: this might be lambda function specific \n");
#endif
             }
       // ROSE_ASSERT(scp.source_sequence_entry != NULL);
#if DEBUG_GET_DECLARATION_SCOPE
          printf ("In getDeclarationScope(): scp.parent_scope == NULL: entry = %p kind = %s \n",scp.source_sequence_entry,summarize_entry_kind_in_string_kind(scp.source_sequence_entry->entity).c_str());
#endif

#if 0
       // a_scope_ptr edg_scope = edgScopeOfDeclarationFromEntity(scp.source_sequence_entry->entity);
          if (edg_scope == NULL)
             {
               printf ("After getScopeOfDeclaration(): edg_scope == NULL \n");
             }
#else
       // DQ (9/10/2013): This case of scp.source_sequence_entry == NULL has to be handled.
       // scopeOfDeclaration = edgScopeOfDeclarationFromEntity(scp.source_sequence_entry->entity);
          if (scp.source_sequence_entry != NULL)
             {
#if DEBUG_GET_DECLARATION_SCOPE
               printf ("In getDeclarationScope(): calling edgScopeOfDeclarationFromEntity() to set scope \n");
#endif
               scopeOfDeclaration = edgScopeOfDeclarationFromEntity(scp.source_sequence_entry->entity);
#if 0
               printf ("In getDeclarationScope(): DONE: calling edgScopeOfDeclarationFromEntity() to set scope: scopeOfDeclaration = %p \n",scopeOfDeclaration);
#endif
#if 0
            // DQ (1/30/2018): I want to see if we get there (test2004_19.C get here).
               if (scopeOfDeclaration != NULL)
                  {
                    printf ("Setting the scopeOfDeclaration using edgScopeOfDeclarationFromEntity(): scopeOfDeclaration = %p \n",scopeOfDeclaration);
                    ROSE_ASSERT(false);
                  }
#endif
             }
#endif

       // DQ (1/29/2018): If this is in a function scope then we should have: parent_via_local_scope_ref == true
#if DEBUG_GET_DECLARATION_SCOPE
          printf ("In getDeclarationScope(): scp.parent_via_local_scope_ref = %s \n",scp.parent_via_local_scope_ref ? "true" : "false");
#endif
       // if (scp.parent_via_local_scope_ref == true)
          if (scopeOfDeclaration == NULL && scp.parent_via_local_scope_ref == true)
             {
#if 0
               printf ("In getDeclarationScope(): scp.enclosing_routine = %p \n",scp.enclosing_routine);
#endif
               ROSE_ASSERT(scp.enclosing_routine != NULL);
#if 0
               printf ("In getDeclarationScope(): calling summarize_routine(): scp.enclosing_routine = %p\n",scp.enclosing_routine);
               summarize_routine(scp.enclosing_routine);
#endif

            // Code to permit this file to compile, as long as it will not run.
            // a_memory_region_number scope_region_number = 0L;
               a_scope_ptr scope = NULL;
#if DEBUG_GET_DECLARATION_SCOPE
               printf ("In getDeclarationScope(): scp.enclosing_routine->function_def_number = %d \n",scp.enclosing_routine->function_def_number);
#endif
               if (scp.enclosing_routine->function_def_number != NULL_function_def_number)
                  {
                 // Note that p->assoc_scope is an EDG specific scope number (not a pointer to a scope).
                 // scope = il_header.region_scope_entry[p->function_def_number];
                    scope = il_header.function_def_table[scp.enclosing_routine->function_def_number].scope;
                  }
               if (scope == NULL)
                  {
                 // DQ (4/23/2017): Converted to used message streams (appears only for case of where Microsoft extensions are used).
                    printf ("In getDeclarationScope(): p->function_def_number = %d \n",(int)scp.enclosing_routine->function_def_number);
                  }
               ROSE_ASSERT(scope != NULL);
#if DEBUG_GET_DECLARATION_SCOPE
               printf ("In getDeclarationScope(): scope = %p \n",scope);
#endif
#if 0
               printf ("In getDeclarationScope(): calling summarize_scope(): scope = %p\n",scope);
               summarize_scope(scope);
#endif
            // DQ (1/30/2018): This is a simplistic way to get a pointer to the declaration that we are looking for 
            // (it might be a type or a constant, or a number of different things).
            // a_tagged_pointer target_declaration = (a_tagged_pointer)&scp;
               char* target_declaration = (char*)&scp;
               a_local_scope_ref_ptr local_scope_ptr = scope->scope_refs;
               while (local_scope_ptr != NULL)
                  {
                    if (local_scope_ptr->referrer.ptr == target_declaration)
                       {
                         ROSE_ASSERT(local_scope_ptr->scope != NULL);

                         if (scope_cache.find(local_scope_ptr->scope) != scope_cache.end())
                            {
                              scopeOfDeclaration = scope_cache[local_scope_ptr->scope];
#if DEBUG_GET_DECLARATION_SCOPE
                              printf ("In getDeclarationScope(): local_scope_ptr: found valid scope from scope_cache: scopeOfDeclaration = %p \n",scopeOfDeclaration);
#endif
                            }
                           else
                            {
                           // Search for the scope using the statement_scope_cache.
                              a_statement_ptr assoc_block = local_scope_ptr->scope->assoc_block;

                           // DQ (1/31/2018): We can't have this new assertion because it fails for test2006_54.C.
                           // ROSE_ASSERT(assoc_block != NULL);
                              if (assoc_block != NULL && statement_scope_cache.find(assoc_block) != statement_scope_cache.end())
                                 {
                                   scopeOfDeclaration = statement_scope_cache[assoc_block];
#if DEBUG_GET_DECLARATION_SCOPE
                                   printf ("In getDeclarationScope(): local_scope_ptr: found valid scope from statement_scope_cache: scopeOfDeclaration = %p \n",scopeOfDeclaration);
#endif
                                 }
                            }
                       }

                    local_scope_ptr = local_scope_ptr->next;
                  }

            // DQ (1/31/2018): We can't have this new assertion because it fails for test2006_54.C.
            // ROSE_ASSERT(scopeOfDeclaration != NULL);
             }
        }

  // DQ (2/15/2016): Reverting to the previous semantics (VLA handling is done and controls if this function is called in convert_variable_use()).
  // DQ (2/13/2016): Allow the case of a sck_function to cause a NULL value to be returned 
  // (and not reset here).  This is important for variable references in VLA types (array dimensions).
  // if (scopeOfDeclaration == NULL)
     ROSE_ASSERT(returnNull == false);
  // if (scopeOfDeclaration == NULL && returnNull == false)
     if (scopeOfDeclaration == NULL)
        {
       // DQ (9/27/2012): I would like to eliminate this way of setting the scope.
          scopeOfDeclaration = topScopeStack();

#ifdef DEBUG_CONVERSION
          printf ("WARNING: scopeOfDeclaration not set using scope_cache (edg scope = %p not yet processed) scopeOfDeclaration = %p = %s \n",scp.parent_scope,scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif
#if DEBUG_GET_DECLARATION_SCOPE
          printf ("Warning: We have not yet eliminated the computation of the scope using topScopeStack() \n");
       // ROSE_ASSERT(false);
#endif
        }

  // ROSE_ASSERT(scopeOfDeclaration != NULL);
     ROSE_ASSERT(scopeOfDeclaration != NULL || returnNull == true);

#if DEBUG_GET_DECLARATION_SCOPE && 0
     if (returnNull == false)
        {
          printf ("Leaving getDeclarationScope(): scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
        }
       else
        {
          printf ("Leaving getDeclarationScope(): scopeOfDeclaration == NULL \n");
        }
#endif

  // DQ (4/8/2018): Look for prototype scopes so that we can handle them more accurately.
  // ROSE_ASSERT(scp.parent_scope != NULL);
     if (scp.parent_scope != NULL && scp.parent_scope->kind == sck_func_prototype)
        {
#if DEBUG_GET_DECLARATION_SCOPE
          printf ("$$$$$$$$$$$$$$$$$$$$$$$ This is a function prototype scope! \n");
#endif
       // ROSE_ASSERT(scp.is_local_to_function == true);
          if (scp.is_local_to_function == true)
             {
               ROSE_ASSERT(internal_intermediate_scope != NULL);
               ROSE_ASSERT(isSgDeclarationScope(internal_intermediate_scope) != NULL);
#if DEBUG_GET_DECLARATION_SCOPE || 1
               printf ("Leaving getDeclarationScope(): setting scopeOfDeclaration to internal_intermediate_scope = %p = %s \n",internal_intermediate_scope,internal_intermediate_scope->class_name().c_str());
#endif
               scopeOfDeclaration = internal_intermediate_scope;
            }
        }

#if DEBUG_GET_DECLARATION_SCOPE
     printf ("Leaving getDeclarationScope(): scope_cache.find(scp.parent_scope) != scope_cache.end() = %s \n",(scope_cache.find(scp.parent_scope) != scope_cache.end()) ? "true" : "false");
#endif
#if DEBUG_GET_DECLARATION_SCOPE
     printf ("Leaving getDeclarationScope(): scopeOfDeclaration = %p \n",scopeOfDeclaration);
     if (scopeOfDeclaration != NULL)
       {
         printf ("Leaving getDeclarationScope(): scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
       }
     printf ("--------------------- \n");
#endif

     return scopeOfDeclaration;
   }

void set_special_function_modifier(SgSpecialFunctionModifier & modifier, a_special_function_kind kind)
   {
  // Note that this function does not just return an enum value because ROSE permits that 
  // multiple values could be set.  This feature is not required for C and C++, but I think
  // the point what that it would permit the greatest flexability for other languages.
  // Or perhaps I just though that certain constructors should also be marked as conversion 
  // functions (however this is not done in ROSE).

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("special function modifier %s\n", db_special_function_kinds[kind]);
#endif

     switch (kind)
        {
          case sfk_none:               modifier.setDefault();     break;
          case sfk_constructor:        modifier.setConstructor(); break;
          case sfk_destructor:         modifier.setDestructor();  break;
          case sfk_conversion:         modifier.setConversion();  break;
          case sfk_udl_operator:       modifier.setUldOperator();      break;
          case sfk_operator:           modifier.setOperator();    break;
          case sfk_lambda_entry_point: modifier.setLambdaEntryPoint(); break;
          case sfk_deduction_guide:    modifier.setDefault(); break;

#if MICROSOFT_EXTENSIONS_ALLOWED
          case  sfk_static_constructor: /* A C++/CLI static constructor. */
          case sfk_finalizer:           /* A C++/CLI finalizer. */
          case sfk_idisposable_dispose:	/* A compiler-generated implementation of the IDisposable::Dispose() member. */
          case sfk_dispose_bool:	/* A compiler-generated Dispose(bool) member. */
          case sfk_object_finalize:	/* A compiler-generated overrider for the Object::Finalize() member. */
          case sfk_property_get:	/* A "get" accessor function of a C++/CLI property. */

       // DQ (7/27/2020): This is an error (duplicate entry).
       // case sfk_first_accessor: // = sfk_property_get

          case sfk_property_set:	/* A "set" accessor function of a C++/CLI property. */
          case sfk_event_add:           /* An "add" accessor function of a C++/CLI event. */
          case sfk_event_remove:	/* A "remove" accessor function of a C++/CLI event. */
          case sfk_event_raise:         /* A "raise" accessor function of a C++/CLI event. */

       // DQ (7/27/2020): This is an error (duplicate entry).
       // case sfk_last_accessor:  // = sfk_event_raise:
#if 1
               printf ("In set_special_function_modifier(): Microsoft specific extensions not yet supported (using setDefault()) \n");
#endif
               modifier.setDefault();
               break;
#endif
          case sfk_last:
          default:
              ERROR_MESSAGE(stderr, "Bad special function modifier %s\n", db_special_function_kinds[kind]); ROSE_ASSERT(false);
        }
   }

void set_function_modifier(SgFunctionModifier & modifier, a_routine_ptr p)
   {
  // Note that this function does not just return an enum value because ROSE permits that 
  // multiple values could be set.  This feature is not required for C and C++, but I think
  // the point what that it would permit the greatest flexability for other languages.
  // Or perhaps I just though that certain constructors should also be marked as conversion 
  // functions (however this is not done in ROSE).

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("function modifier \n");
#endif

#if 0
     printf("In set_function_modifier(): function modifier: modifier (pointer) = %p routine p = %p \n",&modifier,p);
#endif

     if (p->is_virtual == true)
        {
          modifier.setVirtual();
          if (p->pure_virtual == true)
             {
               modifier.setPureVirtual();
             }
        }

     if (p->is_inline == true) 
        {
#if 0
          printf("In set_function_modifier(): function is marked (internally) as inlined (might not be explicitly marked as inlined) \n");
#endif
          modifier.setInline();
        }

  // DQ (1/7/2020): Check for use of the "explicit" keyword.
     if (p->is_explicit_conversion_function == true)
        {
          modifier.setExplicit();
        }

  // DQ (1/7/2020): Check for use of the "explicit" keyword.
     if (p->is_explicit_constructor == true)
        {
          modifier.setExplicit();
        }

     switch (p->special_kind)
        {
          case sfk_none:        // Not a special function.
             {
            // ROSE_ASSERT(!"sfk_none not implemented");
               break;
             }

          case sfk_constructor:	// A constructor. 
             {
            // Note that the SgSpecialFunctionModifier records that this is a constructor (set in set_special_function_modifier()).
               if (p->is_explicit_constructor)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("In set_function_modifier(): detected case of constructor using explicit keyword syntax \n");
#endif
                    modifier.setExplicit();
                  }

            // DQ (4/13/2019): Added support to mark ctors as deleted.
               if (p->is_deleted)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("In set_function_modifier(): detected case of constructor using \"= delete\" syntax \n");
#endif
                    modifier.setMarkedDelete();
                  }

            // DQ (4/13/2019): Added support to mark ctors as default.
               if (p->is_trivial_default_constructor)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("In set_function_modifier(): detected case of default constructor \n");
#endif
                 // ROSE_ASSERT(p->is_defaulted);
                 // modifier.setMarkedDefault();
                  }

            // DQ (4/13/2019): Added support to mark ctors as default.
               if (p->is_defaulted)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("In set_function_modifier(): detected case of constructor using \"= default\" syntax \n");
#endif
                    modifier.setMarkedDefault();

                 // DQ (4/13/2019): This is not always true (see Cxx11_tests/test2019_358.C).
                 // ROSE_ASSERT(p->is_trivial_default_constructor == true);

                 // DQ (4/13/2019): Check that this has not been overwritten.
                    if (p->is_explicit_constructor)
                       {
                         ROSE_ASSERT(modifier.isExplicit() == true);
                       }
                  }
#if 0
            // DQ (4/13/2019): check why this is being set (appears in output when not epecifided explicitly).
               if (modifier.isInline() == true)
                  {
                    printf ("NOTE: modifier.isInline() == true \n");
                  }
#endif
            // DQ (4/13/2019): We could also keep track of these added properties (though setting 
            // them statically would not be as powerful as deriving them dynamically).
            // is_inheriting_ctor
            // is_delegating_ctor
            // is_initializer_list_ctor
            // is_trivial_copy_function
            // is_explicit_conversion_function
               break;
             }

          case sfk_destructor:	// A destructor. 
             {
            // This is set as part of the SgSpecialFunctionModifier
            // ROSE_ASSERT(!"sfk_destructor not implemented");
#ifdef DEBUG_CONVERSION
               printf ("In set_function_modifier(): Specification as destructor is not set in the SgFunctionModifier \n");
#endif
               break;
             }

          case sfk_conversion:	// A conversion operator function. 
             {
            // This is set as part of the SgSpecialFunctionModifier
            // ROSE_ASSERT(!"sfk_conversion not implemented");
#ifdef DEBUG_CONVERSION
               printf ("In set_function_modifier(): Specification as conversion is not set in the SgFunctionModifier \n");
#endif
               break;
             }
           case sfk_udl_operator:
             {
#ifdef DEBUG_CONVERSION
               printf ("In set_function_modifier(): Specification as udl_operator is not set in the SgFunctionModifier \n");
#endif
            // DQ (3/5/2017): Converted to use message logging.
               mprintf ("In set_function_modifier(): Detected a literal operator function \n");
            // ROSE_ASSERT(false);
               break;
             }
          case sfk_operator:		// Any other operator function. 
             {
            // ROSE_ASSERT(!"sfk_operator not implemented");
#ifdef DEBUG_CONVERSION
               printf ("In set_function_modifier(): Specification as operator is not set in the SgFunctionModifier \n");
#endif
               break;
             }

        // DQ (8/13/2014): Added as part of C++11 work.
           case sfk_lambda_entry_point:
             {
#ifdef DEBUG_CONVERSION
               printf ("In set_function_modifier(): Specification as lambda_entry_point is not set in the SgFunctionModifier \n");
#endif
               ROSE_ASSERT(false);
               break;
             }

#if MICROSOFT_EXTENSIONS_ALLOWED
          case sfk_static_constructor: // A C++/CLI static constructor. 
             {
               ROSE_ASSERT(!"sfk_static_constructor not implemented");
               break;
             }

          case sfk_finalizer:	         // A C++/CLI finalizer.
             {
               ROSE_ASSERT(!"sfk_finalizer not implemented");
               break;
             }

        // DQ (8/13/2014): Added as part of C++11 work.
           case sfk_idisposable_dispose:
             {
#ifdef DEBUG_CONVERSION
               printf ("In set_function_modifier(): Specification as idisposable_dispose is not set in the SgFunctionModifier \n");
#endif
               ROSE_ASSERT(false);
               break;
             }

        // DQ (8/13/2014): Added as part of C++11 work.
           case sfk_dispose_bool:
             {
#ifdef DEBUG_CONVERSION
               printf ("In set_function_modifier(): Specification as dispose_bool is not set in the SgFunctionModifier \n");
#endif
               ROSE_ASSERT(false);
               break;
             }

        // DQ (8/13/2014): Added as part of C++11 work.
           case sfk_object_finalize:
             {
#ifdef DEBUG_CONVERSION
               printf ("In set_function_modifier(): Specification as object_finalize is not set in the SgFunctionModifier \n");
#endif
               ROSE_ASSERT(false);
               break;
             }


          case sfk_property_get:	      // A "get" accessor function of a C++/CLI property.
#if !MICROSOFT_EXTENSIONS_ALLOWED
          case sfk_first_accessor:     // Note: sfk_first_accessor == sfk_property_get
#endif
             {
               ROSE_ASSERT(!"sfk_property_get not implemented");
               break;
             }

          case sfk_property_set:        // A "set" accessor function of a C++/CLI property.
             {
               ROSE_ASSERT(!"sfk_property_set not implemented");
               break;
             }

          case sfk_event_add:          // An "add" accessor function of a C++/CLI event.
             {
               ROSE_ASSERT(!"sfk_event_add not implemented");
               break;
             }

          case sfk_event_remove:       // A "remove" accessor function of a C++/CLI event.
             {
               ROSE_ASSERT(!"sfk_event_remove not implemented");
               break;
             }

          case sfk_event_raise:        // A "raise" accessor function of a C++/CLI event.
#if !MICROSOFT_EXTENSIONS_ALLOWED
          case sfk_last_accessor:      // Note: sfk_last_accessor == sfk_event_raise.
#endif
             {
               ROSE_ASSERT(!"sfk_event_raise not implemented");
               break;
             }
#endif

#if GNU_BUILTIN_SYNC_FUNCTIONS_ALLOWED
           case sfk_gnu_sync_concrete_function:
              {
#ifdef DEBUG_CONVERSION
                printf ("In set_function_modifier(): Specification as gnu_sync_concrete_function is not set in the SgFunctionModifier \n");
#endif
                ROSE_ASSERT(false);
                break;
              }
           case sfk_gnu_atomic_nongeneric_function:
              {
#ifdef DEBUG_CONVERSION
                printf ("In set_function_modifier(): Specification as gnu_atomic_nongeneric_function is not set in the SgFunctionModifier \n");
#endif
                ROSE_ASSERT(false);
                break;
              }
#endif
           case sfk_deduction_guide: /* A routine entry representing a C++17 deduction guide. */
              {
#ifdef DEBUG_CONVERSION
                printf ("In set_function_modifier(): Specification as sfk_deduction_guide is not implemented yet in the SgFunctionModifier \n");
#endif
             // ROSE_ASSERT(false);
                break;
              }

          default:
             {
               printf ("Default reached in switch --- p->special_kind = %d \n",p->special_kind);
               ROSE_ASSERT(false);
             }
        }

#if MICROSOFT_EXTENSIONS_ALLOWED
     if (p->interface_slot == true) 
        {
          ROSE_ASSERT(!"MS: interface_slot not implemented");
        }
#endif
   }

bool isInTypeDeclarationMap(a_type_ptr p)  
   {
  // DQ (6/9/2013): Added this function to support debugging.

      ROSE_ASSERT(p != NULL);

     bool returnValue = false;
     if (type_declaration_map.find(p) == type_declaration_map.end())
        {
#ifdef DEBUG_CONVERSION
           printf ("WARNING: isInTypeDeclarationMap(): p = %p is not in the type_declaration_map \n",p);
#endif
        }
       else
        {
       // If it is in the map then make sure that the vector has entries in it.
          ROSE_ASSERT(type_declaration_map[p].empty() == false);
#if 0
          bool isAvailableAsDefiningDeclaration = isInTypeDeclarationMapAsDefiningDeclaration(p);
          printf ("In isInTypeDeclarationMap(): isAvailableAsDefiningDeclaration = %s \n",isAvailableAsDefiningDeclaration ? "true" : "false");
#endif
          returnValue = true;
        }

     return returnValue;
   }

bool hasFirstNondefDeclForTemplate(a_template_ptr p) {
  ROSE_ASSERT(p != NULL);

  if (p->canonical_template != NULL)
    p = p->canonical_template;

  if (first_nondefining_declarations_for_templates.find(p) != first_nondefining_declarations_for_templates.end()) {
    ROSE_ASSERT(first_nondefining_declarations_for_templates[p] != NULL);
    return true;
  } else {
    return false;
  }
}

SgDeclarationStatement * getFirstNondefDeclForTemplate(a_template_ptr p) {
  ROSE_ASSERT(p != NULL);

  if (p->canonical_template != NULL)
    p = p->canonical_template;

  if (first_nondefining_declarations_for_templates.find(p) != first_nondefining_declarations_for_templates.end()) {
    ROSE_ASSERT(first_nondefining_declarations_for_templates[p] != NULL);
    return first_nondefining_declarations_for_templates[p];
  } else {
    return NULL;
  }
}

void setFirstNondefDeclForTemplate(a_template_ptr p, SgDeclarationStatement * decl) {
  ROSE_ASSERT(p != NULL);
  ROSE_ASSERT(decl != NULL);

  if (p->canonical_template != NULL)
    p = p->canonical_template;

  first_nondefining_declarations_for_templates[p] = decl;
}

bool hasDefiningDeclForTemplate(a_template_ptr p) {
  ROSE_ASSERT(p != NULL);

  if (p->canonical_template != NULL)
    p = p->canonical_template;
  if (p->definition_template != NULL)
    p = p->definition_template;

  if (defining_declarations_for_templates.find(p) != defining_declarations_for_templates.end()) {
    ROSE_ASSERT(defining_declarations_for_templates[p] != NULL);
    return true;
  } else {
    return false;
  }
}

SgDeclarationStatement * getDefiningDeclForTemplate(a_template_ptr p) {
  ROSE_ASSERT(p != NULL);

  if (p->canonical_template != NULL)
    p = p->canonical_template;
  if (p->definition_template != NULL)
    p = p->definition_template;

  if (defining_declarations_for_templates.find(p) != defining_declarations_for_templates.end()) {
    ROSE_ASSERT(defining_declarations_for_templates[p] != NULL);
    return defining_declarations_for_templates[p];
  } else {
    return NULL;
  }
}

void setDefiningDeclForTemplate(a_template_ptr p, SgDeclarationStatement * decl) {
  ROSE_ASSERT(p != NULL);
  ROSE_ASSERT(decl != NULL);

  if (p->canonical_template != NULL)
    p = p->canonical_template;
  if (p->definition_template != NULL)
    p = p->definition_template;

  defining_declarations_for_templates[p] = decl;
}

SgDeclarationScope * get_nonreal_declaration_scope(SgNode * n) {
  ROSE_ASSERT(n != NULL);

  SgDeclarationScope * nrscope = NULL;
  switch (n->variantT()) {
    case V_SgTemplateDeclaration: {
      SgTemplateDeclaration * tpl = isSgTemplateDeclaration(n);
      nrscope = tpl->get_nonreal_decl_scope();
      break;
    }
    case V_SgTemplateClassDeclaration: {
      SgTemplateClassDeclaration * tpl = isSgTemplateClassDeclaration(n);
      nrscope = tpl->get_nonreal_decl_scope();
      break;
    }
    case V_SgTemplateFunctionDeclaration: {
      SgTemplateFunctionDeclaration * tpl = isSgTemplateFunctionDeclaration(n);
      nrscope = tpl->get_nonreal_decl_scope();
      break;
    }
    case V_SgTemplateMemberFunctionDeclaration: {
      SgTemplateMemberFunctionDeclaration * tpl = isSgTemplateMemberFunctionDeclaration(n);
      nrscope = tpl->get_nonreal_decl_scope();
      break;
    }
    case V_SgTemplateVariableDeclaration: {
      SgTemplateVariableDeclaration * tpl = isSgTemplateVariableDeclaration(n);
      nrscope = tpl->get_nonreal_decl_scope();
      break;
    }
    case V_SgTemplateTypedefDeclaration: {
      SgTemplateTypedefDeclaration * tpl = isSgTemplateTypedefDeclaration(n);
      nrscope = tpl->get_nonreal_decl_scope();
      break;
    }
    case V_SgNonrealDecl: {
      SgNonrealDecl * tpl = isSgNonrealDecl(n);
      nrscope = tpl->get_nonreal_decl_scope();
      break;
    }
    default: {
      printf("In get_nonreal_declaration_scope: Unrecognized: n = %p (%s)\n", n, n->class_name().c_str());
      ROSE_ASSERT(false);
    }
  }

  ROSE_ASSERT( !( nrscope != NULL ) || ( nrscope->get_parent() == n ) );

  return nrscope;
}

void set_nonreal_declaration_scope(SgNode * n, SgDeclarationScope * nrscope) {
  ROSE_ASSERT(n != NULL);

  switch (n->variantT()) {
    case V_SgTemplateDeclaration: {
      SgTemplateDeclaration * tpl = isSgTemplateDeclaration(n);
      tpl->set_nonreal_decl_scope(nrscope);
      break;
    }
    case V_SgTemplateClassDeclaration: {
      SgTemplateClassDeclaration * tpl = isSgTemplateClassDeclaration(n);
      tpl->set_nonreal_decl_scope(nrscope);
      break;
    }
    case V_SgTemplateFunctionDeclaration: {
      SgTemplateFunctionDeclaration * tpl = isSgTemplateFunctionDeclaration(n);
      tpl->set_nonreal_decl_scope(nrscope);
      break;
    }
    case V_SgTemplateMemberFunctionDeclaration: {
      SgTemplateMemberFunctionDeclaration * tpl = isSgTemplateMemberFunctionDeclaration(n);
      tpl->set_nonreal_decl_scope(nrscope);
      break;
    }
    case V_SgTemplateVariableDeclaration: {
      SgTemplateVariableDeclaration * tpl = isSgTemplateVariableDeclaration(n);
      tpl->set_nonreal_decl_scope(nrscope);
      break;
    }
    case V_SgTemplateTypedefDeclaration: {
      SgTemplateTypedefDeclaration * tpl = isSgTemplateTypedefDeclaration(n);
      tpl->set_nonreal_decl_scope(nrscope);
      break;
    }
    case V_SgNonrealDecl: {
      SgNonrealDecl * tpl = isSgNonrealDecl(n);
      tpl->set_nonreal_decl_scope(nrscope);
      break;
    }
    default: {
      printf("In set_nonreal_declaration_scope: Unrecognized: n = %p (%s)\n", n, n->class_name().c_str());
      ROSE_ASSERT(false);
    }
  }

  if (nrscope != NULL)
    nrscope->set_parent(n);
}

#define DEBUG_GET_NONREAL_DECLARATION_SCOPE_FOR_TEMPLATE 0

SgDeclarationScope * get_nonreal_declaration_scope_for_template_decl(a_template_decl_ptr template_decl) {
  SgDeclarationScope * nonreal_decl_scope = NULL;

  a_scope_ptr scope = template_decl->scope;
  ROSE_ASSERT(scope != NULL);
  ROSE_ASSERT(scope->kind == sck_template_declaration);

  if (nonreal_scope_cache.find(scope) != nonreal_scope_cache.end()) {
    nonreal_decl_scope = nonreal_scope_cache[scope];
  }

  return nonreal_decl_scope;
}

SgDeclarationScope * get_nonreal_declaration_scope_for_template(a_template_ptr p, const char * caller) {
#if DEBUG_GET_NONREAL_DECLARATION_SCOPE_FOR_TEMPLATE
  printf("ENTER get_nonreal_declaration_scope_for_template(...)\n");
  printf(" --- p = %p\n", p);
  printf(" --- caller = %s\n", caller);
#endif

  ROSE_ASSERT(p != NULL);

  SgDeclarationScope * nonreal_decl_scope = NULL;

  if (nonreal_scope_cache_from_template.find(p) != nonreal_scope_cache_from_template.end()) {
    nonreal_decl_scope = nonreal_scope_cache_from_template[p];
#if DEBUG_GET_NONREAL_DECLARATION_SCOPE_FOR_TEMPLATE
    printf(" --- nrscope = %p (found template decl)\n", nonreal_decl_scope);
#endif
  }

  a_template_ptr p_can = p->canonical_template;
  a_template_ptr p_def = p->definition_template;
//a_template_ptr p_pro = p->prototype_template;

  a_template_decl_ptr template_decl     = p->template_decl;
#if DEBUG_GET_NONREAL_DECLARATION_SCOPE_FOR_TEMPLATE
  printf(" --- template_decl = %p\n", template_decl);
#endif
  a_template_decl_ptr template_decl_can = p_can ? p_can->template_decl : NULL;
#if DEBUG_GET_NONREAL_DECLARATION_SCOPE_FOR_TEMPLATE
  printf(" --- template_decl_can = %p\n", template_decl_can);
#endif
  a_template_decl_ptr template_decl_def = p_def ? p_def->template_decl : NULL;
#if DEBUG_GET_NONREAL_DECLARATION_SCOPE_FOR_TEMPLATE
  printf(" --- template_decl_def = %p\n", template_decl_def);
#endif

  if (nonreal_decl_scope == NULL && template_decl != NULL) {
    nonreal_decl_scope = get_nonreal_declaration_scope_for_template_decl(template_decl);
#if DEBUG_GET_NONREAL_DECLARATION_SCOPE_FOR_TEMPLATE
    printf(" --- nrscope = %p (found)\n", nonreal_decl_scope);
#endif
  }

  if (nonreal_decl_scope == NULL && template_decl_can != NULL && template_decl_can != template_decl) {
    nonreal_decl_scope = get_nonreal_declaration_scope_for_template_decl(template_decl_can);
#if DEBUG_GET_NONREAL_DECLARATION_SCOPE_FOR_TEMPLATE
    printf(" --- nrscope = %p (found can)\n", nonreal_decl_scope);
#endif
  }

  if (nonreal_decl_scope == NULL && template_decl_def != NULL && template_decl_def != template_decl && template_decl_def != template_decl_can) {
    nonreal_decl_scope = get_nonreal_declaration_scope_for_template_decl(template_decl_def);
#if DEBUG_GET_NONREAL_DECLARATION_SCOPE_FOR_TEMPLATE
    printf(" --- nrscope = %p (found def)\n", nonreal_decl_scope);
#endif
  }

  if (nonreal_decl_scope == NULL) {
    nonreal_decl_scope = construct_nonreal_declaration_scope();
#if DEBUG_GET_NONREAL_DECLARATION_SCOPE_FOR_TEMPLATE
    printf(" --- nrscope = %p (new)\n", nonreal_decl_scope);
#endif
  }

  if (template_decl != NULL) {
    a_scope_ptr scope = template_decl->scope;
    ROSE_ASSERT(scope != NULL);
    ROSE_ASSERT(scope->kind == sck_template_declaration);

    if (nonreal_scope_cache.find(scope) != nonreal_scope_cache.end()) {
      ROSE_ASSERT(nonreal_decl_scope == nonreal_scope_cache[scope]);
    } else {
      nonreal_scope_cache[scope] = nonreal_decl_scope;
    }
  }

  if (template_decl_can != NULL && template_decl_can != template_decl) {
    a_scope_ptr scope = template_decl_can->scope;
    ROSE_ASSERT(scope != NULL);
    ROSE_ASSERT(scope->kind == sck_template_declaration);

    if (nonreal_scope_cache.find(scope) != nonreal_scope_cache.end()) {
      ROSE_ASSERT(nonreal_decl_scope == nonreal_scope_cache[scope]);
    } else {
      nonreal_scope_cache[scope] = nonreal_decl_scope;
    }
  }

  if (template_decl_def != NULL && template_decl_def != template_decl && template_decl_def != template_decl_can) {
    a_scope_ptr scope = template_decl_def->scope;
    ROSE_ASSERT(scope != NULL);
    ROSE_ASSERT(scope->kind == sck_template_declaration);

    if (nonreal_scope_cache.find(scope) != nonreal_scope_cache.end()) {
      ROSE_ASSERT(nonreal_decl_scope == nonreal_scope_cache[scope]);
    } else {
      nonreal_scope_cache[scope] = nonreal_decl_scope;
    }
  }

  nonreal_scope_for_template_parameters_stack.push_back(nonreal_decl_scope);

#if DEBUG_GET_NONREAL_DECLARATION_SCOPE_FOR_TEMPLATE
  printf("LEAVE get_nonreal_declaration_scope_for_template(...)\n");
#endif

  return nonreal_decl_scope;
}

#define DEBUG_ATTACH_NONREAL_DECLARATION_SCOPE_TO_TEMPLATE 0

void attach_nonreal_declaration_scope_to_template(a_template_ptr p, SgDeclarationStatement * tpldecl, SgDeclarationScope * nrscope, const char * caller) {
#if DEBUG_ATTACH_NONREAL_DECLARATION_SCOPE_TO_TEMPLATE
  printf("ENTER attach_nonreal_declaration_scope_to_template(...):\n");
  printf(" --- p = %p\n", p);
  printf(" --- tpldecl = %p (%s)\n", tpldecl, tpldecl ? tpldecl->class_name().c_str() : "");
  printf(" --- nrscope = %p (%s)\n", nrscope, nrscope ? nrscope->class_name().c_str() : "");
  printf(" --- caller = %s\n", caller);
#endif

  ROSE_ASSERT(tpldecl != NULL);
  ROSE_ASSERT(nrscope != NULL);

  SgDeclarationStatement * first_nondef = tpldecl->get_firstNondefiningDeclaration();
  ROSE_ASSERT(first_nondef != NULL);
#if DEBUG_ATTACH_NONREAL_DECLARATION_SCOPE_TO_TEMPLATE
  printf(" --- first_nondef = %p (%s)\n", first_nondef, first_nondef ? first_nondef->class_name().c_str() : "");
#endif
  
  SgDeclarationStatement * first_nondef_tpl = getFirstNondefDeclForTemplate(p);
  ROSE_ASSERT(first_nondef_tpl != NULL);
#if DEBUG_ATTACH_NONREAL_DECLARATION_SCOPE_TO_TEMPLATE
  printf(" --- first_nondef_tpl = %p (%s)\n", first_nondef_tpl, first_nondef_tpl ? first_nondef_tpl->class_name().c_str() : "");
#endif

//ROSE_ASSERT( !( first_nondef_tpl != NULL ) || ( first_nondef == first_nondef_tpl ) );

  SgNode * current_parent = nrscope->get_parent();
  if (current_parent != NULL && current_parent != first_nondef) {
    printf("WARNING: Parent of non-real scope is not the first nondefining template. Reset!\n");
    set_nonreal_declaration_scope(current_parent, NULL);
  }
#if DEBUG_ATTACH_NONREAL_DECLARATION_SCOPE_TO_TEMPLATE
  printf(" --- current_parent = %p (%s)\n", current_parent, current_parent ? current_parent->class_name().c_str() : "");
#endif

#if DEBUG_ATTACH_NONREAL_DECLARATION_SCOPE_TO_TEMPLATE
  SgDeclarationScope * current_nrscope = get_nonreal_declaration_scope(first_nondef);
  printf(" --- current_nrscope = %p (%s)\n", current_nrscope, current_nrscope ? current_nrscope->class_name().c_str() : "");
#endif

  // TV (09/15/2018): Fails in merge test for the second compilation unit (mergeTest_106.C).
  // ROSE_ASSERT( !( current_nrscope != NULL ) || ( current_nrscope == nrscope ) );

  set_nonreal_declaration_scope(first_nondef, nrscope);
  if (nonreal_scope_cache_from_template.find(p) == nonreal_scope_cache_from_template.end()) {
    nonreal_scope_cache_from_template[p] = nrscope;
  } else {
    ROSE_ASSERT(nonreal_scope_cache_from_template[p] == nrscope);
  }

  ROSE_ASSERT(nonreal_scope_for_template_parameters_stack.size() > 0);
  ROSE_ASSERT(nonreal_scope_for_template_parameters_stack.back() == nrscope);
  nonreal_scope_for_template_parameters_stack.pop_back();

#if DEBUG_ATTACH_NONREAL_DECLARATION_SCOPE_TO_TEMPLATE
  printf("LEAVE attach_nonreal_declaration_scope_to_template(...)\n");
#endif
}

a_scope_ptr
getRootScope(a_namespace_ptr nsp)
   {
  // DQ (8/6/2012): Refactored this code so it could be called from multiple locations.

  // SgScopeStatement* aliasedNamespaceScope = aliased_nsp->scope->sg_scope_ptr;
     a_scope_ptr aliased_nsp_scope = NULL;

  // DQ (7/8/2014): I think we can assert this.
     ROSE_ASSERT(nsp != NULL);

#if 0
     printf ("In getRootScope(): nsp = %p nsp->is_namespace_alias = %s \n",nsp,nsp->is_namespace_alias ? "true" : "false");
#endif

     if (nsp->is_namespace_alias == true)
        {
          ROSE_ASSERT(nsp->variant.assoc_namespace != NULL);
          aliased_nsp_scope = nsp->variant.assoc_namespace->variant.assoc_scope;
          ROSE_ASSERT(aliased_nsp_scope != NULL);

       // DQ (4/9/2018): This is actaully allowed in C++ (see test2018_57.C).
       // DQ (7/8/2014): Detect if the alias namespace is itself an alias.
       // ROSE_ASSERT(nsp->variant.assoc_namespace->is_namespace_alias == false);
        }
       else
        {
          aliased_nsp_scope = nsp->variant.assoc_scope;
          ROSE_ASSERT(aliased_nsp_scope != NULL);
        }

     ROSE_ASSERT(aliased_nsp_scope != NULL);

     return aliased_nsp_scope;
   }


a_namespace_ptr
getRootNamespace(a_namespace_ptr nsp)
   {
  // DQ (8/6/2012): Refactored this code so it could be called from multiple locations.

     a_namespace_ptr tmp_nsp      = nsp; // aliased_nsp->variant.assoc_namespace;
     a_namespace_ptr previous_nsp = nsp; // aliased_nsp->variant.assoc_namespace;

  // This will traverse the alised namespace scopes to get the the root scope (this might not be what we always want).
     while (tmp_nsp != NULL)
        {

          ROSE_ASSERT(tmp_nsp != NULL);
          if (tmp_nsp->is_namespace_alias == true)
             {
               tmp_nsp = tmp_nsp->variant.assoc_namespace;
               ROSE_ASSERT(tmp_nsp != NULL);

               previous_nsp = tmp_nsp;
             }
            else
             {
            // End the while loop
               tmp_nsp = NULL;
             }
        }

     ROSE_ASSERT(previous_nsp != NULL);

     return previous_nsp;
   }

SgSymbol*
getAssociatedTemplateSymbol(a_template_ptr edg_template, SgDeclarationStatement* & declarationStatement)
   {
     ROSE_ASSERT(edg_template != NULL);

     SgSymbol* templateSymbol = NULL;

     SgName nameWithOutTemplateSpecializationArguments = edg_template->source_corresp.name;
     SgName nameWithTemplateSpecializationArguments    = nameWithOutTemplateSpecializationArguments;

     a_scope_ptr edg_scope = edg_template->source_corresp.parent_scope;
     ROSE_ASSERT(edg_scope != NULL);

  // DQ (3/11/2012): check that this is not already in the list before we add it (or
  // verify it is NULL if it is supposed to be in the list but just not initialized yet).
     ROSE_ASSERT (scope_cache.find(edg_scope) != scope_cache.end());

     SgScopeStatement* scopeOfReference = scope_cache[edg_scope];
     ROSE_ASSERT(scopeOfReference != NULL);

#if 0
     printf ("In getAssociatedTemplateSymbol(): scopeOfReference = %p = %s \n",scopeOfReference,scopeOfReference->class_name().c_str());
     printf ("--- nameWithOutTemplateSpecializationArguments         = %s \n",nameWithOutTemplateSpecializationArguments.str());
     printf ("--- nameWithTemplateSpecializationArguments            = %s \n",nameWithTemplateSpecializationArguments.str());
#endif

  // Error check the recursion depth (should never be greater than 1 (I think).
     static int counter = 0;
     if (counter > 1)
        {
#ifdef DEBUG_CONVERSION
          printf ("WARNING: Greater than expected recursive depth: counter = %d \n",counter);
#endif
       // DQ (3/7/2017): Changed to support debugging.
          if (counter > 20)
             {
               printf ("Error: Greater than tollerated recursive depth: counter = %d \n",counter);
               ROSE_ASSERT(false);
             }
        }

  // DQ (9/1/2012): We need to check what kind of TemplateSymbol we need to search for: class, function, variable, ???.
     a_template_kind kind = edg_template->kind;

#if 0
     printf ("In getAssociatedTemplateSymbol(): template kind = %s \n",summarize_template_kind_in_string(kind));
#endif

     switch(kind)
        {
          case templk_none:
             {
               printf ("Error: In getAssociatedTemplateSymbol(): templk_none not implemented \n");
               ROSE_ASSERT(false);
               break;
             }

          case templk_class:
             {
#ifdef DEBUG_CONVERSION
               printf ("In getAssociatedTemplateSymbol(): templk_class templateSymbol = %p \n",templateSymbol);
#endif
               SgTemplateClassSymbol*   templateClassSymbol   = isSgTemplateClassSymbol(templateSymbol);
               SgTemplateTypedefSymbol* templateTypedefSymbol = NULL; // isSgTemplateTypedefSymbol(templateSymbol);

            // DQ (9/10/2012): I think this is always NULL, why do we have this strange construction of code?
               if (templateSymbol == NULL)
                  {
                 // In the FIRST iteration templateSymbol == NULL, so we have to look it up.

                 // DQ (9/10/2012): We need to setup the template parameters for this before we can call lookup_template_class_symbol().
                 // SgTemplateParameterPtrList templateParameterList; // parse_template_parameters( a_type_ptr p, scopeOfReference, NULL);
                    a_template_parameter_ptr templateParameter = edg_template->template_decl->param_list;
                    ROSE_ASSERT(templateParameter != NULL);
#if 0
                    printf ("In getAssociatedTemplateSymbol(): templk_class: Calling parse_template_parameters() \n");
#endif
                    SgTemplateParameterPtrList templateParameterList = parse_template_parameters(templateParameter, scopeOfReference, NULL);
#if 0
                    printf ("DONE: In getAssociatedTemplateSymbol(): templk_class: Calling parse_template_parameters() \n");
#endif
#if 0
                    printf ("ERROR: In getAssociatedTemplateSymbol(): templateParameterList is not setup properly: calling summarize_template() \n");
                    summarize_template(edg_template);
#endif
#if 0
                    printf ("ERROR: In getAssociatedTemplateSymbol(): templateParameterList is not setup properly! \n");
                    ROSE_ASSERT(false);
#endif
#if 1
                    SgTemplateArgumentPtrList templateSpecializationArgumentList;

#ifdef DEBUG_CONVERSION
                    printf ("WARNING: In getAssociatedTemplateSymbol(): Setup an empty templateSpecializationArgumentList since we only have access to the edg_template = %p \n",edg_template);
#endif
#if 0
                    printf ("In getAssociatedTemplateSymbol(): case templk_class: calling summarize_template() \n");
                    summarize_template(edg_template);
#endif
#if 0
                    printf ("In getAssociatedTemplateSymbol(): case templk_class: templateSpecializationArgumentList is not properly setup! (exiting as a test) \n");
                    ROSE_ASSERT(false);
#endif
#else
                 // DQ (9/11/2012): Setup the template specialization arguments (these also have to be passed to the SageBuilder API (since they are also needed to disambiguate template names).
                 // a_template_arg_ptr templateSpecializationArgs = p->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
                 // This is the list of template arguments on which the list of "actual" arguments is based.
                    a_template_arg_ptr templateSpecializationArgs        = p->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
                 // This is the list of "actual" template arguments.
                    a_template_arg_ptr templateSpecializationArgs_actual = p->variant.class_struct_union.extra_info->template_arg_list;
                    SgTemplateArgumentPtrList templateSpecializationArgumentList;
#error "DEAD CODE!"
                    if (templateSpecializationArgs != NULL)
                       {
                      // DQ (9/12/2012): This is tricky: If this is a partial specialization then the "actual" template arguments are in 
                      // the p->variant.class_struct_union.extra_info->template_arg_list and the associated names of the template arguments 
                      // are in the p->variant.class_struct_union.extra_info->partial_spec_template_arg_list list.  This is because the names
                      // need not match that of where they are defined in an outer template.
                         ROSE_ASSERT(templateSpecializationArgs_actual != NULL);
                         templateSpecializationArgumentList = parse_template_arguments(templateSpecializationArgs_actual,scopeOfDeclaration);
                       }
#endif
                 // templateSymbol = scopeOfReference->lookup_template_class_symbol(name);

                 // DQ (9/12/2012): Adding support for template specialization.
                    nameWithTemplateSpecializationArguments = appendTemplateArgumentsToName(nameWithOutTemplateSpecializationArguments,templateSpecializationArgumentList);
#if 0
                 // printf ("In getAssociatedTemplateSymbol(): case templk_class: Calling lookup_template_class_symbol() using name = %s \n",name.str());
                    printf ("In getAssociatedTemplateSymbol(): case templk_class: Calling lookup_template_class_symbol() using nameWithTemplateSpecializationArguments = %s \n",nameWithTemplateSpecializationArguments.str());
#endif
                    templateSymbol = scopeOfReference->lookup_template_class_symbol(nameWithTemplateSpecializationArguments,&templateParameterList,&templateSpecializationArgumentList);
#if 0
                    printf ("1st try: templateSymbol = %p \n",templateSymbol);
#endif
                    templateClassSymbol = isSgTemplateClassSymbol(templateSymbol);

                    if (templateSymbol == NULL)
                       {
                         templateSymbol = scopeOfReference->lookup_template_typedef_symbol(nameWithTemplateSpecializationArguments);
#if 0
                         printf ("2nd try: templateSymbol = %p \n",templateSymbol);
#endif
                         templateTypedefSymbol = isSgTemplateTypedefSymbol(templateSymbol);
                       }
                  }
                 else
                  {
                    ROSE_ASSERT(templateClassSymbol != NULL);
                  }

            // If we found the symbol then get the declaration.
               if (templateClassSymbol != NULL)
                  {
                    declarationStatement = templateClassSymbol->get_declaration();
                    ROSE_ASSERT(declarationStatement != NULL);
                  }
                 else
                  {
                    if (templateTypedefSymbol != NULL)
                       {
                         declarationStatement = templateTypedefSymbol->get_declaration();
                         ROSE_ASSERT(declarationStatement != NULL);
                       }
                  }

#if 0
               printf ("In getAssociatedTemplateSymbol(): templk_class: templateSymbol       = %p \n",templateSymbol);
               printf ("In getAssociatedTemplateSymbol(): templk_class: declarationStatement = %p \n",declarationStatement);
#endif
               break;
             }

          case templk_function:
             {
#ifdef DEBUG_CONVERSION
               printf ("In getAssociatedTemplateSymbol(): templk_function templateSymbol = %p \n",templateSymbol);
#endif
            // DQ (9/1/2012): It seems like it is easier to just get the declaration from the edg_template (this is taken from the cache map if it has been seem previously).
               declarationStatement = convert_template_prototype(edg_template);
               ROSE_ASSERT(declarationStatement != NULL);
               templateSymbol = declarationStatement->search_for_symbol_from_symbol_table();
               ROSE_ASSERT(templateSymbol != NULL);
               break;
             }

          case templk_member_function:
             {
               printf ("Error: In getAssociatedTemplateSymbol(): templk_member_function not implemented (need an example of this) \n");
               ROSE_ASSERT(false);
               break;
             }

          case templk_static_data_member:
             {
               printf ("Error: In getAssociatedTemplateSymbol(): templk_static_data_member not implemented (need an example of this) \n");
               ROSE_ASSERT(false);
               break;
             }

          case templk_member_class:
             {
               printf ("Error: In getAssociatedTemplateSymbol(): templk_member_class not implemented (need an example of this) \n");
               ROSE_ASSERT(false);
               break;
             }

          case templk_template_template_param:
             {
               printf ("Error: In getAssociatedTemplateSymbol(): templk_template_template_param not implemented (need an example of this) \n");
               ROSE_ASSERT(false);
               break;
             }

          default:
             {
               printf ("In getAssociatedTemplateSymbol(): kind unknown kind = %d \n",kind);
               ROSE_ASSERT(false);
             }
        }

  // Make a recursive call to build the template.
     if (templateSymbol == NULL)
        {
       // If we have not found the symbol then call this function again AFTER building the associated template declaration.
          declarationStatement = convert_template_prototype(edg_template);
          ROSE_ASSERT(declarationStatement != NULL);

          counter++;

#if 0
          printf ("************** Making recursive call to getAssociatedTemplateSymbol() \n");
          printf ("--- declarationStatement = %p = %s \n",declarationStatement,declarationStatement->class_name().c_str());
#endif

       // Now go get the associated symbol.
          templateSymbol = getAssociatedTemplateSymbol(edg_template,declarationStatement);
          ROSE_ASSERT(declarationStatement != NULL);

          counter--;

          ROSE_ASSERT(templateSymbol != NULL);
        }
       else
        {
          ROSE_ASSERT(templateSymbol != NULL);
          ROSE_ASSERT(declarationStatement != NULL);
        }

     ROSE_ASSERT(declarationStatement != NULL);

     ROSE_ASSERT(templateSymbol != NULL);

     return templateSymbol;
   }

SgStorageModifier
set_storage_modifiers ( a_storage_class storage_class )
   {
  // This function is called for the setting of storage modifiers within function parameters
     SgStorageModifier m;

     switch (storage_class)
        {
          case sc_extern:
               printf ("In sage_set_storage_modifiers() setting m.setExtern() \n");
               m.setExtern();
               break;
          case sc_static:
               m.setStatic();
               break;
          case sc_auto:
               m.setAuto();
               break;
          case sc_unspecified:
               m.setUnspecified();
               break;
#ifdef CIL
          case sc_typedef:
               m.setTypedef();
               break;
          case sc_register:
               m.setRegister();
               break;
          case sc_asm:
               printf ("Setting storage modifier as asm (I think this should never happen in function parameters?) \n");
               ROSE_ASSERT(false);
               m.setAsm();
               break;
#endif /* ifdef CIL */
#ifdef FIL
          case sc_local:
               m.setLocal();
               break;
          case sc_common:
               m.setCommon();
               break;
          case sc_associated:
               m.setAssociated();
               break;
          case sc_intrinsic:
               m.setIntrinsic();
               break;
          case sc_pointer_based:
               m.setPointerBased();
               break;
#endif /* ifdef FIL */
          case sc_last:
          default:
               printf ("Error: default or sc_last reached \n");
               ROSE_ASSERT (false);
        }

     return m;
   }


// DQ (8/11/2020): changed name from set_access_modifiers() to set_access_modifiers_for_base_class() 
// to avoid confusion with set_access_modifier().
// SgAccessModifier set_access_modifiers ( an_access_specifier access )
SgAccessModifier
set_access_modifiers_for_base_class ( an_access_specifier access )
   {
  // DQ (2/16/2013): Note that this function is somewhat redundant with function: 
  //      void set_access_modifier(SgDeclarationStatement* declarationStatement, unsigned int access_modifiers);
  // This later function was just added to support access preivledges of declarations
  // (where this current function would be more difficult to call, I think).

     SgAccessModifier a;

#if 0
     printf ("In set_access_modifiers_for_base_class(): access = %d \n",access);
     a.display("In set_access_modifiers_for_base_class()");
#endif

  // DQ (8/11/2020): debugging Cxx_tests/test2020_28.C (incores marking of access for extern "C" friend function.
     ROSE_ASSERT(a.isDefault() == false || a.isPublic() == false);

     switch (access)
        {
          case as_public:
#if 0
               printf ("In set_access_modifiers_for_base_class(): Mark as public \n");
#endif
               a.setPublic();
               break;

          case as_protected:
#if 0
               printf ("In set_access_modifiers_for_base_class(): Mark as protected \n");
#endif
               a.setProtected();
               break;

          case as_private:
#if 0
               printf ("In set_access_modifiers_for_base_class(): Mark as private \n");
#endif
               a.setPrivate();
               break;

          default:
               printf ("Error: default reached \n");
               ROSE_ASSERT (false);
        }

#if 0
     printf ("Leaving set_access_modifiers_for_base_class(): access = %d \n",access);
     a.display("Leaving set_access_modifiers_for_base_class()");
#endif

  // DQ (8/11/2020): debugging Cxx_tests/test2020_28.C (incores marking of access for extern "C" friend function.
     ROSE_ASSERT(a.isDefault() == false || a.isPublic() == false);

     return a;
   }

}
