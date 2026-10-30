
#include "edgRose.h"

namespace EDG_ROSE_Translation {

  Sawyer::Message::Facility mlog;

  std::map<a_type_ptr, SgType*> type_translation_cache;
  std::map<a_type_ptr, SgType*> template_type_translation_cache;
  std::map<a_type_ptr, std::vector<SgDeclarationStatement*> > type_declaration_map;
  std::map<a_type_ptr, SgType*> project_wide_type_translation_cache;

  std::map<a_routine_ptr, SgFunctionDeclaration*> first_nondefining_declarations_for_functions;
  std::map<a_routine_ptr, SgFunctionDeclaration*> defining_declarations_for_functions;
  std::map<a_routine_ptr, std::vector<SgDeclarationStatement*> > function_declaration_map;

  std::map<a_routine_ptr, string> function_name_cache;

  std::map<a_routine_ptr, SgFunctionDeclaration*> first_nondefining_declarations_for_template_functions;
  std::map<a_routine_ptr, SgFunctionDeclaration*> defining_declarations_for_template_functions;

  std::map<a_template_ptr, SgDeclarationStatement*> first_nondefining_declarations_for_templates;
  std::map<a_template_ptr, SgDeclarationStatement*> defining_declarations_for_templates;

  std::map<a_variable_ptr, SgVariableSymbol*> variable_cache;
  std::map<a_field_ptr, SgVariableSymbol*> field_cache;
  std::map<a_routine_ptr, SgSymbol*> function_cache;
  std::map<a_label_ptr, SgLabelStatement*> label_cache;

  std::map<a_type_ptr, bool> types_with_emitted_definitions;
  std::set<a_statement_ptr> statement_expression_bodies;

  std::map<a_scope_ptr, SgScopeStatement*> scope_cache;
  std::map<a_statement_ptr, SgScopeStatement*> statement_scope_cache;

  std::map<a_namespace_ptr, SgNamespaceDeclarationStatement*> namespace_cache;
  std::map<a_namespace_ptr, SgNamespaceAliasDeclarationStatement*> namespace_alias_cache;
  std::unordered_map<a_type_ptr, SgType*> typesWithoutDeduction;

  int indent = 0;

  SgSourceFile* currentSourceFile    = NULL;
  SgGlobal*     current_global_scope = NULL;

  std::vector<a_template_ptr> templateDeclarationStack;

  a_scope_ptr global_innermost_function_scope = NULL;

  std::vector<std::map<a_source_sequence_entry_ptr,SgStatement*>*> switchSupportStackOfMaps;

  std::set<a_type_ptr> outputTypesSet;

  std::map<a_type_ptr,      SgTypeOfType*> edg_type_to_rose_typeof_type_map;
  std::map<an_expr_node_ptr,SgTypeOfType*> edg_expression_to_rose_typeof_type_map;

  a_seq_number global_source_sequence_number = 0;

  SgDeclarationScope* internal_intermediate_scope = NULL;

  std::map<a_statement_ptr, SgStatementExpression*> statement_expression_cache;

  std::map<a_scope_ptr, SgDeclarationScope*> nonreal_scope_cache;
  std::map<a_template_ptr, SgDeclarationScope*> nonreal_scope_cache_from_template;
  std::vector<SgDeclarationScope *> nonreal_scope_for_template_parameters_stack;
  SgDeclarationScope * global_nonreal_scope = NULL;

  std::map<a_template_arg_ptr,SgTemplateArgument*> processedTemplateArgsCache;
  std::map<a_template_parameter_ptr,SgTemplateParameter*> processedTemplateParameterCache;

  bool global_call_uses_operator_syntax = false;
  bool global_call_uses_operator_names  = false;
  bool suppress_detection_of_transformations = false;

  std::map<std::string, SgIncludeFile*> edg_include_file_map;
  std::map<std::string, SgIncludeFile*> edg_include_file_using_names_as_written_map;

  SgNamespaceDeclarationStatement* global_std_namespace = NULL;
  SgNamespaceDeclarationStatement* global_cxxabiv1_namespace = NULL;

  std::map<a_template_ptr, SgFunctionDeclaration*> saved_defining_declarations_from_class_templates;

  std::map<a_switch_case_entry_ptr,SgStatement*> switch_case_map;

  bool global_suppress_variable_declaration_normalization = false;

  a_seq_number convert_typedef_last_source_sequence_number = -1;
  SgTypedefDeclaration* convert_typedef_last_decl          = NULL;

  a_seq_number parse_field_declaration_last_source_sequence_number = -1;
  SgVariableDeclaration* parse_field_declaration_last_decl         = NULL;

  a_seq_number parse_variable_declaration_last_source_sequence_number = -1;
  SgVariableDeclaration* parse_variable_declaration_last_decl         = NULL;
}

// DQ (1/23/2013): This is trick to make the value of EDG_ROSE_Translation::global_source_sequence_number available within debugging support (used in print_source_range()).
a_seq_number & global_source_sequence_number_reference = EDG_ROSE_Translation::global_source_sequence_number;

// DQ (11/15/2018): Adding support for testing the include file traversals.
namespace IncludeFileSupport
   {
     void headerFilePrefix ( SgIncludeFile* includeFile );
   }

extern "C" 
   {
  // void cp_gen_be_cleanup(void);

  // DQ (6/20/2012): We need this definition to compile ROSE with EDG (else a link error).
     void
     cp_gen_be_cleanup(void)
        {
#ifdef DEBUG_CONVERSION
          printf ("Inside of cp_gen_be_cleanup() \n");
#endif
        }
   }


// DQ (8/24/2012): Move this from header file to the source file.
// DQ (8/18/2012): Use the defined function debug_print() so that we can control the output.
#if USE_DEBUG_VERSION_OF_PRINTF
// static inline void debug_print(const char* format, ...) __attribute__((format(printf, 1, 2)));
// static inline void debug_print(const char* format, ...)
void debug_print(const char* format, ...)
   {
  // This is a custom printf like function to support debugging.  It don't know if I like
  // this approach, but it has the advantage that it can have it's output easily throttled.
  // But ultimately we want to throttle the calls to this function, not just the output from it.

  // This throttles the output from this function (note that I think we have to process the 
  // vararg list even if we don't use it).
     va_list al;
     va_start(al, format);

#if 1
#ifdef DEBUG_CONVERSION
     vfprintf(stderr, format, al);
#endif
#endif
     va_end(al);
   }
#endif

#define DEBUG_FIXUP_TYPE_REFERENCE_ON_MEMORY_POOL__VISIT 0

// DQ (8/12/2012): Fixup the SgModifiers used to hold type reference in EDG to types in instantiated templates that had not yet been seen).
// This post processing effects test2012_190.C and test2007_141.C.
// #include "fixupTypeReferences.h"

// DQ (8/21/2012): This is a traversal run to fixup the AST as part of the AST post-processing phase.
// This handles the problem of there being some references to types that we have not built (likely
// because they are in instantiated templates) and thus are reference through wrapper type IR nodes
// (we have borrowed the SgModifierType IR nodes for this purpose as a part of initial testing).
// this traversal over the memory pool of SgModifierType IR nodes is really fast and simple.
// the design permits only the reference in the SgModifierType IR node to be fixed up, but it
// does leave these wrappers in the AST as a drawback to the solution.  A more sophisticated
// solution is possible later if required.
void
FixupTypeReferencesOnMemoryPool::visit ( SgNode* node )
   {
     ROSE_ASSERT(node != NULL);
     
#if DEBUG_FIXUP_TYPE_REFERENCE_ON_MEMORY_POOL__VISIT
     printf ("#### FixupTypeReferencesOnMemoryPool::visit(node = %p = %s) \n",node,node->class_name().c_str());
#endif

  // DQ (11/25/2012): Added new test.
     ROSE_ASSERT(EDG_ROSE_Translation::project_wide_type_translation_cache.empty() == false);

     SgModifierType* modifierForReferenceType = isSgModifierType(node);
     ROSE_ASSERT(modifierForReferenceType != NULL);

  // TV (08/10/2018): more advanced test
     SageInterface::detectCycleInType(modifierForReferenceType, "FixupTypeReferencesOnMemoryPool::visit at begining");

     if (modifierForReferenceType->get_frontend_type_reference() != NULL)
        {
#if DEBUG_FIXUP_TYPE_REFERENCE_ON_MEMORY_POOL__VISIT
          printf ("Found a SgModifierType using our type reference (resetting) \n");
#endif
          a_type_ptr edg_type = (a_type_ptr)(modifierForReferenceType->get_frontend_type_reference());
          ROSE_ASSERT(edg_type != NULL);
#if DEBUG_FIXUP_TYPE_REFERENCE_ON_MEMORY_POOL__VISIT
          printf("edg_type = %p \n",edg_type);
#endif

       // DQ (9/9/2012): Added assertion.
          ROSE_ASSERT(EDG_ROSE_Translation::project_wide_type_translation_cache.find(edg_type) != EDG_ROSE_Translation::project_wide_type_translation_cache.end());

          SgType* new_base_type = EDG_ROSE_Translation::project_wide_type_translation_cache[edg_type];
          ROSE_ASSERT(new_base_type != NULL);
          
          SgTypedefType* currentType = isSgTypedefType(new_base_type);
          if (currentType != NULL)
             {
            // DQ (6/7/2016): This fixup is a problem for the new fix to avoid over use of the typedef base type where these special modifiers are used.
#if DEBUG_FIXUP_TYPE_REFERENCE_ON_MEMORY_POOL__VISIT
               printf ("Before resetting: modifierForReferenceType->get_base_type() = %p \n",modifierForReferenceType->get_base_type());
#endif
            // modifierForReferenceType->set_base_type(new_base_type);
               SgModifierType* local_modifier_type = isSgModifierType(isSgTypedefType(currentType)->get_base_type());

            // DQ (6/7/2016): This is the failing case so although I don't really understand it we can at least exclude it.
            // Specifically, it fails in the assertion below.
               if (modifierForReferenceType != local_modifier_type)
                  {
                    modifierForReferenceType->set_base_type(new_base_type);
                  }
#if DEBUG_FIXUP_TYPE_REFERENCE_ON_MEMORY_POOL__VISIT
               printf ("modifierForReferenceType = %p = %s \n",modifierForReferenceType,modifierForReferenceType->class_name().c_str());
               printf ("   --- new_base_type     = %p = %s \n",new_base_type,new_base_type->class_name().c_str());

               if (local_modifier_type != NULL)
                  {
                    printf ("local_modifier_type = %p local_modifier_type->get_base_type() = %p \n",local_modifier_type,local_modifier_type->get_base_type());
                  }
#endif
            // DQ (10/27/2015): Added test to detect cycle in typedef.
               ROSE_ASSERT( (isSgModifierType(isSgTypedefType(currentType)->get_base_type()) == NULL) || 
                            ( (isSgModifierType(isSgTypedefType(currentType)->get_base_type()) != NULL) && 
                              (isSgModifierType(isSgTypedefType(currentType)->get_base_type())->get_base_type() != currentType) ) );
            // TV (08/10/2018): more advanced test
               SageInterface::detectCycleInType(currentType, "FixupTypeReferencesOnMemoryPool::visit after fixup");
             }

       // DQ (10/27/2015): Reset the saved reference pointer to a edg type to clear it.
          modifierForReferenceType->set_frontend_type_reference(NULL);
        }
   }
