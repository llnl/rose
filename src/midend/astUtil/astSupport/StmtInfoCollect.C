#include <sage3basic.h>

#include <iostream>
#include <sstream>

#include "StmtInfoCollect.h"
#include "AnalysisInterface.h"
#include "AstInterface_ROSE.h"
#include <assert.h>
#include "CommandOptions.h"
#include <ROSE_ABORT.h>
#include <ROSE_ASSERT.h>

DebugLog DebugLocalInfoCollect("-debuglocalinfocollect");

void StmtInfoCollect::
AppendFuncCallArguments( AstInterface& fa, const AstNodePtr& fc, AstNodePtr* callee)
{
  AstInterface::AstNodeList args, params;
  AstInterface::AstNodePtr p_callee;
  if (!fa.IsFunctionCall(fc, &p_callee, &args))
      ROSE_ABORT();
  if (callee != 0) {
    *callee = AstNodePtrImpl(p_callee).get_ptr();
  }
  std::string fname;
  if (AstInterface::IsFunctionDefinition(fa.GetFunctionDefinition(p_callee), &fname, &params)) {
    DebugLocalInfoCollect([&fname]() { return "static Function call to : " + fname; });
  }

  AstInterface::AstNodeList::const_iterator p2=params.begin();
  for (AstInterface::AstNodeList::const_iterator p1 = args.begin(); p1 != args.end(); ++p1) {
    AstNodePtr c = AstNodePtrImpl(*p1).get_ptr();
    if ( c == AstNodePtr())
        continue;
    DebugLocalInfoCollect([&c]() { return "Function Argument: " + AstInterface::AstToString(c); } );
    operator()(fa, c);
    // Correlate c with the function parameter it is passed to.
    // Do not increment p2 if it is already empty.
    if (p2 != params.end()) {
       AppendReadLoc(fa, c, (*p2));
       p2++;
    }
  }
}

void StmtInfoCollect::
AppendFuncCallWrite( AstInterface& fa, const AstNodePtr& fc)
{
  DebugLocalInfoCollect([&fc]() { return "Append Function Call write " + AstInterface::AstToString(fc); } );
  AstInterface::AstNodeList outargs;
  if (!fa.IsFunctionCall(fc, 0, 0, &outargs)) {
      DebugLocalInfoCollect([&fc]() { return "quiting b/c Not function call: " + AstInterface::AstToString(fc); } );
      // We can't figure out the arguments right now. Skip. It's OK b/c an unknown will also be returned.
      return;
  }
  for (const AstNodePtr& c : outargs) {
    DebugLocalInfoCollect([&c]() { return "Function write Argument?: " + AstInterface::AstToString(c); } );
    AppendModLoc( fa, c, AST_UNKNOWN);
  }
}

void StmtInfoCollect::
operator()( AstInterface& fa, const AstNodePtr& h)
{
   AstNodePtr curstmttmp = curstmt;
   ProcessAstTreeBase<AstInterface::AstNodePtr>::operator()(fa, h);
   curstmt = curstmttmp;
}

bool StmtInfoCollect ::
ProcessTree( AstInterface &fa, const AstInterface::AstNodePtr& s,
                       AstInterface::TraversalVisitType t)
{
 if (t == AstInterface::PreVisit) {
   AstInterface::AstNodePtr lhs, rhs, body;
   AstInterface::AstNodeList vars, args, designators;
   AstInterface::OperatorEnum opr;
   std::vector<std::string> sigs;
   bool additional = false;

   if (fa.IsFunctionDefinition(s, 0, &vars, &args, &body)) {
      curstmt = AstNodePtrImpl(s).get_ptr();
      // Below is necessary for constructors that initialize member variables.
      for (AstInterface::AstNodePtr param : args) {
         if (fa.IsAssignment(param, &lhs, &rhs, /*read_lhs=*/&additional)) {
            if (rhs != 0) {
               operator()(fa, AstNodePtrImpl(rhs).get_ptr());
            }
            if (additional) { /*read_lhs*/
               operator()(fa, AstNodePtrImpl(lhs).get_ptr());
            }
            AppendModLoc(fa, AstNodePtrImpl(lhs).get_ptr(), AstNodePtrImpl(rhs).get_ptr());
         } 
      }
      SkipUntil(body);
      return true;
   }
   else if (fa.IsStatement(s)) {
      DebugLocalInfoCollect([&s]() { return "previsiting cur statement " + AstInterface::AstToString(s); });
      curstmt = AstNodePtrImpl(s).get_ptr();
   }
   else  {
      DebugLocalInfoCollect([&s](){return "previsiting cur node " + AstInterface::AstToString(s); });
      if (curstmt == 0) {
        AstInterface::AstNodePtr s1 = fa.GetParent(s);
        for ( ; s1 != 0 && !fa.IsStatement(s1); s1 = fa.GetParent(s1));
        if (s1 != 0) {
           curstmt = AstNodePtrImpl(s1).get_ptr();
        } else {
          DebugLocalInfoCollect([](){ return "curstmt = 0"; });
        }
      }
   }
   if (fa.IsAliasingDecl( s, &vars, &args)) {
      AstInterface::AstNodeList::const_iterator pv = vars.begin();
      AstInterface::AstNodeList::const_iterator pa = args.begin();
      while (pv != vars.end()) {
         AstNodePtr ast = *pv;
         AstNodePtr read_ast = *pa;
         AppendAliasDecl( fa, ast, read_ast);
         ++pv;
         ++pa;
      }
      vars.clear(); args.clear(); // clear out these variables so that they can be used later.
   }
   if (fa.IsAssignment(s, &lhs, &rhs, /*read_lhs*/&additional)) {
       DebugLocalInfoCollect([](){return "Is assignment"; });
       ModMap *mp = modstack.size()?  &modstack.back().modmap : 0;
       if (mp == 0 || mp->find(AstNodePtrImpl(lhs).get_ptr()) == mp->end()) {
         modstack.push_back(s);
         modstack.back().modmap[AstNodePtrImpl(lhs).get_ptr()] =  ModRecord(AstNodePtrImpl(rhs).get_ptr(),additional);
       }
   }
   else if (fa.IsVariableDecl( s, &vars, &args, &designators)) {
      DebugLocalInfoCollect([](){ return "Is variable declaration."; });
      assert(args.size() <= designators.size());
      assert(vars.size() == designators.size());
      AstInterface::AstNodeList::const_iterator pv = vars.begin();
      AstInterface::AstNodeList::const_iterator pt = designators.begin();
      AstInterface::AstNodeList::const_iterator pa = args.begin();
      modstack.push_back(s);
      while (pa != args.end()) {
         auto ast = *pv;
         auto t = *pt;
         AstNodePtr read_ast = *pa;
         if (read_ast != 0) {
            operator()(fa, read_ast);
         }
         AppendVariableDecl( fa, ast, read_ast, t);
         ++pv;
         ++pa;
         ++pt;
      }
      Skip(s);
   }
   else  if (fa.IsIOInputStmt(s, &args)) {
     DebugLocalInfoCollect([](){ return "Is IOInput stmt."; });
     modstack.push_back(s);
     for (AstInterface::AstNodeList::reverse_iterator p = args.rbegin();
           p != args.rend(); ++p) {
        AstNodePtr c = AstNodePtrImpl(*p).get_ptr();
        modstack.back().modmap[c]= ModRecord(0,false);
     }
   }
   else if (AstInterface::IsMemoryAllocation(s)) {
      AppendMemoryAllocate(fa, s.get_ptr());
   }
   else if (AstInterface::IsMemoryFree(s, 0, &lhs)) {
      AppendMemoryFree(fa, lhs.get_ptr());
   }
   else if (fa.IsUnaryOp(s, &opr, &lhs)) {
       // Below enumeate unary operators that modify its operand.
       switch (opr) {
         case AstInterface::UOP_INCR1:
         case AstInterface::UOP_INCR1_POST:
         case AstInterface::UOP_DECR1:
         case AstInterface::UOP_DECR1_POST: {
           DebugLocalInfoCollect([](){ return "Is unary operator."; });
           ModMap *mp = modstack.size()?  &modstack.back().modmap : 0;
           if (mp == 0 || mp->find(AstNodePtrImpl(lhs).get_ptr()) == mp->end()) {
              modstack.push_back(s);
              modstack.back().modmap[AstNodePtrImpl(lhs).get_ptr()] =  ModRecord(AstNodePtrImpl(s).get_ptr(),true);
           } } // end of cases
           break;
          default: break;
         }
   }
   else if (fa.IsFunctionCall(s)) {
         DebugLocalInfoCollect([&s]() { return " append function call " + AstInterface::AstToString(s); });
         AppendFuncCall(fa, AstNodePtrImpl(s).get_ptr());
         Skip(s);
   } 
   else if (fa.IsAddressOfOp(s, &lhs)) {
       SkipOnly(lhs);
   }
   else if ( fa.IsMemoryAccess(s, &vars)) {
        DebugLocalInfoCollect([&s](){ return " append read set " + AstInterface::AstToString(s); });
        if (!fa.IsSameVarRef(s, fa.GetParent(s))) { /*QY: skip s if it refers to the same thing as parent*/
          ModMap *mp = modstack.size()?  &modstack.back().modmap : 0;
          if (mp == 0 || mp->find(s) == mp->end() || (*mp)[s].readlhs) {
              AppendReadLoc(fa, s, AST_NULL);
          }
        }
        AstNodeList arglist;
        if (fa.IsArrayAccess(s, 0, &arglist))  {
           for (AstNodeList::const_iterator p = arglist.begin();
                p != arglist.end();  ++p) {
               AstNodePtr c = AstNodePtrImpl(*p).get_ptr();
               operator()(fa, c);
           }
        }
        Skip(s);
   }
 }
 else {
       AstInterface::AstNodeList vars;
       DebugLocalInfoCollect([&s](){ return "postvisiting cur node " + AstInterface::AstToString(s); });
       if (modstack.size() && modstack.back().root == AstNodePtrImpl(s).get_ptr()) {
          const ModMap &modmap = modstack.back().modmap;
          for (typename ModMap::const_iterator p = modmap.begin();
               p != modmap.end(); ++p) {
             std::pair<const AstNodePtr,ModRecord> c = *p;
             if (!fa.IsMemoryAccess(c.first, &vars)  && !vars.empty()) {
                 // Here s is not a single reference but is a group of memory references.
                 for (const auto& v : vars) {
                   AppendModLoc(fa, v, AST_UNKNOWN);
                 }
             } else {
               AppendModLoc( fa, c.first, c.second.rhs);
             }
          }
          modstack.pop_back();
       }
     if (s == curstmt) {
        curstmt = 0;
     }
 }
 return true;
}

class CollectReadRefWrap : public CollectObject<AstInterface::AstNodePtr>
{
  typedef typename SideEffectAnalysisInterface::CollectObject CollectObject; 
  CollectObject* collect;
  FunctionSideEffectInterface* func;
 protected:
  AstNodePtr stmt;
  AstInterface& fa;
 public:
  CollectReadRefWrap( AstInterface& _fa, FunctionSideEffectInterface* f,
                      const AstNodePtr& _stmt,
                      CollectObject* c)
    : collect(c), func(f), stmt(_stmt), fa(_fa) {}
  bool operator() ( const AstInterface::AstNodePtr& ref) override
   {
      AstInterface::AstNodeList args;
      AstInterface::AstNodePtr callee;
      if (fa.IsArrayAccess(ref, 0, &args)) {
        for (AstNodeList::const_iterator p = args.begin();
             p != args.end(); ++p) {
               operator()(*p);
        }
      }
      DebugLocalInfoCollect([&ref,this](){ return "appending reading " + AstInterface::AstToString(ref) + " : " + AstInterface::AstToString(stmt); });
      if (collect != 0) {
        (*collect)( { ref, stmt, AST_NULL} );
      }
      return true;
   }
};

class CollectCallRefWrap : public CollectObject<AstInterface::AstNodePtr>
{
  typedef typename SideEffectAnalysisInterface::CollectObject CollectObject; 
  CollectObject* collect;
  FunctionSideEffectInterface* func;
 protected:
  AstNodePtr stmt;
  AstInterface& fa;
 public:
  CollectCallRefWrap( AstInterface& _fa, FunctionSideEffectInterface* f,
                      const AstNodePtr& _stmt,
                      CollectObject* c)
    : collect(c), func(f), stmt(_stmt), fa(_fa) { ROSE_ASSERT(collect != 0); }
  bool operator() ( const AstInterface::AstNodePtr& ref)
   {
      AstInterface::AstNodeList args;
      AstInterface::AstNodePtr callee;
      if (fa.IsFunctionCall(ref, &callee)) {
        (*collect)( { callee, stmt, AST_NULL});
      } else {
        DebugLocalInfoCollect([&ref](){ return "Error: Expecting a function call but getting:" + AstInterface::AstToString(ref); });
        (*collect)({ref, stmt, NULL});
      }
      return true;
   }
};


class CollectModRefWrap : public CollectReadRefWrap
{
  typedef typename SideEffectAnalysisInterface::CollectObject CollectObject; 

  CollectObject* mod;
  using CollectReadRefWrap::fa;
  using CollectReadRefWrap::stmt;
 public:
  CollectModRefWrap( AstInterface& _fa, FunctionSideEffectInterface* f,
                     const AstNodePtr& _stmt,
                      CollectObject* read,
                      CollectObject* m)
    : CollectReadRefWrap(_fa, f, _stmt, read), mod(m) {}
  bool operator() ( const AstInterface::AstNodePtr& ref) override
   {
      AstNodeList args;
      if (fa.IsFunctionCall(ref, 0,&args) || fa.IsArrayAccess(ref, 0, &args)) {
        CollectReadRefWrap read(*this);
        for (AstInterface::AstNodeList::const_iterator p = args.begin();
             p != args.end(); ++p) {
               read(*p);
        }
      }
      if (mod != 0) {
        DebugLocalInfoCollect([&ref]{ return "collecting modification:" + AstInterface::AstToString(ref); });
        (*mod)({ref, stmt, AST_NULL});
      }
      return true;
   }
};

void StmtSideEffectCollect::
AppendAliasDecl(AstInterface& /* fa */, const AstNodePtr& variable, const AstNodePtr& var_init)
    {
     DebugLocalInfoCollect([&variable,&var_init](){ return "appending alias decl " + AstInterface::AstToString(variable) + " = " + AstInterface::AstToString(var_init); });
     if(curstmt == 0) return;
     if (alias_collect != 0) {
        (*alias_collect)( {variable, var_init, AST_NULL} );
     } 
    }

void StmtSideEffectCollect::
AppendVariableDecl(AstInterface& /* fa */, const AstNodePtr& variable, const AstNodePtr& var_init, const AstNodePtr& desig)
    {
     DebugLocalInfoCollect([&variable,&var_init](){ return "appending var decl " + AstInterface::AstToString(variable) + " = " + AstInterface::AstToString(var_init); });
     if(curstmt == 0) return;
     if (varcollect != 0) {
            (*varcollect)( {variable, var_init, desig});
     } else if (modcollect != 0) {
            (*modcollect)( {variable, var_init, desig});
     }
    }

void StmtSideEffectCollect::
AppendModLoc(AstInterface&, const AstNodePtr& mod, const AstNodePtr& rhs)
    {
       DebugLocalInfoCollect([&mod,&rhs](){ return "appending modifying " + AstInterface::AstToString(mod) + " = " + AstInterface::AstToString(rhs); });
       if(curstmt == 0) return;
       if (killcollect != 0 && rhs != 0) 
            (*killcollect)({mod, rhs, AST_NULL});
       if (modcollect != 0)
            (*modcollect)({mod, rhs, AST_NULL});
    }
void StmtSideEffectCollect::
AppendReadLoc(AstInterface &/*fa*/, const AstNodePtr &read, const AstNodePtr& lhs)
    {
      DebugLocalInfoCollect([&read](){ return "appending reading " + AstInterface::AstToString(read); });
      if (readcollect != 0) {          
         if (lhs==0) {
            (*readcollect)({read, curstmt, AST_NULL});
         } else {
            (*readcollect)({read, lhs, AST_NULL});
         }
      }
    }

void StmtSideEffectCollect::
AppendFuncCall( AstInterface& fa, const AstNodePtr& fc)
{
 CollectReadRefWrap read(fa, funcanal, curstmt, readcollect);
 DebugLocalInfoCollect([&fc](){ return "Checking function info: " + AstInterface::AstToString(fc); });

 AstNodePtr callee;
 AppendFuncCallArguments(fa, fc, &callee);
 if (funcanal == 0 || !funcanal->get_read(fa, fc, &read))  {
      readunknown = true;
      DebugLocalInfoCollect([&fc](){ return "no interprecedural read info for : " + AstInterface::AstToString(fc) + "adding function call arguments."; });
      callee.set_is_unknown_function_call();
  }
  CollectModRefWrap mod(fa, funcanal, curstmt, readcollect, modcollect);
  if (funcanal == 0 || !funcanal->get_modify( fa, fc, &mod))  {
      DebugLocalInfoCollect([&fc](){ return "no interprecedural mod info for : " + AstInterface::AstToString(fc); });
      AppendFuncCallWrite(fa, fc);
      modunknown = true;
      callee.set_is_unknown_function_call();
  }
  if (callcollect != 0) {
     DebugLocalInfoCollect([](){ return "invoking collecting call"; });
     (*callcollect)({callee, fc, AST_NULL});
  }
}
void StmtSideEffectCollect::
AppendMemoryAllocate( AstInterface& /* fa */, const AstNodePtr& s) {
   DebugLocalInfoCollect([](){ return "invoking collecting Memory Allocate"; });
   AstNodePtr init;
   AstNodeList  dest;
   if (AstInterface::IsMemoryAllocation(s, 0, &init, &dest)) {
      if (allocate_collect != 0) {
          (*allocate_collect)({s, init, AST_NULL});
      }
      if (modcollect != 0) {
        for (auto e : dest) {
           (*modcollect)({e, init, AST_NULL});
        }
      }  
   }
}
void StmtSideEffectCollect::
AppendMemoryFree( AstInterface& /* fa */, const AstNodePtr& s) {
   if (free_collect != 0) {
      (*free_collect)({s, curstmt, AST_NULL});
   } else if (modcollect != 0) {
       (*modcollect)({s, curstmt,AST_NULL});
   }
}
class ModifyAliasMap : public CollectObject< std::pair<AstNodePtr, int> >
{
  AstInterface& fa;
  StmtVarAliasCollect::VarAliasMap& aliasmap;
  UF_elem *repr;
  int index;
public:
  ModifyAliasMap(AstInterface& _fa, StmtVarAliasCollect::VarAliasMap& m)
    : fa(_fa), aliasmap(m), repr(0), index(-1) {}

  void reset()
  {
    index = -1;
    repr = 0;
  }

  bool operator() ( const std::pair<AstNodePtr, int>& cur)
  {
    std::string varname;
    AstNodePtr scope;
    DebugLog DebugAliasAnal("-debugaliasanal");
    if (cur.first == 0 || !fa.IsVarRef(cur.first, 0, &varname, &scope, 0, /*use_global_unique_name=*/true))
      return false;
    if (cur.second == index) {
      aliasmap.get_alias_map(varname, scope)->union_with(repr);
      DebugAliasAnal([&varname](){ return "Aliasing with: " + varname; });
    }
    else {
      assert( cur.second > index);
      repr = aliasmap.get_alias_map(varname, scope);
      DebugAliasAnal([&varname](){ return "cur alias-var repr:" + varname; });
      index = cur.second;
    }
    return true;
  }
};

Ast2StringMap* Ast2StringMap::handle = 0;

Ast2StringMap* Ast2StringMap::inst()
{
  if (handle == 0)
    handle = new Ast2StringMap();
  return handle;
}

std::string Ast2StringMap :: get_string( const AstNodePtr& s)
{
  if ( s == 0) return "";
  MapType::const_iterator p = astmap.find(s);
  if (p != astmap.end())
    return (*p).second;
  std::stringstream buf;
  ++cur;
  buf <<  cur;
  std::string r= buf.str();
  astmap[s] = r;
  return r;
}

std::string Ast2StringMap :: lookup_string( const AstNodePtr& s) const
{
  if ( s == 0) return "";
  MapType::const_iterator p = astmap.find(s);
  if (p == astmap.end())  return "";
  return (*p).second;
}

std::string Ast2StringMap :: get_string( const AstNodePtr& s) const
{
  if ( s == 0) return "";
  MapType::const_iterator p = astmap.find(s);
  if (p == astmap.end()) {
     std::cerr << "Error: cannot find ast: " << AstInterface::AstToString(s) << std::endl;
     for (p = astmap.begin(); p != astmap.end(); ++p) {
          std::cerr << " : " << AstInterface::AstToString((*p).first) << ":" << (*p).second << std::endl;
     }
     ROSE_ABORT();
  }
  return (*p).second;
}

std:: string InterProcVariableUniqueRepr::
get_unique_name(AstInterface& fa, const AstNodePtr& exp)
{
  std::string expname = Ast2StringMap::inst()->get_string(exp);
  return get_unique_name(fa, exp, expname);
}

std:: string InterProcVariableUniqueRepr::
get_unique_name(AstInterface& fa, const AstNodePtr& _scope, const std::string& varname)
    {
      if (_scope == 0) return varname;
      std::string fname;
      AstNodePtr func = fa.GetFunctionDefinition(_scope, &fname);
     if (func == 0) return varname;
     return get_unique_name(fname, func, varname);
 }


UF_elem*  StmtVarAliasCollect::VarAliasMap::
get_alias_map( const std::string& varname, const AstNodePtr& scope)
{
  std::string scopename = Ast2StringMap::inst()->get_string(scope);
  std::string name = varname + scopename;
  if (aliasmap.find(name) == aliasmap.end())
     aliasmap[name] = new UF_elem();
  return aliasmap[name];
}

void StmtVarAliasCollect::
AppendVariableDecl( AstInterface& fa, const AstNodePtr& var, const AstNodePtr& init, const AstNodePtr& desig) {
  return AppendModLoc(fa, var, init);
}

void StmtVarAliasCollect::
AppendAliasDecl( AstInterface& fa, const AstNodePtr& var, const AstNodePtr& init) {
  return AppendModLoc(fa, var, init);
}

void StmtVarAliasCollect::
AppendModLoc( AstInterface& fa, const AstNodePtr& mod,
                              const AstNodePtr& rhs)
{
  DebugLog DebugAliasAnal("-debugaliasanal");
  std::string modname;
  AstNodeType modtype;
  AstNodePtr modscope;
  if (rhs == 0 || !fa.IsVarRef(mod, &modtype, &modname, &modscope) || fa.IsScalarType(modtype))
    return;
  AstInterface::AstNodeList args;
  if (fa.IsFunctionCall( rhs, 0, &args) ) {
    ModifyAliasMap collect(fa, aliasmap);
    if (funcanal != 0 && funcanal->may_alias( fa, rhs, mod, collect))
        return;
    hasunknown = true;
    DebugAliasAnal([&rhs](){ return "unknown alias info for function call : " + AstInterface::AstToString(rhs); });
    DebugAliasAnal([&mod](){ return "aliasing all parameters with " + AstInterface::AstToString(mod); });
    collect.reset();
    collect( std::pair<AstNodePtr,int>(mod, 0));
    for (AstInterface::AstNodeList::const_iterator p = args.begin();
          p != args.end() ; ++p) {
       collect( std::pair<AstNodePtr,int>(*p, 0));
    }
  }
  else {
    std::string rhsname;
    AstNodeType rhstype;
    AstNodePtr rhsscope;
    if (fa.IsVarRef(rhs, &rhstype, &rhsname, &rhsscope)) {
      if (!fa.IsScalarType(rhstype))
         aliasmap.get_alias_map(modname, modscope)->union_with(aliasmap.get_alias_map(rhsname, rhsscope));
    }
  }
}

void StmtVarAliasCollect::
AppendFuncCall( AstInterface& fa, const AstNodePtr& fc)
{ 
  ModifyAliasMap collect(fa, aliasmap);
  DebugLog DebugAliasAnal("-debugaliasanal");
  if (funcanal == 0 || !funcanal->may_alias( fa, fc, 0, collect)) {
     hasunknown = true;
     DebugAliasAnal([&fc](){ return "unknown alias info. for function call : " + AstInterface::AstToString(fc) + "; Aliasing all parameters"; });
     collect.reset();
     AstInterface::AstNodeList args;
     if (!fa.IsFunctionCall( fc, 0, &args))
         ROSE_ABORT();
     for (AstInterface::AstNodeList::const_iterator p = args.begin();
           p != args.end(); ++p) {
         collect( std::pair<AstNodePtr,int>(*p, 0));
     }
  }
}

bool StmtVarAliasCollect::
may_alias(AstInterface& fa, const AstNodePtr& r1,
          const AstNodePtr& r2)
{
  DebugLog DebugAliasAnal("-debugaliasanal");
  if (r1 == AST_NULL || r2 == AST_NULL) {
    return false;
  }
  if (r1 == AST_UNKNOWN || r2 == AST_UNKNOWN) {
    return true;
  }
  AstNodeType t1, t2;
  if (!fa.IsExpression(r1, &t1) || !fa.IsExpression(r2, &t2))
    ROSE_ABORT();
  if (!hasresult) {
    DebugAliasAnal([&r1,&r2](){ return "No alias analysis performed. Has alias between " + AstInterface::AstToString(r1) + " and " + AstInterface::AstToString(r2); });
    return true;
  }
  std::string varname1, varname2;
  AstNodePtr scope1, scope2;
  bool global1, global2;
  if (!fa.IsVarRef(r1, 0, &varname1, &scope1, &global1)
      || !fa.IsVarRef(r2, 0, &varname2, &scope2, &global2)) {
    DebugAliasAnal([&r1, &r2](){ return "no alias analysis between non_variables performed. Has alias between " + AstInterface::AstToString(r1) + " and " + AstInterface::AstToString(r2); });
    return true;
  }

  if (global1 && global2 && hasunknown) {
    if (global1) {
       DebugAliasAnal([&varname1,&varname2](){ return "INFO:" + varname1 + " and " + varname2 + " are global"; });
       DebugAliasAnal([&r1,&r2]() { return "INFO: Has alias between " + AstInterface::AstToString(r1) + " and " + AstInterface::AstToString(r2); });
    }
    return true;
  }
  if ( aliasmap.get_alias_map(varname1, scope1)->in_same_group( aliasmap.get_alias_map(varname2, scope2))) {
     DebugAliasAnal([&r1,&r2](){ return "INFO: Alias analysis performed. Has alias between " + AstInterface::AstToString(r1) + " and " + AstInterface::AstToString(r2); });
     return true;
  }
  return false;
}

void StmtVarAliasCollect::
operator()( AstInterface& fa, const AstNodePtr& funcdef)
{ analyze(fa, funcdef);
  return; }


void StmtVarAliasCollect::
analyze( AstInterface& fa, const AstNodePtr& funcdef)
{
  assert(!hasresult);
  hasresult = true;
  AstNodePtr  body;
  AstInterface::AstNodeList params;
  if (!fa.IsFunctionDefinition(funcdef, 0, &params, 0, &body)) {
     std::cerr << "Error: alias analysis requires function definition as input instead of " << AstInterface::AstToString(funcdef) << std::endl;
     ROSE_ABORT();
  }

  ModifyAliasMap collect(fa, aliasmap);
  if (funcanal == 0 || !funcanal->allow_alias( fa, funcdef, collect)) {
     for (AstInterface::AstNodeList::const_iterator p = params.begin();
          p != params.end(); ++p) {
         collect( std::pair<AstNodePtr,int>(*p, 0));
     }
  }
  StmtInfoCollect::operator()(fa, body);
}
