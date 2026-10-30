
#include "edgRose.h"

namespace EDG_ROSE_Translation {

void fixup_scopes_of_earlier_declarations(const vector<SgDeclarationStatement*>& decls, SgScopeStatement* new_scope)
   {
     for (size_t i = 0; i < decls.size(); ++i)
        {
          SgDeclarationStatement* decl = decls[i];

          SgScopeStatement* old_scope = decl->get_scope();
          ROSE_ASSERT (old_scope != NULL);
#if 0
          printf ("In fixup_scopes_of_earlier_declarations(): decl = %p = %s old_scope = %p = %s \n",decl,decl->class_name().c_str(),old_scope,old_scope->class_name().c_str());
#endif
        }
   }

void fixup_defining_declarations_of_earlier_declarations(const vector<SgDeclarationStatement*>& decls, SgDeclarationStatement* def_decl)
   {
     for (size_t i = 0; i < decls.size(); ++i)
        {
          SgDeclarationStatement* decl = decls[i];
          decl->set_definingDeclaration(def_decl);
        }
   }

void fixup_variable_or_field_declaration(SgVariableDeclaration* decl, a_type_ptr type)
   {
#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("fixup variable or field declaration "); print_source_corresp(type->source_corresp); debug_print("\n");
#endif

#define DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION 0

  // DQ (6/19/2012): Adding support for types defined in the variable declaration.
     ROSE_ASSERT(type != NULL);

  // DQ (7/15/2012): Added assertion.
     ROSE_ASSERT(decl != NULL);

#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
     printf ("TOP of fixup_variable_or_field_declaration() decl = %p type = %p type->kind = %s \n",decl,type,summarize_type_kind_in_string(type));
     printf ("TOP of fixup_variable_or_field_declaration(): decl = %p decl->get_variableDeclarationContainsBaseTypeDefiningDeclaration() = %s \n",
          decl,decl->get_variableDeclarationContainsBaseTypeDefiningDeclaration() ? "true" : "false");
#endif
#if 0
     decl->get_file_info()->display("TOP of fixup_variable_or_field_declaration(): debug");
#endif

#if 0
     printf ("In fixup_variable_or_field_declaration(): calling summarize_type() \n");
     summarize_type(type);
#endif

#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
     printf ("In fixup_variable_or_field_declaration(): Why must we use the base type here? \n");
#endif
#if 1
  // DQ (7/16/2012): If this is an array or pointer, then we have to strip this away to find the base type.
     type = getBaseTypeOfEdgArrayOrPointerType(type);
#else
  // DQ (8/18/2016): Test if we can avoid resetting the type.
     printf ("In fixup_variable_or_field_declaration(): Skip resetting edg type using getBaseTypeOfEdgArrayOrPointerType() \n");
#endif

#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
     printf ("In fixup_variable_or_field_declaration() after computing the base type type = %p type->kind = %s \n",type,summarize_type_kind_in_string(type));
#endif

     a_type_ptr unnamed_type = contains_unnamed_type_at_top_level(type);

#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
     printf ("In fixup_variable_or_field_declaration(): unnamed_type = %p \n",unnamed_type);
#endif

  // DQ (7/16/2012): If this is an aray or pointer, then we have to strip this away to find the base type.
     if (unnamed_type != NULL && (unnamed_type->kind == tk_array || unnamed_type->kind == tk_pointer))
        {
       // This case should not happen, since we resolve the base_type before computing the unnamed_type.
          printf ("Exiting since we have to resolve the base type of unnamed_type \n");
          ROSE_ASSERT(false);
        }

#if 0
     printf ("In fixup_variable_or_field_declaration(): unnamed_type = %p \n",unnamed_type);
     if (unnamed_type != NULL)
        {
          printf ("In fixup_variable_or_field_declaration(): types_with_emitted_definitions[unnamed_type] = %s \n",types_with_emitted_definitions[unnamed_type] ? "true" : "false");
        }
     printf ("In fixup_variable_or_field_declaration(): types_with_emitted_definitions[type] = %s \n",types_with_emitted_definitions[type] ? "true" : "false");
#endif

     ROSE_ASSERT(decl != NULL);

  // DQ (10/21/2012): Is this something we can assert?
  // ROSE_ASSERT ( (unnamed_type == NULL) || ( (unnamed_type != NULL) && (types_with_emitted_definitions.find(unnamed_type) != types_with_emitted_definitions.end()) ) );
     if ( (unnamed_type != NULL) && (types_with_emitted_definitions.find(unnamed_type) == types_with_emitted_definitions.end()) )
        {
#ifdef DEBUG_CONVERSION
          printf ("In fixup_variable_or_field_declaration(): unnamed_type = %p kind = %s is NOT in types_with_emitted_definitions map \n",unnamed_type,summarize_type_kind_in_string(unnamed_type));
#endif
        }
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
     printf ("************ decl->get_baseTypeDefiningDeclaration() = %p \n",decl->get_baseTypeDefiningDeclaration());
#endif

#if 1
  // DQ (4/9/2017): This support is added to address where in class declarations unnamed types are used more than once before they are declared.

  // DQ (4/7/2017): This case handles the specific case identified in test2017_17.C.
     if ( (unnamed_type != NULL) && (types_with_emitted_definitions[unnamed_type] == true) && decl->get_baseTypeDefiningDeclaration() == NULL)
        {
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
          printf ("@@@@@@@@@@ This is the case of the variable declaration with the un-named enum type (but previously built because of a reference to the enum field in the class). \n");
#endif
       // The point here is the find the declaration for the type that is being used and to set the 
       // currently null pointer to the baseTypeDefiningDeclaration.

          SgVariableDeclaration*     variableDeclaration     = isSgVariableDeclaration(decl);
          ROSE_ASSERT(variableDeclaration != NULL);
          SgInitializedName* initializedName = SageInterface::getFirstInitializedName(variableDeclaration);
          ROSE_ASSERT(initializedName != NULL);
       // SgType* variableType = variableDeclaration->get_type();
          SgType* variableType = initializedName->get_type();
          ROSE_ASSERT(variableType != NULL);
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
          printf ("variableType = %p = %s \n",variableType,variableType->class_name().c_str());
#endif
          SgClassType*   classType   = isSgClassType(variableType);
          SgEnumType*    enumType    = isSgEnumType(variableType);
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
          SgTypedefType* typedefType = isSgTypedefType(variableType);
       // DQ (7/26/2018): Fixed compiler warnings (I think this is what was intended).
       // if ( classType == NULL && enumType == NULL || typedefType != NULL)
          if ( (classType == NULL && enumType == NULL) || typedefType != NULL)
             {
               printf ("variableType = %p = %s \n",variableType,variableType->class_name().c_str());
             }
#endif

#if 0
          if (classType == NULL && enumType == NULL && typedefType == NULL)
             {
               printf ("In fixup_variable_or_field_declaration(): Skipping assertion on variableType = %p = %s (might need to consider the stripped type) \n",variableType,variableType->class_name().c_str());
             }
       // ROSE_ASSERT(classType != NULL || enumType != NULL || typedefType != NULL);
#endif
          if (classType != NULL)
             {
            // Need to fill this case in.

            // DQ (4/23/2017): Converted to use message logging.
               mprintf ("In fixup_variable_or_field_declaration(): Case of class type used here is not implemented \n");

            // ROSE_ASSERT(false);
             }
            else
             {
            // ROSE_ASSERT(enumType != NULL);
               if (enumType != NULL)
                  {
                    SgEnumDeclaration* enumDeclaration = isSgEnumDeclaration(enumType->get_declaration());
                    ROSE_ASSERT(enumDeclaration != NULL);
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
                    printf ("enumDeclaration = %p enumDeclaration->get_isAutonomousDeclaration() = %s \n",enumDeclaration,enumDeclaration->get_isAutonomousDeclaration() ? "true" : "false");
#endif
                 // Find the defining declaration.
                    SgEnumDeclaration* enumDeclaration_defining = isSgEnumDeclaration(enumDeclaration->get_definingDeclaration());
                    ROSE_ASSERT(enumDeclaration_defining != NULL);
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
                    printf ("enumDeclaration_defining = %p enumDeclaration_defining->get_isAutonomousDeclaration() = %s \n",
                         enumDeclaration_defining,enumDeclaration_defining->get_isAutonomousDeclaration() ? "true" : "false");
#endif
                    ROSE_ASSERT(enumDeclaration_defining->get_isAutonomousDeclaration() == false);
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
                    printf ("enumDeclaration->get_parent() = %p \n",enumDeclaration->get_parent());
                    printf ("variableDeclaration = %p \n",variableDeclaration);
                    printf ("variableDeclaration->get_baseTypeDefiningDeclaration() = %p \n",variableDeclaration->get_baseTypeDefiningDeclaration());
#endif
                 // If this parent was set previously, then this is a second variable or veriable declaration, and we don't 
                 // want to refer to the defining declaration twice.  See test2017_21.C.
                 // if (enumDeclaration->get_parent() == NULL)
                 // if (enumDeclaration->get_parent() == enumDeclaration)
                    if (enumDeclaration->get_parent() != variableDeclaration)
                       {
#if 0
                         printf ("before reset: enumDeclaration->get_parent() = %p = %s \n",enumDeclaration->get_parent(),enumDeclaration->get_parent()->class_name().c_str());
#endif
                         variableDeclaration->set_baseTypeDefiningDeclaration(enumDeclaration_defining);

                      // If this is not true then we should have set it already.
                         ROSE_ASSERT(variableDeclaration->get_variableDeclarationContainsBaseTypeDefiningDeclaration() == true);
                       }
                      else
                       {
                         if (enumDeclaration->get_parent() == NULL)
                            {
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
                              printf ("enumDeclaration->get_parent() == NULL \n");
#endif
                            }
                           else
                            {
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
                              printf ("enumDeclaration->get_parent() = %p = %s \n",enumDeclaration->get_parent(),enumDeclaration->get_parent()->class_name().c_str());
#endif
                            }
                       }
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
                 // DQ (4/9/2017): This case is handled below (case of SgTypedefType).
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
                    printf ("In fixup_variable_or_field_declaration(): Case of SgTypedefType with unnamed type as base type \n");
#endif
                  }
             }
        }
#endif

  // DQ (4/7/2017): This should maybe be independent of the case of (types_with_emitted_definitions[unnamed_type] == false), need to test this idea (see test2017_17.C).
     if ( (unnamed_type != NULL) && (types_with_emitted_definitions[unnamed_type] == false) )
        {
       // If this is an un-named type and with an emitted definition, then we have to output the full type 
       // definition in the reference to the type as part of the variable declaration.  So we have to mark
       // the variable using "set_variableDeclarationContainsBaseTypeDefiningDeclaration(true)".

          SgType* rose_type = convert_type(unnamed_type);
          decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
          decl->set_baseTypeDefiningDeclaration(get_declaration_of_unnamed_type(rose_type));
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
          printf ("In fixup_variable_or_field_declaration(): TRUE BRANCH: decl->get_baseTypeDefiningDeclaration() = %p = %s \n",
               decl->get_baseTypeDefiningDeclaration(),decl->get_baseTypeDefiningDeclaration()->class_name().c_str());
#endif
       // DQ (4/9/2017): Because we can't have an unnamed typedef we only have to consider class and enum declarations.
          SgClassDeclaration* classDeclaration = isSgClassDeclaration(decl->get_baseTypeDefiningDeclaration());
          SgEnumDeclaration*  enumDeclaration  = isSgEnumDeclaration(decl->get_baseTypeDefiningDeclaration());

          ROSE_ASSERT(classDeclaration != NULL || enumDeclaration != NULL);
          if (classDeclaration != NULL)
             {
            // DQ (7/14/2012): This fails for test2012_120.C (for case of "union {};")
            // ROSE_ASSERT(classDeclaration->get_isAutonomousDeclaration() == false);
             }
            else
             {
            // DQ (3/28/2017): trueCrypt demonstrates this case and that this assertion can be false.
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
               printf ("enumDeclaration = %p enumDeclaration->get_isAutonomousDeclaration() = %s \n",enumDeclaration,enumDeclaration->get_isAutonomousDeclaration() ? "true" : "false");
#endif
#if 0
               ROSE_ASSERT(enumDeclaration->get_isAutonomousDeclaration() == false);
#endif
            // DQ (4/3/2017): Maybe this should be set to the variable declaration instead of the scope.
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
               printf ("In fixup_variable_or_field_declaration(): Found an enumDeclaration->get_isAutonomousDeclaration() == true: set the parent to the variable declaration: decl = %p \n",decl);
#endif
               enumDeclaration->set_parent(decl);
               ROSE_ASSERT(enumDeclaration->get_parent() != NULL);

               SgEnumDeclaration* enumDeclaration_nondefining = isSgEnumDeclaration(enumDeclaration->get_firstNondefiningDeclaration());
               ROSE_ASSERT(enumDeclaration_nondefining != NULL);
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
               printf ("enumDeclaration_nondefining = %p parent = %p \n",enumDeclaration_nondefining,enumDeclaration_nondefining->get_parent());
               printf ("enumDeclaration_nondefining->get_isAutonomousDeclaration() = %s \n",enumDeclaration_nondefining->get_isAutonomousDeclaration() ? "true" : "false");
#endif
#if 0
               SgEnumDeclaration* enumDeclaration_defining    = isSgEnumDeclaration(enumDeclaration->get_definingDeclaration());
            // ROSE_ASSERT(enumDeclaration_defining != NULL);
               printf ("enumDeclaration_defining    = %p \n",enumDeclaration_defining);
               if (enumDeclaration_defining != NULL)
                  {
                    printf ("enumDeclaration_defining    = %p parent = %p \n",enumDeclaration_defining,enumDeclaration_defining->get_parent());
                    printf ("enumDeclaration_defining->get_isAutonomousDeclaration() = %s \n",enumDeclaration_defining->get_isAutonomousDeclaration() ? "true" : "false");
                  }
#endif
             }

       // I think that the types_with_emitted_definitions map is the same as the outputTypesSet (so we might not 
       // need this redundant set.
          types_with_emitted_definitions[unnamed_type] = true;
        }
       else
        {
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
          printf ("In fixup_variable_or_field_declaration(): FALSE BRANCH: decl = %p = %s Either unnamed_type == NULL or types_with_emitted_definitions[unnamed_type] == true \n",decl,decl->class_name().c_str());
          SgInitializedName* initializedName = SageInterface::getFirstInitializedName(decl);
          ROSE_ASSERT(initializedName != NULL);
          printf ("   --- variableDeclarationContainsBaseTypeDefiningDeclaration = %s \n",decl->get_variableDeclarationContainsBaseTypeDefiningDeclaration() ? "true" : "false");
          if (decl->get_baseTypeDefiningDeclaration() != NULL)
             {
               printf ("   --- baseTypeDefiningDeclaration                  = %p = %s \n",decl->get_baseTypeDefiningDeclaration(),decl->get_baseTypeDefiningDeclaration()->class_name().c_str());
               if (decl->get_baseTypeDefiningDeclaration()->get_parent() != NULL)
                  {
                    printf ("   --- --- parent                                   = %p = %s \n",decl->get_baseTypeDefiningDeclaration()->get_parent(),decl->get_baseTypeDefiningDeclaration()->get_parent()->class_name().c_str());
                  }
             }
          printf ("   --- variable name =                              = %s \n",initializedName->get_name().str());
          printf ("   --- unnamed_type                                 = %p \n",unnamed_type);
          printf ("   --- types_with_emitted_definitions[unnamed_type] = %s \n",types_with_emitted_definitions[unnamed_type] ? "true" : "false");
#endif

       // DQ (5/7/2013): Handle the case of test2013_152.C.
          if (unnamed_type != NULL)
             {
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
               printf ("Handle simple case of unnamed type \n");
#endif
#if 0
               SgType* rose_type = convert_type(unnamed_type);
               decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
               decl->set_baseTypeDefiningDeclaration(get_declaration_of_unnamed_type(rose_type));
#endif
            // I think this is the key to tell us when the set variableDeclarationContainsBaseTypeDefiningDeclaration 
            // to false (and maybe the baseTypeDefiningDeclaration to NULL as well).  This happens when there are several
            // variables declared with an unnamed type declaration (see test2017_21.C).
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
               printf ("decl                                                  = %p \n",decl);
               printf ("decl->get_baseTypeDefiningDeclaration()->get_parent() = %p \n",decl->get_baseTypeDefiningDeclaration()->get_parent());
#endif
#if 0
               SgEnumDeclaration* enumDeclaration_defining    = isSgEnumDeclaration(enumDeclaration->get_definingDeclaration());
            // ROSE_ASSERT(enumDeclaration_defining != NULL);
               printf ("enumDeclaration_defining    = %p \n",enumDeclaration_defining);
               if (enumDeclaration_defining != NULL)
                  {
                    printf ("enumDeclaration_defining    = %p parent = %p \n",enumDeclaration_defining,enumDeclaration_defining->get_parent());
                    printf ("enumDeclaration_defining->get_isAutonomousDeclaration() = %s \n",enumDeclaration_defining->get_isAutonomousDeclaration() ? "true" : "false");

                    printf ("enumDeclaration_defining                                                  = %p \n",enumDeclaration_defining);
                    printf ("enumDeclaration_defining->get_baseTypeDefiningDeclaration()->get_parent() = %p \n",enumDeclaration_defining->get_baseTypeDefiningDeclaration()->get_parent());

                 // if (decl->get_baseTypeDefiningDeclaration()->get_parent() != NULL && decl->get_baseTypeDefiningDeclaration()->get_parent() != enumDeclaration_defining)
                    if (decl->get_baseTypeDefiningDeclaration()->get_parent() != enumDeclaration_defining)
                       {
                         printf ("This type declaration is refering to a parent that is different from the current variable declaration (reset baseTypeDefiningDeclaration to NULL) \n");
                      // ROSE_ASSERT(false);

                         decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(false);
                         decl->set_baseTypeDefiningDeclaration(NULL);
                       }
                  }
#endif
#if 1
            // if (decl->get_baseTypeDefiningDeclaration()->get_parent() != NULL && decl->get_baseTypeDefiningDeclaration()->get_parent() != decl)
               if (decl->get_baseTypeDefiningDeclaration() != NULL && 
                   decl->get_baseTypeDefiningDeclaration()->get_parent() != NULL && 
                   decl->get_baseTypeDefiningDeclaration()->get_parent() != decl)
                  {
#if 0
                    printf ("This type declaration is refering to a parent that is different from the current variable declaration (reset baseTypeDefiningDeclaration to NULL) \n");
#endif
                    decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(false);
                    decl->set_baseTypeDefiningDeclaration(NULL);
                  }
#endif
             }
        }

  // DQ (10/21/2012): Is this something we can assert?
     ROSE_ASSERT ( (unnamed_type == NULL) || ( (unnamed_type != NULL) && (types_with_emitted_definitions.find(unnamed_type) != types_with_emitted_definitions.end()) ) );

#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
     printf ("In fixup_variable_or_field_declaration(): decl = %p = %s \n",decl,decl->class_name().c_str());
     printf ("In fixup_variable_or_field_declaration(): decl->get_parent() = %p \n",decl->get_parent());
     printf ("In fixup_variable_or_field_declaration(): decl->get_baseTypeDefiningDeclaration() = %p \n",decl->get_baseTypeDefiningDeclaration());
#endif

  // DQ (10/21/2012): Refactored support to handle both the variable/field and also the typedef cases.
     fixup_variable_field_typedef_support(decl,type);

#if 0
     decl->get_file_info()->display("BOTTOM of fixup_variable_or_field_declaration(): debug");
#endif
#if DEBUG_FIXUP_VARIABLE_OR_FIELD_DECLARATION
     printf ("Leaving fixup_variable_or_field_declaration(): decl = %p decl->get_variableDeclarationContainsBaseTypeDefiningDeclaration() = %s \n",
          decl,decl->get_variableDeclarationContainsBaseTypeDefiningDeclaration() ? "true" : "false");
  // printf ("Leaving fixup_variable_or_field_declaration(): decl = %p decl->get_parent() = %p \n",decl,decl->get_parent());
#endif
   }

void fixup_variable_field_typedef_support(SgDeclarationStatement* decl, a_type_ptr type)
   {

#define DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT 0

  // DQ (6/19/2012): Adding support for types defined in the variable declaration.
  // if (types_with_emitted_definitions[type] == true || true)
     if (types_with_emitted_definitions[type] == false)
        {
       // test2012_66.c is a case where unnamed_type != NULL.
       // ROSE_ASSERT(unnamed_type == NULL);

       // printf ("THIS FUNCTION IS CHANGING THE TYPEDEF DECLARATION TO THE ONE FROM THE TYPE (should not be shared)! \n");

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif

#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
          printf ("In fixup_variable_field_typedef_support(): types_with_emitted_definitions[type = %p] = %s \n",type,types_with_emitted_definitions[type] ? "true" : "false");
#endif
          SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(decl);
          SgTypedefDeclaration*  typedefDeclaration  = isSgTypedefDeclaration(decl);

#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
          if (variableDeclaration != NULL)
             {
               printf ("In fixup_variable_field_typedef_support(): test 1: variableDeclaration->get_baseTypeDefiningDeclaration() = %p \n",
                    variableDeclaration->get_baseTypeDefiningDeclaration());
             }

          if (typedefDeclaration != NULL)
             {
               printf ("In fixup_variable_field_typedef_support(): test 1: typedefDeclaration->get_declaration()                 = %p \n",
                    typedefDeclaration->get_declaration());
               printf ("In fixup_variable_field_typedef_support(): test 1: typedefDeclaration->get_baseTypeDefiningDeclaration() = %p \n",
                    typedefDeclaration->get_baseTypeDefiningDeclaration());
             }
#endif

          SgType* rose_type = convert_type(type);
          ROSE_ASSERT(rose_type != NULL);

       // DQ (11/21/2015): If we are in a template declaration, and if the typedef's base type is a local 
       // nested class in the template class declaration, then the type will be a tk_template_param instead 
       // of a tk_class (or tk_struct, or tk_union).  In this case, the type unwinding must access the 
       // orig_nested_type in the tk_template_param object.
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
          printf ("In fixup_variable_field_typedef_support(): rose_type = %p = %s \n",rose_type,rose_type->class_name().c_str());
#endif
       // decl->set_baseTypeDefiningDeclaration(get_declaration_of_unnamed_type(rose_type));

       // DQ (12/29/2019): 
       // SgDeclarationStatement* associatedTypeDeclaration = get_declaration_of_unnamed_type(rose_type);
          SgDeclarationStatement* associatedTypeDeclaration = get_declaration_of_unnamed_type(rose_type);
#if 0
       // DQ (12/29/2019): Commented out to debug failing tests (see Cxx11_tests/tet2019_142.C).
          if (typedefDeclaration != NULL && typedefDeclaration->get_declaration() != NULL)
             {
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
               printf ("Reset associatedTypeDeclaration using typedefDeclaration->get_declaration() = %p \n",typedefDeclaration->get_declaration());
#endif
               associatedTypeDeclaration = typedefDeclaration->get_declaration();
             }
#endif
          if (variableDeclaration != NULL && variableDeclaration->get_baseTypeDefiningDeclaration() != NULL)
             {
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
               printf ("Reset associatedTypeDeclaration using variableDeclaration->get_declaration() = %p \n",variableDeclaration->get_baseTypeDefiningDeclaration());
#endif
               associatedTypeDeclaration = variableDeclaration->get_baseTypeDefiningDeclaration();
             }

#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
          printf ("In fixup_variable_field_typedef_support(): associatedTypeDeclaration = %p \n",associatedTypeDeclaration);
          if (associatedTypeDeclaration != NULL)
             {
               printf ("In fixup_variable_field_typedef_support(): associatedTypeDeclaration = %p = %s \n",associatedTypeDeclaration,associatedTypeDeclaration->class_name().c_str());
               if (associatedTypeDeclaration->get_definingDeclaration() != NULL)
                    printf ("In fixup_variable_field_typedef_support(): associatedTypeDeclaration->get_definingDeclaration() = %p = %s \n",
                         associatedTypeDeclaration->get_definingDeclaration(),associatedTypeDeclaration->get_definingDeclaration()->class_name().c_str());
                 else
                    printf ("There is not defining declaration so no declaration to is available to put into a type reference \n");
             }
#endif
       // ROSE_ASSERT(associatedTypeDeclaration != NULL);
       // if (associatedTypeDeclaration != NULL)
          if (associatedTypeDeclaration != NULL && associatedTypeDeclaration->get_definingDeclaration() != NULL)
             {

            // DQ (7/12/2012): Added as a test (OK if the named non-autonomous declaration is used a 2nd time.
            // ROSE_ASSERT(associatedTypeDeclaration->get_parent() == NULL);

#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
               printf ("In fixup_variable_field_typedef_support(): associatedTypeDeclaration = %p = %s \n",associatedTypeDeclaration,associatedTypeDeclaration->class_name().c_str());
               if (associatedTypeDeclaration->get_definingDeclaration() != NULL)
                    printf ("In fixup_variable_field_typedef_support(): associatedTypeDeclaration->get_definingDeclaration()         = %p = %s \n",
                         associatedTypeDeclaration->get_definingDeclaration(),associatedTypeDeclaration->get_definingDeclaration()->class_name().c_str());
               if (associatedTypeDeclaration->get_firstNondefiningDeclaration() != NULL)
                    printf ("In fixup_variable_field_typedef_support(): associatedTypeDeclaration->get_firstNondefiningDeclaration() = %p = %s \n",
                    associatedTypeDeclaration->get_firstNondefiningDeclaration(),associatedTypeDeclaration->get_firstNondefiningDeclaration()->class_name().c_str());
               printf ("In fixup_variable_field_typedef_support(): associatedTypeDeclaration->get_parent() = %p \n",associatedTypeDeclaration->get_parent());
               if (associatedTypeDeclaration->get_parent() != NULL)
                    printf ("In fixup_variable_field_typedef_support(): associatedTypeDeclaration->get_parent() = %s \n",associatedTypeDeclaration->get_parent()->class_name().c_str());
               printf ("In fixup_variable_field_typedef_support(): type->autonomous_primary_tag_decl = %s \n",type->autonomous_primary_tag_decl ? "true" : "false");
#endif

            // DQ (7/12/2012): We need to set the parent of the declaration to the variable declaration so that only the matching instance (of the type declaration) will trigger the unparsing?).
            // This causes test2003_08.C to fail now (though test2003_17. C now passes)!!! 
            // The fix is to only reset if this parent is not set and to force the SgEnumDeclaration to not be set prematurely.
            // DQ (7/12/2012): I think we could set the parent even if it was already set, or have it be an error to have had it already set in this case of an embedded type declaration.
            // if (type->autonomous_primary_tag_decl == false && associatedTypeDeclaration->get_parent() == NULL)
            // if (type->autonomous_primary_tag_decl == false)
               if (type->autonomous_primary_tag_decl == false && associatedTypeDeclaration->get_parent() == NULL)
                  {
#if 0
                    printf ("In fixup_variable_field_typedef_support(): Skip setting the parent to decl = %p = %s \n",decl,decl->class_name().c_str());
#endif
#if 0
                    associatedTypeDeclaration->set_parent(decl);
#endif
                 // printf ("In parse_structlike(): Mark declaration as AUTONOMOUS \n");
                 // decl->set_isAutonomousDeclaration(true);
                  }

            // DQ (6/19/2012): I think this should be non-null.
            // ROSE_ASSERT(associatedTypeDeclaration->get_parent() != NULL);

            // DQ (10/21/2012): Added support for typedef (see test2010_08.c).
               SgClassDeclaration*   associatedClassDeclaration   = isSgClassDeclaration(associatedTypeDeclaration);
               SgEnumDeclaration*    associatedEnumDeclaration    = isSgEnumDeclaration(associatedTypeDeclaration);
               SgTypedefDeclaration* associatedTypedefDeclaration = isSgTypedefDeclaration(associatedTypeDeclaration);
            // ROSE_ASSERT(associatedClassDeclaration != NULL || associatedEnumDeclaration != NULL);
               ROSE_ASSERT(associatedClassDeclaration != NULL || associatedEnumDeclaration != NULL || associatedTypedefDeclaration != NULL);
#if 0
               printf ("type->autonomous_primary_tag_decl = %s \n",type->autonomous_primary_tag_decl ? "true" : "false");
#endif
#if 0
               printf ("In fixup_variable_field_typedef_support(): calling summarize_type() \n");
               summarize_type(type);
#endif
            // DQ (6/19/2012): Check if this is a declaration that matches the parent (see test2012_108.C).
            // if (decl->get_baseTypeDefiningDeclaration() == decl->get_parent())
            // if (associatedDeclaration == decl->get_parent())
            // if (decl == associatedTypeDeclaration->get_parent())
            // if (type->autonomous_primary_tag_decl == false)

            // If this is an un-named type then variableDeclarationContainsBaseTypeDefiningDeclaration 
            // if (type->autonomous_primary_tag_decl == false && type->source_corresp.name != NULL)
            // bool setVariableDeclarationContainsBaseTypeDefiningDeclaration = (type->autonomous_primary_tag_decl == false) && (type->source_corresp.name != NULL);
               bool setVariableDeclarationContainsBaseTypeDefiningDeclaration = (type->autonomous_primary_tag_decl == false);
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
               printf ("Initial setting: setVariableDeclarationContainsBaseTypeDefiningDeclaration = %s \n",setVariableDeclarationContainsBaseTypeDefiningDeclaration ? "true" : "false");
#endif
            // Check for an un-named type: See test2012_63.c and test2012_65.c.
               if ( (setVariableDeclarationContainsBaseTypeDefiningDeclaration == true) && (type->source_corresp.name == NULL) )
                  {
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
                    printf ("RESET: If this is an un-named declaration then setVariableDeclarationContainsBaseTypeDefiningDeclaration == true \n");
#endif
                 // setVariableDeclarationContainsBaseTypeDefiningDeclaration = false;
                    setVariableDeclarationContainsBaseTypeDefiningDeclaration = true;
                  }
#if 1
            // DQ (10/21/2012): I don't think we want to default to the case of template instantiation declarations being output as nested in declarations.
            // Check for a SgTemplateInstantiationDecl, See test2012_237.c. I don't thing that template instantations appear in variable/field 
            // declarations and typedef declarations.  If it can happen then in seems like a strange construction of code that would have to be
            // a template specialization in a variable/field declaration and typedef declaration.  Need a test code for this if it is issue.
               if ( (setVariableDeclarationContainsBaseTypeDefiningDeclaration == true) && (isSgTemplateInstantiationDecl(associatedClassDeclaration) != NULL) )
                  {
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
                    printf ("RESET: If this is a SgTemplateInstantiationDecl then setVariableDeclarationContainsBaseTypeDefiningDeclaration == false \n");
#endif
                    setVariableDeclarationContainsBaseTypeDefiningDeclaration = false;
                  }
#endif
            // DQ (11/21/2015): This needs to apply to classes and unions, not just structs.
            // DQ (4/3/2014): If this is a nonreal_class then reset.
            // if ( (setVariableDeclarationContainsBaseTypeDefiningDeclaration == true) &&
            //      (type->kind == tk_struct && type->variant.class_struct_union.is_nonreal_class == true) )
               if ( (setVariableDeclarationContainsBaseTypeDefiningDeclaration == true) &&
                    ( (type->kind == tk_struct || type->kind == tk_class || type->kind == tk_union) && 
                      type->variant.class_struct_union.is_nonreal_class == true) )
                  {
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
                    printf ("RESET: If this is a nonreal class then setVariableDeclarationContainsBaseTypeDefiningDeclaration == false \n");
#endif
                    setVariableDeclarationContainsBaseTypeDefiningDeclaration = false;
                  }
#if 0
               if (associatedClassDeclaration != NULL)
                  {
                    printf ("In fixup_variable_field_typedef_support(): associatedClassDeclaration = %p = %s \n",associatedClassDeclaration,associatedClassDeclaration->class_name().c_str());
                    printf ("   --- associatedClassDeclaration->get_isAutonomousDeclaration() = %s \n",associatedClassDeclaration->get_isAutonomousDeclaration() ? "true" : "false");
                  }
#endif
#if 0
            // DQ (11/21/2015): If this is not an autonomous class (i.e. class X {} x;) then it can't be defined in the current variable decleation, typedef declaration, or enum declaration.
               if (associatedClassDeclaration != NULL && associatedClassDeclaration->get_isAutonomousDeclaration() == false)
                  {
#if 0
                    printf ("RESET: If this is a NOT an autonomous class declaration then setVariableDeclarationContainsBaseTypeDefiningDeclaration == false \n");
#endif
                    setVariableDeclarationContainsBaseTypeDefiningDeclaration = false;
                  }
#endif
            // DQ (11/21/2015): This is the case where a typedef's base type in a template class declaration is a nested class type.
            // EDG records this type as a tk_template_param and we have to dig further for the pointer to the original class.
               if (type->kind == tk_template_param && type->variant.template_param.extra_info != NULL && type->variant.template_param.extra_info->orig_nested_type != NULL)
                  {
                 // I think this would be the better type to use if possible. It generates a SgClassType 
                 // with reference to the associated SgTemplateClassDeclaration which is require to get 
                 // the name qualification correct.
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
                    printf ("RESET: type->variant.template_param.extra_info->orig_nested_type != NULL: setVariableDeclarationContainsBaseTypeDefiningDeclaration == false \n");
#endif
                    setVariableDeclarationContainsBaseTypeDefiningDeclaration = false;
                  }
                 else
                  {
                    if (type->kind == tk_template_param && type->variant.template_param.extra_info != NULL && type->variant.template_param.extra_info->class_type != NULL)
                       {
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
                         printf ("RESET: type->variant.template_param.extra_info->class_type != NULL: setVariableDeclarationContainsBaseTypeDefiningDeclaration == false \n");
#endif
                      // Catch this case.
                      // ROSE_ASSERT(type->variant.template_param.extra_info->class_type == NULL);
                         setVariableDeclarationContainsBaseTypeDefiningDeclaration = false;
                       }
                  }

#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
               printf ("In fixup_variable_field_typedef_support(): Final setting: setVariableDeclarationContainsBaseTypeDefiningDeclaration = %s \n",setVariableDeclarationContainsBaseTypeDefiningDeclaration ? "true" : "false");
#endif

               if (setVariableDeclarationContainsBaseTypeDefiningDeclaration == true)
                  {
                 // decl->set_baseTypeDefiningDeclaration(get_declaration_of_unnamed_type(rose_type));
                 // DQ (7/12/2012): Use this simpler code that is independent of SgClassDeclaration or SgEnumDeclaration
                 // if (associatedTypeDeclaration->get_parent() == decl)
                       {
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
                         printf ("In fixup_variable_field_typedef_support(): Setting variableDeclarationContainsBaseTypeDefiningDeclaration() to TRUE decl = %p \n",decl);
#endif
                         if (variableDeclaration != NULL)
                            {
                              variableDeclaration->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
                              variableDeclaration->set_baseTypeDefiningDeclaration(associatedTypeDeclaration);
                            }
                           else
                            {
                              ROSE_ASSERT(typedefDeclaration != NULL);
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
                              printf ("In fixup_variable_field_typedef_support(): Setting typedefBaseTypeContainsDefiningDeclaration == true \n");
#endif
                              typedefDeclaration->set_typedefBaseTypeContainsDefiningDeclaration(true);
                              typedefDeclaration->set_declaration(associatedTypeDeclaration);
                            }

#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
                         printf ("Setting types_with_emitted_definitions[type = %p] to TRUE for decl = %p \n",type,decl);
#endif
                      // This will prevent this type from being considered for possible output as a defining declaration inside of a reference to a type ever again.
                         types_with_emitted_definitions[type] = true;

                      // Make sure that we don't have to set (reset) the value of isAutonomousDeclaration in either the class or enum declaration.
                         if (associatedClassDeclaration != NULL)
                            {
                           // DQ (10/21/2012): commented out this assertion.
                              if (associatedClassDeclaration->get_isAutonomousDeclaration() == true)
                                 {
#ifdef DEBUG_CONVERSION
                                   printf ("WARNING: Expected associatedClassDeclaration->get_isAutonomousDeclaration() == false \n");
#endif
                                 }
                           // ROSE_ASSERT(associatedClassDeclaration->get_isAutonomousDeclaration() == false);
                            }
                           else
                            {
                              ROSE_ASSERT(associatedEnumDeclaration != NULL);

                           // DQ (9/11/2014): This is an assertion failing on sources/utils/AlignedPool.cxx
                              if (associatedEnumDeclaration->get_isAutonomousDeclaration() == true)
                                 {
				   printf("Warning: In fixup_variable_field_typedef_support(): associatedEnumDeclaration->get_isAutonomousDeclaration() == true \n");
                                 }
                           // ROSE_ASSERT(associatedEnumDeclaration->get_isAutonomousDeclaration() == false);
                            }
                       }
                  }
                 else
                  {
                  // DQ (10/21/2012): We can ignore the default case.
#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
                    printf ("Setting variableDeclarationContainsBaseTypeDefiningDeclaration() to FALSE (the default setting) decl = %p \n",decl);
#endif
#if 0
                    decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(false);
                    ROSE_ASSERT(decl->get_variableDeclarationContainsBaseTypeDefiningDeclaration() == false);

                    ROSE_ASSERT(decl->get_baseTypeDefiningDeclaration() == NULL);

                 // Make sure that we don't have to set (reset) the value of isAutonomousDeclaration in either the class or enum declaration.
                    if (associatedClassDeclaration != NULL)
                       {
                      // ROSE_ASSERT(associatedClassDeclaration->get_isAutonomousDeclaration() == true);
                         ROSE_ASSERT((associatedClassDeclaration->get_isAutonomousDeclaration() == true) || (type->source_corresp.name == NULL));
                       }
                      else
                       {
                         ROSE_ASSERT(associatedEnumDeclaration != NULL);
                         if (associatedEnumDeclaration->get_isAutonomousDeclaration() == false)
                            {
#if 0
                              printf ("In fixup_variable_field_typedef_support(): calling summarize_type() \n");
                              summarize_type(type);
#endif
                            }

                      // DQ (8/31/2012): This is a problem for running on parts of ROSE (in the system header files).
#ifdef DEBUG_CONVERSION
                         printf ("In fixup_variable_field_typedef_support(): Commented out assertion of associatedEnumDeclaration->get_isAutonomousDeclaration() == true \n");
#endif
                      // ROSE_ASSERT(associatedEnumDeclaration->get_isAutonomousDeclaration() == true);
                       }
#endif
                  }
             }

#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
          if (variableDeclaration != NULL)
             {
               printf ("In fixup_variable_field_typedef_support(): variableDeclaration = %p variableDeclaration->get_variableDeclarationContainsBaseTypeDefiningDeclaration() = %s \n",
                    variableDeclaration,variableDeclaration->get_variableDeclarationContainsBaseTypeDefiningDeclaration() ? "true" : "false");
             }
            else
             {
               ROSE_ASSERT(typedefDeclaration != NULL);
               printf ("In fixup_variable_field_typedef_support(): typedefDeclaration = %p typedefDeclaration->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",
                    typedefDeclaration,typedefDeclaration->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
             }
#endif
        }
       else
        {
#if 0
          printf ("In fixup_variable_field_typedef_support(): types_with_emitted_definitions[type] == true \n");
#endif
        }

#if DEBUG_FIXUP_VARIABLE_FIELD_TYPEDEF_SUPPORT
     SgVariableDeclaration* tmp_variableDeclaration = isSgVariableDeclaration(decl);
     SgTypedefDeclaration*  tmp_typedefDeclaration  = isSgTypedefDeclaration(decl);
     if (tmp_variableDeclaration != NULL)
        {
          printf ("Leaving fixup_variable_field_typedef_support(): tmp_variableDeclaration->get_baseTypeDefiningDeclaration() = %p \n",
               tmp_variableDeclaration->get_baseTypeDefiningDeclaration());
        }

     if (tmp_typedefDeclaration != NULL)
        {
          printf ("Leaving fixup_variable_field_typedef_support(): tmp_typedefDeclaration->get_declaration()                 = %p \n",
               tmp_typedefDeclaration->get_declaration());
          printf ("Leaving fixup_variable_field_typedef_support(): tmp_typedefDeclaration->get_baseTypeDefiningDeclaration() = %p \n",
               tmp_typedefDeclaration->get_baseTypeDefiningDeclaration());
        }
#endif
   }

void fixup_typedef_declaration(SgTypedefDeclaration* decl, a_type_ptr type)
   {
  // DQ (11/21/2015): Comment: The type parameter is the base type of the typedef, not the typedef type.

     ROSE_ASSERT(type != NULL);
     ROSE_ASSERT(decl != NULL);

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("fixup typedef declaration "); print_source_corresp(type->source_corresp); debug_print("\n");
#endif

#if 0
     printf ("TOP of fixup_typedef_declaration() decl = %p type = %p type->kind = %s \n",decl,type,summarize_type_kind_in_string(type));
     printf ("TOP of fixup_typedef_declaration(): decl = %p decl->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",
          decl,decl->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

  // DQ (7/16/2012): If this is an array or pointer (or tk_typeref), then we have to strip this away to find the base type.
     type = getBaseTypeOfEdgArrayOrPointerType(type);

#if 0
     printf ("In fixup_typedef_declaration() after computing the base type type = %p type->kind = %s \n",type,summarize_type_kind_in_string(type));
#endif

     a_type_ptr unnamed_type = contains_unnamed_type_at_top_level(type);

#if 0
     printf ("!!!!!!!!!!!!!!!!!!! In fixup_typedef_declaration(): unnamed_type = %p \n",unnamed_type);
#endif

#if 0
  // DQ (2/27/2012): I think this is the fix to make sure that declarations in typedefs are 
  // set with the correct parent.  Otherwise it will remain set to the value of get_scope().
     SgDeclarationStatement* associatedDeclaration = decl->get_declaration();
     printf ("In fixup_typedef_declaration(): associatedDeclaration = %p \n",associatedDeclaration);
     if (associatedDeclaration != NULL)
        {
          printf ("associatedDeclaration->get_parent() = %p \n",associatedDeclaration->get_parent());
        }
#endif

#if 0
     if ( (unnamed_type != NULL) && (types_with_emitted_definitions.find(unnamed_type) == types_with_emitted_definitions.end()) )
        {
          printf ("unnamed_type = %p kind = %s is NOT in types_with_emitted_definitions map \n",unnamed_type,summarize_type_kind_in_string(unnamed_type));
        }

     printf ("types_with_emitted_definitions[unnamed_type = %p] = %s \n",unnamed_type,types_with_emitted_definitions[unnamed_type] ? "true" : "false");
#endif

     if (unnamed_type && types_with_emitted_definitions[unnamed_type] == false)
        {
#if 0
          printf ("!!!!!!!!!!!!!!!!!! In fixup_typedef_declaration(): Setting the declaration in the typedef \n");
#endif
          SgType* rose_type = convert_type(unnamed_type);
          decl->set_typedefBaseTypeContainsDefiningDeclaration(true);
          decl->set_declaration(get_declaration_of_unnamed_type(rose_type));
          types_with_emitted_definitions[unnamed_type] = true;
#if 0
          SgDeclarationStatement* associatedDeclaration = decl->get_declaration();
          printf ("In fixup_typedef_declaration(): associatedDeclaration = %p \n",associatedDeclaration);

       // DQ (2/27/2012): I think this is the fix to make sure that declarations in typedefs are 
       // set with the correct parent.  Otherwise it will remain set to the value of get_scope().
          associatedDeclaration->set_parent(decl);
          ROSE_ASSERT(associatedDeclaration->get_firstNondefiningDeclaration() != NULL);
          associatedDeclaration->get_firstNondefiningDeclaration()->set_parent(decl);
#endif
        }
       else
        {
#if 0
          printf ("In fixup_typedef_declaration(): empty else case in the fixup of the declaration in the typedef \n");
#endif
        }

#if 0
     printf ("In fixup_typedef_declaration(): BEFORE fixup_variable_field_typedef_support(): decl->get_declaration() = %p \n",decl->get_declaration());
#endif

  // DQ (10/21/2012): Refactored support to handled both the variable/field and also the typedef cases.
     fixup_variable_field_typedef_support(decl,type);

#if 0
     printf ("In fixup_typedef_declaration(): AFTER fixup_variable_field_typedef_support(): decl->get_declaration() = %p \n",decl->get_declaration());
#endif

#if 0
     printf ("Middle of fixup_typedef_declaration(): decl = %p type = %p type->kind = %s \n",decl,type,summarize_type_kind_in_string(type));
     printf ("Middle of fixup_typedef_declaration(): decl = %p decl->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",
          decl,decl->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif

  // DQ (9/21/2013): Check the parent of any possible associated declaration.
  // SgDeclarationStatement* baseTypeDeclaration = decl->get_typedefBaseTypeContainsDefiningDeclaration();
     SgDeclarationStatement* baseTypeDeclaration = decl->get_declaration();
#if 0
     printf ("In fixup_typedef_declaration(): baseTypeDeclaration = %p \n",baseTypeDeclaration);
#endif
     if (baseTypeDeclaration != NULL)
        {
       // If this has an associated declaration then we have to set the parent of the declaration.
       // ROSE_ASSERT(baseTypeDeclaration->get_parent() != NULL);
       // if (baseTypeDeclaration->get_parent() == NULL || baseTypeDeclaration->get_parent() != decl)

#if 0
          printf ("In fixup_typedef_declaration(): baseTypeDeclaration = %p = %s \n",baseTypeDeclaration,baseTypeDeclaration->class_name().c_str());
#endif

          if (baseTypeDeclaration->get_parent() == NULL)
             {
#if 0
               printf ("In fixup_typedef_declaration(): (parent not set): Setting the baseTypeDeclaration parent to point to the typedef declaration = %p \n",decl);
#endif
               baseTypeDeclaration->set_parent(decl);
             }
            else
             {
#if 0
               printf ("decl = %p baseTypeDeclaration->get_parent() = %p = %s \n",decl,baseTypeDeclaration->get_parent(),baseTypeDeclaration->get_parent()->class_name().c_str());
#endif
            // if (baseTypeDeclaration->get_parent() != decl)
               if (!isSgLambdaExp(baseTypeDeclaration->get_parent()) && baseTypeDeclaration->get_parent() != decl && !isSgNonrealDecl(baseTypeDeclaration))
                  {
#if 0
                    printf ("In fixup_typedef_declaration(): Resetting the baseTypeDeclaration parent to point to the typedef declaration = %p \n",decl);
#endif
                    baseTypeDeclaration->set_parent(decl);
                  }
             }

          ROSE_ASSERT(baseTypeDeclaration->get_parent() != NULL);
          ROSE_ASSERT(isSgNonrealDecl(baseTypeDeclaration) || isSgLambdaExp(baseTypeDeclaration->get_parent()) || baseTypeDeclaration->get_parent() == decl);
        }

#if 0
     printf ("Leaving fixup_typedef_declaration(): decl = %p decl->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",decl,decl->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
     printf (" --- decl->get_declaration()                                = %p \n",decl->get_declaration());
     printf (" --- decl->get_typedefBaseTypeContainsDefiningDeclaration() = %s \n",decl->get_typedefBaseTypeContainsDefiningDeclaration() ? "true" : "false");
#endif
   }

void fixupSubTreeSourcePositions(SgDeclarationStatement* declaration)
   {
  // When we update the source positions of some constructs we have to update a few of there children.
  // An example of this is the SgFunctionParameterList is a SgFunctionDeclaration.  There are
  // likely a few more examples of this as well that have yet to be implemented.

#if 0
     printf ("In fixupSubTreeSourcePositions(): declaration = %p = %s \n",declaration,declaration->class_name().c_str());
#endif

     SgFunctionDeclaration* functionDeclaration = isSgFunctionDeclaration(declaration);
     if (functionDeclaration != NULL)
        {
          SgFunctionParameterList* functionParameterList = functionDeclaration->get_parameterList();
          ROSE_ASSERT(functionParameterList != NULL);
#if 0
          declaration->get_startOfConstruct()->display("In fixupSubTreeSourcePositions(): declaration: startOfConstruct debug");
          declaration->get_endOfConstruct  ()->display("In fixupSubTreeSourcePositions(): declaration: endOfConstruct debug");
          functionParameterList->get_startOfConstruct()->display("In fixupSubTreeSourcePositions(): functionParameterList: startOfConstruct debug");
          functionParameterList->get_endOfConstruct  ()->display("In fixupSubTreeSourcePositions(): functionParameterList: endOfConstruct debug");
#endif
          Sg_File_Info & functionDeclarationSourcePositionStart = *(declaration->get_startOfConstruct());
          Sg_File_Info & functionDeclarationSourcePositionEnd   = *(declaration->get_endOfConstruct());

          Sg_File_Info & functionParameterListSourcePositionStart = *(functionParameterList->get_startOfConstruct());
          Sg_File_Info & functionParameterListSourcePositionEnd   = *(functionParameterList->get_endOfConstruct());

          if (functionDeclarationSourcePositionStart.isSameFile(functionParameterListSourcePositionStart) == true)
             {
            // DQ (7/30/2012): This fails for test2004_120.C
#ifdef DEBUG_CONVERSION
               if (functionDeclarationSourcePositionEnd.isSameFile(functionParameterListSourcePositionEnd) == false)
                  {
                    printf ("WARNING: functionDeclarationSourcePositionEnd is not the same file as functionParameterListSourcePositionEnd \n");
                    declaration->get_endOfConstruct  ()->display("In fixupSubTreeSourcePositions(): declaration: endOfConstruct debug");
                    functionParameterList->get_endOfConstruct  ()->display("In fixupSubTreeSourcePositions(): functionParameterList: endOfConstruct debug");
                  }
#endif
            // ROSE_ASSERT(functionDeclarationSourcePositionEnd.isSameFile(functionParameterListSourcePositionEnd) == true);

               if (functionParameterListSourcePositionStart > functionDeclarationSourcePositionStart ||
                   functionParameterListSourcePositionEnd > functionDeclarationSourcePositionEnd)
                  {
                    functionParameterListSourcePositionStart = functionDeclarationSourcePositionStart;
                    functionParameterListSourcePositionEnd   = functionDeclarationSourcePositionEnd;

                    ROSE_ASSERT(functionDeclarationSourcePositionStart == functionParameterListSourcePositionStart);
                    ROSE_ASSERT(functionDeclarationSourcePositionEnd   == functionParameterListSourcePositionEnd);
#if 0
                    printf ("Error: need to fixup subtree functionParameterList \n");
                    ROSE_ASSERT(false);
#endif
                  }
             }
            else
             {
#ifdef DEBUG_CONVERSION
               printf ("Note: These are not in the same files so the source positions are not comparable \n");
#endif
             }
#if 0
          functionParameterList->get_startOfConstruct()->display("Leaving fixupSubTreeSourcePositions(): functionParameterList: startOfConstruct debug");
          functionParameterList->get_endOfConstruct()->display("Leaving fixupSubTreeSourcePositions(): functionParameterList: endOfConstruct debug");
#endif
        }
       else
        {
          SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(declaration);
          if (variableDeclaration != NULL)
             {
#if 0
               printf ("##### WARNING: In fixupSubTreeSourcePositions(): case of SgVariableDeclaration: must update the child SgInitializedName IR node source positions! \n");
#endif
               Sg_File_Info & variableDeclarationSourcePositionStart = *(declaration->get_startOfConstruct());
               Sg_File_Info & variableDeclarationSourcePositionEnd   = *(declaration->get_endOfConstruct());

               SgInitializedNamePtrList & variables = variableDeclaration->get_variables();
               SgInitializedNamePtrList::iterator i = variables.begin();
               while (i != variables.end())
                  {
                    Sg_File_Info & variableSourcePositionStart = *((*i)->get_startOfConstruct());
                    Sg_File_Info & variableSourcePositionEnd   = *((*i)->get_endOfConstruct());
#if 0
                    variableSourcePositionStart.display("reset source position of SgInitializedName(START): BEFORE");
                    variableSourcePositionEnd  .display("reset source position of SgInitializedName(END)  : BEFORE");
#endif
                 // This is a call to the assignment operator (deep copy, instead of a pointer copy).
                    variableSourcePositionStart = variableDeclarationSourcePositionStart;
                    variableSourcePositionEnd   = variableDeclarationSourcePositionEnd;
#if 0
                    variableSourcePositionStart.display("reset source position of SgInitializedName(START): AFTER");
                    variableSourcePositionEnd  .display("reset source position of SgInitializedName(END)  : AFTER");
#endif
                    i++;
                  }
             }
            else
             {
#ifdef DEBUG_CONVERSION
               printf ("WARNING: In fixupSubTreeSourcePositions(): declaration subtree not checked: declaration = %p = %s \n",declaration,declaration->class_name().c_str());
#endif
             }
        }

     SgNamespaceDeclarationStatement* namespaceDeclaration = isSgNamespaceDeclarationStatement(declaration);
     if (namespaceDeclaration != NULL)
        {
#if 0
          printf ("In fixupSubTreeSourcePositions(): namespaceDeclaration = %p name = %s at (%d,%d) -> (%d,%d) \n",
               namespaceDeclaration,namespaceDeclaration->get_name().str(),
               namespaceDeclaration->get_startOfConstruct()->get_line(),namespaceDeclaration->get_startOfConstruct()->get_col(),
               namespaceDeclaration->get_endOfConstruct()->get_line(),namespaceDeclaration->get_endOfConstruct()->get_col());
#endif
          SgNamespaceDefinitionStatement* namespaceDefinition = namespaceDeclaration->get_definition();
          ROSE_ASSERT(namespaceDefinition != NULL);
#if 0
          printf ("   --- namespaceDefinition at (%d,%d) -> (%d,%d) \n",
               namespaceDefinition->get_startOfConstruct()->get_line(),namespaceDefinition->get_startOfConstruct()->get_col(),
               namespaceDefinition->get_endOfConstruct()->get_line(),namespaceDefinition->get_endOfConstruct()->get_col());

#endif
#if 0
          printf ("In fixupSubTreeSourcePositions(): Set the namespaceDeclaration->get_endOfConstruct() to that of the namespaceDefinition->get_endOfConstruct() \n");
#endif

       // DQ (12/18/2014): Set the end of the SgNamespaceDefinitionStatement to be that of the end of the SgNamespaceDefinitionStatement 
       // plus one (so that we get a position after the namespace name and not the last character of the namespace name).
          *(namespaceDefinition->get_startOfConstruct()) = *(namespaceDeclaration->get_endOfConstruct());
          namespaceDefinition->get_startOfConstruct()->set_col(namespaceDefinition->get_startOfConstruct()->get_col()+1);

       // DQ (12/17/2014): Set the end of the SgNamespaceDeclarationStatement to be that of the SgNamespaceDefinitionStatement
          *(namespaceDeclaration->get_endOfConstruct()) = *(namespaceDefinition->get_endOfConstruct());
        }
   }

#define DEBUG_FIXUP_CASE_STATEMENTS 0

void fixup_case_statements( vector<SgStatement*> & result )
   {
  // DQ (1/2/2018): Fixup interleaved case statements into blocks to be proper compound statements.

#if DEBUG_FIXUP_CASE_STATEMENTS
     printf ("@@@@@@@@@@@@@@@@ In fixup_case_statements(): before processing case statements: \n");
     for (size_t i = 0; i < result.size(); i++)
        {
          printf ("   --- result[%zu] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
        }
#endif
     bool containsLabelStatement = false;

     size_t numberOfCaseStatements = 0;
     for (size_t i = 0; i < result.size(); i++)
        {
          if (result[i] != NULL)
             {
#if DEBUG_FIXUP_CASE_STATEMENTS
               printf ("process case option statement: result[%zu] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
#endif
            // DQ (1/6/2018): test2018_04.C demonstrates why we need to post process lable statements.
            // The issue is that this test will fail in the normalization tests (not for the identity translator).
               SgLabelStatement*    labelStatement         = isSgLabelStatement(result[i]);
               if (labelStatement != NULL)
                 {
                // This will trigger post processing of the label statements.
                   containsLabelStatement = true;
                 }

               SgCaseOptionStmt*    caseOptionStatement    = isSgCaseOptionStmt(result[i]);
               SgDefaultOptionStmt* defaultOptionStatement = isSgDefaultOptionStmt(result[i]);
               if (caseOptionStatement != NULL || defaultOptionStatement != NULL)
                  {
#if DEBUG_FIXUP_CASE_STATEMENTS
                    printf ("Found a case statement: i = %zu \n",i);
#endif
                    if (i+1 < result.size())
                       {
                         SgStatement* body_statement = result[i+1];
                         ROSE_ASSERT(body_statement != NULL);

#if DEBUG_FIXUP_CASE_STATEMENTS
                         printf ("body_statement = %p = %s \n",body_statement,body_statement->class_name().c_str());
#endif

                         labelStatement         = isSgLabelStatement(body_statement);
                         if (labelStatement != NULL)
                            {
#if DEBUG_FIXUP_CASE_STATEMENTS
                              printf ("Found label statement that will be used as body of case or default options statement \n");
#endif
                           // This will trigger post processing of the label statements (which we 
                           // be searched for in the result list and any case or default statements).
                              containsLabelStatement = true;
                            }

                         SgCaseOptionStmt*    body_case_option_statement    = isSgCaseOptionStmt(body_statement);
                         SgDefaultOptionStmt* body_default_option_statement = isSgDefaultOptionStmt(body_statement);
                         SgStatement*         body_another_case_or_default_option_statement = body_case_option_statement != NULL ? (SgStatement*)body_case_option_statement : (SgStatement*)body_default_option_statement;
                      // if (case_body_case_option_statement != NULL)
                         if (body_another_case_or_default_option_statement != NULL)
                            {
                           // No nothing (or perhaps add null statement to case option body).
#if DEBUG_FIXUP_CASE_STATEMENTS
                              printf ("Found case or default option statement followed by a case or default option statement (can't set body to next statement) \n");

                              printf ("body_another_case_or_default_option_statement = %p = %s \n",body_another_case_or_default_option_statement,body_another_case_or_default_option_statement->class_name().c_str());
#endif
                           // Reset the body to a NULL statement.
                              body_statement = buildNullStatement();
                              ROSE_ASSERT(body_statement != NULL);

                           // Mark this as compiler generated.
                              body_statement->setCompilerGenerated();
                            }
                           else
                            {
#if DEBUG_FIXUP_CASE_STATEMENTS
                              printf ("Set the body of result[%zu] = %p = %s to case_body_statement = %p = %s \n",i,result[i],result[i]->class_name().c_str(),body_statement,body_statement->class_name().c_str());
#endif
                              result[i+1] = NULL;

#if DEBUG_FIXUP_CASE_STATEMENTS
                              printf ("result[%zu] has been reset to NULL \n",i+1);
                              printf ("result[%zu] = %p \n",i+1,result[i+1]);
#endif
                              numberOfCaseStatements++;
                            }

                      // caseStatement->set_body(body_statement);
                         if (caseOptionStatement != NULL)
                            {
#if DEBUG_FIXUP_CASE_STATEMENTS
                              printf ("Setup the case option statement with a body: body_statement = %p = %s \n",body_statement,body_statement->class_name().c_str());
#endif
                              caseOptionStatement->set_body(body_statement);
                              body_statement->set_parent(caseOptionStatement);
                            }
                           else
                            {
                              if (defaultOptionStatement != NULL)
                                 {
#if DEBUG_FIXUP_CASE_STATEMENTS
                                   printf ("Setup the default option statement with a body: body_statement = %p = %s \n",body_statement,body_statement->class_name().c_str());
#endif
                                   defaultOptionStatement->set_body(body_statement);
                                   body_statement->set_parent(defaultOptionStatement);
                                 }
                                else
                                 {
                                   printf ("ERROR: result[%zu] is neither a SgCaseOptionStmt nor a SgDefaultOptionStmt \n",i);
                                   ROSE_ASSERT(false);
                                 }
                            }

                       }
                  }
             }
            else
             {
#if DEBUG_FIXUP_CASE_STATEMENTS
               printf ("process case option statement: result[%zu] = %p \n",i,result[i]);
#endif
             }
        }

#if DEBUG_FIXUP_CASE_STATEMENTS
     printf ("@@@@@@@@@@@@@@@@ In fixup_case_statements(): after setting case option bodies: \n");
     for (size_t i = 0; i < result.size(); i++)
        {
          if (result[i] != NULL)
             {
               printf ("   --- result[%zu] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
             }
            else
             {
               printf ("   --- result[%zu] = %p \n",i,result[i]);
             }
        }
#endif

     SgStatement* null_pointer = NULL;
     for (size_t i = 0; i < numberOfCaseStatements; i++)
        {
#if DEBUG_FIXUP_CASE_STATEMENTS
          printf ("Remove NULL entries in result: i = %zu \n",i);
#endif
          result.erase(find(result.begin(),result.end(),null_pointer));
        }

  // Reset to zero.
     numberOfCaseStatements = 0;

#if DEBUG_FIXUP_CASE_STATEMENTS
     printf ("@@@@@@@@@@@@@@@@ In fixup_case_statements(): before processing label statements: output the result vector of statements: \n");
     for (size_t i = 0; i < result.size(); i++)
        {
          ROSE_ASSERT(result[i] != NULL);
          printf ("   --- result[%zu] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
        }
#endif

#if DEBUG_FIXUP_CASE_STATEMENTS
     printf ("containsLabelStatement = %s \n",containsLabelStatement ? "true" : "false");
#endif

     if (containsLabelStatement == true)
        {
#if DEBUG_FIXUP_CASE_STATEMENTS
          printf ("process label statements \n");
#endif
          for (size_t i = 0; i < result.size(); i++)
             {
               if (result[i] != NULL)
                  {
#if DEBUG_FIXUP_CASE_STATEMENTS
                    printf ("   --- result[%zu] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
#endif
                    SgLabelStatement* labelStatement = isSgLabelStatement(result[i]);

                    SgCaseOptionStmt*    case_option_statement    = isSgCaseOptionStmt(result[i]);
                    SgDefaultOptionStmt* default_option_statement = isSgDefaultOptionStmt(result[i]);
                    if (case_option_statement != NULL || default_option_statement != NULL)
                       {
                         ROSE_ASSERT(labelStatement == NULL);
                         if (case_option_statement != NULL)
                            {
#if DEBUG_FIXUP_CASE_STATEMENTS
                              printf ("Looking for label statement in body of case option statement: i = %zu \n",i);
#endif
                              labelStatement = isSgLabelStatement(case_option_statement->get_body());
                            }
                           else
                            {
                              if (default_option_statement != NULL)
                                 {
#if DEBUG_FIXUP_CASE_STATEMENTS
                                   printf ("Looking for label statement in body of default option statement: i = %zu \n",i);
#endif
                                   labelStatement = isSgLabelStatement(default_option_statement->get_body());
                                 }
                                else
                                 {
                                   printf ("Error: this should have been either a case or default option statement! \n");
                                   ROSE_ASSERT(false);
                                 }
                            }
                       }

                    if (labelStatement != NULL)
                      {
#if DEBUG_FIXUP_CASE_STATEMENTS
                         printf ("Found a label statement: i = %zu \n",i);
#endif
                         if (i+1 < result.size())
                            {
                              SgStatement* body_statement = result[i+1];
                              ROSE_ASSERT(body_statement != NULL);

#if DEBUG_FIXUP_CASE_STATEMENTS
                              printf ("body_statement = %p = %s \n",body_statement,body_statement->class_name().c_str());
#endif
                           // SgLabelStatement* body_labelStatement = isSgLabelStatement(result[i]);
                              SgLabelStatement* body_labelStatement = isSgLabelStatement(body_statement);
                              if (body_labelStatement != NULL)
                                 {
                                // No nothing (or perhaps add null statement to label body).
                                // Note that we want to avoid an implementation that would result in chains of label statements.
#if DEBUG_FIXUP_CASE_STATEMENTS
                                   printf ("Found label statement followed by a label statement (can't set body to next statement) \n");

                                   printf ("body_labelStatement = %p = %s \n",body_labelStatement,body_labelStatement->class_name().c_str());
#endif
                                // Reset the body to a NULL statement.
                                   body_statement = buildNullStatement();
                                   ROSE_ASSERT(body_statement != NULL);

                                // Mark this as compiler generated.
                                   body_statement->setCompilerGenerated();
                                 }
                                else
                                 {
#if DEBUG_FIXUP_CASE_STATEMENTS
                                   printf ("Set the body of result[%zu] = %p = %s to body_statement = %p = %s \n",i,result[i],result[i]->class_name().c_str(),body_statement,body_statement->class_name().c_str());
#endif
                                   result[i+1] = NULL;

#if DEBUG_FIXUP_CASE_STATEMENTS
                                   printf ("result[%zu] has been reset to NULL \n",i+1);
                                   printf ("result[%zu] = %p \n",i+1,result[i+1]);
#endif
                                   numberOfCaseStatements++;
                                 }

                              labelStatement->set_statement(body_statement);
                              body_statement->set_parent(labelStatement);
                            }
                       }
                  }
                 else
                  {
#if DEBUG_FIXUP_CASE_STATEMENTS
                    printf ("   --- result[%zu] = %p \n",i,result[i]);
#endif
                  }
             }
        }

#if DEBUG_FIXUP_CASE_STATEMENTS
     printf ("@@@@@@@@@@@@@@@@ In fixup_case_statements(): after setting label bodies: \n");
     for (size_t i = 0; i < result.size(); i++)
        {
          if (result[i] != NULL)
             {
               printf ("   --- result[%zu] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
             }
            else
             {
               printf ("   --- result[%zu] = %p \n",i,result[i]);
             }
        }
#endif

#if DEBUG_FIXUP_CASE_STATEMENTS
     printf ("Before removing NULL entries: numberOfCaseStatements = %zu \n",numberOfCaseStatements);
#endif

  // SgStatement* null_pointer = NULL;
  // result.erase(find(result.begin(),result.end(),null_pointer));
     for (size_t i = 0; i < numberOfCaseStatements; i++)
        {
#if DEBUG_FIXUP_CASE_STATEMENTS
          printf ("Remove NULL entries in result: i = %zu \n",i);
#endif
          result.erase(find(result.begin(),result.end(),null_pointer));
        }

#if DEBUG_FIXUP_CASE_STATEMENTS
     printf ("@@@@@@@@@@@@@@@@ In fixup_case_statements(): output the result vector of statements: \n");
     for (size_t i = 0; i < result.size(); i++)
        {
          ROSE_ASSERT(result[i] != NULL);
          printf ("   --- result[%zu] = %p = %s \n",i,result[i],result[i]->class_name().c_str());
        }
#endif
   }

}
