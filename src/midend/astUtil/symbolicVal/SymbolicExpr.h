#ifndef SYMBOLIC_EXPR
#define SYMBOLIC_EXPR

#include "SymbolicVal.h"
#include "LatticeElemList.h"
#include <assert.h>
#include <iostream>


SymbolicVal ApplyBinOP( SymOpType t, const SymbolicVal &v1,
                        const SymbolicVal &v2);
class SymbolicTerm
{
  int time1, time2;
  SymbolicVal v;
 public:
  SymbolicTerm( int t1, int t2, const SymbolicVal& _v) 
          : time1(t1), time2(t2), v( _v) { }
  SymbolicTerm( int t1, int t2) : time1(t1), time2(t2) {}
  SymbolicTerm() : time1(0), time2(1) {}
  ~SymbolicTerm() {}
  SymbolicTerm& operator = (const SymbolicTerm& that)
     { time1 = that.time1; time2 = that.time2; v = that.v; return *this; }
  SymbolicTerm( const SymbolicTerm& that) :
     time1(that.time1), time2(that.time2), v(that.v) { }

  std::string toString() const;
  bool IsConst() const
      {  return v.IsNIL(); }
  bool IsConstInt( int& val1, int& val2) const 
      {  if (v.IsNIL()) { val1=time1; val2=time2;  return true; } 
         return false;
      }
  bool IsConstInt( int& val) const 
      {  if (v.IsNIL() && time2 == 1) { val=time1; return true; } 
         return false;
      }
  SymbolicVal GetVal( SymOpType op)  const
          { 
            if (v.IsNIL())
                return SymbolicConst(time1, time2); 
            else if ( time1 == 1 && time2 == 1)
               return v;
            return ApplyBinOP( op, v, SymbolicConst(time1,time2));
          }

  bool IsTop() const { return  !v.IsNIL() && time1 == 0; } 
  bool CombineWith( const SymbolicTerm &that)  ;
  bool operator == (const SymbolicTerm& that) const;
  bool operator == (const SymbolicVal &that) const;
 friend class SymbolicExpr;
};

inline bool IsZero( const SymbolicTerm& t)
      { int val1,val2; 
        return t.IsConstInt(val1,val2) && val1 == 0; }
inline bool IsOne( const SymbolicTerm& t)
      { int val1, val2;
        return t.IsConstInt(val1,val2) && val1 == 1 && val2 == 1; }
inline bool operator != (const SymbolicTerm& v1, const SymbolicTerm& v2)
{  return !(v1 == v2); }

class OPApplicator;
class SymbolicOperands
{
  LatticeElemList<SymbolicTerm> opds;
 public:
  typedef LatticeElemList<SymbolicTerm>::iterator OpdIterator;

  SymbolicOperands() {}
  SymbolicOperands( const SymbolicOperands& that)
    { for (OpdIterator iter(that.opds); !iter.ReachEnd(); iter.Advance())
         opds.AddElem(iter.Current());
    }

  void AddOpd( const SymbolicTerm& v, OPApplicator *op = 0) ;
  unsigned NumOfOpds() const { return opds.NumberOfEntries(); }
  OpdIterator GetOpdIterator() const { return OpdIterator(opds); }
  SymbolicTerm& First() const { assert(opds.First() != NULL); return opds.First()->GetEntry(); }
  std::string toString() const;

  SymbolicOperands* Clone() const { return new SymbolicOperands(*this); }
};

class SymbolicExpr : public SymbolicValImpl, 
                     public CountRefHandle<SymbolicOperands>
{
  void operator=(const SymbolicExpr&) {}

 protected:
  SymbolicExpr() {}
  SymbolicExpr( const SymbolicExpr& that)
    : SymbolicValImpl(that), CountRefHandle<SymbolicOperands>(that) {}

  virtual SymOpType GetTermOP() const = 0;
 public:
  SymbolicTerm Val2Term(const SymbolicVal& v) const;
  SymbolicVal Term2Val( const SymbolicTerm& tm) const
          { return tm.GetVal(GetTermOP()); }

  typedef LatticeElemList<SymbolicTerm>::iterator OpdIterator;
  void AddOpd( const SymbolicTerm& v, OPApplicator* op = 0);
  void AddOpd( const SymbolicVal& v, OPApplicator* op = 0);

  virtual void ApplyOpd(  const SymbolicVal& v ) = 0;
  virtual SymbolicExpr* DistributeExpr(SymOpType t, const SymbolicVal& that) const = 0;
  void Visit( SymbolicVisitor* op) const { op->VisitExpr(*this); }
  SymbolicValImpl* Clone() const { return CloneExpr(); }
  std::string toString() const;
  SymbolicValType GetValType() const { return VAL_EXPR; }
  std:: string GetTypeName() const  { return "expr"; }
  void push_back( const SymbolicVal& v) { ApplyOpd(v); }
  virtual std::string GetOPName () const = 0;

  virtual SymOpType GetOpType() const = 0;
  virtual SymbolicExpr* CloneExpr() const  = 0;

  virtual AstNodePtr CodeGenOP( AstInterface &fa, 
                               const AstNodePtr& a1, const AstNodePtr& a2) const =0;

  virtual AstNodePtr CodeGen( AstInterface &fa) const
  {
   OpdIterator iter = GetOpdIterator();
   AstNodePtr l = Term2Val(iter.Current()).CodeGen(fa);
   for (iter.Advance(); !iter.ReachEnd(); iter.Advance())
      l = CodeGenOP(fa, l, Term2Val(iter.Current()).CodeGen(fa));
   return l;
  }

  SymbolicVal GetUnknownOpds() const;
  bool GetConstOpd(int &val1, int &val2) const;
  unsigned NumOfOpds() const ;
  OpdIterator GetOpdIterator() const;
  SymbolicTerm& FirstOpd() const;

  bool operator == (const SymbolicExpr& that) const;
};

class ValTermVisitor : public SymbolicVisitor
{
  SymbolicTerm result;
  SymOpType expOP;
  void VisitConst( const SymbolicConst& e) 
       { 
        int val1, val2;
        if (e.GetIntVal(val1,val2))
           result = SymbolicTerm(val1, val2); 
        else
           result = SymbolicTerm(1,1,e); 
       }
  void VisitFunction( const SymbolicFunction& v)
   {
     int val1, val2;
     if (v.GetOpType() == expOP &&
         v.GetConstOpd(val1, val2)) {
         result = SymbolicTerm(val1, val2, v.GetUnknownOpds());
     }
   }
  void VisitExpr( const SymbolicExpr& v)
   {
     int val1, val2;
     if (v.GetOpType() == expOP &&
         v.GetConstOpd(val1, val2)) {
         result = SymbolicTerm(val1, val2, v.GetUnknownOpds());
     }
   }
  public:
    ValTermVisitor( SymOpType t) : expOP(t) {}
    SymbolicTerm operator()( const SymbolicVal& v)
     { result = SymbolicTerm(1,1,v);
       v.Visit(this);  return result; }
};

class OPApplicator 
{
 public:
  virtual ~OPApplicator() {}
  virtual SymOpType GetOpType() = 0;

  virtual SymbolicExpr *CreateExpr() = 0;
  virtual bool MergeConstInt(int vu1, int vd1, int vu2, int vd2, int& r1, int &r2) = 0;
  virtual bool IsTop( const SymbolicTerm& v)
   { return v.IsTop(); }
  virtual bool MergeElem(const SymbolicTerm& t1, const SymbolicTerm& t2,
                            SymbolicTerm& result)
        { result = t1;
          return result.CombineWith(t2); }
};

inline SymbolicTerm SymbolicExpr::
Val2Term(const SymbolicVal& v) const
          { return ValTermVisitor(GetTermOP())(v); }

SymbolicVal ApplyBinOP( OPApplicator& op, 
                        const SymbolicVal &_v1, const SymbolicVal &_v2);
SymbolicVal GetExprVal( SymbolicExpr *exp); 
   // delete exp if return value if different from exp
SymbolicVal ApplyUnaryOP( OPApplicator& op, 
                        const SymbolicVal &_v);
#endif
