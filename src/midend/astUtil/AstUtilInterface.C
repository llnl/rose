#include "sage3basic.h"
#include "AstUtilInterface.h"
#include "StmtInfoCollect.h"
#include "AstInterface.h"
#include "AstInterface_ROSE.h"
#include "annotation/OperatorAnnotation.h"
#include "dependenceTracking/dependence_analysis.h"
#include "CommandOptions.h"
#include "SymbolicVal.h"

DebugLog DebugAstUtil("-debugastutil");

namespace AstUtilInterface {

static bool do_annot = false;

class VariableSignatureDictionary {
private:
  static DependenceTable* dict;
  static bool do_save;
  
public:
  static void set_doit(bool doit) {
    if (doit && dict == 0) {
       dict = new DependenceTable();
    }
    do_save = doit;
  }

  static bool do_it() { return do_save; }
  static DependenceTable* get_dictionary() {
    return dict;
  } 
};
DependenceTable* VariableSignatureDictionary::dict = 0;
bool VariableSignatureDictionary::do_save = false;

};

void AstUtilInterface::SetSaveVariableDictionary(bool doit) {
  VariableSignatureDictionary::set_doit(doit);
}


void AstUtilInterface::ComputeAstSideEffects(SgNode* ast, 
              std::function<bool(const AstNodePtr&, const AstNodePtr&, const AstUtilInterface::OperatorSideEffect&)>* collect,
              SaveOperatorSideEffectInterface* add_to_dep_analysis) {
    AstInterfaceImpl astImpl(ast);
    AstInterface fa(&astImpl);

    OperatorSideEffectAnnotation* funcAnnot=OperatorSideEffectAnnotation::get_inst();
    assert(funcAnnot != 0);
    DebugAstUtil([&ast](){ return "ComputeAstSideEffect: " + AstInterface::AstToString(ast); });
    // Should we add annotation? Have we added any annotation? (done_annot)
    bool is_function = false, done_annot_mod=false, done_annot_read = false, done_annot_call=false;
    AstInterface::AstNodePtr body;
    AstInterface::AstNodeList ast_params;
    AstInterface::AstTypeList ast_param_types;
    if (AstInterface::IsFunctionDefinition(ast, 0, &ast_params, 0, &body, &ast_param_types, 0, /*use_global_uniqu_name*/false, /*skip_pure_decl*/true) && body != AST_NULL) {
      // Add empty annotations for this function. Details of the side effects will be added later
      // while the body of the function is being analyzed.
      DebugAstUtil([&ast](){ return "Saving side effects for :" + AstInterface::AstToString(ast) + "\n"; });
      is_function = true;
      if (add_to_dep_analysis != 0) {
         add_to_dep_analysis->ClearOperatorSideEffect(ast);
         assert(ast_params.size() == ast_param_types.size());
         auto pt = ast_param_types.begin();
         for (const auto& p : ast_params) {
            add_to_dep_analysis->SaveOperatorSideEffect(ast,p, OperatorSideEffect(OperatorSideEffect::EnumVariant::Parameter, (*pt).get_ptr()));
            pt++;
         }
      }
    }

    StmtSideEffectCollect collect_operator(fa, funcAnnot);
    std::map<std::string, AstNodePtr > alias_map; 
    std::function<bool(const SideEffectAnalysisInterface::SideEffectInfo&)> save_alias = [&collect, &alias_map] 
        (const SideEffectAnalysisInterface::SideEffectInfo& info) {
      DebugAstUtil([&info](){ return "save alias:" + AstInterface::GetVariableSignature(info.first_) + "->" + AstInterface::GetVariableSignature(info.second_); });
      auto sig_first = AstInterface::GetVariableSignature(info.first_);
      AstNodePtr base;
      if (AstInterface::IsAddressOfOp(info.second_, &base)) {
          alias_map["_deref_(" + sig_first + ")"] = base;
      }
      alias_map[sig_first] = info.second_;
      if (collect != 0) {
         (*collect)(info.first_, info.second_, OperatorSideEffect(OperatorSideEffect::EnumVariant::Alias, info.third_.get_ptr()));
      }
      return true;
    };
    auto save_memory_ref = [&alias_map, &is_function, &collect, &ast, &body, add_to_dep_analysis] (AstNodePtr ref, AstNodePtr src, OperatorSideEffect what) {
      bool done_annot = false;
      if (!ref.is_unknown() && !AstInterface::IsMemoryAccess(ref)) {
          DebugAstUtil([&ref](){ return "Do not save non-memory-access ref:" + AstInterface::AstToString(ref); });
          return false;
      }
      bool is_unknown_ref = false;
      bool is_local_ref = AstInterface::IsLocalRef(ref, body, &is_unknown_ref);
      {
        AstNodePtr array;
        // No need to check local ref if annotation is not needed. 
        if (AstInterface::IsArrayAccess(ref, &array)) {
           is_local_ref = false;
           DebugAstUtil([&ref](){ return "Finding array reference:" + AstInterface::AstToString(ref); });
        }
      }
      {
        auto ref_aliased = alias_map.find(AstInterface::GetVariableSignature(ref));
        DebugAstUtil([&ref](){ return "Looking for aliased reference:" + AstInterface::GetVariableSignature(ref); });
        if (ref_aliased != alias_map.end()) {
           ref = AstNodePtr((*ref_aliased).second);
           is_local_ref = false;
           DebugAstUtil([&ref](){ return "Finding aliased reference:" + AstInterface::AstToString(ref); });
        } else {
           if (is_unknown_ref) {
              ref.set_is_unknown_reference();
              what.set_is_unknown(true);
           }
           DebugAstUtil([&ref](){ return "Did not find aliased reference:" + AstInterface::AstToString(ref); });
        }
      }
      if (collect != 0) (*collect)(ref, src, what);
      if (is_function && (ref.is_unknown() || !is_local_ref)) {
           DebugAstUtil([&ref](){ return "save non-local:" + AstInterface::AstToString(ref); });
           if (add_to_dep_analysis != 0) {
              add_to_dep_analysis->SaveOperatorSideEffect(ast, ref, what); 
           } 
           if (do_annot) {
              AddOperatorSideEffectAnnotation(ast, ref, what);
           }
           done_annot = true; /* done annotations */
      }
      return done_annot; /* done annotation? */
    };
    std::function<bool(const SideEffectAnalysisInterface::SideEffectInfo&)> save_mod = [&done_annot_mod,&save_memory_ref] (const SideEffectAnalysisInterface::SideEffectInfo& info) { 
       if (save_memory_ref(info.first_, info.second_, OperatorSideEffect(OperatorSideEffect::EnumVariant::Modify, info.third_.get_ptr(), info.first_.is_unknown()))) { 
          DebugAstUtil([](){ return "Done mod annotation."; });
          done_annot_mod = true;
       }
       return true;
    };
    std::function<bool(const SideEffectAnalysisInterface::SideEffectInfo&)> save_read = [&save_memory_ref,&done_annot_read] (const SideEffectAnalysisInterface::SideEffectInfo& info) {
      if (save_memory_ref(info.first_, info.second_, OperatorSideEffect(OperatorSideEffect::EnumVariant::Read, info.third_.get_ptr(), info.first_.is_unknown()))) {
         DebugAstUtil([](){ return "Done read annotation."; });
         done_annot_read = true;
      }
      return true;
    };
    std::function<bool(const SideEffectAnalysisInterface::SideEffectInfo&)> save_kill = [&collect] (const SideEffectAnalysisInterface::SideEffectInfo& info) {
      if (collect != 0) return (*collect)(info.first_, info.second_, OperatorSideEffect(OperatorSideEffect::EnumVariant::Kill, info.second_.get_ptr(), info.first_.is_unknown()));
      return true;
    };
    std::function<bool(const SideEffectAnalysisInterface::SideEffectInfo&)> save_call = [&collect,&ast, &is_function, &done_annot_call, &done_annot_mod, &done_annot_read, &body, add_to_dep_analysis] (const SideEffectAnalysisInterface::SideEffectInfo& info) {
      if (is_function && !AstInterface::IsLocalRef(info.first_, body)) {
         done_annot_call = true;
         OperatorSideEffect relation(OperatorSideEffect::EnumVariant::Call, info.second_.get_ptr(), info.first_.is_unknown());
         if (add_to_dep_analysis != 0) {
            add_to_dep_analysis->SaveOperatorSideEffect(ast, GetVariableSignature(info.first_), relation); 
         } 
         if (do_annot)  AddOperatorSideEffectAnnotation(ast, info.first_, relation); 
         if (info.first_.is_unknown()) {
            done_annot_call = true;
            done_annot_mod = true;
            done_annot_read = true;
         }
      }
      DebugAstUtil([&info](){ return "save call:" + AstInterface::AstToString(info.first_); });
      if (collect != 0)  (*collect)(info.first_, info.second_,OperatorSideEffect(OperatorSideEffect::EnumVariant::Call, info.third_.get_ptr()));
      return true;
    };
    std::function<bool(const SideEffectAnalysisInterface::SideEffectInfo&)> save_decl = [&collect] (const SideEffectAnalysisInterface::SideEffectInfo& info) {
      auto var = info.first_, init=info.second_, desig=info.third_;
      DebugAstUtil([&var,&init,&desig](){ return "save new decl:" + AstInterface::AstToString(var) + ":" + AstInterface::AstToString(init) + ":" + AstInterface::AstToString(desig); });
      if (collect != 0) {
         (*collect)(var, init, OperatorSideEffect(OperatorSideEffect::EnumVariant::Decl, desig.get_ptr()));
      }
      return true;
    };
    std::function<bool(const SideEffectAnalysisInterface::SideEffectInfo&)> save_allocate = [&collect,&save_call] (const SideEffectAnalysisInterface::SideEffectInfo& info) {
      auto op = info.first_, init=info.second_, details=info.third_;
      DebugAstUtil([&op,&init](){ return "save allocate:" + AstInterface::AstToString(op) + ":" + AstInterface::AstToString(init); });
      if (collect != 0) return (*collect)(op, init, OperatorSideEffect(OperatorSideEffect::EnumVariant::Allocate, details.get_ptr()));
      save_call(info);
      return true;
    };
    std::function<bool(const SideEffectAnalysisInterface::SideEffectInfo&)> save_free = [&collect,&save_mod,&save_call] (const SideEffectAnalysisInterface::SideEffectInfo& info) {
      DebugAstUtil([&info](){ return "save free:" + AstInterface::AstToString(info.first_); });
      if (collect != 0) { 
           (*collect)(info.first_, info.second_, OperatorSideEffect(OperatorSideEffect::EnumVariant::Free, info.third_.get_ptr()));
      }
      save_mod(info); // a free is also a modify.
      save_call(info);
      return true;
    };
    collect_operator.set_modify_collect(save_mod);
    collect_operator.set_alias_collect(save_alias);
    collect_operator.set_read_collect(save_read);
    collect_operator.set_kill_collect(save_kill);
    collect_operator.set_call_collect(save_call);
    collect_operator.set_new_var_collect(save_decl);
    collect_operator.set_allocate_collect(save_allocate);
    collect_operator.set_free_collect(save_free);
    collect_operator(ast);
    if (is_function) {
    // Add empty annot if no annotations have been inserted.
      if (!done_annot_mod) {
          if (add_to_dep_analysis != 0)
            add_to_dep_analysis->SaveOperatorSideEffect(ast, AST_NULL, OperatorSideEffect(OperatorSideEffect::EnumVariant::Modify, 0)) ;
         if (do_annot)
            AddOperatorSideEffectAnnotation(ast, AST_NULL, OperatorSideEffect(OperatorSideEffect::EnumVariant::Modify,0));
      }
      if (!done_annot_read) {
          if (add_to_dep_analysis != 0)
            add_to_dep_analysis->SaveOperatorSideEffect(ast, AST_NULL, OperatorSideEffect(OperatorSideEffect::EnumVariant::Read, 0)) ;
          if (do_annot)
            AddOperatorSideEffectAnnotation(ast, AST_NULL, OperatorSideEffect(OperatorSideEffect::EnumVariant::Read,0));
      }
      if (!done_annot_call) {
          if (add_to_dep_analysis != 0)
             add_to_dep_analysis->SaveOperatorSideEffect(ast, AST_NULL, OperatorSideEffect(OperatorSideEffect::EnumVariant::Call, 0)) ;
          if (do_annot)
             AddOperatorSideEffectAnnotation(ast, AST_NULL, OperatorSideEffect(OperatorSideEffect::EnumVariant::Call, 0));
      } 
   }
}

void AstUtilInterface::ReadAnnotations(std::istream& input, DependenceTable* use_dep_analysis) {
  if (use_dep_analysis != 0) {
     CollectDependences collect(/*update_annotation=*/true);
     collect.CollectFromFile(input, *use_dep_analysis);
  }
  else {
     ReadAnnotation::get_inst()->read(input);
  }
}

void AstUtilInterface::OutputOperatorSideEffectAnnotations(std::ostream& output, DependenceTable* use_dep_analysis) {
  if (use_dep_analysis != 0) {
     use_dep_analysis->OutputDependences(output);
  }
  else {
    OperatorSideEffectAnnotation* funcAnnot=OperatorSideEffectAnnotation::get_inst();
    funcAnnot->write(output);
  }
}

void AstUtilInterface::RegisterOperatorSideEffectAnnotation() {
  OperatorSideEffectAnnotation* funcAnnot=OperatorSideEffectAnnotation::get_inst();
  funcAnnot->register_annot();
//  do_annot = true;
};

void AstUtilInterface::AddOperatorSideEffectAnnotation(
              SgNode* op_ast, const AstNodePtr& var, 
              const AstUtilInterface::OperatorSideEffect& relation)
{
  DebugAstUtil([&var](){ return "Adding operator annotation: " + AstInterface::AstToString(var); });
  if (!AstInterface::IsFunctionDefinition(op_ast)) {
     DebugAstUtil([&op_ast](){ return "Expecting an operator but getting " + AstInterface::AstToString(op_ast);});
     return;
  }
  AstInterfaceImpl astImpl(op_ast);
  AstInterface fa(&astImpl);

  OperatorSideEffectAnnotation* funcAnnot=OperatorSideEffectAnnotation::get_inst();
  OperatorSideEffectDescriptor *desc = 0;
  switch (relation.get_enum()) {
     case OperatorSideEffect::EnumVariant::Modify:
         desc = funcAnnot->get_modify_descriptor(fa, op_ast, true);
          break;
     case OperatorSideEffect::EnumVariant::Read:
        desc = funcAnnot->get_read_descriptor(fa, op_ast, true);
          break;
     case OperatorSideEffect::EnumVariant::Call:
        desc = funcAnnot->get_call_descriptor(fa, op_ast, true);
          break;
     case OperatorSideEffect::EnumVariant::Kill:
     case OperatorSideEffect::EnumVariant::Decl:
     case OperatorSideEffect::EnumVariant::Allocate:
     case OperatorSideEffect::EnumVariant::Free:
          break;
     default: 
        std::cerr << "Unexpected relation: " << (int)relation.get_enum() << "\n";
        assert(0);
  }
  if (desc != 0 && !var.is_null()) {
     if (var.is_unknown() || var.is_unknown_reference() || var.is_unknown_function_call()) {
       DebugAstUtil([](){ return "Setting UNKNOWN.\n"; });
       desc->set_has_unknown(true);  
     } 
     std::string varname = GetVariableSignature(var);
     SymbolicValDescriptor val_desc(SymbolicValGenerator::GetSymbolicVal(fa, var), varname);
     desc->push_back(val_desc);
     DebugAstUtil([desc](){ return "Done adding operator annotation: " + desc->ToString(); });
  }
} 


bool AstUtilInterface::IsLocalRef(SgNode* ref, SgNode* scope, bool* has_ptr_deref) {
   if (ref == 0 || scope == 0) 
      return false;
   return AstInterface::IsLocalRef(ref, scope, has_ptr_deref);
}

std::string AstUtilInterface::GetVariableSignature(const AstNodePtr&  variable) {
  auto sig = AstInterface::GetVariableSignature(variable);
  auto* dict_table = VariableSignatureDictionary::get_dictionary();
  if (VariableSignatureDictionary::do_it() && 
      dict_table != 0 && variable.get_ptr() != 0) {
     std::string filename;
     int lineno = -1; 
     if (AstInterface::get_fileInfo(variable, &filename, &lineno)) {
        std::vector<std::string> deptype;
        std::stringstream loc;
        loc << "line=" << lineno;
        deptype.push_back(loc.str());
        AstNodeType exptype;
        if (AstInterface::IsExpression(variable, &exptype) && exptype != AST_NULL_TYPE) {
          std::stringstream t; t << "type=" << AstInterface::GetTypeName(exptype); 
          deptype.push_back(t.str());
        }
        DependenceEntry e(sig, filename, deptype);
        dict_table->SaveDependence(e);
     }
  }
  return sig;
}

void AstUtilInterface::OutputSignatureDictionary(std::ostream& output) {
  auto* dict_table = VariableSignatureDictionary::get_dictionary();
  if (dict_table != 0) {
       dict_table->OutputDependences(output); 
  }
}


void AstUtilInterface::SetFunctionNameMangling(std::string (*f)(const SgFunctionDeclaration*)) {
  AstInterface::SetFunctionNameMangling(f);
}
