
#ifndef SYMBOLIC_VAL
#define SYMBOLIC_VAL

#include "CountRefHandle.h"
#include "FunctionObject.h"
#include "AstInterface.h"
#include <assert.h>
#include <string>
#include <vector>
#include <ostream>

class SymbolicValImpl;
class SymbolicConst;
class SymbolicVar;
class SymbolicFunction;
class SymbolicExpr;
class SymbolicAstWrap;
class SymbolicVisitor
{
 protected:
  virtual void Default() {}
 public:
  virtual void VisitConst(const SymbolicConst&) { Default(); }
  virtual void VisitVar(const SymbolicVar&) { Default(); }
  virtual void VisitFunction(const SymbolicFunction&) { Default(); }
  virtual void VisitAstWrap(const SymbolicAstWrap&) { Default(); }
  virtual void VisitExpr(const SymbolicExpr&) { Default(); }
  virtual void VisitUnknown( const SymbolicValImpl&) { Default(); }
  virtual ~SymbolicVisitor() {}
};

typedef enum {VAL_BASE = 0, VAL_CONST = 1, VAL_VAR = 2, VAL_AST = 4, 
              VAL_FUNCTION = 8, VAL_EXPR = 16, VAL_UNKNOWN = 32}
      SymbolicValType;
typedef enum { SYMOP_NIL = 0, SYMOP_MULTIPLY=1, SYMOP_PLUS = 2,
               SYMOP_MIN=3, SYMOP_MAX=4, SYMOP_POW = 5,
               SYMOP_AND = 6, SYMOP_OR = 7, SYMOP_EQ = 8,
               SYMOP_NOT = 9} SymOpType;

class SymbolicValImpl 
{
 protected:
  virtual ~SymbolicValImpl() {}
 public:
  virtual std:: string toString() const { return "UNKNOWN"; }
  virtual void Dump() const;

  virtual SymOpType GetOpType() const { return SYMOP_NIL; }
  virtual SymbolicValType GetValType() const { return VAL_UNKNOWN; }
  virtual std:: string GetTypeName() const { return "unknown"; }
  virtual void Visit( SymbolicVisitor *op) const { return op->VisitUnknown(*this); }
  virtual AstNodePtr  CodeGen(AstInterface &) const { return AST_UNKNOWN; }
  virtual SymbolicValImpl* Clone() const  { return new SymbolicValImpl(); }

 friend class CountRefHandle<SymbolicValImpl>;
};

inline SymbolicValImpl* Clone(const SymbolicValImpl& that) 
  { return that.Clone(); }


class SymbolicConst : public SymbolicValImpl
{
  std:: string val, type;
  int intval, dval;

  SymbolicValImpl* Clone() const { return new SymbolicConst(*this); }
 public:
  SymbolicConst( int _val, int _dval = 1) ;
  SymbolicConst( std:: string _val, std:: string type);
  SymbolicConst( const SymbolicConst& that) 
       : val(that.val), type(that.type), intval(that.intval), dval(that.dval) {}
  ~SymbolicConst() {}

  bool operator == (const SymbolicConst& that) const 
    { return val == that.val && type == that.type; }
  virtual std:: string toString() const;
  virtual void Dump() const { std:: cerr << val << "(" << type << ")"; }
  SymbolicValType GetValType() const { return VAL_CONST; }
  std:: string GetTypeName() const { return type; }
  AstNodePtr  CodeGen(AstInterface &) const;
  void Visit( SymbolicVisitor *op) const { op->VisitConst(*this); }
  bool GetIntVal( int &val1, int &val2) const
          { 
            if (type == "int" || type == "fraction") {
             val1 = intval; val2 = dval; return true; 
            }
            return false;
          }
  bool GetConstVal( std:: string& result, std:: string& rtype) const 
          { 
             result = val; rtype = type; return true; 
          }
  std:: string GetVal() const { return val; }
};

class SymbolicVar : public SymbolicValImpl
{
  std:: string varname;
  AstNodePtr scope, exp_;
  SymbolicValImpl* Clone() const { return new SymbolicVar(*this); }
 public:
  SymbolicVar( std:: string name, const AstNodePtr& sc, const AstNodePtr exp = AST_NULL) : varname(name), scope(sc), exp_(exp) {}
  SymbolicVar() : varname(""), scope(AST_NULL), exp_(AST_NULL) {}
  SymbolicVar(const SymbolicVar& that) : varname(that.varname), scope(that.scope), exp_(that.exp_) {}
  SymbolicVar& operator=(const SymbolicVar& that) { varname = that.varname; scope = that.scope; exp_ = that.exp_; return *this; }
  ~SymbolicVar() {}
  virtual std:: string toString() const;
  virtual void Dump() const { std:: cerr << varname; }
  SymbolicValType GetValType() const { return VAL_VAR; }
  AstNodePtr  CodeGen( AstInterface &) const;

  std:: string GetVarName() const { return varname; }
  AstNodePtr GetVarScope() const  { return scope; }
  bool operator == (const SymbolicVar &that) const ;
  void Visit( SymbolicVisitor *op) const { op->VisitVar(*this); }
};

class SymbolicAstWrap : public SymbolicValImpl
{
  AstNodePtr ast;
  Map2Object <AstInterface*, AstNodePtr, AstNodePtr>* codegen;

  SymbolicValImpl* Clone() const { return new SymbolicAstWrap(*this); }
 public:
  SymbolicAstWrap( const AstNodePtr& _ast, 
                   Map2Object <AstInterface*, AstNodePtr, AstNodePtr>* cg = 0) 
    : ast(_ast), codegen(cg) {}
  SymbolicAstWrap() : ast(AST_NULL), codegen(0) {}
  SymbolicAstWrap( const SymbolicAstWrap& that)
    : ast(that.ast), codegen(that.codegen) {}

  ~SymbolicAstWrap() {}
  virtual void Dump() const;
  virtual std:: string toString() const;

  SymbolicValType GetValType() const { return VAL_AST; }
  virtual std:: string GetTypeName() const { return "astwrap"; }
  AstNodePtr CodeGen( AstInterface &) const ;

  void Visit( SymbolicVisitor *op) const { op->VisitAstWrap(*this); }
  bool operator == (const SymbolicAstWrap& that) const
     { return ast == that.ast; }
  const AstNodePtr& get_ast() const { return ast; }
};

class SymbolicVal : public CountRefHandle <SymbolicValImpl>
{
 public:
  SymbolicVal() {}
  SymbolicVal( SymbolicValImpl* _impl)
    : CountRefHandle <SymbolicValImpl>(_impl) {}
  SymbolicVal( const SymbolicValImpl& _impl)
    : CountRefHandle <SymbolicValImpl>(_impl) {}
  SymbolicVal( const std::string& val, const std::string& valtype) 
    : CountRefHandle <SymbolicValImpl>(new SymbolicConst(val,valtype)) {}
  SymbolicVal (int val) ;
  SymbolicVal( const SymbolicVal& that)
   : CountRefHandle <SymbolicValImpl>(that) {}
  SymbolicVal& operator = (const SymbolicVal& that)
   { CountRefHandle <SymbolicValImpl>:: operator = (that); return *this; }
  virtual ~SymbolicVal() {}

  bool IsNIL() const { return ConstPtr() == 0; }
  bool IsSame( const SymbolicVal& that) const
     { return ConstPtr() == that.ConstPtr(); }
  bool IsSame( const SymbolicValImpl& impl) const
     { return ConstPtr() == &impl; }
  AstNodePtr CodeGen(AstInterface &fa) const
     { return (ConstPtr()==0)? AstNodePtr(AST_NULL) : ConstRef().CodeGen(fa); }
  void Visit( SymbolicVisitor *op) const 
         { if (ConstPtr()!=0) 
                 ConstRef().Visit(op); }
  void Dump() const;
  std:: string toString() const;
  bool isConstInt(int &val1, int &val2) const 
   { 
     if (ConstPtr() != 0 && ConstRef().GetValType() == VAL_CONST) {
         const SymbolicConst& c = static_cast<const SymbolicConst&>(ConstRef());
         return c.GetIntVal(val1, val2);
      }
     return false;
   }
  bool isConstInt(int &val) const 
  {
     int val2;
     return isConstInt(val, val2) && val2 == 1;
  }
  bool isAstWrap(AstNodePtr& r) const 
  { 
   if (ConstPtr() != 0 && ConstRef().GetValType() == VAL_AST) {
     const SymbolicAstWrap& c = static_cast<const SymbolicAstWrap&>(ConstRef());
     r = c.get_ast();
     return true;
   }
   return false;
  }
  bool isVar(std:: string& name, AstNodePtr* scope=0) const 
  { 
   if (ConstPtr() != 0 && ConstRef().GetValType() == VAL_VAR) {
     const SymbolicVar& c = static_cast<const SymbolicVar&>(ConstRef());
     name = c.GetVarName();
     if (scope != 0)
       *scope = c.GetVarScope();
     return true;
   }
   return false;
  }
  bool isFunction(AstInterface::OperatorEnum* op =0, std::string* name=0, std:: vector<SymbolicVal>* argp=0) const;

  SymbolicValType GetValType() const 
      { return (ConstPtr()== 0)? VAL_BASE : ConstRef().GetValType(); }
  virtual SymOpType GetOpType() const  
     { return  (ConstPtr()== 0)? SYMOP_NIL : ConstRef().GetOpType(); }
  std:: string GetTypeName() const 
    { return (ConstPtr()== 0)? std:: string("NIL") : ConstRef().GetTypeName(); }
};

class SymbolicFunction : public SymbolicValImpl
{
  SymbolicVal op;
  std:: vector<SymbolicVal> args;
 protected:
  typedef AstInterface::OperatorEnum OpType;
  OpType t;

 public:
  typedef std:: vector<SymbolicVal> Arguments;
  typedef std:: vector<SymbolicVal>::const_iterator const_iterator;
  SymbolicFunction( AstInterface::OperatorEnum _t, const SymbolicVal& _op, 
                    const Arguments& v)
    : op(_op), args(v), t(_t) {} 
  SymbolicFunction( AstInterface::OperatorEnum _t, const std::string& _op, 
                    const Arguments& v)
    : op(new SymbolicConst(_op, "function")), args(v), t(_t) {} 
  SymbolicFunction( AstInterface::OperatorEnum _t, const std::string& _op, 
                    const SymbolicVal& v1, const SymbolicVal& v2)
    : op(new SymbolicConst(_op, "function")), t(_t) { args.push_back(v1); args.push_back(v2); }
  SymbolicFunction( AstInterface::OperatorEnum _t, const std::string& _op, 
                    const SymbolicVal& v)
    : op(new SymbolicConst(_op, "function")), t(_t) { args.push_back(v); }
  SymbolicFunction( const SymbolicFunction& that)
    : op(that.op), args(that.args), t(that.t) {}

  virtual ~SymbolicFunction() {}
  virtual std:: string toString() const;
  SymbolicValType GetValType() const { return VAL_FUNCTION; }
  AstNodePtr CodeGen( AstInterface &) const;
  virtual void Visit( SymbolicVisitor *v) const { v->VisitFunction(*this); }
  bool operator == (const SymbolicFunction& that) const;
  SymbolicVal GetOp() const { return op; }
  virtual OpType GetAstOpType() const { return t; }
  const Arguments& get_args() const { return args; }
  SymbolicVal get_arg(int index) const { return args[index]; }
  const_iterator args_begin() const { return args.begin(); }
  const_iterator args_end() const { return args.end(); }
  SymbolicVal first_arg() const { return args.front(); }
  SymbolicVal last_arg() const { return args.back(); }
  unsigned NumOfArgs() const { return args.size(); }
  virtual SymbolicVal GetUnknownOpds() const 
      { return SymbolicVal(); }
  virtual bool GetConstOpd(int&, int&) const { return false; }
  SymbolicValImpl* Clone() const { return new SymbolicFunction(*this); }
  virtual SymbolicVal cloneFunction(const Arguments& args) const;
};

inline bool 
SymbolicVal:: isFunction(AstInterface::OperatorEnum* op, std::string* name, std:: vector<SymbolicVal>* argp) const
{
   if (ConstPtr() != 0 && ConstRef().GetValType() == VAL_FUNCTION) {
     const SymbolicFunction& c = static_cast<const SymbolicFunction&>(ConstRef());
     if (op != 0)
        *op = c.GetAstOpType();
     if (name != 0) 
        *name = c.GetOp().toString();
     if (argp != 0)
       *argp = c.get_args();
     return true;
   }
   return false;
}

class SymbolicPow : public SymbolicFunction
{
 public:
  SymbolicPow( const SymbolicVal& v, int e ) 
    : SymbolicFunction(AstInterface::OP_NONE, "pow", v, e) { }
  SymbolicPow( const Arguments& _args ) 
       : SymbolicFunction(AstInterface::OP_NONE, "pow", _args) {}
  SymbolicPow( const SymbolicPow& that)
    : SymbolicFunction(that) {}
  virtual SymOpType GetOpType() const { return SYMOP_POW; }
  virtual SymbolicVal GetUnknownOpds() const { return first_arg(); }
  virtual bool GetConstOpd(int &val1, int &val2) const 
            { return last_arg().isConstInt(val1, val2); }
  SymbolicValImpl* Clone() const { return new SymbolicPow(*this); }
  virtual SymbolicVal cloneFunction(const Arguments& args) const
     { SymbolicFunction* r =  new SymbolicPow(args); return r; }
};

/******************* Symbolic Operator interface *************/

class SymbolicValGenerator
{
 public:
 static SymbolicVal GetSymbolicVal( AstInterface &fa, const AstNodePtr& exp);
 static SymbolicVal GetSymbolicVal(AstInterface::OperatorEnum op, 
                                  const std:: vector<SymbolicVal>& args);
 static SymbolicVal GetSymbolicVal( const std::string& sig);
 static SymbolicVal get_null() { return SymbolicVal(); }
 static SymbolicVal get_unknown() { return new SymbolicValImpl(); }
 static bool IsFortranLoop(AstInterface& , const AstNodePtr& s, 
        SymbolicVar* ivar =0,
        SymbolicVal* lb =0, SymbolicVal* ub=0, SymbolicVal* step=0, AstNodePtr* body=0);
};

inline SymbolicVal SymbolicFunction::cloneFunction(const std::vector<SymbolicVal>& args)  const
  { return SymbolicValGenerator::GetSymbolicVal(t, args); }
SymbolicVal ApplyFunction();
SymbolicVal ApplyBinOP( SymOpType t, const SymbolicVal &v1,
                        const SymbolicVal &v2);
SymbolicVal ApplyUnaryOP(SymOpType t, const SymbolicVal &v);
inline SymbolicVal operator + (const SymbolicVal &v1, const SymbolicVal &v2)
  { return ApplyBinOP(SYMOP_PLUS, v1, v2); }
SymbolicVal operator * (const SymbolicVal &v1, const SymbolicVal &v2);
SymbolicVal operator / (const SymbolicVal &v1, const SymbolicVal &v2);
inline SymbolicVal operator - (const SymbolicVal &v1, const SymbolicVal &v2)
         { return v1 + (-1 * v2); }
inline SymbolicVal operator - (const SymbolicVal &v) { return -1 * v; }
                                                                                          
typedef enum {REL_NONE = 0, REL_EQ = 1, REL_LT = 2, REL_LE = 3,
              REL_GT = 4, REL_GE = 5, REL_NE = 6, REL_UNKNOWN = 8} CompareRel;
 
class SymbolicCond
{
  SymbolicVal val1, val2;
  CompareRel t;
 public:
  SymbolicCond() { t = REL_UNKNOWN; }
  SymbolicCond( const CompareRel _t, const SymbolicVal &_val1,
                const SymbolicVal &_val2)
      : val1(_val1), val2(_val2), t(_t) {}
  SymbolicCond( const SymbolicCond& that)
    : val1(that.val1), val2(that.val2), t(that.t) {}
  SymbolicCond& operator = (const SymbolicCond& that)
    { t = that.t; val1 = that.val1; val2 = that.val2;  return *this; }
  ~SymbolicCond() {}
                                                                                          
  SymbolicVal  GetLeftVal() const { return val1; }
  SymbolicVal  GetRightVal() const { return val2; }
  CompareRel GetRelType() const { return t; }
                                                                                          
  bool IsNIL() { return t == REL_UNKNOWN; }
                                                                                          
  void Dump() const;
  std:: string toString () const;
  AstNodePtr  CodeGen(AstInterface&) const;
};

struct SymbolicBound{
  SymbolicVal lb, ub;
  SymbolicBound() {}
  SymbolicBound( const SymbolicVal &l, const SymbolicVal &u): lb(l),ub(u) {}
  std:: string toString() const
      { return "(" + lb.toString() + "," + ub.toString() + ")"; }
  SymbolicBound operator - (const SymbolicVal& v) const
   { return SymbolicBound( lb - v, ub - v); }
  SymbolicBound operator + (const SymbolicVal& v) const
   { return SymbolicBound( lb + v, ub + v); }
  void Union( const SymbolicBound& b2,
              MapObject<SymbolicVal, SymbolicBound>* f = 0);
  void Intersect( const SymbolicBound& b2,
                  MapObject<SymbolicVal, SymbolicBound>* f = 0);
  void ReplaceVars(  MapObject<SymbolicVal,SymbolicBound>& f);
};
                                                                                          
CompareRel Reverse( CompareRel rel);
std:: string RelToString( CompareRel r);
bool Equal(const SymbolicVal &v1, const SymbolicVal& v2);
CompareRel CompareVal( const SymbolicVal &v1, const SymbolicVal &v2, 
                       MapObject<SymbolicVal, SymbolicBound>* f = 0);
                                                                                          
inline bool operator == (const SymbolicVal &v1, const SymbolicVal& v2)
     { return Equal(v1,v2); }
inline bool operator != (const SymbolicVal &v1, const SymbolicVal& v2)
{ return !(v1 == v2); }
inline bool operator <= (const SymbolicVal &v1, const SymbolicVal &v2)
{
  switch (CompareVal(v1,v2)) {
  case REL_EQ: case REL_LT: case REL_LE:
    return true;
  default:
    return false;
  }
}
                                                                                          
inline bool operator >= (const SymbolicVal &v1, const SymbolicVal &v2)
{
  switch (CompareVal(v1,v2)) {
  case REL_EQ: case REL_GT: case REL_GE:
    return true;
  default:
    return false;
  }
}
                                                                                          
bool operator < (const SymbolicVal &v1, const SymbolicVal& v2);
bool operator > (const SymbolicVal &v1, const SymbolicVal& v2);

bool HasFraction(const SymbolicVal& v, SymbolicVal* integral = 0, SymbolicVal* frac = 0);

bool operator ==( const SymbolicBound& b1, const SymbolicBound& b2);
bool operator != (const SymbolicBound &b1, const SymbolicBound& b2);
SymbolicBound& operator &= ( SymbolicBound& b1, const SymbolicBound& b2);
SymbolicBound& operator |= (SymbolicBound& b1, const SymbolicBound& b2);
SymbolicBound GetValBound(SymbolicVal val, MapObject<SymbolicVal, SymbolicBound>& f);
SymbolicVal GetValLB(const SymbolicVal& val, MapObject<SymbolicVal, SymbolicBound>& f);
SymbolicVal GetValUB(const SymbolicVal& val, MapObject<SymbolicVal, SymbolicBound>& f);

SymbolicVal Max(const SymbolicVal &v1, const SymbolicVal &v2,
                       MapObject<SymbolicVal, SymbolicBound>* f = 0);
SymbolicVal Min(const SymbolicVal &v1, const SymbolicVal &v2,
                       MapObject<SymbolicVal, SymbolicBound>* f = 0);
SymbolicVal UnwrapVarCond( const SymbolicCond& valCond,
                         const SymbolicVar &pivot, SymbolicBound& pivotBound ); //return pivot coefficient

bool FindVal( const SymbolicVal &v, const SymbolicVal &sub);
SymbolicVal ReplaceVal( const SymbolicVal &v, const SymbolicVal &sub, const SymbolicVal& newval);
SymbolicVal ReplaceVal( const SymbolicVal &v, MapObject<SymbolicVal, SymbolicVal>& valmap);

class AstTreeReplaceVar : public ProcessAstNode<AstNodePtr>
{
   SymbolicVar oldvar;
   SymbolicVal val;
  public:
   AstTreeReplaceVar( const SymbolicVar& ov, const SymbolicVal &nv)
       : oldvar(ov), val(nv) {}
   bool Traverse( AstInterface &fa, const AstNodePtr& r,
                     AstInterface::TraversalVisitType t)
     {
        std:: string varname;
        if (t == AstInterface::PostVisit && fa.IsVarRef(r, 0, &varname)) {
          if (varname == oldvar.GetVarName()) {
            AstNodePtr r1 = val.CodeGen(fa);
            fa.ReplaceAst(r,r1);
          }
        }
        return true;
     }
   void operator() ( AstInterface &fa, const AstNodePtr& root)
       { ReadAstTraverse( fa, root, *this, AstInterface::PostOrder); }
};

#endif

