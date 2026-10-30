
#include "edgRose.h"

// This is where I have copied EDG specific source code 
// typically from cp_gen_be.c to support some of the
// logic required to translater from EDG to the ROSE IR.


bool
EDG_ROSE_Translation::is_implicitly_cast_integral_constant(a_constant_ptr con)
/*
Return TRUE if the indicated constant is an integral constant cast to
some other type, and the conversion is one that can be done implicitly.
This is used for calls of C-mode functions with parameters that
possibly involve prototype scope types.  For example:

  void f(struct A { int i; } *);
  void g(enum E { AA });
  main() {
    f(0);
    g(5);
  }

These cases are problems because the cast cannot be written explicitly
since the type cannot be named at the call site; therefore we detect them
and put them out as implicit conversions.  Note that for the 0 --> pointer
case we're not checking to see if a prototype scope type is involved in
the conversion, so some cases not involving such types will be rendered
as implicit conversions even though the conversion could be written
explicitly, but that's a good thing for a null pointer constant.  Implicit
conversions on nonconstants are handled in eok_cast processing.
*/
{
  bool is_implicit_cast = FALSE;

  /* The implicit_cast flag is not checked here on purpose.  It's not set
     for the enum case. */
  if (con->kind == (a_constant_repr_kind)ck_integer) {
    a_type_ptr con_type = skip_typerefs(con->type);
    if (con_type->kind == (a_type_kind)tk_pointer &&
        cmplit_integer_constant(con, (a_host_large_integer)0) == 0) {
      /* Zero converted to a pointer type. */
      is_implicit_cast = TRUE;
    } else if (con_type->kind == (a_type_kind)tk_integer &&
               con_type->variant.integer.enum_type &&
               con_type->declared_in_function_prototype) {
      /* Any integral value converted to a prototype scope enum type. */
      is_implicit_cast = TRUE;
    }  /* if */
  }  /* if */
  return is_implicit_cast;
}  /* is_implicitly_cast_integral_constant */


// DQ (12/30/2008): This function is copied from the EDG unparser.
bool
EDG_ROSE_Translation::pm_cast_is_unambiguous(a_constant_ptr constant)
/*
constant points at a ck_ptr_to_member constant.  If the constant requires
a cast to a base or derived class, and the cast is unambiguous when done in
one step instead of class-by-class, return TRUE.
*/
{
  bool unambiguous_cast = FALSE;

  if (constant->variant.ptr_to_member.casting_base_class != NULL) {
    /* See if there is another base class with the same name as the one
       we're starting from.  If not, the cast is unambiguous. */
    if (!constant->variant.ptr_to_member.casting_base_class->ambiguous) {
      unambiguous_cast = TRUE;
    }  /* if */
  }  /* if */
  return unambiguous_cast;
}  /* pm_cast_is_unambiguous */


a_statement_ptr
EDG_ROSE_Translation::find_statement_before_target(a_statement_ptr starting_statement, a_statement_ptr target_statement)
   {
  // DQ (10/24/2012): This function is used in the support for switch statements and identifying statements around case option statements.

  // Note that target_statement CAN be NULL which means that we return the last statement in the list (that is not a stmk_label).

  // DQ (10/28/2012): This function must not count statements that will be associated with a non-compiler-generated label.


  // Return statement.
     a_statement_ptr last_statement_before_target_statement = NULL;

     ROSE_ASSERT(starting_statement != NULL);


  // If this is a stmk_label, it should not be a compiler generated one.
     ROSE_ASSERT((starting_statement->kind != stmk_label) || (starting_statement->variant.label.ptr->source_corresp.name != NULL) );
     if (target_statement != NULL)
        {
          ROSE_ASSERT(target_statement->kind != stmk_label);
        }

#if 0
     printf ("In find_statement_before_target(): starting_statement = %p = %s target_statement = %p = %s \n",
          starting_statement,summarize_statement_in_string(starting_statement),target_statement,summarize_statement_in_string(target_statement));
#endif

  // Start here.
     a_statement_ptr temp_statement = starting_statement;
     while (temp_statement != NULL)
        {
#if 0
          printf ("In find_statement_before_target(): temp_statement = %p = %s \n",temp_statement,summarize_statement_in_string(temp_statement));
#endif
          last_statement_before_target_statement = temp_statement;

       // If we encounter a non-compiler-generated label then it will absorbe the next statement, so advance past this label.
       // Can a non-compiler-generated label be used without an associated statement to absorbe???
          if ( (temp_statement != NULL) && (temp_statement->kind == stmk_label) && (temp_statement->variant.label.ptr->source_corresp.name != NULL) )
             {
#if 0
               a_label_ptr label_statement = temp_statement->variant.label.ptr;
               printf ("In find_statement_before_target(): Found a label %s in a case statement list: the next statement will be absorbed into the label so advance past next statement \n",label_statement->source_corresp.name);
#endif
               temp_statement = (temp_statement->next == target_statement) ? NULL : temp_statement->next;
             }

          if (temp_statement != NULL)
             {
               temp_statement = (temp_statement->next == target_statement) ? NULL : temp_statement->next;
             }

       // Loop over any compiler generated stmk_label statements in the list (note: source_corresp.name == NULL means compiler generated).
       // while (temp_statement != NULL && temp_statement->kind == stmk_label)
          while ( (temp_statement != NULL) && (temp_statement->kind == stmk_label) && (temp_statement->variant.label.ptr->source_corresp.name == NULL) )
             {
#if 0
               printf ("In find_statement_before_target(): loop over stmk_label statements temp_statement = %p = %s \n",temp_statement,summarize_statement_in_string(temp_statement));
#endif
               temp_statement = (temp_statement->next == target_statement) ? NULL : temp_statement->next;
             }

       // Loop over stmk_init statements since they are not a part of the source sequence list.
          while ( (temp_statement != NULL) && (temp_statement->kind == stmk_init) )
             {
#if 0
               printf ("In find_statement_before_target(): loop over stmk_init statements temp_statement = %p = %s \n",temp_statement,summarize_statement_in_string(temp_statement));
#endif
               temp_statement = (temp_statement->next == target_statement) ? NULL : temp_statement->next;
             }
        }

  // Can we assert this?
  // ROSE_ASSERT(last_statement_before_target_statement != NULL);

#if 0
     printf ("Leaving find_statement_before_target(): last_statement_before_target_statement = %p = %s \n",
          last_statement_before_target_statement,last_statement_before_target_statement != NULL ? summarize_statement_in_string(last_statement_before_target_statement) : "null");
#endif

     return last_statement_before_target_statement;
   }

