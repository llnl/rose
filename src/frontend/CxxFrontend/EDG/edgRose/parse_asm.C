
#include "edgRose.h"

SgAsmStmt*
EDG_ROSE_Translation::parse_asm(SourceSequenceList& sse)
   {
     an_asm_entry_ptr p = (an_asm_entry_ptr)(sse->entity.ptr);
     ++sse;

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("parse asm statement"); debug_print("\n");
     ScopedIndent si(indent);
#endif
  
#if 0
     printf ("In parse_asm(): calling summarize_asm_entry() \n");
     summarize_asm_entry(p);
#endif

  // FIXME: This needs a build function!
     SgAsmStmt* asmStmt = new SgAsmStmt();
     ROSE_ASSERT (asmStmt != NULL);

  // DQ (11/3/2012): Setup the default source position information.
     setSourcePosition(asmStmt);

     ROSE_ASSERT (p->asm_string != NULL);
     string asmString = convert_string_constant(p->asm_string);

     SgExpression* asmStringExp = set_file_info_for_corresp(buildStringVal_nfi(asmString), p->source_corresp);

#ifdef DEBUG_CONVERSION
     printf ("In parse_asm(): asmString = %s (warning: the operands are not unparsed) \n",asmString.c_str());
#endif
     asmStringExp->set_parent(asmStmt);

     ROSE_ASSERT (asmStringExp != NULL);

  // DQ (2/4/2014): I think my asm implementation needs to reference the previous edg3x version.
  // asmStmt->get_operands().push_back(asmStringExp);
  // asmStmt->set_assemblyCode(asmStringExp);
     asmStmt->set_assemblyCode(asmString);

     asmStmt->set_definingDeclaration(asmStmt);

  // DQ (1/26/2009): Also set the firstNondefiningDeclaration.
     asmStmt->set_firstNondefiningDeclaration(asmStmt);

#if 1
     asmStmt->set_useGnuExtendedFormat(false);
#if GNU_EXTENSIONS_ALLOWED
     if (p->operands != NULL || p->clobbers != NULL) 
        {
          asmStmt->set_useGnuExtendedFormat(true);
        }

  // DQ (2/4/2014): Note that this is frequently marked as volatile in EDG when it is not marked explicitly in the source code.
     if (p->is_volatile == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("In parse_asm(): Volatile is set! (supressed in IR because it is not explicit in the source code) \n");
#endif
       // Set the isVolatile bit in the typeModifier held in the declarationModifier
       // asmStmt->get_declarationModifier().get_typeModifier().get_constVolatileModifier().setVolatile();
        }

     an_asm_operand_ptr asm_operand_ptr = p->operands;
     while (asm_operand_ptr != NULL)
        {
          string operand_name = asm_operand_ptr->name != NULL ? asm_operand_ptr->name : "";
#if 0
          printf ("In parse_asm(): operand_name = %s \n",operand_name.c_str());
#endif

#if RECORD_RAW_ASM_OPERAND_DESCRIPTIONS
       // TRUE if this entry is for an output operand description.  Otherwise, this represents an input operand description.
#if 0
          bool output_operand = asm_operand_ptr->is_output_operand;
          printf ("In parse_asm(): output_operand     = %s \n",output_operand ? "true" : "false");
#endif
// The constraint string as it appeared in the source.
          string constraints_string;
          if (asm_operand_ptr->constraints_string != NULL)
             {
               constraints_string = asm_operand_ptr->constraints_string;
             }
#if 0
          printf ("In parse_asm(): constraints_string = %s \n",constraints_string.c_str());
          printf ("In parse_asm(): asmStringExp       = %p = %s \n",asmStringExp,asmStringExp->class_name().c_str());
#endif

       // DQ (2/4/2014): Select default values for constraints (I expect that we want to use the constraints_string for the more general implementation.
          SgAsmOp::asm_operand_constraint_enum constraint = (SgAsmOp::asm_operand_constraint_enum) SgAsmOp::e_any;
          SgAsmOp::asm_operand_modifier_enum   modifiers  = (SgAsmOp::asm_operand_modifier_enum)   SgAsmOp::e_unknown;
          SgAsmOp* sageAsmOp = new SgAsmOp(constraint,modifiers,asmStringExp);
          asmStringExp->set_parent(sageAsmOp);
#else
          an_asm_operand_constraint_ptr operand_constraint_ptr = asm_operand_ptr->constraints;

       // This is a byte sized bit vector (see EDG: il_def.h file for more details)
       // printf ("asm_operand_ptr->modifiers   = %o \n",asm_operand_ptr->modifiers);
       // printf ("asm_operand_ptr->expression  = %p \n",asm_operand_ptr->expression);
       // generateFileInfo(asm_operand_ptr->position)->display("location of asm operand");
          SgExpression* tempExp = NULL;
          bool outputOperand = false;
          if (asm_operand_ptr->expression != NULL)
             {
            // DQ (7/23/2006): Output operands need to generate a lvalue (and we have to hand this directly)!
            // tempExp = sage_gen_expression(asm_operand_ptr->expression);
               if (asm_operand_ptr->modifiers & (an_asm_operand_modifier)aom_output)
                  {
                    outputOperand = true;
                    tempExp = sage_gen_lvalue(asm_operand_ptr->expression,sourcePositionData);
                  }
                 else
                  {
                    tempExp = sage_gen_expression(asm_operand_ptr->expression,NULL);
                  }

               ROSE_ASSERT(tempExp != NULL);
            // printf ("tempExp from asm_operand_ptr->expression = %p = %s \n",tempExp,tempExp->class_name().c_str());
            // tempExp->get_startOfConstruct()->display("location of asm operand expression");
             }

          ROSE_ASSERT(operand_constraint_ptr != NULL);
          SgAsmOp::asm_operand_constraint_enum constraint = (SgAsmOp::asm_operand_constraint_enum) operand_constraint_ptr->kind;
          SgAsmOp::asm_operand_modifier_enum   modifiers  = (SgAsmOp::asm_operand_modifier_enum)   asm_operand_ptr->modifiers;
          SgAsmOp* sageAsmOp = new SgAsmOp(constraint,modifiers,tempExp);
          tempExp->set_parent(sageAsmOp);

          DataRequiredForComputationOfSourcePostionInformation sourcePositionData(asm_operand_ptr->expression);
          sourcePositionData.initializeSourcePositionInformation(sageAsmOp);
#endif

          asmStmt->get_operands().push_back(sageAsmOp);
          sageAsmOp->set_parent(asmStmt);
          asm_operand_ptr = asm_operand_ptr->next;
       }

  // DQ (7/26/2006): If the named registers in the clobber list match gnu standard names then the clobber list is not ignored!
     if (p->clobbers != NULL)
        {
          a_named_register_list_ptr registerList = p->clobbers;
          while (registerList != NULL)
             {
               a_named_register registerName = registerList->reg;
               SgInitializedName::asm_register_name_enum sageRegisterName = (SgInitializedName::asm_register_name_enum) registerName;
               asmStmt->get_clobberRegisterList().push_back(sageRegisterName);
               registerList = registerList->next;
             }
        }
#endif

  // DQ (2/4/2014): Can there be a list of these that we need to handle for a single __asm__() statement?
  // Note that the edg3x implementation supported a list of these, but I am unclear if that was the correct implementation.
  // ROSE_ASSERT(p->next == NULL);
#endif

  // return set_file_info_for_corresp(asmStmt, p->source_corresp);
     SgAsmStmt* returnAsmStmt = set_file_info_for_corresp(asmStmt, p->source_corresp);

  // DQ (7/15/2012): Added test for parent.
     ROSE_ASSERT(returnAsmStmt->get_parent() == NULL);

  // DQ (7/15/2012): Added code to set the parent.
     returnAsmStmt->set_parent(topScopeStack());

  // DQ (7/15/2012): Added test for parent.
     ROSE_ASSERT(returnAsmStmt->get_parent() != NULL);

     return returnAsmStmt;

   }
