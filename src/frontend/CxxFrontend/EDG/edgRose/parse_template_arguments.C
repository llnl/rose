
#include "edgRose.h"

#define DEBUG_parse_template_arguments 0

SgTemplateArgumentPtrList EDG_ROSE_Translation::parse_template_arguments( a_template_arg_ptr tpp, SgScopeStatement* associatedScope ) {
#if DEBUG_parse_template_arguments
  printf ("In parse_template_arguments():\n");
  printf ("  tpp = %p\n", tpp);
  printf ("  associatedScope = %p = %s\n", associatedScope, associatedScope->class_name().c_str());
#endif
  ROSE_ASSERT(tpp != NULL);

  SgTemplateArgumentPtrList templateArgumentList;
  while (tpp != NULL) {
    ROSE_ASSERT(tpp != NULL);
#if DEBUG_parse_template_arguments
    printf ("> tpp = %p \n", tpp);
//  summarize_template_arg(tpp);
#endif
    SgTemplateArgument* result_templateArgument = NULL;
    if (processedTemplateArgsCache.find(tpp) != processedTemplateArgsCache.end()) {
      SgTemplateArgument * cached_templateArgument = processedTemplateArgsCache[tpp];
#if DEBUG_parse_template_arguments
      printf ("  cached_templateArgument = %p\n", cached_templateArgument);
#endif
      // TV (10/05/2018): Template arguments cannot be shared as we use them to store name qualification (ROSE-1431)
      if (cached_templateArgument->get_argumentType() == SgTemplateArgument::start_of_pack_expansion_argument) {
        result_templateArgument = cached_templateArgument;
      } else {
        // TODO Rose::Builder::Templates::buildTemplateArgument(SgTemplateArgument *)
        result_templateArgument = new SgTemplateArgument(
            cached_templateArgument->get_argumentType(),
            cached_templateArgument->get_isArrayBoundUnknownType(),
            cached_templateArgument->get_type(),
            cached_templateArgument->get_expression(),
            cached_templateArgument->get_templateDeclaration()
        );
        result_templateArgument->set_is_pack_element(cached_templateArgument->get_is_pack_element());
        result_templateArgument->set_explicitlySpecified(result_templateArgument->get_explicitlySpecified());
        // TODO it is not used below can it come from somewhere else?
        result_templateArgument->set_initializedName(cached_templateArgument->get_initializedName());

        result_templateArgument->set_parent(cached_templateArgument->get_parent());

        // TV (10/05/2018): create a chain of all the sage template argument created for a given edg template argument.
        //                  For private type fixup (ROSE-1145)
        // DQ (12/28/2018): Why is this a chain instead of an STL vector?
        // TV:              If container, we need to decide which nodes has the valid copy of the container,
        //                  then maintain a pointer from each copy to the selected node.
        //                  The chain is more elegant: it makes it part of the graph and easily visualized with graphviz.
        result_templateArgument->set_previous_instance(cached_templateArgument);
        cached_templateArgument->set_next_instance(result_templateArgument);
        processedTemplateArgsCache[tpp] = result_templateArgument;
      }
#if DEBUG_parse_template_arguments
      printf ("  result_templateArgument = %p\n", result_templateArgument);
#endif
    } else {
      SgTemplateArgument::template_argument_enum argument_kind = SgTemplateArgument::argument_undefined;

      SgType*                 typeArgument       = nullptr;
      SgExpression*           expressionArgument = nullptr;
      SgDeclarationStatement* templateArgument   = nullptr;

#if DEBUG_parse_template_arguments
      printf ("  kind = %d : %s\n", (int)(tpp->kind), summarize_templ_arg_kind_in_string(tpp->kind));
#endif
      switch (tpp->kind) {
        case tak_type: {
          argument_kind = SgTemplateArgument::type_argument;
          typeArgument = convert_type(tpp->variant.type);
          break;
        }
        case tak_nontype: {
          argument_kind = SgTemplateArgument::nontype_argument;
          expressionArgument = convert_constant(tpp->variant.constant);
          break;
        }
        case tak_template: {
          argument_kind = SgTemplateArgument::template_template_argument;
          a_template_ptr edg_template = tpp->variant.templ.ptr;
#if DEBUG_parse_template_arguments
          printf ("  edg_template = %p\n", edg_template);
#endif
          if (hasFirstNondefDeclForTemplate(edg_template)) {
              templateArgument = getFirstNondefDeclForTemplate(edg_template);
              ROSE_ASSERT(templateArgument != NULL);
          } else {
#if 1
            // TODO When does that happen?
            if (edg_template->kind == templk_class && edg_template->prototype_instantiation.type == NULL) {
              mprintf ("ERROR: This will fail in convert_template_prototype(): due to no type being available: using internal backup plan to generate type \n");

              argument_kind = SgTemplateArgument::type_argument;
              typeArgument  = SageBuilder::buildBoolType(); // SgTypeBool::createType(); // convert_type(edg_type);
              ROSE_ASSERT(typeArgument != NULL);
            } else {
              templateArgument = convert_template_prototype(edg_template);
            }
#else
            templateArgument = convert_template_prototype(edg_template);
#endif
          }
#if DEBUG_parse_template_arguments
          printf ("  templateArgument = %p : %s \n", templateArgument, templateArgument->class_name().c_str());
#endif
                break;
        }
        case tak_start_of_pack_expansion: {
          argument_kind = SgTemplateArgument::start_of_pack_expansion_argument;
          break;
        }
        default: {
          printf ("Error: Default reached in parse_template_arguments \n");
          ROSE_ABORT();
        }
      }
#if DEBUG_parse_template_arguments
      printf("   typeArgument       = %p : %s\n", typeArgument,       typeArgument       ? typeArgument->class_name().c_str()       : "");
      printf("   expressionArgument = %p : %s\n", expressionArgument, expressionArgument ? expressionArgument->class_name().c_str() : "");
      printf("   templateArgument   = %p : %s\n", templateArgument,   templateArgument   ? templateArgument->class_name().c_str()   : "");
#endif
      ROSE_ASSERT(argument_kind != SgTemplateArgument::argument_undefined);

      // TODO Rose::Builder::Templates::buildTemplateArgument(...)
      result_templateArgument = new SgTemplateArgument(argument_kind, /*isArrayBoundUnknownType=*/false, typeArgument, expressionArgument, templateArgument);
      result_templateArgument->set_is_pack_element(tpp->is_pack_element);

#if DEBUG_parse_template_arguments
      printf ("  tpp->is_pack_element = %s\n", tpp->is_pack_element ? "true" : "false");
      printf ("  tpp->explicitly_specified = %s\n", tpp->explicitly_specified ? "true" : "false");
#endif
      // DQ (2/28/2016): Uncomment so that we can explore using it to control the output of template argument in the unparser.
      // explicitlySpecified is TRUE, for a function template argument list, if the argument was explicitly specified.  When
      // a reference is being processed, this flag is set only for those arguments that were explicitly specified for that
      // reference.  For a template argument list associated with an instance of the function template, this flag
      // is set if any reference to the template explicitly specified the argument.
      // DQ (2/10/2019): When is_pack_element is true, then it seems that tpp->explicitly_specified is relevant (else not always).
      if (result_templateArgument->get_is_pack_element()) {
        result_templateArgument->set_explicitlySpecified(tpp->explicitly_specified);
      }

      // DQ (7/25/2012): We need to set this to the local scope and then fix it up later to point to the 
      // function. Note that if this is NULL then we can't compute the name qualification required to define
      // the name with template arguments (so that we can build the first non-defining declaration.
      // Note also that we still can't test using "testTemplateArgumentParents()" until we reset the 
      // parent more precisely to the function instead of the scope of the function.
      result_templateArgument->set_parent(associatedScope);

#if 1
      // TODO When does that happen? That was old code without comment
      if (expressionArgument != NULL && expressionArgument->get_parent() == NULL) {
        expressionArgument->set_parent(result_templateArgument);
      }
#else
      ROSE_ASSERT(expressionArgument == NULL || expressionArgument->get_parent() != NULL);
#endif
      ROSE_ASSERT(templateArgument == NULL || templateArgument->get_parent() != NULL);

      ROSE_ASSERT(result_templateArgument != NULL);
      processedTemplateArgsCache[tpp] = result_templateArgument;
    }
    if (result_templateArgument != NULL) {
      templateArgumentList.push_back(result_templateArgument);
    }
    tpp = tpp->next;
  }

  return templateArgumentList;
}
