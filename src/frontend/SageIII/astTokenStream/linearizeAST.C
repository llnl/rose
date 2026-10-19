// ROSE is a tool for building preprocessors, this file is an example preprocessor built with ROSE.
// rose.C: Example (default) ROSE Preprocessor: used for testing ROSE infrastructure
// tps (01/14/2010) : Switching from rose.h to sage3.
#include "sage3basic.h"

// DQ (10/14/2010):  This should only be included by source files that require it.
// This fixed a reported bug which caused conflicts with autoconf macros (e.g. PACKAGE_BUGREPORT).
// Interestingly it must be at the top of the list of include files.
#include "rose_config.h"

#include "linearizeAST.h"
#include <boost/lexical_cast.hpp>

class PostSynthesizedAttribute
   {
     public:
       std::vector<SgNode*> nodes;
         PostSynthesizedAttribute()
         {
           
         };
   };


class PostAST : public SgBottomUpProcessing<PostSynthesizedAttribute>
   {
     private:


       SgNode* rootNode;
       std::vector<SgNode*> nodes;

          PostSynthesizedAttribute evaluateSynthesizedAttribute (
              SgNode* astNode,
              SubTreeSynthesizedAttributes synthesizedAttributeList )
          {
            PostSynthesizedAttribute synAttrib;

            
            if( synthesizedAttributeList.size() >2)
            {
              std::string correspondingString =astNode->class_name()+ " ";
              correspondingString+=(astNode)->unparseToString()+" ";
              correspondingString+=boost::lexical_cast<std::string>((astNode)->get_file_info()->get_filenameString());
              correspondingString+= " ";
              correspondingString+=boost::lexical_cast<std::string>((astNode)->get_file_info()->get_line());
              correspondingString+= " ";
              correspondingString+=boost::lexical_cast<std::string>((astNode)->get_file_info()->get_col());

              if(isSgConditionalExp(astNode) != NULL)
                std::cout << "Found conditional exp" << std::endl;


              std::cout << "Node with more that two synthesized attributes: " << correspondingString << std::endl;

              if(isSgExprListExp(astNode) != NULL)
                synAttrib.nodes.push_back(astNode);


              synthesizedAttributeList.size();
              for( size_t i = 0 ; i < synthesizedAttributeList.size(); i++ )
              {
                if( isSgConditionalExp(astNode) != NULL && 
                    i==1 )
                  synAttrib.nodes.push_back(astNode);

                for(size_t j = 0; j < synthesizedAttributeList[i].nodes.size(); j++)
                  synAttrib.nodes.push_back(synthesizedAttributeList[i].nodes[j]);

              }


#ifndef CXX_IS_ROSE_CODE_GENERATION
              if( isSgConditionalExp(astNode) == NULL &&
                  isSgExprListExp(astNode) == NULL )
              ROSE_ABORT();
#endif
            }
#ifndef CXX_IS_ROSE_CODE_GENERATION
           else if(synthesizedAttributeList.size()>0)
            {
              if( isSgUnaryOp(astNode) != NULL  )
              {
                SgUnaryOp* unaryOp = isSgUnaryOp(astNode);


                synAttrib.nodes = synthesizedAttributeList[0].nodes;
                if(unaryOp->get_mode() == SgUnaryOp::prefix  )
                  synAttrib.nodes.insert(synAttrib.nodes.begin(),astNode);
                else
                  synAttrib.nodes.push_back(astNode);

              }else
              {
                synAttrib.nodes = synthesizedAttributeList[0].nodes;
                synAttrib.nodes.push_back(astNode);

              }

              if( synthesizedAttributeList.size() >= 2 )
              {
                for(size_t i = 0; i < synthesizedAttributeList[1].nodes.size();
                    i++ )
                  synAttrib.nodes.push_back(synthesizedAttributeList[1].nodes[i]);

              }
            }else
              synAttrib.nodes.push_back(astNode);
#endif

            if(astNode == rootNode)
              nodes = synAttrib.nodes;
             
 
            return synAttrib;
          };

     public:

          std::vector<SgNode*> traversePreorder(SgNode* node)
          {
            rootNode = node;

            traverse(node);

            return nodes;


            
          };

   };



LinearizeInheritedAttribute::LinearizeInheritedAttribute () :  mustBeInherited(false) {};
LinearizeInheritedAttribute::LinearizeInheritedAttribute ( const LinearizeInheritedAttribute & X ) 
{
mustBeInherited=X.mustBeInherited;
};



// PL (10/19/2026): This is not ROSE-generated from the original source file!
// This is added in by hand to silence a warning about deprecated implicit operator=.
// I have done my best to match the formatting that ROSE generates.
LinearizeInheritedAttribute &
LinearizeInheritedAttribute::operator= ( const LinearizeInheritedAttribute & X ) 
{
mustBeInherited=X.mustBeInherited;
return *this;
};



LinearizeAST::LinearizeAST()
{
 
};

LinearizeInheritedAttribute
LinearizeAST::evaluateInheritedAttribute (
     SgNode* astNode,
     LinearizeInheritedAttribute inheritedAttribute )
   {

     //if(isSgLocatedNode(astNode))
     //std::cout << astNode->class_name() << " " << astNode->unparseToString() << " " << ( inheritedAttribute.mustBeInherited ? "true":"false" ) <<std::endl;

     if(inheritedAttribute.mustBeInherited == true)
       return inheritedAttribute;


//     if( (isSgLocatedNode(astNode)!=NULL) && ((isSgExprStatement(astNode) != NULL ) || (isSgScopeStatement(astNode)!=NULL)||(isSgValueExp(astNode)!=NULL)||(isSgVarRefExp(astNode)!=NULL)|| isSgVariableDeclaration(astNode)!=NULL || (isSgLocatedNode(astNode)->get_file_info()->isCompilerGenerated() == false)) ){


//       if( isSgInitializer(astNode) == NULL && (isSgExpression(astNode) != NULL | isSgExprStatement(astNode) != NULL )) 
     if( isSgBinaryOp(astNode) != NULL  || isSgConditionalExp(astNode) != NULL || isSgExprListExp(astNode) != NULL )
       {
                 //This is needed when the root node that is given the traversal is part of an expression
            inheritedAttribute.mustBeInherited=true;
            PostAST postAST;
            std::vector<SgNode*> inSubTree = postAST.traversePreorder(astNode);
            for(size_t i = 0 ; i < inSubTree.size(); i++)
              nodes.push_back(inSubTree[i]);

         }else{
           nodes.push_back(astNode);
         }

  //   }else
    //       nodes.push_back(astNode);


     //Do not register Binary ops
     
     return inheritedAttribute;
   }




void LinearizeAST::printVector(std::vector<SgNode*>& vec){
 
     for(std::vector<SgNode*>::iterator it_beg = vec.begin();
                     it_beg != vec.end(); ++it_beg){
           std::cout << (*it_beg)->class_name() << std::endl;
     };   
};

std::vector<SgNode*>
LinearizeAST::get_linearization(){

     return nodes; 
}

std::vector<SgNode*>
linearize_subtree( SgNode* node )
   {
  // Build the inherited attribute
     LinearizeInheritedAttribute inheritedAttribute;
     LinearizeAST myTraversal;

  // Call the traversal starting at the sageProject node of the AST
     myTraversal.traverse(node,inheritedAttribute);
  // if(VERBOSE_MESSAGES_OF_WAVE==true)
  //      myTraversal.printVector(myTraversal.inorder);
     return myTraversal.get_linearization();
   }



