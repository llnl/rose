#include <sage3basic.h>

void
SgCompoundInitializer::post_construction_initialization()
   {
     if (get_initializers())
          get_initializers()->set_parent(this);
   }

// DQ: trying to remove the nested iterator class
void
SgCompoundInitializer::append_initializer(SgExpression* what)
   {
     ASSERT_this();

  // DQ (11/15/2006): avoid setting newArgs this late in the process.
     ROSE_ASSERT(p_initializers != NULL);
     if(!p_initializers)
        {
       // set_initializers(new SgExprListExp(this->get_file_info()));
          SgExprListExp* newArgs = new SgExprListExp ( this->get_file_info() );
          assert(newArgs != NULL);
          newArgs->set_endOfConstruct( this->get_file_info() );
          set_initializers(newArgs);
        }

  // insert_initializer(p_initializers->end(),what);
     p_initializers->append_expression(what);
   }

SgExpression*
SgCompoundInitializer::get_next(int& n) const
   {
     if(n==0)
        {
          n++;
          return get_initializers();
        }
       else
          return 0;
   }

int
SgCompoundInitializer::replace_expression(SgExpression* o, SgExpression* n)
   {
  // DQ (12/17/2006): This function should have the semantics that it will represent a
  // structural change to the AST, thus it is free to set the parent of the new expression.

     ROSE_ASSERT(o != NULL);
     ROSE_ASSERT(n != NULL);

     if (get_initializers() == o)
        {
          set_initializers(isSgExprListExp(n));
          n->set_parent(this);
          return 1;
        }
       else
        {
          return 0;
        }
   }

SgType*
SgCompoundInitializer::get_type() const
   {
#if 0
     printf ("In SgCompoundInitializer::get_type() \n");
#endif

     if (p_expression_type != NULL)
        {
          return p_expression_type;
        }
       else
        {
          return SgTypeDefault::createType();
        }
   }
