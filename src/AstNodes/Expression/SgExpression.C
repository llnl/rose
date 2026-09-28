#include <sage3basic.h>

#include <iostream>
#include <map>
#include <string>

Sg_File_Info*
SgExpression::get_file_info() const
   {
  // DQ (11/8/2006): Note that the EDG/SageIII translation only
  // uses set_startOfConstruct() and set_endOfConstruct().

  // This redefines get_file_info() as it is implemented for a SgLocatedNode
  // to use the "get_operatorPosition()" instead of the get_startOfConstruct()"
  // Most if not all operator positions will associated with syntax for the operator.
  // return get_operatorPosition();
     Sg_File_Info* returnFileInfo = get_operatorPosition();
     if (returnFileInfo == NULL)
          returnFileInfo = get_startOfConstruct();
     return returnFileInfo;
   }

void
SgExpression::set_file_info(Sg_File_Info* fileInfo)
   {
  // DQ (11/8/2006): Note that the EDG/SageIII translation only
  // uses set_startOfConstruct() and set_endOfConstruct().

  // This redefines get_file_info() as it is implemented for a SgLocatedNode
  // to use the "get_operatorPosition()" instead of the get_startOfConstruct()"
  // Most if not all operator positions will associated with syntax for the operator.
     return set_operatorPosition(fileInfo);
   }

#define DEBUG_SGEXPRESSION_GET_QUALIFIED_NAME_PREFIX 0

SgName
SgExpression::get_qualified_name_prefix() const
   {
  // DQ (5/29/2011): Added to support for new qualified name generation.
  // This only applies to specific SgSupport IR nodes:
  //    SgVarRefExp
  //    SgFunctionRefExp
  //    SgMemberFunctionRefExp

#if DEBUG_SGEXPRESSION_GET_QUALIFIED_NAME_PREFIX
     printf ("In get_qualified_name_prefix(): search globalQualifiedNameMapForNames: this = %p = %s \n",this,this->class_name().c_str());
#endif

  // DQ (5/28/2011): We have to handle the name qualification directly since types can be qualified
  // different and so it depends upon where the type is referenced.  Thus the qualified name is
  // stored in a map to the IR node that references the type.
     SgName nameQualifier;
     auto const /*iterator*/ i = SgNode::get_globalQualifiedNameMapForNames().find(const_cast<SgExpression*>(this));
  // ROSE_ASSERT(i != SgNode::get_globalQualifiedNameMapForNames().end());

     if (i != SgNode::get_globalQualifiedNameMapForNames().end())
        {
          nameQualifier = i->second;
#if DEBUG_SGEXPRESSION_GET_QUALIFIED_NAME_PREFIX
          printf ("In SgExpression::get_qualified_name_prefix(): Found a valid name qualification: nameQualifier = %s \n",nameQualifier.str());
#endif
        }
       else
        {
#if DEBUG_SGEXPRESSION_GET_QUALIFIED_NAME_PREFIX
          printf ("In SgExpression::get_qualified_name_prefix(): Could NOT find a valid name qualification: nameQualifier = %s \n",nameQualifier.str());
#endif
        }

#if DEBUG_SGEXPRESSION_GET_QUALIFIED_NAME_PREFIX
     printf ("nameQualifier for SgExpression = %p = %p = %s = %s \n",this,const_cast<SgExpression*>(this),class_name().c_str(),nameQualifier.str());
#endif

     return nameQualifier;
   }


SgName
SgExpression::get_qualified_name_prefix_for_referenced_type() const
   {
  // DQ (6/2/2011): Added to support for new qualified name generation.

  // DQ (5/28/2011): We have to handle the name qualification directly since types can be qualified
  // different and so it depends upon where the type is referenced.  Thus the qualified name is
  // stored in a map to the IR node that references the type.
     SgName nameQualifier;
     auto const /*iterator*/ i = SgNode::get_globalQualifiedNameMapForTypes().find(const_cast<SgExpression*>(this));

     if (i != SgNode::get_globalQualifiedNameMapForTypes().end())
        {
          nameQualifier = i->second;
        }

     return nameQualifier;
   }


void
SgExpression::post_construction_initialization()
   {
   }

int
SgExpression::replace_expression(SgExpression *,SgExpression *)
   {
     std::cerr << "Call to undefined SgExpression::replace_expression(): aborting\n";
     std::cerr << "dynamic type: " << this->class_name() << "\n";
     ROSE_ASSERT(false);
     return 0;
   }

SgType*
SgExpression::get_type() const
   {
  // DQ: With this function defined we can be sure that we don't call it by accident.
  // This catches any IR nodes where the fucntion should have been overwritten and was not.

     std::cerr << "Call to undefined SgExpression::get_type(): aborting\n";
     std::cerr << "dynamic type: " << this->class_name() << "\n";
     ROSE_ASSERT(false);

     return 0;
   }

int
SgExpression::precedence() const
   {
     return 0;
   }

int
SgExpression::get_name_qualification_length () const
   {
     ASSERT_this();

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);

     return 0; // p_name_qualification_length;
   }

void
SgExpression::set_name_qualification_length ( int /*name_qualification_length*/ )
   {
     ASSERT_this();
  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);
   }

bool
SgExpression::get_type_elaboration_required () const
   {
     ASSERT_this();

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);

     return false; // p_type_elaboration_required;
   }

void
SgExpression::set_type_elaboration_required ( bool /*type_elaboration_required*/ )
   {
     ASSERT_this();
  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);

  // p_type_elaboration_required = type_elaboration_required;
   }

bool
SgExpression::get_global_qualification_required () const
   {
     ASSERT_this();

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);

     return false; // p_global_qualification_required;
   }

void
SgExpression::set_global_qualification_required ( bool /*global_qualification_required*/ )
   {
     ASSERT_this();
  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);

  // p_global_qualification_required = global_qualification_required;
   }


int
SgExpression::get_name_qualification_for_pointer_to_member_class_length () const
   {
     ASSERT_this();

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);

     return 0; // p_name_qualification_length;
   }

void
SgExpression::set_name_qualification_for_pointer_to_member_class_length ( int /*name_qualification_length*/ )
   {
     ASSERT_this();
  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);

  // p_name_qualification_length = name_qualification_length;
   }


bool
SgExpression::get_type_elaboration_for_pointer_to_member_class_required () const
   {
     ASSERT_this();

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);

     return false; // p_type_elaboration_required;
   }

void
SgExpression::set_type_elaboration_for_pointer_to_member_class_required ( bool /*type_elaboration_required*/ )
   {
     ASSERT_this();
  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);

  // p_type_elaboration_required = type_elaboration_required;
   }

bool
SgExpression::get_global_qualification_for_pointer_to_member_class_required () const
   {
     ASSERT_this();

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);

     return false; // p_global_qualification_required;
   }

void
SgExpression::set_global_qualification_for_pointer_to_member_class_required ( bool /*global_qualification_required*/ )
   {
     ASSERT_this();
  // This can't be called by the name qualification API (see test2015_26.C).
  // set_isModified(true);

     printf ("Error: base class virtual function called by mistake on node = %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);

  // p_global_qualification_required = global_qualification_required;
   }


// DQ (9/23/2011): Use the vitual function version so that we can test within ROSE (part of incremental testing of new original expression tree support).
// DQ (9/19/2011): Put back the original code (non-virtual functions) so that we can test against previously passing tests.
SgExpression*
SgExpression::get_originalExpressionTree() const
   {
     SgExpression* originalExpressionTree = NULL;

     return originalExpressionTree;
   }

// DQ (9/19/2011): Put back the original code (non-virtual functions) so that we can test against previously passing tests.
void
SgExpression::set_originalExpressionTree(SgExpression*)
   {
     printf ("ERROR: base function SgExpression::set_originalExpressionTree() called for %p = %s \n",this,this->class_name().c_str());
     ROSE_ASSERT(false);
   }

bool
SgExpression::hasExplicitType()
   {
  // DQ (3/7/2014):  This could be implemented as a virtual function but would require 11 functions to be
  // implemented. I have thus instead implemented it as a single function on the SgType instead. We
  // can review this if it is important.

  // This function returns true only if this is either a SgTemplateParameterVal, SgComplexVal, SgSizeOfOp, SgAlignOfOp,
  // SgJavaInstanceOfOp, SgJavaAnnotation, SgTypeIdOp, SgVarArgStartOp, SgVarArgStartOneOperandOp,
  // SgVarArgOp, SgVarArgEndOp, SgVarArgCopyOp, SgNewExp, SgRefExp, SgAggregateInitializer, SgCompoundInitializer,
  // SgConstructorInitializer, SgAssignInitializer, SgPseudoDestructorRefExp.

     bool returnValue = false;

  // Need to add: SgUpcLocalsizeofExpression, SgUpcBlocksizeofExpression, SgUpcElemsizeofExpression.

  // There are no access functions for SgJavaAnnotation, SgJavaTypeExpression, SgJavaTypeExpression,

  // DQ (11/10/2014): Added support for SgFunctionParameterRefExp node to store the type explicitly.

     if ( isSgTemplateParameterVal(this) != NULL     || isSgComplexVal(this)    != NULL            || isSgSizeOfOp(this) != NULL                 ||
          isSgAlignOfOp(this) != NULL                || isSgJavaInstanceOfOp(this) != NULL         || isSgTypeIdOp(this)  != NULL                ||
          isSgVarArgStartOp(this) != NULL            || isSgVarArgStartOneOperandOp(this) != NULL  || isSgVarArgOp(this)  != NULL                ||
          isSgVarArgEndOp(this) != NULL              || isSgVarArgCopyOp(this) != NULL             || isSgNewExp(this)  != NULL                  ||
          isSgRefExp(this) != NULL                   || isSgAggregateInitializer(this) != NULL     || isSgCompoundInitializer(this)  != NULL     ||
          isSgUpcLocalsizeofExpression(this) != NULL || isSgUpcBlocksizeofExpression(this) != NULL || isSgUpcElemsizeofExpression(this)  != NULL ||
          isSgJavaAnnotation(this) != NULL           || isSgJavaTypeExpression(this) != NULL       || isSgJavaTypeExpression(this)  != NULL      ||
       // isSgConstructorInitializer(this) != NULL   || isSgAssignInitializer(this) != NULL        || isSgPseudoDestructorRefExp(this)  != NULL  ||
          isSgConstructorInitializer(this) != NULL   || isSgPseudoDestructorRefExp(this)  != NULL  || isSgFunctionParameterRefExp(this) != NULL  ||
       // PP (04/22/22) store type of Attribute directly in AST
          isSgAdaAttributeExp(this) != NULL
          )
        {
          returnValue = true;
        }

     return returnValue;
   }

void
SgExpression::set_explicitly_stored_type( SgType* type )
   {
  // DQ (3/7/2014): Some expressions store internal SgType pointers explicitly, this allows these IR nodes to be reset with new types (used in the snippet support).

     switch (this->variantT())
        {
           case V_SgNewExp:
             {
               SgNewExp* exp = isSgNewExp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_specified_type(type);
               break;
             }

          case V_SgConstructorInitializer:
             {
               SgConstructorInitializer* exp = isSgConstructorInitializer(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_expression_type(type);
               break;
             }

          case V_SgTemplateParameterVal:
             {
               SgTemplateParameterVal* exp = isSgTemplateParameterVal(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_valueType(type);
               break;
             }

          case V_SgComplexVal:
             {
               SgComplexVal* exp = isSgComplexVal(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_precisionType(type);
               break;
             }

          case V_SgSizeOfOp:
             {
               SgSizeOfOp* exp = isSgSizeOfOp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_operand_type(type);
               break;
             }

          case V_SgAlignOfOp:
             {
               SgAlignOfOp* exp = isSgAlignOfOp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_operand_type(type);
               break;
             }

          case V_SgJavaInstanceOfOp:
             {
               SgJavaInstanceOfOp* exp = isSgJavaInstanceOfOp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_operand_type(type);
               break;
             }

          case V_SgTypeIdOp:
             {
               SgTypeIdOp* exp = isSgTypeIdOp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_operand_type(type);
               break;
             }

          case V_SgVarArgStartOp:
             {
               SgVarArgStartOp* exp = isSgVarArgStartOp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_expression_type(type);
               break;
             }

          case V_SgVarArgStartOneOperandOp:
             {
               SgVarArgStartOneOperandOp* exp = isSgVarArgStartOneOperandOp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_expression_type(type);
               break;
             }

          case V_SgVarArgOp:
             {
               SgVarArgOp* exp = isSgVarArgOp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_expression_type(type);
               break;
             }

          case V_SgVarArgEndOp:
             {
               SgVarArgEndOp* exp = isSgVarArgEndOp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_expression_type(type);
               break;
             }

          case V_SgVarArgCopyOp:
             {
               SgVarArgCopyOp* exp = isSgVarArgCopyOp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_expression_type(type);
               break;
             }

          case V_SgRefExp:
             {
               SgRefExp* exp = isSgRefExp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_type_name(type);
               break;
             }

          case V_SgAggregateInitializer:
             {
               SgAggregateInitializer* exp = isSgAggregateInitializer(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_expression_type(type);
               break;
             }

          case V_SgCompoundInitializer:
             {
               SgCompoundInitializer* exp = isSgCompoundInitializer(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_expression_type(type);
               break;
             }

          case V_SgPseudoDestructorRefExp:
             {
               SgPseudoDestructorRefExp* exp = isSgPseudoDestructorRefExp(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_object_type(type);
               break;
             }

           case V_SgUpcLocalsizeofExpression:
             {
               SgUpcLocalsizeofExpression* exp = isSgUpcLocalsizeofExpression(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_operand_type(type);
               break;
             }

           case V_SgUpcBlocksizeofExpression:
             {
               SgUpcBlocksizeofExpression* exp = isSgUpcBlocksizeofExpression(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_operand_type(type);
               break;
             }

           case V_SgUpcElemsizeofExpression:
             {
               SgUpcElemsizeofExpression* exp = isSgUpcElemsizeofExpression(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_operand_type(type);
               break;
             }

        // DQ (3/7/20145): This IR node is generated without access functions for the type.
           case V_SgJavaTypeExpression:
             {
               SgJavaTypeExpression* exp = isSgJavaTypeExpression(this);
               ROSE_ASSERT(exp != NULL);
               exp->set_type(type);
               break;
             }

           case V_SgAdaAttributeExp:
             {
               SgAdaAttributeExp* exp = isSgAdaAttributeExp(this);
               ASSERT_not_null(exp);
               exp->set_attrType(type);
               break;
             }

          default:
             {
               printf ("Error: SgExpression::set_explicit_type(): default reached: expression = %p = %s \n",this,this->class_name().c_str());
               ROSE_ASSERT(false);
             }
        }
   }
