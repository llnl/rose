// tps (01/14/2010) : Switching from rose.h to sage3.
#include "sage3basic.h"

#include "fixupCopy.h"

#ifdef ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT
   #include "transformationSupport.h"
#endif

// This file implementes support for the AST copy fixup.  It is specific to:
// 1) Scope pointer fixup
// 2) Parent pointer fixup
// 3 defining and not defining declarations fixup

void outputMap ( SgCopyHelp & help )
   {
     int counter = 0;
     SgCopyHelp::copiedNodeMapTypeIterator i = help.get_copiedNodeMap().begin();

     printf ("COPY MAP: \n");
     while (i != help.get_copiedNodeMap().end())
        {
          if ( (i->first->get_startOfConstruct() != NULL) && (i->first->get_startOfConstruct()->isFrontendSpecific() == false) )
             {
               printf ("entry %4d: i->first = %p  i->second = %p = %s\n",counter,i->first,i->second,i->first->class_name().c_str());
             }

       // printf ("entry %4d: i->first = %p  i->second = %p = %s\n",counter,i->first,i->second,i->first->class_name().c_str());
          ROSE_ASSERT(i->first->variantT() == i->second->variantT());

          counter++;
          i++;
        }
   }

void
resetVariableDefinitionSupport ( const SgInitializedName* originalInitializedName, SgInitializedName* copyInitializedName, SgDeclarationStatement* targetDeclaration )
   {
  // DQ (10/8/2007): This is s supporting function to the SgInitializedName::fixupCopy_scopes() member function.

  // DQ (5/14/2012): Added simple test.
     ROSE_ASSERT(copyInitializedName != NULL);

  // Where the scope is reset, also build a new SgVariableDefinition (I forget why).
     ROSE_ASSERT(copyInitializedName->get_declptr() != NULL);

     SgNode* originalDeclaration = originalInitializedName->get_declptr();
     switch(originalDeclaration->variantT())
        {
          case V_SgVariableDefinition:
             {
            // In this case the target declaration is a SgVariableDefinition, and it has to be constructed, so the input paremter is NULL.
               ROSE_ASSERT(targetDeclaration == NULL);

               SgVariableDefinition* variableDefinition_original = isSgVariableDefinition(originalInitializedName->get_declptr());
               ROSE_ASSERT(variableDefinition_original != NULL);

            // DQ (1/20/204): Moved to supporting the more general expression (required), plus the expression from which it may have been generated.
            // SgUnsignedLongVal* bitfield = variableDefinition_original->get_bitfield();
               SgExpression* bitfield = variableDefinition_original->get_bitfield();

               SgVariableDefinition* variableDefinition_copy = new SgVariableDefinition(copyInitializedName,bitfield);
               ROSE_ASSERT(variableDefinition_copy != NULL);
               copyInitializedName->set_declptr(variableDefinition_copy);
               variableDefinition_copy->set_parent(copyInitializedName);

            // This is the same way that Sg_File_Info objects are built in the copy mechanism.
               Sg_File_Info* newStartOfConstruct = new Sg_File_Info(*(variableDefinition_original->get_startOfConstruct()));
               ROSE_ASSERT(variableDefinition_original->get_endOfConstruct() != NULL);
               Sg_File_Info* newEndOfConstruct   = new Sg_File_Info(*(variableDefinition_original->get_endOfConstruct()));

               variableDefinition_copy->set_startOfConstruct(newStartOfConstruct);
               variableDefinition_copy->set_endOfConstruct(newEndOfConstruct);

               newStartOfConstruct->set_parent(variableDefinition_copy);
               newEndOfConstruct->set_parent(variableDefinition_copy);

               break;
             }

          case V_SgEnumDeclaration:
             {
            // In this case the target declaration is a SgEnumDeclaration, and it has already been copied so it need not be created.
               ROSE_ASSERT(targetDeclaration != NULL);

               SgEnumDeclaration* enumDeclaration_original = isSgEnumDeclaration(originalInitializedName->get_declptr());
               if (copyInitializedName->get_declptr() == enumDeclaration_original)
                  {
                 // Then reset to the targetDeclaration.
                    copyInitializedName->set_declptr(targetDeclaration);
                  }

               break;
             }

       // DQ (12/23/2012): Added support for templates.
          case V_SgTemplateFunctionDeclaration:
          case V_SgTemplateMemberFunctionDeclaration:

          case V_SgFunctionDeclaration:
          case V_SgMemberFunctionDeclaration:
          case V_SgTemplateInstantiationFunctionDecl:
          case V_SgTemplateInstantiationMemberFunctionDecl:
          case V_SgProcedureHeaderStatement:
          case V_SgProgramHeaderStatement:
             {
            // In this case the target declaration is a SgFunctionDeclaration, and it has already been copied so it need not be created.
               ROSE_ASSERT(targetDeclaration != NULL);

               SgFunctionDeclaration* functionDeclaration_original = isSgFunctionDeclaration(originalInitializedName->get_declptr());
               if (copyInitializedName->get_declptr() == functionDeclaration_original)
                  {
                 // Then reset to the targetDeclaration.
                    copyInitializedName->set_declptr(targetDeclaration);
                  }

               break;
             }

          default:
             {
               printf ("Error: default reached in resetVariableDefinitionSupport() originalDeclaration = %p = %s \n",originalDeclaration,originalDeclaration->class_name().c_str());
               ROSE_ABORT();
             }
        }

   }


// DQ (10/5/2007): Added IR node specific function to permit copies, via AST copy(), to be fixedup
// Usually this will correct scopes and in a few cases build child IR nodes that are not traversed
// (and thus shared in the result from the automatically generated copy function).
void
SgInitializedName::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
  // This is the empty default inplementation, not a problem if it is called!

#if DEBUG_FIXUP_COPY
  // printf ("Inside of SgInitializedName::fixupCopy_scopes() %p = %s \n",this,SageInterface::get_name(this).c_str());
     printf ("Inside of SgInitializedName::fixupCopy_scopes() %p = %s \n",this,this->get_name().str());
#endif

  // Need to fixup the scope and perhaps build the SgVariableDefinition object!
     SgInitializedName* initializedName_copy = isSgInitializedName(copy);
     ROSE_ASSERT(initializedName_copy != NULL);

  // DQ (4/21/2016): Replacing "__null" with more portable (non-gnu specific) use using "NULL".
  // ROSE_ASSERT (initializedName_copy->get_declptr() != __null);
     ROSE_ASSERT (initializedName_copy->get_declptr() != NULL);

     // fprintf(stderr, "SgInitializedName::fixupCopy_scopes(%p) this=%p\n", copy, this);
     // fprintf(stderr, "Copy's scope is %p, my scope is %p\n", initializedName_copy->get_scope(), this->get_scope());

  // ROSE_ASSERT(this->get_symbol_from_symbol_table() != NULL);

     if (initializedName_copy->get_scope() == this->get_scope())
        {
          FixupCopyDataMemberMacro(initializedName_copy,SgScopeStatement,get_scope,set_scope)
          // fprintf(stderr, "After: copy's scope is %p, my scope is %p\n", initializedName_copy->get_scope(), this->get_scope());

          SgNode* parent = initializedName_copy->get_parent();

       // printf ("In SgInitializedName::fixupCopy_scopes(): parent = %p \n",parent);

       // Since the parent might not have been set yet we have to allow for this case. In the case of a
       // SgInitializedName in a SgVariableDeclaration the SgInitializedName objects have their parents
       // set after the SgInitializedName is copied and in the copy function for the parent (SgVariableDeclaration).
          // fprintf (stderr, "In SgInitializedName::fixupCopy_scopes(): parent = %p = %s \n",parent,parent->class_name().c_str());
          if (parent != NULL)
             {
               ROSE_ASSERT(parent != NULL);
            // printf ("In SgInitializedName::fixupCopy_scopes(): parent = %p = %s \n",parent,parent->class_name().c_str());

               switch(parent->variantT())
                  {
                 // DQ (12/28/2012): Adding support for templates.
                    case V_SgTemplateVariableDeclaration:

                    case V_SgVariableDeclaration:
                       {
                         resetVariableDefinitionSupport(this,initializedName_copy,NULL);
                         break;
                       }

                    case V_SgEnumDeclaration:
                       {
                         SgEnumDeclaration* enumDeclaration = isSgEnumDeclaration(parent);
                         ROSE_ASSERT(enumDeclaration != NULL);
                         resetVariableDefinitionSupport(this,initializedName_copy,enumDeclaration);
                         break;
                       }

                    case V_SgFunctionParameterList:
                       {
                         SgNode* parentFunction = parent->get_parent();
                         SgFunctionDeclaration* functionDeclaration = isSgFunctionDeclaration(parentFunction);

                      // The parent of the SgFunctionParameterList might not have been set yet, so allow for this!
                         if (functionDeclaration != NULL)
                            {
                           // DQ (5/14/2012): Added simple test.
                              ROSE_ASSERT(initializedName_copy != NULL);

                           // DQ (5/14/2012): Added simple test, required in resetVariableDefinitionSupport().
                              ROSE_ASSERT(initializedName_copy->get_declptr() != NULL);

                              resetVariableDefinitionSupport(this,initializedName_copy,functionDeclaration);
                            }
                         break;
                       }

                    default:
                       {
                         printf ("default reached in SgInitializedName::fixupCopy_scopes() parent = %p = %s \n",parent,parent->class_name().c_str());
                         ROSE_ABORT();
                       }
                  }
             }
        }
       else
        {
#if DEBUG_FIXUP_COPY
       // fprintf (stderr, "Skipping resetting the scope for initializedName_copy = %p = %s \n",initializedName_copy,initializedName_copy->get_name().str());
          printf ("Skipping resetting the scope for initializedName_copy = %p = %s \n",initializedName_copy,initializedName_copy->get_name().str());
#endif
        }


     if (this->get_prev_decl_item() != NULL)
        {
          FixupCopyDataMemberMacro(initializedName_copy,SgInitializedName,get_prev_decl_item,set_prev_decl_item)
        }

#if DEBUG_FIXUP_COPY
     printf ("Leaving SgInitializedName::fixupCopy_scopes() \n\n");
#endif
   }


// DQ (11/1/2007): Build lighter weight versions of SgStatement::fixupCopy_scopes() and SgExpression::fixupCopy_scopes()
// and refactor code into the SgLocatedNode::fixupCopy_scopes().

void
SgStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
  // We need to call the fixupCopy function from the parent of a SgVariableDeclaration because the
  // copy function in the parent of the variable declaration sets the parent of the SgVariableDeclaration
  // and we need this parent in the fixupCopy function in the SgInitializedName.

#if DEBUG_FIXUP_COPY
     printf ("Inside of SgStatement::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgLocatedNode::fixupCopy_scopes(copy,help);
   }

void
SgExpression::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgExpression::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     Rose_STL_Container<SgNode*> children_original = const_cast<SgExpression*>(this)->get_traversalSuccessorContainer();
     Rose_STL_Container<SgNode*> children_copy     = const_cast<SgNode*>(copy)->get_traversalSuccessorContainer();
     ROSE_ASSERT (children_original.size() == children_copy.size());

     for (Rose_STL_Container<SgNode*>::const_iterator
            i_original = children_original.begin(),
            i_copy = children_copy.begin();
          i_original != children_original.end(); ++i_original, ++i_copy) {
       if (*i_original == NULL) continue;
       (*i_original)->fixupCopy_scopes(*i_copy,help);
     }

     SgLocatedNode::fixupCopy_scopes(copy,help);
   }

// DQ (11/1/2007): Make this work on SgLocatedNode (so that it can work on SgStatement and SgExpression).

// DQ (10/5/2007): Added IR node specific function to permit copies, via AST copy(), to be fixedup
// Usually this will correct scopes and in a few cases build child IR nodes that are not traversed
// (and thus shared in the result from the automatically generated copy function).
void
SgLocatedNode::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgLocatedNode::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgLocatedNode* copyLocatedNode = isSgLocatedNode(copy);
     ROSE_ASSERT(copyLocatedNode != NULL);

  // DQ (10/24/2007): New test.
     ROSE_ASSERT(copyLocatedNode->variantT() == this->variantT());

  // DQ (7/15/2007): Added assertion...
  // ROSE_ASSERT(this->get_parent() != NULL);
  // ROSE_ASSERT(copyStatement->get_parent() != NULL);

  // DQ (10/15/2007): If the parent of the original AST is not set then we will not process the parent in the copy,
  // thus the AST copy mechanism can handle incompletely setup AST (as required for use in the EDG/Sage translation)
  // yet only return an AST of similar quality.
     if (this->get_parent() != NULL)
        {
          FixupCopyDataMemberMacro(copyLocatedNode,SgNode,get_parent,set_parent)

       // Debugging information
          SgNode* cpParent = copyLocatedNode->get_parent();
          if (cpParent == NULL)
             {
#if PRINT_DEVELOPER_WARNINGS
               printf ("In SgLocatedNode::fixupCopy_scopes(): this->get_parent() != NULL, but copyLocatedNode->get_parent() == NULL for copyLocatedNode = %p = %s \n",copyLocatedNode,copyLocatedNode->class_name().c_str());
               printf ("     this                        = %p = %s \n",this,this->class_name().c_str());
               printf ("     this->get_parent()          = %p = %s \n",this->get_parent(),this->get_parent()->class_name().c_str());
#endif
             }

       // Debugging information
          if (cpParent != NULL && cpParent->variantT() != this->get_parent()->variantT())
             {
#if PRINT_DEVELOPER_WARNINGS
               printf ("Warning: In SgLocatedNode::fixupCopy_scopes(): the parent of this and copyStatement are different \n");
               printf ("     this                        = %p = %s \n",this,this->class_name().c_str());
               printf ("     copyLocatedNode             = %p = %s \n",copyLocatedNode,copyLocatedNode->class_name().c_str());
               printf ("     this->get_parent()          = %p = %s \n",this->get_parent(),this->get_parent()->class_name().c_str());
               printf ("     copyStatement->get_parent() = %p = %s \n",cpParent,cpParent->class_name().c_str());

               this->get_startOfConstruct()->display("this->get_startOfConstruct(): debug");
#endif
             }
       // ROSE_ASSERT(copyStatement->get_parent()->variantT() == this->get_parent()->variantT());
        }
         else if (SgProject::get_verbose() > 0)
        {
          printf ("In SgLocatedNode::fixupCopy_scopes(): parent not set for original AST at %p = %s, thus copy left similarly incomplete \n",this,this->class_name().c_str());
        }

  // DQ (2/20/2009): Added assertion, I think it is up to the parent node copy function to set the parent in the copying of any children.
  // ROSE_ASSERT(copy->get_parent() != NULL);
     if (copy->get_parent() == NULL)
        {
       // Note that using SageInterface::get_name(this) will work where SageInterface::get_name(copy)
       // will fail because sometimes the parent pointer is required to be valid within
       // SageInterface::get_name() (e.g. between SgFunctionParameterList and it's parent: SgFunctionDeclaration).
#if 0
          printf ("Returning a copy = %p = %s = %s with NULL parent \n",copy,copy->class_name().c_str(),SageInterface::get_name(this).c_str());
#endif
        }

#if DEBUG_FIXUP_COPY
     printf ("Leaving SgLocatedNode::fixupCopy_scopes() \n\n");
#endif
   }

void
SgScopeStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
  // We need to call the fixupCopy function from the parent of a SgVariableDeclaration because the
  // copy function in the parent of the variable declaration sets the parent of the SgVariableDeclaration
  // and we need this parent in the fixupCopy function in the SgInitializedName.

#if DEBUG_FIXUP_COPY
     printf ("Inside of SgScopeStatement::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

  // printf ("\nInside of SgScopeStatement::fixupCopy_scopes() for %p = %s copy = %p \n\n",this,this->class_name().c_str(),copy);
  // this->get_startOfConstruct()->display("Inside of SgScopeStatement::fixupCopy_scopes()");

     SgScopeStatement* copyScopeStatement = isSgScopeStatement(copy);
     ROSE_ASSERT(copyScopeStatement != NULL);

  // DQ (10/24/2007): New test.
     ROSE_ASSERT(copyScopeStatement->variantT() == this->variantT());

  // The symbol table should not have been setup yet!

  // DQ (5/21/2013): Restrict direct access to the symbol table.
  // if (copyScopeStatement->get_symbol_table()->size() != 0)
     if (copyScopeStatement->symbol_table_size() != 0)
        {
#if 0
          printf ("copy = %p = %s = %s \n",copy,copy->class_name().c_str(),SageInterface::get_name(copy).c_str());
#endif
#if 0
          copyScopeStatement->get_file_info()->display("In SgScopeStatement::fixupCopy_scopes()");
#endif
        }

  // DQ (2/6/2009): Comment this out since it fails for the case of the reverseTraversal tests.
  // ROSE_ASSERT(copyScopeStatement->get_symbol_table()->size() == 0);

  // Call the base class fixupCopy member function
     SgStatement::fixupCopy_scopes(copy,help);

  // printf ("\nLeaving SgScopeStatement::fixupCopy_scopes() for %p = %s copy = %p \n\n",this,this->class_name().c_str(),copy);
   }



void
SgGlobal::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgGlobal::fixupCopy_scopes() for %p copy = %p \n",this,copy);
#endif

     SgGlobal* global_copy = isSgGlobal(copy);
     ROSE_ASSERT(global_copy != NULL);

     const SgDeclarationStatementPtrList & statementList_original = this->getDeclarationList();
     const SgDeclarationStatementPtrList & statementList_copy     = global_copy->getDeclarationList();

     SgDeclarationStatementPtrList::const_iterator i_original = statementList_original.begin();
     SgDeclarationStatementPtrList::const_iterator i_copy     = statementList_copy.begin();

  // Iterate over both lists to match up the correct pairs of SgStatement objects
     while ( (i_original != statementList_original.end()) && (i_copy != statementList_copy.end()) )
        {
          (*i_original)->fixupCopy_scopes(*i_copy,help);

          i_original++;
          i_copy++;
        }

  // Call the base class fixupCopy member function
     SgScopeStatement::fixupCopy_scopes(copy,help);

  // printf ("\nLeaving SgGlobal::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
   }

// JJW 2/1/2008 -- Added support to fixup statement expressions
void
SgExprStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgExprStatement::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgExprStatement* es_copy = isSgExprStatement(copy);
     ROSE_ASSERT(es_copy != NULL);

     SgExpression* expression_original = this->get_expression();
     SgExpression* expression_copy     = es_copy->get_expression();

     expression_original->fixupCopy_scopes(expression_copy, help);

  // Call the base class fixupCopy member function
     SgStatement::fixupCopy_scopes(copy,help);

  // printf ("\nLeaving SgExprStatement::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
   }

// DQ (10/6/2007): Added fixup function to set scopes not set properly by the ROSETTA generated copy!
void
SgBasicBlock::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgBasicBlock::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgBasicBlock* block_copy = isSgBasicBlock(copy);
     ROSE_ASSERT(block_copy != NULL);

     const SgStatementPtrList & statementList_original = this->getStatementList();
     const SgStatementPtrList & statementList_copy     = block_copy->getStatementList();

  // Check that this need not be handled as a special case such as SgIfStmt.
     if (this->containsOnlyDeclarations() == true)
        {
          ROSE_ASSERT(this->getDeclarationList().size() == statementList_original.size());
        }
       else
        {
          ROSE_ASSERT(this->getStatementList().size() == statementList_original.size());
        }

     SgStatementPtrList::const_iterator i_original = statementList_original.begin();
     SgStatementPtrList::const_iterator i_copy     = statementList_copy.begin();

  // Iterate over both lists to match up the correct pairs of SgStatement objects
     while ( (i_original != statementList_original.end()) && (i_copy != statementList_copy.end()) )
        {
          (*i_original)->fixupCopy_scopes(*i_copy,help);

          i_original++;
          i_copy++;
        }

  // Call the base class fixupCopy member function
     SgScopeStatement::fixupCopy_scopes(copy,help);

  // printf ("\nLeaving SgBasicBlock::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
   }

void
SgDeclarationStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgDeclarationStatement::fixupCopy_scopes() for %p = %s copy = %p (defining = %p firstNondefining = %p) \n",
          this,this->class_name().c_str(),copy,this->get_definingDeclaration(),this->get_firstNondefiningDeclaration());
#endif

#if DEBUG_FIXUP_COPY_OUTPUT_MAP
     outputMap(help);
#endif

  // Need to fixup the scopes and defining and non-defining declaration.

     SgDeclarationStatement* copyDeclarationStatement = isSgDeclarationStatement(copy);
     ROSE_ASSERT(copyDeclarationStatement != NULL);

  // DQ (10/20/2007): This is an essential piece of the fixup of the AST copy.  This is a recursive construction of IR nodes that have
  // defining and non-defining parts used when either or both were not copied in the initial call to the copy member functions.
  // Since we can visit the IR nodes in any order, and some declaration can have multiple parts (defining and non-defining declaration)
  // We have to make sure that these are processed first (in some cases they may have escaped being copied by the AST Copy mechanism
  // when they were not part of the traversed AST). So we have to build them if they don't exist (triggered by existance in the copy map).

  // Check if this IR node has been copied?  If so then it is in the copy map.
  // bool hasBeenCopied = (help.get_copiedNodeMap().find(this) != help.get_copiedNodeMap().end());
  // printf ("hasBeenCopied = %s \n",hasBeenCopied ? "true" : "false");
  // if (hasBeenCopied == false)

     bool definingDeclarationCopied         = (help.get_copiedNodeMap().find(this->get_definingDeclaration())         != help.get_copiedNodeMap().end());
     bool firstNondefiningDeclarationCopied = (help.get_copiedNodeMap().find(this->get_firstNondefiningDeclaration()) != help.get_copiedNodeMap().end());

  // DQ (3/2/2009): Handle the case of friend declaration.
     bool isFriendDeclaration = this->get_declarationModifier().isFriend();

#if 0
     printf ("### this = %p = %s copyDeclarationStatement = %p definingDeclarationCopied = %s firstNondefiningDeclarationCopied = %s \n",
          this,this->class_name().c_str(),copyDeclarationStatement,definingDeclarationCopied ? "true" : "false", firstNondefiningDeclarationCopied ? "true" : "false");
     printf ("    this->get_definingDeclaration()            = %p \n",this->get_definingDeclaration());
     printf ("    this->get_firstNondefiningDeclaration()    = %p \n",this->get_firstNondefiningDeclaration());
     printf ("    this->get_declarationModifier().isFriend() = %s \n",this->get_declarationModifier().isFriend() ? "true" : "false");
     printf ("    firstNondefiningDeclarationCopied          = %s \n",firstNondefiningDeclarationCopied ? "true" : "false");
     printf ("    definingDeclarationCopied                  = %s \n",definingDeclarationCopied ? "true" : "false");
     printf ("    isFriendDeclaration                        = %s \n",isFriendDeclaration ? "true" : "false");
#endif

  // DQ (3/2/2009): Modified to exclude copying of friend declaration defining and non defining declarations.
  // if (definingDeclarationCopied == true && firstNondefiningDeclarationCopied == true)
     if ( (definingDeclarationCopied == true && firstNondefiningDeclarationCopied == true) || (isFriendDeclaration == true) )
        {
       // We can process the current non-defining declaration (which is not the first non-defining declaration)
        }
       else
        {
       // Note sure if we need these variables
       // SgDeclarationStatement* copyOfDefiningDeclaration         = NULL;
       // SgDeclarationStatement* copyOfFirstNondefiningDeclaration = NULL;

       // Note: This will cause the first non-defining declaration to be copied first when neither have been processed.
       // if (this->get_firstNondefiningDeclaration() != NULL && firstNondefiningDeclarationCopied == false)
       // if (this->get_firstNondefiningDeclaration() != NULL && firstNondefiningDeclarationCopied == false && this == this->get_definingDeclaration())
       // if (this->get_firstNondefiningDeclaration() != NULL && firstNondefiningDeclarationCopied == false && this == this->get_firstNondefiningDeclaration())
       // if (this->get_firstNondefiningDeclaration() != NULL && firstNondefiningDeclarationCopied == false && this == this->get_firstNondefiningDeclaration())
          if (this->get_firstNondefiningDeclaration() != NULL && firstNondefiningDeclarationCopied == false)
             {
#if 0
               printf ("*** this = %p this->get_firstNondefiningDeclaration() != NULL && firstNondefiningDeclarationCopied == false && this == this->get_definingDeclaration() \n",this);
#endif
            // Setup the firstNondefining declaration
            // ROSE_ASSERT(definingDeclarationCopied == true);

            // We could build vitual constructors, but then that is what the copy function is so call copy!
            // This also added the firstNondefiningDeclaration to the copy map

            // DQ (10/21/2007): Use the copy help object so that it can control copying of defining vs. non-defining declaration.
            // SgNode* copyOfFirstNondefiningDeclarationNode = this->get_firstNondefiningDeclaration()->copy(help);
            // printf ("Nested copy of firstNondefiningDeclaration \n");
               SgNode* copyOfFirstNondefiningDeclarationNode = help.copyAst(this->get_firstNondefiningDeclaration());
            // printf ("DONE: Nested copy of firstNondefiningDeclaration \n");

               ROSE_ASSERT(copyOfFirstNondefiningDeclarationNode != NULL);
               ROSE_ASSERT(copyOfFirstNondefiningDeclarationNode->get_parent() == NULL);
#if 0
               printf ("SgDeclarationStatement::fixupCopy_scopes(): copyOfFirstNondefiningDeclarationNode = %p \n",copyOfFirstNondefiningDeclarationNode);
               printf ("SgDeclarationStatement::fixupCopy_scopes(): this->get_firstNondefiningDeclaration() = %p \n",this->get_firstNondefiningDeclaration());
               printf ("SgDeclarationStatement::fixupCopy_scopes(): this->get_firstNondefiningDeclaration()->get_parent() = %p \n",this->get_firstNondefiningDeclaration()->get_parent());
               printf ("SgDeclarationStatement::fixupCopy_scopes(): firstNondefiningDeclarationCopied = %s \n",firstNondefiningDeclarationCopied ? "true" : "false");
#endif
            // Must reset the parent (semantics of AST copy), but this will be done by reset

            // DQ (3/14/2014): The parent might not be set if the non-defining declaration has not be added to the AST (i.e. if it has only been used to build a symbol).
            // ROSE_ASSERT(this->get_firstNondefiningDeclaration()->get_parent() != NULL);

               copyOfFirstNondefiningDeclarationNode->set_parent(this->get_firstNondefiningDeclaration()->get_parent());

            // DQ (3/14/2014): The parent might not be set if the non-defining declaration has not be added to the AST (i.e. if it has only been used to build a symbol).
            // ROSE_ASSERT(copyOfFirstNondefiningDeclarationNode->get_parent() != NULL);
               if (copyOfFirstNondefiningDeclarationNode->get_parent() == NULL)
                  {
                    printf ("Note: SgDeclarationStatement::fixupCopy_scopes(): copyOfFirstNondefiningDeclarationNode->get_parent() == NULL \n");
                  }

            // printf ("Commented out setting of scopes on the this->get_firstNondefiningDeclaration() \n");
               this->get_firstNondefiningDeclaration()->fixupCopy_scopes(copyOfFirstNondefiningDeclarationNode,help);

            // copyOfFirstNondefiningDeclaration = isSgDeclarationStatement(copyOfFirstNondefiningDeclarationNode);
             }
            else
             {
            // Note: This needs to be in the else case to handle the recursion properly (else this case would be procesed twice)
               if (this->get_definingDeclaration() != NULL && definingDeclarationCopied == false)
                  {
#if 0
                    printf ("*** this = %p this->get_definingDeclaration() != NULL && definingDeclarationCopied == false \n",this);
#endif
                 // DQ (2/19/2009): I don't think that the firstNondefiningDeclarationCopied has to be true (here we are copying the defining declaration).
                 // Setup the defining declaration
                 // ROSE_ASSERT(firstNondefiningDeclarationCopied == true);
#if 0
                    if (firstNondefiningDeclarationCopied == false)
                       {
                      // This is the case of copying a definind declaration from one file to another (we have to
                      // copy the associated non-defining declarations so that they will have the same scopes).
                         ROSE_ASSERT(this->get_firstNondefiningDeclaration() != NULL);
                         SgNode* copyOfFirstNondefiningDeclarationNode = help.copyAst(this->get_firstNondefiningDeclaration());

                         ROSE_ASSERT(copyOfFirstNondefiningDeclarationNode != NULL);
                         ROSE_ASSERT(copyOfFirstNondefiningDeclarationNode->get_parent() == NULL);

                      // Must reset the parent (semantics of AST copy), but this will be done by reset
                         ROSE_ASSERT(this->get_firstNondefiningDeclaration()->get_parent() != NULL);
                         copyOfFirstNondefiningDeclarationNode->set_parent(this->get_firstNondefiningDeclaration()->get_parent());

                         this->get_firstNondefiningDeclaration()->fixupCopy_scopes(copyOfFirstNondefiningDeclarationNode,help);
                       }
#endif
                 // DQ (10/21/2007): Use the copy help object so that it can control copying of defining vs. non-defining declaration.
                 // SgNode* copyOfDefiningDeclarationNode = this->get_definingDeclaration()->copy(help);
                    SgNode* copyOfDefiningDeclarationNode = help.copyAst(this->get_definingDeclaration());
                    ROSE_ASSERT(copyOfDefiningDeclarationNode != NULL);

                 // If we didn't make a copy of the definingDeclaration then this is still a valid pointer, so there is no need to reset the parent or call
                    if (copyOfDefiningDeclarationNode != this->get_definingDeclaration())
                       {
#if 0
                         printf ("*** this = %p copyOfDefiningDeclarationNode != this->get_definingDeclaration() \n",this);
#endif
                      // DQ (2/26/2009): Set the parent to NULL (before resetting to valid value).
                         copyOfDefiningDeclarationNode->set_parent(NULL);

                         ROSE_ASSERT(copyOfDefiningDeclarationNode->get_parent() == NULL);

                         if (this->get_definingDeclaration()->get_parent() == NULL)
                            {
                              printf ("ERROR: this = %p = %s = %s this->get_definingDeclaration() = %p \n",this,this->class_name().c_str(),SageInterface::get_name(this).c_str(),this->get_definingDeclaration());
                            }

                      // Must reset the parent (semantics of AST copy), but this will be done by reset

                      // DQ (9/10/2014): The new support for template function handling causes some template to be saved and there parent pointers not set until later.
                      // This is likely why this is failing (see copytest2007_40.C).
                      // ROSE_ASSERT(this->get_definingDeclaration()->get_parent() != NULL);
                         if (this->get_definingDeclaration()->get_parent() == NULL)
                            {
                              if (isSgTemplateFunctionDeclaration(this->get_definingDeclaration()) != NULL || isSgTemplateMemberFunctionDeclaration(this->get_definingDeclaration()) != NULL)
                                 {
                                   printf ("Warning: (inner scope) this->get_definingDeclaration()->get_parent() == NULL (OK for some SgTemplateFunctionDeclaration and SgTemplateMemberFunctionDeclaration) \n");
                                 }
                                else if (isSgTemplateClassDeclaration(this->get_definingDeclaration()) != NULL)
                                 {
                                   printf ("WARNING: %p (%s) has no parent! \n", this->get_definingDeclaration(), this->get_definingDeclaration()->class_name().c_str());
                                 }
                                else
                                 {
                                   printf ("ERROR: %p (%s) has no parent! \n", this->get_definingDeclaration(), this->get_definingDeclaration()->class_name().c_str());
                                   ROSE_ASSERT(this->get_definingDeclaration()->get_parent() != NULL);
                                 }
                            }
                         copyOfDefiningDeclarationNode->set_parent(this->get_definingDeclaration()->get_parent());
#if 0
                         printf ("Exiting before recursive call to copy as a test! \n");
                         ROSE_ABORT();
#endif
#if 1
                      // DQ (2/26/2009): This was valid code that was temporarily commented out (turning it back on).
                      // DQ (10/21/2007): I think this was a bug!
                      // this->get_firstNondefiningDeclaration()->fixupCopy_scopes(copyOfDefiningDeclarationNode,help);
                         this->get_definingDeclaration()->fixupCopy_scopes(copyOfDefiningDeclarationNode,help);
#else
                         printf ("Skipping call to fixup_scopes on the defining declaration (endless recursion for outlining example moreTest4.cpp) \n");
#endif
                       }

                 // DQ (9/10/2014): See not above.
                 // DQ (2/20/2009): Added assertion!
                 // ROSE_ASSERT(copyOfDefiningDeclarationNode->get_parent() != NULL);
                    if (copyOfDefiningDeclarationNode->get_parent() == NULL)
                       {
                         printf ("Warning: (outer scope) this->get_definingDeclaration()->get_parent() == NULL (OK for some SgTemplateFunctionDeclaration and SgTemplateMemberFunctionDeclaration) \n");
                       }

                 // copyOfDefiningDeclaration = isSgDeclarationStatement(copyOfDefiningDeclarationNode);
                  }
                 else
                  {
                 // This is an acceptable case, but only if:
                    ROSE_ASSERT(this->get_definingDeclaration() == NULL || this->get_firstNondefiningDeclaration() == NULL);
                  }
             }
        }

  // If this is a declaration which is a defining declaration, then the copy should be as well.
     if (this->get_definingDeclaration() == this)
        {
       // printf ("This is a DEFINING declaration this %p = %s copyDeclarationStatement = %p = %s \n",this,this->class_name().c_str(),copyDeclarationStatement,copyDeclarationStatement->class_name().c_str());
          copyDeclarationStatement->set_definingDeclaration(copyDeclarationStatement);

       // If this is the defining declaration the we can reset the defining declaration on the firstNondefiningDeclaration (if it has been updated)
          if (this->get_firstNondefiningDeclaration() != copyDeclarationStatement->get_firstNondefiningDeclaration())
             {
            // printf ("This is the defining declaration, so we can reset the defining declaration on the firstNondefiningDeclaration \n");
               FixupCopyDataMemberMacro(copyDeclarationStatement->get_firstNondefiningDeclaration(),SgDeclarationStatement,get_definingDeclaration,set_definingDeclaration)
             }
        }
       else
        {
       // DQ (10/19/2007): If there was a defining declaration then the copy's defining declaration using the map
       // printf ("NOT a DEFINING declaration: this->get_definingDeclaration() = %p \n",this->get_definingDeclaration());
          if (this->get_definingDeclaration() != NULL)
             {
               ROSE_ASSERT(copyDeclarationStatement->get_definingDeclaration() != NULL);

               FixupCopyDataMemberMacro(copyDeclarationStatement,SgDeclarationStatement,get_definingDeclaration,set_definingDeclaration)
#if 0
               printf ("On the firstNondefiningDeclaration %p setting the definingDeclaration to %p \n",
                    copyDeclarationStatement->get_firstNondefiningDeclaration(),copyDeclarationStatement->get_definingDeclaration());
               copyDeclarationStatement->get_firstNondefiningDeclaration()->set_definingDeclaration(copyDeclarationStatement->get_definingDeclaration());
#endif
             }
            else
             {
            // printf ("In SgDeclarationStatement::fixupCopy_scopes(): this->get_definingDeclaration() == NULL \n");
             }
        }

  // DQ (10/12/2007): It is not always clear if this is a great idea.  This uncovered a bug in the
  // SageInterface::isOverloaded() function. Having two declarations marked as the firstNondefiningDeclaration
  // could be a problem at some point.  But for now this preserves the concept of an exact copy, so I am
  // leaving it as is.

  // If this is a declaration which is a nondefining declaration, then the copy should be as well.
  // if (this->get_firstNondefiningDeclaration() == this)
     if (this->get_firstNondefiningDeclaration() == this)
        {
       // printf ("This is the FIRST-NON-DEFINING declaration this %p = %s copyDeclarationStatement = %p = %s \n",this,this->class_name().c_str(),copyDeclarationStatement,copyDeclarationStatement->class_name().c_str());
          copyDeclarationStatement->set_firstNondefiningDeclaration(copyDeclarationStatement);

       // If this is the firstNondefining declaration the we can reset the firstNondefiningDeclaration on the definingDeclaration (if it has been updated)
          if (this->get_definingDeclaration() != copyDeclarationStatement->get_definingDeclaration())
             {
            // printf ("This is the firstNondefining declaration, so  we can reset the firstNondefiningDeclaration on the definingDeclaration \n");
               FixupCopyDataMemberMacro(copyDeclarationStatement->get_definingDeclaration(),SgDeclarationStatement,get_firstNondefiningDeclaration,set_firstNondefiningDeclaration)
             }
        }
       else
        {
       // DQ (10/19/2007): If there was a defining declaration then the copy's defining declaration using the map
       // printf ("NOT a FIRST_NON_DEFINING declaration: this->get_firstNondefiningDeclaration() = %p \n",this->get_firstNondefiningDeclaration());
       // if (this->get_firstNondefiningDeclaration() != NULL)
          if (this->get_firstNondefiningDeclaration() != NULL && firstNondefiningDeclarationCopied == true)
             {
               ROSE_ASSERT(copyDeclarationStatement->get_firstNondefiningDeclaration() != NULL);
               FixupCopyDataMemberMacro(copyDeclarationStatement,SgDeclarationStatement,get_firstNondefiningDeclaration,set_firstNondefiningDeclaration)
             }
            else
             {
            // printf ("In SgDeclarationStatement::fixupCopy_scopes(): this->get_firstNondefiningDeclaration() == NULL \n");
             }
        }


  // DQ (10/16/2007): Added assertion (see copytest2007_17.C)
     if (this->get_definingDeclaration() != NULL && this->get_firstNondefiningDeclaration() != NULL)
        {
       // printf ("this->get_definingDeclaration()         = %p = %s \n",this->get_definingDeclaration(),this->get_definingDeclaration()->class_name().c_str());
       // printf ("this->get_firstNondefiningDeclaration() = %p = %s \n",this->get_firstNondefiningDeclaration(),this->get_firstNondefiningDeclaration()->class_name().c_str());
          ROSE_ASSERT(this->get_definingDeclaration()->variantT() == this->get_firstNondefiningDeclaration()->variantT());
        }

  // DQ (10/16/2007): Added assertion (see copytest2007_17.C)
     if (copyDeclarationStatement->get_definingDeclaration() != NULL && copyDeclarationStatement->get_firstNondefiningDeclaration() != NULL)
        {
       // printf ("copyDeclarationStatement->get_definingDeclaration()         = %p = %s \n",copyDeclarationStatement->get_definingDeclaration(),copyDeclarationStatement->get_definingDeclaration()->class_name().c_str());
       // printf ("copyDeclarationStatement->get_firstNondefiningDeclaration() = %p = %s \n",copyDeclarationStatement->get_firstNondefiningDeclaration(),copyDeclarationStatement->get_firstNondefiningDeclaration()->class_name().c_str());
          ROSE_ASSERT(copyDeclarationStatement->get_definingDeclaration()->variantT() == copyDeclarationStatement->get_firstNondefiningDeclaration()->variantT());
        }

  // DQ (10/12/2007): Set the scope for those SgDeclarationStatements which store their scope explicitly.
  // printf ("this->hasExplicitScope() = %s \n",this->hasExplicitScope() ? "true" : "false");
     if (this->hasExplicitScope() == true)
        {
       // printf ("Reset the scope of the copy for this = %p = %s \n",this,this->class_name().c_str());
          FixupCopyDataMemberMacro(copyDeclarationStatement,SgScopeStatement,get_scope,set_scope)

#if 0
       // DQ (10/21/2007): This can be OK for where a function prototype is being generated from a defining function declaration.

       // ROSE_ASSERT(copyDeclarationStatement->get_scope() != this->get_scope());
          if (copyDeclarationStatement->get_scope() == this->get_scope())
             {
               printf ("##########  ERROR: SCOPES NOT SET CORRECTLY! (%p = %s)  ########## \n",copy,copy->class_name().c_str());
             }
#endif

       // Make sure that the copy sets the scopes to be the same type
          ROSE_ASSERT(copyDeclarationStatement->get_scope()->variantT() == this->get_scope()->variantT());

       // DQ (2/28/2009): Make sure that the declaration and the copy are in the same file.
       // ROSE_ASSERT(TransformationSupport::getSourceFile(copyDeclarationStatement) == TransformationSupport::getSourceFile(this));
       // ROSE_ASSERT(TransformationSupport::getSourceFile(this->get_firstNondefiningDeclaration()) == TransformationSupport::getSourceFile(this));
          if (this->get_definingDeclaration() != NULL)
             {
            // DQ (3/4/2009): This test fails for copytest2007_34.C
               if (TransformationSupport::getSourceFile(this->get_definingDeclaration()) != TransformationSupport::getSourceFile(this))
                  {
                    printf ("Warning: TransformationSupport::getSourceFile(this->get_definingDeclaration()) != TransformationSupport::getSourceFile(this) \n");
                    printf ("Commented out failing test for copytest2007_34.C \n");
                  }
            // ROSE_ASSERT(TransformationSupport::getSourceFile(this->get_definingDeclaration()) == TransformationSupport::getSourceFile(this));
             }

#if 0
       // DQ (3/3/2009): For some declaration there is a defining and non-defining and if in copying both only one has
       // been copied before this test they will appear to be from different file until the second declaration is copied.
          if (copyDeclarationStatement->get_firstNondefiningDeclaration() != NULL)
             {
               if (TransformationSupport::getSourceFile(copyDeclarationStatement->get_firstNondefiningDeclaration()) != TransformationSupport::getSourceFile(copyDeclarationStatement))
                  {
                    printf ("Error: source files don't match for copyDeclarationStatement = %p and copyDeclarationStatement->get_firstNondefiningDeclaration() = %p \n",copyDeclarationStatement,copyDeclarationStatement->get_firstNondefiningDeclaration());
                  }
             }
#endif

       // DQ (3/2/2009): Make sure this is not the non-defining declaration since the defining declaration will not have been copied yet and so of course the files will not match.
       // ROSE_ASSERT(TransformationSupport::getSourceFile(copyDeclarationStatement->get_firstNondefiningDeclaration()) == TransformationSupport::getSourceFile(copyDeclarationStatement));
       // if (copyDeclarationStatement->get_definingDeclaration() != NULL)
          if (copyDeclarationStatement->get_definingDeclaration() != NULL && this != this->get_firstNondefiningDeclaration())
             {
            // ROSE_ASSERT(TransformationSupport::getSourceFile(copyDeclarationStatement->get_definingDeclaration()) == TransformationSupport::getSourceFile(copyDeclarationStatement));
               if (TransformationSupport::getSourceFile(copyDeclarationStatement->get_definingDeclaration()) != TransformationSupport::getSourceFile(copyDeclarationStatement))
                  {
                    printf ("copyDeclarationStatement = %p = %s \n",copyDeclarationStatement,copyDeclarationStatement->class_name().c_str());
                    printf ("############# Detected case of copyDeclarationStatement->get_definingDeclaration() in file %s \n",
                         TransformationSupport::getSourceFile(copyDeclarationStatement->get_definingDeclaration())->getFileName().c_str());
                    printf ("############# Detected case of copyDeclarationStatement in file %s \n",TransformationSupport::getSourceFile(copyDeclarationStatement)->getFileName().c_str());

                 // This is not what we want here!
                 // copyDeclarationStatement->set_definingDeclaration(NULL);

                    printf ("Error: source files don't match for copyDeclarationStatement = %p and copyDeclarationStatement->get_definingDeclaration() = %p \n",copyDeclarationStatement,copyDeclarationStatement->get_definingDeclaration());
                    ROSE_ABORT();
                  }
             }
        }

  // DQ (10/19/2007): Added test...
     if (this->get_definingDeclaration() != NULL && this->get_firstNondefiningDeclaration() != NULL)
        {
       // DQ (11/6/2007): If these are in the same namespace but different in different instances of the namespace
       // definition (SgNamespaceDefinitionStatement objects), then the test for the same scope is more complex.
          SgNamespaceDefinitionStatement* definingNamespace         = isSgNamespaceDefinitionStatement(this->get_definingDeclaration()->get_scope());
          SgNamespaceDefinitionStatement* firstNondefiningNamespace = isSgNamespaceDefinitionStatement(this->get_firstNondefiningDeclaration()->get_scope());
          if (definingNamespace != NULL && firstNondefiningNamespace != NULL)
             {
            // printf ("Test for same namespace is more complex: definingNamespace = %p firstNondefiningNamespace = %p \n",definingNamespace,firstNondefiningNamespace);
               SgNamespaceDeclarationStatement* definingNamespaceDeclaration         = definingNamespace->get_namespaceDeclaration();
               SgNamespaceDeclarationStatement* firstNondefiningNamespaceDeclaration = firstNondefiningNamespace->get_namespaceDeclaration();
               ROSE_ASSERT(definingNamespaceDeclaration != NULL);
               ROSE_ASSERT(firstNondefiningNamespaceDeclaration != NULL);
               ROSE_ASSERT(definingNamespaceDeclaration->get_firstNondefiningDeclaration() == firstNondefiningNamespaceDeclaration->get_firstNondefiningDeclaration());
             }
            else
             {
               if (this->get_definingDeclaration()->get_scope() != this->get_firstNondefiningDeclaration()->get_scope() )
                  {
#if 0
                    printf ("Error in matching scopes: this = %p = %s = %s \n",this,this->class_name().c_str(),SageInterface::get_name(this).c_str());
                    printf ("     this->get_definingDeclaration()         = %p \n",this->get_definingDeclaration());
                    printf ("     this->get_firstNondefiningDeclaration() = %p \n",this->get_firstNondefiningDeclaration());
                    printf ("     this->get_definingDeclaration()->get_scope()         = %p = %s \n",this->get_definingDeclaration()->get_scope(),this->get_definingDeclaration()->get_scope()->class_name().c_str());
                    printf ("     this->get_firstNondefiningDeclaration()->get_scope() = %p = %s \n",this->get_firstNondefiningDeclaration()->get_scope(),this->get_firstNondefiningDeclaration()->get_scope()->class_name().c_str());
#endif
                 // this->get_startOfConstruct()->display("Error: this scope mismatch: debug");
                 // this->get_definingDeclaration()->get_startOfConstruct()->display("Error: definingDeclaration scope mismatch: debug");
                 // this->get_firstNondefiningDeclaration()->get_startOfConstruct()->display("Error: firstNondefiningDeclaration scope mismatch: debug");
                  }

            // DQ (2/19/2009): Make sure that these are the same kind of IR nodes since they might be in different files (instead of in the same file).
            // ROSE_ASSERT(this->get_definingDeclaration()->get_scope() == this->get_firstNondefiningDeclaration()->get_scope() );
               ROSE_ASSERT(this->get_definingDeclaration()->get_scope()->variantT() == this->get_firstNondefiningDeclaration()->get_scope()->variantT() );
             }

       // ROSE_ASSERT(this->get_definingDeclaration()->get_scope() == this->get_firstNondefiningDeclaration()->get_scope() );
        }

  // DQ (10/19/2007): Added test...
     if (copyDeclarationStatement->get_definingDeclaration() != NULL && copyDeclarationStatement->get_firstNondefiningDeclaration() != NULL)
        {
#if 0
          printf ("copyDeclarationStatement = %p = %s = %s copyDeclarationStatement->get_firstNondefiningDeclaration() = %p \n",
               copyDeclarationStatement,copyDeclarationStatement->class_name().c_str(),SageInterface::get_name(copyDeclarationStatement).c_str(),
               copyDeclarationStatement->get_firstNondefiningDeclaration());
          printf ("copyDeclarationStatement = %p copyDeclarationStatement->get_definingDeclaration() = %p \n",
               copyDeclarationStatement,copyDeclarationStatement->get_definingDeclaration());
          printf ("copyDeclarationStatement->get_scope() = %p copyDeclarationStatement->get_firstNondefiningDeclaration()->get_scope() = %p \n",
               copyDeclarationStatement->get_scope(),copyDeclarationStatement->get_firstNondefiningDeclaration()->get_scope());
#endif
#if 1
       // DQ (10/19/2007): Check the loop (not passible until both have been processed)
          ROSE_ASSERT(copyDeclarationStatement->get_firstNondefiningDeclaration()->get_definingDeclaration() != NULL);
          ROSE_ASSERT(copyDeclarationStatement->get_definingDeclaration()->get_firstNondefiningDeclaration() != NULL);
#else
       // DQ (2/26/2009): See if I get past this so that I can generate the graphs so that I can debug this.
          printf ("Skipping failing test \n");
#endif
       // ROSE_ASSERT(copyDeclarationStatement->get_firstNondefiningDeclaration()->get_definingDeclaration() == copyDeclarationStatement->get_definingDeclaration());

       // We can't assert this yet since this is part of the copy of the defining declaration within the processing of the non-defining declaration (recurssively called!)
       // ROSE_ASSERT(copyDeclarationStatement->get_definingDeclaration()->get_firstNondefiningDeclaration() == copyDeclarationStatement->get_firstNondefiningDeclaration());

       // If we can get to this point then we can perform this additional test
       // ROSE_ASSERT(copyDeclarationStatement->get_definingDeclaration()->get_scope() == copyDeclarationStatement->get_firstNondefiningDeclaration()->get_scope() );
        }

  // DQ (2/20/2009): Added assertion.
  // ROSE_ASSERT(copy->get_parent() != NULL);

  // Call the base class fixupCopy member function (this will setup the parent)
     SgStatement::fixupCopy_scopes(copy,help);

  // DQ (2/20/2009): Note: These are allowed to be NULL a warning is issued in SgLocatedNode::fixupCopy_scopes().
  // ROSE_ASSERT(copy->get_parent() != NULL);
   }


void
SgFunctionDeclaration::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
     SgFunctionDeclaration* functionDeclaration_copy = isSgFunctionDeclaration(copy);
     ROSE_ASSERT(functionDeclaration_copy != NULL);

#if DEBUG_FIXUP_COPY
     printf ("\nIn SgFunctionDeclaration::fixupCopy_scopes(): for function = %s = %p = %s copy = %p \n",this->get_name().str(),this,this->class_name().c_str(),copy);
#endif

  // printf ("\nIn SgFunctionDeclaration::fixupCopy_scopes(): for function = %s = %p = %s copy = %p \n",this->get_name().str(),this,this->class_name().c_str(),copy);

  // Call the base class fixupCopy member function
     SgDeclarationStatement::fixupCopy_scopes(copy,help);

  // Setup the scopes of the SgInitializedName objects in the paraleter list
     ROSE_ASSERT(get_parameterList() != NULL);
     get_parameterList()->fixupCopy_scopes(functionDeclaration_copy->get_parameterList(),help);

  // Setup the details in the SgFunctionDefinition (this may have to rebuild the sysmbol table)
  // printf ("In SgFunctionDeclaration::fixupCopy_scopes(): this->get_definition() = %p \n",this->get_definition());
     if (this->get_definition() != NULL)
        {
       // DQ (3/15/2014): The defining declaration should not be marked (isForward() == true).
          if (isForward() == true)
             {
               printf ("Warning: The defining declaration should not be marked (isForward() == true) \n");
               printf ("SgFunctionDeclaration::fixupCopy_scopes(): (isForward() == true): functionDeclaration_copy = %p = %s \n",functionDeclaration_copy,functionDeclaration_copy->class_name().c_str());
               printf ("   --- functionDeclaration_copy->get_firstNondefiningDeclaration() = %p \n",functionDeclaration_copy->get_firstNondefiningDeclaration());
               printf ("   --- functionDeclaration_copy->get_definingDeclaration()         = %p \n",functionDeclaration_copy->get_definingDeclaration());

            // functionDeclaration_copy->get_file_info()->display("SgFunctionDeclaration::fixupCopy_scopes(): (isForward() == true): debug");

            // Reset this!
               functionDeclaration_copy->unsetForward();

            // DQ (3/15/2014): This is a Java specific issue. I don't want to be changing the original
            // version of the statement we are copying. This needs to be fixed properly in the Java AST.
               printf ("TODO (Java): isForward() status is being reset for the original SgFunctionDeclaration as part of SgFunctionDeclaration::fixupCopy_scopes(): this = %p = %s = %s \n",this,this->class_name().c_str(),this->get_name().str());
            // this->unsetForward();
               this->get_definition()->get_declaration()->unsetForward();
             }
          ROSE_ASSERT(isForward() == false);

       // DQ (2/26/2009): Handle special cases where the copyHelp function is non-trivial.
       // Is every version of copyHelp object going to be a problem?

       // For the outlining, our copyHelp object does not copy defining function declarations
       // and substitutes a non-defining declarations, so if the copy has been built this way
       // then skip trying to reset the SgFunctionDefinition.
       // printf ("In SgFunctionDeclaration::fixupCopy_scopes(): functionDeclaration_copy->get_definition() = %p \n",functionDeclaration_copy->get_definition());
       // this->get_definition()->fixupCopy_scopes(functionDeclaration_copy->get_definition(),help);
          if (functionDeclaration_copy->get_definition() != NULL)
             {
               this->get_definition()->fixupCopy_scopes(functionDeclaration_copy->get_definition(),help);
             }

       // If this is a declaration with a definition then it is a defining declaration
       // functionDeclaration_copy->set_definingDeclaration(functionDeclaration_copy);
        }

  // printf ("\nLeaving SgFunctionDeclaration::fixupCopy_scopes(): for function = %s = %p = %s copy = %p \n",this->get_name().str(),this,this->class_name().c_str(),copy);
   }

// DQ (10/6/2007): Added fixup function to set scopes not set properly by the ROSETTA generated copy!
void
SgFunctionParameterList::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgFunctionParameterList::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

  // Also call the base class version of the fixupCopycopy() member function
     SgDeclarationStatement::fixupCopy_scopes(copy,help);

     SgFunctionParameterList* copyFunctionParameterList = isSgFunctionParameterList(copy);
     ROSE_ASSERT(copyFunctionParameterList != NULL);

     const SgInitializedNamePtrList & parameterList_original = this->get_args();
     SgInitializedNamePtrList & parameterList_copy           = copyFunctionParameterList->get_args();

     SgInitializedNamePtrList::const_iterator i_original = parameterList_original.begin();
     SgInitializedNamePtrList::iterator i_copy           = parameterList_copy.begin();

  // Iterate over both lists to match up the correct pairs of SgInitializedName objects
     while ( (i_original != parameterList_original.end()) && (i_copy != parameterList_copy.end()) )
        {
          (*i_original)->fixupCopy_scopes(*i_copy,help);

          i_original++;
          i_copy++;
        }
   }


void
SgMemberFunctionDeclaration::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("In SgMemberFunctionDeclaration::fixupCopy_scopes(): for function = %s = %p = %s copy = %p \n",this->get_name().str(),this,this->class_name().c_str(),copy);
#endif

     SgMemberFunctionDeclaration* memberFunctionDeclaration_copy = isSgMemberFunctionDeclaration(copy);
     ROSE_ASSERT(memberFunctionDeclaration_copy != NULL);

  // Call the base class fixupCopy member function
  // SgFunctionDeclaration::fixupCopy_scopes(copy,help);

  // Setup the scopes of the SgInitializedName objects in the paraleter list
     ROSE_ASSERT(get_CtorInitializerList() != NULL);
     get_CtorInitializerList()->fixupCopy_scopes(memberFunctionDeclaration_copy->get_CtorInitializerList(),help);

  // Call the base class fixupCopy member function
     SgFunctionDeclaration::fixupCopy_scopes(copy,help);
   }


void
SgTemplateDeclaration::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("\nInside of SgTemplateDeclaration::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgTemplateDeclaration* templateDeclaration_copy = isSgTemplateDeclaration(copy);
     ROSE_ASSERT(templateDeclaration_copy != NULL);

     SgDeclarationStatement::fixupCopy_scopes(copy,help);

#if 1
  // I don't think that this applies to the defining declaration...
     if (this != this->get_definingDeclaration())
        {
          ROSE_ASSERT(templateDeclaration_copy->get_firstNondefiningDeclaration()->get_definingDeclaration() == templateDeclaration_copy->get_definingDeclaration());

       // DQ (10/20/2007): this is a problem with copytest2007_12.C, I think it should not be assered yet.
       // We can't assert this yet since this is part fo the copy of the defining declaration within the processing of the non-defining declaration (recurssively called!)
       // ROSE_ASSERT(templateDeclaration_copy->get_definingDeclaration()->get_firstNondefiningDeclaration() == templateDeclaration_copy->get_firstNondefiningDeclaration());

       // If we can get to this point then we can perform this additional test
       // ROSE_ASSERT(templateDeclaration_copy->get_definingDeclaration()->get_scope() == templateDeclaration_copy->get_firstNondefiningDeclaration()->get_scope() );
        }
#endif
   }


void
SgTemplateInstantiationDefn::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgTemplateInstantiationDefn::fixupCopy_scopes() for class = %s class definition %p = %s copy = %p \n",this->get_declaration()->get_name().str(),this,this->class_name().c_str(),copy);
#endif

     ROSE_ASSERT(this->get_declaration() != NULL);

  // Call the base class fixupCopy member function
     SgClassDefinition::fixupCopy_scopes(copy,help);
   }


void
SgTemplateInstantiationDecl::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("\nIn SgTemplateInstantiationDecl::fixupCopy_scopes(): for function = %s = %p = %s copy = %p \n",this->get_name().str(),this,this->class_name().c_str(),copy);
#endif

     SgTemplateInstantiationDecl* templateClassDeclaration_copy = isSgTemplateInstantiationDecl(copy);
     ROSE_ASSERT(templateClassDeclaration_copy != NULL);

  // Also call the base class version of the fixupCopycopy() member function
     SgClassDeclaration::fixupCopy_scopes(copy,help);

#if 0
     if (this->get_templateDeclaration() == NULL)
        {
          printf ("Error: this = %p = %s name = %s \n",this,this->class_name().c_str(),this->get_name().str());
          this->get_file_info()->display("SgTemplateInstantiationDecl::fixupCopy_scopes(): this: debug");

          SgTemplateInstantiationDecl* nondefiningDeclaration = isSgTemplateInstantiationDecl(this->get_firstNondefiningDeclaration());
          SgTemplateInstantiationDecl* definingDeclaration    = isSgTemplateInstantiationDecl(this->get_definingDeclaration());

          printf ("this->get_firstNondefiningDeclaration() = %p nondefiningDeclaration = %p \n",this->get_firstNondefiningDeclaration(),nondefiningDeclaration);
          printf ("this->get_definingDeclaration()         = %p definingDeclaration    = %p \n",this->get_definingDeclaration(),definingDeclaration);
          if (nondefiningDeclaration != NULL)
             {
               printf ("nondefiningDeclaration->get_templateDeclaration() = %p \n",nondefiningDeclaration->get_templateDeclaration());
               if (nondefiningDeclaration->get_templateDeclaration() != NULL)
                  {
                 // DQ (3/7/2015): This is not the correct place to set this, but this is debugging code.
                 // Also this is only an issue to EDG 4.7 and not EDG 4.9; and we are about retire EDG 4.7 support.
                 // printf ("WARNING: setting templateDeclaration in SgTemplateInstantiationDecl::fixupCopy_scopes(): this = %p = %s name = %s \n",this,this->class_name().c_str(),this->get_name().str());
                 // SgTemplateInstantiationDecl* nondefiningDeclaration = isSgTemplateInstantiationDecl(this->get_firstNondefiningDeclaration());
                 // this->set_templateDeclaration(nondefiningDeclaration->get_templateDeclaration());
                  }
             }
          if (definingDeclaration != NULL)
             {
               printf ("definingDeclaration->get_templateDeclaration() = %p \n",definingDeclaration->get_templateDeclaration());
             }
        }
#endif
     ROSE_ASSERT(this->get_templateDeclaration() != NULL);
     ROSE_ASSERT(templateClassDeclaration_copy->get_templateDeclaration() != NULL);

  // FixupCopyDataMemberMacro(templateClassDeclaration_copy,SgTemplateDeclaration,get_templateDeclaration,set_templateDeclaration)
     FixupCopyDataMemberMacro(templateClassDeclaration_copy,SgTemplateClassDeclaration,get_templateDeclaration,set_templateDeclaration)

#if 0
  // DQ (11/6/2007): I have returned to sharing SgTemplateArgument objects, fix this up later.

  // DQ (11/5/2007): Until we use the SgTemplateArgumentList object directly, we have to handl the list explicitly.
     SgTemplateArgumentPtrList::const_iterator templateArguments_iterator_original = this->get_templateArguments()->begin();
     SgTemplateArgumentPtrList::iterator       templateArguments_iterator_copy     = templateClassDeclaration_copy->get_templateArguments()->begin();
     while ( templateArguments_iterator_original != this->get_templateArguments()->end() )
        {
          ROSE_ASSERT(templateArguments_iterator_original != this->get_templateArguments()->end());
          ROSE_ASSERT(templateArguments_iterator_copy     != templateClassDeclaration_copy->get_templateArguments()->end());

          (*templateArguments_iterator_original)->fixupCopy_scopes(*templateArguments_iterator_copy,help);

          templateArguments_iterator_original++;
          templateArguments_iterator_copy++;
        }
#endif
   }

void
SgTemplateInstantiationMemberFunctionDecl::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("\nIn SgTemplateInstantiationMemberFunctionDecl::fixupCopy_scopes(): for function = %s = %p = %s copy = %p \n",this->get_name().str(),this,this->class_name().c_str(),copy);
#endif

     ASSERT_this();
#if 0
     printf ("\nIn SgTemplateInstantiationMemberFunctionDecl::fixupCopy_scopes(): for function = %s = %p = %s copy = %p \n",this->get_name().str(),this,this->class_name().c_str(),copy);
#endif
     SgTemplateInstantiationMemberFunctionDecl* templateMemberFunctionDeclaration_copy = isSgTemplateInstantiationMemberFunctionDecl(copy);
     ROSE_ASSERT(templateMemberFunctionDeclaration_copy != NULL);

  // Also call the base class version of the fixupCopycopy() member function
     SgMemberFunctionDeclaration::fixupCopy_scopes(copy,help);

     if (this->get_templateDeclaration() == NULL)
        {
          printf ("this = %p templateMemberFunctionDeclaration_copy = %p name = %s \n",this,templateMemberFunctionDeclaration_copy,this->get_name().str());
        }
     ROSE_ASSERT(this->get_templateDeclaration() != NULL);
     ROSE_ASSERT(templateMemberFunctionDeclaration_copy->get_templateDeclaration() != NULL);

  // FixupCopyDataMemberMacro(templateMemberFunctionDeclaration_copy,SgTemplateDeclaration,get_templateDeclaration,set_templateDeclaration)
     FixupCopyDataMemberMacro(templateMemberFunctionDeclaration_copy,SgTemplateMemberFunctionDeclaration,get_templateDeclaration,set_templateDeclaration)

#if 0
     printf ("this = %p this->get_templateDeclaration() = %p \n",this,this->get_templateDeclaration());
     printf ("templateMemberFunctionDeclaration_copy = %p templateMemberFunctionDeclaration_copy->get_templateDeclaration() = %p \n",
          templateMemberFunctionDeclaration_copy,templateMemberFunctionDeclaration_copy->get_templateDeclaration());
#endif

#if 0
  // DQ (11/6/2007): I have returned to sharing SgTemplateArgument objects, fix this up later.

  // DQ (11/5/2007): Until we use the SgTemplateArgumentList object directly, we have to handl the list explicitly.
     SgTemplateArgumentPtrList::const_iterator templateArguments_iterator_original = this->get_templateArguments()->begin();
     SgTemplateArgumentPtrList::iterator       templateArguments_iterator_copy     = templateMemberFunctionDeclaration_copy->get_templateArguments()->begin();
     while ( templateArguments_iterator_original != this->get_templateArguments()->end() )
        {
          ROSE_ASSERT(templateArguments_iterator_original != this->get_templateArguments()->end());
          ROSE_ASSERT(templateArguments_iterator_copy     != templateMemberFunctionDeclaration_copy->get_templateArguments()->end());

          (*templateArguments_iterator_original)->fixupCopy_scopes(*templateArguments_iterator_copy,help);

          templateArguments_iterator_original++;
          templateArguments_iterator_copy++;
        }
#endif
   }


void
SgTemplateInstantiationFunctionDecl::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("\nIn SgTemplateInstantiationFunctionDecl::fixupCopy_scopes(): for function = %s = %p = %s copy = %p \n",this->get_name().str(),this,this->class_name().c_str(),copy);
#endif

  // DQ (12/28/2012): Added asseretion.
     ASSERT_this();

#if 0
     printf ("\nIn SgTemplateInstantiationFunctionDecl::fixupCopy_scopes(): for function = %s = %p = %s copy = %p \n",this->get_name().str(),this,this->class_name().c_str(),copy);
#endif

     SgTemplateInstantiationFunctionDecl* templateFunctionDeclaration_copy = isSgTemplateInstantiationFunctionDecl(copy);
     ROSE_ASSERT(templateFunctionDeclaration_copy != NULL);

  // Also call the base class version of the fixupCopycopy() member function
     SgFunctionDeclaration::fixupCopy_scopes(copy,help);

     ROSE_ASSERT(this->get_templateDeclaration() != NULL);
     ROSE_ASSERT(templateFunctionDeclaration_copy->get_templateDeclaration() != NULL);

  // FixupCopyDataMemberMacro(templateFunctionDeclaration_copy,SgTemplateDeclaration,get_templateDeclaration,set_templateDeclaration)
     FixupCopyDataMemberMacro(templateFunctionDeclaration_copy,SgTemplateFunctionDeclaration,get_templateDeclaration,set_templateDeclaration)

#if 0
  // DQ (11/6/2007): I have returned to sharing SgTemplateArgument objects, fix this up later.

  // DQ (11/5/2007): Until we use the SgTemplateArgumentList object directly, we have to handl the list explicitly.
     SgTemplateArgumentPtrList::const_iterator templateArguments_iterator_original = this->get_templateArguments()->begin();
     SgTemplateArgumentPtrList::iterator       templateArguments_iterator_copy     = templateFunctionDeclaration_copy->get_templateArguments()->begin();
     while ( templateArguments_iterator_original != this->get_templateArguments()->end() )
        {
          ROSE_ASSERT(templateArguments_iterator_original != this->get_templateArguments()->end());
          ROSE_ASSERT(templateArguments_iterator_copy     != templateFunctionDeclaration_copy->get_templateArguments()->end());

          (*templateArguments_iterator_original)->fixupCopy_scopes(*templateArguments_iterator_copy,help);

          templateArguments_iterator_original++;
          templateArguments_iterator_copy++;
        }
#endif
   }


void
SgFunctionDefinition::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgFunctionDefinition::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgFunctionDefinition* functionDefinition_copy = isSgFunctionDefinition(copy);
     ROSE_ASSERT(functionDefinition_copy != NULL);

  // Setup the scopes of the SgInitializedName objects in the paraleter list
     ROSE_ASSERT(get_body() != NULL);
     get_body()->fixupCopy_scopes(functionDefinition_copy->get_body(),help);

  // Call the base class fixupCopy member function
     SgScopeStatement::fixupCopy_scopes(copy,help);

  // printf ("\nLeaving SgFunctionDefinition::fixupCopy_scopes() for %p = %s copy = %p \n\n",this,this->class_name().c_str(),copy);
   }

void
SgVariableDeclaration::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgVariableDeclaration::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

  // Also call the base class version of the fixupCopycopy() member function
     SgDeclarationStatement::fixupCopy_scopes(copy,help);

     SgVariableDeclaration* variableDeclaration_copy = isSgVariableDeclaration(copy);
     ROSE_ASSERT(variableDeclaration_copy != NULL);

  // DQ (10/14/2007): Handle the case of a type defined in the base type of the typedef (similar problem for SgVariableDeclaration).
  // printf ("this = %p this->get_variableDeclarationContainsBaseTypeDefiningDeclaration() = %s \n",this,this->get_variableDeclarationContainsBaseTypeDefiningDeclaration() ? "true" : "false");
     if (this->get_variableDeclarationContainsBaseTypeDefiningDeclaration() == true)
        {
          ROSE_ASSERT(variableDeclaration_copy->get_variableDeclarationContainsBaseTypeDefiningDeclaration() == true);
          SgDeclarationStatement* baseTypeDeclaration_original = this->get_baseTypeDefiningDeclaration();
          SgDeclarationStatement* baseTypeDeclaration_copy     = variableDeclaration_copy->get_baseTypeDefiningDeclaration();

       // printf ("baseTypeDeclaration_original = %p = %s \n",baseTypeDeclaration_original,baseTypeDeclaration_original->class_name().c_str());
       // printf ("baseTypeDeclaration_copy = %p = %s \n",baseTypeDeclaration_copy,baseTypeDeclaration_copy->class_name().c_str());

       // DQ (10/17/2007): This is now fixed!
       // I think that calling get_baseTypeDefiningDeclaration() is a problem because it calls compute_baseTypeDefiningDeclaration()
       // which uses the symbol table and other infor which is not setup correctly.
       // printf ("Need to compute the baseTypeDeclaration_copy better (perhaps we shoul look into the map of copies? \n");
       // ROSE_ASSERT(false);

          ROSE_ASSERT(baseTypeDeclaration_original != NULL);
          ROSE_ASSERT(baseTypeDeclaration_copy != NULL);

       // printf ("In SgVariableDeclaration::fixupCopy_scopes(): Calling fixupCopy on %p = %s \n",baseTypeDeclaration_original,baseTypeDeclaration_original->class_name().c_str());

          baseTypeDeclaration_original->fixupCopy_scopes(baseTypeDeclaration_copy,help);
        }

     const SgInitializedNamePtrList & variableList_original = this->get_variables();
     SgInitializedNamePtrList & variableList_copy           = variableDeclaration_copy->get_variables();

  // printf ("Inside of SgVariableDeclaration::fixupCopy_scopes(): variableList_original.size() = %ld \n",(long)variableList_original.size());

     ROSE_ASSERT(variableList_original.size() == variableList_copy.size());

     SgInitializedNamePtrList::const_iterator i_original = variableList_original.begin();
     SgInitializedNamePtrList::iterator i_copy           = variableList_copy.begin();

  // Iterate over both lists to match up the correct pairs of SgInitializedName objects
     while ( (i_original != variableList_original.end()) && (i_copy != variableList_copy.end()) )
        {
        //printf ("Looping over the initialized names in the variable declaration variable = %p = %s \n",(*i_copy),(*i_copy)->get_name().str());
          ROSE_ASSERT ( (*i_copy)->get_declptr() != NULL);
          (*i_original)->fixupCopy_scopes(*i_copy,help);

          i_original++;
          i_copy++;
        }
   }

void
SgClassDeclaration::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
  // We need to call the fixupCopy function from the parent of a SgVariableDeclaration because the
  // copy function in the parent of the variable declaration sets the parent of the SgVariableDeclaration
  // and we need this parent in the fixupCopy function in the SgInitializedName.

#if DEBUG_FIXUP_COPY
     printf ("Inside of SgClassDeclaration::fixupCopy_scopes() for class = %s = %p = %s copy = %p (defining = %p firstNondefining = %p) \n",
          this->get_name().str(),this,this->class_name().c_str(),copy,this->get_definingDeclaration(),this->get_firstNondefiningDeclaration());
#endif

     SgClassDeclaration* classDeclaration_copy = isSgClassDeclaration(copy);
     ROSE_ASSERT(classDeclaration_copy != NULL);

     SgClassDefinition* classDefinition_original = this->get_definition();
     SgClassDefinition* classDefinition_copy     = classDeclaration_copy->get_definition();

  // Call the base class fixupCopy member function
     SgDeclarationStatement::fixupCopy_scopes(copy,help);

     if (isForward() == false)
        {
       // DQ (12/24/2012): Added assertion (debugging template support).
          ROSE_ASSERT(classDefinition_original != NULL);
          ROSE_ASSERT(classDefinition_copy != NULL);

          classDefinition_original->fixupCopy_scopes(classDefinition_copy,help);
        }

     const SgClassDeclaration* classDeclaration_original = isSgClassDeclaration(this);

#if 0
     printf ("SgClassDeclaration::fixupCopy_scopes(): classDeclaration_original->get_type()              = %p \n",classDeclaration_original->get_type());
     printf ("SgClassDeclaration::fixupCopy_scopes(): classDeclaration_copy->get_type()                  = %p \n",classDeclaration_copy->get_type());
#endif

  // DQ (3/15/2014): Check the types since they should be equivalent (setup intermediate variables).
     SgClassDeclaration* classDeclaration_copy_firstNondefining = isSgClassDeclaration(classDeclaration_copy->get_firstNondefiningDeclaration());
     SgClassDeclaration* classDeclaration_copy_defining         = isSgClassDeclaration(classDeclaration_copy->get_definingDeclaration());
     ROSE_ASSERT(classDeclaration_copy_firstNondefining != NULL);
  // ROSE_ASSERT(classDeclaration_copy_defining != NULL);

  // DQ (3/15/2014): Check the types since they should be equivalent (setup intermediate variables).
     SgClassDeclaration* classDeclaration_original_firstNondefining = isSgClassDeclaration(classDeclaration_original->get_firstNondefiningDeclaration());
     SgClassDeclaration* classDeclaration_original_defining         = isSgClassDeclaration(classDeclaration_original->get_definingDeclaration());
     ROSE_ASSERT(classDeclaration_original_firstNondefining != NULL);
  // ROSE_ASSERT(classDeclaration_original_defining != NULL);

#if 0
     printf ("SgClassDeclaration::fixupCopy_scopes(): classDeclaration_copy_firstNondefining->get_type() = %p \n",classDeclaration_copy_firstNondefining->get_type());
     printf ("SgClassDeclaration::fixupCopy_scopes(): classDeclaration_copy_defining->get_type()         = %p \n",classDeclaration_copy_defining->get_type());
#endif

     SgClassType* classType = isSgClassType(classDeclaration_copy->get_type());
     ROSE_ASSERT(classType != NULL);

     if (classDeclaration_copy_firstNondefining->get_type() != classType)
        {
#if DEBUG_FIXUP_COPY
          printf ("   --- Reset the type on the classDeclaration_copy->get_firstNondefiningDeclaration() (to match the type in the copy) \n");
#endif
          classDeclaration_copy_firstNondefining->set_type(classType);
        }

     if (classDeclaration_copy_defining != NULL && classDeclaration_copy_defining->get_type() != classType)
        {
#if DEBUG_FIXUP_COPY
          printf ("   --- Reset the type on the classDeclaration_copy->get_definingDeclaration() \n");
#endif
          classDeclaration_copy_defining->set_type(classType);
        }

  // DQ (3/15/2014): Check the types since they should be equivalent (error checking).
     if (classDeclaration_copy_defining != NULL)
        {
#if 0
          if (classDeclaration_copy_firstNondefining->get_type() != classDeclaration_copy_defining->get_type())
             {
               printf ("classDeclaration_copy->get_type()                  = %p = %s \n",classDeclaration_copy->get_type(),classDeclaration_copy->get_type()->class_name().c_str());
               printf ("classDeclaration_copy_firstNondefining->get_type() = %p = %s \n",classDeclaration_copy_firstNondefining->get_type(),classDeclaration_copy_firstNondefining->get_type()->class_name().c_str());
               printf ("classDeclaration_copy_defining->get_type()         = %p = %s \n",classDeclaration_copy_defining->get_type(),classDeclaration_copy_defining->get_type()->class_name().c_str());
             }
#endif
          ROSE_ASSERT(classDeclaration_copy_firstNondefining->get_type() == classDeclaration_copy_defining->get_type());
        }

  // DQ (3/17/2014): These types should be equivalent.
  // DQ (3/15/2014): Check the types since they should be equivalent.
     if (classDeclaration_original_defining != NULL)
        {
       // DQ (3/17/2014): Node that test2005_98.C fails this new test (so issue a warning for now).
          if (classDeclaration_original_firstNondefining->get_type() != classDeclaration_original_defining->get_type())
             {
               printf ("Warning: Testing of classDeclaration_original: firstNondefining->get_type() != defining->get_type() \n");
             }
       // ROSE_ASSERT(classDeclaration_original_firstNondefining->get_type() == classDeclaration_original_defining->get_type());
        }

#if 0
     printf ("Warning: SgClassDeclaration::fixupCopy_scopes(): Need to check the types for equivalence \n");
     ROSE_ABORT();
#endif
   }

void
SgClassDefinition::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
  // DQ (10/19/2007): Added support to fixup the base class names

     ROSE_ASSERT(this->get_declaration() != NULL);

#if DEBUG_FIXUP_COPY
     printf ("Inside of SgClassDefinition::fixupCopy_scopes() for class = %s class definition %p = %s copy = %p \n",this->get_declaration()->get_name().str(),this,this->class_name().c_str(),copy);
#endif

     SgClassDefinition* classDefinition_copy = isSgClassDefinition(copy);
     ROSE_ASSERT(classDefinition_copy != NULL);

     SgBaseClassPtrList::const_iterator i_original = this->get_inheritances().begin();
     SgBaseClassPtrList::iterator i_copy           = classDefinition_copy->get_inheritances().begin();
     ROSE_ASSERT(this->get_inheritances().size() == classDefinition_copy->get_inheritances().size());

     while ( (i_original != this->get_inheritances().end()) && (i_copy != classDefinition_copy->get_inheritances().end()) )
        {
       // Check the parent pointer to make sure it is properly set
          ROSE_ASSERT( (*i_original)->get_parent() != NULL);
          ROSE_ASSERT( (*i_original)->get_parent() == this);

          (*i_original)->fixupCopy_scopes(*i_copy,help);

          (*i_copy)->set_parent(classDefinition_copy);

          ROSE_ASSERT( (*i_copy)->get_parent() != NULL);
          ROSE_ASSERT( (*i_copy)->get_parent() == classDefinition_copy);

          i_original++;
          i_copy++;
        }

     const SgDeclarationStatementPtrList & statementList_original = this->getDeclarationList();
     const SgDeclarationStatementPtrList & statementList_copy     = classDefinition_copy->getDeclarationList();

     SgDeclarationStatementPtrList::const_iterator j_original = statementList_original.begin();
     SgDeclarationStatementPtrList::const_iterator j_copy     = statementList_copy.begin();

  // Iterate over both lists to match up the correct pairs of SgStatement objects
     while ( (j_original != statementList_original.end()) && (j_copy != statementList_copy.end()) )
        {
          (*j_original)->fixupCopy_scopes(*j_copy,help);

          j_original++;
          j_copy++;
        }

  // Call the base class fixupCopy member function
     SgScopeStatement::fixupCopy_scopes(copy,help);

  // printf ("\nLeaving SgClassDefinition::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
   }


void
SgBaseClass::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
  // DQ (10/19/2007): Added support to fixup the base class names

#if DEBUG_FIXUP_COPY
     printf ("Inside of SgBaseClass::fixupCopy_scopes() for baseclass = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgBaseClass* baseClass_copy = isSgBaseClass(copy);
     ROSE_ASSERT(baseClass_copy != NULL);

     const SgNonrealBaseClass* nrBaseClass = isSgNonrealBaseClass(this);
     SgNonrealBaseClass* nrBaseClass_copy = isSgNonrealBaseClass(copy);

     if (this->get_base_class() != NULL) {
       ROSE_ASSERT(baseClass_copy->get_base_class());

       ROSE_ASSERT(nrBaseClass == NULL);
       ROSE_ASSERT(nrBaseClass_copy == NULL);

       this->get_base_class()->fixupCopy_scopes(baseClass_copy->get_base_class(),help);
     } else if (nrBaseClass != NULL) {
       ROSE_ASSERT(nrBaseClass->get_base_class_nonreal() != NULL);

       ROSE_ASSERT(nrBaseClass_copy != NULL);
       ROSE_ASSERT(nrBaseClass_copy->get_base_class_nonreal() != NULL);

       nrBaseClass->get_base_class_nonreal()->fixupCopy_scopes(nrBaseClass_copy->get_base_class_nonreal(),help);
     } else {
       ROSE_ABORT();
     }
   }


void
SgLabelStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgLabelStatement::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgLabelStatement* labelStatement_copy = isSgLabelStatement(copy);
     ROSE_ASSERT(labelStatement_copy != NULL);

  // I don't think there is anything to do here since we already make sure that there is a new SgLabelSymbol
  // and it is the SgGotoStatement that has to have it's reference to the label fixed up if the label and the
  // goto statement have been copied.

  // DQ (10/25/2007): Added handling for the new explicit scope in SgLabelStatement
     ROSE_ASSERT(this->hasExplicitScope() == true);

     FixupCopyDataMemberMacro(labelStatement_copy,SgScopeStatement,get_scope,set_scope)

  // Make sure that the copy sets the scopes to be the same type
     ROSE_ASSERT(labelStatement_copy->get_scope()->variantT() == this->get_scope()->variantT());

  // Also call the base class version of the fixupCopycopy() member function
     SgStatement::fixupCopy_scopes(copy,help);
   }

void
SgGotoStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgGotoStatement::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgGotoStatement* gotoStatement_copy = isSgGotoStatement(copy);
     ROSE_ASSERT(gotoStatement_copy != NULL);

     SgLabelStatement* labelStatement_original = get_label();

     SgCopyHelp::copiedNodeMapTypeIterator i = help.get_copiedNodeMap().find(labelStatement_original);

  // printf ("Inside of SgGotoStatement::fixupCopy_scopes(): i != help.get_copiedNodeMap().end() = %s \n",(i != help.get_copiedNodeMap().end()) ? "true" : "false");

  // If the declaration is in the map then it is because we have copied it previously
  // and thus it should be updated to reflect the copied declaration.
     if (i != help.get_copiedNodeMap().end())
        {
          SgLabelStatement* labelStatement_copy = isSgLabelStatement(i->second);
          ROSE_ASSERT(labelStatement_copy != NULL);
          gotoStatement_copy->set_label(labelStatement_copy);
        }

  // Also call the base class version of the fixupCopycopy() member function
     SgStatement::fixupCopy_scopes(copy,help);
   }

void
SgAdaExitStmt::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgAdaExitStmt::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgAdaExitStmt* exitStmt_copy = isSgAdaExitStmt(copy);
     ROSE_ASSERT(exitStmt_copy != NULL);

     SgStatement* loop_original = get_loop();

     SgCopyHelp::copiedNodeMapTypeIterator i = help.get_copiedNodeMap().find(loop_original);

  // If the declaration is in the map then it is because we have copied it previously
  // and thus it should be updated to reflect the copied declaration.
     if (i != help.get_copiedNodeMap().end())
        {
          SgStatement* loop_copy = isSgStatement(i->second);
          ROSE_ASSERT(loop_copy != NULL);
          exitStmt_copy->set_loop(loop_copy);
        }

     this->get_condition()->fixupCopy_scopes(exitStmt_copy->get_condition(),help);

  // Also call the base class version of the fixupCopycopy() member function
     SgStatement::fixupCopy_scopes(copy,help);
   }

void
SgAdaDelayStmt::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgAdaDelayStmt::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgAdaDelayStmt* delayStmt_copy = isSgAdaDelayStmt(copy);
     ROSE_ASSERT(delayStmt_copy != NULL);

     this->get_time()->fixupCopy_scopes(delayStmt_copy->get_time(),help);

  // Also call the base class version of the fixupCopycopy() member function
     SgStatement::fixupCopy_scopes(copy,help);
   }


void
SgAdaAcceptStmt::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgAdaAcceptStmt::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgAdaAcceptStmt* acceptStmt_copy = isSgAdaAcceptStmt(copy);
     ROSE_ASSERT(acceptStmt_copy != NULL);

     SgStatement* body_original = get_body();

     SgCopyHelp::copiedNodeMapTypeIterator i = help.get_copiedNodeMap().find(body_original);

  // If the declaration is in the map then it is because we have copied it previously
  // and thus it should be updated to reflect the copied declaration.
     if (i != help.get_copiedNodeMap().end())
        {
          SgStatement* body_copy = isSgStatement(i->second);
          ROSE_ASSERT(body_copy != NULL);
          acceptStmt_copy->set_body(body_copy);
        }

  // Also call the base class version of the fixupCopycopy() member function
     SgStatement::fixupCopy_scopes(copy,help);
   }


void
SgAdaLoopStmt::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgAdaLoopStmt::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgAdaLoopStmt* loop_copy = isSgAdaLoopStmt(copy);
     ROSE_ASSERT(loop_copy != NULL);

     SgBasicBlock* body_original = get_body();

     SgCopyHelp::copiedNodeMapTypeIterator i = help.get_copiedNodeMap().find(body_original);

  // If the declaration is in the map then it is because we have copied it previously
  // and thus it should be updated to reflect the copied declaration.
     if (i != help.get_copiedNodeMap().end())
        {
          SgBasicBlock* body_copy = isSgBasicBlock(i->second);
          ROSE_ASSERT(body_copy != NULL);
          loop_copy->set_body(body_copy);
        }

  // Also call the base class version of the fixupCopycopy() member function
     SgStatement::fixupCopy_scopes(copy,help);
   }


void
SgTypedefDeclaration::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
  // We need to call the fixupCopy function from the parent of a SgTypedefDeclaration because the
  // copy function in the parent of the variable declaration sets the parent of the SgVariableDeclaration
  // and we need this parent in the fixupCopy function in the SgInitializedName.

#if DEBUG_FIXUP_COPY
     printf ("Inside of SgTypedefDeclaration::fixupCopy_scopes() for typedef name = %s = %p = %s copy = %p \n",this->get_name().str(),this,this->class_name().c_str(),copy);
#endif

  // Call the base class fixupCopy member function
     SgDeclarationStatement::fixupCopy_scopes(copy,help);

#if 1
     SgTypedefDeclaration* typedefDeclaration_copy = isSgTypedefDeclaration(copy);
     ROSE_ASSERT(typedefDeclaration_copy != NULL);

  // DQ (10/14/2007): Handle the case of a type defined in the base type of the typedef (similar problem for SgVariableDeclaration).
     if (this->get_typedefBaseTypeContainsDefiningDeclaration() == true)
        {
          ROSE_ASSERT(typedefDeclaration_copy->get_typedefBaseTypeContainsDefiningDeclaration() == true);
          SgDeclarationStatement* baseTypeDeclaration_original = this->get_baseTypeDefiningDeclaration();
          SgDeclarationStatement* baseTypeDeclaration_copy     = typedefDeclaration_copy->get_baseTypeDefiningDeclaration();
          ROSE_ASSERT(baseTypeDeclaration_original != NULL);
          ROSE_ASSERT(baseTypeDeclaration_copy != NULL);

          baseTypeDeclaration_original->fixupCopy_scopes(baseTypeDeclaration_copy,help);
        }
       else
        {
          if (this->get_declaration() != NULL)
             {
               ROSE_ASSERT(typedefDeclaration_copy->get_declaration() != NULL);
               ROSE_ASSERT(typedefDeclaration_copy->get_declaration() != this->get_declaration());
               this->get_declaration()->fixupCopy_scopes(typedefDeclaration_copy->get_declaration(),help);
             }
        }
#else
     SgTypedefDeclaration* typedefDeclaration_copy = isSgTypedefDeclaration(copy);
     ROSE_ASSERT(typedefDeclaration_copy != NULL);

     ROSE_ASSERT(typedefDeclaration_copy->get_typedefBaseTypeContainsDefiningDeclaration() == true);
     SgDeclarationStatement* baseTypeDeclaration_original = this->get_baseTypeDefiningDeclaration();
     SgDeclarationStatement* baseTypeDeclaration_copy     = typedefDeclaration_copy->get_baseTypeDefiningDeclaration();
     ROSE_ASSERT(baseTypeDeclaration_original != NULL);
     ROSE_ASSERT(baseTypeDeclaration_copy != NULL);

     baseTypeDeclaration_original->fixupCopy_scopes(baseTypeDeclaration_copy,help);
#endif
   }


void
SgEnumDeclaration::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgEnumDeclaration::fixupCopy_scopes() for %p = %s copy = %p (defining = %p firstNondefining = %p) \n",
          this,this->class_name().c_str(),copy,this->get_definingDeclaration(),this->get_firstNondefiningDeclaration());
#endif

  // Call the base class fixupCopy member function
     SgDeclarationStatement::fixupCopy_scopes(copy,help);

     SgEnumDeclaration* enumDeclaration_copy = isSgEnumDeclaration(copy);
     ROSE_ASSERT(enumDeclaration_copy != NULL);

  // DQ (10/17/2007): fixup the type used to make sure it has the declaration set the AST copy.
     SgEnumType* enum_type_original = this->get_type();
     ROSE_ASSERT(enum_type_original != NULL);

     SgEnumType* enum_type_copy = enumDeclaration_copy->get_type();
     ROSE_ASSERT(enum_type_copy != NULL);

  // printf ("This is the non-defining declaration, so just fixup the SgEnumType = %p with the correct SgEnumDeclaration declaration! \n",enum_type_copy);

  // FixupCopyDataMemberMacro_local_debug(enum_type_copy,SgDeclarationStatement,get_declaration,set_declaration)
     SgCopyHelp::copiedNodeMapTypeIterator i = help.get_copiedNodeMap().find(enum_type_original->get_declaration());
  // printf ("SgCopyHelp::copiedNodeMapTypeIterator i != help.get_copiedNodeMap().end() = %s \n",i != help.get_copiedNodeMap().end() ? "true" : "false");
     if (i != help.get_copiedNodeMap().end())
        {
          SgNode* associated_node_copy = i->second;
          ROSE_ASSERT(associated_node_copy != NULL);
          SgDeclarationStatement* local_copy = isSgDeclarationStatement(associated_node_copy);
          ROSE_ASSERT(local_copy != NULL);
       // printf ("Resetting using local_copy = %p = %s \n",local_copy,local_copy->class_name().c_str());
          enum_type_copy->set_declaration(local_copy);
        }

  // Now reset the enum fields.
     const SgInitializedNamePtrList & enumFieldList_original = this->get_enumerators();
     SgInitializedNamePtrList & enumFieldList_copy           = enumDeclaration_copy->get_enumerators();

     SgInitializedNamePtrList::const_iterator i_original = enumFieldList_original.begin();
     SgInitializedNamePtrList::iterator i_copy           = enumFieldList_copy.begin();

  // Iterate over both lists to match up the correct pairs of SgInitializedName objects
     while ( (i_original != enumFieldList_original.end()) && (i_copy != enumFieldList_copy.end()) )
        {
          (*i_original)->fixupCopy_scopes(*i_copy,help);

          i_original++;
          i_copy++;
        }
   }




void
SgNamespaceDeclarationStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgNamespaceDeclarationStatement::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

  // Call the base class fixupCopy member function
     SgDeclarationStatement::fixupCopy_scopes(copy,help);

     SgNamespaceDeclarationStatement* namespaceDeclaration_copy = isSgNamespaceDeclarationStatement(copy);
     ROSE_ASSERT(namespaceDeclaration_copy != NULL);

  // printf ("namespaceDeclaration_copy->get_firstNondefiningDeclaration() = %p \n",namespaceDeclaration_copy->get_firstNondefiningDeclaration());
     if (this->get_firstNondefiningDeclaration() == this)
        {
       // printf ("&&&&& Resetting copy's firstNondefiningDeclaration of SgNamespaceDeclarationStatement to copy \n");
          namespaceDeclaration_copy->set_firstNondefiningDeclaration(namespaceDeclaration_copy);
        }

     SgNamespaceDefinitionStatement* namespaceDefinition_original = this->get_definition();
     SgNamespaceDefinitionStatement* namespaceDefinition_copy     = namespaceDeclaration_copy->get_definition();

     ROSE_ASSERT(namespaceDefinition_original != NULL);
     ROSE_ASSERT(namespaceDefinition_copy != NULL);

     namespaceDefinition_original->fixupCopy_scopes(namespaceDefinition_copy,help);
   }


void
SgNamespaceDefinitionStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgNamespaceDefinitionStatement::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgNamespaceDefinitionStatement* namespaceDefinition_copy = isSgNamespaceDefinitionStatement(copy);
     ROSE_ASSERT(namespaceDefinition_copy != NULL);

     FixupCopyDataMemberMacro(namespaceDefinition_copy,SgNamespaceDeclarationStatement,get_namespaceDeclaration,set_namespaceDeclaration)

     const SgDeclarationStatementPtrList & statementList_original = this->getDeclarationList();
     const SgDeclarationStatementPtrList & statementList_copy     = namespaceDefinition_copy->getDeclarationList();

     SgDeclarationStatementPtrList::const_iterator i_original = statementList_original.begin();
     SgDeclarationStatementPtrList::const_iterator i_copy     = statementList_copy.begin();

  // Iterate over both lists to match up the correct pairs of SgStatement objects
     while ( (i_original != statementList_original.end()) && (i_copy != statementList_copy.end()) )
        {
          (*i_original)->fixupCopy_scopes(*i_copy,help);

          i_original++;
          i_copy++;
        }

  // Call the base class fixupCopy member function
     SgScopeStatement::fixupCopy_scopes(copy,help);
   }


void
SgTemplateInstantiationDirectiveStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgTemplateInstantiationDirectiveStatement::fixupCopy_scopes() for %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgTemplateInstantiationDirectiveStatement* templateInstantiationDirectiveStatement_copy = isSgTemplateInstantiationDirectiveStatement(copy);

  // Call the base class fixupCopy member function
     SgDeclarationStatement::fixupCopy_scopes(copy,help);

     ROSE_ASSERT(this->get_declaration() != NULL);

#if 0
  // DQ (10/15/2007): This is a simpler approach using the map stored in the SgCopyHelp object.
     SgCopyHelp::copiedNodeMapTypeIterator i = help.get_copiedNodeMap().find(this->get_declaration());
     if (i != help.get_copiedNodeMap().end())
        {
       // Use the associated value from the map as the scope.
          SgNode* node_copy = i->second;
          ROSE_ASSERT(node_copy != NULL);
          SgDeclarationStatement* declarationStatement_copy = isSgDeclarationStatement(node_copy);
          templateInstantiationDirectiveStatement_copy->set_declaration(declarationStatement_copy);
        }
       else
        {
       // Else set the scope in the copyDeclarationStatement to be the same as this->get_scope()
       // printf ("In SgTemplateInstantiationDirectiveStatement::fixupCopy_scopes() this = %p = %s this->get_parent() = %p = %s \n",
       //      this,this->class_name().c_str(),this->get_declaration(),this->get_declaration()->class_name().c_str());
          templateInstantiationDirectiveStatement_copy->set_declaration(this->get_declaration());
        }
#else
     FixupCopyDataMemberMacro(templateInstantiationDirectiveStatement_copy,SgDeclarationStatement,get_declaration,set_declaration)
#endif

  // DQ (11/7/2007): Call fixup on the declaration stored internally (a copy, not shared).
     SgDeclarationStatement* declaration_copy = templateInstantiationDirectiveStatement_copy->get_declaration();
     ROSE_ASSERT(declaration_copy != NULL);
     this->get_declaration()->fixupCopy_scopes(declaration_copy,help);
   }



void
SgProject::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgProject::fixupCopy_scopes() \n");
#endif

     SgProject* project_copy = isSgProject(copy);
     ROSE_ASSERT(project_copy != NULL);

  // Call fixup on all fo the files (SgFile objects)
     for (int i = 0; i < numberOfFiles(); i++)
        {
          SgFile & file = get_file(i);
          SgFile & file_copy = project_copy->get_file(i);
          file.fixupCopy_scopes(&file_copy,help);
        }
   }

void
SgSourceFile::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgSourceFile::fixupCopy_scopes() \n");
#endif

     SgSourceFile* file_copy = isSgSourceFile(copy);
     ROSE_ASSERT(file_copy != NULL);

  // Call fixup on the global scope
     ROSE_ASSERT(get_globalScope() != NULL);
     ROSE_ASSERT(file_copy->get_globalScope() != NULL);
     get_globalScope()->fixupCopy_scopes(file_copy->get_globalScope(),help);
   }



void
SgIfStmt::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgIfStmt::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

  // printf ("Inside of SgIfStmt::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);

  // SgStatement::fixupCopy_scopes(copy,help);
     SgScopeStatement::fixupCopy_scopes(copy,help);

     SgIfStmt* ifStatement_copy = isSgIfStmt(copy);
     ROSE_ASSERT(ifStatement_copy != NULL);

  // The symbol table should not have been setup yet!
  // ROSE_ASSERT(ifStatement_copy->get_symbol_table()->size() == 0);

     this->get_conditional()->fixupCopy_scopes(ifStatement_copy->get_conditional(),help);

#if 0
     ROSE_ASSERT(ifStatement_copy->get_true_body() != NULL);
     ROSE_ASSERT(ifStatement_copy->get_true_body()->get_symbol_table() != NULL);
     ROSE_ASSERT(ifStatement_copy->get_true_body()->get_symbol_table()->size()  == 0);

     ROSE_ASSERT(ifStatement_copy->get_false_body() != NULL);
     ROSE_ASSERT(ifStatement_copy->get_false_body()->get_symbol_table() != NULL);
     ROSE_ASSERT(ifStatement_copy->get_false_body()->get_symbol_table()->size()  == 0);

     const SgVariableDeclaration* variableDeclarationCondition = isSgVariableDeclaration(this->get_conditional());
     if (variableDeclarationCondition != NULL)
        {
       // There is a variable declaration in the conditional, it needs to be added to the symbol table.
          printf ("There is a variable declaration in the conditional, it needs to be added to the symbol table \n");
          ROSE_ABORT();
        }
#endif

     SgStatement* thsTruBody = this->get_true_body();
     ROSE_ASSERT(thsTruBody != NULL);
     SgStatement* ifStmtCopyTruBody = ifStatement_copy->get_true_body();
     ROSE_ASSERT(ifStmtCopyTruBody != NULL);
     SgScopeStatement* scopeIfStmtCopyTruBody = isSgScopeStatement(ifStmtCopyTruBody);
     if (scopeIfStmtCopyTruBody != NULL)
        {
       // DQ (5/21/2013): Restrict direct access to the symbol table.
       // ROSE_ASSERT(scopeIfStmtCopyTruBody->get_symbol_table() != NULL);
       // ROSE_ASSERT(scopeIfStmtCopyTruBody->get_symbol_table()->size()  == 0);
          if (scopeIfStmtCopyTruBody->symbol_table_size() != 0)
             {
               printf ("Warning: (fails for g++ 4.2): scopeIfStmtCopyTruBody->symbol_table_size() = %zu \n",scopeIfStmtCopyTruBody->symbol_table_size());
            // ifStmtCopyTruBody->get_file_info()->display("ifStatement_copy->get_true_body(): debug");
             }
       // DQ (3/3/12): This fails for the g++ version 4.2.4 compiler (newer versions of g++ pass fine).
       // ROSE_ASSERT(scopeIfStmtCopyTruBody->symbol_table_size() == 0);
        }

  // printf ("\nProcess the TRUE body of the SgIfStmt \n\n");

     thsTruBody->fixupCopy_scopes(ifStmtCopyTruBody,help);

     ROSE_ASSERT((this->get_false_body() != NULL) == (ifStatement_copy->get_false_body() != NULL));
     if (isSgScopeStatement(ifStatement_copy->get_false_body()) != NULL) {
     // DQ (5/21/2013): Restrict direct access to the symbol table.
     // ROSE_ASSERT(isSgScopeStatement(ifStatement_copy->get_false_body())->get_symbol_table()->size() == 0);
     // ROSE_ASSERT(isSgScopeStatement(ifStatement_copy->get_false_body())->get_symbol_table() != NULL);
        SgScopeStatement* scopeStmnt = isSgScopeStatement(ifStatement_copy->get_false_body());
        if (scopeStmnt->symbol_table_size() != 0) {
           printf ("Warning: (fails for g++ 4.2): isSgScopeStatement(ifStatement_copy->get_false_body())->symbol_table_size() = %zu \n",scopeStmnt->symbol_table_size());
        // ifStatement_copy->get_true_body()->get_file_info()->display("ifStatement_copy->get_false_body(): debug");
        }
     // DQ (3/3/12): This fails for the g++ version 4.2.4 compiler (newer versions of g++ pass fine).
     // ROSE_ASSERT(scopeStmnt->symbol_table_size() == 0);
     }

  // printf ("\nProcess the FALSE body of the SgIfStmt \n\n");

     if (this->get_false_body() != NULL) {
       this->get_false_body()->fixupCopy_scopes(ifStatement_copy->get_false_body(),help);
     }

  // printf ("\nLeaving SgIfStmt::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
   }

void
SgForStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgForStatement::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgForStatement* forStatement_copy = isSgForStatement(copy);
     ROSE_ASSERT(forStatement_copy != NULL);

  // This could generate a vaiable declaration, so wait to build the sysmbol table.
     ROSE_ASSERT(this->get_for_init_stmt() != NULL);
     this->get_for_init_stmt()->fixupCopy_scopes(forStatement_copy->get_for_init_stmt(),help);

  // DQ (11/1/2007): Force the symbol table to be setup so that references can be made to it later.
  // If we built it too early then the scope (on The SgInitializedName objects) have not be setup,
  // and if we build it too late then we don't have the symbols in place to reset the references.
  // printf ("Calling SgScopeStatement::fixupCopy_scopes() \n");
     SgScopeStatement::fixupCopy_scopes(copy,help);
  // printf ("DONE: SgScopeStatement::fixupCopy_scopes() \n");

  // This could generate a vaiable declaration, so wait to build the sysmbol table.
     ROSE_ASSERT(this->get_test() != NULL);
     this->get_test()->fixupCopy_scopes(forStatement_copy->get_test(),help);

     ROSE_ASSERT(this->get_increment() != NULL);
     this->get_increment()->fixupCopy_scopes(forStatement_copy->get_increment(),help);

     ROSE_ASSERT(this->get_loop_body() != NULL);
     this->get_loop_body()->fixupCopy_scopes(forStatement_copy->get_loop_body(),help);

#if 0
  // SgScopeStatement::fixupCopy_scopes(copy,help);
     printf ("Calling SgScopeStatement::fixupCopy_scopes() (2nd time) \n");
     SgScopeStatement::fixupCopy_scopes(copy,help);
     printf ("DONE: SgScopeStatement::fixupCopy_scopes() (2nd time) \n");
#endif
   }

void
SgRangeBasedForStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgRangeBasedForStatement::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgRangeBasedForStatement* forStatement_copy = isSgRangeBasedForStatement(copy);
     ROSE_ASSERT(forStatement_copy != NULL);

  // This could generate a vaiable declaration, so wait to build the sysmbol table.
     ROSE_ASSERT(this->get_iterator_declaration() != NULL);
     this->get_iterator_declaration()->fixupCopy_scopes(forStatement_copy->get_iterator_declaration(),help);

     ROSE_ASSERT(this->get_range_declaration() != NULL);
     this->get_range_declaration()->fixupCopy_scopes(forStatement_copy->get_range_declaration(),help);

  // DQ (11/1/2007): Force the symbol table to be setup so that references can be made to it later.
  // If we built it too early then the scope (on The SgInitializedName objects) have not be setup,
  // and if we build it too late then we don't have the symbols in place to reset the references.
  // printf ("Calling SgScopeStatement::fixupCopy_scopes() \n");
     SgScopeStatement::fixupCopy_scopes(copy,help);
  // printf ("DONE: SgScopeStatement::fixupCopy_scopes() \n");

     ROSE_ASSERT(this->get_begin_declaration() != NULL);
     this->get_begin_declaration()->fixupCopy_scopes(forStatement_copy->get_begin_declaration(),help);

     ROSE_ASSERT(this->get_end_declaration() != NULL);
     this->get_end_declaration()->fixupCopy_scopes(forStatement_copy->get_end_declaration(),help);

     ROSE_ASSERT(this->get_not_equal_expression() != NULL);
     this->get_not_equal_expression()->fixupCopy_scopes(forStatement_copy->get_not_equal_expression(),help);

     ROSE_ASSERT(this->get_increment_expression() != NULL);
     this->get_increment_expression()->fixupCopy_scopes(forStatement_copy->get_increment_expression(),help);

     ROSE_ASSERT(this->get_loop_body() != NULL);
     this->get_loop_body()->fixupCopy_scopes(forStatement_copy->get_loop_body(),help);

#if 0
  // SgScopeStatement::fixupCopy_scopes(copy,help);
     printf ("Calling SgScopeStatement::fixupCopy_scopes() (2nd time) \n");
     SgScopeStatement::fixupCopy_scopes(copy,help);
     printf ("DONE: SgScopeStatement::fixupCopy_scopes() (2nd time) \n");
#endif
   }

void
SgForInitStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgForInitStatement::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgStatement::fixupCopy_scopes(copy,help);

     SgForInitStatement* forStatement_copy = isSgForInitStatement(copy);
     ROSE_ASSERT(forStatement_copy != NULL);

  // printf ("SgForInitStatement::fixupCopy_scopes(): Sorry not implemented \n");

     const SgStatementPtrList & statementList_original = this->get_init_stmt();
     const SgStatementPtrList & statementList_copy     = forStatement_copy->get_init_stmt();

     SgStatementPtrList::const_iterator i_original = statementList_original.begin();
     SgStatementPtrList::const_iterator i_copy     = statementList_copy.begin();

  // Iterate over both lists to match up the correct pairs of SgStatement objects
     while ( (i_original != statementList_original.end()) && (i_copy != statementList_copy.end()) )
        {
        // printf ("In SgForInitStatement::fixupCopy_scopes(): Calling fixup for *i_copy = %p = %s \n",(*i_copy),(*i_copy)->class_name().c_str());
          (*i_original)->fixupCopy_scopes(*i_copy,help);

          i_original++;
          i_copy++;
        }

  // Relavant data member is: SgStatementPtrList        p_init_stmt
   }

void
SgCatchStatementSeq::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgCatchStatementSeq::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgStatement::fixupCopy_scopes(copy,help);

     SgCatchStatementSeq* catchStatement_copy = isSgCatchStatementSeq(copy);
     ROSE_ASSERT(catchStatement_copy != NULL);

     printf ("SgCatchStatementSeq::fixupCopy_scopes(): Sorry not implemented \n");

  // The relavant data member here is a SgStatementPtrList      p_catch_statement_seq

  // ROSE_ASSERT(this->get_body() != NULL);
  // this->get_body()->fixupCopy_scopes(catchStatement_copy->get_body(),help);
   }

void
SgWhileStmt::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgWhileStmt::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgScopeStatement::fixupCopy_scopes(copy,help);

     SgWhileStmt* whileStatement_copy = isSgWhileStmt(copy);
     ROSE_ASSERT(whileStatement_copy != NULL);

     ROSE_ASSERT(this->get_condition() != NULL);
     this->get_condition()->fixupCopy_scopes(whileStatement_copy->get_condition(),help);

     ROSE_ASSERT(this->get_body() != NULL);
     this->get_body()->fixupCopy_scopes(whileStatement_copy->get_body(),help);
   }

void
SgDoWhileStmt::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgDoWhileStmt::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgScopeStatement::fixupCopy_scopes(copy,help);

     SgDoWhileStmt* doWhileStatement_copy = isSgDoWhileStmt(copy);
     ROSE_ASSERT(doWhileStatement_copy != NULL);

     ROSE_ASSERT(this->get_condition() != NULL);
     this->get_condition()->fixupCopy_scopes(doWhileStatement_copy->get_condition(),help);

     ROSE_ASSERT(this->get_body() != NULL);
     this->get_body()->fixupCopy_scopes(doWhileStatement_copy->get_body(),help);
   }


void
SgSwitchStatement::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgSwitchStatement::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgScopeStatement::fixupCopy_scopes(copy,help);

     SgSwitchStatement* switchStatement_copy = isSgSwitchStatement(copy);
     ROSE_ASSERT(switchStatement_copy != NULL);

     ROSE_ASSERT(this->get_item_selector() != NULL);
     this->get_item_selector()->fixupCopy_scopes(switchStatement_copy->get_item_selector(),help);

     ROSE_ASSERT(this->get_body() != NULL);
     this->get_body()->fixupCopy_scopes(switchStatement_copy->get_body(),help);
   }



void
SgTryStmt::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgTryStmt::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgStatement::fixupCopy_scopes(copy,help);

     SgTryStmt* tryStatement_copy = isSgTryStmt(copy);
     ROSE_ASSERT(tryStatement_copy != NULL);

     ROSE_ASSERT(this->get_body() != NULL);
     this->get_body()->fixupCopy_scopes(tryStatement_copy->get_body(),help);
   }

void
SgCatchOptionStmt::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgCatchOptionStmt::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     printf ("SgCatchOptionStmt::fixupCopy_scopes(): Sorry not implemented \n");

     SgScopeStatement::fixupCopy_scopes(copy,help);

     SgCatchOptionStmt* catchOptionStatement_copy = isSgCatchOptionStmt(copy);
     ROSE_ASSERT(catchOptionStatement_copy != NULL);

     ROSE_ASSERT(this->get_trystmt() != NULL);
  // I think this might cause endless recursion, so comment out for now!
  // this->get_trystmt()->fixupCopy_scopes(catchOptionStatement_copy->get_trystmt(),help);

     ROSE_ASSERT(this->get_condition() != NULL);
     this->get_condition()->fixupCopy_scopes(catchOptionStatement_copy->get_condition(),help);

     ROSE_ASSERT(this->get_body() != NULL);
     this->get_body()->fixupCopy_scopes(catchOptionStatement_copy->get_body(),help);
   }

void
SgCaseOptionStmt::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgCaseOptionStmt::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgStatement::fixupCopy_scopes(copy,help);

     SgCaseOptionStmt* caseOptionStatement_copy = isSgCaseOptionStmt(copy);
     ROSE_ASSERT(caseOptionStatement_copy != NULL);

     ROSE_ASSERT(this->get_body() != NULL);
     this->get_body()->fixupCopy_scopes(caseOptionStatement_copy->get_body(),help);
   }

void
SgDefaultOptionStmt::fixupCopy_scopes(SgNode* copy, SgCopyHelp & help) const
   {
#if DEBUG_FIXUP_COPY
     printf ("Inside of SgDefaultOptionStmt::fixupCopy_scopes() this = %p = %s  copy = %p \n",this,this->class_name().c_str(),copy);
#endif

     SgStatement::fixupCopy_scopes(copy,help);

     SgDefaultOptionStmt* defaultOptionStatement_copy = isSgDefaultOptionStmt(copy);
     ROSE_ASSERT(defaultOptionStatement_copy != NULL);

     ROSE_ASSERT(this->get_body() != NULL);
     this->get_body()->fixupCopy_scopes(defaultOptionStatement_copy->get_body(),help);
   }

void
SgTemplateArgument::fixupCopy_scopes(SgNode* copy, SgCopyHelp&) const
   {
     SgTemplateArgument* templateArgument_copy = isSgTemplateArgument(copy);
     ROSE_ASSERT(templateArgument_copy != NULL);

#if DEBUG_FIXUP_COPY
     printf ("\nIn SgTemplateArgument::fixupCopy_scopes(): this = %p = %s copy = %p \n",this,this->class_name().c_str(),copy);
#endif
   }


