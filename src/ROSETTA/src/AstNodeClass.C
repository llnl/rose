// ################################################################
// #                           Header Files                       #
// ################################################################

#include "ROSETTA_macros.h"
#include "grammar.h"
#include "AstNodeClass.h"
#include "grammarString.h"

using namespace std;
using namespace Rose;

#define BOOL2STR(b) ((b) ? "true" : "false")

// ################################################################
// #                 NonTerminal Member Functions                 #
// ################################################################

SubclassListBuilder& SubclassListBuilder::operator|(const AstNodeClass& t) {
  children.push_back(const_cast<AstNodeClass*>(&t));
  return *this;
}

const std::vector<AstNodeClass*>& SubclassListBuilder::getList() const {
  return children;
}

bool
AstNodeClass::isInnerNode() {
  return subclasses.size()>0;
}

bool
AstNodeClass::isLeafNode() {
  return subclasses.size()==0;
}

AstNodeClass::~AstNodeClass()
   {
   }

AstNodeClass::AstNodeClass(const string& lexemeString , Grammar & X , const string& stringVar, const string& tagString,
                           bool canHaveInstances, const SubclassListBuilder & builder)
   : name((stringVar.empty() ? lexemeString : stringVar)),
     baseName((stringVar.empty() ? lexemeString : stringVar)),
     baseClass(NULL),
     subclasses(builder.getList()),
     canHaveInstances(canHaveInstances),
     lexeme(lexemeString),
     tag(tagString.empty() ? (name + "Tag") : tagString),
     lowerLevelGramaticalElement(NULL), 
     buildDefaultConstructor(false),
     predeclarationString(""), postdeclarationString(""), 
     automaticGenerationOfDestructor(true),
     automaticGenerationOfConstructor(true),
     automaticGenerationOfDataAccessFunctions(true),
     automaticGenerationOfCopyFunction(true),
     associatedGrammar(&X),
     p_isBoostSerializable(false),
     generateEssentialDataMembersConstructorImplementation(false),
     generateEnforcedDefaultConstructorImplementation(false)
{
    for (AstNodeClass *subclass: this->subclasses) {
        ROSE_ASSERT(subclass != nullptr);
        ROSE_ASSERT(subclass->getBaseClass() == nullptr);
        subclass->setBaseClass(this);
    }
    X.addGrammarElement(*this);
    ROSE_ASSERT(associatedGrammar != NULL);
}

void
AstNodeClass::insertDerivedClass(AstNodeClass *derivedClass) {
    ROSE_ASSERT(derivedClass != nullptr);
    ROSE_ASSERT(derivedClass->getBaseClass() == nullptr);
    subclasses.push_back(derivedClass);
    derivedClass->setBaseClass(this);
}

void
AstNodeClass::insertDerivedClass(std::vector<AstNodeClass*> &terminalList, const std::string &name) {
    AstNodeClass *derived = lookupTerminal(terminalList, name);
    ROSE_ASSERT(derived != nullptr);
    insertDerivedClass(derived);
}

const std::string&
AstNodeClass::getCppCondition() const {
    return cppCondition;
}

void
AstNodeClass::setCppCondition(const std::string &s) {
    cppCondition = s;
}

void
AstNodeClass::appendPreDefinitionText(const std::string &text) {
    preDefinitionText += text;
}

bool
AstNodeClass::isBoostSerializable() const {
    return p_isBoostSerializable;
}

void
AstNodeClass::isBoostSerializable(bool b) {
    p_isBoostSerializable = b;
}

bool
AstNodeClass::useSmallHeader() const {
    return useSmallHeader_;
}

void
AstNodeClass::useSmallHeader(bool b) {
    useSmallHeader_ = b;
}

void
AstNodeClass::setBaseClass(AstNodeClass* bc) {baseClass = bc;}

AstNodeClass*
AstNodeClass::getBaseClass() const {return baseClass;}

bool
AstNodeClass::isDerivedFrom(const string & s) const 
   {
  // DQ (10/11/2014): This function checks if the input name is a base class of this AstNodeClass.
      printf ("In AstNodeClass::isBaseClass(): s = %s this = %p = %s \n",s.c_str(),this,this->getName().c_str());

     bool returnValue = false;
     AstNodeClass* temp = const_cast<AstNodeClass*>(this);
     while (temp != NULL && temp->getName() != s)
        {
          temp = temp->getBaseClass();
        }

     if (temp != NULL && temp->getName() == s)
        {
          printf ("Found matching AstNodeClass name: s = %s \n",s.c_str());
          returnValue = true;
        }

     return returnValue;
   }

void
AstNodeClass::setCanHaveInstances(bool chi) {canHaveInstances = chi;}

bool
AstNodeClass::getCanHaveInstances() const {return canHaveInstances;}

void
AstNodeClass::setBuildDefaultConstructor ( bool X )
   {
     buildDefaultConstructor = X;
   }

bool
AstNodeClass::getBuildDefaultConstructor () const
   {
     ROSE_ASSERT (buildDefaultConstructor == true || buildDefaultConstructor == false);
     return buildDefaultConstructor;
   }

#define DEBUG_double_delete 0

// AJ ( 10/26/2004)
// 
string
AstNodeClass::buildDestructorBody ()
   {
     string returnString;
     vector<GrammarString *> localList;
     vector<GrammarString *> localExcludeList;
     vector<GrammarString *>::iterator stringListIterator;

  // Initialize with local node data
     localList        = getMemberDataPrototypeList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST);
     localExcludeList = getMemberDataPrototypeList(AstNodeClass::LOCAL_LIST,AstNodeClass::EXCLUDE_LIST);

  // Now edit the list to remove elements appearing within the exclude list
     Grammar::editStringList ( localList, localExcludeList );

     returnString += "\n";

     for( stringListIterator = localList.begin();
          stringListIterator != localList.end();
          stringListIterator++ )
        {
          if (!((*stringListIterator)->getToBeDeleted() == DEF_DELETE))
             continue;

          std::string typeName = (*stringListIterator)->getTypeNameString();
          std::string varName = (*stringListIterator)->getVariableNameString();

          std::string tempString;

          if ( typeName.find("char*") != string::npos && typeName.find("char**") == string::npos )
               tempString = "[] ";

          if ( typeName.find("Sg") == 0 ) {
            tempString  = "    if (p_$DATA && p_$DATA->get_freepointer() == AST_FileIO::IS_VALID_POINTER()) { delete " + tempString + "p_$DATA; }\n";
#if DEBUG_double_delete
            tempString += "    else if (p_$DATA) { ROSE_ASSERT(false); }";
#endif
          }
          if ( typeName == " rose_hash_multimap*" || typeName == "OSEAttributesListContainerPtr" ) {
            tempString  = "    if (p_$DATA) { delete " + tempString + "p_$DATA; }\n";
          }
          tempString = StringUtility::copyEdit(tempString, "$DATA", varName);
          returnString += tempString;
        }

     returnString += "\n";

  // bool exitAsTest = false;

  // Now generate code to reset the pointers to default values.
     for( stringListIterator = localList.begin();
          stringListIterator != localList.end();
          stringListIterator++ )
        {
       // Skip any staticly defined data
          string typeString = (*stringListIterator)->getTypeNameString();
          string variableNameString = (*stringListIterator)->getVariableNameString();
          string initializerString  = (*stringListIterator)->getDefaultInitializerString();
       // DQ (5/24/2006): The SgNode::operator delete() will set the p_freepointer so don't fool with it in the descructor!
          if (typeString.find("static ") == string::npos && (initializerString.empty() == false) && (variableNameString != "freepointer") )
             {
            // string tempString = "     p_$DATA $DEFAULT_VALUE;\n";
               string tempString = (*stringListIterator)->buildDestructorSource();

               tempString = StringUtility::copyEdit (tempString,"$DATA",variableNameString);
               tempString = StringUtility::copyEdit (tempString,"$DEFAULT_VALUE",initializerString);

               returnString += tempString;
             }
        }

     ROSE_ASSERT (localExcludeList.size() == 0);

   returnString = "  if (p_freepointer == AST_FileIO::IS_VALID_POINTER()) {\n" + returnString + "\n  }";
#if DEBUG_double_delete
   returnString = "ROSE_ASSERT(this);\n" + returnString + " else {\n    ROSE_ASSERT(false);\n  }";
#endif
   returnString += "\n";

  // printf ("In AstNodeClass::buildDestructorBody(): returnString = %s \n",returnString.c_str());
     return returnString;
   }

// DQ (9/28/2022): Fixing compiler warning for argument not used.
// string AstNodeClass::buildConstructorBody ( bool withInitializers, ConstructParamEnum config )
string
AstNodeClass::buildConstructorBody ( bool withInitializers )
   {
  // This function builds a string that represents the initialization of member data
  // if the default initializers are used (for default constructors) then all member 
  // data is initialized using the initializer (all member data should be defined 
  // with an initializer string).  If withInitializers == false (for non-default 
  // constructors) then all member data is initialized with the name used for the 
  // for the associated constructor parameter.  Excluded data members are initialized
  // with their default initializer.

     string returnString;

     vector<GrammarString *> localList;
     vector<GrammarString *> localExcludeList;
     vector<GrammarString *>::iterator stringListIterator;

  // DQ (12/7/2003): This is never true, or so it seems
     ROSE_ASSERT (withInitializers == false);

  // Initialize with local node data
     localList        = getMemberDataPrototypeList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST);
     localExcludeList = getMemberDataPrototypeList(AstNodeClass::LOCAL_LIST,AstNodeClass::EXCLUDE_LIST);

  // Now edit the list to remove elements appearing within the exclude list
     Grammar::editStringList ( localList, localExcludeList );

     for( stringListIterator = localList.begin();
          stringListIterator != localList.end();
          stringListIterator++ )
        {
       // QY 11/9/04  added additional types of constructor parameters 
          string variableNameString = (*stringListIterator)->getVariableNameString();
          switch ( (*stringListIterator)->getIsInConstructorParameterList() )
             {
               case NO_CONSTRUCTOR_PARAMETER:
                    if (string( (*stringListIterator)->getDefaultInitializerString()) != "")
                       {
                         returnString = returnString + "     p_" + variableNameString + " " + 
                                        (*stringListIterator)->getDefaultInitializerString() + ";\n";
                       }
                    break;

               case CONSTRUCTOR_PARAMETER:
                    returnString = returnString + "     p_" + variableNameString+ " = " + variableNameString + ";\n";
                    break;
               default:
                    ROSE_ABORT();
             }
       }
     ASSERT_require(localExcludeList.size() == 0);

     return returnString;
   }

string
AstNodeClass::buildConstructorBodyForEssentialDataMembers() {
    string returnString;
    vector<GrammarString*> includeList;
    vector<GrammarString*> excludeList;

    // now generate the additions to the lists from the parent node subtree lists
    associatedGrammar->generateStringListsFromLocalLists(*this, includeList, excludeList, &AstNodeClass::getMemberDataPrototypeList);

    for (auto it = includeList.begin(); it != includeList.end(); ++it) {
        string variableNameString{(*it)->getVariableNameString()};
        if (!associatedGrammar->isFilteredMemberVariable(variableNameString)) {
            returnString = returnString + "    p_" + variableNameString + " = " + variableNameString + ";\n";
        }
    }

    return returnString;
}

StringUtility::FileWithLineNumbers
AstNodeClass::buildCopyMemberFunctionHeader() {
    StringUtility::FileWithLineNumbers returnString;

    if (automaticGenerationOfCopyFunction) {
        if (baseName == "Node") {
            returnString.push_back(StringUtility::StringWithLineNumber("    virtual SgNode* copy(SgCopyHelp& help) const;", "" /* "<copy member function>" */, 1));
        }
        else {
            returnString.push_back(StringUtility::StringWithLineNumber("    virtual SgNode* copy(SgCopyHelp& help) const override;", "" /* "<copy member function>" */, 1));
        }
    }
    else {
        returnString.push_back(StringUtility::StringWithLineNumber("    // copy functions omitted for $CLASSNAME", "" /* "<copy member function>" */, 1));
    }

    return returnString;
}

StringUtility::FileWithLineNumbers
AstNodeClass::buildCopyMemberFunctionSource() {
    // This function builds the copy member function's body

    StringUtility::FileWithLineNumbers returnString;

    if (automaticGenerationOfCopyFunction == true) {
        string filename{"../Grammar/copyMemberFunction.macro"};
        StringUtility::FileWithLineNumbers functionTemplateString{Grammar::readFileWithPos(filename)};
        bool emptyConstructorArg{!generateConstructor() || getBuildDefaultConstructor()};
        string constructArgCopy, constructArgList, postConstructCopy;

        ASSERT_require(getBuildDefaultConstructor() == false);

        for (AstNodeClass* t = this; t != nullptr; t = t->getBaseClass()) {
            string constructArgCopy1, constructArgList1, postConstructCopy1;
            vector<GrammarString*> copyList;

            copyList = t->getMemberDataPrototypeList(AstNodeClass::LOCAL_LIST, AstNodeClass::INCLUDE_LIST);

            for (auto it = copyList.begin(); it != copyList.end(); ++it) {
                GrammarString* data{*it};
                if (data->getToBeCopied() == NO_COPY_DATA) {
                    continue;
                }

                // Have all the code generation be in the lower level function (not mixed up)
                string varNameString{data->getVariableNameString() + "_copy"};
                string varDecl{"    " + data->getTypeNameString() + " " + varNameString + ";\n"};

                if (!emptyConstructorArg && data->getIsInConstructorParameterList() != NO_CONSTRUCTOR_PARAMETER) {
                    string comment{"    // Copy constructor parameter data member: " + varNameString + "\n"};
                    constructArgCopy1 += comment;

                    // Code generated to build constructor parameters can't set the "result->xxx"
                    // data members since the "result" class have not been built (see generated code
                    // for more examples).
                    bool buildConstructorArgument{true};

                    constructArgCopy1 += data->buildCopyMemberFunctionSource(buildConstructorArgument);
                    constructArgList1 += ", " + varNameString;

                    string setParentString = data->buildCopyMemberFunctionSetParentSource();
                    postConstructCopy += setParentString;
                }
                else {
                    // Code built generated to build constructor parameters can't set the "result->xxx"
                    // data members since the "result" class have not been built (see generated code
                    // for more examples).
                    bool buildConstructorArgument{false};

                    switch (data->automaticGenerationOfDataAccessFunctions) {
                      case NO_ACCESS_FUNCTIONS: {
                        string localVarNameString{"result->p_" + data->getVariableNameString()};
                        varNameString = localVarNameString;

                        string comment{"    // Copy non-constructor parameter data member (no access function): " + varNameString + "\n"};
                        postConstructCopy1 += comment;
                        postConstructCopy1 += data->buildCopyMemberFunctionSource(buildConstructorArgument);

                        string setParentString = data->buildCopyMemberFunctionSetParentSource();
                        postConstructCopy1 += setParentString;
                        break;
                      }

                      case BUILD_LIST_ACCESS_FUNCTIONS: {
                        string localVarNameString{"result->get_" + data->getVariableNameString() + "()"};
                        varNameString = localVarNameString;

                        string comment{"    // Copy non-constructor parameter data member (list access function): " + varNameString + "\n"};
                        postConstructCopy1 += comment;

                        postConstructCopy1 += data->buildCopyMemberFunctionSource(buildConstructorArgument);
                        string setParentString = data->buildCopyMemberFunctionSetParentSource();
                        postConstructCopy1 += setParentString;
                        break;
                      }

                      case BUILD_ACCESS_FUNCTIONS:
                      case BUILD_FLAG_ACCESS_FUNCTIONS: {
                        string comment{"  // Copy non-constructor parameter data member (access function): " + varNameString + "\n"};
                        postConstructCopy1 += comment;

                        postConstructCopy1 += data->buildCopyMemberFunctionSource(buildConstructorArgument);
                        string setParentString = data->buildCopyMemberFunctionSetParentSource();
                        postConstructCopy1 += setParentString;
                        break;
                      }

                      default:
                        ASSERT_require2(false, "Default reached in AstNodeClass::buildCopyMemberFunctionSource\n");
                    }
                }
            }

            constructArgCopy  = constructArgCopy1  + constructArgCopy;
            constructArgList  = constructArgList1  + constructArgList;
            postConstructCopy = postConstructCopy1 + postConstructCopy;
        }

        // Not fond of this approach to fixing up the leading "," in what is generated above.
        if (constructArgList != "") {
            constructArgList = constructArgList.c_str() + 1; // get rid of the preceding ','
        }

        returnString = functionTemplateString;
        returnString = StringUtility::copyEdit(returnString, "virtual ", "");
        returnString = StringUtility::copyEdit(returnString, "$CONSTRUCT_ARG_COPY", constructArgCopy);
        returnString = StringUtility::copyEdit(returnString, "$CONSTRUCT_ARG_LIST", constructArgList);
        returnString = StringUtility::copyEdit(returnString, "$POST_CONSTRUCT_COPY", postConstructCopy);
    }

    return returnString;
}

void AstNodeClass::setConnectionToLowerLevelGrammar(AstNodeClass& X) {
    lowerLevelGramaticalElement = &X;
    ASSERT_not_null(lowerLevelGramaticalElement);
}

AstNodeClass& AstNodeClass::getConnectionToLowerLevelGrammar() {
    ASSERT_not_null(lowerLevelGramaticalElement);
    return *lowerLevelGramaticalElement;
}

void
AstNodeClass::show(size_t indent) const {
    ASSERT_this();
    if (subclasses.empty()) { // AstNodeClass
        printf ("%s ",name.c_str());
    } else { // Nonterminal
        for (size_t i=0; i < indent; i++) {
            printf ("..");
        }
        for (auto it = subclasses.begin(); it != subclasses.end(); ++it) {
            ASSERT_not_null(*it);
            displayName(); cout << " -> "; (*it)->AstNodeClass::show(); cout << ";" << endl; //MS edge
            if ((*it)->getCanHaveInstances()) {
                (*it)->show();
                cout << "[style=bold];" << endl; // Terminals are bold! MS node
            }
        }
    }
}

void
AstNodeClass::displayName() const {
    ASSERT_this();
    printf ("%s ", name.c_str());
}

void 
AstNodeClass::setLexeme(const string& label) {
    this->lexeme = label;
}

const string&
AstNodeClass::getLexeme() const {
    return lexeme;
}

void 
AstNodeClass::setName(const string& label, const string& tagName) {
    // baseName is the same as "name" but name will be modified later to
    // include the $GRAMMAR_PREFIX_ we need to keep the base around so that
    // the $PARANT_GRAMMARS_BASE_ can be applied in connections to lower
    // level grammars (e.g. parser's input parameters are elements of the
    // parent's grammar.
    ASSERT_this();

    this->baseName = this->name = label;

    // set the lexeme to match the name
    setLexeme(name);

    // Setup the tag name using the name if the second parameter is NULL
    if (tagName != "") {
        this->tag = tagName;
    }
    else {
        this->tag = name + "Tag";
    }
}

void
AstNodeClass::setGrammar ( Grammar* grammarPointer )
   {
     ASSERT_this();
     ROSE_ASSERT (grammarPointer != NULL);
     ((AstNodeClass*)this)->associatedGrammar = grammarPointer;
   }

Grammar*
AstNodeClass::getGrammar() const
   {
     ASSERT_this();
     ROSE_ASSERT (associatedGrammar != NULL);
     return associatedGrammar;
   }

const string&
AstNodeClass::getName () const
   {
     ASSERT_this();
     ROSE_ASSERT (!name.empty());
     return name;
   }

const string&
AstNodeClass::getBaseName () const
   {
     ASSERT_this();
     return baseName;
   }


void
AstNodeClass::addGrammarPrefixToName()
   {
  // In this function we prefix the grammar's name to the names of all terminals and nonterminals
  // This allows them to have unique names in the global namespace (allowing us to have multiple
  // grammars in use (each built from the same base level C++ grammar (using the new version of SAGE).
  // However we don't want to modify the names of the tags unless they are a part of a higher level 
  // grammar.  This is done to preserve the names already used in SAGE (and in the EDG/SAGE interface 
  // code) which we don't want to change.

  // To match the names used in SAGE the base level grammar will use "Sg" as its name

     string grammarName = "";
     ROSE_ASSERT (getGrammar() != NULL);
     grammarName = getGrammar()->getGrammarPrefixName();

  // printf ("In AstNodeClass::addGrammarPrefixToName: grammarName = %s \n",grammarName);

  // Error Checking! Check to make sure that grammar's name does not already exist in the
  // AstNodeClass's name.  This helps make sure that elements are not represented twice!
     if (GrammarString::isContainedIn(name, grammarName)) {
         std::cerr <<"Grammar's name already exists in the AstNodeClass's name"
                   <<"; name=" <<name <<", grammarName=" <<grammarName <<"\n";
     }
     ROSE_ASSERT(!GrammarString::isContainedIn(name,grammarName));

  // Set the name to include the grammar's prefix
     string newNameWithPrefix = grammarName + name;
     this->name = newNameWithPrefix;

     string grammarTagName = "";
     if (getGrammar()->parentGrammar != NULL)
          grammarTagName = grammarName;

     string tempTag = grammarTagName + tag;
     this->tag  = tempTag;

   }

void 
AstNodeClass::setTagName ( const string& label )
   {
  // printf ("In AstNodeClass::setTagName ( label = %s ) \n",
  //        (label == NULL) ? "NULL" : label);

     this->tag = label;
  // setLexeme(label);

  // Prepend the grammar's name to the tag so that multiple 
  // grammars will have unique tags
  // ((AstNodeClass*)this)->tag = stringConcatinate(getGrammar()->getGrammarName(),tag);
     string grammarName = "";

     this->tag = grammarName + tag;
   }

const string&
AstNodeClass::getTagName () const
   {
     return tag;
   }

void
AstNodeClass::setFunctionPrototype ( const GrammarString & inputMemberFunction )
   {
     AstNodeClass::addElementToList
        ( getMemberFunctionPrototypeList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST),
          inputMemberFunction);
   }

// This was originally a CPP macro instead of a function! I'm not aware of the reason for that, so I'm leaving it. [Matzke]
#define SETUP_MARKER_STRINGS_MACRO                                                                                             \
    string functionString;                                                                                                     \
    string errorMessage;                                                                                                       \
    /* First try the new approach where we locate everything at one place instead of five places; the five being: */           \
    /*   (1) The #include's located at the top of src/ROSETTA/Grammar/xxx.code files */                                        \
    /*   (2) The class declaration and its properties in src/ROSETTA/src/xxx.C files */                                        \
    /*   (3) Extra declarations that aren't needed by ROSETTA, in the middle src/ROSETTA/Grammar/xxx.code files */             \
    /*   (4) Definitions for the extra members, at the end of src/ROSETTA/Grammar/xxx.code files */                            \
    /*   (5) Documentation for the ROSETTA members in docs/testDoxygen/xxx.docs files. */                                      \
    /* We have to use try/catch because Grammar::extractStringFromFile throws (used to abort) if it can't find the string. */  \
    try {                                                                                                                      \
        /* Can't use boost::regex here due to policy prohibition against boost dependencies in ROSETTA */                      \
        string startMarkerString = "#if defined(" + markerString + ") || defined(DOCUMENTATION)";                              \
        string endMarkerString = "#endif // " + markerString;                                                                  \
        string directory;                                                                                                      \
        functionString = StringUtility::toString(Grammar::extractStringFromFile(startMarkerString, endMarkerString,            \
                                                                                filename));                                    \
    } catch (const std::runtime_error &e) {                                                                                    \
        if (strstr(e.what(), "could not locate startMarker") == NULL) {                                                        \
            errorMessage = e.what();                                                                                           \
        }                                                                                                                      \
    }                                                                                                                          \
    /* If new way didn't work, then try the old way, looking for START and END tags */                                         \
    if (functionString.empty() && errorMessage.empty()) {                                                                      \
        string startSuffix = "_START";                                                                                         \
        string endSuffix   = "_END";                                                                                           \
        string startMarkerString = markerString + startSuffix;                                                                 \
        string endMarkerString = markerString + endSuffix;                                                                     \
        string directory;                                                                                                      \
        try {                                                                                                                  \
            functionString = StringUtility::toString(Grammar::extractStringFromFile(startMarkerString, endMarkerString,        \
                                                                                        filename));                            \
        } catch (const std::runtime_error &e) {                                                                                \
            errorMessage = e.what();                                                                                           \
        }                                                                                                                      \
    }                                                                                                                          \
    /* Print the error message that was originally emitted by extractStringFromFile just before it aborted. */                 \
    if (!errorMessage.empty()) {                                                                                               \
        fprintf(stderr, "%s\n", errorMessage.c_str());                                                                         \
        ROSE_ASSERT(false);                                                                                                    \
    }                                                                                                                          \
    GrammarString* codeString = new GrammarString(functionString);                                                             \
    codeString->setVirtual(pureVirtual);

GrammarString* AstNodeClass::setupMarkerStrings(string markerString,string filename, bool pureVirtual) {
    SETUP_MARKER_STRINGS_MACRO;
    return codeString;
}

void
AstNodeClass::setFunctionPrototype ( const string& markerString, const string& filename, bool pureVirtual )
   {
  // We might want to include the path name into the filename string
  // so we don't need to have a directory input parameter
     SETUP_MARKER_STRINGS_MACRO
       // BP : 10/25/2001, fixing memory leak
     setFunctionPrototype (*codeString);
   }

void
AstNodeClass::setFunctionSource ( const string& markerString, const string& filename, bool pureVirtual )
   {
  // We might want to include the path name into the filename string
  // so we don't need to have a directory input parameter
     SETUP_MARKER_STRINGS_MACRO
       // BP : 10/25/2001, fixing memory leak
  // memberFunctionSourceList.addElement(*codeString);
     AstNodeClass::addElementToList ( getMemberFunctionSourceList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST),
                                           *codeString );
   }

void
AstNodeClass::setSubTreeFunctionPrototype ( const GrammarString & inputMemberFunction )
   {
  // When we add a string of code to the subtree we have to also add it explicitly to 
  // the root of the subtree (calling "setFunction" takes care of this).
  // setFunction (inputMemberFunction);
  // This function must be run ONLY after the grammar tree is built!
  // ROSE_ASSERT (grammarSubTree != NULL);
  // subTreeMemberFunctionPrototypeList.addElement(inputMemberFunction);
     AstNodeClass::addElementToList ( getMemberFunctionPrototypeList(AstNodeClass::SUBTREE_LIST,AstNodeClass::INCLUDE_LIST),
                                           inputMemberFunction );
     ROSE_ASSERT (getMemberFunctionPrototypeList(AstNodeClass::SUBTREE_LIST,AstNodeClass::INCLUDE_LIST).size() > 0);
   }

void
AstNodeClass::setSubTreeFunctionPrototype ( const string& markerString, const string& filename, bool pureVirtual )
   {
  // We might want to include the path name into the filename string
  // so we don't need to have a directory input parameter
     SETUP_MARKER_STRINGS_MACRO
     setSubTreeFunctionPrototype (*codeString);
   }

void
AstNodeClass::setSubTreeFunctionSource ( const string& markerString, const string& filename, bool pureVirtual )
   {
  // We might want to include the path name into the filename string
  // so we don't need to have a directory input parameter
     SETUP_MARKER_STRINGS_MACRO
       AstNodeClass::addElementToList ( getMemberFunctionSourceList(AstNodeClass::SUBTREE_LIST,AstNodeClass::INCLUDE_LIST),
                                        *codeString );
   }


void
AstNodeClass::setPredeclarationString  ( const string& markerString, const string& filename )
   {
     bool pureVirtual = false;
     SETUP_MARKER_STRINGS_MACRO
       
     setPredeclarationString(functionString);
   }

void
AstNodeClass::setPostdeclarationString  ( const string& markerString, const string& filename )
   {
     bool pureVirtual = false;
     SETUP_MARKER_STRINGS_MACRO
     setPostdeclarationString(functionString);
   }

void
AstNodeClass::setPredeclarationString  ( const string& declarationString )
   {
  // This function allows the specification of declarations 
  // that are to appear prefixed to a class declaration
     predeclarationString = declarationString;
   }

void
AstNodeClass::setPostdeclarationString ( const string& declarationString )
   {
  // This function allows the specification of declarations 
  // that are to appear postfixed to a class declaration
     postdeclarationString = declarationString;
   }

const string&
AstNodeClass::getPredeclarationString () const
   {
     return predeclarationString;
   }

const string&
AstNodeClass::getPostdeclarationString() const
   {
     return postdeclarationString;
   }

void
AstNodeClass::setDataPrototype ( const GrammarString & inputMemberData)
{
  // MK: This is the correct place to put the memberData info
  AstNodeClass::addElementToList (getMemberDataPrototypeList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST),
                                       inputMemberData );
  
  // Once the string for the class declaration is built we have to 
  // construct one for the data source code list.  This will be a 
  // string representing the access functions (get and set functions)
  // and will be placed into the buildGrammarStringForSourceList (inputMemberData);
  
  bool pureVirtual = false;
  
  string accessFunctionString = buildDataAccessFunctions (inputMemberData);
  GrammarString* sourceCodeString = new GrammarString(accessFunctionString);
  ROSE_ASSERT(sourceCodeString != NULL);
  sourceCodeString->setVirtual(pureVirtual);

  vector<GrammarString*>& l = getMemberFunctionSourceList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST);
  // Data access "functions" should be placed into the LOCAL_LIST since
  // they are accessable though the base classes by definition (of C++)
  AstNodeClass::addElementToList ( l, *sourceCodeString );
  
}

string
AstNodeClass::buildDataAccessFunctions ( const GrammarString & inputMemberData)
   {
  // This function builds the access functions that will be used with the 
  // associated data declared on this AstNodeClass (or non-AstNodeClass).

     string returnString = "";
     string filename = "";
     BuildAccessEnum config = inputMemberData.generateDataAccessFunctions();
  // NOTE: Here we check and see if we really want to use this Data Access String
     if ( (automaticGenerationOfDataAccessFunctions == false) || config == NO_ACCESS_FUNCTIONS )
        {
          return "\n";
        }
     switch (config)
        {
          case BUILD_ACCESS_FUNCTIONS:
               filename = "../Grammar/dataMemberAccessFunctions.macro";
               break;
          case BUILD_FLAG_ACCESS_FUNCTIONS:
               filename = "../Grammar/flagMemberAccessFunctions.macro";
               break;
          case BUILD_LIST_ACCESS_FUNCTIONS:
               filename = "../Grammar/listMemberAccessFunctions.macro";
               break;
          default:
               ROSE_ABORT();
        }

     string markerString = "DATA_ACCESS_FUNCTION";

     bool pureVirtual = false;
     SETUP_MARKER_STRINGS_MACRO

  // We can't edit anything that would be tree position dependent be 
  // we need to get the correct variable names into place so do just 
  // that editing here!
     string typeName     = inputMemberData.getTypeNameString();
     string variableName = inputMemberData.getVariableNameString();

  // DQ (12/20/2005): strip the "static " substring from the typeName
  // so that we generate non-static member access functions and non-static 
  // parameter variable types (which are not legal C++).
     if (typeName.find("static ") != string::npos )
        {
          typeName = typeName.substr(7 /* strlen("static ") */);
        }

     functionString = GrammarString::copyEdit (functionString,"$DATA_TYPE",typeName);
     functionString = GrammarString::copyEdit (functionString,"$DATA",variableName);

     string setParentFunctionCallString = "";  // default setting

  // Skip generation of call to set_parent() member function
  // Check if the type name contains a '*' indicating that it is a pointer 
  // (then assume it is a pointer to a class which we will further assume 
  // has a specific interface which includes a 
  // "set_parent ( const CLASSNAME* thisPointer )" member function
  // QY: generate set_parent only when the data member is in traversal
   //  if ( toBeTraversed && strstr(typeName,"*") != NULL )
     if ( typeName.find('*') != string::npos )
        {
       // printf ("found a pointer type (type = %s) \n",typeName);

       // Now check that is it not something simple like "char*" since char 
       // (as a primative type) would not be a class with an interface.
       // We assume that the we don't have two "*" represented as "* *".
          if ( (typeName.find("char") != string::npos) || (typeName.find("**") != string::npos) )
             {
            // printf ("opps!, it is a (type = %s) \n",typeName);
             }
            else
             {
            // Use the older way of handling this (sometimes causes parents to be types (but for a class referenced in a typedef as in 
            //      typedef int (doubleArray::*doubleArrayMemberVoidFunctionPointerType) (void) const;
            // This seems to be OK, as long as the class declaration was a copy and not the original defining declaration
               string tempString = "\n#if DEBUG_SAGE_ACCESS_FUNCTIONS\n     if (p_$DATA != NULL && $DATA != NULL && p_$DATA != $DATA)\n        {\n          printf (\"Warning: $DATA = %p overwriting valid pointer p_$DATA = %p \\n\",$DATA,p_$DATA);\n#if DEBUG_SAGE_ACCESS_FUNCTIONS_ASSERTION\n          printf (\"Error fails assertion (p_$DATA != NULL && $DATA != NULL && p_$DATA != $DATA) is false\\n\");\n          ROSE_ASSERT(false);\n#endif\n        }\n#endif";
               tempString = GrammarString::copyEdit (tempString,"$DATA",variableName);
               setParentFunctionCallString = tempString;
             }
        }

     functionString = GrammarString::copyEdit (functionString,"$TEST_DATA_POINTER",setParentFunctionCallString);

     delete codeString;
     return functionString;
   }

void
AstNodeClass::setDataPrototype (
     const string& inputTypeNameString,
     const string& inputVariableNameString,
     const string& inputDefaultInitializer,
     const ConstructParamEnum& constructorParameter,
     const BuildAccessEnum& buildAccessDataFunctions,
     const TraversalEnum& toBeTraversedDuringTreeTraversal,
     const DeleteEnum& delete_flag,
     const CopyConfigEnum& toBeCopied)
   {
     GrammarString *temp = 
          new GrammarString (inputTypeNameString,
                             inputVariableNameString, 
                             inputDefaultInitializer,
                             constructorParameter,
                             buildAccessDataFunctions,
                             toBeTraversedDuringTreeTraversal,
                             delete_flag,
                             toBeCopied);
     ROSE_ASSERT(temp != NULL);
     setDataPrototype (*temp);
   }

// Mechanism for excluding code from specific node or subtrees
void
AstNodeClass::excludeFunctionPrototype ( const GrammarString & inputMemberFunction )
   {
     AstNodeClass::addElementToList
        ( getMemberFunctionPrototypeList(AstNodeClass::LOCAL_LIST,AstNodeClass::EXCLUDE_LIST),inputMemberFunction);
   }

void 
AstNodeClass::excludeFunctionPrototype
   ( const string& markerString, const string& filename, bool pureVirtual )
   {
     SETUP_MARKER_STRINGS_MACRO
     AstNodeClass::addElementToList (
          getMemberFunctionPrototypeList (AstNodeClass::LOCAL_LIST,AstNodeClass::EXCLUDE_LIST), *codeString);
   }

void
AstNodeClass::excludeSubTreeFunctionPrototype ( const GrammarString & inputMemberFunction )
   {
     AstNodeClass::addElementToList ( 
          getMemberFunctionPrototypeList(AstNodeClass::SUBTREE_LIST,AstNodeClass::EXCLUDE_LIST),
          inputMemberFunction);
   }

void 
AstNodeClass::excludeSubTreeFunctionPrototype 
   ( const string& markerString, const string& filename, bool pureVirtual )
   {
     SETUP_MARKER_STRINGS_MACRO
     AstNodeClass::addElementToList(
          getMemberFunctionPrototypeList(AstNodeClass::SUBTREE_LIST,AstNodeClass::EXCLUDE_LIST),
          *codeString);
   }

void
AstNodeClass::excludeSubTreeDataPrototype ( const GrammarString & inputMemberData )
{
  // Note that the exclusion of data works slightly differently than for function prototypes
  AstNodeClass::addElementToList(getMemberDataPrototypeList(AstNodeClass::SUBTREE_LIST,AstNodeClass::EXCLUDE_LIST), 
                                      inputMemberData);
}

void
AstNodeClass::excludeSubTreeDataPrototype (
     const string& inputTypeNameString, 
     const string& inputVariableNameString, 
     const string& inputDefaultInitializer )
   {
     GrammarString *temp = 
          new GrammarString (inputTypeNameString,
                             inputVariableNameString, 
                             inputDefaultInitializer,
                             CONSTRUCTOR_PARAMETER,
                             BUILD_ACCESS_FUNCTIONS,
                             DEF_TRAVERSAL, NO_DELETE, NO_COPY_DATA);
     excludeSubTreeDataPrototype (*temp);
   }

void 
AstNodeClass::excludeFunctionSource           
   ( const string& markerString, const string& filename, bool pureVirtual )
   {
     SETUP_MARKER_STRINGS_MACRO
     AstNodeClass::addElementToList(
          getMemberFunctionSourceList(AstNodeClass::LOCAL_LIST,AstNodeClass::EXCLUDE_LIST),
          *codeString);
   }

void 
AstNodeClass::excludeSubTreeFunctionSource
   ( const string& markerString, const string& filename, bool pureVirtual )
   {
     SETUP_MARKER_STRINGS_MACRO
     AstNodeClass::addElementToList(
          getMemberFunctionSourceList (AstNodeClass::SUBTREE_LIST,AstNodeClass::EXCLUDE_LIST),
          *codeString);
   }

// test of work around for Insure++ (removing the const declaration to avoid generation of a copy)
#if !ROSE_MICROSOFT_OS
#if INSURE_BUG
void AstNodeClass::addElementToList ( vector<GrammarString *> & targetList, GrammarString & element )
#else
void AstNodeClass::addElementToList ( vector<GrammarString *> & targetList, const GrammarString & element )
#endif
#else
void AstNodeClass::addElementToList ( vector<GrammarString *> & targetList, const GrammarString & element )
#endif
   {
  // This function abstracts the details of adding GrammarString objects to the lists
  // that are stored internally.  It allows us to implement tests to check for redundencies in 
  // the list of elements (for now it allows us to test the != operator which will be used heavily later)

  // printf ("Inside of AstNodeClass::addElementToList \n");

     int i = 0;
     bool duplicateEntryFound = false;
     vector<GrammarString *>::iterator stringListIterator;
  // Error checking (make sure the element is not already in the list!)
     for( stringListIterator = targetList.begin();
          stringListIterator != targetList.end();
          stringListIterator++ )
       {
         if (*stringListIterator == &element)
           {
             printf ("Duplicate entry found in list! (targetList[%d] == element) \n",i);
             duplicateEntryFound = true;
             
             // If any of the GrammarString objects is marked to skip construction 
             // of access functions (for all the data at a node!) then mark it in the list's copy
             if (element.generateDataAccessFunctions() == NO_ACCESS_FUNCTIONS)
               (*stringListIterator)->setAutomaticGenerationOfDataAccessFunctions(NO_ACCESS_FUNCTIONS);

           }

         i++;
         // ROSE_ASSERT (targetList[i] != element);
       }

  // Since the grammar tree is not build yet we cannot search the parents
  // so this abstraction for adding elements to the lists is not used to
  // exclude any elements (this is post processed).
     if (duplicateEntryFound == false)
       {
         const GrammarString *const &tmpRef = new GrammarString(element);
         targetList.push_back( (GrammarString *const &) tmpRef );
       }
   }

vector<GrammarString *> & AstNodeClass::getMemberFunctionPrototypeList(int i, int j) const
   {
  // printf ("Inside of AstNodeClass::getMemberFunctionPrototypeList() \n");
     return (vector<GrammarString *> &) memberFunctionPrototypeList[i][j];
  // return memberFunctionPrototypeList[i][j];
   }

vector<GrammarString *> & AstNodeClass::getMemberDataPrototypeList(int i, int j) const
   {
     return (vector<GrammarString *> &) memberDataPrototypeList[i][j];
   }

vector<GrammarString *> & AstNodeClass::getMemberFunctionSourceList(int i, int j) const
   {
     return (vector<GrammarString *> &) memberFunctionSourceList[i][j];
   }

vector<GrammarString *> & AstNodeClass::getEditSubstituteTargetList( int i, int j ) const
   {
     return (vector<GrammarString *> &) editSubstituteTargetList[i][j];
   }

vector<GrammarString *> & AstNodeClass::getEditSubstituteSourceList( int i, int j ) const
   {
     return (vector<GrammarString *> &) editSubstituteSourceList[i][j];
   }


// This marco adds the oldString and newString to lists that are later used to
// drive the substitution mechanism.
#define EDIT_SUBSTITUTE_MACRO(X,Y) \
     GrammarString* targetString = new GrammarString (oldString);                 \
     GrammarString* sourceString = new GrammarString (newString);                 \
     getEditSubstituteTargetList(AstNodeClass:: X ,AstNodeClass:: Y ).push_back(targetString);  \
     getEditSubstituteSourceList(AstNodeClass:: X ,AstNodeClass:: Y ).push_back(sourceString);  \
     ROSE_ASSERT (getEditSubstituteTargetList(AstNodeClass:: X,AstNodeClass:: Y ).size() == \
                  getEditSubstituteSourceList(AstNodeClass:: X,AstNodeClass:: Y ).size());

  // printf ("getEditSubstituteTargetList(AstNodeClass:: X,AstNodeClass:: Y ).size() = %d \n",
  //          getEditSubstituteTargetList(AstNodeClass:: X,AstNodeClass:: Y ).size());

void
AstNodeClass::editSubstitute ( const string& oldString, const string& newString )
   {
  // Add these to storage and use then just before writting out the final strings
  // printf ("AstNodeClass::editSubstitute() oldString = %s   newString = %s \n",oldString,newString);

     EDIT_SUBSTITUTE_MACRO(LOCAL_LIST,INCLUDE_LIST)

  // printf ("getEditSubstituteTargetList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST).size() = %d \n",
  //      getEditSubstituteTargetList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST).size());
   }

void
AstNodeClass::editSubstituteSubTree        ( const string& oldString, const string& newString )
   {
  // printf ("AstNodeClass::editSubstituteSubTree() oldString = %s   newString = %s \n",oldString,newString);
     EDIT_SUBSTITUTE_MACRO(SUBTREE_LIST,INCLUDE_LIST)
   }

void
AstNodeClass::editSubstituteExclude        ( const string& oldString, const string& newString )
   {
     EDIT_SUBSTITUTE_MACRO(LOCAL_LIST,EXCLUDE_LIST)
   }

void
AstNodeClass::editSubstituteExcludeSubTree ( const string& oldString, const string& newString )
   {
     EDIT_SUBSTITUTE_MACRO(SUBTREE_LIST,EXCLUDE_LIST)
   }

void
AstNodeClass::editSubstitute ( const string& oldString, const string& markerString, const string& filename )
   {
     bool pureVirtual = false;
  // char* returnString = "";

  // Call MACRO to read new string
     SETUP_MARKER_STRINGS_MACRO
     string newString = codeString->getRawString();
     editSubstitute (oldString,newString);
   }

void
AstNodeClass::editSubstituteSubTree        ( const string& oldString, const string& markerString, const string& filename )
   {
     bool pureVirtual = false;
  // char* returnString = "";

  // Call MACRO to read new string
     SETUP_MARKER_STRINGS_MACRO
     string newString = codeString->getRawString();
     editSubstitute (oldString,newString);
     
   }

void
AstNodeClass::editSubstituteExclude        ( const string& oldString, const string& markerString, const string& filename )
   {
     bool pureVirtual = false;
  // char* returnString = "";

  // Call MACRO to read new string
     SETUP_MARKER_STRINGS_MACRO
     string newString = codeString->getRawString();
     editSubstitute (oldString,newString);
   }

void
AstNodeClass::editSubstituteExcludeSubTree ( const string& oldString, const string& markerString, const string& filename )
   {
     bool pureVirtual = false;
  // char* returnString = "";

  // Call MACRO to read new string
     SETUP_MARKER_STRINGS_MACRO
     string newString = codeString->getRawString();
     editSubstitute (oldString,newString);
   }


void
AstNodeClass::setAutomaticGenerationOfConstructor  ( bool X )
   {
  // All terminals can have there constructors and destructors 
  // automatically generated.  This function controls the automatic
  // generation of the constructor (one with the declared data input 
  // as paramteres).
     automaticGenerationOfConstructor = X;
   }

void
AstNodeClass::setAutomaticGenerationOfDestructor   ( bool X )
   {
  // See description of setAutomaticGenerationOfConstructor(bool) above
     automaticGenerationOfDestructor = X;
   }

bool
AstNodeClass::generateDestructor () const
   {
  // See description of setAutomaticGenerationOfConstructor(bool) above
  // return automaticGenerationOfDestructor;

     return automaticGenerationOfDestructor;
   }

bool
AstNodeClass::generateConstructor() const
   {
  // See description of setAutomaticGenerationOfConstructor(bool) above
     return automaticGenerationOfConstructor;
   }

void
AstNodeClass::setAutomaticGenerationOfDataAccessFunctions ( bool X )
   {
  // These functions set the construction of access functions for all member data of a AstNodeClass!
  // See description of setAutomaticGenerationOfConstructor(bool) above
     automaticGenerationOfDataAccessFunctions = X;
   }

bool
AstNodeClass::generateDataAccessFunctions() const
   {
  // These functions set the construction of access functions for all member data of a AstNodeClass!
  // See description of setAutomaticGenerationOfConstructor(bool) above
     return automaticGenerationOfDataAccessFunctions;
   }

void
AstNodeClass::setAutomaticGenerationOfCopyFunction ( bool X )
   {
  // These functions set the construction of access functions for all member data of a AstNodeClass!
  // See description of setAutomaticGenerationOfConstructor(bool) above
     automaticGenerationOfCopyFunction = X;
   }

bool
AstNodeClass::generateCopyFunction() const
   {
  // These functions set the construction of access functions for all member data of a AstNodeClass!
  // See description of setAutomaticGenerationOfConstructor(bool) above
     return automaticGenerationOfCopyFunction;
   }

void
AstNodeClass::consistencyCheck() const
   {
     ROSE_ASSERT (associatedGrammar != NULL);

  // local index variable
     int j = 0, k = 0;

  // check the list for valid objects

     for (k=0; k < 2; k++)
          for (j=0; j < 2; j++)
             {
            // int i = 0;
               vector<GrammarString *>::const_iterator it;
               for( it = memberDataPrototypeList[j][k].begin(); 
                    it != memberDataPrototypeList[j][k].end(); it++ ) 
                  {
                    assert( *it != NULL );
                    (*it)->consistencyCheck();
                  }

               for( it = memberFunctionPrototypeList[j][k].begin(); 
                    it != memberFunctionPrototypeList[j][k].end(); it++ ) 
                  {
                    assert( *it != NULL );
                    (*it)->consistencyCheck();
                  }

               for( it = memberFunctionSourceList[j][k].begin(); 
                    it != memberFunctionSourceList[j][k].end(); it++ ) 
                  {
                    assert( *it != NULL );
                    (*it)->consistencyCheck();
                  }
               }

  // check the subclass list for valid objects
     for( vector<AstNodeClass*>::const_iterator terminalIterator = subclasses.begin(); 
          terminalIterator != subclasses.end(); 
          terminalIterator++)
       {
         // printf ("terminalList[%d].getName() = %s \n",i,terminalList[i].getName());
         ROSE_ASSERT((*terminalIterator)!=NULL);
         (*terminalIterator)->consistencyCheck();
         ROSE_ASSERT ((*terminalIterator)->getBaseClass() == this);
        }
   }


void
AstNodeClass::display( const string& label ) const
   {
     printf ("In AstNodeClass::display ( %s ) \n",label.c_str());

     printf ("Name     = %s \n", getName().c_str());
     printf ("Lexeme   = %s \n", getLexeme().c_str());
     printf ("Tag Name = %s \n", getTagName().c_str());

     for( vector<AstNodeClass*>::const_iterator terminalIterator = subclasses.begin(); 
          terminalIterator != subclasses.end(); 
          terminalIterator++)
       {
         ROSE_ASSERT((*terminalIterator)!=NULL);
         string nameInProductionRule = (*terminalIterator)->name;
         printf ("     OR:    nameInProductionRule = %s \n", nameInProductionRule.c_str());
       }

   }

// MK: This function is used to check the lists of GrammarString objects that
// are used during the generation phase of the classes of the AST restructuring tool
void
AstNodeClass::checkListOfGrammarStrings(vector<GrammarString *>& checkList)
{
  // Check list for uniqueness and for elements of length 0
  vector<GrammarString *>::iterator it;
  vector<GrammarString *>::iterator jt;
  for( it = checkList.begin(); it != checkList.end(); it++ )
    {
      jt = it; jt++;
      for( jt = checkList.begin(); jt != checkList.end(); jt++ )
        if(*it==*jt) ROSE_ABORT();
    }      
  return;
}

string
AstNodeClass::buildPointerInMemoryPoolCheck ()
   {
  // DQ & JH (1/17/2006): This function generates the code for each IR node to check all its 
  // non-NULL data members pointing to IR nodes to make sure that each IR node is:
  //   1) Is a valid IR node
  //       a. contains p_freepointer equal to AST_FileIO::IS_VALID_POINTER() value 
  //          indicating that it has been allocated using the IR node's new operator
  //       b. has not be deleted using the IR node's delete operator
  //   2) Is in a block of the correct memory pool (STL vector of allocated memory 
  //      blocks for that IR node)

     vector<GrammarString *> copyList;
     vector<GrammarString *>::iterator stringListIterator;

     string classNameString = this-> name;
     string s;
     for (AstNodeClass *t = this; t != NULL; t = t->getBaseClass())
        {
          copyList        = t->getMemberDataPrototypeList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST);
          for ( stringListIterator = copyList.begin(); stringListIterator != copyList.end(); stringListIterator++ )
             {
               GrammarString *data = *stringListIterator;
               string varNameString = string(data->getVariableNameString());
               string varTypeString = string(data->getTypeNameString());
               int length = varTypeString.size();
               if (varNameString != "freepointer" )
                  {
                    bool typeIsStarPointer = (varTypeString.find("*") != std::string::npos);

                    if ( (varTypeString == "$CLASSNAME *" ) || ( ( ( varTypeString.substr(0,15) == "$GRAMMAR_PREFIX" ) || ( varTypeString.substr(0,2) == "Sg" ) ) && typeIsStarPointer ) )
                       {
                         s += "          if ( p_" + varNameString + " != NULL )\n" ;
                         s += "             { \n" ;
                         s += "                 if ( p_" + varNameString + "->get_freepointer() == AST_FileIO::IS_VALID_POINTER() )\n" ;
                         s += "                    { \n" ;
                         s += "                       if ( p_" + varNameString + "->isInMemoryPool() == false ) \n" ;
                         s += "                         { \n" ;
                         s += "                             std::cout << \"" + classNameString + " :: \";\n";
                         s += "                             std::cout << \" p_" + varNameString + " is not in memory pool of \"; \n";
                         s += "                             std::cout <<    p_" + varNameString + "->class_name() << std::endl;\n" ;
                         s += "                         } \n" ;
                         s += "                    } \n" ;
                         s += "                  else \n" ;
                         s += "                    { \n" ;
                         s += "                       std::cout << \"" + classNameString + " :: \" << std::flush;\n" ;
                         s += "                       std::cout << \"" + varTypeString + " p_" + varNameString + " = \" << p_" + varNameString + " << \" --> \" << std::flush;\n" ;
                         s += "                       std::cout << \" not valid \" << std::endl;\n" ;
                         s += "                    } \n" ;
                         s += "             } \n" ;
                         s += "\n" ;
                       }

                    if (  7 < length && varTypeString.substr( length-7, length) == "PtrList" )
                       {
                         s += "     " + varTypeString + "::iterator i_" + varNameString + " = p_" + varNameString + ".begin() ; \n" ;
                         s += "     for ( ; i_" + varNameString + " != p_" + varNameString + ".end(); ++i_" + varNameString + " ) \n";
                         s += "        {\n";
                         s += "          if ( (*i_" + varNameString + ") != NULL )\n" ;
                         s += "             { \n" ;
                         s += "                 if ( (*i_" + varNameString + ")->get_freepointer() == AST_FileIO::IS_VALID_POINTER() )\n" ;
                         s += "                    { \n" ;
                         s += "                       if ( (*i_" + varNameString + ")->isInMemoryPool() == false ) \n" ;
                         s += "                         { \n" ;
                         s += "                             std::cout << \"" + classNameString + " :: \";\n";
                         s += "                             std::cout << \" p_" + varNameString + " ( list of poitners to IR nodes ), entry is not in memory pool of \"; \n";
                         s += "                             std::cout <<    (*i_" + varNameString + ")->class_name() << std::endl;\n" ;
                         s += "                         } \n" ;
                         s += "                    } \n" ;
                         s += "                  else \n" ;
                         s += "                    { \n" ;
                         s += "                       std::cout << \"" + classNameString + " :: \" << std::flush;\n" ;
                         s += "                       std::cout << \"" + varTypeString + " p_" + varNameString + " --> \" << std::flush;\n" ;
                         s += "                       std::cout << \" entry not valid \" << std::endl;\n" ;
                         s += "                    } \n" ;
                         s += "             } \n" ;
                         s += "          else \n" ;
                         s += "             { \n" ;
                         s += "                 std::cout << \"" + varTypeString + " p_" + varNameString + " --> NULL \" << std::endl;\n" ;
                         s += "             } \n" ;
                         s += "        }\n";
                         s += "\n" ;
                       }

                    if (  9 < length && varTypeString.substr( length-9, length) == "PtrVector" )
                       {
                         s += "     " + varTypeString + "::iterator i_" + varNameString + " = p_" + varNameString + ".begin() ; \n" ;
                         s += "     for ( ; i_" + varNameString + " != p_" + varNameString + ".end(); ++i_" + varNameString + " ) \n";
                         s += "        {\n";
                         s += "          if ( (*i_" + varNameString + ") != NULL )\n" ;
                         s += "             { \n" ;
                         s += "                 if ( (*i_" + varNameString + ")->get_freepointer() == AST_FileIO::IS_VALID_POINTER() )\n" ;
                         s += "                    { \n" ;
                         s += "                       if ( (*i_" + varNameString + ")->isInMemoryPool() == false ) \n" ;
                         s += "                         { \n" ;
                         s += "                             std::cout << \"" + classNameString + " :: \";\n";
                         s += "                             std::cout << \" p_" + varNameString + " ( list of poitners to IR nodes ), entry is not in memory pool of \"; \n";
                         s += "                             std::cout <<    (*i_" + varNameString + ")->class_name() << std::endl;\n" ;
                         s += "                         } \n" ;
                         s += "                    } \n" ;
                         s += "                  else \n" ;
                         s += "                    { \n" ;
                         s += "                       std::cout << \"" + classNameString + " :: \" << std::flush;\n" ;
                         s += "                       std::cout << \"" + varTypeString + " p_" + varNameString + " --> \" << std::flush;\n" ;
                         s += "                       std::cout << \" entry not valid \" << std::endl;\n" ;
                         s += "                    } \n" ;
                         s += "             } \n" ;
                         s += "          else \n" ;
                         s += "             { \n" ;
                         s += "                 std::cout << \"" + varTypeString + " p_" + varNameString + " --> NULL \" << std::endl;\n" ;
                         s += "             } \n" ;
                         s += "        }\n";
                         s += "\n" ;
                       }

                    if (  10 < length && varTypeString.substr( length-10, length) == "PtrListPtr" )
                       {
                         std::string varTypeStringWithoutPtr = varTypeString.substr(0,varTypeString.size()-3) ;
                         s += "     if ( p_" + varNameString + " != NULL )\n" ;
                         s += "        { \n" ;
                         s += "          " + varTypeStringWithoutPtr + "::iterator i_" + varNameString + " = p_" + varNameString + "->begin() ; \n" ;
                         s += "          for ( ; i_" + varNameString + " != p_" + varNameString + "->end(); ++i_" + varNameString + " ) \n";
                         s += "             {\n";
                         s += "               if ( (*i_" + varNameString + ") != NULL )\n" ;
                         s += "                  { \n" ;
                         s += "                      if ( (*i_" + varNameString + ")->get_freepointer() == AST_FileIO::IS_VALID_POINTER() )\n" ;
                         s += "                         { \n" ;
                         s += "                            if ( (*i_" + varNameString + ")->isInMemoryPool() == false ) \n" ;
                         s += "                              { \n" ;
                         s += "                                  std::cout << \"" + classNameString + " :: \";\n";
                         s += "                                  std::cout << \" p_" + varNameString + " ( list of poitners to IR nodes ), entry is not in memory pool of \"; \n";
                         s += "                                  std::cout <<    (*i_" + varNameString + ")->class_name() << std::endl;\n" ;
                         s += "                              } \n" ;
                         s += "                         } \n" ;
                         s += "                       else \n" ;
                         s += "                         { \n" ;
                         s += "                            std::cout << \"" + classNameString + " :: \" << std::flush;\n" ;
                         s += "                            std::cout << \"" + varTypeString + " p_" + varNameString + " --> \" << std::flush;\n" ;
                         s += "                            std::cout << \" entry not valid \" << std::endl;\n" ;
                         s += "                         } \n" ;
                         s += "                  } \n" ;
                         s += "               else \n" ;
                         s += "                  { \n" ;
                         s += "                      std::cout << \"" + varTypeString + " p_" + varNameString + " --> NULL \" << std::endl;\n" ;
                         s += "                  } \n" ;
                         s += "             }\n";
                         s += "        } \n" ;
                         s += "\n" ;
                       }

                    if (  varTypeString == " rose_hash_multimap*" )
                       {
                         s += "     if ( p_" + varNameString + " != NULL )\n" ;
                         s += "        { \n" ;
                         s += "          rose_hash_multimap::iterator it; \n" ;
                         s += "          for (it = p_" + varNameString + "->begin(); it != p_" + varNameString + "->end(); ++it)\n" ;
                         s += "             {\n";
                         s += "               if ( it->second != NULL )\n" ;
                         s += "                  { \n" ;
                         s += "                      if ( it->second->get_freepointer() == AST_FileIO::IS_VALID_POINTER() )\n" ;
                         s += "                         { \n" ;
                         s += "                            if ( it->second->isInMemoryPool() == false ) \n" ;
                         s += "                              { \n" ;
                         s += "                                  std::cout << \"" + classNameString + " :: \";\n";
                         s += "                                  std::cout << \" p_" + varNameString + " ( rose_hash_multimap, second entries (SgSymbol) ), entry is not in memory pool of \"; \n";
                         s += "                                  std::cout <<    it->second->class_name() << std::endl;\n" ;
                         s += "                              } \n" ;
                         s += "                         } \n" ;
                         s += "                       else \n" ;
                         s += "                         { \n" ;
                         s += "                            std::cout << \"" + classNameString + " :: \" << std::flush;\n" ;
                         s += "                            std::cout << \"" + varTypeString + " p_" + varNameString + " --> \" << std::flush;\n" ;
                         s += "                            std::cout << \" entry not valid \" << std::endl;\n" ;
                         s += "                         } \n" ;
                         s += "                  } \n" ;
                         s += "               else \n" ;
                         s += "                  { \n" ;
                         s += "                      std::cout << \"" + varTypeString + " p_" + varNameString + " --> NULL \" << std::endl;\n" ;
                         s += "                  } \n" ;
                         s += "             }\n";
                         s += "        } \n" ;
                         s += "\n" ;
                       }
                  }
             }
        }

     return s;
   }


/*************************************************************************************************
*  The function
*       AstNodeClass::buildListIteratorString()
*  supports buildReturnDataMemberPointers() by building the string representing the code 
*  necessary to return all data member pointers to IR nodes contained in STL lists.
*************************************************************************************************/
std::string
AstNodeClass::buildListIteratorString( string typeName, string variableName, string classNameString)
   {
        // AS(2/14/2006) Builds the strings for the list of data member pointers.
        string returnString;

        // Control variables for code generation
           bool typeIsPointerToListOfPointers    = typeName.find("PtrListPtr") != string::npos;
           bool typeIsPointerToListOfNonpointers = (typeIsPointerToListOfPointers == false) && typeName.find("ListPtr") != string::npos;
           bool typeIsPointerToList              = typeIsPointerToListOfPointers || typeIsPointerToListOfNonpointers;

        // By "simple list" we mean NOT a pointer to a list (just a list, e.g. STL list)
           bool typeIsSimpleListOfPointers       = (typeIsPointerToListOfPointers == false) && typeName.find("PtrList") != string::npos;
           bool typeIsList                       = typeIsPointerToList || typeIsSimpleListOfPointers;
           bool typeIsSgNode                     = typeName.find('*') != string::npos;

        // One of these should be true!
           if(typeIsList!=true)
             return "";

           if ((typeIsList && typeIsSgNode) || (!typeIsList && !typeIsSgNode)) {
               cerr << (typeIsList ? "cannot be both" : "must be either")
                    <<" a list and a Sage node: "
                    <<typeName <<" " <<classNameString <<"::" <<variableName <<endl;
           }
           ROSE_ASSERT(typeIsList == true  || typeIsSgNode == true);
           ROSE_ASSERT(typeIsList == false || typeIsSgNode == false);

           string listElementType = "default-error-type";


           if (typeIsList == true)
              {

             // name constant for all cases below (in this scope)
                string listElementName       = "source_list_element";

             // names that are set differently for different cases
                string iteratorBaseType;
                string needPointer;
                string originalList;
                string iteratorName;

             // Access member functions using "->" or "." (set to some string
             // that will cause an error if used, instead of empty string).
                string accessOperator = "error string for access operator";

                if (typeIsPointerToList == true)
                   {
                     if (typeIsPointerToListOfPointers == true)
                        {
                          needPointer = "*";
                          accessOperator = "->";
                        }
                       else
                        {
                          ROSE_ASSERT(typeIsPointerToListOfNonpointers == true);
                          accessOperator = ".";
                        }

                  // iteratorBaseType = string("NeedBaseType_of_") + typeName;
                     int positionOfListPtrSubstring = typeName.find("ListPtr");
                     int positionOfPtrSubstring     = typeName.find("Ptr",positionOfListPtrSubstring);
                     iteratorBaseType = typeName.substr(0,positionOfPtrSubstring);

                  // copyOfList = variableName + "_source";
                     //originalList = string("get_") + variableName + "()";
                     originalList = string("p_") +variableName;

                     iteratorName = variableName + "_iterator";

                   }
                  else
                   {

                     ROSE_ASSERT(typeIsSimpleListOfPointers == true);
                     iteratorBaseType = typeName;
                     needPointer = "*";
                     accessOperator = ".";

                  // Need to generate different code, for example:
                  //      SgStatementPtrList::const_iterator cpinit_stmt = get_init_stmt().begin();
                  // instead of:
                  //      SgStatementPtrList::const_iterator init_stmt_copy_iterator = init_stmt_copy.begin();

                     //originalList = string("get_") + variableName + "()";
                     originalList = string("p_") + variableName;

                     iteratorName = string("source_") + variableName + "_iterator";
                   }

             // Need to get the prefix substring to strings like "SgFilePtrList" (i.e. "SgFile")
                int positionOfPtrListSubstring = iteratorBaseType.find("PtrList");
                int positionOfListSubstring    = iteratorBaseType.find("Ptr",positionOfPtrListSubstring);
                listElementType = typeName.substr(0,positionOfListSubstring) + needPointer;

             // Open up the loop over the list elements
                if(accessOperator=="->"){
                    returnString += "     if(" + originalList + "==NULL)\n";
                    //Return a NULL pointer so that th graph shows that this pointer equals NULL
                    returnString += "        returnVector.push_back(pair<SgNode*,std::string>( NULL,\""+variableName+"\"));\n";

                    returnString += "     else\n";
                }
                returnString += "     for ( " +iteratorBaseType + "::const_iterator " + iteratorName + " = " + originalList + accessOperator + "begin() \n"
                         + "; " + iteratorName
                         + " != " + originalList + accessOperator + "end(); ++" 
                         + iteratorName + ") \n        { \n";

             // Declare the a loop variable (reference to current element of list)
                returnString += "          returnVector.push_back(pair<SgNode*,std::string>( *" + iteratorName + ",\""+variableName+"\"));\n";
                
            // close off the loop
                returnString += "        } \n";

              }
      return returnString;
};

//AS (021406)
/*************************************************************************************************
*  The function
*       AstNodeClass::buildReturnDataMemberPointers()
*  builds the code for returning all data member pointers to IR nodes in the AST.
*************************************************************************************************/
string
AstNodeClass::buildReturnDataMemberPointers ()
   {
  // AS (2/14/2006): This function generates the code for each IR node to return
  // a pair of all data members pointing to IR nodes and their name to:
  //   1) Graph the whole AST
  //   2) Create a general mechanism to do the mechanism mentioned above
  //
     vector<GrammarString *> copyList;
     vector<GrammarString *>::iterator stringListIterator;

     string classNameString = this-> name;
     string s("std::vector<std::pair<SgNode*,std::string> > returnVector;\n") ;
  // s += "   std::cout << \"------------ checking pointers of " + classNameString + "  -------------------\" << std::endl;\n" ;
  // s += "   ROSE_ASSERT ( pointer->p_freepointer == AST_FileIO::IS_VALID_POINTER() );\n";
  // AS Iterate over the AstNodeClass and its parents (base-classes).
  // PS! Everything is treated as terminals; even non-terminals
  // printf ("Derived Class AstNodeClass name = %s \n",name);

     for (AstNodeClass *t = this; t != NULL; t = t->getBaseClass())
        {
          copyList        = t->getMemberDataPrototypeList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST);
       // printf ("Possible base class AstNodeClass name = %s \n",(*t).name);
       // AS Iterate over data memeber of (non-) AstNodeClass class we are looking at 
          for ( stringListIterator = copyList.begin(); stringListIterator != copyList.end(); stringListIterator++ )
             {
               //AS ???
               GrammarString *data = *stringListIterator;
               //AS Name of the actual data member. All the data members in rosetta is called p_<name>, but our name string only
               //contains <name>
               string varNameString = string(data->getVariableNameString());
               //AS A type name is always a type name, but if it is 'const' or 'static' those will we in front of the typename
               //And there are some typedef-types
               string varTypeString = string(data->getTypeNameString());
               //AS Freepointer is a specific data member that contains no AST information, so lets skip that

              if (varNameString != "freepointer" )
                  {
                    
                    //AS Analyse the types to see if it contains a '*' because then it is a pointer
                    bool typeIsStarPointer = ( varTypeString.find("*") != std::string::npos) ;
                    //AS Check to see if it this is an pointer to any IR-node. The varTypeString == "$CLASSNAME *" checks to see if it
                    //is a ir-node pointer which is *not* yet replaced. $CLASSNAME is inside the string and will later be replaced with 
                    //e.g SgTypeInt etc. 'varTypeString.substr(0,15) == "$GRAMMAR_PREFIX"' checks to see if it is part of the grammar.
                    //'varTypeString.substr(0,2) == "Sg" ' and to see if it is a Sg node of some type.
                    s += buildListIteratorString(varTypeString, varNameString,classNameString);

                    if ( (varTypeString == "$CLASSNAME *" ) || ( ( ( varTypeString.substr(0,15) == "$GRAMMAR_PREFIX" ) || ( varTypeString.substr(0,2) == "Sg" ) ) && typeIsStarPointer ) )
                       {
                         //AS Checks to see if the pointer is a data member. Because the mechanism for generating access to variables
                         //is the same as the one accessing access member functions. We do not want the last case to show up here.
                         s += "          returnVector.push_back(pair<SgNode*,std::string>( p_" + varNameString + ",\""+varNameString+"\"));\n";
                    
                       }
                      else
                       {
                         if( varTypeString.find("rose_hash_multimap") != std::string::npos )
                            {
                           // AS(02/24/06) Generate code for returning data member pointers to IR
                              std::string accessOperator = ".";
                              if ( varTypeString.find("rose_hash_multimap*") != std::string::npos )
                                 {
                                   accessOperator = "->";
                                   s += "     if(p_" + varNameString + "==NULL)\n";
                                // Return a NULL pointer so that th graph shows that this pointer equals NULL
                                   s += "        returnVector.push_back(pair<SgNode*,std::string>( NULL,\""+varNameString+"\"));\n";
                                   s += "     else\n";
                                 }

                              s +=  "     for ( rose_hash_multimap::const_iterator it_"+varNameString+ "= p_" + varNameString + accessOperator + "begin();"
                                +   "it_" + varNameString
                                +   "!= p_" + varNameString + accessOperator+ "end(); ++"
                                +   "it_"+varNameString+") \n        { \n";
                           // Declare the a loop variable (reference to current element of list)
                              s += "          returnVector.push_back(pair<SgNode*,std::string>( it_" + varNameString + "->second,std::string(it_"+varNameString+"->first.str()) ) );\n";

                           // close off the loop
                              s += "        } \n";
                            }
                       }
                  }
             }
        }

     s +="     return returnVector;\n";

     return s;
   }

/*************************************************************************************************
*  The function
*       AstNodeClass::buildListIteratorStringForReferenceToPointers()
*  supports buildProcessDataMemberReferenceToPointers() by building the string representing the code 
*  necessary to process references (pointers) all data member pointers to IR nodes contained in STL lists.
*  The traverse parameter indicates whether the property is normally traversed.  This parameter is
*  propagated through to the ReferenceToPointerHandler.
*************************************************************************************************/
// DQ (9/28/2022): Fixing compiler warning for argument not used.
// string AstNodeClass::buildListIteratorStringForReferenceToPointers(string typeName, string variableName, string classNameString, bool traverse)
string AstNodeClass::buildListIteratorStringForReferenceToPointers(string typeName, string variableName, bool traverse)
   {
  // AS(2/14/2006) Builds the strings for the list of data member pointers.
     string returnString;

  // Control variables for code generation
     bool typeIsPointerToListOfPointers    = typeName.find("PtrListPtr") != string::npos;
     bool typeIsPointerToListOfNonpointers = (typeIsPointerToListOfPointers == false) && typeName.find("ListPtr") != string::npos;
     bool typeIsPointerToList              = typeIsPointerToListOfPointers || typeIsPointerToListOfNonpointers;

  // By "simple list" we mean NOT a pointer to a list (just a list, e.g. STL list)
     bool typeIsSimpleListOfPointers       = (typeIsPointerToListOfPointers == false) && typeName.find("PtrList") != string::npos;
     bool typeIsList                       = typeIsPointerToList || typeIsSimpleListOfPointers;
#ifndef NDEBUG
     bool typeIsSgNode                     = typeName.find('*') != string::npos;
#endif
            
  // One of these should be true!
     if(typeIsList!=true)
          return "";

     ROSE_ASSERT(typeIsList == true  || typeIsSgNode == true);
     ROSE_ASSERT(typeIsList == false || typeIsSgNode == false);

     string listElementType = "default-error-type";


     if (typeIsList == true)
        {
       // name constant for all cases below (in this scope)
          string listElementName       = "source_list_element";

       // names that are set differently for different cases
          string iteratorBaseType;
          string needPointer;
          string originalList;
          string iteratorName;

       // Access member functions using "->" or "." (set to some string
       // that will cause an error if used, instead of empty string).
          string accessOperator = "error string for access operator";

          if (typeIsPointerToList == true)
             {
               if (typeIsPointerToListOfPointers == true)
                  {
                    needPointer = "*";
                    accessOperator = "->";
                  }
                 else
                  {
                    ROSE_ASSERT(typeIsPointerToListOfNonpointers == true);
                    accessOperator = ".";
                  }

            // iteratorBaseType = string("NeedBaseType_of_") + typeName;
               int positionOfListPtrSubstring = typeName.find("ListPtr");
               int positionOfPtrSubstring     = typeName.find("Ptr",positionOfListPtrSubstring);
               iteratorBaseType = typeName.substr(0,positionOfPtrSubstring);

            // copyOfList = variableName + "_source";
            // originalList = string("get_") + variableName + "()";
               originalList = string("p_") +variableName;

               iteratorName = variableName + "_iterator";

             }
            else
             {
               ROSE_ASSERT(typeIsSimpleListOfPointers == true);
               iteratorBaseType = typeName;
               needPointer = "*";
               accessOperator = ".";

            // Need to generate different code, for example:
            //      SgStatementPtrList::const_iterator cpinit_stmt = get_init_stmt().begin();
            // instead of:
            //      SgStatementPtrList::const_iterator init_stmt_copy_iterator = init_stmt_copy.begin();

            // originalList = string("get_") + variableName + "()";
               originalList = string("p_") + variableName;

               iteratorName = string("source_") + variableName + "_iterator";
             }

       // Need to get the prefix substring to strings like "SgFilePtrList" (i.e. "SgFile")
          int positionOfPtrListSubstring = iteratorBaseType.find("PtrList");
          int positionOfListSubstring    = iteratorBaseType.find("Ptr",positionOfPtrListSubstring);
          listElementType = typeName.substr(0,positionOfListSubstring) + needPointer;

       // Open up the loop over the list elements
          if (accessOperator=="->")
             {
               returnString += "     if (" + originalList + "== NULL) {}\n";
               returnString += "     else\n";
             }

          returnString += "     for ( " +iteratorBaseType + "::iterator " + iteratorName + " = " + originalList + accessOperator + "begin() \n"
                       + "; " + iteratorName
                       + " != " + originalList + accessOperator + "end(); ++" 
                       + iteratorName + ") \n        { \n";

       // Declare the a loop variable (reference to current element of list)
          returnString += "          handler->apply(*" + iteratorName + ",SgName(\""+variableName+"\"), " + BOOL2STR(traverse) + ");\n";
                
       // close off the loop
          returnString += "        } \n";

        }
     return returnString;
   }

//DQ (4/30/2006): This function is similar to buildReturnDataMemberPointers but returns reference to the pointers.
/*************************************************************************************************
*  The function
*       AstNodeClass::buildProcessDataMemberReferenceToPointers()
*  builds the code for processing references to all data member pointers to IR nodes in the AST.
*************************************************************************************************/
string
AstNodeClass::buildProcessDataMemberReferenceToPointers ()
   {
  // DQ (4/30/2006): This is a modified version of the code for buildReturnDataMemberPointers()

  // AS (2/14/2006): This function generates the code for each IR node to return
  // a pair of all data members pointing to IR nodes and their name to:
  //   1) Graph the whole AST
  //   2) Create a general mechanism to do the mechanism mentioned above
  //
     vector<GrammarString *> copyList;
     vector<GrammarString *>::iterator stringListIterator;

     string classNameString = this-> name;
     string s ;
  // s += "   std::cout << \"------------ checking pointers of " + classNameString + "  -------------------\" << std::endl;\n" ;
  // s += "   ROSE_ASSERT ( pointer->p_freepointer == AST_FileIO::IS_VALID_POINTER() );\n";
  // AS Iterate over the AstNodeClass and its parents (base-classes).
  // PS! Everything is treated as terminals; even non-terminals
  // printf ("Derived Class AstNodeClass name = %s \n",name);

     for (AstNodeClass *t = this; t != NULL; t = t->getBaseClass())
        {
          copyList        = t->getMemberDataPrototypeList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST);
       // printf ("Possible base class AstNodeClass name = %s \n",(*t).name);
       // AS Iterate over data memeber of (non-) AstNodeClass class we are looking at 
          for ( stringListIterator = copyList.begin(); stringListIterator != copyList.end(); stringListIterator++ )
             {
            // AS ???
               GrammarString *data = *stringListIterator;
            // AS Name of the actual data member. All the data members in rosetta is called p_<name>, but our name string only
            // contains <name>
               string varNameString = string(data->getVariableNameString());
            // AS A type name is always a type name, but if it is 'const' or 'static' those will we in front of the typename
            // And there are some typedef-types
               string varTypeString = string(data->getTypeNameString());

            // AS Freepointer is a specific data member that contains no AST information, so lets skip that
              if (varNameString != "freepointer" )
                  {
                 // AS Analyse the types to see if it contains a '*' because then it is a pointer
                    bool typeIsStarPointer = ( varTypeString.find("*") != std::string::npos) ;

                 // PC Check whether this member should be traversed.
                    bool traverse = data->getToBeTraversed() == DEF_TRAVERSAL;

                 // AS Check to see if it this is an pointer to any IR-node. The varTypeString == "$CLASSNAME *" checks to see if it
                 // is a ir-node pointer which is *not* yet replaced. $CLASSNAME is inside the string and will later be replaced with 
                 // e.g SgTypeInt etc. 'varTypeString.substr(0,15) == "$GRAMMAR_PREFIX"' checks to see if it is part of the grammar.
                 // 'varTypeString.substr(0,2) == "Sg" ' and to see if it is a Sg node of some type.

                 // DQ (9/28/2022): Fixing compiler warning for argument not used.
                 // s += buildListIteratorStringForReferenceToPointers(varTypeString, varNameString,classNameString, traverse);
                    s += buildListIteratorStringForReferenceToPointers(varTypeString, varNameString, traverse);

                    if ( (varTypeString == "$CLASSNAME *" ) || ( ( ( varTypeString.substr(0,15) == "$GRAMMAR_PREFIX" ) || ( varTypeString.substr(0,2) == "Sg" ) ) && typeIsStarPointer ) )
                       {
                      // DQ (9/8/2016): Added assertion as part of debugging C++11 test2016_49.C
                      // s += "     ROSE_ASSERT(p_" + varNameString + " != NULL);\n";
                      // s += "     printf (\"ROSETTA varNameString generated debugging: p_" + varNameString + " = %p \\n\",p_" + varNameString + ");\n";

                      // AS Checks to see if the pointer is a data member. Because the mechanism for generating access to variables
                      // is the same as the one accessing access member functions. We do not want the last case to show up here.
                         s += "     handler->apply(p_" + varNameString + ",SgName(\""+varNameString+"\"), " + BOOL2STR(traverse) + ");\n";
                       }
                      else
                       {
                         if( varTypeString.find("rose_hash_multimap") != std::string::npos )
                            {
                           // AS(02/24/06) Generate code for returning data member pointers to IR
                              std::string accessOperator = ".";
                              accessOperator = "->";
                              s += "     if (p_" + varNameString + " == NULL)\n";
                              s += "        {\n";
                              s += "        }\n";
                              s += "     else\n";
                              s += "        {\n";
                              s += "          for ( rose_hash_multimap::iterator it_"+varNameString+ "= p_" + varNameString + accessOperator + "begin(); it_" + varNameString + "!= p_" + varNameString + accessOperator + "end(); ++" + "it_" + varNameString + ")\n";
                              s += "             {\n";
                           // Declare the a loop variable (reference to current element of list)
                              s += "               handler->apply(it_" + varNameString + "->second, it_" + varNameString + "->first, " + BOOL2STR(traverse) + ");\n";
                           // close off the loop
                              s += "             }\n";
                              s += "        }\n";
                            }
                       }
                  }
             }
        }

     return s;
   }


// DQ (3/7/2007): This is support for buildChildIndex() (see below)
/*************************************************************************************************
*  The function
*       AstNodeClass::buildListIteratorStringForChildIndex()
*  supports buildChildIndex() by building the string representing the code 
*  necessary to count the index of all data member pointers to IR nodes 
*  contained in STL lists.
*************************************************************************************************/
// DQ (9/28/2022): Fixing compiler warning for argument not used.
// std::string AstNodeClass::buildListIteratorStringForChildIndex(string typeName, string variableName, string classNameString)
std::string AstNodeClass::buildListIteratorStringForChildIndex(string typeName, string variableName)
   {
  // AS(2/14/2006) Builds the strings for the list of data member pointers.
     string returnString;

  // Control variables for code generation
     bool typeIsPointerToListOfPointers    = typeName.find("PtrListPtr") != string::npos;
     bool typeIsPointerToListOfNonpointers = (typeIsPointerToListOfPointers == false) && typeName.find("ListPtr") != string::npos;
     bool typeIsPointerToList              = typeIsPointerToListOfPointers || typeIsPointerToListOfNonpointers;

  // By "simple list" we mean NOT a pointer to a list (just a list, e.g. STL list)
     bool typeIsSimpleListOfPointers       = (typeIsPointerToListOfPointers == false) && typeName.find("PtrList") != string::npos;
     bool typeIsList                       = typeIsPointerToList || typeIsSimpleListOfPointers;
#ifndef NDEBUG
     bool typeIsSgNode                     = typeName.find('*') != string::npos;
#endif
            
  // One of these should be true!
     if (typeIsList != true)
          return "";


     ROSE_ASSERT(typeIsList == true  || typeIsSgNode == true);
     ROSE_ASSERT(typeIsList == false || typeIsSgNode == false);

     string listElementType = "default-error-type";


     if (typeIsList == true)
        {
       // name constant for all cases below (in this scope)
          string listElementName       = "source_list_element";

       // names that are set differently for different cases
          string iteratorBaseType;
          string needPointer;
          string originalList;
          string iteratorName;

       // Access member functions using "->" or "." (set to some string
       // that will cause an error if used, instead of empty string).
          string accessOperator = "error string for access operator";

          if (typeIsPointerToList == true)
             {
               if (typeIsPointerToListOfPointers == true)
                  {
                    needPointer = "*";
                    accessOperator = "->";
                  }
                 else
                  {
                    ROSE_ASSERT(typeIsPointerToListOfNonpointers == true);
                    accessOperator = ".";
                  }

            // iteratorBaseType = string("NeedBaseType_of_") + typeName;
               int positionOfListPtrSubstring = typeName.find("ListPtr");
               int positionOfPtrSubstring     = typeName.find("Ptr",positionOfListPtrSubstring);
               iteratorBaseType = typeName.substr(0,positionOfPtrSubstring);

            // copyOfList = variableName + "_source";
            // originalList = string("get_") + variableName + "()";
               originalList = string("p_") +variableName;

               iteratorName = variableName + "_iterator";

             }
            else
             {
               ROSE_ASSERT(typeIsSimpleListOfPointers == true);
               iteratorBaseType = typeName;
               needPointer = "*";
               accessOperator = ".";

            // Need to generate different code, for example:
            //      SgStatementPtrList::const_iterator cpinit_stmt = get_init_stmt().begin();
            // instead of:
            //      SgStatementPtrList::const_iterator init_stmt_copy_iterator = init_stmt_copy.begin();

            // originalList = string("get_") + variableName + "()";
               originalList = string("p_") + variableName;

               iteratorName = string("source_") + variableName + "_iterator";
             }

       // Need to get the prefix substring to strings like "SgFilePtrList" (i.e. "SgFile")
          int positionOfPtrListSubstring = iteratorBaseType.find("PtrList");
          int positionOfListSubstring    = iteratorBaseType.find("Ptr",positionOfPtrListSubstring);
          listElementType = typeName.substr(0,positionOfListSubstring) + needPointer;

       // Open up the loop over the list elements
          if (accessOperator=="->")
             {
               returnString += "     if ( " + originalList + " == NULL )\n";

            // Return a NULL pointer so that th graph shows that this pointer equals NULL
            // returnString += "          returnVector.push_back(pair<SgNode*,std::string>( NULL,\""+variableName+"\"));\n";
               returnString += "        { /* do nothing because this is not an IR node */ } \n";
               returnString += "       else\n";
             }

          returnString += "          for ( " +iteratorBaseType + "::const_iterator " + iteratorName + " = " + originalList + accessOperator + "begin(); " + iteratorName
                       + " != " + originalList + accessOperator + "end(); ++" + iteratorName + ") \n             { \n";

       // Declare the a loop variable (reference to current element of list)
       // returnString += "          returnVector.push_back(pair<SgNode*,std::string>( *" + iteratorName + ",\""+variableName+"\"));\n";
       // returnString += "               if ( *" + iteratorName + " == childNode ) { returnValue = indexCounter; } indexCounter++;\n";
          returnString += "               if ( *" + iteratorName + " == childNode ) { return indexCounter; } indexCounter++;\n";

       // close off the loop
          returnString += "             } \n";
        }

     return returnString;
   }

// DQ (3/7/2007): This is support for buildChildIndex() (see below)
/*************************************************************************************************
*  The function
*       AstNodeClass::buildChildIndex()
*  builds the code for returning all data member pointers to IR nodes in the AST.
*************************************************************************************************/
string
AstNodeClass::buildChildIndex()
   {
  // AS (2/14/2006): This function generates the code for each IR node to return
  // a signed integer of the index associated with the child in the IR node. A negative
  // value indicates that the IR node is not a child.  Uses include:
  //   1) Generating unique names for children (e.g. declarations in 
  //      scopes where overloaded template functions in a class don't 
  //      have the parameter list information availalbe so we need to 
  //      prevent over sharing.
  //   2) Forms a lower level implementation of "isChild(SgNode*)" function.
  //   3) Useful in Jeremiah's control flow graph work.

     vector<GrammarString *> copyList;
     vector<GrammarString *>::iterator stringListIterator;

     string classNameString = this-> name;
  // string s = "int indexCounter = 0, returnValue = -1;\n";
     string s = "int indexCounter = 0;\n";

     for (AstNodeClass *t = this; t != NULL; t = t->getBaseClass())
        {
          copyList        = t->getMemberDataPrototypeList(AstNodeClass::LOCAL_LIST,AstNodeClass::INCLUDE_LIST);
       // printf ("Possible base class AstNodeClass name = %s \n",(*t).name);
       // AS Iterate over data memeber of (non-) AstNodeClass class we are looking at 
          for ( stringListIterator = copyList.begin(); stringListIterator != copyList.end(); stringListIterator++ )
             {
               GrammarString *data = *stringListIterator;
            // AS Name of the actual data member. All the data members in rosetta is called p_<name>, but our name string only
            // contains <name>
               string varNameString = string(data->getVariableNameString());
            // AS A type name is always a type name, but if it is 'const' or 'static' those will we in front of the typename
            // And there are some typedef-types
               string varTypeString = string(data->getTypeNameString());
            // AS Freepointer is a specific data member that contains no AST information, so lets skip that

              if (varNameString != "freepointer" )
                  {
                 // AS Analyse the types to see if it contains a '*' because then it is a pointer
                    bool typeIsStarPointer = ( varTypeString.find("*") != std::string::npos) ;
                 // AS Check to see if it this is an pointer to any IR-node. The varTypeString == "$CLASSNAME *" checks to see if it
                 // is a ir-node pointer which is *not* yet replaced. $CLASSNAME is inside the string and will later be replaced with 
                 // e.g SgTypeInt etc. 'varTypeString.substr(0,15) == "$GRAMMAR_PREFIX"' checks to see if it is part of the grammar.
                 // 'varTypeString.substr(0,2) == "Sg" ' and to see if it is a Sg node of some type.

                 // DQ (9/28/2022): Fixing compiler warning for argument not used.
                 // s += buildListIteratorStringForChildIndex(varTypeString, varNameString,classNameString);
                    s += buildListIteratorStringForChildIndex(varTypeString, varNameString);

                    if ( (varTypeString == "$CLASSNAME *" ) || ( ( ( varTypeString.substr(0,15) == "$GRAMMAR_PREFIX" ) || ( varTypeString.substr(0,2) == "Sg" ) ) && typeIsStarPointer ) )
                       {
                         //AS Checks to see if the pointer is a data member. Because the mechanism for generating access to variables
                         //is the same as the one accessing access member functions. We do not want the last case to show up here.
                         // s += "     if ( p_" + varNameString + " == childNode ) { returnValue = indexCounter; } indexCounter++;\n";
                         s += "     if ( p_" + varNameString + " == childNode ) { return indexCounter; } indexCounter++;\n";
                       }
                      else
                       {
                         if( varTypeString.find("rose_hash_multimap") != std::string::npos )
                            {
                           // AS(02/24/06) Generate code for returning data member pointers to IR
                              std::string accessOperator = ".";
                              if ( varTypeString.find("rose_hash_multimap*") != std::string::npos )
                                 {
                                   accessOperator = "->";
                                   s += "     if ( p_" + varNameString + " == NULL )\n";
                                // Return a NULL pointer so that th graph shows that this pointer equals NULL
                                   s += "          indexCounter++;\n";
                                   s += "       else\n";
                                 }

                              s += "          for ( rose_hash_multimap::const_iterator it_"+varNameString+ "= p_" + varNameString + accessOperator + "begin(); it_" + varNameString + " != p_" + varNameString + accessOperator+ "end(); ++it_"+varNameString+") \n";
                              s += "             {\n";
                           // Declare the a loop variable (reference to current element of list)
                           // s += "               if ( it_" + varNameString + "->second == childNode ) { returnValue = indexCounter; } indexCounter++;\n";
                              s += "               if ( it_" + varNameString + "->second == childNode ) { return indexCounter; } indexCounter++;\n";
                           // close off the loop
                              s += "             }\n";
                            }
                       }
                  }
             }
        }

     s += "  // Child not found, return -1 (default value for returnValue) as index position to signal this.\n";
  // s += "     return returnValue;";
     s += "     return -1;";

     return s;
   }


  // DQ (10/12/2014): output the name assocauted with the TypeEvaluation enum values.
string
AstNodeClass::typeEvaluationName ( TypeEvaluation x )
   {
     string s;

     switch (x)
        {
          case CHAR_POINTER:                               s = "CHAR_POINTER"; break;
          case CONST_CHAR_POINTER:                         s = "CHAR_POINTER"; break;
          case ATTACHEDPREPROCESSINGINFOTYPE:              s = "ATTACHEDPREPROCESSINGINFOTYPE"; break;
          case ROSE_HASH_MULTIMAP:                         s = "ROSE_HASH_MULTIMAP";       break;
          case ROSE_GRAPH_HASH_MULTIMAP:                   s = "ROSE_GRAPH_HASH_MULTIMAP"; break;
          case ROSE_GRAPH_DIRECTED_EDGE_HASH_MULTIMAP:     s = "ROSE_GRAPH_DIRECTED_EDGE_HASH_MULTIMAP";   break;
          case ROSE_GRAPH_UNDIRECTED_EDGE_HASH_MULTIMAP:   s = "ROSE_GRAPH_UNDIRECTED_EDGE_HASH_MULTIMAP"; break;
          case ROSE_GRAPH_NODE_EDGE_HASH_MULTIMAP:         s = "ROSE_GRAPH_NODE_EDGE_HASH_MULTIMAP"; break;
          case ROSE_GRAPH_INTEGER_NODE_HASH_MAP:           s = "ROSE_GRAPH_INTEGER_NODE_HASH_MAP";   break;
          case ROSE_GRAPH_INTEGER_EDGE_HASH_MAP:           s = "ROSE_GRAPH_INTEGER_EDGE_HASH_MAP";   break;
          case ROSE_GRAPH_STRING_INTEGER_HASH_MULTIMAP:    s = "ROSE_GRAPH_STRING_INTEGER_HASH_MULTIMAP";    break;
          case ROSE_GRAPH_INTEGER_PAIR_EDGE_HASH_MULTIMAP: s = "ROSE_GRAPH_INTEGER_PAIR_EDGE_HASH_MULTIMAP"; break;
          case ROSE_GRAPH_INTEGER_EDGE_HASH_MULTIMAP:      s = "ROSE_GRAPH_INTEGER_EDGE_HASH_MULTIMAP";      break;
          case SGCLASS_POINTER:                            s = "SGCLASS_POINTER";             break;
          case ROSEATTRUBUTESLISTCONTAINER:                s = "ROSEATTRUBUTESLISTCONTAINER"; break;
          case SGCLASS_POINTER_LIST:                       s = "SGCLASS_POINTER_LIST";   break;
          case SGCLASS_POINTER_VECTOR:                     s = "SGCLASS_POINTER_VECTOR"; break;
          case SGCLASS_POINTER_VECTOR_NAMED_LIST:          s = "SGCLASS_POINTER_VECTOR_NAMED_LIST"; break;
          case STL_CONTAINER:                              s = "STL_CONTAINER"; break;
          case STL_SET:                                    s = "STL_SET";       break;
       // DQ (4/30/2009): Added case of STL_MULTIMAP
          case STL_MULTIMAP:                               s = "STL_MULTIMAP";  break;
          case STL_MAP:                                    s = "STL_MAP";       break;
          case STRING:                                     s = "STRING";        break;
          case SGNAME:                                     s = "SGNAME";        break;
          case BIT_VECTOR:                                 s = "BIT_VECTOR";    break;
          case MODIFIERCLASS:                              s = "MODIFIERCLASS"; break;
          case MODIFIERCLASS_WITHOUTEASYSTORAGE:           s = "MODIFIERCLASS_WITHOUTEASYSTORAGE"; break;
          case ASTATTRIBUTEMECHANISM:                      s = "ASTATTRIBUTEMECHANISM"; break;
          case TO_HANDLE:                                  s = "TO_HANDLE";       break;
          case OSTREAM:                                    s = "OSTREAM";         break;
          case ENUM_TYPE:                                  s = "ENUM_TYPE";       break;
          case BASIC_DATA_TYPE:                            s = "BASIC_DATA_TYPE"; break;
          case SKIP_TYPE:                                  s = "SKIP_TYPE";       break;
       // should be no longer necessary after the implementation is changed to STL lists instead of pointers to lists
          case SGCLASS_POINTER_LIST_POINTER:               s = "SGCLASS_POINTER_LIST_POINTER"; break;

          default:
             {
               printf ("Default reached in switch: x = %d \n",(int)x);
               ROSE_ABORT();
             }
        }

     return s;
   }

void AstNodeClass::setGenerateEssentialDataMembersConstructorImplementation(bool flag) {
  generateEssentialDataMembersConstructorImplementation=flag;
}

void AstNodeClass::setGenerateEnforcedDefaultConstructorImplementation(bool flag) {
  generateEnforcedDefaultConstructorImplementation=flag;
}

bool AstNodeClass::getGenerateEssentialDataMembersConstructorImplementation() {
  return generateEssentialDataMembersConstructorImplementation;
}

bool AstNodeClass::getGenerateEnforcedDefaultConstructorImplementation() {
  return generateEnforcedDefaultConstructorImplementation;
}
