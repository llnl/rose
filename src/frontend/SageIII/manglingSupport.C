
// #include <cctype>
// #include <sstream>

#include "sage3basic.h"

using namespace std;
using namespace Rose;

// DQ (10/31/2015): Need to define this in a single location instead of in the header file included by multiple source files.
MangledNameSupport::setType MangledNameSupport::visitedTemplateDefinitions;


string
replaceNonAlphaNum (const string& s)
{
  ostringstream s_new;
  for (string::size_type i = 0; i < s.size (); ++i)
    {
      char s_i = s[i];
      if (!isalnum (s_i))
        s_new << '_' << (int)s_i;
      else
        s_new << s_i;
    }
  return s_new.str ();
}

string
trimSpaces (const string& s)
  {
    if (s.empty ())
      return s;

    string s_new (s);

    // strip trailing spaces
    string::size_type first_space = s_new.size ();
    ROSE_ASSERT (first_space >= 1);
    if (isspace (s_new[first_space-1]))
      {
        do
          first_space--;
        while (first_space > 0 && isspace (s_new[first_space-1]));
        ROSE_ASSERT (isspace (s_new[first_space]));
        s_new.erase (first_space); // erase to end of string
      }

    // strip leading spaces
    string::size_type first_nonspace = 0;
    while (first_nonspace < s_new.size () && isspace (s_new[first_nonspace]))
      first_nonspace++;
    if (first_nonspace)
      s_new.erase (0, first_nonspace);

    return s_new;
  }

#if 0
bool
isValidMangledName (string name)
   {
     if (name.empty () || isdigit (name[0]))
          return false;

     for (string::size_type i = 0; i < name.size (); ++i)
        {
          if (!isalnum (name[i]) && name[i] != '_')
               return false;
        }
  // Passed all above checks -- assume OK
     return true;
   }
#endif

string
joinMangledQualifiersToString (const string& base, const string& name)
{
  string mangled_name (base);
  if (!base.empty () && !name.empty ())
    mangled_name += "__scope__";
  mangled_name += name;
  return mangled_name;
}

SgName
joinMangledQualifiers (const SgName& base, const SgName& name)
   {
#if 0
  // DQ (8/25/2006): This is debugging code required to find a problem that
  // results in a 480,000,000 character string used of a mangled name prefix
  // within the boost_tests/test_boost_phoenix_v2.C
     printf ("base.getString ().size() = %ld name.getString ().size() = %ld \n",base.getString().size(),name.getString().size());

     const int MODERATE_SIZE = 1000;
     if (base.getString().size() > MODERATE_SIZE)
          printf ("base.getString() = %s \n",base.getString().c_str());

     const int MAX_SIZE = 10000;
     ROSE_ASSERT(base.getString().size() < MAX_SIZE);
     ROSE_ASSERT(name.getString().size() < MAX_SIZE);
#endif

     string mangled_name = joinMangledQualifiersToString (base.getString (),name.getString ());
     return SgName (mangled_name.c_str ());
   }

const SgFunctionDefinition*
findRootFunc (const SgScopeStatement* scope)
   {
   // DQ (12/13/2011): This function is being called recursively (infinite recursion) for test2011_187.C
   // (added support for SgTemplateFunctionDefinition).
      if (scope != nullptr)
        {
          switch (scope->variantT())
            {
              case V_SgFunctionDefinition:
                return isSgFunctionDefinition (scope);

              case V_SgTemplateFunctionDefinition:
                return isSgTemplateFunctionDefinition (scope);

              case V_SgGlobal:
                // Rasmussen (3/17/2021): Reached end of the line (of scopes), return nullptr
                // Note: get_mangled_name() causes havoc for Jovial. See issues RC-227 and RC-765.
                // Returning nullptr at global scope helps solve Jovial problems.
                return nullptr;

              default:
                // DQ (12/13/2011): Adding test for improperly set scope.
                SgScopeStatement* nextOuterScope = scope->get_scope();
                ROSE_ASSERT(nextOuterScope != nullptr);
                ROSE_ASSERT(nextOuterScope != scope);
                return findRootFunc(nextOuterScope);
            }
        }

  // Not found.
     return nullptr;
   }


// size_t getLocalScopeNum ( SgFunctionDefinition* func_def, const SgScopeStatement* target)
size_t
getLocalScopeNum (const SgFunctionDefinition* func_def, const SgScopeStatement* target)
{
#if SKIP_BLOCK_NUMBER_CACHING
  // DQ (10/4/2006): This takes too long and stalls the compilation of
  // some large codes (plum hall e.g. cvs06a/conform/ch7_22.c).  It is
  // rewritten below to use a cache mechanisk link to a cache invalidation
  // mechanism.

  // Preorder traversal to count the number of basic blocks preceeding 'target'
  class Traversal : public AstSimpleProcessing
  {
  public:
    Traversal (const SgScopeStatement* target)
      : target_ (target), count_ (0), found_ (false)
    {}
    void visit (SgNode* node)
    {
      if (!found_)
        {
          const SgScopeStatement* stmt = isSgScopeStatement (node);
          if (stmt)
            {
              count_++;
              found_ = (stmt == target_);
            }
        }
    }
    size_t count (void) const { return found_ ? count_ : 0; }
  private:
    const SgScopeStatement* target_; // Target scope statement
    size_t count_; // found_ ? number of target : number of last block seen
    bool found_; // true <==> target_ has been found
  };

  Traversal counter (target);
  counter.traverse (const_cast<SgFunctionDefinition *> (func_def), preorder);
  return counter.count ();
#else
  // DQ (10/6/2006): Implemented caching of computed lables for scopes in
  // functions to avoid quadratic behavior of previous implementation.  The model
  // for this is the same a s what will be done to support caching of mangled names.
  // printf ("getLocalScopeNum calling func_def->get_scope_number(target)! \n");

     return func_def->get_scope_number(target);
#endif
}

string
mangleLocalScopeToString (const SgScopeStatement* scope)
{
// DQ (3/20/2011): Make this a valid ostringstream.
// ostringstream mangled_name = "";
  ostringstream mangled_name;
// ROSE_ASSERT(scope != NULL);
  if (scope)
    {
      // Find the function definition containing this scope.
      const SgFunctionDefinition* root_func = findRootFunc (scope);
      if (root_func)
        {
          size_t i = getLocalScopeNum (root_func, scope);
          ROSE_ASSERT (i > 0);
          mangled_name << "__SgSS" << i << "__";
        }
    }
  return mangled_name.str ();
}

string
mangleQualifiersToString (const SgScopeStatement* scope)
   {
#if 0
     printf ("In manglingSupport.C: mangleQualifiersToString(const SgScopeStatement*): scope = %p = %s \n",scope,scope->class_name().c_str());
#endif

  // DQ (3/14/2012): I would like to make this assertion (part of required C++ support).
     ROSE_ASSERT(scope != NULL);

     string mangled_name = "";
     if (scope != NULL)
        {
          switch (scope->variantT ())
             {
            // DQ (9/27/2012): Added this case to be the same as that for SgClassDefinition (removed case below).
               case V_SgTemplateClassDefinition:

               case V_SgClassDefinition:
               case V_SgTemplateInstantiationDefn:
                  {
                    const SgClassDefinition* def = isSgClassDefinition (scope);

#define DEBUG_RECURSIVE_USE 1

#if DEBUG_RECURSIVE_USE
                 // DQ (10/23/2015): Added debugging code for recursively defined use (RoseExample_tests_01.C).
                    static SgClassDefinition* previously_used_class_definition = NULL;
                 // printf ("In manglingSupport.C: mangleQualifiersToString(const SgScopeStatement*): previously_used_class_definition = %p def = %p \n",previously_used_class_definition,def);
                    if (def == previously_used_class_definition)
                       {
                         SgName name;
                         if (isSgTemplateInstantiationDefn(def) != NULL)
                            {
                              name = isSgTemplateInstantiationDecl(def->get_declaration())->get_templateName().str();
                            }
                           else
                            {
                              name = def->get_declaration()->get_name().str();
                            }
#if 0
                         printf ("def was just processed: def = %p = %s class declaration name = %s \n",def,def->class_name().c_str(),name.str());
                         def->get_file_info()->display("def used recursively");
#endif
                       }
                    previously_used_class_definition = const_cast<SgClassDefinition*>(def);
#endif

#if 1
                 // DQ (10/31/2015): This is an attempt to break the cycles that would result in infinite
                 // recursive calls to mangle the names of template class instantiations.
#if 0
                    MangledNameSupport::outputVisitedTemplateDefinitions();
#endif
                    SgClassDefinition* nonconst_def = const_cast<SgClassDefinition*>(def);
                    if (MangledNameSupport::visitedTemplateDefinitions.find(nonconst_def) != MangledNameSupport::visitedTemplateDefinitions.end())
                       {
                      // Skip the call that would result in infinte recursion.
#if 0
                         printf ("In manglingSupport.C: mangleQualifiersToString(): skipping the call to process the template class instantiation definition: def = %p = %s \n",def,def->class_name().c_str());
#endif
                       }
                      else
                       {
                         SgClassDefinition* templateInstantiationDefinition = isSgTemplateInstantiationDefn(nonconst_def);
                         if (templateInstantiationDefinition != NULL)
                            {
#if 0
                              printf ("Insert templateInstantiationDefinition = %p into visitedTemplateDeclarations (stl set)\n",templateInstantiationDefinition);
#endif
                           // Not clear why we need to use an iterator to simply insert a pointer into the set.
                           // SgTemplateInstantiationDefn* nonconst_templateInstantiationDefinition = const_cast<SgTemplateInstantiationDefn*>(templateInstantiationDefinition);
                              MangledNameSupport::setType::iterator it = MangledNameSupport::visitedTemplateDefinitions.begin();
                           // MangledNameSupport::visitedTemplateDeclarations.insert(it,nonconst_templateInstantiationDefinition);
                              MangledNameSupport::visitedTemplateDefinitions.insert(it,nonconst_def);
                            }

#if 0
                      // DQ (7/24/2017): This will allow us to see when there is infinit recursion that ends in a stack overflow for test2017_57.C.
                         printf ("In manglingSupport.C: mangleQualifiersToString(): Calling def->get_mangled_name(): def = %p = %s \n",def,def->class_name().c_str());
#endif
#if 0
                         printf ("In manglingSupport.C: mangleQualifiersToString(): def = %p = %s \n",def,def->class_name().c_str());
                         SgScopeStatement* tmp_scope = def->get_scope();
                         ROSE_ASSERT(tmp_scope != NULL);
                         printf ("In manglingSupport.C: mangleQualifiersToString(): tmp_scope = %p = %s \n",tmp_scope,tmp_scope->class_name().c_str());
#endif
                         ROSE_ASSERT(def != NULL);
                         mangled_name = def->get_mangled_name().getString();
#if 0
                         printf ("DONE: In manglingSupport.C: mangleQualifiersToString(): Calling def->get_mangled_name(): def = %p = %s \n",def,def->class_name().c_str());
#endif

                      // DQ (10/31/2015): The rule here is that after processing as a mangled name we remove the
                      // template instantiation from the list so that other non-nested uses of the template
                      // instantiation will force the manged name to be generated.
                         if (templateInstantiationDefinition != NULL)
                            {
#if 0
                              printf ("Erase templateInstantiationDefinition = %p from visitedTemplateDeclarations (stl set)\n",templateInstantiationDefinition);
#endif
                              MangledNameSupport::visitedTemplateDefinitions.erase(nonconst_def);
                            }
                       }
#else

#error "DEAD CODE!"

                 // Original code that demonstrated infinite recursive behavior for rare case of test2015_105.C (extracted from boost use in ROSE header files).
#if 0
                    printf ("In manglingSupport.C: mangleQualifiersToString(): Calling def->get_mangled_name(): def = %p = %s \n",def,def->class_name().c_str());
#endif
                    mangled_name = def->get_mangled_name().getString();
#if 0
                    printf ("DONE: In manglingSupport.C: mangleQualifiersToString(): Calling def->get_mangled_name(): def = %p = %s \n",def,def->class_name().c_str());
#endif
#endif

#if 0
                    const SgClassDeclaration* classDeclaration = def->get_declaration();
                    string name = classDeclaration->get_name().str();
                    printf ("In manglingSupport.C: mangleQualifiersToString(): mangled name for scope = %p = %s is: %s \n",scope,scope->class_name().c_str(),mangled_name.c_str());
                    printf ("In manglingSupport.C: mangleQualifiersToString(): class declaration = %p = %s class name = %s \n",classDeclaration,classDeclaration->class_name().c_str(),name.c_str());
#endif
                    break;
                  }

               case V_SgNamespaceDefinitionStatement:
                  {
                    const SgNamespaceDefinitionStatement* def = isSgNamespaceDefinitionStatement (scope);
                    ROSE_ASSERT(def != NULL);
                    mangled_name = def->get_mangled_name().getString();
#if 0
                    printf ("In manglingSupport.C: mangleQualifiersToString(): mangled name for scope = %p = %s is: %s \n",scope,scope->class_name().c_str(),mangled_name.c_str());
#endif
                    break;
                  }

            // DQ (9/27/2012): Added this case to be the same as that for SgFunctionDefinition (removed case below).
               case V_SgTemplateFunctionDefinition:
               case V_SgFunctionDefinition:
                  {
                    // Rasmussen (3/17/2021): Jovial doesn't need name mangling for functions as
                    // they can cause infinite recursion because a typedef may be a function argument.
                    if (SageInterface::is_Jovial_language())
                      {
                        mangled_name = "";
                      }
                    else
                      {
                     // 'scope' is part of scope for locally defined classes
                        const SgFunctionDefinition* def = isSgFunctionDefinition (scope);
                        ROSE_ASSERT(def != nullptr);
                        mangled_name = def->get_mangled_name().getString();
                      }
                    break;
                  }

               case V_SgRangeBasedForStatement:
               case V_SgCatchOptionStmt:
               case V_SgDoWhileStmt:
               case V_SgForStatement:
               case V_SgIfStmt:
               case V_SgJovialForThenStatement:
               case V_SgSwitchStatement:
               case V_SgWhileStmt:
               case V_SgBasicBlock:
                  {
                    const SgScopeStatement* stmt = isSgScopeStatement(scope);
                    ROSE_ASSERT (stmt != NULL);
                    string stmt_name      = mangleLocalScopeToString (stmt);
                    string par_scope_name = mangleQualifiersToString (scope->get_scope());

                    mangled_name = joinMangledQualifiersToString (par_scope_name,stmt_name);
                    break;
                  }

               case V_SgAdaPackageSpec:
                  {
                    const SgAdaPackageSpec*     spec   = isSgAdaPackageSpec(scope);
                    const SgNode*               parent = spec->get_parent();
                    ROSE_ASSERT(parent);

                    const SgAdaPackageSpecDecl* dcl    = isSgAdaPackageSpecDecl(parent);
                    ROSE_ASSERT(dcl);

                    // \todo \revise dcl may not exist for a special, hidden scope
                    std::string par_scope_name = mangleQualifiersToString(dcl->get_scope());
                    std::string dcl_name       = dcl->get_name();

                    mangled_name = joinMangledQualifiersToString (par_scope_name,dcl_name);
                    break;
                  }

               case V_SgAdaPackageBody:
                 {
                   const SgAdaPackageBody*     spec   = isSgAdaPackageBody(scope);
                   const SgNode*               parent = spec->get_parent();
                   ROSE_ASSERT(parent);

                   const SgAdaPackageBodyDecl* dcl    = isSgAdaPackageBodyDecl(parent);
                   ROSE_ASSERT(dcl);

                   std::string par_scope_name = mangleQualifiersToString(dcl->get_scope());
                   std::string dcl_name       = dcl->get_name();

                   mangled_name = joinMangledQualifiersToString (par_scope_name,dcl_name);
                   break;
                 }

               case V_SgAdaTaskSpec:
                  {
                    const SgAdaTaskSpec*     spec   = isSgAdaTaskSpec(scope);
                    const SgNode*            parent = spec->get_parent();
                    ROSE_ASSERT(parent);

                    std::string dcl_name;
                    std::string par_scope_name;

                    // or get_mangled_name ??
                    if (const SgAdaTaskSpecDecl* taskdcl = isSgAdaTaskSpecDecl(parent))
                    {
                      dcl_name = taskdcl->get_name().getString();
                      par_scope_name = mangleQualifiersToString(taskdcl->get_scope());
                    }
                    else if (const SgAdaTaskTypeDecl* tasktype = isSgAdaTaskTypeDecl(parent))
                    {
                      dcl_name = tasktype->get_name().getString();
                      par_scope_name = mangleQualifiersToString(tasktype->get_scope());
                    }
                    else
                      ROSE_ABORT();

                    mangled_name = joinMangledQualifiersToString (par_scope_name,dcl_name);
                    break;
                  }

               case V_SgAdaTaskBody:
                  {
                    const SgAdaTaskBody*     body   = isSgAdaTaskBody(scope);
                    const SgNode*            parent = body->get_parent();
                    ROSE_ASSERT(parent);

                    // or get_mangled_name ??
                    const SgAdaTaskBodyDecl* dcl = isSgAdaTaskBodyDecl(parent);
                    ROSE_ASSERT(dcl);

                    std::string par_scope_name = mangleQualifiersToString(dcl->get_scope());
                    std::string dcl_name       = dcl->get_name();

                    mangled_name = joinMangledQualifiersToString (par_scope_name,dcl_name);
                    break;
                  }

               case V_SgAdaProtectedSpec:
                  {
                    const SgAdaProtectedSpec* spec   = isSgAdaProtectedSpec(scope);
                    const SgNode*             parent = spec->get_parent();
                    ROSE_ASSERT(parent);

                    std::string dcl_name;
                    std::string par_scope_name;

                    // or get_mangled_name ??
                    if (const SgAdaProtectedSpecDecl* podcl = isSgAdaProtectedSpecDecl(parent))
                    {
                      dcl_name = podcl->get_name().getString();
                      par_scope_name = mangleQualifiersToString(podcl->get_scope());
                    }
                    else if (const SgAdaProtectedTypeDecl* potype = isSgAdaProtectedTypeDecl(parent))
                    {
                      dcl_name = potype->get_name().getString();
                      par_scope_name = mangleQualifiersToString(potype->get_scope());
                    }
                    else
                      ROSE_ABORT();

                    mangled_name = joinMangledQualifiersToString (par_scope_name,dcl_name);
                    break;
                  }

               case V_SgAdaProtectedBody:
                  {
                    const SgAdaProtectedBody* body   = isSgAdaProtectedBody(scope);
                    const SgNode*             parent = body->get_parent();
                    ROSE_ASSERT(parent);

                    // or get_mangled_name ??
                    const SgAdaProtectedBodyDecl* dcl = isSgAdaProtectedBodyDecl(parent);
                    ROSE_ASSERT(dcl);

                    std::string par_scope_name = mangleQualifiersToString(dcl->get_scope());
                    std::string dcl_name       = dcl->get_name();

                    mangled_name = joinMangledQualifiersToString (par_scope_name,dcl_name);
                    break;
                  }

               case V_SgAdaGenericDefn:
                  {
                    const SgAdaGenericDefn*   defn = isSgAdaGenericDefn(scope);
                    const SgAdaGenericDecl*   parent = isSgAdaGenericDecl(defn->get_parent());
                    ROSE_ASSERT(parent);

                    mangled_name = parent->get_name();

#if OBSOLETE_CODE
                    // PP (10/14/22) SgAdaGenericDecl::p_name was added
                    if (const SgFunctionDeclaration* fundec = isSgFunctionDeclaration(parent->get_declaration()))
                      mangled_name = fundec->get_name();
                    else if (const SgAdaPackageSpecDecl* pkgdec = isSgAdaPackageSpecDecl(parent->get_declaration()))
                      mangled_name = pkgdec->get_name();
                    else if (parent->get_declaration() == nullptr)
                    {
                      // \todo could this replace the two branches before?
                      SgSymbol* sy = parent->search_for_symbol_from_symbol_table();

                      ASSERT_not_null(sy);
                      mangled_name = sy->get_name();
                    }
                    else
                    {
                      std::cerr << typeid(*parent->get_declaration()).name() << std::endl;
                      ROSE_ABORT();
                    }
#endif /* OBSOLETE_CODE */

                    // \todo PP (10/10/22) include parent scopes?
                    break;
                  }

           // PP (06/01/20) - not sure how to handle function parameter scope;
           //                 for now, handle like SgGlobal
               case V_SgFunctionParameterScope:
           // DQ (2/22/2007): I'm not sure this is best, but we can leave it for now.
           // I expect that global scope should contribute to the mangled name to avoid
           // confusion with name of declarations in un-name namespaces for example.
               case V_SgGlobal: // Global scope has an 'empty' name
                  {
                 // I think there is nothing to do for this case.
                    break;
                  }

#if 0
               case V_SgTemplateFunctionDefinition:
                  {
#if 1
                 // DQ (8/26/2012): Decrease the volume of warnings from this part of the code.
#ifdef ROSE_DEBUG_NEW_EDG_ROSE_CONNECTION
                    static int count = 0;
                    if (count++ % 100 == 0)
                       {
                         printf ("WARNING: In mangleQualifiersToString(const SgScopeStatement*): Case SgTemplateFunctionDefinition not implemented \n");
                       }
#endif
#else
                    printf ("WARNING: In mangleQualifiersToString(const SgScopeStatement*): Case SgTemplateFunctionDefinition not implemented \n");
#endif
                    break;
                  }
#endif
#if 0
            // DQ (5/12/2012): Implement this case later, but it does not cause anything to fail presently (I think).
               case V_SgTemplateClassDefinition:
                  {
#if 1
                 // DQ (8/26/2012): Decrease the volume of warnings from this part of the code.
                    static int count = 0;
                    if (count++ % 100 == 0)
                       {
                         printf ("WARNING: In mangleQualifiersToString(const SgScopeStatement*): Case SgTemplateClassDefinition not implemented \n");
                       }
#else
                    printf ("WARNING: In mangleQualifiersToString(const SgScopeStatement*): Case SgTemplateClassDefinition not implemented \n");
#endif
                    break;
                  }
#endif

            // DQ (7/24/2017): Added support for new scope used to hold classes constructed from template parameters in the rare cases where this is done.
               case V_SgDeclarationScope: // The declaration scope has an 'empty' name
                  {
                 // I think there is nothing to do for this case.
                    break;
                  }

            // DQ (3/14/2012): I think that defaults should be resurced for errors, and not proper handling of unexpected cases.
               default: // Embed the class name for subsequent debugging.
                  {
                 // DQ (7/24/2017): I think it is a mistake to supress this comment.
#if 0
                 // DQ (3/19/2014): Supress this output, but permit it with verbose level == 1.
                    if (SgProject::get_verbose() > 0)
                       {
                         printf ("WARNING: In mangleQualifiersToString(const SgScopeStatement*): case of scope = %p = %s not handled (default reached) \n",scope,scope->class_name().c_str());
                       }
#else
                    printf ("WARNING: In mangleQualifiersToString(const SgScopeStatement*): case of scope = %p = %s not handled (default reached) \n",scope,scope->class_name().c_str());
#endif

                 // DQ (1/12/13): Assert that this is not a previously deleted IR node (which will have the name = "SgNode").
                    ROSE_ASSERT(scope->class_name() != "SgNode");

                 // DQ (1/12/13): Added assertion.
                    ROSE_ASSERT(scope->get_scope() != NULL);

                 // Surrounding scope name
                    string par_scope_name = mangleQualifiersToString (scope->get_scope());

                 // Compute a local scope name
                 //! \todo Compute local scope names correctly (consistently).
                    ostringstream scope_name;

                    scope_name << scope->class_name ();

                 // DQ (3/15/2016): Only reformatted this code below.
                    AstRegExAttribute *attribute = (AstRegExAttribute *) scope -> getAttribute("name");
                    if (attribute != NULL)
                       {
                         scope_name << "_" << attribute -> expression;
                       }

                 // Build full name
                    mangled_name = joinMangledQualifiersToString (par_scope_name,scope_name.str ());
                    break;
                  }
             }
        }

  // DQ (5/31/2012): Added test for template brackets that are caught later in AstConsistencyTests.
  // Make sure that there is no template specific syntax included in the mangled name
  // if ( mangled_name.find('<') != string::npos )
     if (SageInterface::hasTemplateSyntax(mangled_name) == true)
        {
       // string name = classDeclaration->get_name().str();
       // printf ("In mangleQualifiersToString(): scope = %p = %s unmangled name = %s check mangled class name = %s \n",scope,scope->class_name().c_str(),name.c_str(),mangled_name.c_str());
          printf ("In mangleQualifiersToString(): scope = %p = %s check mangled class name = %s \n",scope,scope->class_name().c_str(),mangled_name.c_str());
        }

  // ROSE_ASSERT(mangled_name.find('<') == string::npos);
     ROSE_ASSERT(SageInterface::hasTemplateSyntax(mangled_name) == false);

  // AS (12/09/2024) Added loop to replace '.' with "_dot_" (for the Libadalang frontend)
     std::size_t dot_loc = mangled_name.find('.');
     while(dot_loc != std::string::npos)
     {
         mangled_name = mangled_name.substr(0, dot_loc) + "_dot_" + mangled_name.substr(dot_loc + 1);
         dot_loc = mangled_name.find('.');
     }

#if 0
     printf ("Leaving mangleQualifiersToString(): scope = %p = %s returning mangled_name = %s \n",scope,scope->class_name().c_str(),mangled_name.c_str());
#endif

     return mangled_name;
   }


SgName
mangleQualifiers( const SgScopeStatement* scope )
   {
  // DQ (3/14/2012): I think we have to assert this here, though it appears to have been commented out.
  // This may become a part of a future set of language dependent assertions in the AST Build Interface
  // since it is more relevant for C++ than for other languges.
  // DQ (3/19/2011): I think that we want a valid scope else there is no proper pointer to the generated string.
  // ROSE_ASSERT(scope != NULL);
     ROSE_ASSERT(scope != NULL);

  // DQ (1/12/13): Assert that this is not a previously deleted IR node (which will have the name = "SgNode").
     ROSE_ASSERT(scope->class_name() != "SgNode");

#if 0
#if 0
     if (isSgDeclarationScope(scope) != NULL)
        {
          printf ("In manglingSupport.C: scope = %p = %s \n",scope,scope->class_name().c_str());
        }
#else
     printf ("In manglingSupport.C: scope = %p = %s \n",scope,scope->class_name().c_str());
#endif
#endif

  // DQ (7/20/2017): Assert that this is not a dangling pointer.
     ROSE_ASSERT(scope->class_name() != "SgLocatedNode");

  // DQ (2/17/2014): Adding debugging code (issue with new options to gnunet).
  // DQ (1/12/13): Added assertion.
     if (scope->get_scope() == NULL)
        {
#if 0
          printf ("Warning: In mangleQualifiers(): (scope->get_scope() == NULL): scope = %p = %s = %s \n",scope,scope->class_name().c_str(),SageInterface::get_name(scope).c_str());
#endif
#if 0
          scope->get_file_info()->display("Warning: scope->get_scope() == NULL: debug \n");
#endif
       // DQ (2/17/2014): In this case there is no mangled name.
          return SgName("");
        }
     ROSE_ASSERT(scope->get_scope() != NULL);

#if 0
     printf ("In manglingSupport.C: scope = %p = %s \n",scope,scope->class_name().c_str());
#endif

     string s = mangleQualifiersToString(scope);

#if 0
     printf ("In manglingSupport.C: mangleQualifiers(const SgScopeStatement*): returning s = %s \n",s.c_str());
#endif

     return SgName(s.c_str());
   }

string
mangleTypesToString (const SgTypePtrList::const_iterator b,
                     const SgTypePtrList::const_iterator e)
   {
     string mangled_name;
     bool is_first = true;

#if 0
     printf ("In mangleTypesToString(): TOP \n");
#endif

     for (SgTypePtrList::const_iterator p = b; p != e; ++p)
        {
          const SgType* type_p = *p;
          ROSE_ASSERT (type_p != NULL);
#if 0
          printf ("In mangleTypesToString(): in loop: is_first = %s type_p = %p = %s \n",is_first ? "true" : "false",type_p,type_p->class_name().c_str());
#endif

#if 0 /* X1 */
       // DQ (2/14/2016): Adding support for VLA types (it is better to add this support directly to the SgArrayType::get_mangled() function).
          SgArrayType* arrayType = isSgArrayType(*p);
          bool is_variable_length_array = false;
          if (arrayType != NULL)
             {
#if 0
               printf ("   --- arrayType->get_is_variable_length_array() = %s \n",arrayType->get_is_variable_length_array() ? "true" : "false");
#endif
            // is_variable_length_array = arrayType->get_is_variable_length_array();
             }
#endif /* X1 */

          if (is_first == true)
             {
               is_first = false;

            // DQ (5/11/2012): Make the mangled names a little more clear.
               mangled_name += "_";
             }
            else
             {
               mangled_name += "__sep__"; // separator between argument types
             }

          ROSE_ASSERT(type_p != NULL);
          ROSE_ASSERT(const_cast<SgType *>(type_p) != NULL);
#if 0
          printf ("In mangleTypesToString(): calling (const_cast<SgType *>(type_p))->get_mangled() \n");
#endif

       // DQ (2/14/2016): Adding support for VLA types.
       //    1) All vla types are equivalent, so no further name mangling is useful.
       //    2) If we proceed then we will cause endless recursion in the evaluation
       //       of the scope of the array index variable reference expressions.
#if 1 /* Y1 */
          SgName mangled_p = (const_cast<SgType *>(type_p))->get_mangled();
#else /* Y1 */
          SgName mangled_p;
          if (is_variable_length_array == true)
             {
#if 0
               printf ("WARNING: We need to compute the mangled name of the base type \n");
#endif
               mangled_p = "_vla_array_type";
             }
            else
             {
               mangled_p = (const_cast<SgType *>(type_p))->get_mangled();
             }
#endif /* Y1 */
#if 0
          printf ("In mangleTypesToString(): mangled_p = %s \n",mangled_p.str());
#endif
          mangled_name += string (mangled_p.str());
#if 0
          printf ("In mangleTypesToString(): accumulated mangled_name = %s \n",mangled_name.c_str());
#endif
        }

  // DQ (5/11/2012): Make the mangled names a little more clear.
     mangled_name += "_";

#if 0
     printf ("In mangleTypesToString(): returning mangled_name = %s \n",mangled_name.c_str());
#endif

     return mangled_name;
   }

SgName
mangleTypes (const SgTypePtrList::const_iterator b,
             const SgTypePtrList::const_iterator e)
  {
    string mangled_name_str = mangleTypesToString (b, e);
    return SgName (mangled_name_str.c_str ());
  }

string
mangleFunctionNameToString (const string& s, const string& ret_type_name )
   {
  // DQ (7/24/2012): This causes a problem for the isspace(s[n_opstr]) test below (likely not the cause of the problem).
     string s_mangled (trimSpaces (s));

    // Special case: destructor names
    if (s[0] == '~')
      {
        s_mangled.replace (0, 1, "__dt");
        return s_mangled;
      }

    // Check for overloaded operators
    const string opstr ("operator");
    const string::size_type n_opstr = opstr.size ();
    const string newstr ("new");
    const string newarrstr ("new[]");
    const string delstr ("delete");
    const string delarrstr ("delete[]");

    if (s.substr (0, n_opstr) == opstr) // begins with "operator"
      {
        if (isspace(s[n_opstr]))
          {
            if (s.substr (n_opstr+1) == newstr) // is "operator new"
              s_mangled.replace (n_opstr, newstr.size ()+1, "__nw");
            else if (s.substr (n_opstr+1) == newarrstr) // "operator new[]"
              s_mangled.replace (n_opstr, newarrstr.size ()+1, "__na");
            else if (s.substr (n_opstr+1) == delstr) // is "operator delete"
              s_mangled.replace (n_opstr, delstr.size ()+1, "__dl");
            else if (s.substr (n_opstr+1) == delarrstr) // "operator delete[]"
              s_mangled.replace (n_opstr, delarrstr.size ()+1, "__da");
            else // Cast operators.
              s_mangled.replace (n_opstr, s.size () - n_opstr,
                                 "__opB__" + ret_type_name + "__opE__");
          }
        else // real operator (suffix after the substring "operator ")
          {
            string s_op = s.substr (n_opstr);
         // DQ (2/7/2006): Bug fix for case of function: operator_takes_lvalue_operand()
         // (this test appears in test2005_198.C).
            string s_op_mangled = s_op;
            if (s_op == "->")     s_op_mangled = "__rf";
            else if (s_op == "->*") s_op_mangled = "__rm";
            else if (s_op == "==")  s_op_mangled = "__eq";
            else if (s_op == "<")   s_op_mangled = "__lt";
            else if (s_op == ">")   s_op_mangled = "__gt";
            else if (s_op == "!=")  s_op_mangled = "__ne";
            else if (s_op == "<=")  s_op_mangled = "__le";
            else if (s_op == ">=")  s_op_mangled = "__ge";
            else if (s_op == "+")   s_op_mangled = "__pl";
            else if (s_op == "-")   s_op_mangled = "__mi";
            else if (s_op == "*")   s_op_mangled = "__ml";
            else if (s_op == "/")   s_op_mangled = "__dv";
            else if (s_op == "%")   s_op_mangled = "__md";
            else if (s_op == "&&")  s_op_mangled = "__aa";
            else if (s_op == "!")   s_op_mangled = "__nt";
            else if (s_op == "||")  s_op_mangled = "__oo";
            else if (s_op == "^")   s_op_mangled = "__er";
            else if (s_op == "&")   s_op_mangled = "__ad";
            else if (s_op == "|")   s_op_mangled = "__or";
            else if (s_op == ",")   s_op_mangled = "__cm";
            else if (s_op == "<<")  s_op_mangled = "__ls";
            else if (s_op == ">>")  s_op_mangled = "__rs";
            else if (s_op == "--")  s_op_mangled = "__mm";
            else if (s_op == "++")  s_op_mangled = "__pp";
            else if (s_op == "~")   s_op_mangled = "__co";
            else if (s_op == "=")   s_op_mangled = "__as";
            else if (s_op == "+=")  s_op_mangled = "__apl";
            else if (s_op == "-=")  s_op_mangled = "__ami";
            else if (s_op == "&=")  s_op_mangled = "__aad";
            else if (s_op == "|=")  s_op_mangled = "__aor";
            else if (s_op == "*=")  s_op_mangled = "__amu";
            else if (s_op == "/=")  s_op_mangled = "__adv";
            else if (s_op == "%=")  s_op_mangled = "__amd";
            else if (s_op == "^=")  s_op_mangled = "__aer";
            else if (s_op == "<<=") s_op_mangled = "__als";
            else if (s_op == ">>=") s_op_mangled = "__ars";
            else if (s_op == "()")  s_op_mangled = "__cl";
            else if (s_op == "[]")  s_op_mangled = "__xi";
            else
              {
             // DQ (1/8/2006): This is the case of a name that just happends to start with
             // the work "operator" (e.g. operator_takes_lvalue_operand, in test2005_198.C)
             // the mangle form is just the unmodified function name.
             // rtmp = fname;
              }
         // DQ (2/7/2006): Bug fix for case of function such as operator_takes_lvalue_operand()
         // In the case of operator_takes_lvalue_operand() this should replace
         // "_takes_lvalue_operand" with "_takes_lvalue_operand" (trivial case).
            s_mangled.replace (n_opstr, s_op.size (), s_op_mangled);
          }
      }
    // else, leave name as is.

     return s_mangled;
   }


SgName
mangleFunctionName (const SgName& n, const SgName& ret_type_name )
  {
    string s_mangled = mangleFunctionNameToString (n.getString (), ret_type_name.str ());
    SgName n_mangled (s_mangled.c_str ());
    return n_mangled;
  }


string
mangleTemplateArgsToString (const SgTemplateArgumentPtrList::const_iterator b, const SgTemplateArgumentPtrList::const_iterator e)
   {
     ostringstream mangled_name;
     bool is_first = true;

     for (SgTemplateArgumentPtrList::const_iterator i = b; i != e; ++i)
        {
          if (is_first == true)
             {
               is_first = false;
             }
            else
             {
            // !is_first, so insert a separator string.
               mangled_name << "__sep__";
             }

          const SgTemplateArgument* arg = *i;
          ROSE_ASSERT (arg != NULL);
          mangled_name << arg->get_mangled_name().str();
        }

     return mangled_name.str();
   }


string
mangleTemplateParamsToString (const SgTemplateParameterPtrList::const_iterator b, const SgTemplateParameterPtrList::const_iterator e)
   {
     ostringstream mangled_name;
     bool is_first = true;

     for (SgTemplateParameterPtrList::const_iterator i = b; i != e; ++i)
        {
          if (is_first == true)
             {
               is_first = false;
             }
            else
             {
            // !is_first, so insert a separator string.
               mangled_name << "__sep__";
             }

          const SgTemplateParameter* param = *i;
          ROSE_ASSERT (param != NULL);
          mangled_name << param->get_mangled_name().str();
        }

     return mangled_name.str();
   }

string
mangleTemplateToString (const string& templ_name,
                        const SgTemplateArgumentPtrList& templ_args,
                        const SgScopeStatement* scope)
   {
  // Mangle all the template arguments

  // DQ (10/29/2015): Added assertion.
  // ROSE_ASSERT(scope != NULL);

#if 0
     printf ("In mangleTemplateToString(string&,SgTemplateArgumentPtrList&,SgScopeStatement*): template name = %s templ_args.size() = %zu scope = %p = %s \n",templ_name.c_str(),templ_args.size(),scope,scope->class_name().c_str());
#endif

     string args_mangled;
     if (templ_args.empty() == true)
        {
          args_mangled = "no_templ_args";
        }
       else
        {
          args_mangled = mangleTemplateArgsToString(templ_args.begin(),templ_args.end());
        }

#if 0
     printf ("In mangleTemplateToString(string&,SgTemplateArgumentPtrList&,SgScopeStatement*): compute the mangled name for the scope: template name = %s templ_args.size() = %zu scope = %p = %s \n",templ_name.c_str(),templ_args.size(),scope,scope->class_name().c_str());
#endif

  // Compute the name qualification, if any.
     string scope_name;
     if (scope == NULL)
        {
          scope_name = "unknown_scope";
        }
       else
        {
          scope_name = mangleQualifiersToString (scope);
        }

  // Compute the final mangled name.
     string mangled_name = joinMangledQualifiersToString (scope_name, templ_name);

     if (mangled_name.empty() == true)
        {
          mangled_name = "unknown_template_name";
        }

     mangled_name += "__tas__" + args_mangled + "__tae__";

  // printf ("args_mangled = %s mangled_name = %s \n",args_mangled.c_str(),mangled_name.c_str());

     return mangled_name;
   }

string
mangleTemplateToString (const string& templ_name,
                        const SgTemplateParameterPtrList& templ_params,
                        const SgScopeStatement* scope)
   {
  // Mangle all the template parameters

  // DQ (10/29/2015): Added assertion.
  // ROSE_ASSERT(scope != NULL);

#if 0
     printf ("In mangleTemplateToString(string&,SgTemplateParameterPtrList&,SgScopeStatement*): template name = %s templ_params.size() = %zu scope = %p = %s \n",templ_name.c_str(),templ_params.size(),scope,scope->class_name().c_str());
#endif

     string params_mangled;
     if (templ_params.empty() == true)
        {
          params_mangled = "unknown_param";
        }
       else
        {
          params_mangled = mangleTemplateParamsToString(templ_params.begin(),templ_params.end());
        }

#if 0
     printf ("In mangleTemplateToString(string&,SgTemplateParameterPtrList&,SgScopeStatement*): compute the mangled name for the scope: template name = %s templ_params.size() = %zu scope = %p = %s \n",templ_name.c_str(),templ_params.size(),scope,scope->class_name().c_str());
#endif

  // Compute the name qualification, if any.
     string scope_name;
     if (scope == NULL)
        {
          scope_name = "unknown_scope";
        }
       else
        {
          scope_name = mangleQualifiersToString (scope);
        }

  // Compute the final mangled name.
     string mangled_name = joinMangledQualifiersToString (scope_name, templ_name);

     if (mangled_name.empty() == true)
        {
          mangled_name = "unknown_template_name";
        }

     mangled_name += "__tps__" + params_mangled + "__tpe__";

  // printf ("params_mangled = %s mangled_name = %s \n",params_mangled.c_str(),mangled_name.c_str());

     return mangled_name;
   }

SgName
mangleTemplate (const SgName& templ_name,
                const SgTemplateArgumentPtrList& templ_args,
                const SgScopeStatement* scope)
   {
  // DQ (10/29/2015): Added assertion.
  // ROSE_ASSERT(scope != NULL);

#if 0
     printf ("In mangleTemplate(SgName&,SgTemplateArgumentPtrList&,SgScopeStatement*): template name = %s templ_args.size() = %zu scope = %p = %s \n",templ_name.str(),templ_args.size(),scope,scope->class_name().c_str());
#endif
     string mangled_name = templ_name.str();
     mangled_name = mangleTemplateToString(mangled_name,templ_args,scope);

#if 0
     printf ("In mangleTemplate(): mangled_name = %s \n",mangled_name.c_str());
#endif

     return SgName (mangled_name.c_str());
   }

SgName
mangleTemplate (const SgName& templ_name,
                const SgTemplateParameterPtrList& templ_params,
                const SgScopeStatement* scope)
   {
  // DQ (10/29/2015): Added assertion.
     ROSE_ASSERT(scope != NULL);

#if 0
     printf ("In mangleTemplate(SgName&,SgTemplateParameterPtrList&,SgScopeStatement*): template name = %s templ_params.size() = %zu scope = %p = %s \n",templ_name.str(),templ_params.size(),scope,scope->class_name().c_str());
#endif

     string mangled_name = mangleTemplateToString(templ_name.getString(),templ_params,scope);

#if 0
     printf ("In mangleTemplate(): mangled_name = %s \n",mangled_name.c_str());
#endif

     return SgName (mangled_name.c_str());
   }

string
mangleTemplateFunctionToString (const string& templ_name,
                                const SgTemplateArgumentPtrList& templ_args,
                                const SgFunctionType* func_type,
                                const SgScopeStatement* scope)
   {
  // Compute a mangled name for this function's type
     string type_name;
     string ret_type_name;

  // DQ (5/31/2012): This should be a valid name to be a template.
     ROSE_ASSERT(templ_name.empty() == false);

  // DQ (5/31/2012): Find locations where this is set and include template syntax.
  // ROSE_ASSERT(templ_name.find('<') == string::npos);
  // ROSE_ASSERT(SageInterface::hasTemplateSyntax(templ_name) == false);

     if (func_type != NULL)
        {
          type_name = func_type->get_mangled().getString();

          const SgType* ret_type = func_type->get_return_type ();
          if (ret_type != NULL)
             {
               ret_type_name = ret_type->get_mangled ().getString ();
             }
        }
       else
        {
          type_name = "UNKNOWN_FUNCTION_TYPE";
        }

  // This function's name, transformed.
     string func_name = mangleFunctionNameToString(templ_name,ret_type_name);

#if 0
  // DQ (7/24/2012): Added test (failing for test2004_141.C); fails later in AST consistency tests.
     ROSE_ASSERT(func_name.find('&') == string::npos);
     ROSE_ASSERT(func_name.find('*') == string::npos);
#endif

  // Compute the final mangled name.
     string mangled_name = mangleTemplateToString(func_name,templ_args,scope) + "__" + type_name;

     return mangled_name;
   }

SgName
mangleTemplateFunction (const string& templ_name,
                        const SgTemplateArgumentPtrList& templ_args,
                        const SgFunctionType* func_type,
                        const SgScopeStatement* scope)
  {
    return mangleTemplateFunctionToString (templ_name, templ_args, func_type, scope);
  }

/*! Mangles a value expression.
 *
 *  This template function is parameterized by a specific Sage III
 *  value type (derived from SgValueExp), and specifically relies on
 *  the 'get_value ()' member function.
 */
template <class SgValueExpType_>
string
mangleSgValueExp (const SgValueExpType_* expr)
  {
 // Verify that SgValueExpType_ descends from SgValueExp.
    ROSE_ASSERT (isSgValueExp (expr) || !expr);

    ostringstream mangled_name;
    if (expr)
      mangled_name << expr->get_value ();
    return mangled_name.str ();
  }

string
mangleSgValueExp (const SgBoolValExp* expr)
  {
    return (expr && expr->get_value ()) ? "true" : "false";
  }

string
mangleValueExp (const SgValueExp* expr)
  {
    string mangled_name;
    switch (expr->variantT ())
      {
      case V_SgBoolValExp:
        mangled_name = mangleSgValueExp<SgBoolValExp> (isSgBoolValExp (expr));
        break;
      case V_SgCharVal:
        mangled_name = mangleSgValueExp<SgCharVal> (isSgCharVal (expr));
        break;
      case V_SgDoubleVal:
        mangled_name = mangleSgValueExp<SgDoubleVal> (isSgDoubleVal (expr));
        break;
      case V_SgEnumVal:
        mangled_name = mangleSgValueExp<SgEnumVal> (isSgEnumVal (expr));
        break;
      case V_SgFloatVal:
        mangled_name = mangleSgValueExp<SgFloatVal> (isSgFloatVal (expr));
        break;
      case V_SgIntVal:
        mangled_name = mangleSgValueExp<SgIntVal> (isSgIntVal (expr));
        break;
      case V_SgLongDoubleVal:
        mangled_name = mangleSgValueExp<SgLongDoubleVal> (isSgLongDoubleVal (expr));
        break;
      case V_SgLongIntVal:
        mangled_name = mangleSgValueExp<SgLongIntVal> (isSgLongIntVal (expr));
        break;
     case V_SgLongLongIntVal: // Added by Liao, 2/10/2009
        mangled_name = mangleSgValueExp<SgLongLongIntVal> (isSgLongLongIntVal (expr));
        break;
     case V_SgShortVal:
        mangled_name = mangleSgValueExp<SgShortVal> (isSgShortVal (expr));
        break;
      case V_SgStringVal:
        mangled_name = mangleSgValueExp<SgStringVal> (isSgStringVal (expr));
        break;
      case V_SgUnsignedCharVal:
        mangled_name = mangleSgValueExp<SgUnsignedCharVal> (isSgUnsignedCharVal (expr));
        break;
      case V_SgSignedCharVal:
        mangled_name = mangleSgValueExp<SgSignedCharVal> (isSgSignedCharVal (expr));
        break;
      case V_SgUnsignedIntVal:
        mangled_name = mangleSgValueExp<SgUnsignedIntVal> (isSgUnsignedIntVal (expr));
        break;
      case V_SgUnsignedLongLongIntVal:
        mangled_name = mangleSgValueExp<SgUnsignedLongLongIntVal> (isSgUnsignedLongLongIntVal (expr));
        break;
      case V_SgUnsignedLongVal:
        mangled_name = mangleSgValueExp<SgUnsignedLongVal> (isSgUnsignedLongVal (expr));
        break;
      case V_SgUnsignedShortVal:
        mangled_name = mangleSgValueExp<SgUnsignedShortVal> (isSgUnsignedShortVal (expr));
        break;
      case V_SgWcharVal:
        mangled_name = mangleSgValueExp<SgWcharVal> (isSgWcharVal (expr));
        break;
      case V_SgCastExp:
        {
          const SgCastExp* cast_expr = isSgCastExp (expr); ROSE_ASSERT (cast_expr);
          const SgExpression* op = cast_expr->get_operand ();
          const SgType* cast_type = cast_expr->get_type ();
          mangled_name = "__Cstb__"
            + cast_type->get_mangled ().getString ()
            + mangleExpression (op)
            + "__Cste__";
        }
        break;

      case V_SgNullptrValExp:
        {
          mangled_name = "__nullptr";
          break;
        }

   // DQ (7/21/2012): Added support for IR node not seen previously except in new C++11 work.
      case V_SgTemplateParameterVal:
         {
           mangled_name = "unsupported_SgTemplateParameterVal"; // mangleSgValueExp<SgTemplateParameterVal> (isSgTemplateParameterVal(expr));
           break;
         }

   // DQ (2/14/2019): Adding support for C++14 void values (still unclear what this should look like).
      case V_SgVoidVal:
         {
           mangled_name = "unsupported_SgVoidVal";
           break;
         }

      default:
        std::cerr<<"Error! Unhandled case in mangleValueExp() for "<<expr->sage_class_name()<<std::endl;
        ROSE_ABORT (); // Unhandled case.
      }

    return replaceNonAlphaNum (mangled_name);
  }

static void mangleUnaryOp(const SgUnaryOp * uop, std::ostringstream & mangled_name, std::string opname) {
  mangled_name << "_b" << opname << "_" << mangleExpression (uop->get_operand_i()) << "_e" << opname << "_";
}

static void mangleBinaryOp(const SgBinaryOp * binop, std::ostringstream & mangled_name, std::string opname) {
  mangled_name << "_b" << opname << "_" << mangleExpression (binop->get_lhs_operand_i()) << "__" << mangleExpression (binop->get_rhs_operand_i()) << "_e" << opname << "_";
}

string
mangleExpression (const SgExpression* expr)
  {
    ostringstream mangled_name;
    ROSE_ASSERT(expr != NULL);

#if 0
    printf("In mangleExpression: expr = %p (%s)\n", expr, expr ? expr->class_name().c_str() : "");
    printf("                     expr : %s\n", expr ? expr->unparseToString().c_str() : "");
#endif
    const SgValueExp* val = isSgValueExp (expr);
    if (val != NULL) {
      mangled_name << mangleValueExp (val);
    } else {
      switch (expr->variantT()) {
        case V_SgVarRefExp: {
          const SgVarRefExp* e = isSgVarRefExp (expr);
          SgVariableSymbol * vsym = e->get_symbol();
          ROSE_ASSERT(vsym != NULL);
          SgInitializedName * iname = vsym->get_declaration();
          ROSE_ASSERT(iname != NULL);
          mangled_name << iname->get_mangled_name().str();
          break;
        }
        case V_SgFunctionRefExp: {
          const SgFunctionRefExp* e = isSgFunctionRefExp (expr);
          SgFunctionSymbol * fsym = e->get_symbol_i();
          ROSE_ASSERT(fsym != NULL);
          SgFunctionDeclaration * fdecl = fsym->get_declaration();
          ROSE_ASSERT(fdecl != NULL);
          mangled_name << fdecl->get_mangled_name().str();
          break;
        }
        case V_SgTemplateFunctionRefExp: {
          const SgTemplateFunctionRefExp* e = isSgTemplateFunctionRefExp (expr);
          SgTemplateFunctionSymbol * fsym = e->get_symbol_i();
          ROSE_ASSERT(fsym != NULL);
          SgFunctionDeclaration * fdecl = fsym->get_declaration();
          ROSE_ASSERT(fdecl != NULL);
          mangled_name << fdecl->get_mangled_name().str();
          break;
        }
        case V_SgMemberFunctionRefExp: {
          const SgMemberFunctionRefExp* e = isSgMemberFunctionRefExp (expr);
          SgMemberFunctionSymbol * fsym = e->get_symbol_i();
          ROSE_ASSERT(fsym != NULL);
          SgMemberFunctionDeclaration * fdecl = fsym->get_declaration();
          ROSE_ASSERT(fdecl != NULL);
          mangled_name << fdecl->get_mangled_name().str();
          break;
        }
        case V_SgTemplateMemberFunctionRefExp: {
          const SgTemplateMemberFunctionRefExp* e = isSgTemplateMemberFunctionRefExp (expr);
          SgTemplateMemberFunctionSymbol * fsym = e->get_symbol_i();
          ROSE_ASSERT(fsym != NULL);
          SgMemberFunctionDeclaration * fdecl = fsym->get_declaration();
          ROSE_ASSERT(fdecl != NULL);
          mangled_name << fdecl->get_mangled_name().str();
          break;
        }
        case V_SgNonrealRefExp: {
          const SgNonrealRefExp* nrref = isSgNonrealRefExp (expr);
          SgNonrealSymbol * nrsym = nrref->get_symbol();
          ROSE_ASSERT(nrsym != NULL);
          SgNonrealDecl * nrdecl = nrsym->get_declaration();
          ROSE_ASSERT(nrdecl != NULL);
          nrdecl->get_is_nonreal_template(); // FIXME TMP Reference to get as it is not visible in the lib!!!
          mangled_name << nrdecl->get_mangled_name().str();
          break;
        }

        case V_SgCastExp: {
          const SgCastExp * cast = isSgCastExp(expr);
          ROSE_ASSERT(cast != NULL);
          SgExpression * op = cast->get_operand_i();
          ROSE_ASSERT(op != NULL);
          SgType * cast_type = cast->get_type();
          ROSE_ASSERT(cast_type != NULL);
          mangled_name << "_bCastExp_" << mangleExpression(op) << "_totype_" << cast_type->get_mangled().str() << "_eCastExp_";
          break;
        }

        case V_SgNotOp:            mangleUnaryOp( (const SgUnaryOp *)expr, mangled_name, "NotOp");           break;
        case V_SgBitComplementOp:  mangleUnaryOp( (const SgUnaryOp *)expr, mangled_name, "BitComplementOp"); break;
        case V_SgMinusOp:          mangleUnaryOp( (const SgUnaryOp *)expr, mangled_name, "MinusOp");         break;
        case V_SgUnaryAddOp:       mangleUnaryOp( (const SgUnaryOp *)expr, mangled_name, "UnaryAddOp");      break;
        case V_SgAddressOfOp:      mangleUnaryOp( (const SgUnaryOp *)expr, mangled_name, "AddressOfOp");     break;
        case V_SgPointerDerefExp:  mangleUnaryOp( (const SgUnaryOp *)expr, mangled_name, "PointerDerefExp"); break;
        case V_SgPlusPlusOp:       mangleUnaryOp( (const SgUnaryOp *)expr, mangled_name, "PlusPlusOp");      break;
        case V_SgMinusMinusOp:     mangleUnaryOp( (const SgUnaryOp *)expr, mangled_name, "MinusMinusOp");    break;

        case V_SgAddOp:            mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "AddOp");            break;
        case V_SgAndOp:            mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "AndOp");            break;
        case V_SgBitAndOp:         mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "BitAndOp");         break;
        case V_SgBitXorOp:         mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "BitXorOp");         break;
        case V_SgBitOrOp:          mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "BitOrOp");          break;
        case V_SgOrOp:             mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "OrOp");             break;
        case V_SgMultiplyOp:       mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "MultiplyOp");       break;
        case V_SgDivideOp:         mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "DivideOp");         break;
        case V_SgEqualityOp:       mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "EqualityOp");       break;
        case V_SgSubtractOp:       mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "SubtractOp");       break;
        case V_SgDotExp:           mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "DotExp");           break;
        case V_SgModOp:            mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "ModOp");            break;
        case V_SgArrowExp:         mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "ArrowExp");         break;
        case V_SgLessThanOp:       mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "LessThanOp");       break;
        case V_SgLessOrEqualOp:    mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "LessOrEqualOp");    break;
        case V_SgGreaterThanOp:    mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "GreaterThanOp");    break;
        case V_SgGreaterOrEqualOp: mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "GreaterOrEqualOp"); break;
        case V_SgAssignOp:         mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "AssignOp");         break;
        case V_SgPlusAssignOp:     mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "PlusAssignOp");     break;
        case V_SgMinusAssignOp:    mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "MinusAssignOp");    break;
        case V_SgAndAssignOp:      mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "AndAssignOp");      break;
        case V_SgXorAssignOp:      mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "XorAssignOp");      break;
        case V_SgIorAssignOp:      mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "IorAssignOp");      break;
        case V_SgMultAssignOp:     mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "MultAssignOp");     break;
        case V_SgDivAssignOp:      mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "DivAssignOp");      break;
        case V_SgLshiftAssignOp:   mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "LshiftAssignOp");   break;
        case V_SgRshiftAssignOp:   mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "RshiftAssignOp");   break;
        case V_SgModAssignOp:      mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "ModAssignOp");      break;
        case V_SgNotEqualOp:       mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "NotEqualOp");       break;
        case V_SgRshiftOp:         mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "RshiftOp");         break;
        case V_SgLshiftOp:         mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "LshiftOp");         break;
        case V_SgCommaOpExp:       mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "CommaOpExp");       break;
        case V_SgDotStarOp:        mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "DotStarOp");        break;
        case V_SgArrowStarOp:      mangleBinaryOp( (const SgBinaryOp *)expr, mangled_name, "ArrowStarOp");      break;

        case V_SgNoexceptOp: {
          const SgNoexceptOp* e = isSgNoexceptOp (expr);
          mangled_name << "_bNoexceptOp_" << mangleExpression (e->get_operand_expr()) << "_eNoexceptOp_";
          break;
        }
        case V_SgSizeOfOp: {
          const SgSizeOfOp* e = isSgSizeOfOp (expr);
          mangled_name << "_bSizeOfOp_";
          if (e->get_operand_expr() != NULL) {
            mangled_name << "_expr_" << mangleExpression (e->get_operand_expr());
          } else if (e->get_operand_type()) {
            mangled_name << "_type_" << e->get_operand_type()->get_mangled().getString();
          } else {
            ROSE_ABORT();
          }
          mangled_name << "_eSizeOfOp_";
          break;
        }
        case V_SgAlignOfOp: {
          const SgAlignOfOp* e = isSgAlignOfOp (expr);
          mangled_name << "_bAlignOfOp_";
          if (e->get_operand_expr() != NULL) {
            mangled_name << "_expr_" << mangleExpression (e->get_operand_expr());
          } else if (e->get_operand_type()) {
            mangled_name << "_type_" << e->get_operand_type()->get_mangled().getString();
          } else {
            ROSE_ABORT();
          }
          mangled_name << "_eAlignOfOp_";
          break;
        }

        case V_SgConditionalExp: {
          const SgConditionalExp* e = isSgConditionalExp (expr);
          mangled_name << "_bConditionalExp_";
          mangled_name << mangleExpression(e->get_conditional_exp());
          mangled_name << "__";
          mangled_name << mangleExpression(e->get_true_exp());
          mangled_name << "__";
          mangled_name << mangleExpression(e->get_false_exp());
          mangled_name << "_eConditionalExp_";
          break;
        }
        case V_SgFunctionCallExp: {
          const SgFunctionCallExp* e = isSgFunctionCallExp (expr);
          std::string mangledFn = mangleExpression (e->get_function());
          std::string mangledArgs = mangleExpression (e->get_args());

          mangled_name << "_bFunctionCallExp_" << mangledFn << "__" << mangledArgs << "_eFunctionCallExp_";
          break;
        }
        case V_SgConstructorInitializer: {
          const SgConstructorInitializer* e = isSgConstructorInitializer (expr);
          ROSE_ASSERT(e != NULL);

          mangled_name << "_bConstructorInitializer_";

          mangled_name << e->get_type()->get_mangled().getString();

          mangleExpression (e->get_args());

          mangled_name << "_eConstructorInitializer_";

          break;
        }
        case V_SgPntrArrRefExp: {
          const SgPntrArrRefExp * e = isSgPntrArrRefExp(expr);
          mangled_name << "_bPntrArrRefExp_" << mangleExpression (e->get_lhs_operand_i()) << "__" << mangleExpression (e->get_rhs_operand_i()) << "_ePntrArrRefExp_";
          break;
        }
        case V_SgTypeTraitBuiltinOperator: {
          const SgTypeTraitBuiltinOperator* e = isSgTypeTraitBuiltinOperator (expr);
          mangled_name << "_bTypeTraitBuiltinOperator_" << e->get_name().getString();
          mangled_name << "_bNodePtrList_";
          SgNodePtrList::const_iterator it;
          const SgNodePtrList & args = e->get_args();
          for (it = args.begin(); it != args.end(); it++) {
            if (it != args.begin())
              mangled_name << "__sep__";
            SgType * tit = isSgType(*it);
            SgExpression * eit = isSgExpression(*it);
            if (tit != NULL) {
              mangled_name << tit->get_mangled().getString();
            } else if (eit != NULL) {
              mangled_name << mangleExpression(eit);
            } else {
              ROSE_ABORT();
            }
          }
          mangled_name << "_eNodePtrList_";
          mangled_name << "_eTypeTraitBuiltinOperator_";
          break;
        }
        case V_SgExprListExp: {
          const SgExprListExp* e = isSgExprListExp (expr);
          mangled_name << "_bExprListExp_";
          SgExpressionPtrList::const_iterator it;
          const SgExpressionPtrList & args = e->get_expressions();
          for (it = args.begin(); it != args.end(); it++) {
            if (it != args.begin())
              mangled_name << "__sep__";
            mangled_name << mangleExpression (*it);
          }
          mangled_name << "_eExprListExp_";
          break;
        }
        case V_SgAggregateInitializer: {
          const SgAggregateInitializer* e = isSgAggregateInitializer (expr);
          mangled_name << "_bAggregateInitializer_" << mangleExpression (e->get_initializers()) << "_eAggregateInitializer_";
          break;
        }
        case V_SgBracedInitializer: {
          const SgBracedInitializer* e = isSgBracedInitializer (expr);
          mangled_name << "_bBracedInitializer_" << mangleExpression (e->get_initializers()) << "_eBracedInitializer_";
          break;
        }
        case V_SgAssignInitializer: {
          const SgAssignInitializer* e = isSgAssignInitializer (expr);
          mangled_name << "_bAssignInitializer_" << mangleExpression (e->get_operand_i()) << "_eAssignInitializer_";
          break;
        }
        case V_SgNewExp: {
          // FIXME ROSE-1783
          const SgNewExp* e = isSgNewExp (expr);
          mangled_name << "_bNewExpr_" << std::hex << e << "_eNewExpr_";
          break;
        }
        case V_SgDeleteExp: {
          // libc++ uses delete-expressions inside decltype in template arguments
          // (e.g. decltype(delete declval<_Tp>())); mangle the operand like a unary op.
          const SgDeleteExp* e = isSgDeleteExp (expr);
          mangled_name << (e->get_is_array() ? "_bDeleteArrayExp_" : "_bDeleteExp_");
          if (e->get_variable() != NULL)
            mangled_name << mangleExpression(e->get_variable());
          mangled_name << "_eDeleteExp_";
          break;
        }
        case V_SgFunctionParameterRefExp: {
          const SgFunctionParameterRefExp* e = isSgFunctionParameterRefExp (expr);
          mangled_name << "_bFunctionParameterRefExp_" << std::hex << e << "_eFunctionParameterRefExp_";
          break;
        }
        case V_SgAdaAttributeExp: {
          const SgAdaAttributeExp* e = isSgAdaAttributeExp (expr);
          mangled_name << "_badaAttributeExp_" << std::hex << e << "_eadaAttributeExp_";
          break;
        }
        case V_SgAdaOthersExp: {
          const SgAdaOthersExp* e = isSgAdaOthersExp(expr);
          mangled_name << "_badaOthersExp_" << std::hex << e << "_eadaOthersExp_";
          break;
        }
        case V_SgAdaRenamingRefExp: {
          const SgAdaRenamingRefExp* e = isSgAdaRenamingRefExp(expr);
          mangled_name << "_badaRenamingRefExp_" << std::hex << e << "_eadaRenamingRefExp_";
          break;
        }
        case V_SgAdaUnitRefExp: {
          const SgAdaUnitRefExp* e = isSgAdaUnitRefExp(expr);
          mangled_name << "_badaUnitRefExp_" << std::hex << e << "_eadaUnitRefExp_";
          break;
        }
        case V_SgRangeExp: {
          const SgRangeExp* e = isSgRangeExp(expr);
          mangled_name << "_bRangeExp_" << mangleExpression(e->get_start())
                       << ".." << mangleExpression(e->get_end())
                       << "_eRangeExp_";
          break;
        }
        // PP 2/13/22 added to support stop gap implementation for type from ada instantiations
        case V_SgTypeExpression: {
          const SgTypeExpression* e = isSgTypeExpression(expr);
          mangled_name << "_btypeExp_" << std::hex << e << "_etypeExp_";
          break;
        }
        case V_SgFoldExpression: {
          const SgFoldExpression * fold = isSgFoldExpression(expr);
          mangled_name << "_bfold_" << mangleExpression(fold->get_operands()) << "_op_" << fold->get_operator_token() << "_assoc_" << ( fold->get_is_left_associative() ? "left" : "right" ) << "_efold_";
          break;
        }
        default: {
          printf("In mangleExpression: Unsupported expression %p (%s)\n", expr, expr ? expr->class_name().c_str() : "");
          ROSE_ABORT();
        }
      }
    }

    return mangled_name.str ();
  }

bool
declarationHasTranslationUnitScope (const SgDeclarationStatement* decl)
   {
     ROSE_ASSERT(decl != NULL);
     SgNode *declParent = decl->get_parent();

  // DQ (9/8/2014): This now fails as a result of new template declaration handling.
     if (declParent == NULL)
        {
#if 0
          printf ("WARNING: In declarationHasTranslationUnitScope(): (declParent == NULL): decl = %p = %s (using non-defining declaration parent) \n",decl,decl->class_name().c_str());
#endif
          ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);
          declParent = decl->get_firstNondefiningDeclaration()->get_parent();
        }
     ROSE_ASSERT(declParent != NULL);

     VariantT declParentV = declParent->variantT();

     if (declParentV == V_SgGlobal || declParentV == V_SgNamespaceDefinitionStatement)
        {
       // If the declaration is static (in the C sense), it will have translation unit scope.
          if (decl->get_declarationModifier().get_storageModifier().isStatic())
               return true;

       // Likewise if the declaration is an inline function.
          if (const SgFunctionDeclaration *fnDecl = isSgFunctionDeclaration(decl))
             {
               if (fnDecl->get_functionModifier().isInline())
                    return true;
             }
        }

  // Likewise if the declaration is an anonymous namespace
     if (const SgNamespaceDeclarationStatement *nsDecl = isSgNamespaceDeclarationStatement(decl))
        {
          if (nsDecl->get_isUnnamedNamespace())
               return true;
        }

     return false;
   }

std::string mangleTranslationUnitQualifiers (const SgDeclarationStatement * decl)  {
  // Adds file-id prefix to mangled name of *static* declarations to prevent collision between translation units
  if (declarationHasTranslationUnitScope(decl) == true) {
    // Uses global (or enclosing) scope as reference because forward declaration can be compiler generated
    SgLocatedNode * lnode_ref = SageInterface::getGlobalScope(decl);
    if (lnode_ref == NULL) {
      lnode_ref = SageInterface::getEnclosingScope(const_cast<SgDeclarationStatement *>(decl));
    }
    ROSE_ASSERT(lnode_ref != NULL);

    // We must find an actual file-id as we want to specify the file where this declaration is static
    int file_id = lnode_ref->get_file_info()->get_file_id();
    switch (file_id) {
      case Sg_File_Info::COPY_FILE_ID:                                 return "_COPY_FILE_ID_";
      case Sg_File_Info::NULL_FILE_ID:                                 return "_NULL_FILE_ID_";
      case Sg_File_Info::TRANSFORMATION_FILE_ID:                       return "_TRANSFORMATION_FILE_ID_";
      case Sg_File_Info::COMPILER_GENERATED_FILE_ID:                   return "_COMPILER_GENERATED_FILE_ID_";
      case Sg_File_Info::COMPILER_GENERATED_MARKED_FOR_OUTPUT_FILE_ID: return "_COMPILER_GENERATED_MARKED_FOR_OUTPUT_FILE_ID_";
      case Sg_File_Info::BAD_FILE_ID:                                  return "_BAD_FILE_ID_";
      default: {
        ROSE_ASSERT(file_id >= 0);
        return "_file_id_" + StringUtility::numberToString(file_id) + "_";
      }
    }
  } else {
    return "";
  }
}
