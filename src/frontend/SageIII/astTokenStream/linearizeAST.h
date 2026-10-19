#ifndef LINEARIZE_AST_H
#define LINEARIZE_AST_H

// ROSE is a tool for building preprocessors, this file is an example preprocessor built with ROSE.
// rose.C: Example (default) ROSE Preprocessor: used for testing ROSE infrastructure

//#include "rose.h"
#include <stack>

class LinearizeInheritedAttribute
   {
     public:
       bool mustBeInherited;
          LinearizeInheritedAttribute ();
          LinearizeInheritedAttribute ( const LinearizeInheritedAttribute & X );
          LinearizeInheritedAttribute & operator= ( const LinearizeInheritedAttribute & X );
   };



class LinearizeAST : public SgTopDownProcessing<LinearizeInheritedAttribute>
   {
     private:
          //The idea is to create an inorder ordering of the nodes using a SgTopDownBottomUp
          //traversal. The idea is that every node can be viewed as a leaf node if you diregard
          //it's subtree. Using 4 nodes we can describe any such operation
          //                                 A
          //                                 |
          //                                 B
          //                                /|
          //                               C D
          // Here we will first pop C and B. then D and A. 
          // In another example
          //                                 A
          //                                 |
          //                                 B
          //                                 |
          //                                 C
          // we will first pop C and B, then A.
          //
          // PS! This operation will only be applied on expression. For all other constructs the
          // ordering of the preorder traversal is used.
          //Handle the case where the root node is part of an expression


     private:
    // DQ (3/25/2017): Remove to avoid Clang warning about unused private variable.
    // bool isRootNode;

       std::vector<SgNode*> nodes;

          // Functions required
          LinearizeInheritedAttribute evaluateInheritedAttribute (
              SgNode* astNode, 
              LinearizeInheritedAttribute inheritedAttribute );


      public:
      
       LinearizeAST();

 
          std::vector<SgNode*>
          get_linearization();
          void printVector(std::vector<SgNode*>& vec);
   };


std::vector<SgNode*>
linearize_subtree( SgNode* node );

// extern bool VERBOSE_MESSAGES_OF_WAVE;


#endif
