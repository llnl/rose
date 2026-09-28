#include <sage3basic.h>

SgName
SgTemplateParameterVal::get_template_parameter_name() const
   {
  // DQ (11/28/2011): Added to support template declarations in the AST.

  // Note that we have to store the index position into the template parameter list
  // so that this template parameter can be identified and its name used.  It
  // can be different from the template parameter name of the template class declaration
  // (which is the name referenced in EDG), so we have to find the enclosing template
  // declaration and use the index position into its template parameter list (which
  // can of course use different names).

  // DQ (7/24/2012): Added test.
     ASSERT_this();

     ROSE_ASSERT(get_parent() != NULL);

  // DQ (7/24/2012): For the new EDG support (4.x) and new template handling, we don't use
  // a generic SgTemplateDeclaration; instead we have specific kinds of template declarations.
  // So templateDeclaration will always be NULL now.

     int template_parameter_position = get_template_parameter_position();

  // DQ (7/25/2012): This reflects the new design of how templates are supported in ROSE.
     SgDeclarationStatement* templateDeclaration = TransformationSupport::getTemplateDeclaration(this);
     if (templateDeclaration == NULL)
        {
          printf ("Error: In SgTemplateParameterVal::get_template_parameter_name(): templateDeclaration == NULL: this = %p = %s \n",this,this->class_name().c_str());

       // DQ (6/6/2013): If the template parameter is used as a template argument, then we just return a simple name.
       // Nowever, maybe the constant should have been shared so that it would be correctly associated with a template declaration.
          SgName name = "__template_parameter_used_as_template_argument__";
          return name;
        }
     ROSE_ASSERT(templateDeclaration != NULL);

     SgTemplateParameterPtrList* templateParameters = NULL;
     switch(templateDeclaration->variantT())
        {
          case V_SgTemplateDeclaration:               templateParameters = &(isSgTemplateDeclaration(templateDeclaration)->get_templateParameters());               break;
          case V_SgTemplateClassDeclaration:          templateParameters = &(isSgTemplateClassDeclaration(templateDeclaration)->get_templateParameters());          break;

       // DQ (2/1/2019): Bug fix, likely copy-paste error, casting to wrong type.
       // case V_SgTemplateFunctionDeclaration:       templateParameters = &(isSgTemplateMemberFunctionDeclaration(templateDeclaration)->get_templateParameters()); break;
          case V_SgTemplateFunctionDeclaration:       templateParameters = &(isSgTemplateFunctionDeclaration(templateDeclaration)->get_templateParameters());       break;

          case V_SgTemplateMemberFunctionDeclaration: templateParameters = &(isSgTemplateMemberFunctionDeclaration(templateDeclaration)->get_templateParameters()); break;
          case V_SgTemplateVariableDeclaration:       templateParameters = &(isSgTemplateVariableDeclaration(templateDeclaration)->get_templateParameters());       break;

          default:
             {
               printf ("Error: default reached in SgTemplateParameterVal::get_template_parameter_name(): templateDeclaration = %p = %s \n",templateDeclaration,templateDeclaration->class_name().c_str());
               ROSE_ASSERT(false);
             }
        }

     ROSE_ASSERT(templateParameters != NULL);

     long int numberOfTemplateParameters = (long int) templateParameters->size();

  // DQ (12/13/2011): this is a problem demonstrated by test2011_186.C, so we have to disable the template parameter position error checking.
     bool getParameterNameFromTemplateParameterList = true;
     if (template_parameter_position >= numberOfTemplateParameters)
        {
          getParameterNameFromTemplateParameterList = false;
        }

     SgName name;
     if (getParameterNameFromTemplateParameterList == true)
        {
          ROSE_ASSERT(template_parameter_position >= 0);
          ROSE_ASSERT(template_parameter_position < numberOfTemplateParameters);

          name = "__template_parameter_name__error__";

          SgTemplateParameter* templateParameter = NULL;
          if (template_parameter_position >= 0)
             {
               templateParameter = (*templateParameters)[template_parameter_position];
               ROSE_ASSERT(templateParameter != NULL);
               ROSE_ASSERT(templateParameter->get_parameterType() == SgTemplateParameter::nontype_parameter);

               SgInitializedName* initializedName = templateParameter->get_initializedName();
               ROSE_ASSERT(initializedName != NULL);

               name = initializedName->get_name();
             }
        }
       else
        {
       // This is not the favored case since i think it can be problematic to use template parameter names.
          name = get_valueString();
        }

     return name;
   }

SgType*
SgTemplateParameterVal::get_type() const
   {
  // DQ (8/6/2013): The correct type is now saved explicitly so that it can be used to disambiguate
  // template functions overloaded on template parameters. See test2013_303.C for an example.
     ROSE_ASSERT(this->get_valueType() != NULL);

#if 0
     printf ("In SgTemplateParameterVal::get_type() \n");
#endif

     return this->get_valueType();
   }
