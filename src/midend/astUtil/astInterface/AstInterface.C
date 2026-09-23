#include <sage3basic.h>
#include "unparser.h"
#include "unparser_opt.h"
#include "AstInterface.h"
#include "AstInterface_ROSE.h"
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <CommandOptions.h>
#include <stdexcept>
#include "OperatorAnnotation.h"
#include "ROSE_FALLTHROUGH.h"

#include "AstTraversal.h"
#include "astPostProcessing.h"
#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

using namespace Rose::Diagnostics;
Sawyer::Message::Facility AstInterface::mlog;
std::string get_type_name( SgType* t);


#define ASTNODE2PTR(n)  AstNodePtrImpl(n).get_ptr()
#define IS_FORTRAN_LANGUAGE() \
        SageInterface::is_Fortran_language()

#define NEW_EXPR_STMT(stmt,exp)  \
        stmt = new SgExprStatement(GetFileInfo(), exp); \
        exp->set_parent(stmt); stmt->set_endOfConstruct(stmt->get_file_info())

#define NEW_SYMBOL(sym, className, scope, name) \
        sym = new className(name); name->set_scope(scope); \
        scope->insert_symbol( name->get_name(), sym)

#define NEW_BLOCK(block) \
        block = new SgBasicBlock(GetFileInfo()); \
        block->set_endOfConstruct(block->get_file_info())

#define NEW_BLOCK1(block,stmt) \
        NEW_BLOCK(block); \
        block->append_statement(stmt); stmt->set_parent(block)

#define NEW_FUNCTION_REF(fr, fsym) \
        fr = new SgFunctionRefExp(GetFileInfo(), fsym); \
        fr->set_endOfConstruct(fr->get_file_info())

#define NEW_MFUNCTION_REF(fr, fsym) \
        fr = new SgMemberFunctionRefExp(GetFileInfo(), fsym); \
        fr->set_endOfConstruct(fr->get_file_info()); \
        fr->set_need_qualifier(false)

#define NEW_EXPR_LIST(explist)   \
        explist = new SgExprListExp(GetFileInfo()); \
        explist->set_endOfConstruct(explist->get_file_info())

#define NEW_FUNCTION_CALL(fcall, fref, args)  \
        fcall = new SgFunctionCallExp(GetFileInfo(), fref, args); \
        fcall->set_endOfConstruct(fcall->get_file_info()); \
        fref->set_parent(fcall); args->set_parent(fcall)

#define NEW_VAR_INIT(init, var, exp) \
        init = new SgAssignInitializer(GetFileInfo(), exp, exp->get_type() ); \
        init->set_endOfConstruct(init->get_file_info()); exp->set_parent(init); \
        var->set_initializer(init); exp->set_parent(init); init->set_parent(var) \

#define NEW_ASSIGN(exp, lhs, rhs) \
     exp = new SgAssignOp(GetFileInfo(), lhs, rhs); \
     exp->set_endOfConstruct(exp->get_file_info()); \
     lhs->set_parent(exp);  \
     rhs->set_parent(exp);

#define NEW_BIN_OP(op,className,lhs,rhs) \
    op = new className( GetFileInfo(), lhs, rhs); \
    op->set_endOfConstruct(op->get_file_info()); \
    lhs->set_parent(op); rhs->set_parent(op)


#define NEW_IF(r, cond, tbody)   \
    r = new SgIfStmt(GetFileInfo(), cond, tbody,new SgBasicBlock(GetFileInfo())); \
    r->set_has_end_statement(true); \
    r->set_endOfConstruct(r->get_file_info()); \
    r->get_false_body()->set_endOfConstruct(r->get_file_info()); \
    cond->set_parent(r); tbody->set_parent(r); r->get_false_body()->set_parent(r) 

#define NEW_IF_ELSE(r, cond, tbody, fbody)   \
    r = new SgIfStmt(GetFileInfo(), cond, tbody, fbody); \
    r->set_has_end_statement(true); \
    r->set_endOfConstruct(r->get_file_info()); \
    cond->set_parent(r); tbody->set_parent(r); fbody->set_parent(r) 

#define NEW_THIS_EXP(r, decl) \
    r = new SgThisExp(isSgClassSymbol(decl->get_symbol_from_symbol_table()), 0, 0); \
    r->set_endOfConstruct(decl->get_file_info()); \
    r->set_startOfConstruct(decl->get_file_info())

Sg_File_Info* GetFileInfo()
   {
  // DQ (3/8/2006): This is the easiest way to represent a transformation
  // since we have to both mark the file info object as a transformation
  // AND to be output in the code generation phase as well.
     return Sg_File_Info::generateDefaultFileInfoForTransformationNode();
   }



namespace {
DebugLog DebugVariable("-debugvariable");
DebugLog DebugScope("-debugscope");
DebugLog DebugDiff("-debugdiff");
DebugLog DebugDecl("-debugdecl");
static std::function<std::string(const SgFunctionDeclaration*)> function_name_mangling_;


AstNodePtr GetFunctionDecl( const AstNodePtr& _s)
{
    SgNode* s = AstNodePtrImpl(_s).get_ptr();
    int t = s->variantT();
    switch (t) {
    case V_SgFunctionDefinition: 
         return AstNodePtrImpl(isSgFunctionDefinition(s)->get_declaration());
    case V_SgTemplateFunctionRefExp:
      return AstNodePtrImpl(isSgTemplateFunctionDefinition(s)->get_declaration());
    case V_SgFunctionDeclaration:
    case V_SgMemberFunctionDeclaration:
    case V_SgTemplateMemberFunctionDeclaration:
    case V_SgProcedureHeaderStatement:
    case V_SgTemplateInstantiationFunctionDecl:
    case V_SgTemplateInstantiationMemberFunctionDecl:
        return _s;
    case V_SgMemberFunctionRefExp:
         return AstNodePtrImpl(isSgMemberFunctionRefExp(s)->get_symbol()->get_declaration());
    case V_SgNonrealRefExp:
         return AstNodePtrImpl(isSgNonrealRefExp(s)->get_symbol()->get_declaration());
    case V_SgFunctionSymbol:
          return AstNodePtrImpl(isSgFunctionSymbol(s)->get_declaration());
    case V_SgFunctionRefExp:
          return AstNodePtrImpl(isSgFunctionRefExp(s)->get_symbol()->get_declaration());
    case V_SgMemberFunctionSymbol:
         return AstNodePtrImpl(isSgMemberFunctionSymbol(s)->get_declaration());
    case V_SgConstructorInitializer:
         return AstNodePtrImpl(isSgConstructorInitializer(s)->get_declaration());
    case V_SgDotExp:
         return GetFunctionDecl( AstNodePtrImpl(isSgDotExp(s)->get_rhs_operand()));
    case V_SgVarRefExp:
         return AstNodePtrImpl(isSgVarRefExp(s)->get_symbol()->get_declaration());
    case V_SgNonrealDecl: return _s;
    default: 
        mlog[ERROR] << "Error: not recognizable function type : " << s->sage_class_name() << std::endl;
        mlog[ERROR] << " at " << s->get_file_info()->get_filenameString() << ":" << s->get_file_info()->get_line() << std::endl;
        mlog[ERROR] << s->unparseToString() << std::endl;
        ROSE_ABORT();
    }
    return AST_NULL;
}


//! Get a unique string name for a type, similar to qualified names in C++
std::string GetFunctionSignature( const AstNodePtr& /* f */, const std::string& fname, const AstInterface::AstTypeList& plist)
{
  std::stringstream fname_stream;
  fname_stream << fname;
  for ( AstInterface::AstTypeList::const_iterator p = plist.begin();
          p != plist.end();  ++p) {
      AstNodeType t = *p;
      std::string name;
      AstInterface::GetTypeInfo( t, &name);
      fname_stream << "_" << name;
  }
  DebugVariable([&fname,&fname_stream](){ return "Function signature:" + fname + " =>" + fname_stream.str(); });
  return fname_stream.str();
}

std::string StripGlobalQualifier(std::string name)
{
   while (name.size() > 1 && name[0] == ':' && name[1] == ':') {
      name = std::string(name.c_str()+2);
   }
   return name;
}

std::string StripQualifier(std::string name)
{
   unsigned i = name.size(); 
   for ( ; i > 0; --i) {
      if (name[i-1]==':' && i > 1 && name[i-2] == ':') break;
   }
   return std::string(name.c_str() + i);
}

bool DebugSymbol()
{
  static int r = 0;
  if (r == 0) {
    if (CmdOptions::GetInstance()->HasOption("-debugsymbol"))
       r = 1;
    else
       r = -1;
  }
  return r == 1;
}

SgScopeStatement* GetNullScope()
{
  static SgGlobal* global = 0;
  if (global == 0) {
    global = new SgGlobal(GetFileInfo());
  }
  return global;
}

inline bool HasNullParent(SgNode* n)
{
  return n->get_parent() == 0 || n->get_parent() == GetNullScope();
}

inline SgVarRefExp* ToVarRef( AstInterfaceImpl& fa, SgNode* exp)
{
  switch (exp->variantT()) {
  case V_SgVarRefExp:
     return isSgVarRefExp( exp );
  case V_SgInitializedName:
     {
      SgInitializedName* var = isSgInitializedName(exp);
      SgName varname = var->get_name();
      SgNode* r = fa.CreateVarRef( std::string(varname.str()), exp);
      r->set_parent(exp->get_parent());
      return isSgVarRefExp(r);
     }
  default: break;
  }
  return 0;
}

inline SgExpression* ToExpression( AstInterfaceImpl& fa, SgNode* s)
{
  SgExpression *exp = ToVarRef(fa, s);
  if (exp == 0) exp = isSgExpression(s);
  return exp;
}
//
//! Strip the casting operations to get to the real expression.
inline SgNode* SkipCasting(SgNode*  exp)
{
  SgCastExp* cast_exp = isSgCastExp(exp);
  if (cast_exp != NULL)
    {
      SgExpression* operand = cast_exp->get_operand();
      assert(operand != 0);
      return SkipCasting(operand);
    }
  else      
    return exp;
}


template <class const_iterator>
SgExprListExp*
AstNodeList2ExpressionList(const_iterator b, const_iterator e)
{
  SgExprListExp* NEW_EXPR_LIST(explist);
  for (const_iterator p = b; p != e; ++p) {
    SgExpression* e = isSgExpression(AstNodePtrImpl(*p).get_ptr());
    assert(e);
    explist->append_expression(e);
    e->set_parent(explist);
  }
  return explist;
}

SgStatement* ToStatement(SgNode* _stmts)
{
    SgStatement *stmts = isSgStatement(_stmts);
    if (stmts == 0) {
       SgExpression *exp = isSgExpression(_stmts);
       ASSERT_not_null(exp);
       NEW_EXPR_STMT(stmts,exp);
    }
    return stmts;
}

SgClassDefinition* GetClassDefn(SgClassDeclaration* classDecl)
{
  SgDeclarationStatement* decl = classDecl->get_definingDeclaration();
  ASSERT_not_null(decl);
  classDecl = isSgClassDeclaration(decl);
  ASSERT_not_null(classDecl);
  SgClassDefinition* classDefn = classDecl->get_definition();
  return classDefn;
}

// Strip leading "const" and tailing '&'
std::string StripParameterType( const std::string& name)
{
  std::string r = name;
  if (name.substr(0,5) == "const") 
     r = name.substr(5, name.size()-5);
  ROSE_ASSERT (!r.empty());
  size_t end = r.size()-1;
  if (r[end] == '&') {
       r[end] = ' ';
  }
  std::string result = "";
  for (size_t i = 0; i < r.size(); ++i) {
    if (r[i] != ' ')
      result.push_back(r[i]);
  }
  return result; 
} 

SgNode* CreateAssignment(AstInterfaceImpl& fa, SgExpression* lhsexp, SgExpression* rhsexp)
{
  //assert(HasNullParent(lhsexp));
  //assert(HasNullParent(rhsexp));
  SgExpression *exp = 0;
  SgType* lhstype = lhsexp->get_type(); 

  if (lhstype->variantT() == V_SgClassType) {
    SgClassType *lhstype1 = isSgClassType(lhstype);
    SgName classname = lhstype1->get_name();
    SgClassDeclaration *c = isSgClassDeclaration(fa.LookupNestedDeclaration( std::string(classname.str()), fa.get_scope(lhsexp)));
    assert (c != 0);
    SgExpressionPtrList args;
    args.push_back( rhsexp);
    SgMemberFunctionSymbol *f = fa.GetMemberFunc(c, "operator=", &args);
    if (f != 0) {
        SgMemberFunctionRefExp *NEW_MFUNCTION_REF(fr,f);
        SgExpression *NEW_BIN_OP(func, SgDotExp, lhsexp, fr);
        SgExprListExp *NEW_EXPR_LIST(argexp);
        SgExpressionPtrList &l = argexp->get_expressions();
        l = args;
        NEW_FUNCTION_CALL(exp, func, argexp);
    } 
  }
  if (exp == 0) { NEW_ASSIGN(exp,lhsexp, rhsexp); }
  return exp;
}

std::string unparseToString( SgNode* s)
{
  if (s == 0) return "";
  std::string r = "";
  switch(s->variantT()) {
  case V_SgName: return isSgName(s)->str();
  case V_SgVarRefExp:
    {
      SgVarRefExp *var = isSgVarRefExp( s );
      SgVariableSymbol *sb = var->get_symbol();
      r = r +  sb->get_name().str();
    }
    break;
  case V_SgInitializedName:
    {
      SgInitializedName* var = isSgInitializedName(s);
      r = r + var->get_name();
    }
    break;
  case V_SgProject:  
    {
      SgProject* sageProject = static_cast<SgProject*>(s);  
      for (int i = 0; i < sageProject->numberOfFiles(); ++i) {
        SgSourceFile* sageFile = isSgSourceFile(sageProject->get_fileList()[i]);
        r = r + unparseToString(sageFile);
      }
      break;
    }
  default: r = r + s->unparseToString();
  }
  return r;
}

}; // namespace

SgVariableSymbol* AstInterfaceImpl::
LookupVar( const std::string& name, SgNode* loc) {
  assert(loc != 0);
  const char* start = name.c_str();

  // check if it is a fully qualified name, if yes, use the special lookup function instead
  if (name.find("::")!=std::string::npos) {
    AstNodePtr result = LookupNestedDeclaration(name, loc);
    SgInitializedName* initname = isSgInitializedName(result.get_ptr());
    if (initname != 0) {
      SgVariableSymbol* varsym = isSgVariableSymbol(initname->search_for_symbol_from_symbol_table ());
      if (varsym == 0) {
         NEW_SYMBOL(varsym,SgVariableSymbol, initname->get_scope(),initname);
      }
      return varsym;
    }
    return 0;
  }
  {
    SgNamedType* t = isSgNamedType(loc);
    if (t != 0) {
      return LookupVar(name, t->getAssociatedDeclaration());
    }
  }
  {
    SgClassDeclaration* class_decl = isSgClassDeclaration(loc); 
    if (class_decl != 0) {
      return LookupVar(name, GetClassDefn(class_decl));
    }
  }
  SgClassDefinition *cdef = isSgClassDefinition(loc);
  if (cdef != 0) {
     SgVariableSymbol* r = dynamic_cast<SgVariableSymbol*>(cdef->lookup_symbol(start));
     if (DebugSymbol()) {
           if (r == 0) 
              std:: cerr << "failed to find variable " << start;
           else
              std:: cerr << "found variable " << start;
           std:: cerr << " in scope " << unparseToString(loc) << "\n";
           std::cerr << " symbols of which include: ";
           for (SgSymbol* p = cdef->first_any_symbol(); p != 0; p = cdef->next_any_symbol())
               std::cerr << p->get_name().str() << ";";
           std::cerr << "\n"; 
     }
     if (r != 0)
         return r;
     SgBaseClassPtrList& l = cdef->get_inheritances();
     for (SgBaseClassPtrList::iterator p = l.begin(); p != l.end(); ++p) {
        SgBaseClass* cur = *p;
        SgClassDeclaration *decl = cur->get_base_class();
        if (decl != 0) {
           SgClassDefinition *def = GetClassDefn(decl);
           assert(def != 0);
           r = LookupVar(name, def);
           if (r != 0)
             return r;
        }
      }
     return 0;
  }
  else {
     SgScopeStatement *loc_scope = isSgScopeStatement(loc);
     assert(loc_scope != 0);
     SgVariableSymbol* f = 0;
     do {
        f = dynamic_cast<SgVariableSymbol*>(loc_scope->lookup_symbol(start));
        if (DebugSymbol()) {
           if (f == 0) 
              std:: cerr << "failed to find variable ";
           else
              std:: cerr << "found variable ";
           std:: cerr << start << " in scope " << unparseToString(loc) << "\n"; 
        }
        if (loc_scope->variantT() == V_SgGlobal || f != 0)
             break;
        loc_scope = loc_scope->get_scope();
     }
     while ( loc_scope != 0 && f == 0);
     return f;
  }
}

SgType* AstInterfaceImpl::GetTypeInt()
{
  static SgType* typeint = 0;
  if (typeint == 0)
     typeint = new SgTypeInt();
  return typeint;
}

SgNode* AstInterfaceImpl::
LookupNestedDeclaration(const std::string& name, SgNode* loc) {
  int sz=name.size();
  assert (sz!=0); 
  assert (loc); 

  int pos=0; 
  // skip leading :: if they are present.
  if (sz>=2 && name[0]==':' && name[1]==':')
    pos=2; 

  assert (sz-2!=0); 

  // Save the current matching declarations in cur_results.
  // Save the declarations to sewarch in to_search_next.
  AstInterface::AstNodeList cur_results, to_search_next;
  to_search_next.push_back(SageInterface::getGlobalScope(loc));

  // split the name into segments
  std::string currentname; 
  DebugVariable([&name](){return "Looking for variable:" + name; });
  // Search for each scope name and advance the search accordingly.
  while (pos<= sz) {  // we reach the last + 1 pos, very tricky here!!
    if (name[pos] !=':' && pos< sz) { 
     // characters other than :, accumulate to current name
     // We have not yet reached the end of a scope name.
      currentname.push_back(name[pos++]);
      continue;
    } 
    // We have a complete scope name. Double checking.
    assert (currentname.size()!=0);
    // First, advance the given name to the next scope if needed. 
    if (pos < sz && name[pos]==':') {
        if (name[pos-1]!=':') { // this is the first : of ::
          assert(pos+1< sz && name[pos+1]==':'); 
          pos+=2;  // skip two chars
        }
    }
    else { 
       // last char. Advance pos to exit the surrounding while loop. 
        pos++; 
    }

    // Use decl_ptr_list as a work list to store all declarations to check.
    // Use matched_decls to save all declarations that match the current name.
    // Use new_decls to save new declarations to search for current name.
    AstInterface::AstNodeList decl_ptr_list, matched_decls;
    // Start from all matching declarations before reaching current name. 
    decl_ptr_list = to_search_next;
    cur_results.clear(); to_search_next.clear();
    // Now search for the scope that have the given scope name..
    DebugVariable([&currentname]() { return "Looking for name:" + currentname; });
    // Iterate until the list is empty.
    while (!decl_ptr_list.empty()) {
        // Pop out the current declaration.
        auto cur_decl= decl_ptr_list.back();
        decl_ptr_list.pop_back(); 

        DebugVariable([&cur_decl]() { return "processing decl:" + ((cur_decl==0)?"NULL":AstInterface::AstToString(cur_decl)); });
        std::string tmp_name;
        AstInterface::AstNodeList new_decls;
        if (AstInterface::IsBlock(cur_decl, &tmp_name, &new_decls)) {
          size_t i = tmp_name.rfind("::");
          if (i < tmp_name.size()) { // strip qualified names.
             tmp_name = tmp_name.substr(i+2, tmp_name.size()-i+2);
          }
          DebugVariable([&tmp_name]() { return "Is Block " + tmp_name;});
          if (tmp_name == currentname) {
            cur_results.push_back(cur_decl); 
            DebugVariable([&currentname]() { return "Found name : " + currentname; });
            to_search_next.insert(to_search_next.end(), new_decls.begin(), new_decls.end());
          } else {
             decl_ptr_list.insert(decl_ptr_list.end(), new_decls.begin(), new_decls.end());
          }
        }
        else if (SgVariableDeclaration* var_decl = isSgVariableDeclaration(cur_decl.get_ptr())) {
          DebugVariable([]() { return "Is variable declaration."; });
          auto vars = var_decl->get_variables();
          decl_ptr_list.insert(decl_ptr_list.end(), vars.begin(), vars.end());
        } else if (SgInitializedName* initname  = isSgInitializedName(cur_decl.get_ptr())) {
             DebugVariable([&initname]() { return "name:" + initname->get_name().getString(); });
             if (initname->get_name().getString() == currentname) {
               cur_results.push_back(cur_decl);
               DebugVariable([&currentname]() { return "Found name : " + currentname; });
             }
         } else if (auto* typedef_decl = isSgTypedefType(cur_decl.get_ptr())) {
             if (typedef_decl->get_name().str() == currentname) {
                cur_results.push_back(cur_decl); 
                DebugVariable([&currentname]() { return "Found name : " + currentname; });
                auto* decl1 = typedef_decl->get_base_type();
                to_search_next.push_back(decl1);
             } 
         } else {
             DebugVariable([&cur_decl]() { return "Not looking at:" + AstInterface::AstToString(cur_decl); });
         }
    }
    if (cur_results.empty()) {
       // The search fails. Output a warning and return NULL.
       std::cerr<<"Warning: cannot find qualified name for "<< currentname << "\n";
       return NULL; // cannot find the declaration
    } else {
      // reset scope name to accept next name   
      currentname = ""; 
    }
  }
  assert(!cur_results.empty());
  return cur_results[0].get_ptr();
}

void AstInterface :: SetRoot( const AstNodePtr& root)
{ impl->set_top(AstNodePtrImpl(root).get_ptr()); }

AstNodePtr AstInterface :: GetRoot() const 
{ return AstNodePtrImpl(impl->get_scope(0)); }

void AstInterface :: AttachObserver( AstObserver* ob)
{
  impl->AttachObserver(ob);
}

void AstInterface :: DetachObserver( AstObserver* ob)
{
  impl->DetachObserver(ob);
}

void AstInterface::initDiagnostics()
{
  static bool initialized = false;
  if (!initialized)
    {
      initialized = true;
      Rose::Diagnostics::initAndRegister(&mlog, "Rose::AstInterface");
      mlog.comment("Qing's side effect analysis");
    }
}

AstNodePtr AstInterface::GetFunctionDefinition( const AstNodePtr &n, std::string* name)
{
  AstNodePtr r=n;
  while (r != AST_NULL && !IsFunctionDefinition(r, name)) {
     r = GetParent(r);
  }
  return r;
}

AstNodePtr AstInterface::GetFunctionDefinitionFromDeclaration( const AstNodePtr &_decl) {
  SgFunctionDeclaration* decl = isSgFunctionDeclaration(_decl.get_ptr()); 
  SgFunctionDefinition* def = decl->get_definition();
  if (def == 0) {
      auto* decl1 = decl->get_definingDeclaration();
      if (decl1 != 0) {
         auto* def_decl = isSgFunctionDeclaration(decl1);
         assert(def_decl != 0);
         def = def_decl->get_definition();
      }
  }
  return def;
}

class SageSetTransformation: public AstTopDownProcessing< AstNodePtrImpl >
{
   AstNodePtrImpl evaluateInheritedAttribute(SgNode* astNode, AstNodePtrImpl)
     { 
         Sg_File_Info* r = astNode->get_file_info();
         r->setTransformation();
         r->setCompilerGenerated();
         r->setOutputInCodeGeneration();
       return astNode;
     }
  public:
   SageSetTransformation() {}
   void operator() ( SgNode* node )
       {
         AstTopDownProcessing<AstNodePtrImpl>::traverse(node, node->get_parent());
       }
};
class SageResetParent : public AstTopDownProcessing< AstNodePtrImpl >
{
   AstNodePtrImpl evaluateInheritedAttribute(SgNode* astNode, AstNodePtrImpl inheritedValue)
     { if ( inheritedValue != 0) { 
          //assert(astNode->get_parent() == inheritedValue || astNode->get_parent() == 0);
          astNode->set_parent( inheritedValue.get_ptr() );
       }
       return astNode;
     }
  public:
   SageResetParent() {}
   void operator() ( SgNode* node )
       {
         AstTopDownProcessing<AstNodePtrImpl>::traverse(node, node->get_parent());
       }
};


/* QY: 7/2011 This function is not invoked anywhere
SgSymbol* AddDecls( AstInterfaceImpl* scope, const SgDeclarationStatementPtrList& decls)
{
     SgSymbol* result = 0;
     for (SgDeclarationStatementPtrList::const_iterator p = decls.begin(); p != decls.end(); ++p) {
         SgDeclarationStatement* cur = *p;
         cur->set_file_info( GetFileInfo());
         SgFunctionDeclaration* d1 = isSgFunctionDeclaration(cur);
         if (d1 != 0) {
            result = scope->AddFunc(d1);
         }
         else {
            SgVariableDeclaration* d2 = isSgVariableDeclaration(cur);
            if (d2 != 0) {
                result = scope->AddVar(d2);
            }
            else  {
               SgClassDeclaration* d3 = isSgClassDeclaration(cur);
               if (d3 != 0)
                   result = scope->AddClass(d3);
               else
                   assert(false);
            }
         }
    }
    return result;
}
*/

SgMemberFunctionSymbol * AstInterfaceImpl::
GetMemberFunc(SgClassDeclaration* decl, 
               const std::string& funcname, SgExpressionPtrList* args)
{
  SgName classname = decl->get_name();
  SgClassDefinition * def = GetClassDefn(decl);
  if (def == 0) {
      std::cerr << "no definition in locating member function " << funcname << std::endl;
       return 0;
  }
  const char* start = funcname.c_str();
  if (args == 0) {
     SgFunctionSymbol* f = def->lookup_function_symbol(start);
     if (f != 0) {
        SgMemberFunctionSymbol* mf = isSgMemberFunctionSymbol(f);
        assert(mf != 0);
        return mf;
     }
     else
     {
       std::cerr <<"AstInterface.C GetMemberFunc() cannot find a symbol for "<<funcname<<" within a class "<<classname<< std::endl;
       return 0;
     }
  }
  else {
     SgDeclarationStatementPtrList & decls = def->get_members ();
     for (SgDeclarationStatementPtrList::iterator p = decls.begin(); p != decls.end();
          ++p) { 
        SgDeclarationStatement *cur = *p;
        if (cur->variantT() != V_SgMemberFunctionDeclaration)
          continue;
        SgMemberFunctionDeclaration *md = isSgMemberFunctionDeclaration(cur);
        SgName name = md->get_name();
        if ( std::string(name.str()) != funcname)
            continue;
        SgInitializedNamePtrList &pars = md->get_args ();
        if (pars.size() != args->size())
           continue;
        SgInitializedNamePtrList::iterator pp = pars.begin();
        SgExpressionPtrList::iterator pa = args->begin();
        bool match = true;
        for ( ; pp != pars.end(); ++pp, ++pa) {
           SgType* tp = (*pp)->get_type();
           SgType* ta = (*pa)->get_type();
           std::string partype, argtype;
           GetTypeInfo(tp, 0, &partype);
           GetTypeInfo(ta, 0, &argtype);
           if (partype != argtype) {
              match = false;
              break;
           }
        }
        if (match) {
           //QY:1/7/08: this should be only temporary. should not create a new symbol if the symbol is already in the symbol table (no search mechanism available yet?)
           SgMemberFunctionSymbol* f = new SgMemberFunctionSymbol(md);
           return f;
        }
     }
  }
  return 0;
}

void AstInterfaceImpl:: set_top( SgNode* top) 
  { 
      global = 0;
      scope = 0;
      if (top != 0) {
        scope = isSgScopeStatement(top);
        if (scope == 0) {
          auto* t = GetScope(top);
          while (scope == 0 && t != 0) {
             scope = isSgScopeStatement(t);
             t = GetScope(t);
         }
        }
        SgStatement *cur = scope;   
        while (cur != 0 && global == 0) {
          global = isSgGlobal(cur);
          if (cur->get_parent() != 0)
             cur = cur->get_scope();
          else
             cur = 0;
        } 
      }
  }
 
SgFunctionSymbol* AstInterfaceImpl::LookupFunction(const char* start, SgScopeStatement* in_scope) {
     assert(in_scope!=0);
     SgScopeStatement *cur = in_scope;
     SgFunctionSymbol* f = 0;
     do {
        f = cur->lookup_function_symbol(start);
        if (DebugSymbol()) {
           if (f == 0) 
              std::cerr << "failed to find function symbol " << start << " in scope " << cur->sage_class_name() << " : " << cur->unparseToString() << "\n";
           else
              std::cerr << "found function symbol " << start << " in scope " << cur->sage_class_name() << " : " << cur->unparseToString() << "\n";
        }
       
        if (cur->variantT() == V_SgGlobal)
             break;
        assert(cur->get_scope() != cur);
        cur = cur->get_scope();
     }
     while ( cur != 0 && f == 0); 
     if (DebugSymbol()) {
        if (cur == 0 || cur == GetNullScope()) 
           std::cerr << "exit with cur = " << (cur == 0? "NULL" : "NULL scope") << "\n";
        else 
           std::cerr << "exit with cur = " << cur->sage_class_name() << "\n";
     }
     return f;
    }

SgClassSymbol* AstInterfaceImpl:: AddClass( SgClassDeclaration* d) 
  {
    assert(global != 0);
    SgDeclarationStatementPtrList& l = global->get_declarations();
    l.insert( l.begin(), d);
    SgClassSymbol *NEW_SYMBOL(c,SgClassSymbol, global,d);
    return c;
  }

SgFunctionSymbol* AstInterfaceImpl:: AddFunc( SgFunctionDeclaration *d)
   {
     assert(global != 0);
     SgDeclarationStatementPtrList& l = global->get_declarations();
     l.insert( l.begin(), d);
     SgFunctionSymbol* NEW_SYMBOL(f,SgFunctionSymbol, global, d);
     d->set_parent(global);
     return f;
   }

SgMemberFunctionSymbol* AstInterfaceImpl:: 
AddMemberFunc( SgClassDefinition *def, SgMemberFunctionDeclaration *d)
   {
     SgMemberFunctionSymbol* NEW_SYMBOL(f, SgMemberFunctionSymbol, def, d);
     d->set_parent(def);
     return f;
   }

void AstInterfaceImpl::
SaveVarDecl(SgVariableDeclaration *d, SgScopeStatement* curscope)
   {
     if (curscope == 0)
        curscope = scope;
     assert(curscope != 0);

     d->set_parent(curscope);
     newVarList.push_back(std::pair<SgScopeStatement*,SgVariableDeclaration*>(curscope, d));
}

SgVariableSymbol* AstInterfaceImpl::
InsertVar(SgInitializedName *d, SgScopeStatement* curscope)
   {
     if (curscope == 0) curscope = scope;
     assert(curscope != 0);
     SgVariableSymbol *NEW_SYMBOL(v, SgVariableSymbol, curscope, d); 
     return v;
   }

SgVariableSymbol* AstInterfaceImpl::
NewVar( SgType* type, const std::string& _name, bool makeunique, bool delayDecl, SgExpression* initexp, SgScopeStatement* loc)
{ 
  std::string varname = _name;
  if (varname == "" ) {
     varname = "_var_";
     makeunique = true;
  }
  if (makeunique) {
     char buf[20]; 
     snprintf(buf, sizeof(buf), "%d", newVarIndex);
     varname = varname + std::string(buf);
     ++newVarIndex; 
  }

  SgVariableSymbol *v = LookupVar(varname, (loc==0)? scope : loc);
  if (v == 0) {
     //variable declaration has not been inserted
     SgName name(varname.c_str());
     SgType *t = isSgType( type);
     assert(t != 0);
     SgInitializedName *def = new SgInitializedName( GetFileInfo(), name,  t, 0, 0, 0, 0);
     def->set_endOfConstruct(def->get_file_info());
     v = InsertVar(def, loc);
     SgVariableDeclaration *decl = new SgVariableDeclaration( GetFileInfo());
     decl->set_endOfConstruct(decl->get_file_info());

     if (initexp != 0) {
        SgAssignInitializer *NEW_VAR_INIT(init, def, initexp);
        decl->append_variable(def, init);
     }
     else decl->append_variable(def, 0);
     def->set_parent(decl);
     if (delayDecl) SaveVarDecl(decl,loc);
     else if (loc != 0) 
        { loc->insertStatementInScope(decl, true); decl->set_parent(loc); }
     else ROSE_ABORT();

  }
  else {
     std::cerr << "Warning: new var has already been initialized: " << varname << "\n";
  }
  return v;
}

SgFunctionSymbol* AstInterfaceImpl::GetFunc( const std::string& name)
{
  const char* start = name.c_str();
  SgFunctionSymbol* f = LookupFunction(start, scope);
  return f;
}

SgFunctionSymbol* AstInterfaceImpl::
NewFunc( const std::string& name, SgType*  rtype, const std::list<SgInitializedName*>& args)
{
  const char *start = name.c_str();
  SgFunctionType *ft = new SgFunctionType(rtype, false);
  SgFunctionDeclaration  *d = new SgFunctionDeclaration(GetFileInfo(), start, ft);
  for (std::list<SgInitializedName*>::const_iterator p = args.begin(); p != args.end();
       ++p) {
     SgInitializedName* cur = *p;
     d->append_arg(cur);
  }
  return AddFunc(d);
}

SgClassSymbol* AstInterfaceImpl :: NewClass( const std::string& classname)
{
 if (DebugSymbol())
    std:: cerr << "adding new class " << classname << "\n";
 SgClassDeclaration *decl = new SgClassDeclaration( GetFileInfo(), classname.c_str());
 
 return AddClass(decl);
}

SgMemberFunctionSymbol * AstInterfaceImpl :: 
NewMemberFunc( SgClassDeclaration* classDecl, const std::string& name, SgType*  rtype, 
               const std::list<SgInitializedName*>& args)
{
  const char * start = name.c_str();
  SgClassDefinition* classDefn = GetClassDefn(classDecl);
  if (classDefn == 0) {
     if (DebugSymbol())
         std:: cerr << " creating new class defn " << classDecl->get_name().str() << "when member function " << start << "was not found. \n"; 
     classDefn = new SgClassDefinition(GetFileInfo(), classDecl);
     classDefn->set_endOfConstruct(classDefn->get_file_info());
     assert(scope != 0);
     classDecl->set_parent(scope);
     classDecl->set_definition(classDefn);
  }

  SgMemberFunctionType *ft = new SgMemberFunctionType(rtype, false);
  SgMemberFunctionDeclaration  *d = new SgMemberFunctionDeclaration(GetFileInfo(), start, ft, 0);
  d->set_scope(classDefn);
  for (std::list<SgInitializedName*>::const_iterator p = args.begin(); p != args.end();
       ++p) {
     SgInitializedName* cur = *p;
     d->append_arg(cur);
  }
  return AddMemberFunc( classDefn, d);
}

std::string AstInterface::
GetGlobalUniqueName(const AstNodePtr& _scope, std::string expname, bool do_not_add_file_name) {
  SgNode* scope = AstNodePtrImpl(_scope).get_ptr();
  assert(scope != 0);
  std::string result = expname;
  std::string scopename = expname;
  if (function_name_mangling_ && IsFunctionDefinition(_scope)) { 
    SgFunctionDeclaration* d = isSgFunctionDeclaration(GetFunctionDecl(_scope).get_ptr());
    if(d == 0) { return "_UNKNOWN_"; }
    scopename = function_name_mangling_(d);
    if (expname != "") return scopename + "::" + expname;
    return scopename;
  }
  while (scope != 0 && scope->variantT() != V_SgGlobal) {
      DebugVariable([&scope](){ return "GetGlobalUniqueName:scope:" + AstToString(scope); });
      if (IsBlock(scope, &scopename) && scopename != "" && result.find(scopename+"::") >= result.size()) {
            DebugVariable([&scopename](){ return "GetGlobalUniqueName:scope_name:" + scopename; });
            if (result == "") result = scopename;
            else {
            auto result_in_scopename_index = scopename.find(result);
            bool result_in_scopename = result_in_scopename_index < scopename.size(); 
            // Check that result is indeed part of the scope name on both ends (b0 and b1).
            bool b0_is_good = result_in_scopename_index == 0 ||
                              (result_in_scopename && scopename[result_in_scopename_index-1] == ':');
            bool b1_is_good = result_in_scopename && 
                (result_in_scopename_index + result.size() == scopename.size() || 
                  scopename[result_in_scopename_index + result.size()] == '_');
            if (b0_is_good && b1_is_good) result = scopename;
            else result = scopename + "::" + result;
            }
      } 
      scope = AstInterfaceImpl::GetScope(scope);
  }
  if (!do_not_add_file_name) {
    if (scopename == "main" || CmdOptions::GetInstance()->HasOption("-global_via_filename")) {
      std::string filename = scope->get_file_info()->get_filenameString();
      auto location = filename.rfind("/");
      if (location < filename.size()) {
         filename = filename.substr(location+1);
      }
      if (result.find(filename) >= result.size()) {
         result = filename + "::" + result;
      }
    }
  }
  result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
  return result;
}



bool AstInterface::IsExprStmt(const AstNodePtr& n, AstNodePtr* exp)
{
  SgExprStatement* s = isSgExprStatement((SgNode*)n.get_ptr());
  if (s == 0) return false;
  if (exp != 0) *exp = s->get_expression();
  return true;
}

std::string AstInterface::toString (OperatorEnum op)
{
    const char* nameList[] = { 
        "OP_NONE", 
        "UOP_MINUS", "UOP_ADDR", "UOP_DEREF", 
        "UOP_ALLOCATE", 
        "UOP_NOT",
        "UOP_CAST", "UOP_INCR1", "UOP_DECR1", "UOP_BIT_COMPLEMENT",
        "BOP_DOT_ACCESS", "BOP_ARROW_ACCESS", 
        "BOP_TIMES", "BOP_DIVIDE", "BOP_MOD", "BOP_PLUS", "BOP_MINUS", 
        "BOP_EQ", "BOP_LE", "BOP_LT", "BOP_NE", "BOP_GT", "BOP_GE", 
        "BOP_AND", "BOP_OR", "BOP_BIT_AND", "BOP_BIT_OR", "BOP_BIT_RSHIFT", 
        "BOP_BIT_LSHIFT", "OP_ARRAY_ACCESS", "OP_ASSIGN", "OP_UNKNOWN"}; 
   return std::string(nameList[op]);
}

std::string AstInterface::unparseToString( const AstNodePtr& n)
{ 
  SgNode* s = (SgNode*)n.get_ptr();
  return ::unparseToString(s);
}

std::string AstInterface::AstTypeToString( const AstNodePtr& n) {
  std::string res;
  switch (n.get_type()) {
   case AstNodePtr::SpecialAstType::NULL_AST:  res = "_NULL_";  break;
   case AstNodePtr::SpecialAstType::NULL_AST_ARRAY_INDEX:  res = "_NULL_ARRAY_IDX";  break;
   case AstNodePtr::SpecialAstType::NULL_AST_FIELD_INDEX:  res = "_NULL_FIELD_IDX";  break;
   case AstNodePtr::SpecialAstType::UNKNOWN_AST:  res = "_UNKNOWN_";  break;
   case AstNodePtr::SpecialAstType::UNKNOWN_FUNCTION_CALL:  
        res = "_UNKNOWN_FUNCTION_CALL_"; break;
   case AstNodePtr::SpecialAstType::UNKNOWN_PTR_REF:  
        res =  "_UNKNOWN_PTR_REF_"; break; 
   case AstNodePtr::SpecialAstType::SG_AST: res = ""; break;
   case AstNodePtr::SpecialAstType::GLOBAL_SIGNATURE: 
        res = "_GLOBAL_"+n.get_signature(); break;
   default:
      std::cerr << "Error: Unhandled case." << "\n";
      assert(0);
  }
  return res;
}

std::string AstInterface::AstToString( const AstNodePtr& n, bool withClassName)
{ 
  std::string res = AstTypeToString(n);
  SgNode* s = (SgNode*)n.get_ptr();
  if (s == 0) return res;
  if (withClassName)
      res = res + std::string(s->sage_class_name()) + ":";
  res = res + ::unparseToString(s);
  return res;
}

// Return "@line_number:column_number" for an AST node  
// Used for debugging or pretty-printing an node
std::string AstInterface::getAstLocation(const AstNodePtr& _s)
{
   SgNode* s = AstNodePtrImpl(_s).get_ptr();
  if (s == 0) 
     return "";
  std::string r = "";

  // Add line:column info.
  Sg_File_Info * fileInfo = s->get_file_info();
  std::stringstream sline, scol;
  sline<<fileInfo->get_line();
  scol<<fileInfo->get_col();
  r = r+"@"+sline.str()+":"+scol.str();
  return r;
}

void AstInterface::FreeAstTree(const AstNodePtr&)
{ }

void NotifyTreeCopy ( AstInterfaceImpl& fa, const AstNodePtr& _orig, const AstNodePtr& _n) 
{
  AstNodePtrImpl orig(_orig), n(_n);
  std::vector<SgNode*> childvec = orig->get_traversalSuccessorContainer();
  std::vector<SgNode*> childvec1 = n->get_traversalSuccessorContainer();
  assert( childvec.size() == childvec1.size());
  for (size_t i = 0; i < childvec.size(); ++i) {
      AstNodePtrImpl c = childvec[i], c1 = childvec1[i];
      if (c != c1)
         NotifyTreeCopy( fa, c, c1);
  } 
  CopyAstRecord info(fa, orig, n);
  fa.Notify(info);
};

bool AstInterface :: get_fileInfo(const AstNodePtr& _n, std:: string* fname, int* lineno )
{
  SgNode* n = AstNodePtrImpl(_n).get_ptr();
  Sg_File_Info *f =n->get_file_info(); 
  if (f == 0) {
    return false;
  }
  if (fname != NULL) {
     *fname = f->get_filename();
  }
  if (lineno != NULL) 
     *lineno = f->get_line();
  return true;
}

AstNodePtr AstInterface :: CopyAstTree( const AstNodePtr &_orig) 
{
 SgNode* orig = AstNodePtrImpl(_orig).get_ptr();
 if (orig == 0) {
    return _orig;
 }
 if (orig->variantT() == V_SgInitializedName) {
   AstNodePtrImpl r(ToVarRef(*impl, orig));
   return r;
 }
 SgTreeCopy copyOption;
 SgNode* r = orig->copy( copyOption);
 if ( impl->NumberOfObservers() )
    NotifyTreeCopy( *impl, _orig, AstNodePtrImpl(r));
 return AstNodePtrImpl(r);
}

AstInterface::AstNodeList AstInterface :: GetChildrenList( const AstNodePtr &_n)
{
  SgNode* n = AstNodePtrImpl(_n).get_ptr();
  AstNodeList childlist;
   const std::vector<SgNode*>& childvec = n->get_traversalSuccessorContainer();
   for (size_t i = 0; i < childvec.size(); ++i) {
      childlist.push_back(childvec[i]);
   }
  return childlist;
}

void AstInterface :: SetParent(const AstNodePtr& n, const AstNodePtr& p)
{
  SgNode* node = AstNodePtrImpl(n).get_ptr();
  assert(node != NULL);
  SgNode* parent = AstNodePtrImpl(p).get_ptr();
  node->set_parent(parent);
}

AstNodePtr AstInterface :: GetParent( const AstNodePtr &n)
{
  AstNodePtrImpl node(n);
  if (node == GetRoot())
      return AST_NULL;
  return AstNodePtrImpl(node->get_parent());
}

bool AstInterface:: IsDecls( const AstNodePtr& _s) 
        { 
          AstNodePtrImpl s(_s);
          switch (s->variantT()) {
            case V_SgVariableDeclaration:
            case V_SgClassDeclaration:
            case V_SgFunctionDeclaration:
            case V_SgDeclarationStatement:
            case V_SgEnumDeclaration:
            case V_SgTypedefDeclaration: 
            case V_SgTemplateDeclaration:
                 return true;
            default:
                 return false;
            }
          }
bool AstInterface:: IsStatement( const AstNodePtr& _s)
{
  SgNode* s = AstNodePtrImpl(_s).get_ptr();
  return isSgStatement(s) != 0;
}

bool AstInterface::IsExecutableStmt( const AstNodePtr& _s) 
{ 
  // jichi(9/11/2009): Add in support for fortran loops.
  AstNodePtrImpl s(_s);
  switch (s->variantT()) {
  case V_SgFortranDo:
  //case V_SgFortranNonBlockedDo:        // This kind of Fortran block is temporarily not supported.

  case V_SgForStatement:
  case V_SgCaseOptionStmt:
  case V_SgExprStatement:
  case V_SgExpression:
  case V_SgGotoStatement: 
  case V_SgIfStmt: 
  case V_SgWhileStmt:
  case V_SgDoWhileStmt:
  case V_SgTryStmt:
  case V_SgBreakStmt:
  case V_SgContinueStmt:
  case V_SgReturnStmt:
  case V_SgSpawnStmt:
  //case V_SgVariableDeclaration: 
  // QY: Do not consider vardecl as executable or it will break loopProcessor
    return true;

  case V_SgLabelStatement:
    // jichi (10/9/2009): Disable process of empty Fortran label statement.
    // Mostly the node is replaced from SgContinueStmt paired with FortranDo.
    if (IS_FORTRAN_LANGUAGE()
        && isSgLabelStatement(s.get_ptr())->get_statement() == NULL)
      return false;
    else
      return true;

  default:
    return false;
  }
} 

AstNodePtr AstInterface::GetPrevStmt( const AstNodePtr& s)
{
  SgNode* n = AstNodePtrImpl(s).get_ptr();
   SgNode *p = n->get_parent();
   assert(p != 0);
   std::vector<SgNode*> childvec = p->get_traversalSuccessorContainer();
   size_t i = 0;
   for (; i < childvec.size(); ++i) 
       if (childvec[i] == n)
           break;
   if (i == 0) 
     return AST_NULL;
   else {
     AstNodePtrImpl r = childvec[i-1];
       return r;    
   }
}

AstNodePtr AstInterface::GetNextStmt( const AstNodePtr& s)
{
   SgNode* n = AstNodePtrImpl(s).get_ptr();
   SgNode *p = n->get_parent();
   assert(p != 0);
   std::vector<SgNode*> childvec = p->get_traversalSuccessorContainer();
   size_t i = 0;
   for (; i < childvec.size(); ++i)
       if (childvec[i] == n)
           break;
   if (i == childvec.size()-1)
     return AST_NULL;
   else {
     AstNodePtrImpl r = childvec[i+1];
       return r;
   }
}

bool AstInterface::IsIf( const AstNodePtr& _s, AstNodePtr* cond,
                                AstNodePtr* truebody, AstNodePtr* falsebody) 
{ 
  SgNode* s = AstNodePtrImpl(_s).get_ptr();
  int t = s->variantT();
  switch (t) {
  case V_SgIfStmt:
    {
      SgIfStmt *is = isSgIfStmt(s);
      if (cond != 0)
        *cond = AstNodePtrImpl(is->get_conditional());
      if (truebody != 0)
        *truebody = AstNodePtrImpl(is->get_true_body());
      if (falsebody != 0)
        *falsebody = AstNodePtrImpl(is->get_false_body());
    }
    break;
  case V_SgCaseOptionStmt:
    {
      SgCaseOptionStmt* cs = isSgCaseOptionStmt(s);
      if (cond != 0)
        *cond = AstNodePtrImpl(cs->get_key());
      if (truebody != 0)
        *truebody = AstNodePtrImpl(cs->get_body());
      if (falsebody != 0)
        *falsebody = AST_NULL;
    }
    break;
  case V_SgConditionalExp:
    {
      SgConditionalExp *is = isSgConditionalExp(s);
      if (cond != 0)
        *cond = AstNodePtrImpl(is->get_conditional_exp());
      if (truebody != 0)
        *truebody = AstNodePtrImpl(is->get_true_exp());
      if (falsebody != 0)
        *falsebody = AstNodePtrImpl(is->get_false_exp());
    }
    break;
  default:
    return false;
  }
  return true;
}

bool AstInterface::IsLabelStatement( const AstNodePtr& _s)
{
  AstNodePtrImpl s(_s);
  return s->variantT() == V_SgLabelStatement;
}

bool AstInterface::
IsReturn(const AstNodePtr& _s, AstNodePtr* val)
{
  SgNode* s = AstNodePtrImpl(_s).get_ptr();
  switch (s->variantT()) {
  case V_SgReturnStmt: 
      if (val != 0) {
          *val = AstNodePtrImpl(isSgReturnStmt(s)->get_expression());
      }
      return true;
  default: return false;
  }
}

bool AstInterface::
IsGoto( const AstNodePtr& _s, AstNodePtr* dest) 
{ 
  // TODO jichi(9/11/2009): Add in support for fortran loops.
  SgNode* s = AstNodePtrImpl(_s).get_ptr();
  switch (s->variantT()) {
  case V_SgGotoStatement:
    if (dest != 0) {
      SgLabelStatement *label = isSgGotoStatement(s)->get_label();
      *dest = AstNodePtrImpl(label); 
    }
    break;
  case V_SgReturnStmt: 
    if (dest != 0) {
      SgNode *scope = NULL;
      for (scope = s->get_parent(); 
           ((scope != NULL) && (scope->variantT() != V_SgFunctionDefinition));
           scope = scope->get_parent()){
        assert(scope != NULL);
      }
      *dest = AstNodePtrImpl(scope);
    }
    break;
  case V_SgContinueStmt:
    // jichi (10/9/2009): Add in FortranDo support
    if (dest != 0) {
      SgNode* scope = 0;
      for (scope = s->get_parent(); ; scope = scope->get_parent()){
        int t = scope->variantT();
        if (t == V_SgForStatement || t == V_SgWhileStmt || 
            t == V_SgDoWhileStmt)
          break;
      }
      if (scope->variantT() == V_SgFortranDo)
        return false;

      if (scope->variantT() == V_SgForStatement)
        scope = isSgForStatement(scope)->get_increment();
      *dest = AstNodePtrImpl(scope);
    }
    break;
  case V_SgBreakStmt:
    if (dest != 0) {
      SgNode* scope = 0;
      for (scope = s->get_parent(); ; scope = scope->get_parent()){
        int t = scope->variantT();
        if (t == V_SgForStatement || t == V_SgWhileStmt || 
            t == V_SgDoWhileStmt || t == V_SgSwitchStatement)
          break;
      }
      *dest =  AstNodePtrImpl(scope);
    }
    break;
  default: return false;
  } 
  return true;
}
// goto the point before destination
bool AstInterface::IsGotoBefore( const AstNodePtr& _s)
{ 
  AstNodePtrImpl s(_s);
  switch (s->variantT()) {
  case V_SgGotoStatement:
  case V_SgContinueStmt:
    return true;
  default: return false;
  }
}
bool AstInterface::IsGotoAfter( const AstNodePtr& _s)
{ 
  AstNodePtrImpl s(_s);
  switch (s->variantT()) {
  case V_SgReturnStmt:
  case V_SgBreakStmt:
    return true;
  default: return false;
  }
}

//! Returns whether t is a function type and if yes, returns its parameter
//! types and return type.
bool AstInterface:: IsFunctionType( const AstNodeType& _t,
                       AstTypeList* paramtypes, AstNodeType* returntype) {
    SgType* t = AstNodeTypeImpl(_t).get_ptr(); 
    DebugVariable([t](){ return "IsFunctionType:" + t->class_name(); });
    SgFunctionType* ftype = isSgFunctionType(t);
    if (ftype != 0) {
        if (paramtypes != 0) {
          SgMemberFunctionType* mf_type = isSgMemberFunctionType(ftype);
          if (mf_type != 0) {
             paramtypes->push_back(AstNodeTypeImpl(mf_type->get_class_type()));
          }  
          SgTypePtrList atypes = ftype->get_arguments();
          for (SgTypePtrList::const_iterator p = atypes.begin(); p != atypes.end(); ++p) {
            paramtypes->push_back(AstNodeTypeImpl(*p));
          }
        }
        if (returntype != 0)
           *returntype = AstNodeTypeImpl(ftype->get_return_type());
        return true;
    }
    SgTypedefType* dtype = isSgTypedefType(t);
    if (dtype != 0) {
       return IsFunctionType(AstNodeTypeImpl(dtype->get_base_type()), paramtypes, returntype);
    }
    SgPointerType* ptype = isSgPointerType(t);
    if (ptype != 0) {
       return IsFunctionType(AstNodeTypeImpl(ptype->get_base_type()), paramtypes, returntype);
    }
    return false;
}


bool AstInterface::
IsFunctionDefinition(  const AstNodePtr& _s, std:: string* name,
                    AstNodeList* params, AstNodeList* outpars, AstNodePtr* body,
                    AstTypeList* paramtype, AstNodeType* returntype, 
                    bool use_global_unique_name, bool skip_function_declaration) 

{
  SgNode* s = AstNodePtrImpl(_s).get_ptr();
  if (s == 0) return false;
  SgFunctionParameterList *l = 0;
  SgCtorInitializerList *ctor = 0;
  SgNode* d = s;
  AstNodePtr def;
  if (s->variantT() ==  V_SgFunctionDefinition) 
    {
      auto* s_def =  isSgFunctionDefinition(s);
      def = s_def;
      d = s_def->get_declaration();
      ctor = s_def->get_CtorInitializerList();
  }
  
  switch (d->variantT()) {
  case V_SgTemplateInstantiationFunctionDecl: 
  case V_SgProcedureHeaderStatement:
  case V_SgFunctionDeclaration: 
    { 
      SgFunctionDeclaration *decl = isSgFunctionDeclaration(d);
      if (def == AST_NULL && skip_function_declaration && decl->get_definition() == 0) {
           // Do non consider a function declaration as definition
           return false;
      }
      if (returntype != 0)
        *returntype = AstNodeTypeImpl(decl->get_type()->get_return_type());
      if (name != 0) 
        *name =  std::string(decl->get_name().str());
      if (paramtype != 0 || params != 0) 
        l = decl->get_parameterList();
      if (def == AST_NULL) {
         def = AstInterface::GetFunctionDefinitionFromDeclaration(decl);
      }
      break;
    }
  case V_SgNonrealDecl: 
    { 
      SgNonrealDecl *decl = isSgNonrealDecl(d);
      if (returntype != 0)
        *returntype = AstNodeTypeImpl(decl->get_type());
      if (name != 0) 
        *name =  std::string(decl->get_name().str());
      // I can't seem to get parameters from an SgNonrealDecl -Jim Leek
      //      if (paramtype != 0 || params != 0) 
      //  l = decl->get_parameterList();
      break;
    }
    // Liao 2/6/2015, try to extend to support Fortran
    case V_SgProgramHeaderStatement:
    {
     SgProgramHeaderStatement* decl = isSgProgramHeaderStatement(d);
       if (returntype != 0)
         *returntype = AstNodeTypeImpl(decl->get_type()->get_return_type());
       if (name != 0) 
         *name =  std::string(decl->get_name().str());
       if (paramtype != 0 || params != 0) 
         l = decl->get_parameterList();
       break;
   } 
  case V_SgTemplateMemberFunctionDeclaration:
  case V_SgTemplateInstantiationMemberFunctionDecl: 
  case V_SgMemberFunctionDeclaration:
    {
      SgMemberFunctionDeclaration* decl = isSgMemberFunctionDeclaration(d);
      if (def == AST_NULL && skip_function_declaration && decl->get_definition() == 0) {
           // Do non consider a function declaration as definition
           return false;
      }
      if (returntype != 0)
        *returntype = AstNodeTypeImpl(decl->get_type()->get_return_type());
      if (name != 0) {
        SgName cn = decl->get_scope()->get_qualified_name(); 
        SgName fn = decl->get_name();
        *name =  StripGlobalQualifier(std::string(cn.str())) + "::" + ::StripGlobalQualifier(std::string(fn.str()));
      }
      if (paramtype != 0 || params != 0) {
         l = decl->get_parameterList();
         SgClassDeclaration* classDecl= isSgClassDeclaration(decl->get_associatedClassDeclaration());
         if (classDecl != 0) {
            if (paramtype != 0) paramtype->push_back(AstNodeTypeImpl(classDecl->get_type()));  
            if (params != 0) {
               SgThisExp* NEW_THIS_EXP(p, classDecl);
               params->push_back(p);
            }
         }
      } 
      if (outpars != 0) {
         auto init = decl->get_CtorInitializerList()->get_ctors();
         for (auto p = init.begin(); p != init.end(); ++p) {
            SgInitializedName* name = *p;
            outpars->push_back(name); 
         }
      }
      def = AstInterface::GetFunctionDefinitionFromDeclaration(decl);
      break;
    }
  case V_SgInitializedName:
  {
      SgInitializedName* var = isSgInitializedName(d);
      SgType* t = var->get_type();
      if (IsFunctionType(AstNodeTypeImpl(t), paramtype, returntype) && !skip_function_declaration) {
         if (name != 0) {
            *name = var->get_name().str();
         } 
         return true;
      }
      return false;
  }
  default: 
    return false;
  }
  if (l != 0) {
    SgInitializedNamePtrList& names = l->get_args();
    for (SgInitializedNamePtrList::iterator p = names.begin(); 
         p != names.end(); ++p) {
      SgInitializedName* cur = *p;
      if (paramtype != 0)
         paramtype->push_back(AstNodeTypeImpl(cur->get_type())); 
      if (params != 0) params->push_back(cur);
      if (outpars != 0 && cur->get_type()->variantT() == V_SgReferenceType)
         outpars->push_back(cur);
    }
  }
  if (body != 0 && def != 0) {
     SgFunctionDefinition* f_def = isSgFunctionDefinition(def.get_ptr());
     assert(f_def != 0);
     *body = AstNodePtrImpl(f_def->get_body());
  }
  if (ctor != 0 && outpars != 0) {
    SgInitializedNamePtrList& names = ctor->get_ctors();
    for (SgInitializedNamePtrList::iterator p = names.begin(); 
         p != names.end(); ++p) {
      SgInitializedName* cur = *p;
      outpars->push_back(cur);
    }
  }
  if (use_global_unique_name && name != 0) {
    *name = GetGlobalUniqueName(s, "");
  }
  return true;
}

//! Check if a node is an assignment statement/expression, grab its lhs and rhs.
//! Use readlhs to tell whether the value of lhs is read before being modified 
//! in the assignment (e.g., whether the assignment is +=, -= etc.)
bool AstInterfaceImpl::
IsAssignment( SgNode* s, SgNode** lhs, SgNode** rhs, bool *readlhs) 
{ 
  if (s == 0) return false;
  if (s->variantT() == V_SgInitializedName) {
        const SgNode* parent = s->get_parent();
        if (parent != 0 && parent->variantT() == V_SgCtorInitializerList) {
           // Treat constructor member variable initialization as assignment.
          if (rhs != 0) *rhs = isSgInitializedName(s)->get_initializer();
          if (lhs != 0) *lhs = s;
          return true;
        } 
        return false;
   } 
  SgExprStatement *n = isSgExprStatement(s);
  SgExpression *exp = (n != 0)? n->get_expression() : isSgExpression(s);

  if (exp != 0) {
    switch (exp->variantT()) {
    case V_SgPlusAssignOp:
    case V_SgMinusAssignOp:
    case V_SgAndAssignOp:
    case V_SgIorAssignOp:
    case V_SgMultAssignOp:
    case V_SgDivAssignOp:
    case V_SgModAssignOp:
    case V_SgXorAssignOp:
       {
        const SgBinaryOp* s2 = isSgBinaryOp(exp);
        if (lhs != 0) *lhs = s2->get_lhs_operand();
        if (rhs != 0) { *rhs = exp; }
        if (readlhs != 0) *readlhs = true;
        return true;
       }
    case V_SgAssignOp:
      {
        const SgBinaryOp* s2 = isSgBinaryOp(exp);
        if (lhs != 0) *lhs = s2->get_lhs_operand();
        if (rhs != 0) {
          SgNode* init = s2->get_rhs_operand();
          if ( init->variantT() == V_SgAssignInitializer)
            init = isSgAssignInitializer(init)->get_operand();
          *rhs = init;
        }
        if (readlhs != 0) *readlhs = false;
        return true;
      }
    default: return false;
    }
  }
  return false;
}

bool AstInterface::
IsAssignment( const AstNodePtr& s, AstNodePtr* lhs, AstNodePtr* rhs, bool *readlhs) 
{ 
  SgNode* local_lhs = 0, *local_rhs = 0;
  SgNode** _lhs = (lhs == 0)? ((SgNode**)0) : &local_lhs;
  SgNode** _rhs = (rhs == 0)? ((SgNode**)0) : &local_rhs;
  if (AstInterfaceImpl::IsAssignment(s.get_ptr(), _lhs, _rhs, readlhs)) {
     if (lhs) *lhs = AstNodePtr(*_lhs);
     if (rhs) *rhs = AstNodePtr(*_rhs);
     return true;
  }
  return false;
}

bool AstInterface::
IsAliasingDecl( const AstNodePtr& _s, AstList* vars, AstList* aliases)
{
  SgNode* s = AstNodePtrImpl(_s).get_ptr(); 
  if (s == 0) return false;
  DebugVariable([&_s](){ return "IsAliasingDecl:" + AstInterface::AstToString(_s); });
  switch (s->variantT()) {
   case V_SgVariableDeclaration: {
      bool has_alias = false;
      SgInitializedNamePtrList& names = isSgVariableDeclaration(s)->get_variables();
      for ( SgInitializedNamePtrList::iterator p = names.begin(); 
            p != names.end(); ++p) {
         if (IsAliasingDecl(*p, vars, aliases))
            has_alias = true;
      }
     return has_alias;
     } 
   case V_SgInitializedName: {
     auto* var = isSgInitializedName(s);
     SgExpression* init = var->get_initializer();
     if (init == 0) return false;
     if (init->variantT() == V_SgAssignInitializer) {
        init = isSgAssignInitializer(init)->get_operand();
     }
     switch (var->get_type()->variantT()) {
      case V_SgPointerType: {
         if (IsAddressOfOp(init)) {
           if (vars != 0) vars->push_back(var);
           if (aliases != 0) aliases->push_back(init);
           return true;
         }
         return false;
       }
      case V_SgReferenceType: {
         if (IsMemoryAccess(init)) {
           if (vars != 0) vars->push_back(var);
           if (aliases != 0) aliases->push_back(init);
           return true;
         }
         return false;
       }
      default:
         return false;
     }
    }
  case V_SgCommonBlockObject: {
    if (vars != 0 || aliases != 0) {
      SgCommonBlockObject* comm = isSgCommonBlockObject(s);
      assert(comm != 0);
      for (auto* e : comm->get_variable_reference_list()->get_expressions()) {
         SgVarRefExp* v = isSgVarRefExp(e);
         assert(v != 0);
         if (vars != 0) vars->push_back(AstNodePtrImpl(v->get_symbol()->get_declaration()));
         if (aliases != 0) {
             AstNodePtr global("_COMMON_"+ comm->get_block_name() + "::" + v->get_symbol()->get_name().str());
             aliases->push_back(global);
         }
      }
    } 
    return true;
  case V_SgCommonBlock:
      for (auto* e : 
          isSgCommonBlock(s)->get_block_list()) {
         bool r = IsAliasingDecl(e, vars, aliases);
         assert(r); 
      }
      return true;
   default:
    return false;
   }
 }
}

//! Check if $_s$ is a variable declaration node; 
//! If yes, return the declared variables and their initial values
bool AstInterface:: 
IsVariableDecl(const AstNodePtr& _s, AstNodeList* vars, AstNodeList* init, AstList* designators)
{
  SgNode* s = AstNodePtrImpl(_s).get_ptr(); 

  if (s == 0) return false;

  // Sanity checks since we have three lists that need to be coordinated.
  // init may have fewer entries when we are dealing with aggregate initializations.
  assert(vars == 0 || init == 0 || init->size() <= vars->size()); 
  assert(vars == 0 || designators == 0 || designators->size() <= vars->size()); 
  assert(init == 0 || designators == 0 || designators->size() == init->size()); 

  auto _init_push_back_if_needed_ = [&init,&vars,&designators]() {
      if (!init->back().is_null()) { 
             init->push_back(AST_NULL); 
             if (vars != 0) vars->push_back(vars->back()); 
             if (designators != 0) designators->push_back(designators->back()); 
      }
  };

  switch (s->variantT()) {
    case V_SgVariableDeclaration: {
      SgVariableDeclaration *decl = isSgVariableDeclaration(s);
      DebugDecl([&_s]{return "Finding variable decl:" + AstToString(_s);});
      if (vars == 0 && init == 0 && designators == 0)
         return true;
      SgInitializedNamePtrList& names = decl->get_variables();
      for ( SgInitializedNamePtrList::iterator p = names.begin(); 
            p != names.end(); ++p) {
         SgInitializedName* cur_var = (*p);
         IsVariableDecl(cur_var, vars, init, designators);
      }
      return true;
     }
    case V_SgInitializedName: {
      SgInitializedName* cur_var = isSgInitializedName(s);
      DebugDecl([&_s]{return "Finding variable decl:" + AstToString(_s);});
      if (vars == 0 && init == 0 && designators == 0)
         return true;
      if (vars != 0) { 
           vars->push_back(cur_var);
      }
      if (init != 0) init->push_back(AST_NULL);
      if (designators != 0) {
          designators->push_back(cur_var->get_type());
      }
      IsVariableDecl(cur_var->get_initializer(), vars, init, designators);
      return true;
     }
  case V_SgConstructorInitializer: {
      DebugDecl([&_s]{return "Finding variable decl:" + AstToString(_s);});
      if (vars!=0 && vars->empty()) return false; // calling initializer without variable.
      if (init == 0) return true;
      SgConstructorInitializer* ctorInit = isSgConstructorInitializer(s);
      SgExprListExp *argsList = ctorInit->get_args();  
      if (argsList != nullptr) {  
        for (SgExpression *expr : argsList->get_expressions()) {  
            _init_push_back_if_needed_();
            init->back() = expr;
        }  
      }
      return true;
      }
  case V_SgDesignatedInitializer: {
      DebugDecl([&_s]{return "Finding variable decl:" + AstToString(_s);});
      SgDesignatedInitializer* design_init = isSgDesignatedInitializer(s);
      SgExpression *  designator  = design_init->get_designatorList()->get_expressions()[0];
      SgInitializer * initializer = design_init->get_memberInit();
      _init_push_back_if_needed_();
      if (designators != 0) designators->back() = designator;
      if (init != 0) init->back() = initializer;
      return true;
   }
  case V_SgAggregateInitializer: {
      if (init==0 || init->empty()) return false; // No variables; only empty initializations.
      DebugDecl([&_s]{return "Finding variable decl:" + AstToString(_s);});
      SgAggregateInitializer* aggr_init = isSgAggregateInitializer(s);
      _init_push_back_if_needed_();
      if (designators != 0) {
            auto* t = designators->back().get_ptr();
            assert(t != 0);
            switch (t->variantT()) {
              case V_SgArrayType:
                init->back() = AST_NULL_ARRAY_INDEX(std::to_string(1));
                designators->back() = isSgArrayType(t)->get_base_type();
                break;
              case V_SgClassType: {
                init->back() = AST_NULL_FIELD_INDEX(std::to_string(1));
                auto* class_def = GetClassDefn(isSgClassDeclaration(isSgClassType(t)->get_declaration()));
                if (vars != 0) {
                  // Pop off aggregate entries to match members with elemental initializations.
                  // Here we assume there is at least one member variable inside class_def.
                  while (vars->size() > init->size()) vars->pop_back();
                  for (auto* member :  class_def->get_members()) {  
                     // Here future variables are pushed in.
                     IsVariableDecl(member, vars, 0, 0);
                  }
                }
                break;
                }
              default: break;
             }
      }
      auto& list = aggr_init->get_initializers()->get_expressions();
      for (auto& cur_var : list) {
         IsVariableDecl(cur_var, vars, init, designators);
      }
      return true;
   }
  case V_SgAssignInitializer: {
      DebugDecl([&_s]{return "Finding variable decl:" + AstToString(_s);});
      if (init != 0) {
          if (init->empty()) return false; // We don't have an earlier SgInitializedName. Unexpected.
          auto* def = isSgAssignInitializer(s)->get_operand();
          auto cur_init= init->back();
          _init_push_back_if_needed_();
          init->back()  =  def;
          if (vars != 0 && vars->size() > init->size() && cur_init.is_null_field_index()) {
               // update the vars entry to be the member variable.
               int index = atoi(cur_init.get_signature().c_str()); 
               assert(index != 0);
               init->push_back(AST_NULL_FIELD_INDEX(std::to_string(index+1)));
               if (designators != 0) designators->push_back(designators->back());
          }
      }
      return true;
     } 
  default:
     DebugDecl([&_s]{return "Not recognized variable decl:" + AstToString(_s);});
  return false;
  }
}

AstNodePtr AstInterface:: 
CreateAllocateArray( const AstNodePtr& _arr, const AstNodeType& _elemtype,
               const AstNodeList& indexsize)
{
  SgType* elemtype = AstNodeTypeImpl(_elemtype).get_ptr();
   SgType* atype = elemtype; 
   for (AstNodeList::const_iterator p = indexsize.begin(); 
        p != indexsize.end(); ++p) {
      SgExpression* exp = isSgExpression(AstNodePtrImpl((*p)).get_ptr());
      assert(exp != 0);
      atype = new SgArrayType(atype, exp);
   }
   SgType* baseType = elemtype;
   assert(baseType != NULL);
   SgExpression* arr = ToExpression(*impl,AstNodePtrImpl(_arr).get_ptr());
   SgNewExp* rhs = new SgNewExp(GetFileInfo(), atype, 0, new SgConstructorInitializer(GetFileInfo(),NULL,NULL,baseType,false,false,false,true));
   return AstNodePtrImpl(::CreateAssignment(*impl, arr, rhs));
}

AstNodePtr AstInterface:: CreateDeleteArray( const AstNodePtr& _arr)
{
  SgNode* arr = AstNodePtrImpl(_arr).get_ptr();
  SgExpression* var = isSgExpression(arr);
  assert(var != 0);
  return AstNodePtrImpl(new SgDeleteExp(GetFileInfo(), var, true));
}

AstNodePtr AstInterface::
CreateLoop( const AstNodePtr& _cond, const AstNodePtr& _body)
{
  SgStatement* cond = isSgStatement(AstNodePtrImpl(_cond).get_ptr());
  assert(cond != 0);
  SgStatement* bstmt = isSgStatement(AstNodePtrImpl(_body).get_ptr());
  assert(bstmt != 0);
  SgBasicBlock* body = isSgBasicBlock(AstNodePtrImpl(_body).get_ptr());
  if (body == 0) {
     NEW_BLOCK1(body,bstmt);
  }
  SgNode* result = new SgWhileStmt(GetFileInfo(), cond, body);
  cond->set_parent(result); body->set_parent(result);
  return AstNodePtrImpl(result);
}

AstNodePtr AstInterface::
CreateAssignment( const AstNodePtr& _lhs, const AstNodePtr& _rhs)
{
  SgNode* lhs = AstNodePtrImpl(_lhs).get_ptr(), *rhs = AstNodePtrImpl(_rhs).get_ptr();
  SgExpression *lhsexp = ToExpression(*impl, lhs);
  SgExpression* rhsexp = ToExpression(*impl, rhs);
  AstNodePtrImpl res = ::CreateAssignment(*impl, lhsexp, rhsexp);
  if ( impl->NumberOfObservers() ) {
    CopyAstRecord info(*impl, _rhs, _lhs);
    impl->Notify(info);
  }
  return res;
}

bool AstInterface::
IsIOInputStmt(const AstNodePtr&, AstNodeList*) {
    return false;
}
bool AstInterface::
IsIOOutputStmt(const AstNodePtr&, AstNodeList*) {
    return false;
}

//! Check if $_exp$ is a single integer constant; if yes, return the constant value in $val$.
bool AstInterface::IsConstInt( const AstNodePtr& _exp, int *val) 
{ 
  SgNode* exp = SkipCasting(AstNodePtrImpl(_exp).get_ptr());
  if (exp == 0) return false;
  if (exp->variantT() == V_SgIntVal) {
    if (val != 0) 
      *val = isSgIntVal(exp)->get_value();
    return true;
  }
  return false;
}

bool AstInterface::
IsConstant( const AstNodePtr& _exp, std::string* valtype, std::string *val)
{
  SgNode* exp = SkipCasting(AstNodePtrImpl(_exp).get_ptr());
  if (exp == 0) return false;
  switch (exp->variantT()) {
  case V_SgStringVal:
      if (valtype != 0) *valtype = "string";
      // Do not want to use unparseToString for SgStringVal;
      if (val != 0) *val = isSgStringVal(exp)->get_value();
      return true;
  case V_SgCharVal:
  case V_SgWcharVal:
  case V_SgSignedCharVal:
  case V_SgUnsignedCharVal:
      if (valtype != 0) *valtype = "char";
      break;
  case V_SgShortVal:
  case V_SgUnsignedShortVal:
  case V_SgIntVal:
  case V_SgEnumVal:
  case V_SgUnsignedIntVal:
  case V_SgLongIntVal:
  case V_SgLongLongIntVal:
  case V_SgUnsignedLongLongIntVal:
  case V_SgUnsignedLongVal:
      if (valtype != 0) *valtype = "int";
      break;
  case V_SgFloatVal:
      if (valtype != 0) *valtype = "float";
      break;
  case V_SgDoubleVal:
  case V_SgLongDoubleVal:
      if (valtype != 0) *valtype = "double";
      break;
  case V_SgSizeOfOp:  /* consider size of a constant b/c it's value doesn't change */
      {
         if (valtype != 0) *valtype = "int";
         break;
      }
  default:
     return false;
  };
  if (val != 0) {
     assert(isSgType(exp) == 0);
     *val = exp->unparseToString();
  }
  return true;
}

//! Two references are the same if they have the same name and same scope
bool AstInterface::
IsSameVarRef( const AstNodePtr& _n1, const AstNodePtr& _n2,
                   std::function<bool(const AstNodePtr&, const AstNodePtr&)>* call_on_diff)
{
   AstNodePtrImpl n1(_n1), n2(_n2);
   std::string name1, name2;
   AstNodePtr scope1, scope2;
   if (IsVarRef(n1, 0,&name1, 0, 0, /*use_global_unique_name=*/true) && IsVarRef(n2, 0, &name2, 0, 0, /*use_global_unique_name=*/true)) {
     DebugDiff([name1,name2](){ return "SameVarRef name: " + name1 + " vs " + name2; });
     bool is_same = (name1 == name2);
     if (!is_same && call_on_diff != 0 && !(*call_on_diff)(name1, name2)) {
        name1 = GetVarName(n1, false);
        name2 = GetVarName(n2, false); // use_globa_unique_name=false
        return name1 == name2 || !(*call_on_diff)(name1, name2);
     }
     return is_same;
   }
   return false;
}

bool AstInterface:: IsMin( const AstNodePtr& _exp)
{
   std::string name;
   if (!IsVarRef(_exp, 0, &name, 0, 0)) {
      return false;
   }
   if (name == "min" || name == "min2" || name=="min3")
      return true;
   return false;
}

bool AstInterface::IsMax( const AstNodePtr& _exp)
{
   std::string name;
   if (!IsVarRef(_exp, 0, &name, 0, 0))
      return false;
   if (name=="max" || name=="max2" || name=="max3")
      return true;
   return false;
}

//! Check if $_exp$ is a variable reference (including all name references which may have
//! functions or objects have values)
/* Does not deal correctly with templates SgNorealExp */
bool AstInterfaceImpl::
IsVarRef( SgNode* exp, SgType** vartype, std::string* varname,
          SgNode** _scope, bool *defined_in_global, 
          bool use_global_unique_name, bool *has_ptr_deref) 
{ 
  if (exp == 0) return false;
  SgNode *decl = 0, *scope = 0;
  switch (exp->variantT()) {
    case V_SgNonrealRefExp:
      {
         SgNonrealSymbol *sb = isSgNonrealRefExp(exp)->get_symbol();
         assert(sb != 0);
         SgScopeStatement *cdef = sb->get_scope();
         assert(cdef != 0);
         if (varname != 0) {
            *varname = StripGlobalQualifier(cdef->get_qualified_name())+"::"+StripGlobalQualifier(sb->get_name().str());
         }
         if (vartype != 0) *vartype = sb->get_type();
      }
      break;
    case V_SgMemberFunctionRefExp: 
      {
        const SgMemberFunctionRefExp *var = isSgMemberFunctionRefExp( exp );
        assert(var != 0);
        SgFunctionSymbol *sb = var->get_symbol();
        assert(sb != 0);
        decl = sb->get_declaration();
        if (vartype != 0) *vartype = sb->get_type();
        if (varname != 0)  *varname = sb->get_name().str();
        scope = decl;
      }
      break;
    case V_SgTemplateMemberFunctionRefExp: 
      {
        const SgTemplateMemberFunctionRefExp *var = isSgTemplateMemberFunctionRefExp( exp );
        assert(var != 0);
        SgFunctionSymbol *sb = var->get_symbol();
        assert(sb != 0);
        decl = sb->get_declaration();
        if (vartype != 0) *vartype = sb->get_type();
        if (varname != 0)  *varname = sb->get_name().str();
        scope = decl;
      }
      break;
    case V_SgTemplateFunctionRefExp:
      {
        const SgTemplateFunctionRefExp *var = isSgTemplateFunctionRefExp( exp );
        assert(var != 0);
        SgFunctionSymbol *sb = var->get_symbol();
        assert(sb != 0);
        decl = sb->get_declaration();
        if (vartype != 0) *vartype = sb->get_type();
        if (varname != 0)  *varname = sb->get_name().str();
        scope = decl;
      }
      break;
    case V_SgFunctionRefExp:
      {
        const SgFunctionRefExp *var = isSgFunctionRefExp( exp );
        assert(var != 0);
        SgFunctionSymbol *sb = var->get_symbol();
        assert(sb != 0);
        decl = sb->get_declaration();
        if (vartype != 0) *vartype = sb->get_type();
        if (varname != 0)  *varname = sb->get_name().str();
        scope = decl;
      }
      break;
    case V_SgVarRefExp:
      {
        const SgVarRefExp *var = isSgVarRefExp( exp );
        SgVariableSymbol *sb = var->get_symbol();
        if (vartype != 0) *vartype = sb->get_type();
        if (varname != 0) *varname = sb->get_name().str();
        decl = sb->get_declaration();
        scope = AstInterfaceImpl::GetScope(decl);
      }
      break;
    case V_SgThisExp:
      {
        const SgThisExp *var = isSgThisExp( exp );
        assert(var != 0);
        if (vartype != 0) {
          if (var->get_class_symbol() != NULL) {
             *vartype = var->get_class_symbol()->get_type();
          }
        }
        if (varname != 0) *varname = "this";
        scope = GetScope(exp);
      }
       break;
    case V_SgInitializedName:
      {
        SgInitializedName* var = isSgInitializedName(exp);
        if (var->get_name().str() == 0) {
           std::cerr << "no name for initname " << var->unparseToString() << "\n";
           return false;
        }
        SgType* t = var->get_type();
        assert( t != 0);
        if (vartype != 0) *vartype = t;
        if (varname != 0) *varname = var->get_name().str();
        decl = var;
        scope = AstInterfaceImpl::GetScope(var);
      }
      break;
    case V_SgPointerDerefExp:
       if (has_ptr_deref != 0) {
             *has_ptr_deref = true;
       }
       if (IsVarRef(isSgPointerDerefExp(exp)->get_operand(), vartype, varname, _scope, defined_in_global, use_global_unique_name)) {
          if (varname != 0) {
             (*varname) = "_deref_(" + (*varname) +")";
          }
          if (vartype != 0) {
            SgPointerType* ptype = isSgPointerType(AstNodeTypeImpl(*vartype).get_ptr());
            if (ptype != 0) {
                *vartype =  AstNodeTypeImpl(ptype->get_base_type()).get_ptr();
            }
          }
          break;
       }
       return false; 
    case V_SgDotStarOp:
      {
       const SgBinaryOp *exp1 = isSgBinaryOp(exp);
       SgVarRefExp* var2 = isSgVarRefExp(exp1->get_rhs_operand());
       if (var2 == 0)
          return false;
       if (has_ptr_deref != 0) {
          *has_ptr_deref = true;
       }
       SgVariableSymbol *sb2 = var2->get_symbol();
       if (vartype != 0) *vartype = sb2->get_type();
       if (varname != 0) {
          *varname = StripQualifier(std::string(sb2->get_name().str()));
       }
     }
     break;
    case V_SgArrowExp:
       if (has_ptr_deref != 0 && !isSgThisExp(isSgBinaryOp(exp)->get_lhs_operand())) {
          *has_ptr_deref = true;
       }
       ROSE_FALLTHROUGH;
    case V_SgDotExp:
     {
       const SgBinaryOp *exp1 = isSgBinaryOp(exp);
       SgNode* lhs = exp1->get_lhs_operand();
       if (isSgThisExp(lhs)!=0) { 
          if (!IsVarRef(exp1->get_rhs_operand(),vartype,varname,_scope, defined_in_global, use_global_unique_name, has_ptr_deref)) {
              return false;
          }
          return true;
       } 
       std::string varname1;
       if (!IsVarRef(lhs,0, &varname1, &scope, defined_in_global, use_global_unique_name, has_ptr_deref)) {
          return false;
       }
       SgVarRefExp* var2 = isSgVarRefExp(exp1->get_rhs_operand());
       if (var2 == 0)
          return false;
       SgVariableSymbol *sb2 = var2->get_symbol();
       if (vartype != 0) *vartype = sb2->get_type();
       if (varname != 0) {
          auto dot = (exp->variantT() == V_SgDotExp)? "." : "->";
          *varname = varname1 + dot + StripQualifier(std::string(sb2->get_name().str()));
       }
    }
     break;
  default:
    return false;
  }
  if (_scope != 0 || defined_in_global != 0 || use_global_unique_name) {
    if (scope == 0 && decl != 0) {
      std::cerr << "Both should be defined, or neither should. \n";
       assert(false);
    }
    if (_scope != 0 && scope != 0) {
        *_scope =  (isSgScopeStatement(scope)? scope : AstInterfaceImpl::GetScope(scope));
    }
    if (defined_in_global != 0)
       *defined_in_global = (scope == 0 || scope->variantT() == V_SgGlobal);
    if (use_global_unique_name && varname != 0 && (*varname) != "") {
       DebugVariable([varname,scope](){ return "Variable-scope:" + *varname + ":" + AstInterface::AstToString(scope); });
       if (scope != 0) {
          if (decl != scope) {
             *varname = AstInterface::GetGlobalUniqueName(scope, *varname);
          } else { // If decl == scope, the variable is a function declaration.
             *varname = AstInterface::GetGlobalUniqueName(scope, "");
          }
       }
       DebugVariable([varname](){ return "global Variable name:" + *varname; });
    }
  }
  if (varname != 0) {
     DebugVariable([exp,varname](){ return "IsVarRef:" + exp->class_name() + ":" + *varname; });
  }
  return true;
}

bool AstInterface::
IsVarRef( const AstNodePtr& exp, AstNodeType* vartype, std::string* varname,
          AstNodePtr* scope, bool *defined_in_global, 
          bool use_global_unique_name, bool* has_ptr_deref ) 
{ 
  if (exp == AST_NULL) return false;
  SgNode* var_scope = 0;
  SgType* var_type = 0;
  SgType** _vartype = (vartype==0)? (SgType**)0 : &var_type;
  SgNode** _scope = (scope==0)? (SgNode**) 0 : &var_scope;
   if  (AstInterfaceImpl::IsVarRef(exp.get_ptr(), _vartype, varname, _scope, defined_in_global, use_global_unique_name, has_ptr_deref)) {
       if (_vartype != 0)  *vartype = *_vartype;
       if (_scope != 0)  *scope = *_scope;
       return true;
   }
   return false;
}

std::string AstInterface::GetVarName( const AstNodePtr& exp, bool use_global_unique_name)
{
  std::string name;
  if (IsVarRef(exp, 0, &name, 0, 0, use_global_unique_name)) {
    return name;
  }
  std::cerr << "Error: expecting a variable reference but getting:" + AstToString(exp); ;
  assert(false);
  return "";
}

AstNodeType AstInterface::GetExpressionType( const AstNodePtr& s)
{
  AstNodeType t;
  if (!IsExpression(s, &t))
     ROSE_ABORT();
  return t;
}


std::string AstInterface:: 
NewVar( const AstNodeType& _type, const std::string& name, bool makeunique,
        bool delayInsert, const AstNodePtr& _declLoc, const AstNodePtr& _init)
{
  DebugVariable([&name](){ return "Enter NewVar:" + name; });
  SgType* type = AstNodeTypeImpl(_type).get_ptr();

  SgNode* declLoc = AstNodePtrImpl(_declLoc).get_ptr();

  SgScopeStatement *scope = (declLoc == 0)? 0 : isSgScopeStatement(declLoc);
  if (scope == 0 && declLoc != 0) scope = isSgScopeStatement(AstInterfaceImpl::GetScope(declLoc));

  SgExpression* e = 0;
  if (_init != AST_NULL) e = ToExpression( *impl, (SgNode*)_init.get_ptr());
  SgVariableSymbol *sb = impl->NewVar( isSgType(type), name, makeunique, delayInsert, e, scope);
#ifndef NDEBUG
  SgInitializedName* def = sb->get_declaration();
  assert(def != 0 && !HasNullParent(def));
#endif

  DebugVariable([&name](){ return "Finish creating NewVar:" + name; });
  SgName n =  sb->get_name();
  std::string varname =  std::string( n.str());
  return varname;
}

void AstInterface:: AddNewVarDecls()
{ impl->AddNewVarDecls(); }

void AstInterface:: CopyNewVarDecls(const AstNodePtr& nblock, bool clear)
{ 
   SgBasicBlock* blk = isSgBasicBlock((SgNode*)nblock.get_ptr()); 
   if (blk == 0) { std::cerr << "nblock is not a block: " << AstToString(nblock) << "\n"; ROSE_ABORT(); }
   impl->CopyNewVarDecls(blk, clear);
}

void AstInterfaceImpl:: AddNewVarDecls()
   {
      for ( size_t i = newVarList.size(); i > 0; --i) {
            std::pair<SgScopeStatement*, SgVariableDeclaration*> cur = newVarList[i-1];
            cur.first->insertStatementInScope(cur.second, true);
            cur.second->set_parent(cur.first);
      } 
      newVarList.clear();
   } 

void AstInterfaceImpl:: CopyNewVarDecls(SgBasicBlock* blk, bool clear)
   {
      for ( size_t i = 0; i < newVarList.size(); ++i) {
            std::pair<SgScopeStatement*, SgVariableDeclaration*> cur = newVarList[i];
            SgVariableDeclaration* s = cur.second;
            if (!clear) { 
               SgTreeCopy copyOption;
               s = isSgVariableDeclaration(cur.second->copy( copyOption));
            }
            assert(s != 0);
            blk->append_statement(s); s->set_parent(blk);
            //QY: each new decl has only one variable*/
            const SgInitializedNamePtrList& names = s->get_variables();
            assert(names.size() == 1);
            SgInitializedName* n = *names.begin();
            assert(n != 0);
            n->set_parent(s);
            InsertVar(n, blk); 
      } 
      if (clear) newVarList.clear();
   } 

SgVarRefExp*
AstInterfaceImpl:: CreateFieldRef(SgNode* decl, std::string name2)
{
  assert(decl != 0);
  SgVariableSymbol *vs = LookupVar( name2, decl);
  SgVarRefExp* r = new SgVarRefExp(GetFileInfo(),vs);
  r->set_endOfConstruct(r->get_file_info());
  return r;
}

AstNodePtr AstInterface:: CreateFieldRef(std::string name1, std::string name2) {
  auto* decl = impl->LookupNestedDeclaration(name1, impl->get_scope(0));
  return AstNodePtrImpl(impl->CreateFieldRef(decl, name2));
}

AstNodePtr AstInterface::
CreateMethodRef(std::string classname, std::string fieldname, bool createIfNotFound)
{ 
      SgClassDeclaration *c = isSgClassDeclaration(impl->LookupNestedDeclaration(classname, impl->get_scope(0)));
      if (c == 0) {
         std::cerr << "Error: cannot find class declaration for " << classname << std::endl;
         ROSE_ABORT();
      }
      SgMemberFunctionSymbol *f1 = impl->GetMemberFunc(c, fieldname);
      if (f1 == 0) {
         if (!createIfNotFound) {
            std::cerr << "Error: cannot find member function " << fieldname << std::endl;
            ROSE_ABORT();
         }
         else {
            f1 = impl->NewMemberFunc(c, fieldname, 
                  impl->GetTypeInt(), std::list<SgInitializedName*>()); 
        }
      }
      SgMemberFunctionRefExp *NEW_MFUNCTION_REF(fr,f1);
      return AstNodePtrImpl(fr);
}


SgDotExp* AstInterfaceImpl::
CreateVarMemberRef(std::string name1, std::string name2, SgNode* loc)
{
   auto* obj = CreateVarRef(name1, loc);
   if (obj == 0) return 0;
   SgType* vartype = AstInterface::GetBaseType(obj->get_type()).get_ptr();
   assert(vartype != 0);
   auto *field = CreateFieldRef(vartype, name2);
   SgDotExp* NEW_BIN_OP(r, SgDotExp, obj, field);
   return r;
}

SgExpression* AstInterfaceImpl::
CreateVarRef(std::string varname, SgNode* loc) {
    SgNode *loc1 = AstNodePtrImpl(loc).get_ptr();
    if (loc1 == 0) loc1 = scope;
    int hasdot = varname.rfind(".", varname.size()-1);
    if (hasdot > 0) {
         std::string name1 = varname.substr(0, hasdot);
         std::string name2 = varname.substr( hasdot+1, varname.size()-hasdot);
         return CreateVarMemberRef(name1, name2, loc1);
    }
    int hasarrow = varname.rfind("->", varname.size()-1);
    if (hasarrow > 0) {
         std::string name1 = varname.substr(0, hasarrow);
         std::string name2 = varname.substr( hasarrow+2, varname.size()-hasarrow);
         return CreateVarMemberRef(name1, name2, loc1);
    }
    SgScopeStatement* loc1_s = isSgScopeStatement(loc1);
    if (loc1_s == 0) loc1_s = isSgScopeStatement(GetScope(loc1));
    assert(loc1_s != 0);
    int is_this = varname.rfind("::this", varname.size()-1);
    if (is_this > 0) {
         std::string name1 = varname.substr(0, is_this);
         auto* decl = LookupNestedDeclaration(name1, loc1_s);
         assert(decl != 0);
         SgClassDeclaration* decl1 = isSgClassDeclaration(decl);
         assert(decl1 != 0);
         auto* NEW_THIS_EXP(p, decl1);
         return p;
    }
    std::string lookup_name = varname; 
    while (lookup_name[0] == '*') lookup_name = lookup_name.substr(1,lookup_name.size()-1);
    SgVariableSymbol *sym = LookupVar(lookup_name, loc1_s);
    if (sym == 0) {
         std::cerr << "Error : variable " << varname << " not found in scope " << loc1->class_name() << ", which is derived from " << ((loc==0)? "NULL" : loc->class_name()) << "\n";
         ROSE_ABORT();
    }
    SgExpression *r = new SgVarRefExp( GetFileInfo(), sym);
    while (varname[0] == '*') {
       r = new SgPointerDerefExp( GetFileInfo(), r, r->get_type());
       varname = lookup_name.substr(1,varname.size()-1);
    }
    return r;
  }

AstNodePtr AstInterface::
CreateVarRef(std::string varname, const AstNodePtr& loc) {
    auto result = impl->CreateVarRef(varname, loc.get_ptr());
    if (result == 0) {
         return AST_UNKNOWN;
    }
    return AstNodePtrImpl(result);
 }

AstNodeType AstInterface::GetType(const std::string& name) 
{
  if (name[name.size()-1] == '*') {
    std::string name1 = name.substr(0, name.size()-1);
    SgType *t = isSgType(AstNodeTypeImpl(GetType(name1)).get_ptr());
    SgPointerType *ptr = t->get_ptr_to();
    if (ptr == 0) {
       ptr = new SgPointerType(t);
       t->set_ptr_to(ptr);
    }
    return AstNodeTypeImpl(ptr);
  }
  else if (name == "char") 
      return AstNodeTypeImpl(new SgTypeChar());
  else if (name == "int")
      return AstNodeTypeImpl(AstInterfaceImpl::GetTypeInt());
  else if (name == "long")
       return AstNodeTypeImpl(new SgTypeLong());
  else if (name == "void")
       return AstNodeTypeImpl(new SgTypeVoid());
  else if (name == "float")
       return AstNodeTypeImpl(new SgTypeFloat());
  else if (name == "double")
       return AstNodeTypeImpl(new SgTypeDouble());
  else if (name == "string")
       return AstNodeTypeImpl(new SgTypeString());
  else if (name == "bool")
        return AstNodeTypeImpl(new SgTypeBool());
  else {
       SgClassDeclaration *c = isSgClassDeclaration(impl->LookupNestedDeclaration(name, impl->get_scope(0)));
       if (c == 0) {
          std::cerr << "Error: not recognize type name : " << name << std::endl;
          ROSE_ABORT();
       }
       else
          return AstNodeTypeImpl(new SgClassType(c));
  }
} 

AstNodeType
AstInterface::GetArrayType(const AstNodeType& base, const AstNodeList& index)
{
  if (IS_FORTRAN_LANGUAGE()) {
    SgType* btype = AstNodeTypeImpl(base).get_ptr();
    SgArrayType* atype = new SgArrayType(btype);

    SgExprListExp*  NEW_EXPR_LIST(dim);
    for (AstNodeList::const_iterator p = index.begin(); p != index.end(); ++p) {
      SgExpression* i = isSgExpression(ASTNODE2PTR(*p));
      assert(i);
      dim->append_expression(i);
      i->set_parent(dim);
    }
    atype->set_dim_info(dim);
    dim->set_parent(atype);

    atype->set_rank(1);
    return AstNodeTypeImpl(atype);

  } else {

    SgType* r = AstNodeTypeImpl(base).get_ptr();
    for (AstNodeList::const_iterator p = index.begin(); p != index.end();
         ++p) {
       if ((ASTNODE2PTR(*p))->variantT() != V_SgIntVal) {
          return AstNodeTypeImpl(new SgPointerType(r));
       }
    }
    for (AstNodeList::const_iterator p1 = index.begin(); p1 != index.end();
         ++p1) {
       SgExpression* ie = isSgExpression(ASTNODE2PTR(*p1));
       assert( ie != 0);
       r = new SgArrayType(r, ie);
    }
  
    return AstNodeTypeImpl(r);
  }
}

bool AstInterface::
IsAddressOfOp( const AstNodePtr& _s, AstNodePtr* ref)
{  
  SgNode* s = AstNodePtrImpl(_s).get_ptr();
  if (s == 0) return false;
  switch (s->variantT()) {
    case V_SgAssignInitializer: 
        return IsAddressOfOp(isSgAssignInitializer(s)->get_operand(), ref); 
    case V_SgCastExp:
        return IsAddressOfOp(isSgCastExp(s)->get_operand(), ref); 
    default: break;
  }
  if (s->variantT() == V_SgAddressOfOp) {
      if (ref != 0) *ref=isSgAddressOfOp(s)->get_operand();
      return true;
  }  
  return false;
}


bool AstInterface::
IsMemoryAllocation( const AstNodePtr& s, AstNodeType* exptype, AstNodePtr* init, AstNodeList* dest)
{
  AstNodePtrImpl s1 = SkipCasting(s.get_ptr()), f;
  if (IsFunctionCall(s1, &f)) {
    std::string name;
    if (!IsVarRef(f, 0, &name)) {
       return false;
    }
    if (name == "malloc") {
       if (exptype != 0) {
          *exptype = GetExpressionType(s);
       }
       if (init != 0) {
          *init = AST_NULL;
       }
       return true;
    }
    return false;
  }
  SgNewExp* is_new = isSgNewExp(s1.get_ptr());
  if (is_new != 0) {
     if (exptype != 0) {
       *exptype = AstNodeTypeImpl(is_new->get_type()); 
     }
     if (init != 0) {
       *init = is_new->get_constructor_args();
     }
    if (dest != 0) {
       auto* placements = is_new->get_placement_args();
       if (placements != 0) { 
          *dest = GetChildrenList(placements);
       }
    }
    return true;
  }
  return false;
}

bool AstInterface::
IsMemoryFree( const AstNodePtr& s, AstNodeType* exptype, AstNodePtr* variable)
{
  AstNodePtrImpl s1 = SkipCasting(s.get_ptr()), f;
  AstNodeList params;
  if (IsFunctionCall(s1, &f, &params)) {
    std::string name;
    if (!IsVarRef(f, 0, &name)) {
       return false;
    }
    if (name == "free") {
       assert(params.size() == 1);
       if (variable != 0) {
         *variable = SkipCasting(params.front().get_ptr()); 
       }
       if (exptype != 0) {
          *exptype = GetExpressionType(params.front()); 
       }
       return true;
    }
    return false;
  }
  SgDeleteExp* is_delete = isSgDeleteExp(s1.get_ptr());
  if (is_delete != 0) {
    if (variable != 0) {
         * variable = is_delete->get_variable();
    }
    if (exptype != 0) {
      *exptype = AstNodeTypeImpl(is_delete->get_variable()->get_type()); 
    }
    return true;
  }
  return false;
}

bool AstInterface::
IsMemoryAccess( const AstNodePtr& _s, AstNodeList* subrefs)
{  
  if (_s.is_unknown())
    return true;
  SgNode* s = AstNodePtrImpl(_s).get_ptr();
  if (s == 0) return false;
  switch (s->variantT()) {
  case V_SgConstructorInitializer: return false;
  case V_SgAssignInitializer:
     if (subrefs != 0) {
        IsMemoryAccess(isSgAssignInitializer(s)->get_operand(), subrefs);
     }
     return false;
  case V_SgCastExp:
      return IsMemoryAccess(isSgUnaryOp(s)->get_operand(), subrefs);
  case V_SgAddressOfOp:
      if (subrefs != 0) {
          IsMemoryAccess(isSgUnaryOp(s)->get_operand(), subrefs);
      }
      return false;
  case V_SgCommaOpExp: {
     auto* e1 = isSgCommaOpExp(s)->get_lhs_operand(), *e2 = isSgCommaOpExp(s)->get_rhs_operand();
     IsMemoryAccess(e1, subrefs);
     IsMemoryAccess(e2, subrefs);
     return false;
    }
  case V_SgConditionalExp: {
     auto* e1 = isSgConditionalExp(s)->get_true_exp(), *e2 = isSgConditionalExp(s)->get_false_exp();
     IsMemoryAccess(e1, subrefs);
     IsMemoryAccess(e2, subrefs);
     return false;
    }
  case V_SgPointerDerefExp:
      if (subrefs != 0) { subrefs->push_back(isSgUnaryOp(s)->get_operand()); }
      return true;
  case V_SgDotExp:
  case V_SgArrowExp:
   {
     if (isSgBinaryOp(s)->get_rhs_operand()->variantT() == V_SgVarRefExp) {
        if (subrefs != 0) { subrefs->push_back(s); }
        return true;
     }
     return false;
   }
  default: {
     if (IsVarRef(_s) || IsArrayAccess(_s)) {
        if (subrefs != 0) subrefs->push_back(s); 
        return true;
     }
     // Function call returning C++ reference type is a memory access
     AstNodeTypeImpl t;
     if (s->variantT() == V_SgFunctionCallExp && IsExpression(_s,&t))
     {
      //member function's return type may have several levels of typedef
      //Strip SgTypedefType off to get the real base type
       SgType* base_type= t.get_ptr();
       assert(base_type!=0);
       while (isSgTypedefType(base_type))
           base_type = isSgTypedefType(base_type)->get_base_type();
       if (base_type->variantT() == V_SgReferenceType) {
         if (subrefs != 0) { subrefs->push_back(s); }
         return true;
       }
     }
     return false;
    }
  } // end switch
}

//! Check if _s is an array access.
//If so, store array name in array, and subscripts into index[]
bool AstInterface::
IsArrayAccess( const AstNodePtr& _s, AstNodePtr* array, AstList* index)
{
  SgNode* s = AstNodePtrImpl(_s).get_ptr();
  if (s == 0) return false;
  if (s->variantT() == V_SgDotExp) {
        SgDotExp* dot = isSgDotExp(s);
        s = dot->get_lhs_operand();
  }
  switch (s->variantT()) {
  case V_SgPntrArrRefExp:
      if (index != 0 || array != 0) {
        SgNode* n = s;
        while (true) {
          SgPntrArrRefExp *arr = isSgPntrArrRefExp(n);
          if (arr == 0)
            break;
          n = arr->get_lhs_operand();
          if (array != 0)
            *array = AstNodePtrImpl(n);
          if (index != 0) {
            // jichi(9/25/2009): Add Support for SgExprListExp for Fortran array index.
            SgNode* exp = arr->get_rhs_operand();
            switch(exp->variantT()) {
            case V_SgExprListExp:        // Fortan indices as expression list.
              {
                SgExprListExp* indexexp = isSgExprListExp(exp);
                assert(indexexp);

                SgExpressionPtrList &l = indexexp->get_expressions();
                SgExpressionPtrList::const_iterator p = l.begin(); 
                for ( ;p != l.end(); ++p) {
                  SgExpression *pr = isSgExpression((SgNode*)(*p));
                  assert(pr);
                  index->push_back(pr);
                }
              }
              break;
            default: index->push_back(exp);
            }
          }
        }
      }
      return true;
  default:
      break;
  }
  return false;
}

bool AstInterface::
IsBinaryOp( const AstNodePtr& _exp, OperatorEnum* opr,
            AstNodePtr* opd1, AstNodePtr* opd2)
{ 
  SgNode* exp = AstNodePtrImpl(_exp).get_ptr();
  if (exp == 0) return false;

  SgBinaryOp *op = isSgBinaryOp(exp);
  switch (exp->variantT()) {
    case V_SgEqualityOp:
         if (opr != 0)  *opr = BOP_EQ;
         break;
    case V_SgNotEqualOp:
         if (opr != 0)  *opr = BOP_NE;
         break;
    case V_SgGreaterOrEqualOp:
         if (opr != 0)  *opr = BOP_GE;
         break;
    case V_SgLessOrEqualOp:
         if (opr != 0)  *opr = BOP_LE;
         break;
    case V_SgLessThanOp:
         if (opr != 0)  *opr = BOP_LT;
         break;
    case V_SgGreaterThanOp:
         if (opr != 0)  *opr = BOP_GT;
         break;
    case V_SgAndOp:
         if (opr != 0)  *opr = BOP_AND;
         break;
    case V_SgOrOp:
         if (opr != 0)  *opr = BOP_OR;
         break;
    case V_SgMultiplyOp:
         if (opr != 0)  *opr = BOP_TIMES;
         break;
    case V_SgDivideOp:
         if (opr != 0)  *opr = BOP_DIVIDE; 
         break;
    case V_SgModOp:
         if (opr != 0)  *opr = BOP_MOD; 
         break;
    case V_SgAddOp:
         if (opr != 0)  *opr = BOP_PLUS; 
         break;
    case V_SgSubtractOp:
         if (opr != 0)  *opr = BOP_MINUS; 
         break;
    case V_SgDotExp:
        if (opr != 0)  *opr = BOP_DOT_ACCESS;
        break;
    case V_SgArrowExp:
        if (opr != 0)  *opr = BOP_ARROW_ACCESS;
        break;
   case V_SgBitOrOp:
        if (opr != 0)  *opr = BOP_BIT_OR;
        break;
   case V_SgBitAndOp:
        if (opr != 0)  *opr = BOP_BIT_AND;
        break;
   case V_SgRshiftOp:
        if (opr != 0)  *opr = BOP_BIT_RSHIFT;
        break;
   case V_SgLshiftOp:
        if (opr != 0)  *opr = BOP_BIT_LSHIFT;
        break;
    default:
       return false;
  }
  if (opd1 != 0) *opd1 = AstNodePtrImpl(op->get_lhs_operand()); 
  if (opd2 != 0) *opd2 = AstNodePtrImpl(op->get_rhs_operand()); 
  return true;
}

//! Check if $_exp$ is an unary operation; if yes, return its operation type and operand 
bool AstInterface::
IsUnaryOp( const AstNodePtr& _exp, OperatorEnum* opr, AstNodePtr* opd) 
{ 
  SgNode* exp = AstNodePtrImpl(_exp).get_ptr();
  if (exp == 0) return false;

  switch (exp->variantT()) {
    case V_SgMinusOp: 
       if (opd != 0) *opd = AstNodePtrImpl(isSgMinusOp(exp)->get_operand()); 
       if (opr != 0)  { *opr = UOP_MINUS;  }
       return true;
    case V_SgAddressOfOp :
       if (opd != 0) *opd = AstNodePtrImpl(isSgAddressOfOp(exp)->get_operand()); 
       if (opr != 0) *opr = UOP_ADDR; 
       return true;
    case V_SgPointerDerefExp: 
       if (opr != 0) *opr = UOP_DEREF; 
       if (opd != 0) *opd = AstNodePtrImpl(isSgPointerDerefExp(exp)->get_operand()); 
       return true;
    case V_SgNewExp:
       if (opr != 0) *opr = UOP_ALLOCATE; 
       if (opd != 0) *opd = AstNodePtrImpl(isSgNewExp(exp)->get_constructor_args());
       return true;
    case V_SgCastExp:
       if (opr != 0) {
          switch (isSgCastExp(exp)->cast_type()) {
          case SgCastExp::cast_type_enum::e_C_style_cast: *opr = UOP_CAST_C; break;
          case SgCastExp::cast_type_enum::e_const_cast: *opr = UOP_CAST_CONST; break;
          case SgCastExp::cast_type_enum::e_static_cast: *opr = UOP_CAST_STATIC; break;
          case SgCastExp::cast_type_enum::e_dynamic_cast: *opr = UOP_CAST_DYNAMIC; break;
          case SgCastExp::cast_type_enum::e_reinterpret_cast: *opr = UOP_CAST_REINTERP; break;
          case SgCastExp::cast_type_enum::e_safe_cast: *opr = UOP_CAST_SAFE; break;
          case SgCastExp::cast_type_enum::e_ada_type_qualification: *opr = UOP_CAST_ADA; break;
          default: std::cerr << "Error: unexpected type cast enum." << "\n"; ROSE_ASSERT(false);
          }
       }
       if (opd != 0) *opd = AstNodePtrImpl(isSgCastExp(exp)->get_operand()); 
       return true;
    case V_SgMinusMinusOp:
        if (opr != 0) *opr = (isSgMinusMinusOp(exp)->get_mode() == SgUnaryOp::Sgop_mode::prefix)? UOP_DECR1 : UOP_DECR1_POST; 
        if (opd != 0) *opd = AstNodePtrImpl(isSgMinusMinusOp(exp)->get_operand());
        return true;
    case V_SgPlusPlusOp:
        if (opr != 0) *opr = (isSgPlusPlusOp(exp)->get_mode() == SgUnaryOp::Sgop_mode::prefix)?  UOP_INCR1 : UOP_INCR1_POST; 
        if (opd != 0) *opd = AstNodePtrImpl(isSgPlusPlusOp(exp)->get_operand());
        return true;
    case V_SgAsmOp:
        if (opd != 0) *opd = AstNodePtrImpl(isSgAsmOp(exp)->get_expression());
        if (opr != 0) *opr = OP_UNKNOWN; 
        return true;
    case V_SgBitComplementOp: 
       if (opd != 0) *opd = AstNodePtrImpl(isSgBitComplementOp(exp)->get_operand());
       if (opr != 0)  { *opr = UOP_BIT_COMPLEMENT;  }
       return true;
    default: 
       return false;
  }
}

bool AstInterface::
IsBlock( const AstNodePtr& _n, std::string* blockname, AstNodeList* _stmts)
{
  if (_n.is_null()) { return false; }

  {
    AstNodePtr body;
    AstTypeList param_types;
    if (IsFunctionDefinition(_n, blockname, _stmts, 0, &body, &param_types)) {
      if (body != 0 && _stmts != 0) {
         _stmts->push_back(AstNodePtrImpl(body).get_ptr());
      } 
      if (blockname != 0) {
        *blockname = GetFunctionSignature(_n, *blockname, param_types);
      }
      return true;
    }
  }
  AstNodePtrImpl n(_n);
  SgStatementPtrList l;
  SgDeclarationStatementPtrList l2;
  switch (n->variantT()) {
  case V_SgBasicBlock:
    if (_stmts != 0) {
      l = isSgBasicBlock(n.get_ptr())->get_statements();
    }
    break;
  case V_SgDeclarationScope:
    if (blockname != 0) {
       *blockname = isSgDeclarationScope(n.get_ptr())->get_qualified_name();
    } 
    break;
  case V_SgSwitchStatement:
    if (_stmts != 0) {
       _stmts->push_back(isSgSwitchStatement(n.get_ptr())->get_body());
    }
    break;
  case V_SgForInitStatement:
    if (_stmts != 0) {
      l = isSgForInitStatement(n.get_ptr())->get_init_stmt();
    }
    break;
  case V_SgGlobal: 
     if (_stmts != 0) {
         l2 = isSgGlobal(n.get_ptr())->get_declarations();
     }
     break;
  case V_SgNamespaceDefinitionStatement:
     if (blockname != 0) {
         *blockname = isSgNamespaceDefinitionStatement(n.get_ptr())->get_namespaceDeclaration()->get_name().str();
     }
    if (_stmts != 0) {
        l2 = isSgNamespaceDefinitionStatement(n.get_ptr())->getDeclarationList();
    }
    break;
  case V_SgNamespaceDeclarationStatement:
     if (blockname != 0) {
         *blockname = isSgNamespaceDeclarationStatement(n.get_ptr())->get_name().str();
     }
    if (_stmts != 0) {
        l2 = isSgNamespaceDeclarationStatement(n.get_ptr())->get_definition()->getDeclarationList();
    }
    break;
   case V_SgClassDeclaration:
      // If there is no definition, it is not a block.
      if (isSgClassDeclaration(n.get_ptr())->get_definition() == 0) 
          return false;
      if (blockname != 0) {
          *blockname = isSgClassDeclaration(n.get_ptr())->get_name().str();
      }
      if (_stmts != 0) {
         l2 = isSgClassDeclaration(n.get_ptr())->get_definition()->getDeclarationList();
      }
      break;
  case V_SgTemplateInstantiationDefn:
  case V_SgTemplateClassDefinition:
  case V_SgClassDefinition: 
    if (blockname != 0) {
         *blockname = isSgClassDefinition(n.get_ptr())->get_declaration()->get_name().str();
     }
    if (_stmts != 0) {
        l2 = isSgClassDefinition(n.get_ptr())->getDeclarationList();
    }
    break;
  default: 
      return false;
  };
  if (_stmts != 0) {
    for (SgStatementPtrList::iterator p = l.begin(); p != l.end(); ++p) {
      _stmts->push_back(*p);
    }
    for (SgDeclarationStatementPtrList::iterator p = l2.begin(); p != l2.end(); ++p) {
      _stmts->push_back(*p);
    }
  }
  return true;
}

bool DeclarationAttributesIdentical(SgDeclarationStatement*  q1, SgDeclarationStatement* q2) {
  if (q1 == nullptr || q2 == nullptr) {
     DebugDiff([](){ return "AST attribute checking : at least one of the declaration is null."; });
     return q1 == q2;
  }
  if (q1->get_definingDeclaration() != 0 || q2->get_definingDeclaration() != 0) {
     q1 = q1->get_definingDeclaration();
     q2 = q2->get_definingDeclaration();
     if (q1 == nullptr || q2 == nullptr) {
       DebugDiff([](){ return "AST different attribute checking : only one of the definining declaration is null. Returnning identical"; });
       return true;
     }
  }
  DebugDiff([q1,q2](){ return "AST checking attribute: " + AstInterface::AstToString(q1) + " vs " + AstInterface::AstToString(q2); });
  if (q1->get_declarationModifier().get_storageModifier().isExtern() !=
    q2->get_declarationModifier().get_storageModifier().isExtern()) {
    DebugDiff([q1,q2](){ return "AST different attribute extern : " + AstInterface::AstToString(q1) + " vs " + AstInterface::AstToString(q2); });
    return false;
  }
  if (q1->get_declarationModifier().get_storageModifier().isStatic() !=
    q2->get_declarationModifier().get_storageModifier().isStatic()) {
    DebugDiff([q1,q2](){ return "AST different attribute static : " + AstInterface::AstToString(q1) + " vs " + AstInterface::AstToString(q2); });
    return false;
  }
  if (q1->get_declarationModifier().get_storageModifier().isMutable() !=
    q2->get_declarationModifier().get_storageModifier().isMutable()) {
    DebugDiff([q1,q2](){ return "AST different attribute mutable : " + AstInterface::AstToString(q1) + " vs " + AstInterface::AstToString(q2); });
    return false;
  }
  if (q1->get_declarationModifier().get_storageModifier().isRegister() !=
      q2->get_declarationModifier().get_storageModifier().isRegister()) { 
    DebugDiff([q1,q2](){ return "AST different attribute is register : " + AstInterface::AstToString(q1) + " vs " + AstInterface::AstToString(q2); });
    return false;
  }
  if (q1->get_declarationModifier().get_accessModifier().isPrivate() !=
      q2->get_declarationModifier().get_accessModifier().isPrivate()) { 
    DebugDiff([q1,q2](){ return "AST different attribute access modifier: " + AstInterface::AstToString(q1) + " vs " + AstInterface::AstToString(q2); });
    return false;
  }

   auto* p1 = isSgVariableDeclaration(q1), *p2 = isSgVariableDeclaration(q2);
   if (p1 != 0 && p2 != 0) {
      if (p1->get_is_constexpr() != p2->get_is_constexpr())  {
          DebugDiff([p1,p2](){ return "AST different attribute constexpr : " + AstInterface::AstToString(p1) + " vs " + AstInterface::AstToString(p2); });
          return false;
      }
      if (p1->get_is_thread_local() != p2->get_is_thread_local())  {
          DebugDiff([p1,p2](){ return "AST different attribute thread_local : " + AstInterface::AstToString(p1) + " vs " + AstInterface::AstToString(p2); });
          return false;
       }
  }


  return true;
}

bool AstInterface:: AstTypeIdentical(const AstNodeType& _first, const AstNodeType& _second, 
                   std::function<bool(const AstNodePtr&, const AstNodePtr&)>* call_on_diff)
{
  if (_first == _second) { return true; }
  SgType* first = AstNodeTypeImpl(_first).get_ptr(), *second = AstNodeTypeImpl(_second).get_ptr(); 
  DebugDiff([&first,&second](){ return "Checking AST Type Identical:" + first->class_name() + "::" + first->unparseToString() + " vs " + second->class_name() + "::" + second->unparseToString(); });
  if (first == 0 || second == 0) {
     DebugDiff([](){ return "AST different: one of them is null."; });
  }
  if (first->variantT() != second->variantT()) { 
      DebugDiff([&_first,&_second](){ return "AST different variant: " + GetTypeName(_first) + " vs " + GetTypeName(_second); });
      return false;
  }
  switch (first->variantT()) {
   case V_SgEnumType: {
        SgEnumType* t1 = isSgEnumType(first), *t2 = isSgEnumType(second);
        std::string n1 = std::string(t1->get_name().str()), n2 = std::string(t2->get_name().str());
        return (n1 == n2 || (call_on_diff != 0 && !(*call_on_diff)(n1,n2))) 
              && AstIdentical(t1->get_declaration(), t2->get_declaration(), call_on_diff);
     }
  case V_SgPointerType: 
       return AstTypeIdentical(static_cast<SgPointerType*>(first)->get_base_type(), static_cast<SgPointerType*>(second)->get_base_type());
  case V_SgReferenceType: 
       return AstTypeIdentical(static_cast<SgReferenceType*>(first)->get_base_type(), static_cast<SgReferenceType*>(second)->get_base_type());
  case V_SgModifierType: 
       return AstTypeIdentical(static_cast<SgModifierType*>(first)->get_base_type(), static_cast<SgModifierType*>(second)->get_base_type());
  case V_SgArrayType: 
       return AstTypeIdentical(static_cast<SgArrayType*>(first)->get_base_type(), static_cast<SgArrayType*>(second)->get_base_type()) && static_cast<SgArrayType*>(first)->get_rank() == static_cast<SgArrayType*>(second)->get_rank() && AstIdentical(static_cast<SgArrayType*>(first)->get_index(), static_cast<SgArrayType*>(second)->get_index());
  default:
      if (first->unparseToString() == second->unparseToString()) {
        DebugDiff([](){ return "Ast Type is equivalent."; });
        return true;
      }
  }
  DebugDiff([](){ return "Ast Type is not equivalent."; });
  return false;
}

bool AstInterface:: AstIdentical(const AstNodePtr& _first, const AstNodePtr& _second,
                   std::function<bool(const AstNodePtr&, const AstNodePtr&)>* call_on_diff) {
  SgNode* first = AstNodePtrImpl(_first).get_ptr(); 
  SgNode* second = AstNodePtrImpl(_second).get_ptr(); 
  if (first == second) {
    return true;
  }
  DebugDiff([&_first,&_second](){ return "Checking AST Identical:" + AstToString(_first) + " vs " + AstToString(_second); });
  if (first == 0 || second == 0) {
     DebugDiff([](){ return "AST different: one of them is null."; });
     return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
  }
  if (first->variantT() != second->variantT()) { 
      DebugDiff([&_first,&_second](){ return "AST different variant: " + AstToString(_first) + " vs " + AstToString(_second); });
     return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
  }
  std::string name1, name2;
  AstNodePtr f1, f2;
  AstNodeList params1, params2, args1, args2;
  AstNodePtr body1, body2;
  AstTypeList paramtypes1, paramtypes2;
  AstNodeType returntype1, returntype2;
  // Both first and second should have the same variantT.
  switch (first->variantT()) {
  case V_SgLabelStatement:
       return AstIdentical(isSgLabelStatement(first)->get_statement(),
                           isSgLabelStatement(second)->get_statement(), call_on_diff);
  case V_SgInitializedName: {
        SgInitializedName *p1 = isSgInitializedName(first);
        SgInitializedName *p2 = isSgInitializedName(second);
        assert (p1 != 0 && p2 != 0); 
        if (!DeclarationAttributesIdentical(p1->get_declaration(), p2->get_declaration())) {
            DebugDiff([&_first,&_second](){ return "AST different declaration attribute: " + AstToString(_first) + " vs " + AstToString(_second); });
            return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
        }
        if (std::string(p1->get_name().str()) != std::string(p2->get_name().str())) {
            DebugDiff([&_first,&_second](){ return "AST different  name: " + AstToString(_first) + " vs " + AstToString(_second); });
            return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
        }
        if (!AstTypeIdentical(AstNodeType(p1->get_type()), AstNodeType(p2->get_type()), call_on_diff)) {
            DebugDiff([&_first,&_second](){ return "AST different  type: " + AstToString(_first) + " vs " + AstToString(_second); });
            return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
        }
        if (!AstIdentical(AstNodePtr(p1->get_initializer()), AstNodePtr(p2->get_initializer()), call_on_diff)) {
            DebugDiff([&_first,&_second](){ return "AST different  initializer: " + AstToString(_first) + " vs " + AstToString(_second); });
           return false;
        }
        SgVariableDefinition* d1 = isSgVariableDefinition(p1->get_declptr());
        SgVariableDefinition* d2 = isSgVariableDefinition(p2->get_declptr());
        if (d1 != 0 && d2 != 0) {
            SgExpression* b1 = d1->get_bitfield(), *b2 = d2->get_bitfield();
             if (b1 != 0 && b2 != 0 && !AstIdentical(b1,b2,call_on_diff)) {
                 DebugDiff([&_first,&_second](){ return "AST different bit filed declptr: " + AstToString(_first) + " vs " + AstToString(_second); });
                 return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
             }
             if (!DeclarationAttributesIdentical(d1->get_declaration(), d2->get_declaration())) {
                  DebugDiff([&_first,&_second](){ return "AST different declaration attribute declptr: " + AstToString(_first) + " vs " + AstToString(_second); });
                 return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
             }
        }
        return true;
    }
  case V_SgVariableDeclaration: {
       auto* p1 = isSgVariableDeclaration(first), *p2 = isSgVariableDeclaration(second);
       if (!DeclarationAttributesIdentical(p1,p2)) {
             DebugDiff([&_first,&_second](){ return "AST different declaration attribute declptr: " + AstToString(_first) + " vs " + AstToString(_second); });
            return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
       }
       return AstListIdentical<SgInitializedNamePtrList,AstNodePtr>(p1->get_variables(), p2->get_variables(), call_on_diff);
     }
  case V_SgClassDefinition: {
      SgClassDefinition* p1 = isSgClassDefinition(first);
      SgClassDefinition* p2 = isSgClassDefinition(second);
      assert (p1 != 0 && p2 != 0); 
      const std::vector<SgBaseClass*>& base1 = p1->get_inheritances(), &base2 = p2->get_inheritances();
      if (!AstListIdentical<std::vector<SgBaseClass*>, AstNodePtr>(base1, base2, call_on_diff)) {
         DebugDiff([&_first,&_second](){ return "AST different base classes: " + AstToString(_first) + " vs " + AstToString(_second); });
         return false;
      }
      if (p1->get_declaration()->get_declarationModifier().isFinal() !=
          p2->get_declaration()->get_declarationModifier().isFinal()) {
         DebugDiff([&_first,&_second](){ return "AST different final declaration: " + AstToString(_first) + " vs " + AstToString(_second); });
         return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      // The rest will be handled by the IsBlock check below.
      break;
   }
   case V_SgBaseClass: {
      SgBaseClass* p1 = isSgBaseClass(first);
      SgBaseClass* p2 = isSgBaseClass(second);
      if (p1->get_baseClassModifier()->get_modifier() == p2->get_baseClassModifier()->get_modifier() && p1->get_baseClassModifier()->get_accessModifier() == p2->get_baseClassModifier()->get_accessModifier() &&
          std::string(p1->get_base_class()->get_name().str()) == std::string(p2->get_base_class()->get_name().str())) { 
         return true;
      }
      DebugDiff([&_first,&_second](){ return "AST different base class: " + AstToString(_first) + " vs " + AstToString(_second); });
      return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
    } 
  case V_SgMemberFunctionDeclaration:
  case V_SgFunctionDeclaration: {
      SgFunctionDeclaration* decl1 = isSgFunctionDeclaration(first), *decl2 = isSgFunctionDeclaration(second);
      assert (decl1 != 0 && decl2 != 0); 
      if (!DeclarationAttributesIdentical(decl1,decl2)) return false;
      if (decl1->get_is_constexpr() != decl2->get_is_constexpr())  {
          DebugDiff([decl1,decl2](){ return "AST different attribute constexpr : " + AstInterface::AstToString(decl1) + " vs " + AstInterface::AstToString(decl2); });
          return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      bool v1 = decl1->get_functionModifier().isVirtual();
      bool v2 = decl2->get_functionModifier().isVirtual();
      if (v1 != v2)  { 
         DebugDiff([v1,v2](){ return "function different attribute virtual: " + std::string(v1?"true":"false") + " vs " + std::string(v2?"true":"false"); });
         return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      v1 = decl1->get_functionModifier().isMarkedDelete();
      v2 = decl2->get_functionModifier().isMarkedDelete();
      if (v1 != v2)  { 
         DebugDiff([v1,v2](){ return "function different attribute delete: " + std::string(v1?"true":"false") + " vs " + std::string(v2?"true":"false"); });
         return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      v1 = decl1->get_declarationModifier().isFinal();
      v2 = decl2->get_declarationModifier().isFinal();
      if (v1 != v2)  { 
        DebugDiff([v1,v2](){ return "function different attribute final: " + std::string(v1?"true":"false") + " vs " + std::string(v2?"true":"false"); });
         return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      v1 = decl1->get_functionModifier().isMarkedDefault();
      v2 = decl2->get_functionModifier().isMarkedDefault(); 
      if (v1 != v2)  { 
        DebugDiff([v1,v2](){ return "function different attribute default: " + std::string(v1?"true":"false") + " vs " + std::string(v2?"true":"false"); });
        return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      v1 = decl1->get_functionModifier().isInline();
      v2 = decl2->get_functionModifier().isInline(); 
      if (v1 != v2)  { 
         DebugDiff([v1,v2](){ return "function attribute inline: " + std::string(v1?"true":"false") + " vs " + std::string(v2?"true":"false"); });
         return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      v1 = decl1->get_functionModifier().isGnuAttributeNoThrow();
      v2 = decl2->get_functionModifier().isGnuAttributeNoThrow(); 
      if (v1 != v2)  { 
         DebugDiff([v1,v2](){ return "function attribute no_throw: " + std::string(v1?"true":"false") + " vs " + std::string(v2?"true":"false"); });
         return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      v1 = decl1->get_functionModifier().isExplicit();
      v2 = decl2->get_functionModifier().isExplicit(); 
      if (v1 != v2)  { 
         DebugDiff([v1,v2](){ return "function attribute explicit: " + std::string(v1?"true":"false") + " vs " + std::string(v2?"true":"false"); });
         return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      break;
     }
   case V_SgTypedefDeclaration:
   case V_SgTemplateTypedefDeclaration: {
      SgTypedefDeclaration* decl1 = isSgTypedefDeclaration(first), *decl2 = isSgTypedefDeclaration(second);
      assert(decl1 != 0 && decl2 != 0);
      if (std::string(decl1->get_name().str()) != std::string(decl2->get_name().str())) {
         DebugDiff([&_first,&_second](){ return "AST different typedef name: " + AstToString(_first) + " vs " + AstToString(_second); });
         return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      } 
      return AstTypeIdentical(decl1->get_base_type(), decl2->get_base_type());
    }
   default: break;
  }
  if (IsVarRef(first)) {
     if (!IsSameVarRef(first,second, call_on_diff)) {
        DebugDiff([&_first,&_second](){ return "AST different variable name: " + AstToString(_first) + " vs " + AstToString(_second); });
        return false;
     }
     return true;
  }
  if (IsFunctionCall(_first, &f1, &params1, &args1, 0, &returntype1) && 
       IsFunctionCall(_second, &f2, &params2, &args2, 0, &returntype2)) {
     return AstIdentical(f1, f2, call_on_diff) &&
             AstListIdentical<AstNodeList, AstNodePtr>(params1, params2, call_on_diff) &&
             AstListIdentical<AstNodeList, AstNodePtr>(args1, args2, call_on_diff) &&
             AstIdentical(body1, body2, call_on_diff) && AstTypeIdentical(returntype1, returntype2);
  }
  if ((IsFunctionDefinition(first, &name1, &params1, &args1, &body1, 0, &returntype1) &&
      IsFunctionDefinition(_second,&name2, &params2, &args2, &body2, 0, &returntype2))) {
      if (name1 != name2 &&   (call_on_diff == 0 || (*call_on_diff)(name1, name2))) {
         DebugDiff([&_first,&_second](){ return "AST different function name: " + AstToString(_first) + " vs " + AstToString(_second); });
         return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      auto bodydiff = AstIdentical(body1, body2, call_on_diff); 
      return AstListIdentical<AstNodeList, AstNodePtr>(params1, params2, call_on_diff) &&
             AstListIdentical<AstNodeList, AstNodePtr>(args1, args2, call_on_diff) &&
             AstTypeIdentical(returntype1, returntype2, call_on_diff) && 
             bodydiff; 
  }
  if (IsBlock(_first, 0, &args1) && IsBlock(_second, 0, &args2)) {
     return AstListIdentical<AstNodeList,AstNodePtr>(args1, args2, call_on_diff);
  }
  { AstNodePtr exp1, exp2;
    if ( (IsExprStmt(_first, &exp1) && IsExprStmt(_second, &exp2)) ||
         (IsReturn(_first, &exp1) && IsReturn(_second, &exp2)) ) {
       return AstIdentical(exp1, exp2, call_on_diff);
    }
  }
  { AstNodePtr init1, init2, cond1, cond2, step1, step2, body1, body2;
    if (IsLoop(_first, &init1, &cond1, &step1, &body1) && 
        IsLoop(_second,&init2, &cond2, &step2, &body2)) {
       auto bodydiff = AstIdentical(body1, body2, call_on_diff); 
       return(
             AstIdentical(init1, init2, call_on_diff) &&  
             AstIdentical(cond1, cond2, call_on_diff) &&  
             AstIdentical(step1, step2, call_on_diff) &&  
             bodydiff); 
    }
  }

  { AstNodePtr cond1, cond2, body1, body2, falsebody1, falsebody2;
    if (IsIf(_first, &cond1, &body1, &falsebody1) && IsIf(_second, &cond2, &body2, &falsebody2)) {
       // Here we force the invocation of comparison on bodies to support computing statistics.
       auto truediff = AstIdentical(body1, body2, call_on_diff); 
       auto falsediff = AstIdentical(falsebody1, falsebody2, call_on_diff);
       return(AstIdentical(cond1, cond2, call_on_diff) && truediff && falsediff);
    }
  }
  { AstNodePtr lhs1, lhs2, rhs1, rhs2;
    if (IsAssignment(_first, &lhs1, &rhs1) && IsAssignment(_second, &lhs2, &rhs2) && 
       /* Check assignment only for regular assignments. Not irregular ones such as +=, -=*/
       lhs1 != _first && lhs2 != _second && rhs1 != _first && rhs2 != _second) {
      return ( AstIdentical(lhs1, lhs2, call_on_diff) && AstIdentical(rhs1, rhs2, call_on_diff));
    }
  }
  { AstNodePtr lhs1, lhs2, rhs1, rhs2;
    OperatorEnum opr1, opr2;
    if (IsBinaryOp(_first, &opr1, &lhs1, &rhs1) && IsBinaryOp(_second, &opr2, &lhs2, &rhs2)) {
      if (opr1 != opr2) {
         DebugDiff([&_first,&_second](){ return "AST different operation: " + AstToString(_first) + " vs " + AstToString(_second); });
         return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      return AstIdentical(lhs1, lhs2, call_on_diff) && AstIdentical(rhs1, rhs2, call_on_diff);
    }
    if (IsUnaryOp(_first, &opr1, &lhs1) && IsUnaryOp(_second, &opr2, &lhs2)) {
      if (opr1 != opr2) {
         DebugDiff([&_first,&_second](){ return "AST different operation: " + AstToString(_first) + " vs " + AstToString(_second); });
         return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
      }
      return AstIdentical(lhs1, lhs2, call_on_diff);
    }
  }
  if (AstToString(_first) != AstToString(_second)) {
    DebugDiff([&_first,&_second](){ return "AST different unparseToString:" + AstToString(_first) + " vs " + AstToString(_second); });
    return (call_on_diff != 0 && !(*call_on_diff)(_first,_second)); 
  } 
  DebugDiff([](){ return "AST identical\n";});
  return true;
}

//! Check if $s$ is a function call; if yes, return the function and arguments
bool AstInterfaceImpl::
IsFunctionCall( SgNode* s, SgNode** func, AstNodeList* args, AstTypeList* paramtypes, AstNodeType* returntype)
{
  SgNode *exp = s;
  SgNode *f = 0;
  SgExprListExp *argexp = 0;
 
  if (exp == 0) return false; 
  switch (exp->variantT()) {
  case V_SgExprStatement:
     exp = isSgExprStatement(exp)->get_expression();
     return IsFunctionCall(exp, func, args, paramtypes, returntype);
  case V_SgAssignInitializer:
    exp = isSgAssignInitializer(exp)->get_operand();
    return IsFunctionCall(exp, func, args, paramtypes, returntype);
  case V_SgConstructorInitializer: {
    auto* init = isSgConstructorInitializer(exp); 
    f = init->get_declaration();
    if (f == 0) return false;
    argexp = init->get_args();
    break;
  }
  case V_SgFunctionCallExp:
    {
      SgFunctionCallExp *fs = isSgFunctionCallExp(exp);
      f = fs->get_function(); //Can be SgFunctionRefExp or a class method.
      argexp = fs->get_args(); // SgExprListExp
    }
    break;
  default:
    DebugVariable([&exp](){ return "Not Function call:" + exp->class_name(); });
    return false;
  }
  
  DebugVariable([&f](){ return "Function being called:" + AstInterface::AstToString(f); });
  SgExpression* object_arg = 0;
  AstNodeType object_type;
  switch (f->variantT()) {
  case V_SgDotExp: 
    { 
        SgDotExp* dot = isSgDotExp(f);
        object_arg = dot->get_lhs_operand();
        object_type = AstInterface::GetExpressionType(object_arg);
        f = dot->get_rhs_operand();
      }
      break;
  case V_SgArrowExp:
      { 
        SgArrowExp* arrow = isSgArrowExp(f);
        object_arg = arrow->get_lhs_operand();
        object_type = AstInterface::GetExpressionType(object_arg);
        f = arrow->get_rhs_operand();
      }
      break;
  case V_SgArrowStarOp:
      { 
        SgArrowStarOp* arrow = isSgArrowStarOp(f);
        object_arg = arrow->get_lhs_operand();
        object_type = AstInterface::GetExpressionType(object_arg);
        f = arrow->get_rhs_operand();
      }
      break;
  case V_SgPntrArrRefExp:
      { 
        SgPntrArrRefExp* arrow = isSgPntrArrRefExp(f);
        object_arg = arrow->get_lhs_operand();
        object_type = AstInterface::GetExpressionType(object_arg);
        f = arrow->get_rhs_operand();
      }
      break;
  case V_SgTemplateMemberFunctionRefExp:
     {
       SgTemplateMemberFunctionSymbol* sym = isSgTemplateMemberFunctionRefExp(f)->get_symbol();
       // QY: This is not fully correct but how do we know whehter the this object is passed in as parameter already?
       if (sym->get_name() == "operator()") break;
       if (sym->get_scope() ==0)  break;
       if (argexp == 0 || argexp->get_expressions().empty() || argexp->get_expressions().size() == sym->get_declaration()->get_args().size()) {
          SgClassDeclaration* decl = sym->get_scope()->get_declaration();
          NEW_THIS_EXP(object_arg, decl);
          object_type = AstNodeTypeImpl(decl->get_type());
       }
       break;
     }
  default: break;
  }
  if (object_arg != 0) {
     if (args != 0) {
       args->push_back( object_arg ); 
     }
  }
 if (argexp != 0) {
      SgExpressionPtrList l = argexp->get_expressions();
      for ( SgExpressionPtrList::iterator p = l.begin(); p != l.end(); ++p) {
            SgExpression* cur = *p;
            DebugVariable([&cur](){ return "Call argument:" + AstInterface::AstToString(cur); });
            if (args != 0)  { args->push_back(cur); }
      }
  }
  if (paramtypes != 0 || returntype != 0) {
     AstNodeType _ftype;
     if (AstInterface::IsVarRef(AstNodePtrImpl(f), &_ftype)) {
        SgType* t = AstNodeTypeImpl(_ftype).get_ptr();
        if (t != 0 && t->variantT() == V_SgPointerType)
           t = static_cast<SgPointerType*>(t)->get_base_type();
        if (!AstInterface::IsFunctionType(_ftype, paramtypes, returntype)) {// not a function type
            DebugVariable([&s](){ return "Non-function type called: " +  AstInterface::AstToString(s); });
            return false;
        } 
     }
 }
 if (func != 0)
    *func = f;
 return true;
}

/* Does not deal correctly with templates SgNorealExp */
bool AstInterface::
IsFunctionCall( const AstNodePtr& _s, AstNodePtr* fname, AstNodeList* args, 
                AstNodeList* outargs, AstTypeList* paramtypes, AstNodeType* returntype)
{
  DebugVariable([&_s](){ return "Checking IsFunctionCall: " + AstToString(_s); });
  AstNodePtrImpl s(_s);
  AstNodeList arglist;
  AstTypeList paramtypelist;
  if (outargs != 0 && paramtypes == 0) {
     paramtypes = &paramtypelist; 
  }
  if (outargs != 0 && args == 0) {
    args = &arglist;
  }
  SgNode* f;
  // Grab functionRefExp and argument expression list
  if (!AstInterfaceImpl::IsFunctionCall(s.get_ptr(), &f, args, paramtypes, returntype))
     return false;
     
  if (f->variantT() == V_SgPointerDerefExp)
     f = isSgPointerDerefExp(f)->get_operand();
  if (fname != 0) {
    *fname = AstNodePtrImpl(f);
  }
  // Store arguments of reference types into outargs
  if (outargs != 0) {
        assert(paramtypes != 0 && args != 0);
          AstNodeList::const_iterator p1 = args->begin();
          for (AstTypeList::const_iterator p = paramtypes->begin(); 
             p != paramtypes->end() && p1 != args->end(); ++p,++p1) {
             SgType* t = AstNodeTypeImpl(*p).get_ptr();
             AstNodePtr ref = *p1;
             if (t != 0 && 
                 ((t->variantT() == V_SgReferenceType)  || 
                      (t->variantT() == V_SgPointerType && IsAddressOfOp(*p1, &ref)))) {
                auto* modifier = isSgConstVolatileModifier(t->get_modifiers());
                if (modifier == 0 || !modifier->isConst()) {
                   outargs->push_back(ref); 
                }
             }
        }
  } 
  return true;
}

AstNodeType AstInterface::
GetBaseType(const AstNodeType& t) {
  {
    SgPointerType* pointer_type = isSgPointerType(t.get_ptr());
    if (pointer_type != 0) {
       return GetBaseType(pointer_type->get_base_type());
    }
  }
  {
    SgModifierType* mod_type = isSgModifierType(t.get_ptr());
    if (mod_type != 0) {
       return GetBaseType(mod_type->get_base_type());
    }
  }
  return t;
}


void AstInterfaceImpl::
GetTypeInfo(SgType* t, std::string *tname, std::string* stripname, int* size, bool use_global_name)
{
  {
    SgPointerType* pointer_type = isSgPointerType(t);
    if (pointer_type != 0) {
       GetTypeInfo(pointer_type->get_base_type(), tname, stripname, size, use_global_name);
       if (tname != 0) {
          *tname = (*tname) + "_ptr_";
       }
       return;
    }
  }
  std::string typeName = get_type_name(t);
  // For instantiated template types, return the original template type name.
  if (isSgNamedType(t))
  {
    SgDeclarationStatement * decl = isSgNamedType(t)->get_declaration();
    SgTemplateInstantiationDecl* insDecl= isSgTemplateInstantiationDecl(decl);
    if (insDecl) {
      typeName=insDecl->get_templateDeclaration()->get_qualified_name();
    }
    else if (use_global_name) {
      typeName = AstInterface::GetGlobalUniqueName(decl, typeName, /*do not use file name*/true);
    } 
  }

  std::string r1 = ::StripGlobalQualifier(typeName);
  std::string result = "";
  for (size_t i = 0; i < r1.size(); ++i) {
    if (r1[i] == '[' || r1[i] == ']' || r1[i] == '{' || r1[i] == '}' || r1[i] == ',' || r1[i] == '.') {
      result.push_back('_');
    }
    else if (r1[i] == '&') {
      result = result + "_ref_";
    }
    else if (r1[i] == '*') {
      result = result + "_deref_";
    }
    else if (r1[i] != ' ')
      result.push_back(r1[i]);
    else if (i + 2 < r1.size() && r1[i+1]==':' && r1[i+2]==':') {
        i+= 2; 
    }
  }
  if (tname != 0) {
    *tname = result;
  }
  if (stripname != 0)
    *stripname = StripParameterType(result);
  if (size != 0)
    *size = 4;
}

void AstInterface::
GetTypeInfo(const AstNodeType& t, std::string *tname, std::string* stripname, int* size, bool use_global_name)
{ AstInterfaceImpl::GetTypeInfo(AstNodeTypeImpl(t).get_ptr(), tname, stripname, size, use_global_name); }

bool
AstInterface::IsPointerType(const AstNodeType& __type)
{
  AstNodeTypeImpl type(__type);
  return type.get_ptr()->variantT() == V_SgPointerType;
}

bool
AstInterface::IsArrayType(const AstNodeType& __type, int* __dim,
                          AstNodeType* __base_type, std::string* annotation)
{
  AstNodeTypeImpl type(__type);
  SgArrayType* t = isSgArrayType(type.get_ptr());
  if (t == 0)
    return false;

  if (__base_type)
    (*__base_type) = AstNodeTypeImpl(t->get_base_type());
  if (__dim)
    (*__dim) = t->get_rank();
  if (annotation != 0) {
/*
    SgDeclarationStatement *d = t->getAssociatedDeclaration ();
    if (p != NULL) {
      *annotation = p->getString();
std::cerr << "ANNOTATION:" << *annotation << "\n";
    } 
*/
  }
  return true;
}

bool
AstInterface::IsScalarType(const AstNodeType& __type)
{
  AstNodeTypeImpl type(__type);
  if (type.get_ptr() == 0) return false;
  switch(type->variantT()) {
  case V_SgTypeChar :
  case V_SgTypeSignedChar :
  case V_SgTypeUnsignedChar :
  case V_SgTypeShort :
  case V_SgTypeSignedShort :
  case V_SgTypeUnsignedShort :
  case V_SgTypeInt :
  case V_SgTypeSignedInt :
  case V_SgTypeUnsignedInt :
  case V_SgTypeLong :
  case V_SgTypeSignedLong :
  case V_SgTypeUnsignedLong :
  case V_SgTypeVoid :
  case V_SgTypeWchar:
  case V_SgTypeFloat:
  case V_SgTypeDouble:
  case V_SgTypeLongLong:
  case V_SgTypeUnsignedLongLong:
  case V_SgTypeLongDouble:
  case V_SgTypeString:
  case V_SgTypeBool:
  case V_SgTypeComplex:
  case V_SgTypeImaginary:
     return true;
  default: break;
  }
  return false;
}

bool AstInterface::GetArrayBound( const AstNodePtr& _arrayref, int dim, int &lb, int &ub) 
    { 
      AstNodePtrImpl arrayref(_arrayref);
      SgNode *n = arrayref.get_ptr();
      SgArrayType *t = 0;
      if (dim == 0) {
         SgVarRefExp *var = isSgVarRefExp( n );
         if (var == 0)
             return false;
         SgVariableSymbol *sb = var->get_symbol();
         SgType* vt = sb->get_type(); 
         t = isSgArrayType( vt);
      }
      else {
         for (int i = 0; i < dim; ++i) {
           n = n->get_parent();
         }
         SgPntrArrRefExp *ar = isSgPntrArrRefExp(n);
         if (ar == 0)
            return false;
         t = isSgArrayType(ar->get_type());
      }
      if (t == 0)
          return false;
      SgExpression *exp = t->get_index();
      lb = 0;
      return IsConstInt( AstNodePtrImpl(exp), &ub);
    }

//! Check whether $_s$ is an expression; If yes, return the expression 
// (strip off SgExpressionRoot) and grab its type
bool AstInterface::
IsExpression( const AstNodePtr& _s, AstNodeType* exptype, AstNodePtr* strip_exp)
{
  AstNodePtrImpl s(_s);
  if (s.get_ptr() ==  0) return false; 
  {
   SgExprStatement* is_expstmt = isSgExprStatement(s.get_ptr());
   if (is_expstmt != 0) {
     s = AstNodePtrImpl(is_expstmt->get_expression());
   }
  }
  if (IsVarRef(s, exptype)) {
     if (strip_exp != 0) *strip_exp = s;  
     return true;
  }
  {
  SgExpression* exp = isSgExpression(s.get_ptr());
  if (exp != 0) {
    switch (exp->variantT()) {
      case V_SgExpressionRoot:
           exp = isSgExpressionRoot(exp)->get_operand();
           break;
      default: break;
    }
    if (exptype != 0) {
      if (exp->get_type() != 0) {
         *exptype = AstNodeTypeImpl(exp->get_type());
      } else {
         *exptype = AstNodeType(AstNodeType::SpecialAstType::UNKNOWN_TYPE);
      }
    }
    if (strip_exp != 0) *strip_exp = AstNodePtrImpl(exp);
    return true;
   }
  }
  return false;
}

// if yes, grab init, condition, increment, and body
bool AstInterface::
IsLoop( const AstNodePtr& _s, AstNodePtr* init, AstNodePtr* cond,
        AstNodePtr* incr, AstNodePtr* body)
{
  AstNodePtrImpl s(_s);
  if (s.get_ptr() == 0) return false;
  switch (s->variantT()) {
  case V_SgForStatement:
    {
      SgForStatement *f = isSgForStatement(s.get_ptr());
      if (init != 0) {
        SgForInitStatement* pinit = f->get_for_init_stmt(); 
        if (pinit != 0 && pinit->get_init_stmt().size() == 0)
           pinit = 0;
        *init = AstNodePtrImpl(pinit);
      }
      if (incr != 0)
        *incr = AstNodePtrImpl(f->get_increment());
      if (cond != 0)
        *cond = AstNodePtrImpl(f->get_test());
      if (body != 0)
        *body = AstNodePtrImpl(f->get_loop_body());
    }
    break;
  case V_SgWhileStmt:
    {
      SgWhileStmt* w = isSgWhileStmt(s.get_ptr());
      if (init != 0)
        *init = AST_NULL;
      if (incr != 0)
        *incr = AST_NULL;
      if (cond != 0)
        *cond = AstNodePtrImpl(w->get_condition());
      if (body != 0)
        *body = AstNodePtrImpl(w->get_body());
    }
    break;
  case V_SgDoWhileStmt:
    {
      SgDoWhileStmt *w = isSgDoWhileStmt(s.get_ptr());
      if (init != 0)
        *init = AST_NULL;
      if (incr != 0)
        *incr = AST_NULL;
      if (cond != 0)
        *cond = AstNodePtrImpl(w->get_condition());
      if (body != 0)
        *body = AstNodePtrImpl(w->get_body());
    }
    break;

  case V_SgFortranDo:
    // FIXME: increment/bound in fortran are not equivalent to incr/cond in Cxx.
    {
      SgFortranDo *f = isSgFortranDo(s.get_ptr());
      if (init != 0)
        *init = AstNodePtrImpl(f->get_initialization());
      if (incr != 0)
        *incr = AstNodePtrImpl(f->get_increment());
      if (cond != 0)
        *cond = AstNodePtrImpl(f->get_bound());
      if (body != 0)
        *body = AstNodePtrImpl(f->get_body());
    }
    break;

  default:
    return false;
  }
  return true;
}

// The loop must be in the format: for (ivar=lb; ivar <= ub; ivar += step)
bool AstInterfaceImpl::IsFortranLoop( const SgNode* s, SgNode** ivar , SgNode** lb , SgNode** ub, SgNode** step, SgNode** body)
{ 
  if (s == 0) return false;
  switch (s->variantT()) {
    case V_SgFortranDo:
    {
      const SgFortranDo *f = isSgFortranDo(s);
      SgExpression *init = f->get_initialization();
      SgNode* ivarast, *lbast;
      if (!IsAssignment( init, &ivarast, &lbast))
        ROSE_ABORT();

      if (ivar != 0) *ivar = ivarast;
      if (lb != 0) *lb = lbast;
      if (ub != 0) *ub = f->get_bound();
      if (step != 0) *step = f->get_increment();
      if (body != 0) *body = f->get_body();
    }
    return true;

  case V_SgForStatement:
    {
      const SgForStatement *fs = isSgForStatement(s);
      const SgStatementPtrList &init = fs->get_init_stmt();
      if (init.size() != 1) return false;
      
      SgNode* init1= init.front();
      SgNode *ivarast, *lbast, *ubast, *stepast;
 

      if (!IsAssignment( init1, &ivarast, &lbast)) {
         SgVariableDeclaration *decl = isSgVariableDeclaration(init1);
         if (decl != 0) {
           SgInitializedNamePtrList& names = decl->get_variables();
           if (names.size() != 1) return false;
           SgInitializedName* var = names.front();
           ivarast = var;
           SgExpression* def = var->get_initializer();
           if (def != 0 && def->variantT() != V_SgAssignInitializer)
               return false;
           lbast = isSgAssignInitializer(def)->get_operand();
         }
         else return false;
      }
      std::string varname;
      if (!IsVarRef(ivarast, 0, &varname))
        return false; 
  
      SgExpression* test = fs->get_test_expr();
      int t = test->variantT();
      switch (t) {
      case V_SgLessOrEqualOp:
      case V_SgGreaterOrEqualOp:
      case V_SgNotEqualOp:
        break;
      default:  
        return false;
      }
  
      SgNode* testlhs = isSgBinaryOp(test)->get_lhs_operand();
      std::string testvarname;
      if (!IsVarRef(SkipCasting(testlhs), 0, &testvarname) ||
           varname != testvarname)
        return false;
  
      ubast = isSgBinaryOp(test)->get_rhs_operand();
      SgExpression* incr = fs->get_increment();
      switch (incr->variantT()) {
      case V_SgPlusAssignOp:
        break;
      default:
        return false;
       }
  
      SgNode* incrlhs = isSgBinaryOp(incr)->get_lhs_operand();
      std::string incrvarname;
      if ( !IsVarRef(SkipCasting(incrlhs), 0, &incrvarname) ||
          varname != incrvarname) 
        return false;
      stepast = isSgBinaryOp(incr)->get_rhs_operand();
      if (ivar != 0) *ivar = ivarast;
      if (lb != 0) *lb = lbast;
      if (ub != 0) *ub = ubast;
      if (step != 0) *step = stepast;
      if (body != 0) *body = fs->get_loop_body();
    }
    return true;

  default:
    return false; 
  }
}

bool AstInterface::IsFortranLoop( const AstNodePtr& _s, AstNodePtr* ivar , AstNodePtr* lb , AstNodePtr* ub, AstNodePtr* step, AstNodePtr* body)
{ 
  AstNodePtrImpl s(_s);
  if (s.get_ptr() == 0) return false;

  SgNode* local_ivar = 0, *local_lb = 0, *local_ub = 0, *local_step = 0, *local_body = 0;

  if (AstInterfaceImpl::IsFortranLoop(s.get_ptr(),&local_ivar, &local_lb, &local_ub, &local_step, &local_body)) {
     if (ivar != 0) *ivar = local_ivar;
     if (lb != 0) *lb = local_lb;
     if (ub != 0) *ub = local_ub;
     if (step != 0) *step = local_step;
     if (body != 0) *body = local_body;
     return true;
  }
  return false; 
}

bool AstInterface::IsPostTestLoop( const AstNodePtr& _s)
{
  AstNodePtrImpl s(_s);
  if (s.get_ptr() == 0) return false;
  switch (s->variantT()) {
  case V_SgDoWhileStmt:
    return true;
  default: break;
  }
  return false;
}

AstNodePtr AstInterface::
CreateLoop( const AstNodePtr& _ivar, const AstNodePtr& _lb, const AstNodePtr& _ub,
            const AstNodePtr& _step, const AstNodePtr& _stmts, bool decrementIvar)
{ 
  // jichi(9/11/2009): Add in support for SgFortranDo.
  if (IS_FORTRAN_LANGUAGE()) { // Generate fortran loop.
    AstNodePtrImpl ivar(_ivar), lb(_lb), ub(_ub), step(_step), stmts(_stmts);
    assert(lb != 0);

    // Create new loop.
    SgFortranDo *result = new SgFortranDo(GetFileInfo());
    result->set_endOfConstruct(result->get_file_info());

    // Set loop expressions.
    SgExpression* ivarexp = ToExpression(*impl, ivar.get_ptr());
    SgExpression* lbexp = ToExpression(*impl, lb.get_ptr());
    SgExpression* ubexp = ToExpression(*impl, ub.get_ptr());
    SgExpression* stepexp = ToExpression(*impl, step.get_ptr());

    SgExpression* initexp = isSgExpression(::CreateAssignment(*impl, ivarexp, lbexp));
    assert(initexp != 0);
    ivarexp->set_parent(initexp);
    ubexp->set_parent(initexp);

    result->set_initialization(initexp);
    initexp->set_parent(result);

    result->set_bound(ubexp);
    ubexp->set_parent(result);

    result->set_increment(stepexp);
    stepexp->set_parent(result);
    
    // set loop body
    SgStatement* stmtptr = ToStatement(stmts.get_ptr());
    assert(stmtptr != 0);
    SgBasicBlock *b = isSgBasicBlock(stmtptr);

    if (b == 0) {
      NEW_BLOCK1(b, stmtptr);
    } else
      assert( HasNullParent(b));

    result->set_body(b);
    b->set_parent(result);

    result->set_has_end_statement(true);
    result->set_parent(GetNullScope());
    return AstNodePtrImpl(result);

  } else {
    AstNodePtrImpl ivar(_ivar), lb(_lb), ub(_ub), step(_step), stmts(_stmts);
    SgForStatement *result = new SgForStatement(GetFileInfo());
    result->set_endOfConstruct(result->get_file_info());
    SgExpression* ivarexp = ToExpression(*impl, ivar.get_ptr());
    SgExpression* lbexp = (lb == 0)? 0 : ToExpression(*impl, lb.get_ptr());
    SgExpression* ubexp = ToExpression(*impl, ub.get_ptr());
    SgExpression* stepexp = ToExpression(*impl, step.get_ptr());
    SgNode* init = 0; 
    if (lbexp != 0) init=::CreateAssignment(*impl, ivarexp, lbexp);
    SgStatement *initstmt = isSgStatement(init);
    if (initstmt == 0 && init != 0) {
      SgExpression *initexp = ToExpression(*impl, init);
      NEW_EXPR_STMT(initstmt, initexp);
    }
    if (initstmt != 0) {
      result->append_init_stmt( initstmt );
      initstmt->set_parent(result->get_for_init_stmt());
    }
    SgExpression* ivarexp1 = isSgExpression(AstNodePtrImpl(CopyAstTree(AstNodePtrImpl(ivarexp))).get_ptr());
    if (decrementIvar) {
      assert( HasNullParent(ubexp));
  
      SgExpression* testExp = new SgGreaterOrEqualOp( GetFileInfo(), ivarexp1, ubexp);
      ivarexp1->set_parent(testExp); ubexp->set_parent(testExp);
      SgExprStatement* NEW_EXPR_STMT(test,testExp);
      result->set_test(test); test->set_parent(result);

    } else { 
      assert( HasNullParent(ubexp));
  
      SgExpression* NEW_BIN_OP(testExp,SgLessOrEqualOp, ivarexp1, ubexp);
      SgExprStatement* NEW_EXPR_STMT(test,testExp);
      result->set_test(test); test->set_parent(result);
    }
  
    SgExpression* ivarexp2 = isSgExpression(AstNodePtrImpl(CopyAstTree(AstNodePtrImpl(ivarexp))).get_ptr());
    assert( HasNullParent(stepexp));
    SgPlusAssignOp *NEW_BIN_OP(incr,SgPlusAssignOp, ivarexp2, stepexp);
    result->set_increment(incr); incr->set_parent(result);
    SgStatement* stmtptr = ToStatement(stmts.get_ptr());
    assert(stmtptr != 0);
    SgBasicBlock *b = isSgBasicBlock(stmtptr);

    if (b == 0) {
      NEW_BLOCK1(b, stmtptr);
    } 
    result->set_loop_body(b);
    b->set_parent(result);
    result->set_parent(GetNullScope());
    return AstNodePtrImpl(result);
  }
}
    
    
AstNodePtr AstInterface::CreateConstInt( int val)  
{ 
  SgIntVal* res = new SgIntVal(GetFileInfo(),val);
  res->set_endOfConstruct(res->get_file_info());
  return AstNodePtrImpl(res);
}

AstNodePtr AstInterface::
CreateConstant( const std::string& valtype, const std::string& val) 
{
  if (valtype == "int") {
    int intval = atoi(val.c_str());
    return CreateConstInt(intval);
  } 
  else if (valtype == "bool") {
      int i = atoi(val.c_str());
      if (val == "true" || i != 0)
         return AstNodePtrImpl(new SgBoolValExp(GetFileInfo(), -1));
      else
         return AstNodePtrImpl(new SgBoolValExp(GetFileInfo(), 0));
  }
  else if (valtype == "string") {
         char *r = new char[val.size() + 1];
         strcpy( r, val.c_str());
         SgStringVal *tmp = new SgStringVal(GetFileInfo(), r);
         assert(tmp != NULL);
         return AstNodePtrImpl(tmp);
  }
  else if (valtype == "char") {
         return AstNodePtrImpl(new SgCharVal(GetFileInfo(), val[0]));
  }
  else if (valtype == "float") {
         std::istringstream in(val);
         float num = 0;
         in >> num;
         return AstNodePtrImpl(new SgFloatVal(GetFileInfo(), num));
  }
  else if (valtype == "double") {
         std::istringstream in(val);
         double num = 0;
         in >> num;
         return AstNodePtrImpl(new SgDoubleVal(GetFileInfo(), num));
  }
  else if (valtype == "function") {
      SgFunctionSymbol *fsym = impl->GetFunc(val);
      if ( fsym == 0) {
         return AST_NULL;
      }
      SgFunctionRefExp* NEW_FUNCTION_REF(fr,fsym);
      return AstNodePtrImpl(fr);
  }
  else {
       std::cerr << "Error: non-recognized value type for creating constant AST: " << valtype << std::endl;
        ROSE_ABORT();
        abort();
  }
}

SgFunctionSymbol* 
CreateMinMaxFunction(AstInterfaceImpl* impl, const std::string& name, int numOfPars, bool isMin)
{
    SgType* typeint = AstInterfaceImpl::GetTypeInt();
    std::list<SgInitializedName*> pars; 
    for (int i = 0; i < numOfPars; ++i) { 
      std::string parname = "a";
      parname.push_back(i + '0');
      SgName curname(parname.c_str());
      SgInitializedName* curVar = new SgInitializedName(GetFileInfo(), curname, typeint, 0, 0, 0, 0); 
      pars.push_back(curVar);
    }
    SgFunctionSymbol* funcSymbol = impl->NewFunc(name, typeint, pars); 
    if (IS_FORTRAN_LANGUAGE()) return funcSymbol;

    SgFunctionDeclaration* funcDecl = funcSymbol->get_declaration();
    funcDecl->set_requiresNameQualificationOnReturnType(false);
    funcDecl->set_definingDeclaration(funcDecl);
    SgBasicBlock* NEW_BLOCK(funcBody);
    SgFunctionDefinition* funcDefn = new SgFunctionDefinition(GetFileInfo(), funcBody);
    funcBody->set_parent(funcDefn); funcDefn->set_parent(funcDecl);

    std::list<SgVariableSymbol*> parSymbols; 
    for (std::list<SgInitializedName*>::const_iterator iterPars=pars.begin(); iterPars != pars.end(); ++iterPars) { 
      SgInitializedName* curPar = *iterPars; 
      SgVariableSymbol* NEW_SYMBOL(curSymbol, SgVariableSymbol, funcDefn, curPar);
      parSymbols.push_back(curSymbol);
    }
    funcDecl->set_definition(funcDefn);
    funcDecl->set_endOfConstruct(funcDecl->get_file_info());
    if (numOfPars == 2) {
      SgVarRefExp* v1 = new SgVarRefExp( GetFileInfo(), parSymbols.front());
      SgVarRefExp* v2 = new SgVarRefExp( GetFileInfo(), parSymbols.back());
      SgExpression * cond = 0;
      if (isMin) cond = new SgLessThanOp(GetFileInfo(), v1,v2) ;
      else cond = new SgGreaterThanOp(GetFileInfo(), v1,v2); 
      v1->set_parent(cond); v2->set_parent(cond);
      v1 = new SgVarRefExp( GetFileInfo(), parSymbols.front());
      v2 = new SgVarRefExp( GetFileInfo(), parSymbols.back());
      SgExpression* returnExp = new SgConditionalExp(GetFileInfo(), cond, v1, v2, NULL);
      cond->set_parent(returnExp); v1->set_parent(returnExp); v2->set_parent(returnExp);
      SgStatement* returnStmt = new SgReturnStmt(GetFileInfo(), returnExp);
      funcBody->append_statement(returnStmt);
      returnStmt->set_parent(funcBody); 
    }
    else {
      SgName  resName("res");
      SgInitializedName* resVar = new SgInitializedName(GetFileInfo(), resName,typeint, 0, 0, 0, 0); 
      SgVariableSymbol * NEW_SYMBOL(resSymbol,SgVariableSymbol, funcBody,resVar);
      std::list<SgVariableSymbol*>::const_iterator iterParSymbols = parSymbols.begin();
      SgVarRefExp* parRef = new SgVarRefExp(GetFileInfo(), *iterParSymbols); 
      ++iterParSymbols; 
      SgAssignInitializer *NEW_VAR_INIT(resInit,resVar,parRef);
      SgVariableDeclaration *resDecl = new SgVariableDeclaration(GetFileInfo());
      resDecl->append_variable(resVar,resInit); 
      resVar->set_parent(resDecl);
      resDecl->set_endOfConstruct(resDecl->get_file_info());
      funcBody->append_statement(resDecl); 

      for (int i = 1; i < numOfPars; ++i) {
         SgVarRefExp* resRef = new SgVarRefExp(GetFileInfo(), resSymbol);
         parRef = new SgVarRefExp(GetFileInfo(), *iterParSymbols); 
         SgExpression * cond = 0;
         if (isMin) cond = new SgLessThanOp(GetFileInfo(), parRef,resRef); 
         else cond = new SgGreaterThanOp(GetFileInfo(), parRef,resRef); 
         resRef->set_parent(cond); parRef->set_parent(cond);
         SgStatement* NEW_EXPR_STMT(condStmt,cond);
         resRef = new SgVarRefExp(GetFileInfo(), resSymbol);
         parRef = new SgVarRefExp(GetFileInfo(), *iterParSymbols); 
         ++iterParSymbols; 
         SgExpression* assignExp = new SgAssignOp(GetFileInfo(), resRef, parRef);
         resRef->set_parent(assignExp); parRef->set_parent(assignExp);
         SgStatement* NEW_EXPR_STMT(assignStmt,assignExp);
         SgBasicBlock* NEW_BLOCK1(assignBlock,assignStmt);
         SgIfStmt* NEW_IF(ifStmt, condStmt, assignBlock);
         funcBody->append_statement(ifStmt);
      }
      SgVarRefExp* resRef = new SgVarRefExp(GetFileInfo(), resSymbol);
      SgStatement* returnStmt = new SgReturnStmt(GetFileInfo(), resRef);
      resRef->set_parent(returnStmt);
      funcBody->append_statement(returnStmt);
    }
    return funcSymbol;
}

SgNode* AstInterfaceImpl :: 
CreateFunction( std::string name, int numOfPars)
{
  bool isMin = (name == "min");
  bool isMax = (name == "max");
  if ((isMin || isMax) && !IS_FORTRAN_LANGUAGE())
    name.push_back((char)numOfPars+'0');
  SgFunctionSymbol* funcSymbol = GetFunc( name);
  if (funcSymbol == 0) {
     if (isMin || isMax) {
       funcSymbol = CreateMinMaxFunction(this,name, numOfPars, isMin);
     }
     else  {
       std::cerr << "Unknown function: " << name << "\n";
       ROSE_ABORT();
     }
  }
  SgFunctionRefExp* NEW_FUNCTION_REF(result,funcSymbol);
  return result;
}

AstNodePtr AstInterface::
CreateUnaryOP( OperatorEnum op, const AstNodePtr& _a0)
{
  AstNodePtrImpl a0(_a0);
  assert( HasNullParent(a0.get_ptr()));
  SgExpression *e = ToExpression(*impl, a0.get_ptr());
  SgNode* result = 0;
  switch (op) {
   case UOP_ADDR: 
     result = new SgAddressOfOp( GetFileInfo(), e, e->get_type());
     break;
   case UOP_MINUS: 
     result = new SgMinusOp( GetFileInfo(), e, e->get_type());
     break;
   case UOP_CAST_C:
   case UOP_CAST_STATIC:
   case UOP_CAST_REINTERP:
     result = new SgCastExp( GetFileInfo(), e, e->get_type());
     break;
   case UOP_DEREF:
     result = new SgPointerDerefExp( GetFileInfo(), e, e->get_type());
     break;
   default:
     std::cerr << "unexpected uop:" << op << "\n";
     ROSE_ABORT();
  }
  e->set_parent(result);
  return AstNodePtrImpl(result);
}

AstNodePtr AstInterface:: 
CreateBinaryOP( OperatorEnum op, const AstNodePtr& _a0, const AstNodePtr& _a1) 
{
  SgNode* a0 = AstNodePtrImpl(_a0).get_ptr(); 
  SgNode* a1 = AstNodePtrImpl(_a1).get_ptr(); 
  //assert( HasNullParent(a1) && HasNullParent(a0));
  SgExpression *e0 = ToExpression(*impl, a0);
  SgExpression *e1 = ToExpression(*impl, a1);
  assert( e0 != 0 && e1 != 0);
  SgBinaryOp* n = 0;
  switch (op) {
  case BOP_DOT_ACCESS: 
      n = new SgDotExp(GetFileInfo(), e0, e1, e1->get_type()); 
      break;
  case BOP_ARROW_ACCESS: 
      n = new SgArrowExp(GetFileInfo(), e0, e1, e1->get_type()); break;
  case BOP_DIVIDE:
      n = new SgDivideOp( GetFileInfo(), e0,e1); break;
  case BOP_TIMES: 
      n = new SgMultiplyOp(GetFileInfo(), e0, e1); break;
  case BOP_PLUS: 
      n = new SgAddOp(GetFileInfo(), e0,e1); break;
  case BOP_MINUS: 
      n = new SgSubtractOp(GetFileInfo(), e0,e1); break;
  case BOP_EQ:
      n =  new SgEqualityOp( GetFileInfo(), e0,e1); break;
  case BOP_NE:
      n = new SgNotEqualOp( GetFileInfo(), e0,e1); break;
  case BOP_LT:
      n = new SgLessThanOp( GetFileInfo(), e0,e1); break;
  case BOP_GT:
      n = new SgGreaterThanOp(GetFileInfo(), e0,e1); break;
  case BOP_LE:
      n = new SgLessOrEqualOp(GetFileInfo(), e0,e1); break;
  case BOP_GE:
      n = new SgGreaterOrEqualOp(GetFileInfo(), e0,e1); break;
  case BOP_AND:
      n =  new SgAndOp(GetFileInfo(), e0,e1); break;
  case BOP_OR:
      n =  new SgOrOp(GetFileInfo(), e0,e1); break;
  case BOP_BIT_RSHIFT:
      n = new SgRshiftOp(GetFileInfo(), e0,e1); break;
  case BOP_BIT_LSHIFT:
      n = new SgLshiftOp(GetFileInfo(), e0,e1); break;
  default:
      std::cerr << "Error: non-recognized binary operator: \n";
      ROSE_ABORT();
  }
  e0->set_parent(n); e1->set_parent(n);
  n->set_endOfConstruct(n->get_file_info());
  return AstNodePtrImpl(n);
}

AstNodePtr AstInterface::
CreateArrayAccess( const AstNodePtr& arr, const AstNodePtr& index)
{
  SgExpression* r = isSgExpression(AstNodePtrImpl(arr).get_ptr());
  SgExpression *e2 = isSgExpression((SgNode*)index.get_ptr());
  assert(r);
  if (IS_FORTRAN_LANGUAGE()) {
    AstNodePtr arr_ref;
    AstList arr_index;
    arr_index.push_back(e2); //QY: prepend the new dimension now

    // If arr is array access, append offsets to its first dimension.
    if (IsArrayAccess(arr, &arr_ref, &arr_index)) {
      return CreateArrayAccess(arr_ref, AstNodeList2ExpressionList(arr_index.begin(), arr_index.end()));
    }

    SgExpression* aref = new SgPntrArrRefExp(GetFileInfo(), r, e2);
    assert(aref);
    aref->set_endOfConstruct(aref->get_file_info());

    r->set_parent(aref);
    e2->set_parent(aref);
    return AstNodePtrImpl(aref);

  } else {
       assert(e2 != 0);
       SgExpression* r1 = new SgPntrArrRefExp(GetFileInfo(), r, e2);
       r1->set_endOfConstruct(r1->get_file_info());
       r->set_parent(r1); e2->set_parent(r1);
       r = r1;
    return AstNodePtrImpl(r);
  }
}

AstNodePtr GetOverloadOperatorOpd1( const AstNodePtr& _exp)
{
  AstNodePtrImpl exp(_exp);
  SgFunctionCallExp *fs = isSgFunctionCallExp(exp.get_ptr());
  assert(fs != 0);
  SgExpression *func = fs->get_function();
  if (func->variantT() == V_SgDotExp) {
    return  AstNodePtrImpl( isSgDotExp(func)->get_lhs_operand() );
  }
  SgExpressionPtrList &l = fs->get_args()->get_expressions();
  return AstNodePtrImpl(l.front());
}

AstNodePtr GetOverloadOperatorOpd2( const AstNodePtr& _exp)
{
  AstNodePtrImpl exp(_exp);
  SgFunctionCallExp *fs = isSgFunctionCallExp(exp.get_ptr());
  assert(fs != 0);
  SgExpressionPtrList &l = fs->get_args()->get_expressions();
  return AstNodePtrImpl(l.back());
}

AstNodePtr AstInterface::
CreateFunctionCall( const AstNodePtr& func, AstList::const_iterator b, AstList::const_iterator e)
{
  return AstNodePtrImpl(impl->CreateFunctionCall(AstNodePtrImpl(func).get_ptr(), b, e));
}

AstNodePtr AstInterface::
CreateFunctionCall( const std::string& fname, AstList::const_iterator b, AstList::const_iterator e)
 {
  unsigned num = 0;
  for (AstList::const_iterator p = b; p != e; ++p) ++num;
  SgNode* f = impl->CreateFunction(fname, num);
  return AstNodePtrImpl(impl->CreateFunctionCall(f, b, e));
}

SgNode* AstInterfaceImpl::
CreateFunctionCall( SgNode* func, AstNodeList::const_iterator b, AstNodeList::const_iterator e)
{
  assert( HasNullParent(func));        // which implies func is not in the global AST now.
  SgExpression *fr = isSgExpression(func);

  AstNodeList::const_iterator p = b;
  if (fr->variantT() == V_SgMemberFunctionRefExp) {
    SgExpression* obj = isSgExpression(ASTNODE2PTR(*p));
    assert(obj != 0 && HasNullParent(obj));
    ++p;
    SgExpression* fr1 = 0;
    if (obj->get_type()->variantT() == V_SgPointerType) {
      NEW_BIN_OP(fr1, SgArrowExp, obj, fr);
    } else {
      NEW_BIN_OP(fr1, SgDotExp, obj, fr);
    }
    obj->set_parent(fr1); fr->set_parent(fr1);
    fr = fr1;
  }
  SgExprListExp *argexp=AstNodeList2ExpressionList(b,e);
  SgFunctionCallExp *NEW_FUNCTION_CALL(result,fr, argexp);
  return result;
}


AstNodePtr
AstInterface::CreateReadStatement(const AstNodeList& l) const
{
  SgExprListExp* explist = AstNodeList2ExpressionList(l.begin(), l.end());
  SgReadStatement* ret = new SgReadStatement(GetFileInfo());
  assert(ret);
  ret->set_endOfConstruct(ret->get_file_info());

  ret->set_io_stmt_list(explist);
  explist->set_parent(ret);

  SgExpression* f = new SgAsteriskShapeExp(GetFileInfo());
  f->set_endOfConstruct(f->get_file_info());
  ret->set_format(f);
  f->set_parent(ret);

  return AstNodePtrImpl(ret);
}

AstNodePtr
AstInterface::CreateNullStatement() const
{
  SgStatement* s = new SgNullStatement(GetFileInfo());
  s->set_endOfConstruct(s->get_file_info());
  return AstNodePtrImpl(s);
}

AstNodePtr
AstInterface::CreateWriteStatement(const AstNodeList& __l) const
{
  assert(!__l.empty());

  SgExprListExp* NEW_EXPR_LIST(explist);
  for (AstNodeList::const_iterator p = __l.begin(); p != __l.end(); ++p) {
    SgExpression* e = isSgExpression(ASTNODE2PTR(*p));
    assert(e);
    explist->append_expression(e);
    e->set_parent(explist);
  }

  SgWriteStatement* ret = new SgWriteStatement(GetFileInfo());
  assert(ret);
  ret->set_endOfConstruct(ret->get_file_info());

  ret->set_io_stmt_list(explist);
  explist->set_parent(ret);

  SgExpression* f = new SgAsteriskShapeExp(GetFileInfo());
  f->set_endOfConstruct(f->get_file_info());
  ret->set_format(f);
  f->set_parent(ret);

  return AstNodePtrImpl(ret);
}

AstNodePtr
AstInterface::CreatePrintStatement(const AstNodeList& l) const
{
  SgExprListExp* explist = AstNodeList2ExpressionList(l.begin(),l.end());
  SgPrintStatement* ret = new SgPrintStatement(GetFileInfo());
  assert(ret);
  ret->set_endOfConstruct(ret->get_file_info());

  ret->set_io_stmt_list(explist);
  explist->set_parent(ret);

  SgExpression* f = new SgAsteriskShapeExp(GetFileInfo());
  f->set_endOfConstruct(f->get_file_info());
  ret->set_format(f);
  f->set_parent(ret);

  return AstNodePtrImpl(ret);
}

AstNodePtr
AstInterface::CreateIf(const AstNodePtr& __cond, const AstNodePtr& __istmt,
                       const AstNodePtr& __estmt) const
{ 
  AstNodePtrImpl cond(__cond), istmt(__istmt);
  assert( HasNullParent(cond.get_ptr())
          && HasNullParent(istmt.get_ptr()) );

  SgStatement* c = ToStatement(cond.get_ptr());
  SgBasicBlock *ib = isSgBasicBlock(istmt.get_ptr());    // if-block
  if (!ib) {
     SgStatement* p = ToStatement(istmt.get_ptr());
     NEW_BLOCK1(ib, p);
  }

  SgIfStmt* ret;
  if (__estmt == AST_NULL) {    // no else-statement
    NEW_IF(ret, c, ib);

  } else {
    AstNodePtrImpl estmt(__estmt);
    assert(HasNullParent(estmt.get_ptr()));

    SgBasicBlock *eb = isSgBasicBlock(estmt.get_ptr());    // else-block
    if (!eb) {
       SgStatement* p = ToStatement(estmt.get_ptr());
       NEW_BLOCK1(eb, p);
    }

    NEW_IF_ELSE(ret, c, ib, eb);
  }

  return AstNodePtrImpl(ret);
}

AstNodePtr AstInterface::CreateBlock( const AstNodePtr& _orig) 
 { 
    AstNodePtrImpl orig(_orig);
    SgBasicBlock *NEW_BLOCK(r);
    r->set_parent(GetNullScope());
    if (orig != 0) {
       SgBasicBlock *r1 = isSgBasicBlock(orig.get_ptr());
       if (r1 != 0) {
          r->getAttachedPreprocessingInfo() = r1->getAttachedPreprocessingInfo();
       }
    }
    return AstNodePtrImpl(r);
 }

void BlockPrependAppendStmt(AstInterfaceImpl* /*impl*/, AstNodePtr& _b, const AstNodePtr& _s, bool isAppend, bool flatten=true)
    { 
      AstNodePtrImpl b(_b), s(_s);
      SgStatement* stmt = ToStatement(s.get_ptr());
      assert(stmt != 0);

      SgBasicBlock *sb = isSgBasicBlock(stmt);
      SgBasicBlock *basicBlock = isSgBasicBlock(b.get_ptr());
      assert(basicBlock != 0);

      stmt->set_parent(basicBlock);

      if (sb == 0)  {
         if (!isAppend)
            basicBlock->prepend_statement( stmt);
         else
            basicBlock->append_statement( stmt);
      }
      else {
         SgStatementPtrList l = sb->get_statements();
         bool hasdecl = false;
         if (!flatten) {
           for (SgStatementPtrList::iterator p = l.begin(); p != l.end(); ++p) {
            if (isSgDeclarationStatement(*p) != 0) {
                hasdecl = true;
                break;
            } 
           } 
         }
         if (hasdecl) {
            if (!isAppend)
               basicBlock->prepend_statement(sb);
            else
               basicBlock->append_statement(sb);
         }
         else if (isAppend) {
            for (SgStatementPtrList::iterator p = l.begin(); p != l.end(); ++p) {
              SgStatement * cur = *p;
              basicBlock->append_statement(cur);
              cur->set_parent(basicBlock);
         } } 
         else {
            for (SgStatementPtrList::reverse_iterator p = l.rbegin(); p != l.rend(); ++p) {
              SgStatement * cur = *p;
              basicBlock->prepend_statement(cur);
              cur->set_parent(basicBlock);
          }
         } 
      }
    }

void AstInterface::BlockAppendStmt( AstNodePtr& _b, const AstNodePtr& _s, bool flatten)
{ BlockPrependAppendStmt(impl,_b, _s, true, flatten); }
 
void AstInterface::
BlockPrependStmt( AstNodePtr& _b, const AstNodePtr& _s)
{ BlockPrependAppendStmt(impl,_b, _s, false); }

void AstInterface::
InsertStmt(AstNodePtr const & _orig, AstNodePtr const &_n, bool insertbefore,
           bool extractfromBlock)
{
   AstNodePtrImpl n(_n), orig(_orig);
   assert( HasNullParent(n.get_ptr()));
   SgStatement *s = isSgStatement(orig.get_ptr()), *ns = ToStatement(n.get_ptr());
   assert(s != 0);
   SgStatement *p = isSgStatement(s->get_parent());
   assert(p != 0);
   SgBasicBlock *nb = isSgBasicBlock(ns);
   if (extractfromBlock && nb != 0) {
      p->insert_statement_from_basicBlock(s, nb, insertbefore);
      SgStatementPtrList l = nb->get_statements();
      for (SgStatementPtrList::iterator pn = l.begin(); pn != l.end(); ++pn) {
              SgStatement * cur = *pn;
              cur->set_parent(p);
      } 
   }
   else {
      p->insert_statement(s, ns, insertbefore);
      ns->set_parent(p);
   }
}

void AstInterface::
InsertAnnot(AstNodePtr const& _n, const std::string& annot, bool insertbefore)
{
  SgNode* n = AstNodePtrImpl(_n).get_ptr(); 
  SgLocatedNode* loc = isSgLocatedNode(n);
  assert (loc != 0);
   {
     Sg_File_Info* nf = loc->get_file_info();

  // DQ (7/19/2008): Modified interface to PreprocessingInfo
  // Note that this function could directly call SageInterface::attachComment(SgLocatedNode*,std::string);
     PreprocessingInfo * info = 
       new PreprocessingInfo( PreprocessingInfo::C_StyleComment, 
                              annot, nf->get_filename(),
                              nf->get_line(), nf->get_col(), 1, 
                              (insertbefore)? (PreprocessingInfo::before) 
                                            :  (PreprocessingInfo::after));
     loc->addToAttachedPreprocessingInfo( info);
   }

}

bool AstInterface::RemoveStmt( const AstNodePtr& _n)
{
   AstNodePtrImpl n(_n);
   SgStatement* s = isSgStatement(n.get_ptr());
   assert (s != 0); 
   SgStatement* p = isSgStatement(n->get_parent());
   assert( p != 0);
   p->remove_statement(s);
   //s->set_parent(GetNullScope()); /*QY: not reseting parent due to dangling pointers from symbols etc. */
   return true;
}

bool AstInterfaceImpl::
ReplaceAst( SgNode* orig, SgNode* n)
{ 
    SgNode *p = orig->get_parent();
    if (p == 0) return false;
    n->set_parent(p);

    SgStatement *stmtOrig = isSgStatement(orig);
    SgStatement* stmtParent = isSgStatement(p);
    if (stmtOrig != 0) {
         SgStatement *stmtNew  = isSgStatement(n);
         assert(stmtParent != 0 && stmtNew != 0);
         stmtParent->replace_statement(stmtOrig, stmtNew);
         stmtNew->set_parent(stmtParent);
     }
     else {
         SgExpression *expOrig = isSgExpression(orig);
         SgExpression *expNew = isSgExpression(n);
         assert(expOrig != 0 && expNew != 0);
         if (stmtParent != 0)  {
            stmtParent->replace_expression(expOrig, expNew);
            expNew->set_parent(stmtParent);
         }
         else {
            SgExpression *expParent = isSgExpression(p);
            if (expParent != 0) {
               expParent->replace_expression(expOrig, expNew);
               expNew->set_parent(expParent);
            }
            else {
               SgInitializedName *nameParent = isSgInitializedName(p);
               assert(nameParent != 0);
               SgAssignInitializer *NEW_VAR_INIT(init, nameParent, expNew);
            }
         }
    }
    //orig->set_parent(GetNullScope()); /*QY: not reseting parent due to dangling pointers from symbols etc. */
    return true;
 }
bool AstInterface::ReplaceAst( const AstNodePtr& _orig, const AstNodePtr& _n)
    { 
      SgNode*  orig = AstNodePtrImpl(_orig).get_ptr();
      SgNode*  n = AstNodePtrImpl(_n).get_ptr();
      return impl->ReplaceAst(orig, n);
    }

//typedef bool BoolAttribute;
class BoolAttribute
{
  bool val;
 public:
  BoolAttribute( bool v = true) : val(v) {}
  operator bool() const { return val; } 
};
class SageProcessAstNode : public AstTopDownBottomUpProcessing<BoolAttribute,BoolAttribute> 
{
   AstInterface *fa;
   AstInterface::TraversalOrderType t;
   ProcessAstNode<AstNodePtr>& op;
   BoolAttribute evaluateInheritedAttribute(SgNode* astNode, 
                                            BoolAttribute inheritedValue)
     { if (t == AstInterface::PostOrder)
          return inheritedValue;
       return inheritedValue? op.Traverse( *fa, AstNodePtrImpl(astNode), AstInterface::PreVisit) : false; 
     }
   BoolAttribute evaluateSynthesizedAttribute(SgNode* astNode, 
                                              BoolAttribute inheritedValue, 
                                                 SynthesizedAttributesList l)
    {   if (t == AstInterface::PreOrder)
           return inheritedValue; 
        if (! inheritedValue)
            return false;
        for (size_t i = 0; i < l.size(); ++i) 
           if (!l[i])
              return false;
        return op.Traverse( *fa, AstNodePtrImpl(astNode), AstInterface::PostVisit); 
    }
  public:
   SageProcessAstNode( ProcessAstNode<AstNodePtr>& _op) : op(_op) {}
   bool Traverse( AstInterface *_fa, SgNode* node, AstInterface::TraversalOrderType _t)
       { fa = _fa;
         t = _t; 
         return AstTopDownBottomUpProcessing<BoolAttribute,BoolAttribute>::traverse(node, true);
       }
};

template <class Transform>
class PerformPreTransformationTraversal 
 : public AstTopDownBottomUpProcessing<_DummyAttribute, AstNodePtrImpl>
{
    SgNode* head;
    AstNodePtrImpl result, orig;
    bool succ;
    Transform& op;
    AstInterface& fa;
    _DummyAttribute evaluateInheritedAttribute(SgNode* astNode, _DummyAttribute)
     {
        if (!succ) {
          succ = op(fa, AstNodePtrImpl(astNode), result);
          if (succ) {
               assert(result != 0);
               orig = astNode;
          }
        }
       return _DummyAttribute();
     }
   AstNodePtrImpl evaluateSynthesizedAttribute(SgNode* astNode, _DummyAttribute, SynthesizedAttributesList)
      {
          if (orig == astNode) {
              orig = 0;
              succ = false;
              if (result != astNode && (astNode != head || astNode->get_parent() != 0)) {
                  fa.ReplaceAst(AstNodePtrImpl(astNode), result);
              }
              return result;
          }
          else 
            return astNode;
      }

  public:
    PerformPreTransformationTraversal( AstInterface& _fa, Transform& _op)
       : result(0), orig(0), succ(false), op(_op), fa(_fa) {}
    AstNodePtrImpl operator() ( SgNode* n)
          {
             succ = false;
             head = n;
             AstNodePtrImpl r = 
                AstTopDownBottomUpProcessing<_DummyAttribute, AstNodePtrImpl>::
                           traverse(n, _DummyAttribute());
             return r;
          }
};

template <class Transform>
class PerformPostTransformationTraversal : public AstBottomUpProcessing<AstNodePtrImpl>
{
  SgNode *head;
    AstInterface& fa;
    Transform& op;
  AstNodePtrImpl evaluateSynthesizedAttribute(SgNode* astNode, SynthesizedAttributesList)
      {
          AstNodePtrImpl r  = astNode;
          if (op(fa, AstNodePtrImpl(astNode), r) && r != astNode) {
              assert(r != 0);
              if (r != astNode && (astNode != head || astNode->get_parent() != 0))
                       fa.ReplaceAst(AstNodePtrImpl(astNode), r);
              return r;
          }
          else 
            return astNode;
      }

  public:
    PerformPostTransformationTraversal( AstInterface& _fa, Transform& _op)
       : fa(_fa), op(_op) { }
    AstNodePtrImpl operator() ( SgNode* n)
      {
         head = n;
         AstNodePtrImpl r = 
             AstBottomUpProcessing<AstNodePtrImpl>::traverse(n);
         return r;
      }
};

AstNodePtr TransformAstTraverse( AstInterface& fa, const AstNodePtr& r, 
          bool (*op)( AstInterface&, const AstNodePtr&, AstNodePtr&),
                                AstInterface::TraversalVisitType t )
{
  if (t == AstInterface::PreVisit) {
     PerformPreTransformationTraversal<bool (*)(AstInterface&, const AstNodePtr&, 
                                                AstNodePtr& ) > traverse(fa, op);
     return traverse(AstNodePtrImpl(r).get_ptr());
  }
  else {
     PerformPostTransformationTraversal<bool (*)(  AstInterface&, const AstNodePtr&, 
                                                    AstNodePtr&) > traverse(fa, op);
     return traverse(AstNodePtrImpl(r).get_ptr());
  }
}

AstNodePtr TransformAstTraverse( AstInterface& fa, const AstNodePtr& r, 
                              TransformAstTree& op, 
                              AstInterface::TraversalVisitType t) 
{
  if (t == AstInterface::PreVisit) {
     PerformPreTransformationTraversal<TransformAstTree> traverse(fa, op);
     AstNodePtr result = traverse(AstNodePtrImpl(r).get_ptr());
     return result;
  }
  else {
     PerformPostTransformationTraversal<TransformAstTree> traverse(fa, op);
     AstNodePtr result = traverse(AstNodePtrImpl(r).get_ptr());
     return result;
  }
}

bool ReadAstTraverse(AstInterface& fa, const AstNodePtr& _root,
                        ProcessAstNode<AstNodePtr>& op,
                        AstInterface::TraversalOrderType t)
{
   AstNodePtrImpl root(_root);
   return SageProcessAstNode(op).Traverse(&fa, root.get_ptr(), t);
}


template class PerformPreTransformationTraversal<bool (*)(AstInterface &, AstNodePtr const &, AstNodePtr &)>;
template class PerformPostTransformationTraversal<bool (*)(AstInterface &, AstNodePtr const &, AstNodePtr &)>;
template class PerformPreTransformationTraversal<TransformAstTree>;
template class PerformPostTransformationTraversal<TransformAstTree>;
template class std::list<SgExpression *, std::allocator<SgExpression *> >;
template class std::vector<AstNodePtr, std::allocator<AstNodePtr> >;
template class AstTopDownBottomUpProcessing<_DummyAttribute, AstNodePtr>;
template class AstBottomUpProcessing<AstNodePtr>;
template class SgTreeTraversal<_DummyAttribute, AstNodePtr>;

class CheckSymbolTable : public AstTopDownProcessing<AstNodePtrImpl>
{
 public:
  void operator() ( SgNode* n)
  { AstTopDownProcessing<AstNodePtrImpl>::traverse( n, n->get_parent() ); }

  static void fix_vardecl( SgVariableDeclaration* d )  
  {
   SgScopeStatement* scope = d->get_scope();
   if (DebugSymbol()) {
        std::cerr << "Adding symbol info for variable declaration: ";
        std::cerr << d->unparseToString() << " : into scope " << scope << std::endl;
        
   }
   SgInitializedNamePtrList & l = d->get_variables();
   for (SgInitializedNamePtrList::iterator p = l.begin(); p != l.end(); ++p) {
        SgInitializedName* n = *p;
        SgName name = n->get_name() ;
        SgVariableSymbol *sb = scope->lookup_var_symbol(name);
        if (sb == 0) {
           if (DebugSymbol())
              std::cerr << "Adding symbol for variable: " << name.str() << std::endl;
           NEW_SYMBOL(sb, SgVariableSymbol, scope, n);
        }
        else if (n != sb->get_declaration()) {
           ROSE_ABORT();
        }
    }
  }

  static void fix_classdecl( SgClassDeclaration* d1 ) 
  {
   SgScopeStatement* scope = d1->get_scope();
   if (DebugSymbol()) {
     std::cerr << "Adding symbol info for class declaration: ";
     std::cerr << d1->unparseToString() << std::endl;
   }
   SgName name = d1->get_name();
   SgClassSymbol *sb = scope->lookup_class_symbol(name);
   if (sb == 0) {
        if (DebugSymbol())
           std::cerr << "Adding symbol for class: " << name.str() << std::endl;
        NEW_SYMBOL(sb, SgClassSymbol, scope, d1);
   }
  }

  private:
  AstNodePtrImpl evaluateInheritedAttribute(SgNode* ast, AstNodePtrImpl v)
  {
     if (v.get_ptr() != 0 && v != ast->get_parent()) {
        if (ast->get_parent() == NULL)
           ast->set_parent(v.get_ptr());
        std::cerr << "Incorrect parent for AST: " << AstInterface::AstToString(AstNodePtrImpl(ast)) << "\n";
        std::cerr << "It has parent : " << ((ast->get_parent() == v.get_ptr())? "NULL" : AstInterface::AstToString(AstNodePtrImpl(ast->get_parent()))) << "\n";
        std::cerr << "It should have parent: "  << AstInterface::AstToString(v) << "\n";
        ROSE_ABORT();
     }
     switch (ast->variantT()) {
     case V_SgVariableDeclaration:
        fix_vardecl(isSgVariableDeclaration(ast));
        break;
     case V_SgClassDeclaration:
        fix_classdecl(isSgClassDeclaration(ast));
        break;
     case V_SgVarRefExp:
        {
         SgVarRefExp *var = isSgVarRefExp(ast);
         SgScopeStatement *scope = isSgScopeStatement(AstInterfaceImpl::GetScope(ast));
         assert(scope != 0);
         std::string name = var->get_symbol()->get_name().str();
         SgVariableSymbol *r =  isSgVariableSymbol(AstInterfaceImpl::LookupVar(name, scope));
         if (r == 0) {
             std::cerr << "failed to find symbol for variable: " << name << " in scope " << scope << std::endl;
             //assert(false);
         }
         else
            var->set_symbol( r);
        }
        break;
     default: break;
     }
    return ast;
  }
};


void FixSgTree( SgNode *r)
{  
     assert(r!=0);
     //AstPostProcessing(r);

  if (r->get_parent() != 0 && isSgScopeStatement(r) != 0) {
     CheckSymbolTable symbolfix;
     symbolfix(r);
  }
}

ROSE_DLL_API void FixSgProject( SgProject &sageProject)
{
   int filenum = sageProject.numberOfFiles();
   for (int i = 0; i < filenum; ++i) {
     SgFile &sageFile = sageProject.get_file(i);
     FixSgTree(&sageFile);
   }
}

SgStatement* AstInterfaceImpl::GetScope( SgNode* loc)
{
     if (loc == 0 || loc->get_parent() == 0) return 0;

     if (loc->variantT() == V_SgThisExp) {
        SgMemberFunctionDeclaration* r = 0;
        while (loc != 0 && r == 0 && loc->get_parent() != 0) {
           loc = loc->get_parent();
           r = isSgMemberFunctionDeclaration(loc);
        }
        return (r == 0)? 0 : r->get_class_scope();
     }
     if (loc->get_parent() != 0 && loc->get_parent()->variantT() == V_SgLambdaCapture) {
        // Go up to the expression chain to the enclosing statement.
        while (loc != 0 && loc->get_parent() != 0 && isSgStatement(loc) == 0) {
           loc = loc->get_parent();
        }
        assert(loc != 0);
        return GetScope(loc);
     }
    {
     const SgInitializedName* initializedName = isSgInitializedName(loc);
     if (initializedName != NULL) {
        if (loc->get_parent() != 0 && loc->get_parent()->variantT() == V_SgFunctionParameterList) {
          return initializedName->get_scope();
        } 
        DebugScope([loc, initializedName](){ return "GetScope invoked: loc is " + AstInterface::AstToString(loc) + "; parent->parent is " + loc->get_parent()->get_parent()->class_name() + "; scope=" + AstInterface::AstToString(initializedName->get_scope()); });
        return GetScope(loc->get_parent());
     }
    }
     if (isSgSourceFile(loc)) {
         return 0;
     }
    {
     SgScopeStatement* stmt = isSgScopeStatement(loc->get_parent());
     while (stmt == 0 && loc->get_parent() != 0) { 
         stmt = isSgScopeStatement(loc->get_parent());
         loc = loc->get_parent();
     }
     return stmt;
    }
}

std::string AstInterface:: GetVariableSignature(const AstNodePtr& _variable) {
    if (_variable.get_type() == AstNodePtr::SpecialAstType::GLOBAL_SIGNATURE) {
       return _variable.get_signature();
    }
    SgNode* variable = _variable.get_ptr();
    if (variable == 0) return AstInterface::AstTypeToString(_variable);

    std::string res;
    SgType* variable_is_type = isSgType(variable);
    if (variable_is_type != 0) {
        std::string variable_name;
        AstInterface::GetTypeInfo(variable_is_type, &variable_name, 0, 0, true);
        return variable_name;
    }
    switch (variable->variantT()) {
     case V_SgVariableSymbol:
          return isSgVariableSymbol(variable)->get_name();
     case V_SgNamespaceDeclarationStatement:
          return isSgNamespaceDeclarationStatement(variable)->get_name().getString();
     case V_SgUsingDirectiveStatement:
          return "using_" + isSgUsingDirectiveStatement(variable)->get_namespaceDeclaration()->get_name().getString();
     case V_SgEnumDeclaration: {
          std::string name = isSgEnumDeclaration(variable)->get_name();
          return "enum_" + (name.empty()? std::string((*isSgEnumDeclaration(variable)->get_enumerators().begin())->get_name()) : name);
       }
     case V_SgTypedefDeclaration:
     case V_SgTemplateTypedefDeclaration:
          return "typedef_" + AstInterface::GetGlobalUniqueName(variable->get_parent(), isSgTypedefDeclaration(variable)->get_name().getString(), /*do not use file name*/true);
     case V_SgStaticAssertionDeclaration:
          return OperatorDeclaration::operator_signature(variable);
     case V_SgClassDefinition:
          return GetVariableSignature(isSgClassDefinition(variable)->get_declaration());
     case V_SgAssignInitializer:
          return GetVariableSignature(isSgAssignInitializer(variable)->get_operand());
     case V_SgConstructorInitializer:
          return GetVariableSignature(isSgConstructorInitializer(variable)->get_declaration());
     case V_SgClassDeclaration:
     case V_SgTemplateClassDeclaration:
          return "class_" + std::string(isSgClassDeclaration(variable)->get_name().str()); 
     default: break;
    }
    
    if (AstInterface::IsFunctionDefinition(variable)) {
        return OperatorDeclaration::operator_signature(variable);
    } 
    {
      std::string value, valtype;
      if (AstInterface::IsConstant(variable, &valtype, &value)) {
         if (valtype == "int") {
             return value;
         }
         return "CONSTANT";
      }
    }
    {
    AstNodePtr f;
    AstNodeList args;
    if (AstInterface::IsArrayAccess(variable, &f)) {
       res += "_deref_(" + GetVariableSignature(f.get_ptr()) + ")";
       DebugVariable([&res](){ return "Variable signature:" + res; });
       return res;
    } 
    if (AstInterface::IsFunctionCall(variable, &f, &args)) {
       res += GetVariableSignature(f.get_ptr()) + "(";
       bool is_first = true;
       for (auto x : args) {
         if (!is_first) { res += ","; }
         else { is_first = false; }
         res += AstInterface::GetVariableSignature(x);
       }
       res  +=  ")";
       DebugVariable([&res](){ return "Variable signature:" + res; });
       return res;
    }
    }
    {
     AstNodeType alloc_type; 
     if (IsMemoryAllocation(variable, &alloc_type)) {
        res = res + "new_" + GetGlobalUniqueName(variable, GetTypeName(alloc_type), /*do_not_use_file_name*/true);
        DebugVariable([&res](){ return "Variable signature:" + res; });
        return res;
     }
    }
    {
    std::string name;
    if (IsVarRef(variable, 0, &name, 0, 0, /*use_global_unique_name=*/true)) {
        res = res + name;
        DebugVariable([&res](){ return "Variable signature:" + res; });
       return res; 
      }
    }
    res += "_UNKNOWN_" + variable->class_name();
    DebugVariable([&res](){ return "Variable signature:" + res; });
    return res;
}

bool AstInterface::IsLocalRef(const AstNodePtr& ref, const AstNodePtr& scope, bool* has_ptr_deref) {
   if (ref.is_unknown()) { return false; }
   if (ref == AST_NULL) { return true; }
   if (ref.get_ptr() == 0) { return false; }

   std::string scope_name;
   if (! AstInterface::IsBlock(scope, &scope_name)) {
     //std::cerr << "TODO: Expecting a block but getting :" << scope->class_name() << "\n";
     //TODO(QY): need to make each scope a block for consistency. 
     //assert(false);
   }
   DebugScope([&ref,&scope](){ return "IsLocalRef invoked: var is " + AstInterface::AstToString(ref) + "; scope is " + AstToString(scope); });
   AstNodePtr cur_scope;
   if (!AstInterface::IsVarRef(ref, 0, 0, &cur_scope, 0, false, has_ptr_deref)) {
      return false;
   }  
   std::string cur_scope_name;
   while (cur_scope != AST_NULL && cur_scope->variantT() != V_SgGlobal) {
         if (!AstInterface::IsBlock(cur_scope, &cur_scope_name)) {
             //TODO(QY): will need to make this consistent. 
             std::cerr << "Expecting a block but getting :" << cur_scope->class_name() << "\n";
             //assert(false);
         }  
         if (cur_scope.get_ptr() == scope.get_ptr() || (scope_name != "" && cur_scope_name == scope_name)) {
             DebugScope([&ref](){ return "variable is local:" + AstToString(ref); });
             return true;
         }
         DebugScope([&cur_scope](){ return "IsLocalRef current scope:" + cur_scope->class_name(); });
         SgNode* n = AstInterfaceImpl::GetScope(cur_scope.get_ptr());
         if (n == 0) break;
         cur_scope = n;
   }   
   DebugScope([&ref](){ return "variable is not local:" + AstToString(ref); });
   return false;
}

SgVariableSymbol* AstInterfaceImpl::
LookupVar(const std:: string& name) { return LookupVar(name, scope); }

void AstInterface::SetFunctionNameMangling(std::function<std::string(const SgFunctionDeclaration*)> f) {
  function_name_mangling_ = f;
}

