#include <sage3basic.h>

void
SgPseudoDestructorRefExp::post_construction_initialization()
   {
     SgMemberFunctionType *memFnType = new SgMemberFunctionType(SgTypeVoid::createType(), false, p_object_type);
     set_expression_type(memFnType);
   }


// DQ (1/18/2020): Adding name qualification support to SgPseudoDestructorRefExp.
int
SgPseudoDestructorRefExp::get_name_qualification_length () const
   {
     ASSERT_this();
     return p_name_qualification_length;
   }

// DQ (1/18/2020): Adding name qualification support to SgPseudoDestructorRefExp.
void
SgPseudoDestructorRefExp::set_name_qualification_length ( int name_qualification_length )
   {
     ASSERT_this();
  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     p_name_qualification_length = name_qualification_length;
   }

// DQ (1/18/2020): Adding name qualification support to SgPseudoDestructorRefExp.
bool
SgPseudoDestructorRefExp::get_type_elaboration_required () const
   {
     ASSERT_this();
     return p_type_elaboration_required;
   }

// DQ (1/18/2020): Adding name qualification support to SgPseudoDestructorRefExp.
void
SgPseudoDestructorRefExp::set_type_elaboration_required ( bool type_elaboration_required )
   {
     ASSERT_this();
  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     p_type_elaboration_required = type_elaboration_required;
   }

// DQ (1/18/2020): Adding name qualification support to SgPseudoDestructorRefExp.
bool
SgPseudoDestructorRefExp::get_global_qualification_required () const
   {
     ASSERT_this();
     return p_global_qualification_required;
   }

// DQ (1/18/2020): Adding name qualification support to SgPseudoDestructorRefExp.
void
SgPseudoDestructorRefExp::set_global_qualification_required ( bool global_qualification_required )
   {
     ASSERT_this();

  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     p_global_qualification_required = global_qualification_required;
   }
