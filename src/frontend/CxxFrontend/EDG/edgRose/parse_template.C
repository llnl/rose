
#include "edgRose.h"

#define DEBUG_EDG_ROSE_Translation__parse_template 0

SgDeclarationStatement * EDG_ROSE_Translation::parse_template(SourceSequenceList& sse) {
#if DEBUG_EDG_ROSE_Translation__parse_template
  printf ("In parse_template(sse):\n");
#endif

  SgDeclarationStatement* result = NULL;

  a_template_ptr p = (a_template_ptr)(sse->entity.ptr);
#if DEBUG_EDG_ROSE_Translation__convert_template_prototype
  printf("  p = %p\n", p);
#endif
  ROSE_ASSERT (p != NULL);

  if (p->prototype_template) {
    p = p->prototype_template;
  }
#if DEBUG_EDG_ROSE_Translation__convert_template_prototype
  printf("  p = %p\n", p);
#endif

  string edg_template_string = p->text ? p->text : "/* NO TEMPLATE IN EDG IL */";
  ROSE_ASSERT(!edg_template_string.empty());

  templateDeclarationStack.push_back(p);
  SgDeclarationScope * nonreal_decl_scope = get_nonreal_declaration_scope_for_template(p, "parse_template");

#ifndef NDEBUG
  testEdgRoseMaps();
#endif

  a_template_kind kind = p->kind;
#if DEBUG_EDG_ROSE_Translation__parse_template
  printf ("  kind = %s \n",summarize_template_kind_in_string(kind));
#endif
  switch (kind) {
    case templk_function:
    case templk_member_function: {
      result = convert_templk_function(p);
      ROSE_ASSERT( hasFirstNondefDeclForTemplate(p) || hasDefiningDeclForTemplate(p) );
      ++sse;
      break;
    }
    case templk_variable:
    case templk_static_data_member: {
      result = convert_templk_variable(p);
      ROSE_ASSERT( hasFirstNondefDeclForTemplate(p) || hasDefiningDeclForTemplate(p) );
      ++sse;
      break;
    }
    case templk_member_class:
    case templk_class: {
      a_type_ptr type = p->prototype_instantiation.type;
      ROSE_ASSERT(type != NULL);

      if (type->kind != tk_typeref) {
        SgDeclarationStatement * declstmt = parse_structlike(sse, type, SgClassDeclaration::e_class, true, p);
        SgTemplateClassDeclaration* templateClassDeclaration = isSgTemplateClassDeclaration(declstmt);
        ROSE_ASSERT(templateClassDeclaration != NULL);

        set_file_info_for_template(templateClassDeclaration,p);
        *(templateClassDeclaration->get_endOfConstruct()) = *(templateClassDeclaration->get_definition()->get_endOfConstruct());

        templateClassDeclaration->set_string(edg_template_string);

        ROSE_ASSERT(matching_defining_and_nondefining_variants(templateClassDeclaration));

        result = templateClassDeclaration;
      } else {
        result = convert_templk_class_for_typeref(p);
        ROSE_ASSERT(result != NULL);
        ++sse;
      }
      break;
    }
    case templk_concept: {
      printf ("templk_concept: Case not handled! \n");
      ROSE_ABORT();
    }
    case templk_member_enum: {
      printf ("templk_member_enum: Case not handled! \n");
      ROSE_ABORT();
    }
    case templk_template_template_param: {
      printf ("templk_template_template_param: Case not handled! \n");
      ROSE_ABORT();
    }
    default: {
      printf ("Bad type kind in parse_template %d\n", kind);
      ROSE_ABORT();
    }
  }

  attach_nonreal_declaration_scope_to_template(p, result, nonreal_decl_scope, "parse_template");
  templateDeclarationStack.pop_back();

  ROSE_ASSERT(matching_defining_and_nondefining_variants(result));
  testEdgRoseMaps();

  // DQ (3/2/2013): In this case the template in EDG is makes as having default access privledge 
  // while the value in the function declaration of the non-defining declaration might be private 
  // or protected.  We need to only set the access value if is is non already set.  So we have
  // to map the ROSE values back to the edg value (which are different, and opposent in ordering).
  unsigned int edg_access_level  = p->source_corresp.access; // values: 0-2 (public  = 0, protected = 1, private = 2)
  set_access_modifier(result,edg_access_level);
  ROSE_ASSERT(result->get_declarationModifier().get_accessModifier().isDefault() == false);

  return result;
}
