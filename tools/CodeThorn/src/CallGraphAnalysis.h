#ifndef CALL_GRAPH_ANALYSIS_H
#define CALL_GRAPH_ANALYSIS_H 20250214

#include <sage3basic.h>
#include <tuple>
#include <Sawyer/Graph.h>
#include <Rose/FlowAnalysisInterface.h>

#include "ClassHierarchyAnalysis.h"

/*!
 * \author Peter Pirkelbauer
 * \date 2022-2024
 */

namespace CodeThorn
{
  struct CallEdge
  {
    enum Property : std::int8_t
    {
      unknown        = 0,
      normalCall     = (1 << 0),
      addressTaken   = (1 << 1),
      virtualCall    = (1 << 2),
      overrider      = (1 << 3),
      defaultValue   = normalCall | addressTaken | virtualCall | overrider,
    };

    /// tests if this has property \p prop.
    bool hasProperty(Property prop) const;

    /// returns the associated call expression.
    /// \note this is the raw call in the AST, the callee (target) may not be
    ///       the immediate lhs.
    ExpressionKeyType call() const;

    /// the expression referring to the function
    ExpressionKeyType func() const;

    ExpressionKeyType callExpr;
    ExpressionKeyType funcExpr;
    Property          kind;
  };

  inline
  CallEdge::Property
  operator|(CallEdge::Property lhs, CallEdge::Property rhs)
  {
    return static_cast<CallEdge::Property>(lhs | std::int8_t(rhs));
  }

  inline
  CallEdge::Property
  operator&(CallEdge::Property lhs, CallEdge::Property rhs)
  {
    return static_cast<CallEdge::Property>(lhs & std::int8_t(rhs));
  }

  using CGVertex  = FunctionKeyType;
  using CGEdge    = CallEdge;
  using CallGraph = Sawyer::Container::Graph<CGVertex, CGEdge, CGVertex>;

  using CallDataSequence         = std::vector<CallData>;
  using FunctionCallDataBase     = std::tuple<FunctionKeyType, CallDataSequence>;

  struct FunctionCallData : FunctionCallDataBase
  {
    using base = FunctionCallDataBase;
    using base::base;

    FunctionKeyType              function() const { return std::get<0>(*this); }
    const std::vector<CallData>& calls()    const { return std::get<1>(*this); }
  };

  using FunctionCallDataSequence = std::vector<FunctionCallData>;

  /// generates a CallGraph and call information from a ROSE project
  ///   \param  ast           the ROSE project
  ///   \param  vfa           a virtual function analysis, if overriders should be added to virtual calls
  ///   \param  withAddrTaken if set, taking an address of a function generates a callgraph entry
  ///   \return a call graph and detailed function call information.
  /// \details
  ///   calls to function that are not reachable from \p proj are ignored.
  std::tuple<CallGraph, FunctionCallDataSequence>
  generateCallGraphFromAST(ASTRootType proj, const VirtualFunctionAnalysis* vfa = nullptr, bool withAddrTaken = true);

  /// generates a CallGraph and call information for all functions in the memory pool
  ///   \param  ast           the ROSE project
  ///   \param  vfa           a virtual function analysis, if overriders should be added to virtual calls
  ///   \param  withAddrTaken if set, taking an address of a function generates a callgraph entry
  ///   \return a call graph and detailed function call information.
  std::tuple<CallGraph, FunctionCallDataSequence>
  generateCallGraphFromMemoryPool(const VirtualFunctionAnalysis* vfa = nullptr, bool withAddrTaken = true);

  // CallGraph generateCallGraphFromNormalizedAST(SgProject* proj);
} // end of namespace CodeThorn

template <>
class Rose::FlowGraphInterface::CallGraphAnalysis
      <CodeThorn::CGVertex, CodeThorn::CGEdge, CodeThorn::FunctionCallDataSequence,
Rose::FlowGraphInterface::AnalysisSupportOption::CodeThorn> {
  typedef FlowGraphAccessInterface<CodeThorn::CGVertex, CodeThorn::CGEdge> GraphAccess;

// QY: This will be working only after making Sawyer::container::Graph inherits FlowGraphAccessInterface.
// GraphAccess& getResultGraph() { return result_graph_; }
 CodeThorn::FunctionCallDataSequence&  getSavedCallData() { return call_data_; }

 bool performAnalysis(SgProject* ast) {
   std::tie(result_graph_, call_data_) = CodeThorn::generateCallGraphFromAST(ast);
   return true;
 }
 private:
   CodeThorn::CallGraph result_graph_;
   CodeThorn::FunctionCallDataSequence call_data_;
};


#endif /* CALL_GRAPH_ANALYSIS_H */
