
#include "sage3basic.h"

#include "Rose/AST/Utility.h"
#include "sageBuilder.h"
#include "Rose/Diagnostics.h"

#if !defined(ROSE_PEDANTIC_FIXUP_CONSTANT_FOLDING)
#  define ROSE_PEDANTIC_FIXUP_CONSTANT_FOLDING 0
#endif

#if !defined(ROSE_DIAGNOSE_FIXUP_CONSTANT_FOLDING)
#  define ROSE_DIAGNOSE_FIXUP_CONSTANT_FOLDING 0
#endif

#if !defined(ROSE_GRAPHVIZ_EXPRESSION_TREES)
#  define ROSE_GRAPHVIZ_EXPRESSION_TREES ROSE_DIAGNOSE_FIXUP_CONSTANT_FOLDING
#endif

#if !defined(ROSE_WARN_ORPHAN_EXPRESSIONS)
#  define ROSE_WARN_ORPHAN_EXPRESSIONS ROSE_DIAGNOSE_FIXUP_CONSTANT_FOLDING
#endif

#if !defined(ROSE_WARN_DISCONNECTED_EXPRESSIONS)
#  define ROSE_WARN_DISCONNECTED_EXPRESSIONS ROSE_DIAGNOSE_FIXUP_CONSTANT_FOLDING
#endif

// ****************************************************************************
//   Supporting function used in both cases of the constant folding handling
// ****************************************************************************

/// deletes \p node and all owned subtrees
/// \param node     the node to be deleted
/// \param safeMode if set, node types that could lead to dangling pointers
///                 are not deleted in constant folding.
/// \todo safeMode only patches over the underlying cause, where nodes
///       that continue being referenced are deleted.
/// \details
///   More info on the dangling pointer issue is described under Issue #907.
static void deleteExpressionAndOriginalExpressionTree(SgNode * node, bool safeMode = false) {
  // Skip node that have already been deleted
  if (node->get_freepointer() != AST_FileIO::IS_VALID_POINTER()) return;

  if (safeMode && isSgTypeTraitBuiltinOperator(node)) return;

  // If it is an expression delete the associated original expression tree
  SgExpression * exp = isSgExpression(node);
  if (exp != NULL && exp->get_originalExpressionTree() != NULL) {
    deleteExpressionAndOriginalExpressionTree(exp->get_originalExpressionTree(), safeMode);
  }

  // Traverse the AST and delete successors
  // Since only the traversal successors are traversed,
  // PP wonders if this leaks memory if a node also owns other AST objects
  // that are not included in the traversal.
  std::vector<SgNode*> successors = node->get_traversalSuccessorContainer();
  for (std::vector<SgNode*>::iterator n = successors.begin(); n != successors.end(); ++n) {
    if (*n != NULL ) {
      deleteExpressionAndOriginalExpressionTree(*n, safeMode);
    }
  }

  delete node;
}

struct CollectExpressionTrees : public ROSE_VisitTraversal {
  // Root of an expression trees but *parent is not set*
  //    Triggers an ASSERTION if not empty
  std::unordered_set<SgExpression *> orphans;

  // Root of an expression tree that is part of the AST
  std::unordered_set<SgExpression *> roots;

  // Root of an original expression tree (not part of the AST)
  std::unordered_set<SgExpression *> originals;

  // Root of an expression tree not reacheable from the AST (only testing successors)
  std::unordered_set<SgExpression *> disconnected;

#if ROSE_GRAPHVIZ_EXPRESSION_TREES
  // Part of an expression tree but not root (only used for graph generation)
  std::set<SgExpression *> subtrees;
#endif

  void visit (SgNode * node) {
    SgExpression * expr = isSgExpression(node);
    if (expr != NULL) {
      SgNode * parent = expr->get_parent();
      if (parent == NULL) {
        orphans.insert(expr);
        return;
      }

      SgExpression * expr_parent = isSgExpression(parent);
      if (expr_parent == NULL) {
        roots.insert(expr);
        return;
      }

      if (expr_parent->get_originalExpressionTree() == expr) {
        originals.insert(expr);
        return;
      }

      if (expr_parent->get_alternativeExpr() == expr) {
        /* nothing to do, constant folding has already been resolved */
        return;
      }

      std::vector<SgNode*> successors = expr_parent->get_traversalSuccessorContainer();
      if (std::find(successors.begin(), successors.end(), node) != successors.end()) {
#if ROSE_GRAPHVIZ_EXPRESSION_TREES
        subtrees.insert(expr);
#endif
        return;
      }

      SgTypeTraitBuiltinOperator * ttbo_parent = isSgTypeTraitBuiltinOperator(expr_parent);
      if (ttbo_parent != NULL) {
        for (auto arg: ttbo_parent->get_args()) {
          if (arg == expr) {
#if ROSE_GRAPHVIZ_EXPRESSION_TREES
            subtrees.insert(expr);
#endif
            return;
          }
        }
      }

      disconnected.insert(expr);
    }
  }

#if ROSE_GRAPHVIZ_EXPRESSION_TREES
  void toDot(std::ostream & out) {
    out << "digraph expr_trees {" << std::endl;
    for (auto expr: orphans) {
      out << "  n_" << std::hex << expr << " [label=\"orphan\n" << std::hex << expr << "\n" << expr->class_name() << "\", color=red];" << std::endl;
    }
    for (auto expr: roots) {
      out << "  n_" << std::hex << expr << " [label=\"root\n" << std::hex << expr << "\n" << expr->class_name() << "\"];" << std::endl;
    }
    for (auto child: originals) {
      auto folded = child->get_parent();
      out << "  n_" << std::hex << child << " [label=\"original\n" << std::hex << child << "\n" << child->class_name() << "\"];" << std::endl;
      out << "  n_" << std::hex << folded << " -> n_" << std::hex << child << " [constraint=false, label=\"original\", color=green];" << std::endl;
    }
    for (auto child: disconnected) {
      auto parent = child->get_parent();
      out << "  n_" << std::hex << child << " [label=\"disconnected\n" << std::hex << child << "\n" << child->class_name() << "\"];" << std::endl;
      out << "  n_" << std::hex << parent << " -> n_" << std::hex << child << " [constraint=false, color=red];" << std::endl;
    }
    for (auto child: subtrees) {
      auto parent = child->get_parent();
      out << "  n_" << std::hex << child << " [label=\"subtree\n" << std::hex << child << "\n" << child->class_name() << "\"];" << std::endl;
      out << "  n_" << std::hex << parent << " -> n_" << std::hex << child << " [constraint=true, color=blue];" << std::endl;
    }
    out << "}" << std::endl;
  }
#endif
};

inline SgExpression * get_parent_if_folded_in(SgExpression * expr) {
  SgExpression * result = isSgExpression(expr->get_parent());
  if (result != NULL && result->get_originalExpressionTree() == expr) {
    return result;
  } else {
    return NULL;
  }
}

/// EDG 7.0 constant-folds copies of empty class objects (e.g. the member initializer
/// "m_title(title)" with an empty class QCString), so the folded expression can be an
/// SgInitializer whose original expression tree is a plain expression.  An initializer slot
/// must stay an initializer, so wrap such a replacement in an SgAssignInitializer.
static SgExpression*
replacementFor(SgExpression* folded, SgExpression* child)
{
  if (isSgInitializer(folded) && !isSgInitializer(child)) {
    SgAssignInitializer* wrapper = SageBuilder::buildAssignInitializer_nfi(child, child->get_type());
    wrapper->set_parent(folded->get_parent());
    return wrapper;
  }
  child->set_parent(folded->get_parent()); // prevent replacement from creating self loop if child is direct descendant of folded
  return child;
}

/// checks if the original expression tree \p child can replace
///   the folded expression \p folded.
/// \param folded the folded expression
/// \param child  folded's child representing the original expression tree
/// \param delete_set a set of expressions that are marked for deletion.
bool
exprIsUnfoldable(SgExpression* folded, const SgExpression* child, const std::unordered_set<SgExpression *>& delete_set)
{
  bool res = delete_set.count(folded) == 0;

  // TODO 1st issue: the initializer of a variable using enum-value (`X::enum_e e = X::none`):
  //        -> the value is in the original tree of the enum-value
  //        -> this replacement create a type error in C++
  // TODO 2nd issue: `enum { s = sizeof(struct X {  } };` definition of X would not be unparsed
  //        -> not an issue if struct is anonymous
  //        -> I cannot find a correct predicate would probably need to save more info in EDG
  res &= !isSgEnumVal(folded);

  // DQ (7/23/2020): Only required now for C++11 code using EDG 6.0 and GNU 10.1 (see Cxx11_tests/test2015_02.C).
  // DQ (7/18/2020): Added support to permit Cxx11_tests/test2020_69.C to pass.
  // TV: moved that to not break the pattern
  res &= !isSgLambdaExp(child);

  // skip unfolding when ROSE public github issue #103 is detected.
  res &= !(  isSgAggregateInitializer(folded)
          && (!isSgInitializer(child))
          && isSgDesignatedInitializer(folded->get_parent())
          );
  return res;
}

void removeConstantFoldedValue(SgProject* /*project*/) {
  CollectExpressionTrees cet;
  cet.traverseMemoryPool();

#if ROSE_GRAPHVIZ_EXPRESSION_TREES
  std::ofstream out("expr_trees.dot");
  cet.toDot(out);
#endif

#if ROSE_WARN_ORPHAN_EXPRESSIONS
  if (!cet.orphans.empty()) {
    std::cerr << "# Found " << std::dec << cet.orphans.size() << " orphaned expressions: parent is not set." << std::endl;
    for (auto expr: cet.orphans) {
      std::cerr << "#  - " << std::hex << expr << " ( " << expr->class_name() << " )" << std::endl;
    }
    std::cerr << "# These nodes and all their successors will be deleted from the AST." << std::endl << std::endl;
  }
#endif

#if ROSE_WARN_DISCONNECTED_EXPRESSIONS
  if (!cet.disconnected.empty()) {
    std::cerr << "# Found " << std::dec << cet.disconnected.size() << " disconnected expressions: parent is set but expression is not in its successors." << std::endl;
    for (auto expr: cet.disconnected) {
      std::cerr << "#  - " << std::hex << expr << " ( " << expr->class_name() << " )" << std::endl;
    }
    std::cerr << "# These nodes and all their successors will be deleted from the AST." << std::endl << std::endl;
  }
#endif

  std::map<SgNode *, SgNode *> replace_map;
  std::unordered_set<SgExpression *> delete_set;
  
  delete_set.insert(cet.disconnected.begin(), cet.disconnected.end());
  delete_set.insert(cet.orphans.begin(), cet.orphans.end());

  // We cut the chain as we process it starting from the latest element.
  // Some visited links could be seen as last element of a chain that has been cut by mistake.
  std::unordered_set<SgExpression *> seen_in_ot_chain;

  for (auto child: cet.originals) {
    // Check that it is the actual original tree of a *potential* chain of substitutions
    //    Note: I am not sure chain of substitutions occur with latest version of EDG
    if (child->get_originalExpressionTree() == NULL && seen_in_ot_chain.find(child) == seen_in_ot_chain.end()) {
      seen_in_ot_chain.insert(child);

      SgExpression * folded = get_parent_if_folded_in(child);
      ROSE_ASSERT(folded != NULL);
      ROSE_ASSERT(folded != child);

      // Detach the selected expression from the chain of substitutions
      //   `folded` is the direct parent of `child` in the chain
      folded->set_originalExpressionTree(NULL);
      seen_in_ot_chain.insert(folded);

      // Traverse the chain of substitutions to find its first element
      SgExpression * tmp = get_parent_if_folded_in(folded);
      while (tmp != NULL) {
        folded = tmp;
        seen_in_ot_chain.insert(folded);
        tmp = get_parent_if_folded_in(folded);
      }

      // Mark all intermediaries to be deleted
      if (SgExpression* foldedOrig = folded->get_originalExpressionTree()) {
        delete_set.insert(foldedOrig);
        folded->set_originalExpressionTree(NULL);
      }

      bool const replace_folded_by_child = exprIsUnfoldable(folded, child, delete_set);

      if (replace_folded_by_child) {
        replace_map[folded] = replacementFor(folded, child);
        
        delete_set.insert(folded);
      } else if (!isSgTypeTraitBuiltinOperator(child)) {
        delete_set.insert(child);
      }
    }
  }

  Rose::AST::Utility::edgePointerReplacement(replace_map);

  for (auto expr: delete_set) {
    deleteExpressionAndOriginalExpressionTree(expr);
  }
}

/// replaces constant folded values with the original tree in the AST.
/// preserves the folded value as an alternative, so it can be used
/// by analysis that require or prefer constant folded values.
/// \details
///   modeled after removeConstantFoldedValue, but this
///   preserves the folded values as alternative.
void preserveConstantFoldedValue(SgProject* /*project*/) {
  CollectExpressionTrees cet;
  cet.traverseMemoryPool();

  std::map<SgNode *, SgNode *> replace_map;
  std::vector<std::pair<SgExpression*, SgExpression*> > folded_mapping;
  std::unordered_set<SgExpression *> delete_set;

  // Disconnected could be unused constant folded values.
  // e.g., SgEnumVal, for which a constant folded value exists
  //       for now it's unclear whether they need to be preserved also...
  delete_set.insert(cet.disconnected.begin(), cet.disconnected.end());

  // Orphans seem to be SgVarRefExp in template context.
  // \todo consider getting to the bottom of why they exist and why are they traversed..
  delete_set.insert(cet.orphans.begin(), cet.orphans.end());

  // We cut the chain as we process it starting from the latest element.
  // Some visited links could be seen as last element of a chain that has been cut by mistake.
  std::unordered_set<SgExpression *> seen_in_ot_chain;

  for (auto child: cet.originals) {
    // Check that it is the actual original tree of a *potential* chain of substitutions
    //    Note: I am not sure chain of substitutions occur with latest version of EDG
    // \note
    //   - child->get_originalExpressionTree() should be null in all original expression trees
    //     I almost think that this should be asserted
    //   - seen_in_ot_chain.insert(child).second is true if child has not been seen before
    if (child->get_originalExpressionTree() == NULL && seen_in_ot_chain.insert(child).second) {
      SgExpression* folded = get_parent_if_folded_in(child);
      ASSERT_not_null(folded);
      ASSERT_require(folded != child);

      // Detach the selected expression from the chain of substitutions
      //   `folded` is the direct parent of `child` in the chain
      folded->set_originalExpressionTree(NULL);
      seen_in_ot_chain.insert(folded);

      // Traverse the chain of substitutions to find its first element with
      //   the same original expression tree.
      // Delete all expressions chains and just preserve the original folded value
      SgExpression* folded_value = folded;
      SgExpression* tmp = get_parent_if_folded_in(folded);

      while (tmp) {
        folded = tmp;
        seen_in_ot_chain.insert(folded);
        tmp = get_parent_if_folded_in(folded);
      }

      // Mark all (?) intermediaries to be deleted
      if (folded->get_originalExpressionTree() != NULL) {
        folded->set_originalExpressionTree(NULL);
      }

      bool const replace_folded_by_child = exprIsUnfoldable(folded, child, delete_set);

      if (replace_folded_by_child) {
        replace_map[folded] = replacementFor(folded, child);
        folded_mapping.emplace_back(child, folded_value);

        SgExpression* tmp2 = get_parent_if_folded_in(folded_value);

        // the top most expression (folded)
        //   will be replaced with child. child gets the bottom most
        //   folded expression (folded_value) as alternative expression.
        //   we can delete any node in the sequence (folded_value, folded].
        while (tmp2) {
          delete_set.insert(tmp2);
          tmp2 = get_parent_if_folded_in(tmp2);
        }
      } else if (!isSgTypeTraitBuiltinOperator(child)) {
        delete_set.insert(child);
      }
    }
  }

  Rose::AST::Utility::edgePointerReplacement(replace_map);

  // attach the constant folded values as alternative under the original expression tree
  for (auto repl : folded_mapping) {
    SgExpression* original = repl.first;
    SgExpression* folded   = repl.second;

    ASSERT_not_null(folded);
    ASSERT_not_null(original);

    original->set_alternativeExpr(folded);
    folded->set_parent(original);

    ASSERT_require(delete_set.count(folded) == 0);
    ASSERT_require(delete_set.count(original) == 0);
  }

  // delete the no-longer needed AST nodes.
  for (auto expr: delete_set) {
    deleteExpressionAndOriginalExpressionTree(expr);
  }
}


struct RemoveOriginalExpressionTrees : public ROSE_VisitTraversal {
  void visit (SgNode* node) {
    if (SgExpression * exp = isSgExpression(node)) {
      if (SgExpression * oet = exp->get_originalExpressionTree()) {
        exp->set_originalExpressionTree(nullptr);        
        // deleting SgTypeTraitBuiltinOperator may leave dangling pointers,
        //   thus they are temporarily not deleted.
        deleteExpressionAndOriginalExpressionTree(oet, true /*keep SgTypeTraitBuiltinOperator*/);
      }
    }
  }
};

//! This removes the original expression tree from value expressions where it has been constant folded by EDG.
void resetConstantFoldedValues( SgNode* node ) {
   SgProject* const                       proj   = isSgProject(node);
   SgProject::constant_folding_enum const choice = proj ? proj->get_frontendConstantFolding()
                                                        : SgProject::e_folded_values_only;

   switch (choice)
   {
      case SgProject::e_original_expressions_only:
         {
            TimingPerformance timer("resetConstantFoldedValues (original expression tree only):");
            removeConstantFoldedValue(proj);
            break;
         }

      case SgProject::e_folded_values_only:
         {
            TimingPerformance timer("resetConstantFoldedValues (folded values only):");
            RemoveOriginalExpressionTrees astFixupTraversal;
            astFixupTraversal.traverseMemoryPool();
            break;
         }

      case SgProject::e_original_expressions_and_folded_values:
         {
            TimingPerformance timer("resetConstantFoldedValues (original expressions and folded values):");
            preserveConstantFoldedValue(proj);
            break;
         }

      default:
         {
            Rose::Diagnostics::mlog[Sawyer::Message::WARN] << "   p_frontendConstantFolding                 = skipped";
         }
   }
}
