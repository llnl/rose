#include <sage3basic.h>

SgType*
SgFunctionParameterRefExp::get_type() const
   {
  // DQ (8/11/2014): Added support for C++11 decltype used in new function return syntax.

     ASSERT_this();

     SgType* returnType = NULL;

#if 0
     printf ("In SgFunctionParameterRefExp::get_type() \n");
#endif

     if (p_parameter_expression != NULL)
        {
          ROSE_ASSERT(p_parameter_type == NULL);
          returnType = p_parameter_expression->get_type();
          ROSE_ASSERT(returnType != NULL);
        }
       else
        {
          returnType = p_parameter_type;
          ROSE_ASSERT(returnType != NULL);
        }

     ROSE_ASSERT(returnType != NULL);
     return returnType;
   }
