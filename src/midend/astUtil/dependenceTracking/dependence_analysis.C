#include "sage3basic.h"

#include <sstream>
#include <string>

#include "dependence_analysis.h"

#include "CommandOptions.h"
#include "AstInterface.h"
#include "OperatorDescriptors.h"
#include "OperatorAnnotation.h"
#include "AstUtilInterface.h"

namespace AstUtilInterface {

std::string CollectDependences::local_read_string(std::istream& input_file) {
    std::string next_string;
    char c = 0;
    while ((input_file >> c).good()) {
       switch (c) {
       case '\\':
          next_string.push_back(c);
          if (!(input_file >> c).good()) 
              return next_string;  
          break; 
       case '\"': 
          next_string.push_back(c);
          while ((input_file >> c).good() && c != '\"') {
             next_string.push_back(c);
          }
          next_string.push_back(c);
          Log.push("reading quoted string " + next_string);
          return next_string;
       case ' ':
       case '\n':
       case '\r':
            if (next_string != "") {
              // This starts a new token. Return the current one.
              Log.push("Seeing separator. Finished reading token " + next_string);
              return next_string;
            } 
            // Skip empty space.
            break;
       case ':': {
          // Make sure read double "::" as part of a name.
          char c1 = input_file.peek();
          if (c1 == ':') {
             input_file >> c1;
             next_string += "::";
             Log.push("Seeing `::'. continue reading token " + next_string);
          } else {
            if (next_string != "") { 
               input_file.putback(c);
               Log.push("Seeing separator. Finished reading token " + next_string);
            } else {
               next_string.push_back(c);
            }
            return next_string;
         }
         break;
         }
       case '=':
          if (next_string.size()>8 && 
                 (next_string.substr(next_string.size()-8,8) == "operator" || next_string.substr(next_string.size()-9,8) == "operator")) {
            next_string.push_back(c);
            break;
          }
          [[fallthrough]]; // Explicitly indicates intentional fall-through 
       case '[':
       case ']':
       case '{':
       case '}':
       case ';':
            if (next_string == "") {
             // Found a token. Return it.
             next_string.push_back(c);
             Log.push("reading separator token " + next_string);
            } 
            else {
              // This starts a new token. Return the current one.
              input_file.putback(c);
              Log.push("Seeing separator. Finished reading token " + next_string);
            }
            return next_string;
       default: 
             next_string.push_back(c);
             break;
      }
    }
    Log.push("Return Next token  " + next_string);
    return next_string;
}

void CollectDependences::CollectFromFile(std::istream& input_file, DependenceTable& main_table) {
    Log.push("Collect past results of dependences");
    while (input_file.good()) {
        // Each line starts with the name of a component in the software.
        std::string dest = local_read_string(input_file);
        if (dest == "")  break;
        if (dest == "}" || dest == ";") continue;
        Log.push("Destination name: " + dest);
        std::string next_string, source;
        std::vector<std::string> dep_types;
        // Read and process all the components that `dest' depends on immediately.
        while ((next_string = local_read_string(input_file)) != "") {
             if (next_string == ";") {
               if (source != "") {
                  DependenceEntry e(dest, source, dep_types);
                  main_table.SaveDependence(e);
                  if (update_annotations_) save_annotation(e);
                  Log.push( "Saving " + e.to_string());
               } else {
               }
               Log.push("Done reading line\n");
               break;
            } else if (next_string == "->" || next_string == ":") {
               bool reverse = (next_string == "->");
               source = local_read_string(input_file); 
               while (source == "[") {
                  source = "";
                  while ((next_string = local_read_string(input_file)) != "]") {
                    if (next_string == "") {
                       Log.fatal("Expecting \"]\" but get " + next_string);
                    }
                    source += next_string; 
                  }
                  dep_types.push_back(source);
                  source = local_read_string(input_file); 
               }
               if (reverse) {
                  std::swap(source, dest);
               }
               if (source == ";") {
                  Log.push("Createing Node with attribute for " + dest);
                  main_table.InsertNode(dest); 
                  for (auto& e : dep_types) {
                     main_table.get_nodeInfo(dest).push_back(e);
                  }
                  next_string = ";"; source = "";
                  break; 
               } 
               Log.push("Successfully setting source = " + source);
            } else if (next_string == "{") {
               Log.push("Skipping graph configuration: " + dest + " " + next_string);
               break;
            } else {
               Log.fatal("Unexpected token " + next_string);
            }
        }
        if (next_string != ";" && next_string != "}" && next_string != "{") {
            Log.fatal("Expecting `;' or `}' but getting " + next_string);
        } else if (input_file.peek() == EOF) {
            break;
        }
    }
    Log.push("Done collecting past results of dependences");
}

template <class NodeIterator, class EdgeIterator>
bool WholeProgramDependenceAnalysis<NodeIterator,EdgeIterator>:: ComputeDependences() {
  Log.push("Compute dependences.");
  if (sageProject == 0) {
    return false;
  }
  int filenum = sageProject->numberOfFiles();
  for (int i = 0; i < filenum; ++i) {
    SgSourceFile* sageFile = isSgSourceFile(sageProject->get_fileList()[i]);
    ROSE_ASSERT(sageFile != NULL);

    std::string fname = sageFile->get_file_info()->get_raw_filename();
    Log.push("Targeting file:"+fname);

    SgGlobal *root = sageFile->get_globalScope();
    ROSE_ASSERT(root != NULL);

    SgDeclarationStatementPtrList declList = root->get_declarations ();
    for (SgDeclarationStatementPtrList::iterator p = declList.begin(); p != declList.end(); ++p)
    {
      SgNode* func = *p;
      if (func == 0) continue;
      Log.push("Analyzing declaration " + func->unparseToString() + " in " + fname);
      ComputeDependences(func, root);
    }
  }
  return true;
}

template <class NodeIterator, class EdgeIterator>
void WholeProgramDependenceAnalysis<NodeIterator,EdgeIterator>::ComputeDependences(SgNode* input, SgNode* root) {
  DebugLog DebugSaveDep("-debugdep");
  std::string function_name;
  AstInterface::AstNodeList children;
  std::function<bool(const AstNodePtr&, const AstNodePtr&, const AstUtilInterface::OperatorSideEffect&)> save_function_dep = 
        [this,input] (const AstNodePtr& first, const AstNodePtr&, const AstUtilInterface::OperatorSideEffect& relation) {
        assert(main_table != 0);
        auto first_sig = AstInterface::GetVariableSignature(first);
        if (first_sig != "") {
           main_table->addEdge(main_table->addNode(input), main_table->addNode(first.get_ptr()), relation);
        }
        return true;
  };
  std::function<bool(const AstNodePtr&, const AstNodePtr&, const AstUtilInterface::OperatorSideEffect&)> save_variable_dep = 
        [this,input] (const AstNodePtr& first, const AstNodePtr& second, const AstUtilInterface::OperatorSideEffect& third) {
        assert(main_table != 0);
        if (second != AST_NULL) {
           auto first_sig = AstInterface::GetVariableSignature(first);
           auto init = AstInterface::GetVariableSignature(second);
           auto desig = third.get_details();
           if (first_sig != "" && init.find("_UNKNOWN_") == std::string::npos)  {
             main_table->addEdge(main_table->addNode(first.get_ptr()), main_table->addNode(second.get_ptr()), AstUtilInterface::OperatorSideEffect::EnumVariant::Init);
           }
           if (desig != 0) {
             main_table->addEdge(main_table->addNode(desig), main_table->addNode(second.get_ptr()), AstUtilInterface::OperatorSideEffect::EnumVariant::Init);
           }
        }
        return true;
  };
  if (AstInterface::IsBlock(input, &function_name, &children)) {
    if (!function_name.empty()) {
       Log.push("Computing dependences for " + function_name);
       AstUtilInterface::ComputeAstSideEffects(input, &save_function_dep, annot_table);
    }
    for (AstInterface::AstNodeList::const_iterator p = children.begin(); p != children.end(); ++p) {
      AstNodePtr current = *p;
      ComputeDependences(current.get_ptr(), root);
    }
  } else if (AstInterface::IsVariableDecl(input)) {
     AstUtilInterface::ComputeAstSideEffects(input, &save_variable_dep, annot_table);
  }
}
void CollectDependences:: save_annotation(const DependenceEntry& e) {
     DebugLog DebugSaveDep("-debugdep");
     DebugSaveDep([&e](){ return "saving annotation: " + e.to_string(); });

    // Save into annotation  if necessary.
    if (e.type_entry() == "parameter") {
      OperatorSideEffectAnnotation* funcAnnot = OperatorSideEffectAnnotation::get_inst();
      OperatorSideEffectDescriptor* desc1 = funcAnnot->get_modify_descriptor(e.first_entry(), true);
      assert(desc1 != 0);
      desc1->get_param_decl().add_param( /*param type*/ e.attr_entry(),  /* param name*/ e.second_entry());
      OperatorSideEffectDescriptor* desc2 = funcAnnot->get_read_descriptor(e.first_entry(), true);
      assert(desc2 != 0);
      desc2->get_param_decl().add_param( /*param type*/ e.attr_entry(),  /* param name*/ e.second_entry());
      DebugSaveDep([&e](){ return "Saving parameter " + e.second_entry(); });
    }
    else if (e.type_entry() == "modify") {
      OperatorSideEffectAnnotation* funcAnnot = OperatorSideEffectAnnotation::get_inst();
      OperatorSideEffectDescriptor* desc = funcAnnot->get_modify_descriptor(e.first_entry(), true);
      assert(desc != 0);
      DebugSaveDep([&e](){ return "processing " + e.second_entry(); });
      SymbolicVal var = SymbolicValGenerator::GetSymbolicVal(e.second_entry());
      desc->push_back(var);
      DebugSaveDep([&var](){ return "Saving modify " + var.toString(); });
    } else if (e.type_entry() == "read") {
      OperatorSideEffectAnnotation* funcAnnot = OperatorSideEffectAnnotation::get_inst();
      OperatorSideEffectDescriptor* desc = funcAnnot->get_read_descriptor(e.first_entry(), true);
      assert(desc != 0);
      SymbolicVal var = SymbolicValGenerator::GetSymbolicVal(e.second_entry());
      desc->push_back(var);
      DebugSaveDep([&var](){ return "Saving read " + var.toString(); });
    }
}


template class WholeProgramDependenceAnalysis<std::string, DependenceEntry>;


};

