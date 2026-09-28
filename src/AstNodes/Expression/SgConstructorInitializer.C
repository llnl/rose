#include <sage3basic.h>

/** Obtain the class declaration for a constructor initializer if possible.
 *
 * If this is a constructor initializer for a class type then the first non-defining declaration of the constructor's class is
 * returned; the null pointer is returned for constructor initializers of primitive types.
 *
 * The SgConstructorInitializer::p_declaration points to a SgMemberFunctionDeclaration when the constructor is declared, but is
 * the null pointer when the constructor is not declared or when the constructor is for a non-class type.
 *
 * When the p_declaration is the null pointer then the SgConstructorInitializer::p_expression_type can be used to determine
 * whether the constructor initializer is for a class type without a declared constructor, or for a non-class type.  In the
 * former case, the p_expression_type points to a SgClassType.  In the latter case it points to either a SgFunctionType (EDG4)
 * or null (EDG3). */
SgClassDeclaration*
SgConstructorInitializer::get_class_decl () const
{
    SgClassDeclaration *class_decl = NULL;

    if (SgMemberFunctionDeclaration *fdecl = get_declaration()) {
     // The constructor is declared (and is a member of a class type)
     // DQ (11/11/2014): Modified version of code to address the fact that Fortran is organized a bit differently.
     // That it is different from C++ might be something to revisit later in the Fortran support.
        ROSE_ASSERT(get_declaration() != NULL);
        ROSE_ASSERT(fdecl->get_scope() != NULL);

        SgClassDefinition *cdef = isSgClassDefinition(fdecl->get_scope());
        if (cdef != NULL)
           {
             assert(cdef!=NULL);
             class_decl = isSgClassDeclaration(cdef->get_parent());
             assert(class_decl!=NULL);
           }
    } else if (SgClassType *expr_type = isSgClassType(get_expression_type())) {
        // Constructor initializer is for a class type with a non-declared default constructor.  We can follow the
        // p_expression_type's declaration.
        class_decl = isSgClassDeclaration(expr_type->get_declaration());
        assert(class_decl!=NULL);
    } else if (SgJavaParameterizedType* parmType = isSgJavaParameterizedType(p_expression_type)) {
      // 12/16/12 PC added case of SgJavaParameterizedType... Note that Strictly speaking,
      // an SgJavaParameterizedType is a class.  However, this is not the case in the current
      // SAGE Representation.
        class_decl = isSgClassDeclaration(parmType->get_declaration());
        ROSE_ASSERT(class_decl != NULL);
    } else if (SgJavaQualifiedType *qualType = isSgJavaQualifiedType(p_expression_type)) {
      // 01/24/15 PC added case of SgJavaQualifiedType... Note that Strictly speaking,
      // an SgJavaQualifiedType is a class.  However, this is not the case in the current
      // SAGE Representation.
        class_decl = isSgClassDeclaration(qualType->get_declaration());
        ROSE_ASSERT(class_decl != NULL);
    } else {
        // Constructor is for a primitive type.  There is no class declaration to return.
    }

    if (class_decl) {
        class_decl = isSgClassDeclaration(class_decl->get_firstNondefiningDeclaration());
        assert(class_decl!=NULL);
    }
    return class_decl;
}

void
SgConstructorInitializer::post_construction_initialization()
   {
     ROSE_ASSERT(p_expression_type != NULL);
     if (p_declaration == NULL)
        {
         // PP (4/17/21) : Added exception for Ada
         //   In Ada, SgConstructorInitializer nodes can be used with
         //   typedef types and built in types (e.g., x := new Integer'(2); )

       // This can be NULL for the case of an undeclared constructor.
       // 12/16/12 PC added case of SgJavaParameterizedType... Note that Strictly speaking,
       // an SgJavaParameterizedType is a class.  However, this is not the case in the current
       // SAGE Representation.
       //01/24/15 added case of SgJavaQualifiedType...

       // DQ (1/19/2019): This is failing in the copyAST tests for test2005_24.C

          ROSE_ASSERT(SageInterface::is_Ada_language() ||
                      (isSgJavaParameterizedType(p_expression_type) || isSgJavaQualifiedType(p_expression_type) ||
                       isSgTypedefType(p_expression_type) || isSgClassType(p_expression_type) != NULL) ||
                      (p_associated_class_unknown == true));
        }

  // DQ (11/15/2006): avoid setting newArgs this late in the process.
  // ROSE_ASSERT(p_args != NULL);
     if (p_args == NULL)
        {
       // Build an empty argument list

       // DQ (11/16/2006): Need to handle use in new constructors that don't have fileInfo parameters.
       // p_args = new SgExprListExp(New_File_Info(this));
          if (get_startOfConstruct() != NULL)
             {
               p_args = new SgExprListExp(New_File_Info(this));
               p_args->set_endOfConstruct(New_File_Info(this));
             }
            else
             {
               p_args = new SgExprListExp();
             }
          ROSE_ASSERT(p_args != NULL);
        }

  // if (get_args() != NULL)
  //      get_args()->set_parent(this);
     get_args()->set_parent(this);

     ROSE_ASSERT(p_args != NULL);
   }

SgExpression*
SgConstructorInitializer::get_next(int& n) const
   {
     if(n==0)
        {
          n++;
          return get_args();
        }
     return 0;
   }

int
SgConstructorInitializer::replace_expression(SgExpression *o, SgExpression *n)
   {
  // DQ (12/17/2006): This function should have the semantics that it will represent a
  // structural change to the AST, thus it is free to set the parent of the new expression.

     ROSE_ASSERT(o != NULL);
     ROSE_ASSERT(n != NULL);

     if(get_args() == o)
        {
          set_args(isSgExprListExp(n));
          n->set_parent(this);
          return 1;
        }
       else
          return 0;
   }

// DQ (6/11/2015): Moved these six access functions, they should not be generated by ROSETTA
// so that we could avoid them setting the isModified flag which is a problem in the
// name qualification support for C++ (interfering with the token-based unparsing).
int
SgConstructorInitializer::get_name_qualification_length () const
   {
     ASSERT_this();
     return p_name_qualification_length;
   }

void
SgConstructorInitializer::set_name_qualification_length ( int name_qualification_length )
   {
     ASSERT_this();
  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     p_name_qualification_length = name_qualification_length;
   }

bool
SgConstructorInitializer::get_type_elaboration_required () const
   {
     ASSERT_this();

     return p_type_elaboration_required;
   }

void
SgConstructorInitializer::set_type_elaboration_required ( bool type_elaboration_required )
   {
     ASSERT_this();
  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     p_type_elaboration_required = type_elaboration_required;
   }

bool
SgConstructorInitializer::get_global_qualification_required () const
   {
     ASSERT_this();
     return p_global_qualification_required;
   }

void
SgConstructorInitializer::set_global_qualification_required ( bool global_qualification_required )
   {
     ASSERT_this();

  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     p_global_qualification_required = global_qualification_required;
   }
