/*
 * By Chunhua "Leo" Liao, 9/3/2014
 *
 * A tool to move declarations into innermost scopes when applicable.
 *  On the request from Jeff Keasler, we provide this transformation:
 *  For a declaration, find the innermost scope we can move it into, without breaking the code's original semantics
 *  For a single use place, move to the innermost scope.
 *  For the case of multiple uses, we may need to duplicate the declarations and move to two scopes if there is no variable reuse in between, 
 *    otherwise, we move the declaration into the innermost common scope of the multiple uses. 
 * ********************************************************************************************** 
 *  User instructions: 
 *
 * The translator accepts the following options: 
 *
 * -rose:merge_decl_assign  will merge the moved declaration with an immediately followed assignment. 
 *
 * -rose:aggressive  : turn on the aggressive mode, which will move declarations with initializers, and across loop boundaries.   
 *  A warning message will be sent out if the move crosses a loop boundary.  Without this option, the tool only moves a declaration 
 *  without an initializer to be safe.
 *
 *
 * -rose:debug, which is turned on by default in the testing.  
 *             Some dot graph files will be generated for scope trees of variables for debugging purpose.
 *
 * -rose:keep_going  will ignore assertions as much as possible (currently on skip the assertion on complex for loop initialization statement list).
 *   Without this option, the tool will stop on assertion failures. 
 *  
 * -rose:identity  will turn off any transformations and act like an identity translator. Useful for debugging purposes. 
 *
 * -rose:trans-tracking   will turn on the transformation tracking mode, showing the source statements of a move/merged declaration 
 *
 * ********************************************************************************************** 
 *  Internals: (For people who are interested in how this tool works internally) 
 *
 *  Data structure: we maintain a scope tree, in which each node is 
 *     a scope 1) defining or 2) use the variable, or 3) is live in between .
 *  Several implementation choices for storing the tree
 *  1) The scope tree reuses the AST. AST attribute is used to store extra information  
 *  2) A dedicated scope tree independent from AST. 
 *  3) Storing individual scope chains (paths) in the tree
 *     hard to maintain consistency if we trim paths, hard to debug.
 *
 *  For efficiency, we save only a single scope node  if there are multiple uses in the same scope. 
 *  Also for two use scopes with enclosing relationship, we only store the outer scope in the scope tree and trim the rest. 
 *
 *  Algorithm V1 :
 *    Save the scope of the declaration int DS (declaration statement)
 *
 *    Step 1: create a scope tree first, with trimming 
 *    Pre-order traversal to find all variable references to the declaration
 *    For each reference place
 *    {
 *       back track all its scopes until we reach DS
 *       if DS == US, nothing can be done to move the declaration, stop. 
 *       all scopes tracked are formed into a scope chain (a vector: US .. DS).
 *
 *       create a scope node of use type, called US.
 *       For all scopes in between, create nodes named intermediate scope or IS. 
 *       Add Ds-IS1-IS2..-US (reverse order of the scope chain) into the scope tree, consider consolidating overlapped scope chains;
 *          1) if an intermediate scope is equal to a use scope of another scope chain; stop add the rest of this chain. 
 *          2) if we are adding a use scope into the scope tree, but the same scope is already added by a intermediate scope from anther chain
 *             we mark the  existing scope node as a use scope, and remove its children from the scope tree.
 *    }
 *
 *    Step 2: find the scopes to move the declaration into
 *    find the innermost scope containing all paths to leaves: inner_scope: single parent, multiple children
 *    count the number of children of inner_scope: 
 *      if this is only one leaf: move the declaration to the innermost scope
 *      if there are two scopes: 
 *          not liveout for the variable in question?  Duplicate the declaration and move to each scope chain. 
 *          if yes liveout in between two scopes.  No duplication, move the declaration to inner_scope
 *
 *  Iterative moving process: 
 *
 *   For duplicated declarations inserted into inner scopes, we have to consider if they can be moved further.
 *     So we use a worklist to store all variable declarations to be considered.
 *     The worklist is initialized with original declarations in the code.
 *     New declarations are added into the worklist during the duplication/insertion process.
 *     The entire process terminate when the worklist becomes empty.      
 *
 * Algorithm V2: further optimization on top of v1
 *    for analysis-move, we find all bottom scopes at once and only do move once after that
 *    the scope trees are the new worklist now.
 *    This eliminates the intermediate moving of declarations and is much more efficient. 
 *    Details for how to do the iterative analysis can be found at comments for findFinalTargetScopes ()
 *
 *  //TODO optimize efficiency for multiple declarations
 * //TODO move to a separated source file or even namespace
*/

#include "rose.h"
#include <Rose/CommandLine.h>
#include "wholeAST_API.h"
#include "transformationTracking.h"
#include <iostream>
#include <queue> // used for a worklist of declarations to be moved 
#include <stack> // used for a worklist of declarations to be moved , first found, last processing
#include <boost/foreach.hpp>
#include <map> // used to store special var reference's scope
#include <Sawyer/CommandLine.h>

// another level of control over transformation tracking code
#define ENABLE_TRANS_TRACKING 1


static const char *purpose = "This tool moves variable declarations to their innermost possible scopes";
static const char *description =
    "This tool is designed to help parallelizing code by moving variable declarations "
    "into their innermost possible scopes (essentially making many of them private)."
    "As a result, less variables need to be shared.";

using namespace std;
using namespace Rose;
using namespace SageInterface;

class Scope_Node;


//---  a set of internal flags ----------------------
//! Enable debugging mode or not
bool debug = false;

//! Acting as an identity translator or not
bool isIdentity = false;

//! We now use improved algorithm v2, not exposed to users now
bool useAlgorithmV2 = true; 


//! If we keep track of transformation, mapping nodes back to original input nodes
bool transTracking = false;  

//! Users want to see the tool working by default
//! By default ASSERT should block the execution to find issues.
//! But some users want to keep the tool going even when some assertion fails. 
//!This is only supported by moveDeclarationToInnermostScope() and associated functions for now
bool tool_keep_going = false;

//! Users want to see conservative and aggressive moving
// Like any other compiler-based tools. We do things conservatively by default.
// Declarations with initializers will not be moved
// if it is set to false.  Declaration with initializers will be moved, 
// sends out warning if it crosses a loop boundaries in between. 
bool decl_mover_conservative = true;

//! If we further merge a naked variable declaration (without initialization) with a followed variable assignment within the same scope
bool merge_decl_assign = false; 

//! This function creates a description of command-line switches that are recognized by the too. 
Sawyer::CommandLine::SwitchGroup commandLineSwitches();

//--------------end of internal flags ----------------

// a global variable storing inserted declaration per input file processed
static  std::vector <SgVariableDeclaration* > inserted_decls; 
//! Move a declaration to a scope which is the closest to the declaration's use places. It may generate new declarations to be considered later on so worklist is used.
bool moveDeclarationToInnermostScope_v1(SgVariableDeclaration* decl, std::stack<SgVariableDeclaration *> &worklist, bool debug/*= false */);

//! An alternative algorithm: separating analysis from transformation into two phases. The move is final.
// Improved 2-step algorithm:
// Step 1: iterative subalgorithm to find the real bottom scopes 
// Step 2: copy & move source declaration to all the bottom scopes.
// return the final scopes accepting the moved declarations.
//std::vector <SgVariableDeclaration*> 
void moveDeclarationToInnermostScope_v2(SgVariableDeclaration* decl, std::vector <SgVariableDeclaration*>& my_inserted_decls, bool debug/*= false */);

// store scope of var ref used in array types. There is no easy way to find it by AST traversal.
//  e.g. double *buffer = new double[numItems] ; // numItems is referenced. But cannot get its scope. We store SgConstructorInitialzer for it to establish its scope info.
// TODO: report this issue to Dan.
static std::map <SgVarRefExp *, SgExpression*> specialVarRefScopeExp; 

// Keep track which scope tree has been considered as a new root for further possible decl moves. 
// This is used to avoid considering the same one multiple times, causing infinite looping.
static  std::map <Scope_Node*, bool> scopeTreeConsideredMap; 

//! Check if a statement is an assignment to a variable 
//TODO : move to SageInterface ?
static bool isAssignmentStmtOf(SgStatement * stmt, SgInitializedName* init_name)
{
  bool rt = false;

  ROSE_ASSERT (stmt != NULL);
  ROSE_ASSERT (init_name != NULL);
  if (SgExprStatement* exp_stmt = isSgExprStatement(stmt))
  {
    if (SgAssignOp * assign_op = isSgAssignOp (exp_stmt->get_expression()))
    {
       if (SgVarRefExp* var_exp = isSgVarRefExp (assign_op->get_lhs_operand()) )
      {
         if (var_exp->get_symbol()->get_declaration() == init_name)
          rt = true;
      }
    }
  }
  return rt; 
}

#if 0
/*
We cannot simply merge a declaration at a location A with an assignment at a location B.
This is essentially moving one statement across some code region so liveness or side effect analysis info. is needed for safety.
If the assignment's right hand operand uses any variable declared or written somewhere between A and B, 
we cannot move it up and merge it with the declaration.

int i=0
int tmp =10;
i = myarray[tmp]; // this cannot be moved up

Consider only right hand is not enough, the variable declared on left side must be in the set by default.

     int j;
   {
// initially built into scope tree, later trimmed. 
          j = 0;
        }
//this should trigger trimming the previous path 
      j = 2; // this cannot be moved up ??


The best solution is liveness analysis (or side effect analysis).
An approximation being used now is to not move when there is any references to variables used by the assignment (either lhs or rhs).

Pre-order iteration of decls is needed to check if it is mergeable: move assign up to merge with the declaration.
Example: if use reverse order: double abc will be checked first,  the last assignment cannot be moved up since rhs uses aa, which is defined in between.

Using pre-order : double aa is considered first, aa=a[i] is moved up.   Then double abc; abc= aa* aa; can be merged also.
        double aa;
        double abc; //
        aa = a[i];   
        abc = aa * aa;  //
*/
static  bool isUpwardMergeable (SgVariableDeclaration* decl, SgExprStatement* assign_stmt)
{
  bool rt = false;
  ROSE_ASSERT (decl != NULL);
  ROSE_ASSERT (assign_stmt != NULL);
  std::vector <SgStatement* > stmts_in_middle ;
  SgStatement* next_stmt = SageInterface::getNextStatement (decl);
  while (next_stmt != assign_stmt && next_stmt != NULL)
  {
    stmts_in_middle.push_back(next_stmt);
    next_stmt = SageInterface::getNextStatement (next_stmt);  
  }  

  if (next_stmt == NULL)
  {
    cout<<"Error in isMergeable (decl, assign_stmt): assign_stmt is not one of next statements for decl!"<<endl;
    ROSE_ASSERT (false);
  } 

  // collect all variables used in between
  // We must use SgInitializedName instead of SgVarRefExp since a declaration in between has only SgInitializedName. 
  std::set<SgVariableSymbol* > usedSymbolsInBetween; 

  std::vector <SgVarRefExp* > usedVarRefsInRhs;
  std::set<SgVariableSymbol* > usedSymbolsInAssignment; 

  std::set<SgVariableSymbol* > intersectSymbols; 

  // 1. Collect symbols used in between
  // TODO wrap into a SageInterface function: query both SgVarRefExp (including these used in types) and SgInitializedName
  for (size_t i=0; i< stmts_in_middle.size(); i++)
  {
    std::vector<SgVarRefExp*> varRefsInBetween; 
   // collect variable references first 
    SageInterface::collectVarRefs (stmts_in_middle[i], varRefsInBetween);
    // convert varRef to SgInitializedName 
     for (size_t j = 0; j< varRefsInBetween.size(); j++)
     {
        if (transTracking)
        { 
          cout<<"found referenced/used symbol in between "<< varRefsInBetween[j]->get_symbol()->get_name() <<endl;
        }
        usedSymbolsInBetween.insert (varRefsInBetween[j]->get_symbol());
     }

   // collect initialized name also for declarations
    if (SgVariableDeclaration* mid_decl = isSgVariableDeclaration (stmts_in_middle[i]))
    {
      if (transTracking)
        cout<<"found declared /used symbol in between "<< SageInterface::getFirstVarSym(mid_decl)->get_name() <<endl;
      usedSymbolsInBetween.insert(SageInterface::getFirstVarSym(mid_decl));
    }
  }

  // 2. collect symbols used by assign_op's rhs and lhs (must consider both sides!)
  SageInterface::collectVarRefs ( isSgAssignOp(assign_stmt->get_expression())->get_rhs_operand(), usedVarRefsInRhs);
  for (size_t k=0; k< usedVarRefsInRhs.size(); k++)
  {
      if (transTracking)
        cout<< "found used in assignment rhs" << usedVarRefsInRhs[k]->get_symbol()-> get_name()<<endl;
    usedSymbolsInAssignment.insert (usedVarRefsInRhs[k]->get_symbol());
  }
  if (transTracking)
        cout<< "found used in assignment lhs" << SageInterface::getFirstVarSym (decl)-> get_name()<<endl;
  usedSymbolsInAssignment.insert (SageInterface::getFirstVarSym (decl));

 // intersection is not NULL, cannot merge or move across the area using the variable
  set_intersection(usedSymbolsInBetween.begin(), usedSymbolsInBetween.end(), usedSymbolsInAssignment.begin(), usedSymbolsInAssignment.end(),
                    inserter (intersectSymbols, intersectSymbols.begin()));
  if (intersectSymbols.size() == 0 )
    rt = true;
  if (transTracking)
  {
    cout<< "isUpwardMergeable () returns "<< rt<< "----------------" <<endl;
  }
 
  return rt; 
}
#endif

// Check if a decl can be moved downwards to an assignment
/*
current algorithm: scan statements in between, if declared variable is not referenced, we can do the downwards merge
    double dx2; // decl
    double dy1 = yy[i2] - y[i2];
    double dy2;
    double a1;
    double a2;
    dx2 = x[i1] - x[i2]; // assignment
 */
static  bool isDownwardMergeable (SgVariableDeclaration* decl, SgExprStatement* assign_stmt)
{
  bool rt = false;
  ROSE_ASSERT (decl != NULL);
  ROSE_ASSERT (assign_stmt != NULL);
  std::vector <SgStatement* > stmts_in_middle ;
  SgStatement* next_stmt = SageInterface::getNextStatement (decl);
  while (next_stmt != assign_stmt && next_stmt != NULL)
  {
    stmts_in_middle.push_back(next_stmt);
    next_stmt = SageInterface::getNextStatement (next_stmt);  
  }  

  if (next_stmt == NULL)
  {
    cout<<"Error in isDownwardMergeable (decl, assign_stmt): assign_stmt is not one of next statements for decl!"<<endl;
    ROSE_ASSERT (false);
  } 

  // collect all variables used in between
  // We must use SgInitializedName instead of SgVarRefExp since a declaration in between has only SgInitializedName. 
  std::set<SgVariableSymbol* > usedSymbolsInBetween; 

  // 1. Collect symbols used in between
  // TODO wrap into a SageInterface function: query both SgVarRefExp (including these used in types) and SgInitializedName
  for (size_t i=0; i< stmts_in_middle.size(); i++)
  {
    std::vector<SgVarRefExp*> varRefsInBetween; 
   // collect variable references first 
    SageInterface::collectVarRefs (stmts_in_middle[i], varRefsInBetween);
    // convert varRef to SgInitializedName 
     for (size_t j = 0; j< varRefsInBetween.size(); j++)
     {
        if (transTracking)
        { 
          cout<<"found referenced/used symbol in between "<< varRefsInBetween[j]->get_symbol()->get_name() <<endl;
        }
        usedSymbolsInBetween.insert (varRefsInBetween[j]->get_symbol());
     }

   // collect initialized name also for declarations
    if (SgVariableDeclaration* mid_decl = isSgVariableDeclaration (stmts_in_middle[i]))
    {
      if (transTracking)
        cout<<"found declared /used symbol in between "<< SageInterface::getFirstVarSym(mid_decl)->get_name() <<endl;
      usedSymbolsInBetween.insert(SageInterface::getFirstVarSym(mid_decl));
    }
  }

  SgVariableSymbol * declared_sym = SageInterface::getFirstVarSym (decl);

  if (usedSymbolsInBetween.find(declared_sym) ==usedSymbolsInBetween.end()) // not found? We can safely do downwards merge then
    rt = true;
  if (transTracking)
  {
    cout<< "isDownwardMergeable () returns "<< rt<< "----------------" <<endl;
  }
 
  return rt; 
}


// for each decl in the declaration vector, 
// if it has no initialization, we find the first followed assignment within the same scope and merge the assignment into the decl as an initializer
static void collectiveMergeDeclarationAndAssignment (std::vector <SgVariableDeclaration*> decls)
{
  // declarations which are not UpwardMergeable with its assignment
//  std::vector <SgVariableDeclaration*> remaining_decls; 
  
  // Must not separate analysis and transformation sine the success of next stmt relies on previous merge
  // pre-order iteration to check if it is mergeable: move assign up to merge with the declaration.

#if 0
  //for (size_t i = 0; i< decls.size(); i++)
  // The top level traversal is reverse-order of preorder
  // However, to support the isUpwardMergeable()'s move-up analysis (move assign up to the decl)
  // we must scan candidate decls using pre-order, or multiple decls case will not work properly!

  for (int i = decls.size()-1; i>=0; i--) // -- case will cause overflow for size_t type, must not use size_t!!
  {
    SgVariableDeclaration* current_decl = decls[i];
    ROSE_ASSERT (current_decl != NULL);
    SgInitializedName* init_name = SageInterface::getFirstInitializedName (current_decl);
    ROSE_ASSERT (init_name!= NULL);
    SgInitializer * initor =  init_name->get_initptr();
    if (initor == NULL)
    { 
      SgStatement* next_stmt = SageInterface::getNextStatement(current_decl);

      while (next_stmt)
      {
        if (isAssignmentStmtOf (next_stmt, init_name) )
        {  
          if (isUpwardMergeable (current_decl, isSgExprStatement (next_stmt)))
          {
            //SageInterface::mergeDeclarationAndAssignment (current_decl, isSgExprStatement (next_stmt));
            SageInterface::mergeAssignmentWithDeclaration (isSgExprStatement (next_stmt), current_decl);
#if ENABLE_TRANS_TRACKING
            if (transTracking)
            {
              // No need to patch up IDs for a merge transformation
              // directly record input node 
              TransformationTracking::addInputNode (current_decl, next_stmt);
            }
#endif              
          } // end if Mergeable
          else
          {
//            remaining_decls.push_back(current_decl);
          }
          // mergeable or , we stop going to next stmt
          next_stmt = NULL; // We stop when the first match is found, we also stop if the first matching assign is not mergeable
        } 
        else
          next_stmt = SageInterface::getNextStatement(next_stmt);
      } 

    } // end if null initializer
  } // end for
#endif

// 
  // Consider downward merge for the remaining declarations
  // Does the order matter??
  for (int i = decls.size()-1; i>=0; i--) // i-- will cause overflow for size_t type, must not use size_t!!
  {
    SgVariableDeclaration* current_decl = decls[i];
    ROSE_ASSERT (current_decl != NULL);
    SgInitializedName* init_name = SageInterface::getFirstInitializedName (current_decl);
    ROSE_ASSERT (init_name!= NULL);
    SgInitializer * initor =  init_name->get_initptr();
    if (initor == NULL)
    { 
      SgStatement* next_stmt = SageInterface::getNextStatement(current_decl);

      while (next_stmt)
      {
        if (isAssignmentStmtOf (next_stmt, init_name) )
        {  
          if (isDownwardMergeable(current_decl, isSgExprStatement (next_stmt)))
          {
            SageInterface::mergeDeclarationWithAssignment(current_decl, isSgExprStatement (next_stmt));
#if ENABLE_TRANS_TRACKING
            if (transTracking)
            {
              // No need to patch up IDs for a merge transformation
              // directly record input node 
              TransformationTracking::addInputNode (current_decl, next_stmt);
            }
#endif              
          } // end if Mergeable
          // mergeable or not , we stop going to next stmt
          next_stmt = NULL; // We stop when the first match is found, we also stop if the first matching assign is not mergeable
        } 
        else
          next_stmt = SageInterface::getNextStatement(next_stmt);
      } 
    } // end if null initializer
  } // end for
}


class visitorTraversal : public AstSimpleProcessing
{
  protected:
    void virtual visit (SgNode* n)
    {
#if 0
      printf ("In visitorTraversal::visit(): n = %p = %s \n",n,n->class_name().c_str());
#endif
      //      if (isSgFunctionDeclaration(n)!=NULL)
      //      This will match SgTemplateInstantiationFunctionDecl, which is not wanted.
      if (n->variantT() == V_SgFunctionDeclaration || n->variantT() == V_SgMemberFunctionDeclaration)
      {
	ROSE_ASSERT (n->variantT() != V_SgTemplateInstantiationFunctionDecl);
	SgFunctionDeclaration* func = isSgFunctionDeclaration(n);  
	ROSE_ASSERT(func != NULL);
	if (func->get_definition() == NULL) return;

	// TODO: skip things from headers. 
	// skip compiler generated codes, mostly from template headers
	if (func->get_file_info()->isCompilerGenerated())
	{
	  return;
	}

	SgBasicBlock* body = func->get_definition()->get_body();
	if (body == NULL) return; 
	// Prepare the function body: ensure body basic block for while, for, if, etc.
	SageInterface::changeAllBodiesToBlocks (body, false);

	Rose_STL_Container<SgNode*> var_decls= NodeQuery::querySubTree(body,V_SgVariableDeclaration);
	if (debug )
	  cout<<"Number of declarations to be considered = "<<var_decls.size()<<endl;

//	std::queue<SgVariableDeclaration* > worklist;
	std::stack<SgVariableDeclaration* > worklist;

	for (size_t i=0; i< var_decls.size(); i++)
        //Liao 2015/11/2
        //reverse the order for better result:  int i; int j;  order will be preserved after moving both of them to the new location
	//for (size_t i=var_decls.size()-1; i>=0; i--)
	{
	  SgVariableDeclaration* decl = isSgVariableDeclaration(var_decls[i]);
	  ROSE_ASSERT(decl!= NULL);
	  // skip compiler generated (frontend) declarations
	  if (decl->get_file_info()->isCompilerGenerated())
	    continue; 
	  worklist.push(decl);
	}

	// using a worklist instead of a fixed vector, since we will iteratively consider declarations added in the process.
	// e.g. we insert a declaration into the true/false bodies of if statement if a target scope is a if statement.
	// These two inserted declarations will be further considered.
	while (!worklist.empty())
	{    
	  SgVariableDeclaration* decl = isSgVariableDeclaration(worklist.top());
	  ROSE_ASSERT(decl!= NULL);
	  worklist.pop();

	  //bool result=false;
	  if (SageInterface::isStatic(decl))
	  {
	    if (debug)
	      cout<<"skipping a static variable declaration .."<<endl;
	  }
	  else
	  {
	    bool null_initializer = false;
	    SgInitializedName* init_name = SageInterface::getFirstInitializedName (decl);
	    ROSE_ASSERT (init_name!= NULL);
	    SgInitializer * initor =  init_name->get_initptr();
	    if (initor == NULL) 
	      null_initializer = true;
#if 0
	    else
	    {
	      SgAssignInitializer* assign_initor = isSgAssignInitializer (initor);
	      if (assign_initor != NULL)
	      {
		if (isSgValueExp(assign_initor->get_operand()))
		  null_or_literal_initializer = true;
	      }
	    } 
#endif
	    // conservative mode:  Only move declarations with no initializer
	    if (decl_mover_conservative)
	    {
	      if (debug)
		cout<<"Consiering conservative moving for decl: "<<decl->get_file_info()->get_line() <<endl;
	      if (null_initializer)   
	      {
#if 0
                printf ("In visitorTraversal::visit(): n = %p = %s (conservative moving: useAlgorithmV2 = %s) \n",n,n->class_name().c_str(),useAlgorithmV2 ? "true" : "false");
#endif
		if (useAlgorithmV2)
		  moveDeclarationToInnermostScope_v2(decl, inserted_decls, debug);
		else
		  moveDeclarationToInnermostScope_v1(decl, worklist, debug);
#if 0
                printf ("DONE: In visitorTraversal::visit(): n = %p = %s (conservative moving: useAlgorithmV2 = %s) \n",n,n->class_name().c_str(),useAlgorithmV2 ? "true" : "false");
#endif
	      }
	      else
	      {
		if (debug)
		  cout<<"Skipping a declaration since it has initializer .."<<endl;
	      }
	    }
	    else // aggressive move
	    {

	      if (debug)
		cout<<"Using aggressive moving for decl .."<<endl;
	      if (useAlgorithmV2)
		moveDeclarationToInnermostScope_v2(decl, inserted_decls, debug);
	      else
		moveDeclarationToInnermostScope_v1(decl, worklist, debug);
	    }

	  } // end if non-static 
	} // end while (worklist)
      } // end if function
    } // end visit()
};

// TOO1 (2014/12/05): Temporarily added this to support keep-going in rose-sh.
std::vector<std::string>
GetSourceFilenamesFromCommandline(const std::vector<std::string>& argv)
{
  std::vector<std::string> filenames =
      CommandlineProcessing::generateSourceFilenames(argv, false);
  return filenames;
}

//! Initialize the switch group and its switches.
Sawyer::CommandLine::SwitchGroup commandLineSwitches() {
  using namespace Sawyer::CommandLine;

  SwitchGroup switches("The move tool switches");
  switches.doc("These switches control the move tool. ");
  switches.name("rose"); // be backwards compatible with -rose:optionX 

  switches.insert(Switch("debug")
      .intrinsicValue(true, debug)
      .doc("Enable the debugging mode."));

  switches.insert(Switch("identity")
      .intrinsicValue(true, isIdentity)
      .doc("Enable acting like an identity translator, not doing any transformation at all."));

  switches.insert(Switch("trans-tracking")
      .intrinsicValue(true, transTracking)
      .doc("Enable tracking of transformation steps."));

  switches.insert(Switch("keep_going")   //TODO: how to keep this option to be used by later phases?
      .intrinsicValue(true, tool_keep_going)
      .doc("Allow the tool to proceed without being stopped by assertions."));

  switches.insert(Switch("aggressive")
      .intrinsicValue(true, decl_mover_conservative)
      .doc("Enable aggressive mode: declarations with initializers will be moved."));

  switches.insert(Switch("merge_decl_assign")
      .intrinsicValue(true, merge_decl_assign)
      .doc("After the move, further merge the moved naked variable declaration (without initialization) with a followed variable assignment."));

  return switches;
}

//! Command line processing
static std::vector<std::string> parseCommandLine(std::vector< std::string > & argvList) // this will cause trouble, not sure why.
//static std::vector<std::string> parseCommandLine(int argc, char* argv[]) 
{
  using namespace Sawyer::CommandLine;
  Parser p = Rose::CommandLine::createEmptyParserStage(purpose, description);
  p.doc("Synopsis", "@prop{programName} @v{switches} @v{files}...");
  p.longPrefix("-");

// initialize generic Sawyer switches: assertion, logging, threads, etc.
  p.with(Rose::CommandLine::genericSwitches()); 

// initialize tool switches
  p.with(commandLineSwitches());  

// switches to be passed to ROSE, somehow not working yet. TODO
  SwitchGroup tool("ROSE's built-in switches");
  // We want the "--rose:help" switch to appear in the Sawyer documentation but we have to pass it to the next stage also. We
  // could do this two different ways. 
  // 1. The older way (that still works) is to have Sawyer process the switch and then we
  // prepend it into the returned vector for processing by later stages.  

#if 0
  // 2. The newer way is to set the switch's "skipping"
  // property that causes Sawyer to treat it as a skipped (unrecognized) switch.  We'll use SKIP_STRONG, but SKIP_WEAK is
  // sort of a cross between Sawyer recognizing it and not recognizing it.
  tool.insert(Switch("rose:help")
      .skipping(SKIP_STRONG)                  // appears in documentation and is parsed, but treated as skipped
      .doc("Show the ROSE switch documentation.")
      .action(showHelpAndExit(0)) );
#else
  bool showRoseHelp = false;
  tool.insert(Switch("rose:help")
             .longPrefix("-")
             .intrinsicValue(true, showRoseHelp)
             .doc("Show the old-style ROSE help.")); 

#endif


  // Copy this tool's switches into the parser.
  p.with(tool);

  // Parse the command-line, stopping at the first "--" or positional argument. Return the unparsed stuff so it can be passed
  // to the next stage.  ROSE's frontend expects arg[0] to be the name of the command, which Sawyer has already processed, so
  // we need to add it back again.
//  std::vector<std::string> remainingArgs = p.parse(argc, argv).apply().unparsedArgs(true);
//  remainingArgs.insert(remainingArgs.begin(), argv[0]);
  std::vector<std::string> remainingArgs = p.parse(argvList).apply().unparsedArgs(true);
//  remainingArgs.insert(remainingArgs.begin(), argvList[0]); // somehow this is not needed if I use the vector argument version of parse()

  if (showRoseHelp)
    SgFile::usage(0); 

#if 0 // DEBUGGING [Robb P Matzke 2016-09-27]
  std::cerr <<"These are the arguments after parsing with Sawyer:\n";
  BOOST_FOREACH (const std::string &s, remainingArgs)
    std::cerr <<"    \"" <<s <<"\"\n";
#endif

  return remainingArgs;
}

int main(int argc, char * argv[])
{
  ROSE_INITIALIZE;

  //! Command line process begin --------------------------
  vector <string> argvList (argv, argv+argc)  ; 
  //argvList =  parseCommandLine (argc, argv);
  argvList =  parseCommandLine (argvList);
  // TOO1 (2014/12/05): Temporarily added this to support keep-going in rose-sh.
  if (CommandlineProcessing::isOption (argvList,"--list-filenames","",true))
  {
    std::vector<std::string> filenames =
      GetSourceFilenamesFromCommandline(
          std::vector<std::string>(argv, argv + argc));
    BOOST_FOREACH(std::string filename, filenames)
    {
      std::cout << filename << std::endl;
    }
    return 0;
  }

  // -------------end of command line processing -------------------

  SgProject *project = frontend (argvList);

  // DQ (11/20/2015): AST consistency tests (optional for users, but this enforces more of our tests).
  // I have added this to detect a SgTemplateClassDefinition that is being visited twice.
  AstTests::runAllTests(project);

#if ENABLE_TRANS_TRACKING
  // assign unique ID's for all nodes
  if (transTracking)
    TransformationTracking::registerAstSubtreeIds (project);  
#endif
  // DQ (12/11/2014): Added output of graph after transformations.
  if (SgProject::get_verbose() > 0)
  {
#if 0
    printf ("Generating a DOT graph of the AST \n");
    generateDOTforMultipleFile(*project);
#endif
  }

#if 0
  // DQ (10/13/2015): debugging the token-based unparsing (setting SgForStatement as modified.
  printf ("NOTE: Setting verbose to value 3 to trigger debugging after the AST is built \n");
  SgProject::set_verbose(3);
#endif

#if 1
  // DQ (10/6/2015): Remove transformation for debugging token-unparsing.
  if (!isIdentity)
  {
    SgFilePtrList file_ptr_list = project->get_fileList();
    visitorTraversal exampleTraversal;
    for (size_t i = 0; i<file_ptr_list.size(); i++)
    {
      SgFile* cur_file = file_ptr_list[i];
      SgSourceFile* s_file = isSgSourceFile(cur_file);
      if (s_file != NULL)
      {
        inserted_decls.clear(); // For each file, reset this.
        // exampleTraversal.traverseInputFiles(project,preorder);
        exampleTraversal.traverseWithinFile(s_file, preorder);
        if (inserted_decls.size()>0 && merge_decl_assign)
        { 
          if (transTracking)
            cout<<"Begin merging declarations # "<<inserted_decls.size()<<endl;
          collectiveMergeDeclarationAndAssignment (inserted_decls);
        }
      }
    }
    string filename= SageInterface::generateProjectName(project);
#if 0
    // DQ (1/14/2015): This is a problem since it causes us to run out of disk space on large projects.
    generateDOTforMultipleFile(*project);
#endif
  }
#endif

  // string filename= SageInterface::generateProjectName(project);
  // generateDOTforMultipleFile(*project);

  if (SgProject::get_verbose() > 0)
  {
#if 1
    printf ("Generating a WHOLE AST DOT graph \n");
    generateDOTforMultipleFile(*project);
#endif
    // Output an optional graph of the AST (the whole graph, of bounded complexity, when active)
    const int MAX_NUMBER_OF_IR_NODES_TO_GRAPH_FOR_WHOLE_GRAPH = 10000;
    generateAstGraph(project,MAX_NUMBER_OF_IR_NODES_TO_GRAPH_FOR_WHOLE_GRAPH,"");
  }

#if 0
  // DQ (10/6/2015): Remove transformation for debugging token-unparsing.
  printf ("Calling cleanupNontransformedBasicBlockNode() \n");
#endif
#if 1
  // DQ (1/18/2015): Denormalize some specific normalized bodies as a test.
  SageInterface::cleanupNontransformedBasicBlockNode();
  // printf ("DONE: Calling cleanupNontransformedBasicBlockNode() \n");
#endif

#if ENABLE_TRANS_TRACKING
  if (transTracking)
  { 
    std::map<AST_NODE_ID, std::set<AST_NODE_ID> >::iterator iter;
    for (iter = TransformationTracking::inputIDs.begin(); iter != TransformationTracking::inputIDs.end(); iter++)
    {
      std::set<AST_NODE_ID> ids = (*iter).second; 
      if (ids.size()>0)
      {
        string src_comment = "Transformation generated based on ";
        cout<<"Found a node with IR mapping info"<<endl;
        SgNode* affected_node = TransformationTracking::getNode((*iter).first);
#if 0
        // DQ (11/2/2015): Save and set to high value to trigger the verbose level to control tracking of isModified flag.
        int verbose_level = SgProject::get_verbose();
        SgProject::set_verbose(3);
#endif
        cout<<isSgLocatedNode(affected_node)->unparseToString()<<endl;
        cout<<"-- with input nodes ----------"<<endl;
        std::set<AST_NODE_ID>::iterator iditer;
        for(iditer = ids.begin(); iditer != ids.end(); iditer ++)
        {
          SgNode* input_node = TransformationTracking::getNode((*iditer));
          SgLocatedNode* lnode = isSgLocatedNode(input_node); 
          cout<<lnode->unparseToString()<<endl;  //TODO this function has unexpected side effects impacting token-based unparsing
          cout<<"//Transformation generated based on line #"<< lnode->get_file_info()->get_line() <<endl;
          src_comment += " line # " + StringUtility::numberToString(lnode->get_file_info()->get_line());
        }
#if 0
        // DQ (11/2/2015): Reset the verbose level to trigger tracking of isModified flag.
        SgProject::set_verbose(verbose_level);
#endif
        src_comment +="\n";
        //        SgStatement* enclosing_stmt = getEnclosingStatement(affected_node);
        cout<<src_comment<<endl;
        //TODO: turn this on and update the reference results
        //        attachComment (enclosing_stmt, src_comment);
      } // end if ids.size() >0
    }  // end for inputIDs
  } // end if transTracking 
#endif

#if 0
  // DQ (10/13/2015): debugging the token-based unparsing (setting SgForStatement as modified.
  printf ("NOTE: Setting verbose to value 3 to trigger debugging after the AST is built and before running AstTests::runAllTests() \n");
  SgProject::set_verbose(3);
#endif

  // run all tests
  AstTests::runAllTests(project);

#if 0
  // DQ (10/13/2015): debugging the token-based unparsing (setting SgForStatement as modified.
  printf ("NOTE: Setting verbose to value 3 to trigger debugging before unparsing of AST \n");
  SgProject::set_verbose(3);
#endif

  return backend(project);
}


//==================================================================================

// Three types of scope for a variable access
// 1. Variable is being declared.
// 2. Variable is being used: read or written 
// 3. Not either of the above cases, just a scope in between them. 
enum ScopeType {s_decl, s_intermediate, s_use};
class Scope_Node {
  public: 
    Scope_Node (SgScopeStatement* s, ScopeType t):scope(s), s_type(t) {};
    //TODO: protect members behind access functions
    // scope information
    SgScopeStatement* scope; 
    ScopeType s_type;
    int depth; // the depth in the tree (path or chain) starting from 0

    // for tree information
    Scope_Node* parent; // point to the parent scope in the chain
    std::vector < Scope_Node* > children ; // point to children scopes 
  
    //print the subtree rooted at this node to a dot file named as filename.dot
    void printToDot(std::string filename);
    std::string prettyPrint();
    //! Depth-first traverse to get all nodes within the current subtree
    void traverse_node(std::vector<Scope_Node* >& allnodes);
    //! Delete all children, grandchildren, etc.
    void deep_delete_children();
    //! Starting from the root, find the first node which has more than one children.
    // This is useful to identify the innermost common scope of all leaf scopes.
    Scope_Node * findFirstBranchNode();
    //~Scope_Node () {  deep_delete_children(); }

   private: 
      //! Recursive traverse the current subtree and write dot file information
      void traverse_write (Scope_Node* n, std::ofstream & dotfile);
      std::string getLineNumberStr(){ int lineno = scope->get_file_info()->get_line(); return StringUtility::numberToString(lineno); } ;
      // Dot graph Node Id: unique memory address, prepend with "n_".
      std::string getDotNodeId() { return "n_"+StringUtility::numberToString(scope); };
      std::string getScopeTypeStr() 
      { string rt;  
        if (s_type == s_decl) rt = "s_decl"; 
        else if (s_type == s_intermediate) rt = "s_intermediate";
        else if (s_type == s_use) rt = "s_use";
        else ROSE_ASSERT (false);
        return rt; }
  };

// Topdown traverse a tree to find the first node with multiple children
// Intuitively, the innermost common scope for a variable.
// ----------- No adjustment any more ---------------
// However, we have to adjust a few special cases: 
// For example: if-stmt case
//  A variable is used in both true and false body.
 // Naive analysis will find if-stmt is the inner-most common scope.
//  But we cannot really move a declaration to the if-stmt scope (condition)
Scope_Node*  Scope_Node::findFirstBranchNode()
{
  Scope_Node* first_branch_node = this; 
  // three cases: 0 children, 1 children, >1 children
  while (first_branch_node->children.size()==1)
    first_branch_node = first_branch_node->children[0];

#if 0 // this adjustment should not be done until we figure out if the variable can be
      // moved downward into the two branch scopes or not.
      // We only adjust if we cannot move downward further,but trying to move var decl to if-stmt.
      // With this consideration, we do this adjust later after considering liveness between mutliple scopes.
      //
  // Adjust for if-stmt: special adjustment
  // switch stmt needs not to be adjusted since there is a middle scope as the innermost scope
  // if a variable is used in multiple case: scopes. 
   if (isSgIfStmt (first_branch_node->scope))
      first_branch_node = first_branch_node->parent;
#endif
  // now the node must has either 0 or >1 children.
  return first_branch_node; 
}
  
//TODO make this a member of Scope_Node.  
std::map <SgScopeStatement* , Scope_Node*>  ScopeTreeMap; // quick query if a scope is in the scope tree, temp variable shared by all trees.
void Scope_Node::deep_delete_children()
{
  std::vector <Scope_Node*> allnodes;
  // TODO better way is to use width-first traversal and delete from the bottom to the top
  this->traverse_node (allnodes);
  // allnodes[0] is the root node itself, we keep it.
  // reverse order of preorder to delete things?
  //for (size_t i =1; i<allnodes.size(); i++) 
  for (size_t i =allnodes.size()-1 ; i>0; i--) 
  {
    Scope_Node* child = allnodes[i];
    // mark the associated scope as not in the scope tree
    ScopeTreeMap[child->scope] = NULL;
    // TODO: currently a workaround for multiple delete due to recursive call issue. 
    if (child != NULL)
    {
      delete child;
      allnodes[i] = NULL;
    }
  }
  // no children for the current node
  children.clear(); 
}

void Scope_Node::traverse_node(std::vector<Scope_Node* >& allnodes)
{
  // action on current node, save to the vector
  ASSERT_this(); // we should not push NULL pointers
  allnodes.push_back(this);
  std::vector < Scope_Node* > children = this->children;
  for (size_t i=0; i<children.size(); i++) 
  {
     Scope_Node* child = children[i];
     if (child != NULL) // we may delete child nodes and have NULL pointers
       child->traverse_node(allnodes);
  }
}

std::string Scope_Node::prettyPrint()
{
  int lineno = scope->get_file_info()->get_line();
  return StringUtility::numberToString(scope)+"@"+StringUtility::numberToString(lineno);
}

void Scope_Node::printToDot (std::string filename)
{
  string full_filename = filename + ".dot";
  ofstream dotfile (full_filename.c_str());
  dotfile <<"digraph scopetree{"<<endl;
  traverse_write (this, dotfile);
  dotfile <<"}"<<endl;
}

void Scope_Node::traverse_write(Scope_Node* n,std::ofstream & dotfile)
{
  std::vector < Scope_Node* > children = n->children;
  // must output both node and edge lines: here is the node
  dotfile<<n->getDotNodeId()<<"[label=\""<< StringUtility::numberToString(n->scope)
         <<"\n"<<n->scope->class_name()
         <<"\nLine="<<n->getLineNumberStr()
         <<" Depth="<<StringUtility::numberToString(n->depth)
         <<"\nType="<<n->getScopeTypeStr()<<"\"];"<<endl;
  for (size_t i=0; i<children.size(); i++)
  {
    // Here is the edge
    dotfile<<n->getDotNodeId()<<"->"<<children[i]->getDotNodeId()<<";"<<endl;
    traverse_write (children[i], dotfile);
  }
}

// The default NodeQuery::querySubTree() will miss variables referenced in array type's index list.
// e.g. double *buffer = new double[numItems] ; 
// TODO: fix the root cause of NodeQuery::querySubTree() 
static  int collectUpArrayTypeIndexVariables (SgScopeStatement* scope, Rose_STL_Container<SgNode*> & currentVarRefList) 
{
  int rt = 0;
  ROSE_ASSERT (scope != NULL);
  Rose_STL_Container<SgNode*> constructorList= NodeQuery::querySubTree(scope, V_SgConstructorInitializer);
  for (size_t i =0; i< constructorList.size(); i++)
  {
    SgConstructorInitializer * c_init = isSgConstructorInitializer (constructorList[i]);
    if (SgArrayType* a_type = isSgArrayType(c_init->get_expression_type()))
    {
      Rose_STL_Container<SgNode*> varList = NodeQuery::querySubTree (a_type->get_index(),V_SgVarRefExp);
      for (size_t j =0 ; j< varList.size(); j++)
      {
	SgVarRefExp* var_exp =  isSgVarRefExp(varList[j]) ;
	if (debug)
	{
	  cout<<"Found a var ref in array type:"<<var_exp->get_symbol()->get_name()<<endl;
	}
	currentVarRefList.push_back(var_exp);
	specialVarRefScopeExp[var_exp] = c_init ;
	rt ++;
      }
    }
  }
  return rt; 
}
 
//! A helper function to skip some scopes, such as while stmt scope: special adjustment.
// used for a variable showing up in condition expression of some statement. 
// We return a grand parent scope for those variables. 
/*
Differentiate two situations when handling special scopes
1:  when obtaining a scope for a variable in the condition expression. The scope should be adjusted since we cannot insert things to the conditions (??)
2:  when deciding on if we can insert a decl to the special scopes:  This has two further situations: 
   2.1  the special scope is a final bottom scope . We have backtrack to its parent scope  
   2.2  the special scope is an intermediate scope.  we can still put it into the worklist since the iterative algorithm v 2

To simplify the problem, we consider them all in one pace: situation 2. hasUnsupportedScope() helps decide how to put scopes to the worklist.
We don't do anything special for situation 1.

TODO: Alternatively, we screen out bad scopes as early and possible in this function.
     Later algorithm2 will not be bothered. 

 *We don't try to merge a variable decl into the conditional of a while statement
 * The reason is that often the declaration has an initializer , which must be preserved.
 * The conditional contain the use of the declared variable, which usually cannot be merged with the declaration.
 * e.g.  int yy = 10; while (yy< 100) { yy ++; ... }
 * Another tricky thing is even though yy shows up in both condition and while-body, 
 * the scope tree will only show while-stmt as the single scope node since the body scope is shadowed. 
 * Note: the scope of the condition is considered to be the while-stmt.
 * TODO in a rare case like   int yy = 10; while (yy) { } we can merge the declaration and condition together.
 * */
static SgScopeStatement * getAdjustedScope(SgNode* n)
{
  ROSE_ASSERT (n!= NULL); 
  // link to its SgConstructorInitializer for  variable X in "= new double[X]". 
  // TODO: report this to Dan to have a better AST to track this scope down.
  if (isSgVarRefExp(n))
    if (specialVarRefScopeExp[isSgVarRefExp(n)])
      n = specialVarRefScopeExp[isSgVarRefExp(n)] ; 
  SgScopeStatement* result =  SageInterface::getScope (n);
#if 0 // for algorithm 2, we check these special scopes anyway: bottom ? backtrack,   not bottom? add then into the worklist for further consideration
  if (isSgWhileStmt (result) || isSgIfStmt (result) || isSgDoWhileStmt (result) || isSgSwitchStatement(result) )
    result = SageInterface::getEnclosingScope(result, false);

  // TODO: can recursive while-stmt scope happen?
  ROSE_ASSERT  (isSgWhileStmt (result) == NULL);
  ROSE_ASSERT  (isSgIfStmt (result) == NULL);
#endif  
  return result; 
}
//! Generate a scope tree for a declaration: the tree is trimmed. 
//  To trim the tree , the inner scopes using the variable are removed if there is a use scope which enclosing the inner scopes. 
//  Return the tree, can not be a NULL pointer. At least we return a node for the scope of the declaration
//  Also return the leaf scopes represented by the tree
//  TODO: make it a SageInterface function ?
Scope_Node* generateScopeTree(SgDeclarationStatement* decl, bool debug = false)//std::map <SgScopeStatement*, Scope_Node*>& ScopeTreeMap)
{
  ScopeTreeMap.clear();
//  bool debug = true; // debugging flag
  ROSE_ASSERT (decl != NULL);
  SgVariableDeclaration* var_decl = isSgVariableDeclaration(decl);
  ROSE_ASSERT (var_decl != NULL);
  SgScopeStatement * decl_scope = decl->get_scope();
  ROSE_ASSERT (decl_scope != NULL);

  SgVariableSymbol* var_sym = isSgVariableSymbol(SageInterface::getFirstVarSym(var_decl));
  ROSE_ASSERT (var_sym != NULL);

  // the root of the scope tree
  Scope_Node* scope_tree =new Scope_Node (decl_scope, s_decl);
  scope_tree->depth = 0;
  scope_tree->parent= NULL;
  ScopeTreeMap[decl_scope] = scope_tree; 

  // Step 1. Find all variable references to the declared variable.
  // Note: querySubTree uses pre-order traversal. 
  // This is essential since we want to preserve the order of the scopes within a parent scope.
  // The order of found variable references reflects the order of their scopes in AST. 
  //TODO: optimize for multiple declarations, avoid redundant query
  //   We can batch-generate scope trees for all declarations within a function
  Rose_STL_Container<SgNode*> nodeList = NodeQuery::querySubTree(decl_scope, V_SgVarRefExp);

// Liao, 12/5/2014, patch up the variable references for SgNewExp's SgConstructorInitializer, with ArrayType of index expression
  collectUpArrayTypeIndexVariables (decl_scope, nodeList);

  std::vector  <SgVarRefExp*> var_refs; 
  bool usedInSameScope = false; // if the declared variable is also used within the same scope
  for (Rose_STL_Container<SgNode *>::iterator i = nodeList.begin(); i != nodeList.end(); i++)
  {
    SgVarRefExp *vRef = isSgVarRefExp(*i);
    if (vRef->get_symbol() == var_sym )
    {
      //if (SageInterface::getScope(vRef) == decl_scope) 
      if (getAdjustedScope(vRef) == decl_scope) 
      {
        usedInSameScope = true; 
        break; // jump out the entire loop? 
        // continue; // we should just skip a single iteration instead. No difference in the end since the function will return if a single usedInSameScope is set.!
      } // same scope
      var_refs.push_back(vRef);
    } // match symbol
  }
  // If there is a single variable use which happens within the declaration scope,
  // we immediately return.
  if (usedInSameScope) 
  { 
    if (debug)
      cout<<"Found a declaration with a varRefExp in the same scope, return a single node scope tree."<<endl;
    return scope_tree; 
  }

  // Step 2: For each reference, generate a scope chain, stored in a temp stack. 
  // Each scope can be either where the variable is declared, used, or a scope in between (intermediate)
  /*
   *    For each reference place
   *    {
   *       back track all its scopes until we reach DS
   *       if DS == US, nothing can be done to move the declaration, stop. 
   *       all scopes tracked are formed into a scope chain (a vector: US .. DS).
   *
   *       create a scope node of use type, called US.
   *       For all scopes in between, create nodes named intermediate scope or IS. 
   *       Add Ds-IS1-IS2..-US (reverse order of the scope chain) into the scope tree, consider consolidating overlapped scope chains;
   *          1) if an intermediate scope is equal to a use scope of another scope chain; stop add the rest of this chain. 
   *          2) if we are adding a use scope into the scope tree, but the same scope is already added by a intermediate scope from anther chain
   *             we mark the  existing scope node as a use scope, and remove its children from the scope tree.
   *             This is hard to do if we store paths explicitly (we have to change all other paths containing the scope being added) 
   *    }
   */
  // There may be multiple variable references within a same scope, 
  // we use a set to record processed scope in order to avoid repetitively considering the same use scopes.
  std::set<SgScopeStatement*> processedUseScopes;
  // For each variable reference, we backtrack its scopes, and add the scope chain into the scope tree of this variable.
  for (size_t i =0; i< var_refs.size(); i++)
  {
    std::stack <SgScopeStatement*> temp_scope_stack;
    SgVarRefExp *vRef = var_refs[i];
    //SgScopeStatement* var_scope = SageInterface::getScope (vRef);
    SgScopeStatement* var_scope = getAdjustedScope(vRef);
    SgScopeStatement * current_scope = var_scope;
    ROSE_ASSERT (current_scope != decl_scope); // we should have excluded this situation already
    temp_scope_stack.push (current_scope) ;  // push the very first bottom scope
    do {
      // this won't work since getScope () will return the input scope as it is!!
      //current_scope = getScope (current_scope);
      current_scope = current_scope->get_scope(); // get parent scope and push
      temp_scope_stack.push (current_scope) ; 
    }
    while (current_scope != decl_scope) ;
    // exit condition is current_scope == decl_scope, as a result  
    // at this point , the declaration scope is already pushed into the stack  

    if (debug)
    {
      cout<<"scope stack size="<<temp_scope_stack.size()<<endl;
    }

    // Step 3: add nodes into the scope tree from the stack.
    //if the current use scope is not yet considered
    // add nodes into the scope tree, avoid duplicated add the scope node containing multiple var references.
    if (processedUseScopes.find(var_scope) == processedUseScopes.end())
    {
      // add each scope into the scope tree
      Scope_Node* current_parent = scope_tree;
      int depth_counter = 0;
      while (!temp_scope_stack.empty())
      { // TODO: verify that the scope tree preserves the original order of children scopes.
        current_scope = temp_scope_stack.top();
        Scope_Node * current_node = NULL; 
        ScopeType s_t; 
        if (current_scope == var_scope) 
          s_t = s_use;
        else 
          s_t = s_intermediate; 
        // avoid add duplicated node into the tree, the first one, root node, is duplicate.
        if (ScopeTreeMap[current_scope] == NULL )
        {
          current_node = new Scope_Node(current_scope, s_t);
          (current_parent->children).push_back(current_node);
          current_node->parent = current_parent;
          current_node->depth = depth_counter;

          ScopeTreeMap[current_scope] = current_node;
        }
        else
        {
          //TODO: optimize 3: we only push scopes which are not yet in the scope tree in to the temp_scope_stack for a new variable use!!
          // This will save overhead of pushing duplicated scopes into stack and later adding them into the scope tree!!
          // Downside: need to calculate depth separately, not using the stack depth. 
          //
          //handle possible overlapped paths, we only want to store the shortest path reaching a s_use node.
          // Further path to deeper use of a variable does not need to be considered. 
          // This is called trimmed path or trimmed tree. 
          //
          // Three conditions
          //1. Reuse the existing scope node, continue next scope in the stack
          //  e.g. the root node and the intermediate nodes shared by multiple scope chains
          // no special handling at all here. 

          //2. Reuse the existing scope node, stop going further
          //  e.g. the existing scope node is already a leaf node (s_use type)
          //  There is no need to go deeper for the current scope chain.
          Scope_Node* existing_node = ScopeTreeMap[current_scope]; 
          if (existing_node->s_type == s_use)
          {
            break; // jump out the while loop for the stack. 
          }  
          //3. Modify the existing scope node's type to be decl, stop g
          //  e.g We are reaching the leaf node of the current scope chain (s_use type)
          //  The existing scope node is s_intermediate. 
          //  we should delete the existing subtree and add the new leaf node
          //  To simplify the implementation, we change type of the existing node to be s_use type and delete all its children nodes. 
          else if (s_t == s_use) 
          {
            ROSE_ASSERT (existing_node->s_type == s_intermediate);
            existing_node->s_type = s_use; 
            existing_node->deep_delete_children(); 
          }  
        } 
        temp_scope_stack.pop();
        // must use the one in the tree, not necessary current_node from the stack
        current_parent = ScopeTreeMap[current_scope]; 
        //         if (current_node != ScopeTreeMap[current_scope])
        //            delete current_node; // delete redundant Scope Node.
        depth_counter++;
      } // end while pop scope stack  

      // mark the current leaf scope as processed.
      processedUseScopes.insert (var_scope);
    } // end if not processed var scope  
  } // end of adding a scope stack into the  scope tree

  //------------- debug the scope tree---------
  if (debug)
  {
    SgFunctionDeclaration* func_decl = SageInterface::getEnclosingFunctionDeclaration (decl);
    std::string func_name = (func_decl->get_name()).getString();
    int decl_lineno = decl->get_file_info()->get_line();
    std::string filename = "scope_tree_"+func_name+"_"+(var_sym->get_name()).getString()+ "_at_"+ StringUtility::numberToString(decl_lineno);
    // report the number of nodes in the scope tree
    std::vector <Scope_Node*> allnodes; 
    scope_tree->traverse_node (allnodes);
    cout<<"Scope tree node count:"<<allnodes.size()<<endl;
    // write the tree into a dot file
    scope_tree->printToDot(filename);
  }
  return scope_tree; 
}  



#if 0
static bool hasLoopInBetween (SgScopeStatement* top_scope, SgScopeStatement* bottom_scope)
{
  bool rt = false;
  ROSE_ASSERT (top_scope && bottom_scope);
  if (SageInterface::isAncestor (top_scope, bottom_scope))
  {
    do {
      bottom_scope = bottom_scope->get_scope(); // current bottom does not cout, we may move to the loop header
      if (isSgForStatement(bottom_scope)||isSgDoWhileStmt(bottom_scope) || isSgWhileStmt(bottom_scope))
      {
        rt = true;
//        if (debug)
        {
          cout<<"Found a loop boundary at line "<< bottom_scope->get_file_info()->get_line()<<endl;
//          cout<<"The declaration in question has the following file info:"<<endl;
        }
//        decl->get_file_info()->display();
      }
    } while (top_scope!=bottom_scope);
  }
  else
  {
    if (!SageInterface::tool_keep_going)
    {
      cerr<<"Error. declaration scope is not an ancestor scope of the target scope"<<endl;
      ROSE_ASSERT (false);
    }
  }
  return rt;
}

#endif

// A helper function to process target scopes
// if a target scope is a if statement, we grab the true and false body scopes as the new target scopes and remove the if-stmt scope from the original scope vector.
std::vector <SgScopeStatement*> processTargetScopes(std::vector <SgScopeStatement*> scopes)
{
  std::vector <SgScopeStatement*> processed_scopes; 
  for (size_t i = 0; i< scopes.size(); i++)
  {
    SgScopeStatement* target_scope = scopes[i];
    if (SgIfStmt* if_stmt = isSgIfStmt (target_scope))
    {
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        if (if_stmt->get_true_body())     
        {
          SgStatement* old_body = if_stmt->get_true_body();

       // bool old_body_is_compiler_generated = old_body->isCompilerGenerated();
       // bool old_body_is_compiler_generated_fromFileInfo = old_body->get_file_info()->isCompilerGenerated();

          SageInterface::ensureBasicBlockAsTrueBodyOfIf (if_stmt);
          SgScopeStatement* true_body = isSgScopeStatement(if_stmt->get_true_body());

#if 0
       // if (old_body != true_body)
          if (old_body_is_compiler_generated == true || old_body_is_compiler_generated_fromFileInfo == true)
             {
               ROSE_ASSERT(true_body->get_file_info()->isTransformation() == true);
               ROSE_ASSERT(true_body->isTransformation() == true);

               printf ("In processTargetScopes(): true_body = %p = %s \n",true_body,true_body->class_name().c_str());

               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
             }
#endif
          processed_scopes.push_back (true_body);
	  if (true_body != old_body)
	  {
	    assert (true_body->get_file_info()->isTransformation());
	  }
        }

        if (if_stmt->get_false_body())
        {
	  SgStatement * old_body = if_stmt->get_false_body();
          SageInterface::ensureBasicBlockAsFalseBodyOfIf (if_stmt);
          SgScopeStatement* false_body = isSgScopeStatement(if_stmt->get_false_body());
          processed_scopes.push_back (false_body);
	  if (false_body != old_body)
	  {
	    assert (false_body->get_file_info()->isTransformation());
	  }
        }
    }
    else
    {
      processed_scopes.push_back(target_scope);
    }
  }
  return processed_scopes;
}

//TODO: move into SageInterface 
//Check if a variable (symbol s) is ever referenced by a loop header, including init_stmt, test, and increment expressions.
static bool isReferencedByLoopHeader (SgVariableSymbol* s, SgForStatement * for_loop)
{
  std::map<SgVariableSymbol*, bool> symbolMap; 
  ROSE_ASSERT (s != NULL);
  ROSE_ASSERT (for_loop != NULL);
  if (for_loop->get_for_init_stmt()!= NULL)
  {
   Rose_STL_Container <SgNode*> testList = NodeQuery::querySubTree (for_loop->get_for_init_stmt(), V_SgVarRefExp);
   for (size_t i =0; i< testList.size(); i++)
   {
     SgVarRefExp * var_ref = isSgVarRefExp (testList[i]);
     symbolMap[var_ref->get_symbol()] = true; 
   }
  }

  if (for_loop->get_test()!= NULL)
  {
   Rose_STL_Container <SgNode*> testList = NodeQuery::querySubTree (for_loop->get_test(), V_SgVarRefExp);
   for (size_t i =0; i< testList.size(); i++)
   {
     SgVarRefExp * var_ref = isSgVarRefExp (testList[i]);
     symbolMap[var_ref->get_symbol()] = true; 
   }
  }


  if (for_loop->get_increment()!= NULL)
  {
   Rose_STL_Container <SgNode*> testList = NodeQuery::querySubTree (for_loop->get_increment(), V_SgVarRefExp);
   for (size_t i =0; i< testList.size(); i++)
   {
     SgVarRefExp * var_ref = isSgVarRefExp (testList[i]);
     symbolMap[var_ref->get_symbol()] = true; 
   }
  }
  return symbolMap[s]; 
}

// Copy/Move a single declaration into multiple scopes. 
// For each target scope:
// 1. Copy the decl to a new decl , 
// 2. Insert into the target_scopes, 
// 3. Replace variable references to the new copies
// 4. Finally, erase the original decl 
//
// Simply move won't work when there are more than one scopes to move into. 
//
// if the target scope is a For loop && the variable is index variable,  merge the decl to be for( int i=.., ...).
// Accumulate the set of inserted declarations.
//std::vector<SgVariableDeclaration* > 
void copyMoveVariableDeclaration(SgVariableDeclaration* decl, std::vector <SgScopeStatement*> scopes, std::stack<SgVariableDeclaration*> &worklist, 
 std::vector<SgVariableDeclaration* > & inserted_copied_decls)
{
  ROSE_ASSERT (decl!= NULL);
  ROSE_ASSERT (scopes.size() != 0);
  SgInitializedName* i_name = SageInterface::getFirstInitializedName (decl);
  ROSE_ASSERT (i_name != NULL);
  SgVariableSymbol * sym = SageInterface::getFirstVarSym(decl);
  ROSE_ASSERT (sym != NULL);
  SgScopeStatement* orig_scope = sym->get_scope();

  // used to keep track of transformation for this copyMoveVariableDeclaration()
  std::vector<SgVariableDeclaration* >  newly_inserted_copied_decls; 

#if 1 // TODO we should make sure target scopes are all legitimate at this point
  // when we adjust first branch node  (if-stmt with both true and false body )in the scope tree, we may backtrack to the decl's scope 
  // We don't move anything in this case.
  if ((scopes.size()==1) && (scopes[0] == decl->get_scope()))
  {
     return ; //inserted_copied_decls;
  }
#endif
  //TODO, no longer need this, simply ensure BB if it is a single statement of true/false body
  scopes = processTargetScopes(scopes);

  for (size_t i = 0; i< scopes.size(); i++)
  {
    SgScopeStatement* target_scope = scopes[i]; 
    ROSE_ASSERT (target_scope != decl->get_scope());

    SgScopeStatement* adjusted_scope = target_scope; 
    SgVariableDeclaration * decl_copy =  NULL; // we may not want to actually make copies here until the copy will really be inserted into AST
#if 0
 // DQ (11/2/2015): Save and zero the verbose level to control tracking of isModified flag.
    int verbose_level = SgProject::get_verbose();
    SgProject::set_verbose(0);
#endif
    decl_copy = SageInterface::deepCopy(decl);

    // Liao 1/14/2015
    // AST copy will copy the pointer to attached preprocessing information of the original declaration.
    // We don't want this behavior since it may duplicate the troublesome #endif for each copy of the declaration
    // A workaround is to clean this pointer
    decl_copy->set_attachedPreprocessingInfoPtr (NULL);
#if 0
 // DQ (11/2/2015): Reset the verbose level to trigger tracking of isModified flag.
    SgProject::set_verbose(verbose_level);
#endif
    //bool skip = false; // in some rare case, we skip a target scope, no move to that scope (like while-stmt)
    //This won't work. The move must happen to all scopes or not at all, or dangling variable use without a declaration.
    //We must skip scopes when generating scope tree, not wait until now.

    switch (target_scope->variantT())
    {
      case V_SgBasicBlock:
        {
#if 0
       // DQ (11/2/2015): Save and zero the verbose level to control tracking of isModified flag.
          verbose_level = SgProject::get_verbose();
          SgProject::set_verbose(0);
       // ROSE_ASSERT(adjusted_scope->get_isModified() == false);
#endif
          SageInterface::prependStatement (decl_copy, adjusted_scope);
          inserted_copied_decls.push_back(decl_copy); 
          // TODO: this only only useful for algorithm v1. Need better control
          newly_inserted_copied_decls.push_back(decl_copy);
#if 0
       // DQ (11/2/2015): Reset the verbose level to trigger tracking of isModified flag.
          SgProject::set_verbose(verbose_level);
       // ROSE_ASSERT(adjusted_scope->get_isModified() == false);
#endif
          break;
        }
      case V_SgForStatement:
        {
          SgForStatement* stmt = isSgForStatement (target_scope);
          ROSE_ASSERT (stmt != NULL);
          // target scope is a for loop and the declaration declares its index variable.
	  // A better condition is if the variable declared is referenced in the for header, we should insert the decl into the header.
          if (i_name == SageInterface::getLoopIndexVariable (stmt)) 
          {
            if (i_name == NULL)
            {
              cerr<<"Warning: in moveVariableDeclaration(): target_scope is a for loop with unrecognized index variable. Skipping it ..."<<endl;
              break;
            }
            // we move int i; to be for (int i=0; ...);
            SgStatementPtrList& stmt_list = stmt->get_init_stmt();
            // Try to match a pattern like for (i=0; ...) here
            // assuming there is only one assignment like i=0
            // We don't yet handle more complex cases
            if (stmt_list.size() !=1)
            {
              // TODO, how to handle two variable declaration?  I think this cannot happen in this context
              // find the inner most used scope of variable a.  It cannot be declaration of a. 
              cerr<<"Error in moveVariableDeclaration(): only single init statement is handled for SgForStatement now."<<endl;
              ROSE_ASSERT (stmt_list.size() ==1);
            }
            SgExprStatement* exp_stmt = isSgExprStatement(stmt_list[0]);
            ROSE_ASSERT (exp_stmt != NULL);
            SgAssignOp* assign_op = isSgAssignOp(exp_stmt->get_expression());
            if (assign_op != NULL)
	    {
	      ROSE_ASSERT (assign_op != NULL);
	      stmt_list.clear();
              // SageInterface::removeStatement() cannot handle this case, we remove it on our own
#if 0
           // DQ (11/2/2015): Save and zero the verbose level to control tracking of isModified flag.
              verbose_level = SgProject::get_verbose();
              SgProject::set_verbose(0);
#endif
	      SageInterface::mergeDeclarationAndAssignment (decl_copy, exp_stmt, false);
#if 0
           // DQ (11/2/2015): Reset the verbose level to trigger tracking of isModified flag.
              SgProject::set_verbose(verbose_level);
              ROSE_ASSERT(adjusted_scope->get_isModified() == false);
              ROSE_ASSERT(stmt->get_isModified() == false);
#endif
              SageInterface::deepDelete (exp_stmt);
              ROSE_ASSERT (stmt_list.size() ==0);
	      // insert the merged decl into the list, TODO preserve the order in the list
	      // else other cases: we simply prepend decl_copy to the front of init_stmt
#if 0
              // SageInterface:prependStatement () cannot be used here since SgForStatement is a scope 
              // but inserting things into it is ambugious semanticly. 
	      stmt_list.insert (stmt_list.begin(),  decl_copy);
	      decl_copy->set_parent(stmt->get_for_init_stmt());
              ROSE_ASSERT (stmt_list.size() ==1);
#else
              // A variant version should be able to handle it  
              SageInterface::prependStatement (decl_copy, stmt->get_for_init_stmt());
#endif              
	      ROSE_ASSERT (decl_copy->get_parent() != NULL); 
              // we already merged with assignment, we skip it so it won't be considered again?
              inserted_copied_decls.push_back(decl_copy);
              newly_inserted_copied_decls.push_back(decl_copy);

	    }
	    // TODO: it can be SgCommanOpExp
            else if (isSgCommaOpExp (exp_stmt->get_expression()) )
            {
               cerr<<"Error in moveVariableDeclaration(), multiple expressions in for-condition is not supported now. "<<endl;
               if (tool_keep_going) 
                 break;
               else  
                 ROSE_ASSERT (assign_op != NULL);
            } 
         } //
          else if (isReferencedByLoopHeader (sym ,stmt)) 
	  {
	    cerr<<"Error in moveVariableDeclaration(), A variable declaration is referenced in the loop header. But it is not loop index. It is a bad loop form and should be skipped long time ago. "<<endl;
	    if (tool_keep_going)
	      break;
	    else
	      ROSE_ASSERT (false);
	  }
	  else // now, the declared variable is not loop index and not referenced in the header. We can safely move it into the loop body
          {
            SgBasicBlock* loop_body = SageInterface::ensureBasicBlockAsBodyOfFor (stmt);
            adjusted_scope = loop_body;
            SageInterface::prependStatement (decl_copy, adjusted_scope);
            inserted_copied_decls.push_back(decl_copy);
            newly_inserted_copied_decls.push_back(decl_copy);
          }

       // DQ (11/2/2015): Added assertion.
          ROSE_ASSERT(adjusted_scope->get_isModified() == false);

          break;
        }
      // we duplicate and insert the declaration into true (and false) body, if the body exists
      // The two duplicated declarations are added into the  work list to be processed later on
      case V_SgIfStmt: 
        {
          cerr<<"if statement @ line"<< target_scope->get_file_info()->get_line()<< " should not show up in target scope vector after processTargetScopes()"<<endl;
          ROSE_ASSERT (false);
          break;
        }
      /*
       *We don't try to merge a variable decl into the conditional of a while statement
       * The reason is that often the declaration has an initializer , which must be preserved.
       * The conditional contain the use of the declared variable, which cannot be merged with the declaration.
       * e.g.  int yy = 10; while (yy< 100) { yy ++; ... }
       * Another tricky thing is even though yy shows up in both condition and while-body, 
       * the scope tree will only show while-stmt as the single scope node since the body scope is shadowed. 
       * Note: the scope of the condition is considered to be the while-stmt.
       * */  
      case V_SgWhileStmt: 
          cerr<<"while statement @ line"<< target_scope->get_file_info()->get_line()<< " should not show up in scope tree"<<endl;
          ROSE_ASSERT (false);
          break;
      default:
        {
          cerr<<"Error. Unhandled target scope type:"<<target_scope->class_name()
            << " when moving decl at:"<< decl->get_file_info()->get_line()
            << " for variable "<< sym->get_name()
            << " to scope at:"<< target_scope->get_file_info()->get_line()<<endl;
          ROSE_ASSERT  (false);
        }
    }
#if 1 
    // check what is exactly copied:
    // Symbol is not copies. It is shared instead
    SgVariableSymbol * new_sym = SageInterface::getFirstVarSym (decl_copy);

    // init name is copied, but its scope is not changed!
    // but the symbol cannot be find by calling init_name->get_symbol_from_symbol_table ()
    SgInitializedName* init_name_copy = SageInterface::getFirstInitializedName (decl_copy);

#if 0
 // DQ (11/2/2015): Save and zero the verbose level to control tracking of isModified flag.
    verbose_level = SgProject::get_verbose();
    SgProject::set_verbose(0);
#endif

 // Note that this will set the isModified flag be it can be ignored.
    init_name_copy->set_scope (adjusted_scope);

#if 0
 // DQ (11/2/2015): Reset the verbose level to trigger tracking of isModified flag.
    SgProject::set_verbose(verbose_level);
 // ROSE_ASSERT(adjusted_scope->get_isModified() == false);
#endif

    //ROSE_ASSERT (false);
    if (orig_scope != adjusted_scope)
    {
      // SageInterface::fixVariableDeclaration() cannot switch the scope for init name.
      // it somehow always reuses previously associated scope.
      ROSE_ASSERT (i_name != init_name_copy);
      // we have to manually copy the symbol and insert it
      SgName sname = sym->get_name();

   // DQ (11/2/2015): Added assertion.
   // ROSE_ASSERT(adjusted_scope->get_isModified() == false);

      adjusted_scope->insert_symbol(sname, new SgVariableSymbol (init_name_copy));

   // DQ (11/2/2015): Added assertion.
   // ROSE_ASSERT(adjusted_scope->get_isModified() == false);
    }
    new_sym = SageInterface::getFirstVarSym (decl_copy);
    ROSE_ASSERT (sym!=new_sym);
    // This is difficult since C++ variables have namespaces
    // Details are in SageInterface::fixVariableDeclaration()
    ROSE_ASSERT (adjusted_scope->symbol_exists(new_sym));

 // DQ (11/2/2015): Added assertion.
 // ROSE_ASSERT(adjusted_scope->get_isModified() == false);
#endif     
   

#if 1
#if 0
 // DQ (11/2/2015): Save and zero the verbose level to control tracking of isModified flag.
    verbose_level = SgProject::get_verbose();
    SgProject::set_verbose(0);
#endif

    // replace variable references
    SageInterface::replaceVariableReferences  (sym, new_sym, adjusted_scope);

#if 0
 // DQ (11/2/2015): Reset the verbose level to trigger tracking of isModified flag.
    SgProject::set_verbose(verbose_level);
 // ROSE_ASSERT(adjusted_scope->get_isModified() == false);
#endif
#endif 

// No need to check how the target scopes are identified.
// I treat them all the same. All newly inserted variables will be added into the worklist for further consideration!
    // add declarations into the worklist if the target scope is a marked true/false body of a if-stmt.
    // Note: not all bodies should be added. Only consider the marked scopes!!
//   if (todo_scopes.find(target_scope) != todo_scopes.end()) 
   {
     ROSE_ASSERT (decl_copy->get_parent() != NULL);
     worklist.push(decl_copy);
   }

   //SageInterface::setSourcePositionForTransformation (decl_copy);
#if 1
// send out warning info if there is a for loop between declaration's scope and the target scope
// a declaration is moved across the loop boundary.
   SgScopeStatement* top_scope = orig_scope;
   SgScopeStatement* bottom_scope = target_scope;
   if (SageInterface::isAncestor (top_scope, bottom_scope))
   {
     do {
       bottom_scope = bottom_scope->get_scope();
       if (isSgForStatement(bottom_scope)||isSgDoWhileStmt(bottom_scope) || isSgWhileStmt(bottom_scope))
       {
       //  cout<<"Warning: aggressive declaration moving across a loop boundary at line "<< bottom_scope->get_file_info()->get_line()<<endl;
#if 0
      // DQ (12/26/2014): commented out to avoid overly verbose output.
         cout<<"The declaration in question has the following file info:"<<endl;
         decl->get_file_info()->display();
#endif
       }

     } while (top_scope!=bottom_scope);
   }
   else
   {
     if (!tool_keep_going)
     {
       cerr<<"Error. declaration scope is not an ancestor scope of the target scope"<<endl;
       ROSE_ASSERT (false);
     }
   }
#endif

  } //end for all scopes

  // Special handing of preprocessing info.
  // Must happen before removing decl
  if (decl->get_attachedPreprocessingInfoPtr() != NULL)
  {
    // For a variable declaration to be copy/moved, the assumption is that they must have next statements (or no movement is possible)
    // Another assumption is that the preprocessing info must be attached to the "before" position of the declaration.
    SgStatement* next_stmt = SageInterface::getNextStatement(decl);
    if (next_stmt== NULL)
    {
      cerr<<"Error. Cannot find the next statement of the declaration to be moved!"<<endl;
      if (!tool_keep_going)
        ROSE_ASSERT (next_stmt!= NULL);
    }
    else
    { 
      // consider things attached before, move to the same location, using preprepend  to insert it.
       SageInterface::movePreprocessingInfo(decl, next_stmt, PreprocessingInfo::before, PreprocessingInfo::before, true); 
    }
  } // end if preprocessingInfo

 // DQ (11/2/2015): Added assertion.
 // ROSE_ASSERT(adjusted_scope->get_isModified() == false);

  // remove the original declaration , must use false to turn off auto-relocate comments, since it does not work correctly.
  // TODO: fix this in SageInterface or redesign how to store comments in AST: independent vs. attachments
  SageInterface::removeStatement(decl, false);

 // DQ (11/2/2015): Added assertion.
 // ROSE_ASSERT(adjusted_scope->get_isModified() == false);

// support transformation tracking/ IR mapping

#if ENABLE_TRANS_TRACKING
  if (transTracking)
    {
      // patch up IDs for the changed subtree 
      TransformationTracking::registerAstSubtreeIds (orig_scope);
      std::vector <SgVariableDeclaration*>::iterator iter;
      for (iter = newly_inserted_copied_decls.begin(); iter!= newly_inserted_copied_decls.end(); iter++)
      { //TransformationTracking::addInputNode (affected_node, input_node)
        TransformationTracking::addInputNode (*iter, decl); 
      }
    }  // end if transTracking
#endif
  //TODO deepDelete is problematic
  //SageInterface::deepDelete(decl);  // symbol is not deleted?
  //orig_scope->remove_symbol(sym);
  //delete i_name;
//  return inserted_copied_decls;
}

//! Check if a variable (symbol) is live in for a scope
// TODO A real solution is to check liveness analysis result here. However, the analysis is not ready for production use.
// The current workaround is to use a syntactic analysis: 
//
//    The opposite of dead:  !dead, not first defined
//    check if the first reference (in a pre-order traversal) to the variable is a definition (write access) only, exclude a reference which is both read and write, like a+=1; 
//    first defined means the variable is the lhs of a= rhs_exp; and it does not show up on rhs_exp; 
static bool isLiveIn(SgVariableSymbol* var_sym, SgScopeStatement* scope)
{
  // Approximate liveness analysis, only the first reference matters
  SgVarRefExp * first_ref = NULL; 
  Rose_STL_Container <SgNode*> testList = NodeQuery::querySubTree (scope, V_SgVarRefExp);
  for (size_t i=0; i< testList.size(); i++)
  {
    SgVarRefExp * current_exp = isSgVarRefExp (testList[i]);
    ROSE_ASSERT (current_exp != NULL);
    if (current_exp->get_symbol() == var_sym)
    {
      first_ref = current_exp; 
      break;
    }
  } // end for search

  // No reference at all?  Not liveIn
  if (first_ref == NULL) return false; 

  // Now go through common cases for finding definitions. 
  // 1. We don't care about declarations since the variables will be different from var_sym!
  // 2. For unary operations: ++ or --,  they must read the variables first then change the values
  // 3. Binary operations: only SgAssignOp does not read lhs oprand (define only )
  // 4. Function call parameters: addressOf(a): cannot move, is live

  // if a = rhs_exp;  and a does not show up in rhs_exp; 
  // then it is not live in (return false).
// TODO : handle casting
// TODO: handle rare case like (t=i)=8; 
  SgNode* parent = first_ref ->get_parent();
  ROSE_ASSERT (parent != NULL);
  if ( (isSgAssignOp(parent) && isSgAssignOp(parent)->get_lhs_operand()==first_ref) )
  {
    // check if it shows up on the rhs
    bool onRhs = false; 
    Rose_STL_Container <SgNode*> rhs_vars = NodeQuery::querySubTree (isSgAssignOp(parent)->get_rhs_operand(), V_SgVarRefExp);
    for (size_t i=0; i< rhs_vars.size(); i++)
    {
      SgVarRefExp * current_var = isSgVarRefExp (rhs_vars[i]);
      ROSE_ASSERT (current_var != NULL);
      if (current_var->get_symbol() == var_sym)
      {
        onRhs = true;
        break;
      }
    } // end for search
    if (!onRhs ) return false; 
  } // end if assignOp

  //All other cases, we conservatively assume the variable is live in for the scope
  return true; 
}

//
//! A helper function to check if there is a target scope which is not supported (cannot insert a decl into it)
// a for loop with complex init_stmt list
// a while loop with init_stmt
// TODO :other cases like SgDoWhileStmt ?
// With algorithm 2: we will consider the scopes for further moves.
// The check only matters if the target scope is the bottom scope
// For intermediate scopes, we allow them to be put into the candidate scope set
/*
Differentiate two situations when handling special scopes
1: When obtaining a scope for a variable in the condition expression. The scope should be adjusted since we cannot insert things to the conditions (??)
   can we remove this adjustment? Only handling them when deciding worklist issue??
2: When deciding on if we can insert a decl to the special scopes:  This has two further situations: 
  2.1  the special scope is a final bottom scope . We have backtrack to its parent scope  
  2.2  the special scope is an intermediate scope.  we can still put it into the worklist since the iterative algorithm 2

To simplify the problem, we consider them all in one pace: situation 2. hasUnsupportedScope() helps decide how to put scopes to the worklist.
We don't do anything special for situation 1.
*/

static bool isUnsupportedScope (SgScopeStatement* current_scope, SgVariableDeclaration* decl)
{
  ROSE_ASSERT (current_scope != NULL);
  ROSE_ASSERT (decl!= NULL);
  // The check only matters if the target scope is the bottom scope
  // Only scopes in the scope tree can be candidate scope to be considered
  ROSE_ASSERT (ScopeTreeMap[current_scope] != NULL);
  if (ScopeTreeMap[current_scope]->s_type == s_use)
  {
    if (SgForStatement* for_loop = isSgForStatement (current_scope))
    {
      SgInitializedName* i_name = getFirstInitializedName (decl);
      SgVariableSymbol * sym = getFirstVarSym (decl);
      ROSE_ASSERT (i_name != NULL);
      ROSE_ASSERT (sym != NULL);

      // multiple init statements or expressions
      if (SageInterface::hasMultipleInitStatmentsOrExpressions (for_loop))
      {
        return true;
      }
      else if (i_name != getLoopIndexVariable (for_loop) && isReferencedByLoopHeader (sym, for_loop))
      {  // single init, but cannot match loop index,  and the variable is referenced in the loop header
        // We cannot insert into the init stmt list since it will cause compilation error. e.g. inputmoveDeclarationToInnermostScope_13.C
        // We cannot move into the loop body neither since it is referenced in the loop header.
        return true;
      }
       // any other cases, we try to insert a decl into the for loop's init_stmt_list
    }
    else if (isSgWhileStmt (current_scope))
    {
      return true;
    }
    else if (isSgIfStmt(current_scope))
    {
      return true;
    }
    else if (isSgDoWhileStmt(current_scope))
    {
      return true;
    }
    else if (isSgSwitchStatement(current_scope))
    {
      return true;
    }
  } // end if bottom scopes

  // other types of scopes are supported
  return false;
}
static bool hasUnsupportedScope(SgVariableDeclaration* decl, std::vector <SgScopeStatement *> &target_scopes, SgScopeStatement** bad_apple_scope)
{
  ROSE_ASSERT (decl != NULL);
  SgInitializedName* i_name = getFirstInitializedName (decl);
  SgVariableSymbol * sym = getFirstVarSym (decl);
  ROSE_ASSERT (i_name != NULL);
  ROSE_ASSERT (sym != NULL);

  for (size_t i= 0; i< target_scopes.size(); i++)
  {
    SgScopeStatement* current_scope = target_scopes[i];
    if (isUnsupportedScope (current_scope, decl))
    {
        *bad_apple_scope = current_scope;
        return true; 
    }
  } // end for ()
  return false;
}


//! A helper function to check if there is a target scope which is a for loop with complex init_stmt list
static SgForStatement* hasALoopWithComplexInitStmt( SgVariableDeclaration* decl, std::vector <SgScopeStatement *> &target_scopes)
{
  ROSE_ASSERT (decl != NULL);
  SgInitializedName* i_name = getFirstInitializedName (decl);
  SgVariableSymbol * sym = getFirstVarSym (decl);
  ROSE_ASSERT (i_name != NULL);
  ROSE_ASSERT (sym != NULL);

   for (size_t i= 0; i< target_scopes.size(); i++)
   {
     SgScopeStatement* current_scope = target_scopes[i];
     if (SgForStatement* for_loop = isSgForStatement (current_scope))
     {
       // multiple init statements or expressions
       if (SageInterface::hasMultipleInitStatmentsOrExpressions (for_loop))
         return for_loop;
       else 
      {  // single init, but cannot match loop index,  and the variable is referenced in the loop header
         // We cannot insert into the init stmt list since it will cause compilation error. e.g. inputmoveDeclarationToInnermostScope_13.C
         // We cannot move into the loop body neither since it is referenced in the loop header.
         if (i_name != getLoopIndexVariable (for_loop) && isReferencedByLoopHeader (sym, for_loop))
           return for_loop; 
      }
     }
    }
  return NULL;
}

//! A helper functions to move special scopes of target scopes into source scope trees for further consideration
// Essentially handle all types of branching scopes , replacing each of them with their children scopes. Breaking the boundary of the branches.
// If a target statement is a if-stmt, we should replace it with two scopes, one for its true body, the other for its false body for further consideration. 
// SgBasicBlock may have children block scopes. They have to be considered further. 
std::vector <SgScopeStatement *> moveSpecialTargetScopesIntoScopeTreeQueue (const std::vector <SgScopeStatement *> &target_scopes, std::queue<Scope_Node* > &source_scope_trees)
{
  std::vector <SgScopeStatement*> processed_scopes;
  for (size_t i = 0; i< target_scopes.size(); i++)
  {
    SgScopeStatement* target_scope = target_scopes[i];
    
    Scope_Node* target_scope_node = ScopeTreeMap[target_scope];
    ROSE_ASSERT (target_scope_node != NULL);
    // If the scope tree is already considered as a root for further possible moving, skip it and directly save it the final target scope set
    if (scopeTreeConsideredMap [target_scope_node] ) 
    {
      processed_scopes.push_back(target_scope);
      continue;
    }

#if 0 // this is not necessary.  we now handle if-stmt the same as other types of intermedidate scope 11/015/2015
    // Now we handle all scope nodes which have not yet considered as a root for further possible moving
    // Special handling for if-stmt: true , false body is considered
    //TODO any other branching scope statements in AST? switch(), while(), for() all have a single inner body scope, not branching
    if (SgIfStmt* if_stmt = isSgIfStmt (target_scope))
    {
      if (if_stmt->get_true_body())
      {
	// the normalization must have been already done at this point, or the BB will not be in the scope tree
	//          SageInterface::ensureBasicBlockAsTrueBodyOfIf (if_stmt);
	SgScopeStatement* true_body = isSgScopeStatement(if_stmt->get_true_body());
	assert (true_body != NULL);
        // rare case : true body may be empty, no references to the declared variable, no scope tree node for it
        // Then no need to consider it
	if (ScopeTreeMap[true_body]) 
        {
          source_scope_trees.push(ScopeTreeMap[true_body]);
          scopeTreeConsideredMap[ScopeTreeMap[true_body]] = true;
        }

      }

      if (if_stmt->get_false_body())
      {
	// the normalization must have been already done at this point, or the BB will not be in the scope tree
	//          SageInterface::ensureBasicBlockAsFalseBodyOfIf (if_stmt);
	SgScopeStatement* false_body = isSgScopeStatement(if_stmt->get_false_body());
	assert (false_body != NULL);
        // Liao 2015/11/4,  the false body may be empty, without references to the declared variable.
        // As a result, there is no corresponding scope tree node created for this empty false body.
        // In this case, we should not try to consider this false body for further declaration movement.
        if (ScopeTreeMap[false_body])
        {
          source_scope_trees.push(ScopeTreeMap[false_body]);
          scopeTreeConsideredMap[ScopeTreeMap[false_body]] = true;
        }
   
      }
    } 
    // For all other non-bottom scope node, add them into source_scope_trees for further consideration
    else 
#endif
    if (target_scope_node->s_type ==s_intermediate)
    {
      //cout<<"Debugging   moveSpecialTargetScopesIntoScopeTreeQueue()  pushing a scope node to the worklist .."<< target_scope <<"@ "<< target_scope->variantT() << " "<< target_scope->get_file_info()->get_line() <<endl;
      //collectCandidateTargetScopes() may backtrack to  a parent scope for consideration
      //This may cause the same scope being considered twice  and cause infinite looping
      //To avoid this case, we have to make sure the same scope node is only considered once
      source_scope_trees.push(target_scope_node);
      scopeTreeConsideredMap[target_scope_node] = true;
    }
    else  
    // The rest target scopes are preserved, no need for further consideration  
    {
     // ROSE_ASSERT (target_scope_node->s_type ==s_use); 
      processed_scopes.push_back(target_scope);
    }
  }
  return processed_scopes;
}

// For a scope tree, collect final target scopes and middle target scopes for further consideration
// This is the core step of algorithm V2's findFinalTargetScopes()
// TODO: this is a much more sophisticated problem than I originally thought!
/* 
      n0
      | 
      n1
     / \
    n2  n3
       / \
      n4  n5 

The issue is that n3 can be unsupported scope (scope into which we cannot insert a decl, like a while-stmt scope).
n3 is not bottom. On the surface, we can put n3 into scope_trees worklist for further consideration.
n2 is put to the final_target_scopes as a final scope.

But if n3 has no legitimate children or grand children nodes to accept the moved down declarations, 
we have to back track and invalid n2 also. 

Option 1: simplest.  create a new basic block scope before the unsupported scope. looks a bit bad. but simple to implement.
//TODO : ask users for their opinions

Option 2: whenever a search for a scope tree at a root does not find a single eligible final target scope, 
we have to backtrack trough the scope tree, find an upper eligible final target scope.
In the meantime, we have to invalidate the root's sibling nodes (and all children?) in the final target scope set, 
  and possibly invalidate some nodes (sibling and all children?) in the scope tree worklist. 

   invalidate and mark the affected nodes as considered also. 

The scope tree worklist is a queue?, depth first traverse is used

The invalidation can happen in a postprocessing function: search 

Option 3: run the analysis twice: the first time mark all possible final scopes, then mark each scope nodes to indicate if it has a potential
  child target node. 
 The 2nd time uses the previous results to decide if we can put a unsupported scope node into the worklist
This is not elegant, but easy to understand and implement.

The 2nd run is only necessary when there is a unsupported, non-bottom scope node being pushed to the worklist!!
TODO: this function is not ready for use. Under development. 2015-11-05
*/
void  collectFinalAndMiddleTargetScopes (SgVariableDeclaration* decl, Scope_Node* scope_tree, 
               std::vector <SgScopeStatement *> &final_target_scopes, std::queue<Scope_Node* >& /* source_scope_trees */,  bool debug)
{
  // single node scope tree, it is already the bottom, move to the final target scope 
  // if it is different from the original decl scope && is supported
  // Can the iterative algorithm generate new single node scope tree? No ! we checked the new root to be non-bottom!!
  if ((scope_tree->children).size() == 0 )
  {
    if (scope_tree->scope != decl->get_scope() && !isUnsupportedScope (scope_tree->scope, decl) )
    {
      final_target_scopes.push_back(scope_tree->scope);
      // no addition to the source_scope_trees worklist
    }  
    return ;
  }

  // for a scope tree with two or more nodes  
  Scope_Node* first_branch_node = scope_tree->findFirstBranchNode();

  // Step 2: simplest case, only a single use place, single path tree
  // -----------------------------------------------------
  // the first branch node is also the bottom node
  if ((first_branch_node->children).size() ==0)
  {
    SgScopeStatement* bottom_scope = first_branch_node->scope;
    ROSE_ASSERT(!isUnsupportedScope (bottom_scope, decl));
    //TODO: what if backtracked to the root, which is a unsupported node added to the worklist scope trees?
    // We must enforce unsupported scope bottom are backtraced when generating the scope tree! So the bottoms are all legal supported target scopes!
    // getAdjustedScope () must ensure this and mark the backtracked scope as bottom (s-use). 
    final_target_scopes.push_back(bottom_scope);
  } // end the single decl-use path case
  else  
  { 
    // multiple path tree
    //Step 3: multiple scopes
    // -----------------------------------------------------
    // there are multiple (0 to n - 1 )child scopes in which the variable is used. 
    // if for all scope 1, 2, .., n-1
    //  the variable is defined before being used (not live)
    //  Then we can move the variable into each child scope
    // Conversely, if any of scope has liveIn () for the declared variable, we cannot move
    bool moveToMultipleScopes= true ; 

    for (size_t i =0; i< (first_branch_node->children).size(); i++)
    {
      Scope_Node * current_child_node = first_branch_node->children[i];
      SgScopeStatement * current_child_scope = current_child_node->scope;
      ROSE_ASSERT (current_child_scope != NULL); 
      if (i>0) // consider the 2nd and later scope
      {
        SgVariableSymbol * var_sym = SageInterface::getFirstVarSym (decl); 
        ROSE_ASSERT (var_sym != NULL);
        if (isLiveIn (var_sym, current_child_scope))
        {
          moveToMultipleScopes = false;
          break; // Find one is enough!
        } 
      }
#if 1 // not working yet, many places to identify candidates. Better screen them later when this call is finished
      // if a child scope is unsupported and bottom (no supported grandchildren) we cannot move to this level neither.  
      // A wiggle room here: the child scope is unsupported, but there is a supported grandchild scope underneath it,
      // we can still put it into the worklist
      // TODO: need a search function to ensure this!! bool existingEligibleFinalTargetScope(root, decl)
      if (isUnsupportedScope (current_child_scope, decl) && current_child_node->s_type == s_use)
      {
        moveToMultipleScopes = false;
        break; // Find one is enough!
      }  
#endif 
    }  // end for all scopes

    if (moveToMultipleScopes)
    {
      if (debug)
	cout<<"Found a movable declaration for multiple child scopes"<<endl;
      for (size_t i =0; i< (first_branch_node->children).size(); i++)
      {
#if 0        
	// we try to get the bottom for each branch, not just the upper scope
	// This is good for the case like: "if () { for (i=0;..) {}}" and if-stmt's scope is a child of the first branch scope node
	// TODO: one branch may fork multiple branches. Should we move further down on each grandchildren branch?
	//       Not really, we then find the common inner most scope of that branch. just simply move decl there!
	// Another thought: A better fix: we collect all leaf nodes of the scope tree! It has nothing to do with the first branch node!
	//       this won't work. First branch node still matters. 
	Scope_Node* current_child_scope = first_branch_node->children[i];     
        //TODO: this is not clean from a recursive function point of view, This should be handled by the top while loop iterating on the scope tree queue. 
	Scope_Node* bottom_node = current_child_scope -> findFirstBranchNode ();
	SgScopeStatement * bottom_scope = bottom_node->scope;
	ROSE_ASSERT (bottom_scope!= NULL);
	target_scopes.push_back (bottom_scope);
#else
        // Liao 2015/11/2.  We only need to generate the candidate scopes here. 
        // Delegate the processing of bottom (s_use for the scope tree node) vs. non-bottom scope (scope tree node type is s_intermedidate)
        // to later moveSpecialTargetScopesIntoScopeTreeQueue().
        // I don't want to mix the collection of candidates with other scope handling logic here.
       // 1. collect candidates --> 2. decide on if root or children should be the legitimate target --> 3. add non-bottom targets to worklist
	Scope_Node* current_child_scope_node = first_branch_node->children[i];     
	SgScopeStatement * child_scope = current_child_scope_node->scope;
	ROSE_ASSERT (child_scope!= NULL);
	final_target_scopes.push_back (child_scope);
#endif        
      }
    }
    else // we still have to move it to the innermost common scope
    {
      SgScopeStatement* bottom_scope = first_branch_node->scope;
      if (decl->get_scope() != bottom_scope)
      {
	final_target_scopes.push_back(bottom_scope);
      }
    } // end else
  } // end else multiple scopes
}


// For a scope tree, collect candidate target scopes. This is the core step of algorithm V2's findFinalTargetScopes()
/*
 * 3 cases
 *  1. single node scope tree: add to target_scopes if it is different from the decl's scope
 *  2. multiple nodes, single bottom, add the bottom scope to target_scopes.
 *  3. multiple scopes under the first branch scope (FBS): 
 *    No Live In violation: add all child scopes of FBS to target_scopes., go to the bottom for each child path
 *    Yes LiveIn violation: add the first branch scope to target_scope
 * */ 
std::vector <SgScopeStatement *> collectCandidateTargetScopes (SgVariableDeclaration* decl, Scope_Node* scope_tree, bool debug)
{
  std::vector <SgScopeStatement *> target_scopes; 
  // single node scope tree, move to the scope if it is different from the original decl scope
  if ((scope_tree->children).size() == 0 )
  {
    if (scope_tree->scope != decl->get_scope())
      target_scopes.push_back(scope_tree->scope);
    return target_scopes; // otherwise duplicated scopes will be inserted.
  }

  // for a scope tree with two or more nodes  
  Scope_Node* first_branch_node = scope_tree->findFirstBranchNode();

  // Step 2: simplest case, only a single use place
  // -----------------------------------------------------
  // the first branch node is also the bottom node
  if ((first_branch_node->children).size() ==0)
  {
    SgScopeStatement* bottom_scope = first_branch_node->scope;
    target_scopes.push_back(bottom_scope);
  } // end the single decl-use path case
  else 
  { 
    //Step 3: multiple scopes
    // -----------------------------------------------------
    // there are multiple (0 to n - 1 )child scopes in which the variable is used. 
    // if for all scope 1, 2, .., n-1
    //  the variable is defined before being used (not live)
    //  Then we can move the variable into each child scope
    // Conversely, if any of scope has liveIn () for the declared variable, we cannot move
    bool moveToMultipleScopes= true ; 

    for (size_t i =0; i< (first_branch_node->children).size(); i++)
    {
      SgScopeStatement * current_child_scope = (first_branch_node->children[i])->scope;
      ROSE_ASSERT (current_child_scope != NULL); 
      if (i>0) // consider the 2nd and later scope
      {
        SgVariableSymbol * var_sym = SageInterface::getFirstVarSym (decl); 
        ROSE_ASSERT (var_sym != NULL);
        if (isLiveIn (var_sym, current_child_scope))
        {
          moveToMultipleScopes = false;
          break; // Find one is enough!
        } 
      }
#if 0 // not working yet, many places to identify candidates. Better screen them later when this call is finished
      // if a child scope is unsupported? we cannot move to this level neither.  
      if (isUnsupportedScope (current_child_scope, decl))
      {
        moveToMultipleScopes = false;
        break; // Find one is enough!
      }  
#endif 
    }  // end for all scopes

    if (moveToMultipleScopes)
    {
      if (debug)
	cout<<"Found a movable declaration for multiple child scopes"<<endl;
      for (size_t i =0; i< (first_branch_node->children).size(); i++)
      {
#if 0        
	// we try to get the bottom for each branch, not just the upper scope
	// This is good for the case like: "if () { for (i=0;..) {}}" and if-stmt's scope is a child of the first branch scope node
	// TODO: one branch may fork multiple branches. Should we move further down on each grandchildren branch?
	//       Not really, we then find the common inner most scope of that branch. just simply move decl there!
	// Another thought: A better fix: we collect all leaf nodes of the scope tree! It has nothing to do with the first branch node!
	//       this won't work. First branch node still matters. 
	Scope_Node* current_child_scope = first_branch_node->children[i];     
        //TODO: this is not clean from a recursive function point of view, This should be handled by the top while loop iterating on the scope tree queue. 
	Scope_Node* bottom_node = current_child_scope -> findFirstBranchNode ();
	SgScopeStatement * bottom_scope = bottom_node->scope;
	ROSE_ASSERT (bottom_scope!= NULL);
	target_scopes.push_back (bottom_scope);
#else
        // Liao 2015/11/2.  We only need to generate the candidate scopes here. 
        // Delegate the processing of bottom (s_use for the scope tree node) vs. non-bottom scope (scope tree node type is s_intermedidate)
        // to later moveSpecialTargetScopesIntoScopeTreeQueue().
        // I don't want to mix the collection of candidates with other scope handling logic here.
       // 1. collect candidates --> 2. decide on if root or children should be the legitimate target --> 3. add non-bottom targets to worklist
	Scope_Node* current_child_scope_node = first_branch_node->children[i];     
	SgScopeStatement * child_scope = current_child_scope_node->scope;
	ROSE_ASSERT (child_scope!= NULL);
	target_scopes.push_back (child_scope);
#endif        
      }
    }
    else // we still have to move it to the innermost common scope
    {
      SgScopeStatement* bottom_scope = first_branch_node->scope;
#if 0 // We no longer do the adjustment here. We delay the logic in the insertion logic. And generate two additional declarations to be considered later on
      // special adjustment here for if-stmt as the single bottom scope  to be inserted into the var decl.
      // we should adjust here, not any other places !!
      // TODO may have to include other special stmt like while?
      if (isSgIfStmt(bottom_scope))
	bottom_scope = SageInterface::getEnclosingScope(bottom_scope, false);
#endif 
      if (decl->get_scope() != bottom_scope)
      {
	target_scopes.push_back(bottom_scope);
      }
    } // end else
  } // end else multiple scopes
  return target_scopes; 
}

// V2 algorithm's first step: iterative algorithm to find all final target scopes
/*
Find all bottom scopes to move into: no side effect on AST at all

1. Initialization: 
  source_scope_trees: the top scope tree of the single decl in question

2. For each tree of  source_scope_trees: find candidate scopes, save into target_scopes
  collect candidate scopes 
      n0
      | 
      n1
     / \
    n2  n3
 
We differentiate two kinds of candidate scopes: 

 final_target_scopes: know for certain this is the final one. No need for further consideration. 
  * It can be a real bottom scope , or 
  * an intermediate scope which is reached by backtracking from a unsupported bottom scope. No use to go deeper.

 scopetree worklist_scopes: a scope which is a child of a first branch node. and it is not the bottom.  

   a. Single node scope tree, if diff from orig_scope, add to target_scopes.  
        TODO: && !isUnSupportedScope ()   It is already a bottom scope (s_use type) 
   b. Multiple nodes tree: find the first branching node
                         && !isUnSupportedScope()  Does this matter? if some middle scope is not supported. No big deal really.  
      i. Single path tree: First branch node is a bottom: push to target_scopes as a candidate
                      Is supportedScope important?  Yes. unsupported will not accept decl to be inserted. 
                      TODO: how to handle this cleanly?  backtrack to a scope node which is supported, mark it as final? No redundant consideration.

      ii. Multiple path tree:  First branch node has multiple children
          1. No move down if has liveIn, push first-branch scope into target_scope, if it is diff from orig_scope
          2. No move down if any of the children is not supported (while, if, switch, etc.) && is a final (s_use type) target scope 
             Reason: a final, unsupported scope cannot accept the declaration!!
                     an intermediate , unsupported scope can serve as a new root for further move consideration
          For both no move down case: it is the same handling for the single path case b-i (reuse the code)           

          3. Move to multiple scopes: for each child’s first branch 
             if the child scope is final , add to final_target_scope
             if not final, add to worklist_scopes.  

3. Target_scopes to source_scope_trees transition, caused by non-bottom scopes
   This is to handle uneven bottom scope issues, as demonstrated below:
      n0
      | 
      n1
     / \
    n2  n3
       / \
      n4  n5 
First branching node is n1.   No liveIn between n2 and n3. So candidate collection step will collect n2 and n3.
For all candidate scopes, if they have further children, they should be treated as a new root for brand new consideration.
One example is n3 is If-stmt which is a scope. 
We need to add n3 as a new scope tree root for further consideration. 
n2 is kept/saved as one of the final target scopes

4. if (!Check stop condition):   repeat 2 and 3, essentially do (2, 3) while ()
   a. scope tree: root scopes are processed (finished) a scopelist, source_scopes becomes empty
   b. // Implicitly ensured by 3 all target_scopes are bottom, no non-bottom scopes like if-stmt anymore

Finally delete the scope tree in any cases.  IF the same AS orig_scope, skip moving.  delete still. 

Liao 1/27/2015 
 */
// Hopefully a new cleaner version
void findFinalTargetScopes_V2(SgVariableDeclaration* declaration, std::vector <SgScopeStatement *> &final_target_scopes, bool debug)
{
  // A single original scope tree can spawn to multiple sub-trees, depending on where to start as a root
  // Each target scope will be treated as a root to consider further search for the bottom scopes.
  std::queue<Scope_Node* > source_scope_trees; 
  SgVariableDeclaration * decl = declaration;
  ROSE_ASSERT (decl != NULL);
  // Step 1: generate a scope tree for the declaration
  // -----------------------------------------------------
  // Initially only one scope tree
  Scope_Node* orig_scope_tree = generateScopeTree (decl, debug);
  source_scope_trees.push(orig_scope_tree);
  scopeTreeConsideredMap[orig_scope_tree] = true;

  while (!source_scope_trees.empty())
  {
    Scope_Node* scope_tree = source_scope_trees.front();
    source_scope_trees.pop(); // remove it from the queue

    std::vector <SgScopeStatement *> candidate_scopes;  // per scope tree info.
    // collect final and middle target scopes, save them into two different set
    collectFinalAndMiddleTargetScopes (decl, scope_tree, final_target_scopes, source_scope_trees, debug);
  }  // end while

  // delete the original scope tree
  orig_scope_tree->deep_delete_children ();
  delete orig_scope_tree;
}


// old working version ....
void findFinalTargetScopes(SgVariableDeclaration* declaration, std::vector <SgScopeStatement *> &target_scopes, bool debug)
{
  // A single original scope tree can spawn to multiple sub-trees, depending on where to start as a root
  // Each target scope will be treated as a root to consider further search for the bottom scopes.
  std::queue<Scope_Node* > source_scope_trees; 
  SgVariableDeclaration * decl = declaration;
  ROSE_ASSERT (decl != NULL);
  // Step 1: generate a scope tree for the declaration
  // -----------------------------------------------------
  // Initially only one scope tree
  Scope_Node* orig_scope_tree = generateScopeTree (decl, debug);
  source_scope_trees.push(orig_scope_tree);
  scopeTreeConsideredMap[orig_scope_tree] = true;

  // some target scopes may not be valid one: like init-stmt scope within a for-loop, which has a list of things. 
  // we need to screen out them and invalid the move for the associated scope tree.
  // 
  while (!source_scope_trees.empty())
  {
    Scope_Node* scope_tree = source_scope_trees.front();
    source_scope_trees.pop(); // remove it from the queue

    std::vector <SgScopeStatement *> candidate_scopes;  // per scope tree info.
    candidate_scopes= collectCandidateTargetScopes (decl, scope_tree, debug);
    if (candidate_scopes.size() > 0)
    {
#if 1  // Tried to put the supported or not logic inside collectCandidateTargetScopes() , But it did not work
      // ignore complex for init stmt for now 
      // A single bad apple will invalidate the entire move of this scope tree
//      SgForStatement* bad_loop = hasALoopWithComplexInitStmt (declaration, candidate_scopes);
//      if (bad_loop != NULL) // with bad apple? obtain the scope_tree's scope as target scope
       SgScopeStatement * bad_scope_stmt= NULL; 
       if (hasUnsupportedScope(declaration, candidate_scopes, & bad_scope_stmt ))
       {
         ROSE_ASSERT (bad_scope_stmt !=NULL);
         cerr<<"Error: SageInterface::moveDeclarationToInnermostScope() gives up moving a variable decl due to a unsupported target scope"<<endl;
         cerr<<"Variable declaration in question is:"<<endl;
         declaration->get_file_info()->display();
         cerr<<"Unsupported scope:" << bad_scope_stmt << " " << bad_scope_stmt->class_name() << " " << bad_scope_stmt->get_file_info()->get_line() <<endl;
        // if candidate children nodes are excluded from consideration because a bad apple, 
         //  we use the root scope node as the move target
         target_scopes.push_back(scope_tree->scope);
       }
      else // no bad apple?  moves can happen for all candidate children scopes of the scope tree
#endif
      {
	for (size_t i =0; i<candidate_scopes.size(); i++)
	{
	  target_scopes.push_back(candidate_scopes[i]);
	}
      }
    } // end if (candidate_scopes.size() > 0)

    // target_scopes to source_scope_trees transition, caused by if-stmt
    /* Essentially handle all types of branching scopes , replacing each of them with their children scopes. Breaking the boundary of the branches.
     * Take if-stmt as example, find all if-stmt scopes of target_scopes, 
     * remove them from target_scopes
     * add their true/false scopes into source_scope_trees // the removed ones can be added back later for single node scope tree case.
     * */ 
    target_scopes = moveSpecialTargetScopesIntoScopeTreeQueue (target_scopes, source_scope_trees);
  }  // end while
  // delete the original scope tree
  orig_scope_tree->deep_delete_children ();
  delete orig_scope_tree;

// DQ (11/7/2015): THIS IS THE  BUG FIX: Also clear the scopeTreeConsideredMap (since it references nodes in the scope tree that has just been deleted).
  scopeTreeConsideredMap.clear();
}

// Improved 2-step algorithm:
// Step 1: iterative algorithm to find the real bottom scopes 
// Step 2: copy & move source declaration to all the bottom scopes.
// return the final scopes accepting the moved declarations.
//std::vector <SgVariableDeclaration*> 
void moveDeclarationToInnermostScope_v2 (SgVariableDeclaration* declaration, std::vector <SgVariableDeclaration*>& my_inserted_decls, bool debug = false)
{
//  std::vector <SgVariableDeclaration* > inserted_decl; 
  SgScopeStatement* orig_scope = declaration->get_scope();
  ROSE_ASSERT  (orig_scope != NULL );
  std::vector <SgScopeStatement *> target_scopes;
#if 0
  printf ("In moveDeclarationToInnermostScope_v2(): declaration = %p = %s (calling findFinalTargetScopes()) \n",declaration,declaration->class_name().c_str());
#endif
  findFinalTargetScopes (declaration, target_scopes, debug);
  std::stack<SgVariableDeclaration*> worklist;   // not really useful in this algorithm, dummy parameter
  if (target_scopes.size() > 0)
  {
#if 0
  printf ("In moveDeclarationToInnermostScope_v2(): declaration = %p = %s (calling copyMoveVariableDeclaration())\n",declaration,declaration->class_name().c_str());
#endif
    copyMoveVariableDeclaration (declaration, target_scopes, worklist, my_inserted_decls);

#if 0 // we should not iterate my_inserted_decls here since it may contain previously inserted declarations.
      // this should be moved inside copyMoveVariableDeclaration() to only recordly newly inserted declarations
// support transformation tracking/ IR mapping
  if (transTracking)
    {
      // patch up IDs for the changed subtree 
      TransformationTracking::registerAstSubtreeIds (orig_scope);
      std::vector <SgVariableDeclaration*>::iterator iter;

      for (iter = my_inserted_decls.begin(); iter!= my_inserted_decls.end(); iter++)
      { //TransformationTracking::addInputNode (affected_node, input_node)
        TransformationTracking::addInputNode (*iter, declaration); 
      }
    }  // end if transTracking
#endif
  } // end target_scopes.size()
}

// Old algorithm: iteratively find target scopes and actually move declarations.
// The downside is that declaration will be moved into temporary target scopes, not efficient
// Harder to keep track of the final target scopes
bool moveDeclarationToInnermostScope_v1(SgVariableDeclaration* declaration, std::stack<SgVariableDeclaration*> &worklist, bool debug = false)
{
  SgVariableDeclaration * decl = isSgVariableDeclaration(declaration);
  ROSE_ASSERT (decl != NULL);
  // Step 1: generate a scope tree for the declaration
  // -----------------------------------------------------
  Scope_Node* scope_tree = generateScopeTree (decl, debug);

  // single node scope tree, nowhere to move into. 
  if ((scope_tree->children).size() == 0 )
  {
    scope_tree->deep_delete_children ();
    delete scope_tree;
    return false; 
  }

  // for a scope tree with two or more nodes  
  Scope_Node* first_branch_node = scope_tree->findFirstBranchNode();

  // the scopes to be moved into
  std::vector <SgScopeStatement *> target_scopes; 
  // Step 2: simplest case, only a single use place
  // -----------------------------------------------------
  // the first branch node is also the bottom node
  if ((first_branch_node->children).size() ==0)
  {
    SgScopeStatement* bottom_scope = first_branch_node->scope;
    target_scopes.push_back(bottom_scope);
  } // end the single decl-use path case
  else 
  { 
    //Step 3: multiple scopes
    // -----------------------------------------------------
    // there are multiple (0 to n - 1 )child scopes in which the variable is used. 
    // if for all scope 1, 2, .., n-1
    //  the variable is defined before being used (not live)
    //  Then we can move the variable into each child scope
    // Conversely, if any of scope has liveIn () for the declared variable, we cannot move
    bool moveToMultipleScopes= true ; 

    for (size_t i =1; i< (first_branch_node->children).size(); i++)
    {
      SgVariableSymbol * var_sym = SageInterface::getFirstVarSym (decl); 
      ROSE_ASSERT (var_sym != NULL);
      SgScopeStatement * current_child_scope = (first_branch_node->children[i])->scope;
      ROSE_ASSERT (current_child_scope != NULL); 
      if (isLiveIn (var_sym, current_child_scope))
        moveToMultipleScopes = false;
    }  // end for all scopes

    if (moveToMultipleScopes)
    {
      if (debug)
        cout<<"Found a movable declaration for multiple child scopes"<<endl;
      for (size_t i =0; i< (first_branch_node->children).size(); i++)
      {
        // we try to get the bottom for each branch, not just the upper scope
        // This is good for the case like: "if () { for (i=0;..) {}}" and if-stmt's scope is a child of the first branch scope node
        // TODO: one branch may fork multiple branches. Should we move further down on each grandchildren branch?
        //       Not really, we then find the common inner most scope of that branch. just simply move decl there!
        // Another thought: A better fix: we collect all leaf nodes of the scope tree! It has nothing to do with the first branch node!
        //       this won't work. First branch node still matters. 
        Scope_Node* current_child_scope = first_branch_node->children[i];     
        Scope_Node* bottom_node = current_child_scope -> findFirstBranchNode ();
        SgScopeStatement * bottom_scope = bottom_node->scope;
        ROSE_ASSERT (bottom_scope!= NULL);
        if (bottom_scope != decl->get_scope())
         target_scopes.push_back (bottom_scope);
      }
    }
    else // we still have to move it to the innermost common scope
    {
      SgScopeStatement* bottom_scope = first_branch_node->scope;
#if 0 // We no longer do the adjustment here. We delay the logic in the insertion logic. And generate two additional declarations to be considered later on
      // special adjustment here for if-stmt as the single bottom scope  to be inserted into the var decl.
      // we should adjust here, not any other places !!
      // TODO may have to include other special stmt like while?
      if (isSgIfStmt(bottom_scope))
        bottom_scope = SageInterface::getEnclosingScope(bottom_scope, false);
#endif 
      if (decl->get_scope() != bottom_scope)
      {
        target_scopes.push_back(bottom_scope);
      }
    } // end else
  } // end else multiple scopes

  if (target_scopes.size()>0)
  {
    // ignore complex for init stmt for now 
    SgForStatement* bad_loop = hasALoopWithComplexInitStmt (decl, target_scopes);
    if (bad_loop != NULL)
    {
      cerr<<"Error: SageInterface::moveDeclarationToInnermostScope() gives up moving a variable decl due to a complex target loop scope"<<endl;
      cerr<<"Variable declaration in question is:"<<endl;
      decl->get_file_info()->display();
      cerr<<"Loop scope with complex init stmt is:"<<endl;
      bad_loop->get_file_info()->display();
#if 0  // We no longer assert this since the complex loops are out of our scope. Users will make sure their loops are canonical.
      if (!tool_keep_going )
        ROSE_ASSERT (false);
#endif 
    }
    else
      copyMoveVariableDeclaration (decl, target_scopes, worklist, inserted_decls);
    scope_tree->deep_delete_children ();
    delete scope_tree;
    return true;
  }
  else 
  {
    scope_tree->deep_delete_children ();
    delete scope_tree;
    return false;  
  }
}

