
#include <iostream>

#include "sage3basic.h"
#include "clang-to-dot-private.hpp"
#include "rose_config.h"

#if 0
// DQ (4/5/2017): nothing works since we need the version of Clang/LLVM that we are using to be3 compilied without the "-fno-rtti" option.
// DQ (3/1/2017): Trying to get rid of linker error.
#ifdef ROSE_USE_CLANG_FRONTEND
#include "typeinfo"
// clang::ASTConsumer::typeinfo() {};
// clang::PPCallbacks::typeinfo() {};
// extern "C" {
#if 0
namespace clang {
  namespace ASTConsumer {
     typeinfo() {};
  }
  namespace PPCallbacks {
     typeinfo() {};
  }
}
#endif

void clang::ASTConsumer::type_info() {};
void clang::PPCallbacks::type_info() {};

// }
#endif
#endif

using namespace Sawyer::Message;

Rose::Diagnostics::Facility ClangToDotTranslator::logger;
Rose::Diagnostics::Facility ClangToDotPreprocessorRecord::logger;


extern bool roseInstallPrefix(std::string&);

// DQ (11/1/2020): Added to resolve types (e.g string)
using namespace std;

// File for output for generated graph.
std::ofstream CLANG_ROSE_Graph::file;

// DQ (11/1/2020): Added to resolve namespace variables.
using namespace CLANG_ROSE_Graph;

// DQ (10/23/2020): We need a list to make sure that we don't add nodes twice (once when traversing the source sequence lists and a second time when traversing the AST).
// We need to show the AST plus the edges that are specific to the source sequence lists (and which node's source sequence list they are associated with).
std::set<void*> graphNodeSet;

// #define DEBUG_CLANG_DOT_GRAPH_SUPPPORT 0
// #define DEBUG_HEADER_GRAPH_SUPPPORT 0


// int clang_to_dot_main(int argc, char ** argv, SgSourceFile& sageFile) 
int clang_to_dot_main(int argc, char ** argv) 
   {
     Rose::Diagnostics::mprefix->showProgramName(false);
     Rose::Diagnostics::mprefix->showThreadId(false);
     Rose::Diagnostics::mprefix->showElapsedTime(false);
  // DQ (9/6/2013): Build a dot graph of the EDG AST.

  // Build filename...
     string filename = "clangGraph";

     string dot_header = filename;
     filename += ".dot";

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     ::mlog[DEBUG] << "In clang_to_dot_main(): filename " << filename.c_str() << "\n";
#endif

  // Open file...(file is declared in the CLANG_ROSE_Graph namespace).
     file.open(filename.c_str());

  // 0 - Analyse Cmd Line

    std::vector<std::string> inc_dirs_list;
    std::vector<std::string> define_list;
    std::vector<std::string> inc_list;
    std::string input_file;

    for (int i = 0; i < argc; i++) {
        std::string current_arg(argv[i]);
        if (current_arg.find("-I") == 0) {
            if (current_arg.length() > 2) {
                inc_dirs_list.push_back(current_arg.substr(2));
            }
            else {
                i++;
                if (i < argc)
                    inc_dirs_list.push_back(current_arg);
                else
                    break;
            }
        }
        else if (current_arg.find("-D") == 0) {
            if (current_arg.length() > 2) {
                define_list.push_back(current_arg.substr(2));
            }
            else {
                i++;
                if (i < argc)
                    define_list.push_back(current_arg);
                else
                    break;
            }
        }
        else if (current_arg.find("-c") == 0) {}
        else if (current_arg.find("-o") == 0) {
            if (current_arg.length() == 2) {
                i++;
                if (i >= argc) break;
            }
        }
        else {
            // TODO -include
#if DEBUG_ARGS
            ::mlog[DEBUG] << "argv[" << i << "] = " << current_arg << " is neither define or include dir. Use it as input file."  << "\n";
#endif
            input_file = current_arg;
        }
    }

    ClangToDotTranslator::Language language = ClangToDotTranslator::unknown;

    size_t last_period = input_file.find_last_of(".");
    std::string extention(input_file.substr(last_period + 1));

    if (extention == "c") {
        language = ClangToDotTranslator::C;
    }
    else if (extention == "C" || extention == "cxx" || extention == "cpp" || extention == "cc") {
        language = ClangToDotTranslator::CPLUSPLUS;
    }
    else if (extention == "objc") {
        language = ClangToDotTranslator::OBJC;
    }
    else if (extention == "cu") {
        language = ClangToDotTranslator::CUDA;
    }
    else if (extention == "ocl" || extention == "cl") {
        language = ClangToDotTranslator::OPENCL;
    }

    ROSE_ASSERT(language != ClangToDotTranslator::unknown);

    const char * cxx_config_include_dirs_array [] = CXX_INCLUDE_STRING;
    const char * c_config_include_dirs_array   [] = C_INCLUDE_STRING;

    std::vector<std::string> cxx_config_include_dirs (
                                                       cxx_config_include_dirs_array,
                                                       cxx_config_include_dirs_array + sizeof(cxx_config_include_dirs_array) / sizeof(const char*)
                                                     );
    std::vector<std::string> c_config_include_dirs   (
                                                       c_config_include_dirs_array,
                                                       c_config_include_dirs_array + sizeof(c_config_include_dirs_array) / sizeof(const char*)
                                                     );

    std::string rose_include_path;
    bool in_install_tree = roseInstallPrefix(rose_include_path);
    if (in_install_tree) {
        rose_include_path += "/include/";
    }
    else {
        rose_include_path = std::string(ROSE_AUTOMAKE_TOP_BUILDDIR) + "/include-staging/";
    }

    std::vector<std::string>::iterator it;
    for (it = c_config_include_dirs.begin(); it != c_config_include_dirs.end(); it++)
        if (it->length() > 0 && (*it)[0] != '/')
            *it = rose_include_path + *it;
    for (it = cxx_config_include_dirs.begin(); it != cxx_config_include_dirs.end(); it++)
        if (it->length() > 0 && (*it)[0] != '/')
            *it = rose_include_path + *it;

    inc_dirs_list.push_back(rose_include_path + "clang/");


    // FIXME add ROSE path to gcc headers...
    switch (language) {
        case ClangToDotTranslator::C:
            inc_dirs_list.insert(inc_dirs_list.begin(), c_config_include_dirs.begin(), c_config_include_dirs.end());
            inc_list.push_back("clang-builtin-c.h");
            break;
        case ClangToDotTranslator::CPLUSPLUS:
            inc_dirs_list.insert(inc_dirs_list.begin(), cxx_config_include_dirs.begin(), cxx_config_include_dirs.end());
            inc_list.push_back("clang-builtin-cpp.hpp");
            break;
        case ClangToDotTranslator::CUDA:
            inc_dirs_list.insert(inc_dirs_list.begin(), cxx_config_include_dirs.begin(), cxx_config_include_dirs.end());
            inc_list.push_back("clang-builtin-cuda.hpp");
            break;
        case ClangToDotTranslator::OPENCL:
//          inc_dirs_list.insert(inc_dirs_list.begin(), c_config_include_dirs.begin(), c_config_include_dirs.end());
            // FIXME get the path right
            inc_list.push_back("clang-builtin-opencl.h");
            break;
        case ClangToDotTranslator::OBJC:
          {
         // DQ (10/23/2020): Added error message for Objective C language not supported in ROSE.
            ::mlog[WARN] << "Objective C langauge support is not available in ROSE \n";
            ROSE_ABORT();
          }
        default:
          {
            ::mlog[WARN] << "Default reached in switch(language) support \n";
            ROSE_ABORT();
          }
    }

 // FIXME should be handle by Clang ?
    define_list.push_back("__I__=_Complex_I");

    unsigned cnt = define_list.size() + inc_dirs_list.size() + inc_list.size();
    char ** args = new char*[cnt];
    std::vector<std::string>::iterator it_str;
    unsigned i = 0;
    for (it_str = define_list.begin(); it_str != define_list.end(); it_str++) {
        args[i] = new char[it_str->size() + 3];
        args[i][0] = '-';
        args[i][1] = 'D';
        strcpy(&(args[i][2]), it_str->c_str());
#if DEBUG_ARGS
        ::mlog[DEBUG] << "args[" << i << "] = " << args[i] << "\n";
#endif
        i++;
    }
    for (it_str = inc_dirs_list.begin(); it_str != inc_dirs_list.end(); it_str++) {
        args[i] = new char[it_str->size() + 3];
        args[i][0] = '-';
        args[i][1] = 'I';
        strcpy(&(args[i][2]), it_str->c_str());
#if DEBUG_ARGS
        ::mlog[DEBUG] << "args[" << i << "] = " << args[i] << "\n";
#endif
        i++;
    }
    for (it_str = inc_list.begin(); it_str != inc_list.end(); it_str++) {
        args[i] = new char[it_str->size() + 9];
        args[i][0] = '-'; args[i][1] = 'i'; args[i][2] = 'n'; args[i][3] = 'c';
        args[i][4] = 'l'; args[i][5] = 'u'; args[i][6] = 'd'; args[i][7] = 'e';
        strcpy(&(args[i][8]), it_str->c_str());
#if DEBUG_ARGS
        ::mlog[DEBUG] << "args[" << i << "] = " << args[i] << "\n";
#endif
        i++;
    }


  // 2 - Create a compiler instance

    // (10/30/2026) lin32: Clang 21 installs the invocation through CompilerInstance's constructor, not setInvocation().
    // (10/30/2026) lin32: DiagnosticOptions is no longer intrusive-refcounted; use invocation-owned options by reference.
    auto invocation_shptr = std::make_shared<clang::CompilerInvocation>();
    clang::CompilerInvocation * invocation = invocation_shptr.get();
#if CLANG_VERSION_MAJOR >= 21
    clang::CompilerInstance * compiler_instance = new clang::CompilerInstance(invocation_shptr);
    clang::TextDiagnosticPrinter * diag_printer = new clang::TextDiagnosticPrinter(llvm::errs(), invocation->getDiagnosticOpts());
#else
    clang::CompilerInstance * compiler_instance = new clang::CompilerInstance();
    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions();
    clang::TextDiagnosticPrinter * diag_printer = new clang::TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
#endif
    compiler_instance->createDiagnostics(diag_printer, true);
    llvm::ArrayRef<const char *> argsArrayRef(args, &(args[cnt]));
    clang::CompilerInvocation::CreateFromArgs(*invocation, argsArrayRef, compiler_instance->getDiagnostics());
#if CLANG_VERSION_MAJOR < 21
    compiler_instance->setInvocation(invocation_shptr);
#endif

    clang::LangOptions & lang_opts = compiler_instance->getLangOpts();

    switch (language) {
        case ClangToDotTranslator::C:
//          compiler_instance->getInvocation().setLangDefaults(lang_opts, clang::IK_C, );
            break;
        case ClangToDotTranslator::CPLUSPLUS:
            lang_opts.CPlusPlus = 1;
//          compiler_instance->getInvocation().setLangDefaults(lang_opts, clang::IK_CXX, );
            break;
        case ClangToDotTranslator::CUDA:
            lang_opts.CUDA = 1;
//          lang_opts.CPlusPlus = 1;
//          compiler_instance->getInvocation().setLangDefaults(lang_opts, clang::IK_CUDA,   clang::LangStandard::lang_cuda);
            break;
        case ClangToDotTranslator::OPENCL:
            lang_opts.OpenCL = 1;
//          compiler_instance->getInvocation().setLangDefaults(lang_opts, clang::IK_OpenCL, clang::LangStandard::lang_opencl);
            break;
        case ClangToDotTranslator::OBJC:
            ROSE_ASSERT(!"Objective-C is not supported by ROSE Compiler.");
//          compiler_instance->getInvocation().setLangDefaults(lang_opts, clang::IK_, );
        default:
            ROSE_ABORT();
    }

    // (10/30/2026) lin32: Clang 21 takes TargetOptions by reference; invocation ownership keeps it alive with TargetInfo.
#if CLANG_VERSION_MAJOR >= 21
    clang::TargetOptions & target_options = invocation->getTargetOpts();
    target_options.Triple = llvm::sys::getDefaultTargetTriple();
    clang::TargetInfo * target_info = clang::TargetInfo::CreateTargetInfo(compiler_instance->getDiagnostics(), target_options);
#else
    clang::TargetOptions target_options;
    target_options.Triple = llvm::sys::getDefaultTargetTriple();
    std::shared_ptr<clang::TargetOptions> targetOption_shptr = std::make_shared<clang::TargetOptions>(target_options);
    clang::TargetInfo * target_info = clang::TargetInfo::CreateTargetInfo(compiler_instance->getDiagnostics(), targetOption_shptr);
#endif
    compiler_instance->setTarget(target_info);

    compiler_instance->createFileManager();
    // (10/30/2026) lin32: Clang 22 createSourceManager() uses the instance's FileManager without an argument.
#if CLANG_VERSION_MAJOR >= 22
    compiler_instance->createSourceManager();
#else
    compiler_instance->createSourceManager(compiler_instance->getFileManager());
#endif

#if CLANG_VERSION_MAJOR >= 18
    llvm::Expected<clang::FileEntryRef> ret  = compiler_instance->getFileManager().getFileRef(input_file);
    if (!ret) { // Check if there was an error
      llvm::Error err = ret.takeError();
      // Handle the error, e.g., log it, print it, etc.
      llvm::errs() << "Failed to get file: " << err << "\n";
      // Ensure proper error handling, e.g., return from function
      return diag_printer->getNumErrors();
    }
    const clang::FileEntryRef input_file_entryRef = *ret; 
    clang::FileID mainFileID = compiler_instance->getSourceManager().createFileID(input_file_entryRef, clang::SourceLocation(), clang::SrcMgr::C_User);
#else
    llvm::ErrorOr<const clang::FileEntry *> ret  = compiler_instance->getFileManager().getFile(input_file);
    const clang::FileEntry * input_file_entry = ret.get(); 
    //clang::FileID mainFileID = compiler_instance->getSourceManager().createFileID(input_file_entry, clang::SourceLocation(), compiler_instance->getSourceManager().getFileCharacteristic(clang::SourceLocation()));
    clang::FileID mainFileID = compiler_instance->getSourceManager().createFileID(input_file_entry, clang::SourceLocation(), clang::SrcMgr::C_User);
#endif

    compiler_instance->getSourceManager().setMainFileID(mainFileID);

    if (!compiler_instance->hasPreprocessor()) compiler_instance->createPreprocessor(clang::TU_Complete);

    if (!compiler_instance->hasASTContext()) compiler_instance->createASTContext();

    ClangToDotTranslator translator(compiler_instance, language);
    compiler_instance->setASTConsumer(std::move(std::unique_ptr<clang::ASTConsumer>(&translator)));

    if (!compiler_instance->hasSema()) compiler_instance->createSema(clang::TU_Complete, NULL);

    ROSE_ASSERT (compiler_instance->hasDiagnostics());
    ROSE_ASSERT (compiler_instance->hasTarget());
    ROSE_ASSERT (compiler_instance->hasFileManager());
    ROSE_ASSERT (compiler_instance->hasSourceManager());
    ROSE_ASSERT (compiler_instance->hasPreprocessor());
    ROSE_ASSERT (compiler_instance->hasASTContext());
    ROSE_ASSERT (compiler_instance->hasSema());

  // 3 - Translate

 // printf ("\nCalling clang::ParseAST() (generate Dot file of Clang AST) \n");

    compiler_instance->getDiagnosticClient().BeginSourceFile(compiler_instance->getLangOpts(), &(compiler_instance->getPreprocessor()));
    clang::ParseAST(compiler_instance->getPreprocessor(), &translator, compiler_instance->getASTContext());
    compiler_instance->getDiagnosticClient().EndSourceFile();

 // printf ("\nDONE: Calling clang::ParseAST() (generate Dot file of Clang AST) \n\n");

 // 4 - Generate Graphviz

  // printf ("Calling translator.toDot(out); to call the DOT graph generator \n");

    translator.toDot(file);

 // printf ("DONE: Calling translator.toDot(out); to call the DOT graph generator \n");

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ABORT();
#endif

    return 0;
}

#if 0
// We don't need this for for the generation of the dot file.
void finishSageAST(ClangToDotTranslator & translator) {
    SgGlobal * global_scope = translator.getGlobalScope();

 // 1 - Label Statements: Move sub-statement after the label statement.

    std::vector<SgLabelStatement *> label_stmts = SageInterface::querySubTree<SgLabelStatement>(global_scope, V_SgLabelStatement);
    std::vector<SgLabelStatement *>::iterator label_stmt_it;
    for (label_stmt_it = label_stmts.begin(); label_stmt_it != label_stmts.end(); label_stmt_it++) {
        SgStatement * sub_stmt = (*label_stmt_it)->get_statement();
        if (!isSgNullStatement(sub_stmt)) {
            SgNullStatement * null_stmt = SageBuilder::buildNullStatement_nfi();
            translator.setCompilerGeneratedFileInfo(null_stmt);
            (*label_stmt_it)->set_statement(null_stmt);
            null_stmt->set_parent(*label_stmt_it);
            SageInterface::insertStatementAfter(*label_stmt_it, sub_stmt);
        }
    }

 // 2 - Place Preprocessor informations
}
#endif

#if 0
SgGlobal * ClangToDotTranslator::getGlobalScope() { return p_global_scope; }
#endif


// void ClangToDot::toDot(std::ostream & out) const 
void ClangToDotTranslator::toDot(std::ostream & out) const 
   {
#if 0
     printf ("In ClangToDotTranslator::toDot(): p_node_desc.size() = %zu \n",p_node_desc.size());
#endif
     out << "digraph {" << "\n";

     std::map<std::string, NodeDescriptor>::const_iterator it_node;
     for (it_node = p_node_desc.begin(); it_node != p_node_desc.end(); it_node++)
         it_node->second.toDot(out);

     out << "}" << "\n";
   }


#if 1
ClangToDotTranslator::ClangToDotTranslator(clang::CompilerInstance * compiler_instance, Language language_) :
    clang::ASTConsumer(),
    p_compiler_instance(compiler_instance),
    p_decl_translation_map(),
    p_stmt_translation_map(),
    p_type_translation_map(),
    p_node_desc(),
 // DQ (11/3/2020): Added data member.
    p_sage_preprocessor_recorder(new ClangToDotPreprocessorRecord(&(p_compiler_instance->getSourceManager()))),
    language(language_),
    ident_cnt(0)
{
   Rose::Diagnostics::initAndRegister(&logger, "ClangToDotTranslator"); 
   Rose::Diagnostics::mprefix->showProgramName(false);
   Rose::Diagnostics::mprefix->showThreadId(false);
   Rose::Diagnostics::mprefix->showElapsedTime(false);
}
#endif

ClangToDotTranslator::~ClangToDotTranslator() {}

std::string ClangToDotTranslator::genNextIdent() {
    std::ostringstream oss;
    oss << "ident" << ident_cnt++;
    return oss.str();
}


/* Overload of ASTConsumer::HandleTranslationUnit, it is the "entry point" */

void ClangToDotTranslator::HandleTranslationUnit(clang::ASTContext & ast_context) {

#if 0
     printf ("In ClangToDotTranslator::HandleTranslationUnit(): calling Traverse(ast_context.getTranslationUnitDecl()); \n");
#endif

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ABORT();
#endif

    Traverse(ast_context.getTranslationUnitDecl());
}

/* Preprocessor Stack */

std::pair<Sg_File_Info *, PreprocessingInfo *> ClangToDotTranslator::preprocessor_top() {
    return p_sage_preprocessor_recorder->top();
}
bool ClangToDotTranslator::preprocessor_pop() {
    return p_sage_preprocessor_recorder->pop();
}

// struct NextPreprocessorToInsert

ClangToDotNextPreprocessorToInsert::ClangToDotNextPreprocessorToInsert(ClangToDotTranslator & translator_) :
  cursor(NULL),
  candidat(NULL),
  next_to_insert(NULL),
  translator(translator_)
{}

ClangToDotNextPreprocessorToInsert * ClangToDotNextPreprocessorToInsert::next() {
    if (!translator.preprocessor_pop()) return NULL;

    ClangToDotNextPreprocessorToInsert * res = new ClangToDotNextPreprocessorToInsert(translator);

    std::pair<Sg_File_Info *, PreprocessingInfo *> next = translator.preprocessor_top();
    res->cursor = next.first;
    res->next_to_insert = next.second;
    res->candidat = candidat;
}

// class

ClangToDotNextPreprocessorToInsert * ClangToDotPreprocessorInserter::evaluateInheritedAttribute(SgNode * astNode, ClangToDotNextPreprocessorToInsert * inheritedValue) {
    SgLocatedNode * loc_node = isSgLocatedNode(astNode);
    if (loc_node == NULL) return inheritedValue;

    Sg_File_Info * current_pos = loc_node->get_startOfConstruct();

    bool passed_cursor = *current_pos > *(inheritedValue->cursor);

    if (passed_cursor) {
        // TODO insert on inheritedValue->candidat
        return inheritedValue->next();
    }

    
}

// class SagePreprocessorRecord

ClangToDotPreprocessorRecord::ClangToDotPreprocessorRecord(clang::SourceManager * source_manager) :
  p_source_manager(source_manager),
  p_preprocessor_record_list()
{
   Rose::Diagnostics::initAndRegister(&logger, "ClangToDotPreprocessorRecord"); 
   Rose::Diagnostics::mprefix->showProgramName(false);
   Rose::Diagnostics::mprefix->showThreadId(false);
   Rose::Diagnostics::mprefix->showElapsedTime(false);
}

void ClangToDotPreprocessorRecord::InclusionDirective(clang::SourceLocation HashLoc, const clang::Token & IncludeTok, llvm::StringRef FileName, bool IsAngled,
                                                const clang::FileEntry * File, clang::SourceLocation EndLoc, llvm::StringRef SearchPath, llvm::StringRef RelativePath) {
    logger[INFO] << "InclusionDirective" << "\n";

    bool inv_begin_line;
    bool inv_begin_col;

    unsigned ls = p_source_manager->getSpellingLineNumber(HashLoc, &inv_begin_line);
    unsigned cs = p_source_manager->getSpellingColumnNumber(HashLoc, &inv_begin_col);

    std::string file = RoseClangCompat::getFileName(*p_source_manager, p_source_manager->getFileID(HashLoc));

    logger[INFO] << "    In file  : " << file << "\n";
    logger[INFO] << "    From     : " << ls << ":" << cs << "\n";
    logger[INFO] << "    Included : " << FileName.str() << "\n";
    logger[INFO] << "    Is angled: " << (IsAngled ? "T" : "F") << "\n";

    Sg_File_Info * file_info = new Sg_File_Info(file, ls, cs);
    PreprocessingInfo * preproc_info = new PreprocessingInfo(
                                                  PreprocessingInfo::CpreprocessorIncludeDeclaration,
                                                  FileName.str(),
                                                  file,
                                                  ls,
                                                  cs,
                                                  0,
                                                  PreprocessingInfo::before
                                           );

    p_preprocessor_record_list.push_back(std::pair<Sg_File_Info *, PreprocessingInfo *>(file_info, preproc_info));
}

void ClangToDotPreprocessorRecord::EndOfMainFile() {
    logger[INFO] << "EndOfMainFile" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::Ident(clang::SourceLocation Loc, const std::string & str) {
    logger[INFO] << "Ident" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::PragmaComment(clang::SourceLocation Loc, const clang::IdentifierInfo * Kind, const std::string & Str) {
    logger[INFO] << "PragmaComment" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::PragmaMessage(clang::SourceLocation Loc, llvm::StringRef Str) {
    logger[INFO] << "PragmaMessage" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::PragmaDiagnosticPush(clang::SourceLocation Loc, llvm::StringRef Namespace) {
    logger[INFO] << "PragmaDiagnosticPush" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::PragmaDiagnosticPop(clang::SourceLocation Loc, llvm::StringRef Namespace) {
    logger[INFO] << "PragmaDiagnosticPop" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::PragmaDiagnostic(clang::SourceLocation Loc, llvm::StringRef Namespace, clang::diag::Severity Severity, llvm::StringRef Str) {
    logger[INFO] << "PragmaDiagnostic" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::MacroExpands(const clang::Token & MacroNameTok, const clang::MacroInfo * MI, clang::SourceRange Range) {
    logger[INFO] << "MacroExpands" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::MacroDefined(const clang::Token & MacroNameTok, const clang::MacroInfo * MI) {
    logger[INFO] << "" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::MacroUndefined(const clang::Token & MacroNameTok, const clang::MacroInfo * MI) {
    logger[INFO] << "MacroUndefined" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::Defined(const clang::Token & MacroNameTok) {
    logger[INFO] << "Defined" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::SourceRangeSkipped(clang::SourceRange Range) {
    logger[INFO] << "SourceRangeSkipped" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::If(clang::SourceRange Range) {
    logger[INFO] << "If" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::Elif(clang::SourceRange Range) {
    logger[INFO] << "Elif" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::Ifdef(const clang::Token & MacroNameTok) {
    logger[INFO] << "Ifdef" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::Ifndef(const clang::Token & MacroNameTok) {
    logger[INFO] << "Ifndef" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::Else() {
    logger[INFO] << "Else" << "\n";
    ROSE_ABORT();
}

void ClangToDotPreprocessorRecord::Endif() {
    logger[INFO] << "Endif" << "\n";
    ROSE_ABORT();
}

std::pair<Sg_File_Info *, PreprocessingInfo *> ClangToDotPreprocessorRecord::top() {
    return p_preprocessor_record_list.front();
}

bool ClangToDotPreprocessorRecord::pop() {
    p_preprocessor_record_list.erase(p_preprocessor_record_list.begin());
    return !p_preprocessor_record_list.empty();
}



ClangToDotTranslator::NodeDescriptor::NodeDescriptor(std::string ident_) :
    ident(ident_),
    kind_hierarchy(),
    successors(),
    attributes()
{}


// DQ (11/27/2020): Adding support from Tristan's ClangToDot version.
// void ClangToDot::NodeDescriptor::toDot(std::ostream & out) const 
void ClangToDotTranslator::NodeDescriptor::toDot(std::ostream & out) const 
  {
    std::vector<std::string>::const_iterator str_it;
    std::vector<std::pair<std::string, std::string> >::const_iterator pair_str_it;

#if 0
    printf ("In ClangToDotTranslator::NodeDescriptor::toDot(): kind_hierarchy.size() = %zu \n",kind_hierarchy.size());
    printf ("In ClangToDotTranslator::NodeDescriptor::toDot(): attributes.size()     = %zu \n",attributes.size());
    printf ("In ClangToDotTranslator::NodeDescriptor::toDot(): successors.size()     = %zu \n",successors.size());

    if (kind_hierarchy.size() > 0)
       {
      // printf ("kind_hierarchy.front() = %s \n",(*(kind_hierarchy.front())).c_str());
      // printf ("kind_hierarchy.front() = %s \n",kind_hierarchy.front()->c_str());
         cout << "\nkind_hierarchy.front() = " << kind_hierarchy.front() << "\n\n";
       }
      else
       {
         printf ("kind_hierarchy is EMPTY \n");
       }
#endif

    out << "\t" << ident << "[";
        out << "label=\"";
            out << kind_hierarchy.front() << "\\n";

         // DQ  (11/27/2020): Note that if kind_hierarchy is empty, this is a runtime error.
            for (str_it = kind_hierarchy.begin() + 1; str_it != kind_hierarchy.end(); str_it++)
                out << "::" << *str_it;
            out << "\\n\\n";
            for (pair_str_it = attributes.begin(); pair_str_it != attributes.end(); pair_str_it++)
                out << pair_str_it->first << " : " << pair_str_it->second << "\\n";
        out << "\"";
        // TODO color from node type (NIY)
    out << "];" << "\n";

    for (pair_str_it = successors.begin(); pair_str_it != successors.end(); pair_str_it++) 
        {
          if (pair_str_it->second != "")
              out << "\t\t" << ident << " -> " << pair_str_it->second << "[label=\"" << pair_str_it->first << "\"];" << "\n";
        }

    out << "\n";
}


void ClangToDotTranslator::VisitTemplateArgument(const clang::TemplateArgument & template_argument, ClangToDotTranslator::NodeDescriptor & node_desc, std::string prefix) {
    std::ostringstream oss;
    oss << prefix;
    switch (template_argument.getKind()) {
        case clang::TemplateArgument::Null:
            oss << " null";
            node_desc.attributes.push_back(std::pair<std::string, std::string>(oss.str(), ""));
            break;
        case clang::TemplateArgument::Type:
            oss << " type";
            node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(template_argument.getAsType().getTypePtr())));
            break;
        case clang::TemplateArgument::Declaration:
            oss << " declaration";
            node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(template_argument.getAsDecl())));
            break;
        case clang::TemplateArgument::Integral:
            oss << " integral";
         // assert(DEBUG_TODO == 0); // TODO
            ROSE_ASSERT(FAIL_TODO == 0); // TODO
            break;
        case clang::TemplateArgument::Template:
            oss << " template";
         // assert(DEBUG_TODO == 0); // TODO
            ROSE_ASSERT(FAIL_TODO == 0); // TODO
            break;
        case clang::TemplateArgument::TemplateExpansion:
            oss << " template_expansion";
         // assert(DEBUG_TODO == 0); // TODO
            ROSE_ASSERT(FAIL_TODO == 0); // TODO
            break;
        case clang::TemplateArgument::Expression:
            oss << " expression";
            node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(template_argument.getAsExpr())));
            break;
        case clang::TemplateArgument::Pack:
        {
            clang::TemplateArgument::pack_iterator pack_it;
            unsigned cnt = 0;
            for (pack_it = template_argument.pack_begin(); pack_it != template_argument.pack_end(); pack_it++) {
                oss << " pack[" << cnt++ << "]";
                VisitTemplateArgument(*pack_it, node_desc, oss.str());
            }
            break;
        }
    }
}

void ClangToDotTranslator::VisitNestedNameSpecifier(RoseClangCompat::NestedNameSpecifier nested_name_specifier, ClangToDotTranslator::NodeDescriptor & node_desc, std::string prefix) {
    // (10/30/2026) lin32: Clang 22 uses value qualifiers and scoped kinds; namespace prefixes come from NamespaceAndPrefix.
#if CLANG_VERSION_MAJOR >= 22
    switch (nested_name_specifier.getKind()) {
        case clang::NestedNameSpecifier::Kind::Null:
            return;
        case clang::NestedNameSpecifier::Kind::Global:
            node_desc.attributes.push_back(std::pair<std::string, std::string>(prefix, "global (::)"));
            break;
        case clang::NestedNameSpecifier::Kind::Namespace: {
            VisitNestedNameSpecifier(nested_name_specifier.getAsNamespaceAndPrefix().Prefix, node_desc, prefix + " prefix");
            if (clang::NamespaceDecl *decl = RoseClangCompat::getAsNamespace(nested_name_specifier))
                node_desc.successors.push_back(std::pair<std::string, std::string>(prefix + " namespace", Traverse(decl)));
            else if (clang::NamespaceAliasDecl *decl = RoseClangCompat::getAsNamespaceAlias(nested_name_specifier))
                node_desc.successors.push_back(std::pair<std::string, std::string>(prefix + " namespace_alias", Traverse(decl)));
            break;
        }
        case clang::NestedNameSpecifier::Kind::Type:
            node_desc.successors.push_back(std::pair<std::string, std::string>(prefix + " type_specifier", Traverse(nested_name_specifier.getAsType())));
            break;
        case clang::NestedNameSpecifier::Kind::MicrosoftSuper:
            node_desc.successors.push_back(std::pair<std::string, std::string>(prefix + " microsoft_super", Traverse(nested_name_specifier.getAsMicrosoftSuper())));
            break;
    }
#else
    if (nested_name_specifier == NULL) return;

    ClangToDotTranslator::VisitNestedNameSpecifier(nested_name_specifier->getPrefix(), node_desc, prefix + " prefix");

    switch (nested_name_specifier->getKind()) {
        case clang::NestedNameSpecifier::Identifier:
            node_desc.attributes.push_back(
                std::pair<std::string, std::string>(prefix + " identifier", nested_name_specifier->getAsIdentifier()->getName().data())
            );
            break;
        case clang::NestedNameSpecifier::Namespace:
            node_desc.successors.push_back(
                std::pair<std::string, std::string>(prefix + " namespace", Traverse(nested_name_specifier->getAsNamespace()))
            );
            break;
        case clang::NestedNameSpecifier::NamespaceAlias:
            node_desc.successors.push_back(
                std::pair<std::string, std::string>(prefix + " namespace_alias", Traverse(nested_name_specifier->getAsNamespaceAlias()))
            );
            break;
        case clang::NestedNameSpecifier::TypeSpec:
            node_desc.successors.push_back(
                std::pair<std::string, std::string>(prefix + " type_specifier", Traverse(nested_name_specifier->getAsType()))
            );
            break;
        // (10/30/2026) lin32: Clang 21 removed the separate TypeSpecWithTemplate qualifier kind.
#if CLANG_VERSION_MAJOR < 21
        case clang::NestedNameSpecifier::TypeSpecWithTemplate:
            node_desc.successors.push_back(
                std::pair<std::string, std::string>(prefix + " type_specifier_with_template", Traverse(nested_name_specifier->getAsType()))
            );
            break;
#endif
        case clang::NestedNameSpecifier::Global:
            node_desc.attributes.push_back(std::pair<std::string, std::string>(prefix, "global (::)"));
            break;
    }
#endif
}

void ClangToDotTranslator::VisitTemplateName(const clang::TemplateName & template_name, ClangToDotTranslator::NodeDescriptor & node_desc, std::string prefix) {
    std::ostringstream oss;
    oss << prefix;
    switch (template_name.getKind()) {
        case clang::TemplateName::Template:
            oss << " template";
            node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(template_name.getAsTemplateDecl())));
            break;
        case clang::TemplateName::OverloadedTemplate:
        {
            clang::OverloadedTemplateStorage * overloaded_template_storage = template_name.getAsOverloadedTemplate();
         // assert(DEBUG_TODO == 0); // TODO
            ROSE_ASSERT(FAIL_TODO == 0); // TODO
            break;
        }
        case clang::TemplateName::QualifiedTemplate:
            oss << " qualified_template";
#if CLANG_VERSION_MAJOR > 14
            VisitNestedNameSpecifier(template_name.getAsQualifiedTemplateName()->getQualifier(), node_desc, oss.str() + "nested_name_specifier");
            node_desc.successors.push_back(
                std::pair<std::string, std::string>(oss.str() + "declaration", Traverse(template_name.getAsQualifiedTemplateName()->getUnderlyingTemplate().getAsTemplateDecl()))
            );
#else
            VisitNestedNameSpecifier(template_name.getAsQualifiedTemplateName()->getQualifier(), node_desc, oss.str() + "nested_name_specifier");
            node_desc.successors.push_back(
                std::pair<std::string, std::string>(oss.str() + "declaration", Traverse(template_name.getAsQualifiedTemplateName()->getDecl()))
            );
            node_desc.successors.push_back(
                std::pair<std::string, std::string>(oss.str() + "template_declaration", Traverse(template_name.getAsQualifiedTemplateName()->getTemplateDecl()))
            );
#endif
            break;
        case clang::TemplateName::DependentTemplate:
         // assert(DEBUG_TODO == 0); // TODO
            ROSE_ASSERT(FAIL_TODO == 0); // TODO
            break;
        case clang::TemplateName::SubstTemplateTemplateParm:
         // assert(DEBUG_TODO == 0); // TODO
            ROSE_ASSERT(FAIL_TODO == 0); // TODO
            break;
        case clang::TemplateName::SubstTemplateTemplateParmPack:
         // assert(DEBUG_TODO == 0); // TODO
            ROSE_ASSERT(FAIL_TODO == 0); // TODO
            break;
    }
}
