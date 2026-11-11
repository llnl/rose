
#include <iostream>

#include "sage3basic.h"
#include "clang-frontend-private.hpp"

#include "rose_config.h"

#include "clang-to-dot.hpp"
extern bool roseInstallPrefix(std::string&);

// DQ (11/28/2020): Use this for testing the DOT graph generator.
#define EXIT_AFTER_BUILDING_DOT_FILE 0

Rose::Diagnostics::Facility ClangToSageTranslator::logger;
Rose::Diagnostics::Facility SagePreprocessorRecord::logger;

using namespace Sawyer::Message;
int clang_main(int argc, char ** argv, SgSourceFile& sageFile) {

    Rose::Diagnostics::mprefix->showProgramName(false);
    Rose::Diagnostics::mprefix->showThreadId(false);
    Rose::Diagnostics::mprefix->showElapsedTime(false);

 // ::mlog[INFO] << "sageFile.get_clang_il_to_graphviz() = " << (sageFile.get_clang_il_to_graphviz() ? "true" : "false") << "\n";

 // DQ (11/27/2020): Use the -rose:clang_il_to_graphviz option to comntrol the use of the Clang Dot generator.
#if EXIT_AFTER_BUILDING_DOT_FILE
    if (true)
#else
    if (sageFile.get_clang_il_to_graphviz() == true)
#endif
       {
#if 1
      // DQ (10/23/2020): Calling clang-to-dot generator (I don't think this modifies the argv list).
         int clang_to_dot_status = clang_to_dot_main(argc,argv);
#if 0
         ::mlog[ERROR] << "Exiting as a test!" << "\n";
         ROSE_ABORT();
#endif
         if (clang_to_dot_status != 0)
            {
              ::mlog[WARN] << "Error in generation of dot file of Clang IR: returing from top of clang_main(): clang_to_dot_status = " << clang_to_dot_status << "\n";
              return clang_to_dot_status;
            }
           else
            {
#if 0
              ::mlog[WARN] << "Note: Dot file of Clang IR output in file: clangGraph.dot \n";
#endif
            }
#endif

#if EXIT_AFTER_BUILDING_DOT_FILE
         return 0;
#endif
       }

  // 0 - Analyse Cmd Line

    std::vector<std::string> sys_dirs_list;
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

    ClangToSageTranslator::Language language = ClangToSageTranslator::unknown;

    size_t last_period = input_file.find_last_of(".");
    std::string extention(input_file.substr(last_period + 1));

    if (extention == "c") {
        language = ClangToSageTranslator::C;
    }
    else if (extention == "C" || extention == "cxx" || extention == "cpp" || extention == "cc") {
        language = ClangToSageTranslator::CPLUSPLUS;
    }
    else if (extention == "objc") {
        language = ClangToSageTranslator::OBJC;
    }
    else if (extention == "cu") {
        language = ClangToSageTranslator::CUDA;
    }
    else if (extention == "ocl" || extention == "cl") {
        language = ClangToSageTranslator::OPENCL;
    }

    ROSE_ASSERT(language != ClangToSageTranslator::unknown);

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

    sys_dirs_list.push_back(rose_include_path + "clang/");

    switch (language) {
        case ClangToSageTranslator::C:
            sys_dirs_list.insert(sys_dirs_list.begin(), c_config_include_dirs.begin(), c_config_include_dirs.end());
            inc_list.push_back("clang-builtin-c.h");
            break;
        case ClangToSageTranslator::CPLUSPLUS:
            sys_dirs_list.insert(sys_dirs_list.begin(), cxx_config_include_dirs.begin(), cxx_config_include_dirs.end());
            inc_list.push_back("clang-builtin-cpp.hpp");
            break;
        case ClangToSageTranslator::CUDA:
            sys_dirs_list.insert(sys_dirs_list.begin(), cxx_config_include_dirs.begin(), cxx_config_include_dirs.end());
            inc_list.push_back("clang-builtin-cuda.hpp");
            break;
        case ClangToSageTranslator::OPENCL:
            sys_dirs_list.insert(sys_dirs_list.begin(), c_config_include_dirs.begin(), c_config_include_dirs.end());
            inc_list.push_back("clang-builtin-opencl.h");
            break;
        case ClangToSageTranslator::OBJC:
          {
            ::mlog[ERROR] << "Objective C langauge support is not available in ROSE \n";
            ROSE_ABORT();
          }
        default:
          {
            ::mlog[ERROR] << "Default reached in switch(language) support \n";
            ROSE_ABORT();
          }
    }

 // FIXME should be handle by Clang ?
    define_list.push_back("__I__=_Complex_I");

    unsigned cnt = 1 + define_list.size() + inc_dirs_list.size() + sys_dirs_list.size() + inc_list.size();
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
    for (it_str = sys_dirs_list.begin(); it_str != sys_dirs_list.end(); it_str++) {
        args[i] = new char[it_str->size() + 9];
        args[i][0] = '-'; args[i][1] = 'i'; args[i][2] = 's'; args[i][3] = 'y';
        args[i][4] = 's'; args[i][5] = 't'; args[i][6] = 'e'; args[i][7] = 'm';
        strcpy(&(args[i][8]), it_str->c_str());
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

//    clang::LangOptions & lang_opts = compiler_instance->getLangOpts();

    // Pei-Hung (07/18/2023): Language args are handled by CompilerInvocation::ParseLangArgs,
    // triggered by CompilerInvocation::CreateFromArgs.
    // Providing language switch , -x, will properly setup the lang_opts for a given language.
 
    std::string languageArg = "";
    switch (language) {
        case ClangToSageTranslator::C:
//          compiler_instance->getInvocation().setLangDefaults(lang_opts, clang::IK_C, );
            languageArg = "-xc";
            break;
        case ClangToSageTranslator::CPLUSPLUS:
//            lang_opts.CPlusPlus = 1;
//            lang_opts.Bool = 1;
            languageArg = "-xc++";
//          compiler_instance->getInvocation().setLangDefaults(lang_opts, clang::IK_CXX, );
            break;
        case ClangToSageTranslator::CUDA:
//            lang_opts.CUDA = 1;
            languageArg = "-xcuda";
//          lang_opts.CPlusPlus = 1;
//          compiler_instance->getInvocation().setLangDefaults(lang_opts, clang::IK_CUDA,   clang::LangStandard::lang_cuda);
            break;
        case ClangToSageTranslator::OPENCL:
//            lang_opts.OpenCL = 1;
            languageArg = "-xcl";
//          compiler_instance->getInvocation().setLangDefaults(lang_opts, clang::IK_OpenCL, clang::LangStandard::lang_opencl);
            break;
        case ClangToSageTranslator::OBJC:
            ROSE_ASSERT(!"Objective-C is not supported by ROSE Compiler.");
            languageArg = "-xobjective-c";
//          compiler_instance->getInvocation().setLangDefaults(lang_opts, clang::IK_, );
        default:
            ROSE_ABORT();
    }
    // (10/30/2026) lin32: Reserve space for the terminating NUL copied by strcpy when assembling Clang arguments.
    args[i] = new char[languageArg.size() + 1];
    strcpy(&(args[cnt-1][0]), languageArg.c_str());
    llvm::ArrayRef<const char *> argsArrayRef(args, &(args[cnt]));
    clang::CompilerInvocation::CreateFromArgs(*invocation, argsArrayRef, compiler_instance->getDiagnostics());
#if CLANG_VERSION_MAJOR < 21
    compiler_instance->setInvocation(invocation_shptr);
#endif

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
    if (!mainFileID.isValid()) { // Check if the FileID is valid
      llvm::errs() << "Failed to create FileID for the input file.\n";
    }
#else
    llvm::ErrorOr<const clang::FileEntry *> ret  = compiler_instance->getFileManager().getFile(input_file);
    const clang::FileEntry * input_file_entry = ret.get(); 
    clang::FileID mainFileID = compiler_instance->getSourceManager().createFileID(input_file_entry, clang::SourceLocation(), clang::SrcMgr::C_User);
#endif

    compiler_instance->getSourceManager().setMainFileID(mainFileID);

    if (!compiler_instance->hasPreprocessor()) compiler_instance->createPreprocessor(clang::TU_Complete);

    if (!compiler_instance->hasASTContext()) compiler_instance->createASTContext();

    ClangToSageTranslator translator(compiler_instance, language);
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

//  ::mlog[DEBUG] << "Calling clang::ParseAST()\n";

    compiler_instance->getDiagnosticClient().BeginSourceFile(compiler_instance->getLangOpts(), &(compiler_instance->getPreprocessor()));
    clang::ParseAST(compiler_instance->getPreprocessor(), &translator, compiler_instance->getASTContext());
    compiler_instance->getDiagnosticClient().EndSourceFile();

//  ::mlog[DEBUG] << "Clang found " << diag_printer->getNumWarnings() <<  " warning and " << diag_printer->getNumErrorss() << " errors\n";

    SgGlobal * global_scope = translator.getGlobalScope();

  // 4 - Attach to the file

    if (sageFile.get_globalScope() != NULL) SageInterface::deleteAST(sageFile.get_globalScope());

    sageFile.set_globalScope(global_scope);

    global_scope->set_parent(&sageFile);

    std::string file_name(input_file);

    Sg_File_Info * start_fi = new Sg_File_Info(file_name, 0, 0);
    Sg_File_Info * end_fi   = new Sg_File_Info(file_name, 0, 0);

    global_scope->set_startOfConstruct(start_fi);

    global_scope->set_endOfConstruct(end_fi);

  // 5 - Finish the AST (fixup phase)

    finishSageAST(translator);

    return diag_printer->getNumErrors();
}

void finishSageAST(ClangToSageTranslator & translator) {
    SgGlobal * global_scope = translator.getGlobalScope();

 // 1 - Label Statements: Move sub-statement after the label statement.

// Pei-Hung (05/13/2022) This step is no longer needed as subStatement is properly handled.  
/*
    std::vector<SgLabelStatement *> label_stmts = SageInterface::querySubTree<SgLabelStatement>(global_scope, V_SgLabelStatement);
    std::vector<SgLabelStatement *>::iterator label_stmt_it;
    for (label_stmt_it = label_stmts.begin(); label_stmt_it != label_stmts.end(); label_stmt_it++) {
        SgLabelStatement* labelStmt = isSgLabelStatement(*label_stmt_it);
        SgStatement * sub_stmt = labelStmt->get_statement();
        if (!isSgNullStatement(sub_stmt)) {
            SgNullStatement * null_stmt = SageBuilder::buildNullStatement_nfi();
            translator.setCompilerGeneratedFileInfo(null_stmt);
            labelStmt->set_statement(null_stmt);
            null_stmt->set_parent(labelStmt);
            SageInterface::insertStatementAfter(labelStmt, sub_stmt);
        }
    }
*/
 // 2 - Place Preprocessor informations
}

SgGlobal * ClangToSageTranslator::getGlobalScope() { return p_global_scope; }

ClangToSageTranslator::ClangToSageTranslator(clang::CompilerInstance * compiler_instance, Language language_) :
    clang::ASTConsumer(),
    p_decl_translation_map(),
    p_stmt_translation_map(),
    p_type_translation_map(),
    p_global_scope(NULL),
    p_class_type_decl_first_see_in_type(),
    p_enum_type_decl_first_see_in_type(),
    p_compiler_instance(compiler_instance),
    p_sage_preprocessor_recorder(new SagePreprocessorRecord(&(p_compiler_instance->getSourceManager()))),
    language(language_)
{
   Rose::Diagnostics::initAndRegister(&logger, "ClangToSageTranslator"); 
   Rose::Diagnostics::mprefix->showProgramName(false);
   Rose::Diagnostics::mprefix->showThreadId(false);
   Rose::Diagnostics::mprefix->showElapsedTime(false);
}

ClangToSageTranslator::~ClangToSageTranslator() {
    delete p_sage_preprocessor_recorder;
}

/* (protected) Helper methods */

void ClangToSageTranslator::applySourceRange(SgNode * node, clang::SourceRange source_range) 
   {
     SgLocatedNode * located_node = isSgLocatedNode(node);
     SgInitializedName * init_name = isSgInitializedName(node);

#if DEBUG_SOURCE_LOCATION
     logger[DEBUG] << "Set File_Info for " << node << " of type " << node->class_name() << "\n";
#endif

     if (located_node == NULL && init_name == NULL) 
        {
          logger[ERROR] << "Consistency error: try to apply a source range to a Sage node which is not a SgLocatedNode or a SgInitializedName." << "\n";
          exit(-1);
        }
       else 
        {
          if (located_node != NULL) 
             {
               Sg_File_Info * fi = located_node->get_startOfConstruct();
               if (fi != NULL) delete fi;
               fi = located_node->get_endOfConstruct();
               if (fi != NULL) delete fi;
             }
            else
             {
               if (init_name != NULL)
                  {
                    Sg_File_Info * fi = init_name->get_startOfConstruct();
                    if (fi != NULL) delete fi;
                    fi = init_name->get_endOfConstruct();
                    if (fi != NULL) delete fi;
                  }
             }
        }

     Sg_File_Info * start_fi = NULL;
     Sg_File_Info * end_fi   = NULL;

     if (source_range.isValid()) 
        {
          clang::SourceLocation begin  = source_range.getBegin();
          clang::SourceLocation end    = source_range.getEnd();

          if (begin.isValid() && end.isValid())
             {
               if (begin.isMacroID())
                  {
#if DEBUG_SOURCE_LOCATION
                    logger[DEBUG] << "\tDump SourceLocation begin as it is a MacroID: ";
                    begin.dump(p_compiler_instance->getSourceManager());
                    logger[DEBUG] << "\n";
#endif
                    begin = p_compiler_instance->getSourceManager().getExpansionLoc(begin);
                    ROSE_ASSERT(begin.isValid());
                  }

               if (end.isMacroID())
                  {
#if DEBUG_SOURCE_LOCATION
                    logger[DEBUG] << "\tDump SourceLocation end as it is a MacroID: ";
                    end.dump(p_compiler_instance->getSourceManager());
                    logger[DEBUG] << "\n";
#endif
                    end = p_compiler_instance->getSourceManager().getExpansionLoc(end);
                    ROSE_ASSERT(end.isValid());
                  }

               clang::FileID file_begin = p_compiler_instance->getSourceManager().getFileID(begin);
               clang::FileID file_end   = p_compiler_instance->getSourceManager().getFileID(end);

               bool inv_begin_line;
               bool inv_begin_col;
               bool inv_end_line;
               bool inv_end_col;

               unsigned ls = p_compiler_instance->getSourceManager().getSpellingLineNumber(begin, &inv_begin_line);
               unsigned cs = p_compiler_instance->getSourceManager().getSpellingColumnNumber(begin, &inv_begin_col);
               unsigned le = p_compiler_instance->getSourceManager().getSpellingLineNumber(end, &inv_end_line);
               unsigned ce = p_compiler_instance->getSourceManager().getSpellingColumnNumber(end, &inv_end_col);

               if (file_begin.isInvalid() || file_end.isInvalid() || inv_begin_line || inv_begin_col || inv_end_line || inv_end_col)
                  {
                    ROSE_ASSERT(!"Should not happen as everything have been check before...");
                  }

               if (p_compiler_instance->getSourceManager().getFileEntryForID(file_begin) != NULL) 
                  {
                    std::string file = RoseClangCompat::getFileName(p_compiler_instance->getSourceManager(), file_begin);

                 // start_fi = new Sg_File_Info(file, ls, cs);
                 // end_fi   = new Sg_File_Info(file, le, ce);
#if 0
                    std::string rawFileName         = node->get_file_info()->get_raw_filename();
                    std::string filenameWithoutPath = Rose::StringUtility::stripPathFromFileName(rawFileName);
                    ::mlog[DEBUG] << "filenameWithoutPath = " << filenameWithoutPath.c_str() << " file = " << file.c_str() << "\n";
#endif
                    if (file.find("clang-builtin-c.h") != std::string::npos) 
                       {
#if 0
                         ::mlog[DEBUG] << "Processing a frontend specific file \n";
#endif
                         start_fi = new Sg_File_Info(file, ls, cs);
                         end_fi   = new Sg_File_Info(file, le, ce);

                      // DQ (11/29/2020): This is not doing what I had hoped it would do.
                      // I think the solution is to use the -DSKIP_ROSE_BUILTIN_DECLARATIONS option,
                      // but that is not working as I expected either.  Time to go home.
                      // start_fi = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
                      // end_fi   = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
                         start_fi->set_classificationBitField(Sg_File_Info::e_frontend_specific);
                         end_fi  ->set_classificationBitField(Sg_File_Info::e_frontend_specific);
                       }
                      else
                       {
                         start_fi = new Sg_File_Info(file, ls, cs);
                         end_fi   = new Sg_File_Info(file, le, ce);
                       }

#if DEBUG_SOURCE_LOCATION
                    logger[DEBUG] << "\tCreate FI for node in " << file << ":" << ls << ":" << cs << "\n";
#endif
                  }
#if DEBUG_SOURCE_LOCATION
                 else
                  {
                    logger[DEBUG] << "\tDump SourceLocation for \"Invalid FileID\": " << "\n" << "\t";
                    begin.dump(p_compiler_instance->getSourceManager());
                    logger[DEBUG] << "\n" << "\t";
                    end.dump(p_compiler_instance->getSourceManager());
                    logger[DEBUG] << "\n";
                  }
#endif
             }
        }

     if (start_fi == NULL && end_fi == NULL) 
        {
          start_fi = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
          end_fi   = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();

          start_fi->setCompilerGenerated();
          end_fi->setCompilerGenerated();
#if DEBUG_SOURCE_LOCATION
          logger[DEBUG] << "Create FI for compiler generated node" << "\n";
#endif
        }
       else
        {
          if (start_fi == NULL || end_fi == NULL)
             {
               ROSE_ASSERT(!"start_fi == NULL xor end_fi == NULL");
             }
        }

     if (located_node != NULL)
        {
          located_node->set_startOfConstruct(start_fi);
          // Pei-Hung (09/29/2022) SgExpression::get_file_info() checks and returns get_operatorPosition()
          // therefore, call set_operatorPosition() to make sure the Sg_File_Info is properly set.
          if(isSgExpression(located_node))
          {
             isSgExpression(located_node)->set_operatorPosition(start_fi);
          }
          located_node->set_endOfConstruct(end_fi);
        }
       else
        {
          if (init_name != NULL)
             {
               init_name->set_startOfConstruct(start_fi);
               init_name->set_endOfConstruct(end_fi);
             }
        }
   }

void ClangToSageTranslator::setCompilerGeneratedFileInfo(SgNode * node, bool to_be_unparse) {
    Sg_File_Info * start_fi = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
    Sg_File_Info * end_fi   = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();

    start_fi->setCompilerGenerated();
    end_fi->setCompilerGenerated();

    if (to_be_unparse) {
        start_fi->setOutputInCodeGeneration();
        end_fi->setOutputInCodeGeneration();
    }

    ROSE_ASSERT(start_fi != NULL && end_fi != NULL);

#if DEBUG_SOURCE_LOCATION
    logger[DEBUG] << "Set File_Info for " << node << " of type " << node->class_name() << "\n";
#endif

    SgLocatedNode * located_node = isSgLocatedNode(node);
    SgInitializedName * init_name = isSgInitializedName(node);

    if (located_node == NULL && init_name == NULL) {
        logger[DEBUG] << "Consistency error: try to set a Sage node which is not a SgLocatedNode or a SgInitializedName as compiler generated" << "\n";
        exit(-1);
    }
    else if (located_node != NULL) {
        Sg_File_Info * fi = located_node->get_startOfConstruct();
        if (fi != NULL) delete fi;
        fi = located_node->get_endOfConstruct();
        if (fi != NULL) delete fi;

        located_node->set_startOfConstruct(start_fi);
        // Pei-Hung (07/12/2023) This is added for the same reason shown above in applySourceRange 
        // SgExpression::get_file_info() checks and returns get_operatorPosition()
        // therefore, call set_operatorPosition() to make sure the Sg_File_Info is properly set.
        if(isSgExpression(located_node))
        {
           fi = located_node->get_file_info();
           if (fi != NULL) delete fi;
           isSgExpression(located_node)->set_file_info(start_fi);
        }
        located_node->set_endOfConstruct(end_fi);
    }
    else if (init_name != NULL) {
        Sg_File_Info * fi = init_name->get_startOfConstruct();
        if (fi != NULL) delete fi;
        fi = init_name->get_endOfConstruct();
        if (fi != NULL) delete fi;

        init_name->set_startOfConstruct(start_fi);
        init_name->set_endOfConstruct(end_fi);
    }
}

/* Overload of ASTConsumer::HandleTranslationUnit, it is the "entry point" */

void ClangToSageTranslator::HandleTranslationUnit(clang::ASTContext & ast_context) {
    Traverse(ast_context.getTranslationUnitDecl());
}

/* Preprocessor Stack */

std::pair<Sg_File_Info *, PreprocessingInfo *> ClangToSageTranslator::preprocessor_top() {
    return p_sage_preprocessor_recorder->top();
}

bool ClangToSageTranslator::preprocessor_pop() {
    return p_sage_preprocessor_recorder->pop();
}

// struct NextPreprocessorToInsert

// NextPreprocessorToInsert::NextPreprocessorToInsert(ClangToSageTranslator & translator_) :
NextPreprocessorToInsert::NextPreprocessorToInsert(ClangToSageTranslator & translator_) :
  cursor(NULL),
  candidat(NULL),
  next_to_insert(NULL),
  translator(translator_)
{}

NextPreprocessorToInsert * NextPreprocessorToInsert::next() {
    if (!translator.preprocessor_pop()) return NULL;

    NextPreprocessorToInsert * res = new NextPreprocessorToInsert(translator);

    std::pair<Sg_File_Info *, PreprocessingInfo *> next = translator.preprocessor_top();
    res->cursor = next.first;
    res->next_to_insert = next.second;
    res->candidat = candidat;
}

// class

NextPreprocessorToInsert * PreprocessorInserter::evaluateInheritedAttribute(SgNode * astNode, NextPreprocessorToInsert * inheritedValue) {
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

SagePreprocessorRecord::SagePreprocessorRecord(clang::SourceManager * source_manager) :
  p_source_manager(source_manager),
  p_preprocessor_record_list()
{
   Rose::Diagnostics::initAndRegister(&logger, "SagePreprocessorRecord"); 
   Rose::Diagnostics::mprefix->showProgramName(false);
   Rose::Diagnostics::mprefix->showThreadId(false);
   Rose::Diagnostics::mprefix->showElapsedTime(false);
}

void SagePreprocessorRecord::InclusionDirective(clang::SourceLocation HashLoc, const clang::Token & IncludeTok, llvm::StringRef FileName, bool IsAngled,
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

void SagePreprocessorRecord::EndOfMainFile() {
    logger[INFO] << "EndOfMainFile" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::Ident(clang::SourceLocation Loc, const std::string & str) {
    logger[INFO] << "Ident" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::PragmaComment(clang::SourceLocation Loc, const clang::IdentifierInfo * Kind, const std::string & Str) {
    logger[INFO] << "PragmaComment" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::PragmaMessage(clang::SourceLocation Loc, llvm::StringRef Str) {
    logger[INFO] << "PragmaMessage" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::PragmaDiagnosticPush(clang::SourceLocation Loc, llvm::StringRef Namespace) {
    logger[INFO] << "PragmaDiagnosticPush" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::PragmaDiagnosticPop(clang::SourceLocation Loc, llvm::StringRef Namespace) {
    logger[INFO] << "PragmaDiagnosticPop" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::PragmaDiagnostic(clang::SourceLocation Loc, llvm::StringRef Namespace, clang::diag::Severity Severity, llvm::StringRef Str) {
    logger[INFO] << "PragmaDiagnostic" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::MacroExpands(const clang::Token & MacroNameTok, const clang::MacroInfo * MI, clang::SourceRange Range) {
    logger[INFO] << "MacroExpands" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::MacroDefined(const clang::Token & MacroNameTok, const clang::MacroInfo * MI) {
    logger[INFO] << "" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::MacroUndefined(const clang::Token & MacroNameTok, const clang::MacroInfo * MI) {
    logger[INFO] << "MacroUndefined" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::Defined(const clang::Token & MacroNameTok) {
    logger[INFO] << "Defined" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::SourceRangeSkipped(clang::SourceRange Range) {
    logger[INFO] << "SourceRangeSkipped" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::If(clang::SourceRange Range) {
    logger[INFO] << "If" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::Elif(clang::SourceRange Range) {
    logger[INFO] << "Elif" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::Ifdef(const clang::Token & MacroNameTok) {
    logger[INFO] << "Ifdef" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::Ifndef(const clang::Token & MacroNameTok) {
    logger[INFO] << "Ifndef" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::Else() {
    logger[INFO] << "Else" << "\n";
    ROSE_ABORT();
}

void SagePreprocessorRecord::Endif() {
    logger[INFO] << "Endif" << "\n";
    ROSE_ABORT();
}

std::pair<Sg_File_Info *, PreprocessingInfo *> SagePreprocessorRecord::top() {
    return p_preprocessor_record_list.front();
}

bool SagePreprocessorRecord::pop() {
    p_preprocessor_record_list.erase(p_preprocessor_record_list.begin());
    return !p_preprocessor_record_list.empty();
}
