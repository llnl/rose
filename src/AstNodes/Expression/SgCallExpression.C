#include <sage3basic.h>

void
SgCallExpression::post_construction_initialization()
   {
     if (p_function != NULL)
          p_function->set_parent(this);
   }


SgExpression*
SgCallExpression::get_next(int& n) const
   {
     if(n==0)
        {
          n++;
          return get_function();
        }
       else
          if(p_args && n==1)
             {
               n++;
               return get_args();
             }

     return 0;
   }

// DQ: trying to remove the nested iterator class
void
SgCallExpression::append_arg(SgExpression* what)
   {
     ASSERT_this();

  // DQ (11/15/2006): avoid setting newArgs this late in the process.
     ROSE_ASSERT(p_args != NULL);
     if (p_args == NULL)
        {
       // set_args(new SgExprListExp(this->get_file_info()));
          SgExprListExp* newArgs = new SgExprListExp ( this->get_file_info() );
          assert(newArgs != NULL);
          newArgs->set_endOfConstruct( this->get_file_info() );
          set_args(newArgs);
        }

    // insert_arg(p_args->end(),what);
       p_args->append_expression(what);
   }

int
SgCallExpression::replace_expression(SgExpression *o, SgExpression *n)
   {
  // DQ (12/17/2006): This function should have the semantics that it will represent a
  // structural change to the AST, thus it is free to set the parent of the new expression.

     ROSE_ASSERT(o != NULL);
     ROSE_ASSERT(n != NULL);

     if (get_function() == o)
        {
          set_function(n);
          n->set_parent(this);

          return 1;
        }
       else
        {
          if (p_args == o)
             {
            // DQ (12/17/2006): Make this code safer (avoid passing NULL pointers to functions that we call!
            // set_args(isSgExprListExp(n));
               SgExprListExp* expressionList = isSgExprListExp(n);
               ROSE_ASSERT(expressionList != NULL);
               set_args(expressionList);
               n->set_parent(this);
               return 1;
             }
        }

     return 0;
   }

SgType*
SgCallExpression::get_type() const
   {
#if 0
     printf ("In SgCallExpression::get_type() \n");
#endif

     return SageBuilder::buildVoidType(); //TODO this is wrong
   }
