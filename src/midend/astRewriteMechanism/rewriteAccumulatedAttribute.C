// tps (01/14/2010) : Switching from rose.h to sage3.
//#include "sage3basic.h"

#if 0
   These are no longer needed

unsigned
AST_Rewrite::AccumulatedDeclarationsAttribute::size() const
   {
     ASSERT_this();
     return stackOfLists.size();
   }

void
AST_Rewrite::AccumulatedDeclarationsAttribute::pushScope()
   {
     ASSERT_this();
     list<SgDeclarationStatement*> emptyDeclarationList;
     stackOfLists.push(emptyDeclarationList);
     ROSE_ASSERT (stackOfLists.size() > 0);

  // error checking
     consistencyCheck();
   }

void
AST_Rewrite::AccumulatedDeclarationsAttribute::popScope()
   {
     ASSERT_this();
     ROSE_ASSERT (stackOfLists.size() > 0);
     stackOfLists.pop();

  // error checking
     consistencyCheck();
   }

void
AST_Rewrite::AccumulatedDeclarationsAttribute::addNewDeclaration( SgDeclarationStatement* declaration )
   {
     ASSERT_this();
     ROSE_ASSERT (stackOfLists.size() > 0);

     stackOfLists.top().push_back(declaration);
     ROSE_ASSERT (stackOfLists.top().size() > 0);

  // error checking
     consistencyCheck();
   }

void
AST_Rewrite::AccumulatedDeclarationsAttribute::display( const string & label )
   {
     ASSERT_this();
     printf ("In AST_Rewrite::AccumulatedDeclarationsAttribute::display(%s) \n",label.c_str());
  // printf ("   (output of declarations is in reverse order of declaration in AST) \n");

  // stack< list<SgDeclarationStatement*> >::iterator stackIterator;
     stack< list<SgDeclarationStatement*> > copy = stackOfLists;

     list<string> declarationStringList;

  // printf ("copy.size() = %" PRIuPTR " \n",copy.size());

     int indentation = (copy.size()-0) * 5;
     while (!copy.empty())
        {
       // printf ("Inside of loop! (indentation = %d) \n",indentation);

          list<SgDeclarationStatement*> & scope = copy.top();
          list<SgDeclarationStatement*>::reverse_iterator i;
          indentation -= 5;
          for (i=scope.rbegin(); i != scope.rend(); i++)
             {
                ROSE_ASSERT ( (*i) != NULL );
                string declarationString = (*i)->unparseToString();
                if (declarationString == "")
                     declarationString = string("unparseToString() not implemented: ") +
                                         string((*i)->sage_class_name()) + "\n";
                for (int n=0; n < indentation; n++)
                     declarationString = string(" ") + declarationString;
             // printf ("declaration: %s \n",declarationString.c_str());
                declarationStringList.push_back(declarationString);
             }
       // indentation += 5;
          ROSE_ASSERT (indentation >= 0);

       // error checking (remove later)
       // ROSE_ASSERT (indentation < 100);

       // pop the stack
          copy.pop();
        }

     printf ("Display declarations: \n");
     list<string>::reverse_iterator r;
     for (r=declarationStringList.rbegin(); r != declarationStringList.rend(); r++)
        {
           printf ("%s\n",(*r).c_str());
        }

   }

string
AST_Rewrite::AccumulatedDeclarationsAttribute::generateDeclarationString() const
   {
     string globalString           = generateGlobalDeclarationString();
     string openingNonGlobalString = generateOpeningNonGlobalDeclarationString();
     string closingNonGlobalString = generateClosingNonGlobalDeclarationString();

#if 0
     printf ("globalString           = %s \n",globalString.c_str());
     printf ("openingNonGlobalString = %s \n",openingNonGlobalString.c_str());
     printf ("closingNonGlobalString = %s \n",closingNonGlobalString.c_str());
#endif

     consistencyCheck();

     return globalString + openingNonGlobalString + closingNonGlobalString;
   }

string
AST_Rewrite::AccumulatedDeclarationsAttribute::
generateDeclarationString ( SgDeclarationStatement* declaration ) const
   {
  // This function generates a string for a declaration. The string is required for 
  // the intermediate file to make sure that all transformation code will compile 
  // (since it could depend on declarations defined within the code).

  // Details:
  //   1) Only record declarations found within the source file (exclude all header files 
  //      since they will be seen when the same header files are included).
  //   2) Resort the variable declarations to remove redundent entries.
  //        WRONG: variable declarations could have dependences upon class declarations!
  //   3) Don't sort all declarations since some could have dependences.
  //        a) class declarations
  //        b) typedefs
  //        c) function declarations
  //        d) template declarations
  //        e) variable definition???

      ASSERT_this();
      ROSE_ASSERT ( declaration != NULL );
      string declarationString;

   // Build a SgUnparse_Info object to represent formatting options for
   // this statement (use the default values).
      SgUnparse_Info info;

   // exclude comments
      info.set_SkipComments();

   // exclude all CPP directives (since they have already been evaluated by the front-end)
      info.set_SkipCPPDirectives();

      switch ( declaration->variantT() )
        {
       // Enum declarations should not skip their definition since 
       // this is where the constants are declared.
          case V_SgEnumDeclaration:

          case V_SgVariableDeclaration:
          case V_SgTemplateDeclaration:
          case V_SgTypedefDeclaration:
            // Need to figure out if a forward declaration would work or be 
            // more conservative and always output the complete class definition.

            // turn off output of initializer values
               info.set_SkipInitializer();
            // output the declaration as a string
               declarationString = globalUnparseToString(declaration,&info);
               break;

          case V_SgClassDeclaration:
            // Need to figure out if a forward declaration would work or be 
            // more conservative and always output the complete class definition.

            // turn off the generation of the function definitions only 
            // (we still want the restof the class definition since these 
            // define all member data and member functions).
            // info.set_SkipClassDefinition();
               info.set_SkipFunctionDefinition();
               info.set_AddSemiColonAfterDeclaration();

            // output the declaration as a string
               declarationString = globalUnparseToString(declaration,&info);
               break;

       // For functions just output the declaration and skip the definition
       // (This also avoids the generation of redundent definitions since the 
       // function we are in when we generate all declarations would be included).
          case V_SgMemberFunctionDeclaration:
          case V_SgFunctionDeclaration:
             {
            // turn off the generation of the definition
               info.set_SkipFunctionDefinition();
               info.set_AddSemiColonAfterDeclaration();

            // output the declaration as a string
               declarationString = globalUnparseToString(declaration,&info);
               break;
             }

          case V_SgFunctionParameterList:
             {
            // Handle generation of declaration strings this case differnetly from unparser
            // since want to generate declaration strings and not function parameter lists
            // (function parameter lists would be delimited by "," while declarations would
            // be delimited by ";").
               SgFunctionParameterList* parameterListDeclaration = dynamic_cast<SgFunctionParameterList*>(declaration);
               ROSE_ASSERT (parameterListDeclaration != NULL);
               SgInitializedNameList & argList = parameterListDeclaration->get_args();
               SgInitializedNameList::iterator i;
               for (i = argList.begin(); i != argList.end(); i++)
                  {
                    string typeNameString = (*i).get_type()->unparseToString();
                 // (9/8/2003) Bug Fix suggested by Nils
                 // string variableName   = (*i).get_name().str();
                    string variableName;
                    SgName nodeName   = (*i).get_name();
                    if(nodeName.str() != NULL)
                         variableName = nodeName.str();
                    else
                         variableName = "";

                    declarationString += typeNameString + " " + variableName + "; ";
                  }
               break;
             }

       // ignore this case ... not really a declaration
          case V_SgCtorInitializerList:
            // printf ("Ignore the SgCtorInitializerList (constructor initializer list) \n");
               break;

          case V_SgVariableDefinition:
               printf ("ERROR: SgVariableDefinition nodes not used in AST \n");
               ROSE_ABORT();
               break;

       // default case should always be an error
          default:
               printf ("Default reached in AST_Rewrite::AccumulatedDeclarationsAttribute::generateDeclarationString() \n");
               printf ("     declaration->sage_class_name() = %s \n",declaration->sage_class_name());
               ROSE_ABORT();
               break;
        }

  // Add a space to make it easier to read (not required)
     declarationString += " ";

  // printf ("For this scope: declarationString = %s \n",declarationString.c_str());

     return declarationString;
   }

string
AST_Rewrite::AccumulatedDeclarationsAttribute::generateGlobalDeclarationString() const
   {
  // This function generates a string of global declarations required for in the intermediate file to
  // make sure that all transformation code will compile (since it could depend on declarations
  // defined within the code).

  // Since the last declaration is typically the declaration of the function that we are
  // traversing (as part of the rewrite mechanism which uses the traversal mechanism) it is
  // tempting to skp the last declaration.  But, in fact it would be required if the
  // transformation code were to specify a recursive function call!  So we leave it is and
  // a often redundently generated forward declaration to the function we are transforming
  // (rewriting).

  // error checking
     ASSERT_this();

  // Make a copy of the stack of lists becuase extracting 
  // information from the stack distroys the stack
     stack< list<SgDeclarationStatement*> > copy = stackOfLists;

  // printf ("In generateGlobalDeclarationString(): stackOfLists.size() = %" PRIuPTR " \n",stackOfLists.size());

  // Build the list of strings that will be built from the stack of lists
     list<string> declarationStringList;

  // error checking
     ROSE_ASSERT(copy.size() >= 1);

  // pop all the scopes off the stack until we get to the global scope
     while (copy.size() > 1)
        {
       // pop the stack to get to the outer level scope next
          copy.pop();
        }

     list<SgDeclarationStatement*> & scope = copy.top();

  // Note that since we only have to iterate over a single scope this code is simpler than
  // the related loop construction in the generateOpeningNonGlobalDeclarationString() member 
  // function below.
     string returnString;
     list<SgDeclarationStatement*>::const_iterator i;
     for (i=scope.begin(); i != scope.end(); i++)
        {
           ROSE_ASSERT ( (*i) != NULL );

           returnString += generateDeclarationString(*i);
        }

     return returnString;
   }

string
AST_Rewrite::AccumulatedDeclarationsAttribute::generateOpeningNonGlobalDeclarationString() const
   {
  // This function generates a string of declarations required for in the intermediate file to
  // make sure that all transformation code will compile (since it could depend on declarations
  // defined within the code).

  // error checking
     ASSERT_this();

  // printf ("In generateOpeningNonGlobalDeclarationString(): stackOfLists.size() = %" PRIuPTR " \n",stackOfLists.size());

  // Make a copy of the stack of lists becuase extracting 
  // information from the stack distroys the stack
     stack< list<SgDeclarationStatement*> > copy = stackOfLists;

  // Build the list of strings that will be built from the stack of lists
     list<string> declarationStringList;

  // Look only at non-global scopes (the last scope is the global scope by definition)
     while (copy.size() > 1)
        {
       // printf ("In loop over scopes: process a new scope copy.size() = %" PRIuPTR " \n",copy.size());

          list<SgDeclarationStatement*> & scope = copy.top();

          string declarationString;
       // Traverse in reverse order because the declarations are
       // added to the end of the list in source sequence order.
          if (scope.size() == 0)
             {
               declarationString = " { ";
               declarationStringList.push_front(declarationString);
             }
            else
             {
               list<SgDeclarationStatement*>::reverse_iterator i;
               for (i=scope.rbegin(); i != scope.rend(); i++)
                  {
                    ROSE_ASSERT ( (*i) != NULL );
                 // printf ("(*i)->sage_class_name() = %s \n",(*i)->sage_class_name());
                    if (i == scope.rbegin())
                         declarationString = " { " + generateDeclarationString(*i);
                      else
                         declarationString = generateDeclarationString(*i);

                 // Push onto the FRONT of the list (if we pushed them to the back 
                 // of the list then we would have to reverse the order of accumulation 
                 // of strings into the returnString.
                    declarationStringList.push_front(declarationString);
                  }
             }

       // pop the stack to get to the outer level scope next
          copy.pop();
        }

     string returnString;

  // Now assemble the list of strings into a single string
     list<string>::iterator r;
     for (r=declarationStringList.begin(); r != declarationStringList.end(); r++)
        {
          returnString += *r;
        }

     return returnString;
   }

string
AST_Rewrite::AccumulatedDeclarationsAttribute::generateClosingNonGlobalDeclarationString() const
   {
     string returnString;

     ASSERT_this();

  // printf ("In generateClosingNonGlobalDeclarationString(): stackOfLists.size() = %" PRIuPTR " \n",stackOfLists.size());

  // skip the last scope (the last "}")
     for (unsigned i=1; i < stackOfLists.size(); i++)
          returnString += " } ";

     return returnString;
   }

void
AST_Rewrite::AccumulatedDeclarationsAttribute::consistencyCheck() const
   {
   // This function checks for a consistant number of "{" and "}" in the
   // generated strings (a common error in development).

      string openingNonGlobalString = generateOpeningNonGlobalDeclarationString();
      string closingNonGlobalString = generateClosingNonGlobalDeclarationString();

#if 0
      printf ("openingNonGlobalString = %s \n",openingNonGlobalString.c_str());
      printf ("closingNonGlobalString = %s \n",closingNonGlobalString.c_str());
#endif

      unsigned i;
      int openingBraceCounter = 0;
      for (i = 0; i < openingNonGlobalString.length(); i++)
           openingBraceCounter += (openingNonGlobalString[i] == '{') ? 1 : 0;

      int closingBraceCounter = 0;
   // Count the closing braces in the openingNonGlobalString since
   // class declarations include both "{" and "}" braces
      for (i = 0; i < openingNonGlobalString.length(); i++)
           closingBraceCounter += (openingNonGlobalString[i] == '}') ? 1 : 0;

      for (i = 0; i < closingNonGlobalString.length(); i++)
           closingBraceCounter += (closingNonGlobalString[i] == '}') ? 1 : 0;
      
   // printf ("openingBraceCounter = %d closingBraceCounter = %d \n",openingBraceCounter,closingBraceCounter);

      ROSE_ASSERT (openingBraceCounter == closingBraceCounter);
   }

#endif

#if 0
   These are no longer needed

// **************************************************************************
//                AccumulatedFunctionParameterListAttribute
// **************************************************************************

AST_Rewrite::AccumulatedFunctionParameterListAttribute::AccumulatedFunctionParameterListAttribute()
   {
     functionParameters = NULL;
   }

void
AST_Rewrite::AccumulatedFunctionParameterListAttribute::clear()
   {
     functionParameters = NULL;
     ROSE_ASSERT (functionParameters == NULL);
   }

void
AST_Rewrite::AccumulatedFunctionParameterListAttribute::setFunctionParameters ( SgFunctionParameterList* parameters )
   {
     ROSE_ASSERT (parameters != NULL);
     functionParameters = parameters;
   }

SgFunctionParameterList*
AST_Rewrite::AccumulatedFunctionParameterListAttribute::getFunctionParameters ()
   {
     return functionParameters;
   }

#endif


