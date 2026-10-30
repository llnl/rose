
#include "edgRose.h"

void fe_early_init(void);
void fe_one_time_init(void);

void fe_wrapup(void);
void fe_wrapup_part_2(void);
void fe_cleanup(void);

namespace EDG_ROSE_Translation {

void analyse_header_files(SgSourceFile & sageFile, const an_il_header & header) {

  using namespace EDG_ROSE_Translation;

  // DQ (9/17/2018): Output information about the primary source file.
  // The description of the primary source file, and linkage to include file information.  In
  // the front end, when there are secondary translation units, this is a list of the top-level 
  // files, one for each translation unit.
     a_source_file_ptr primary_source_file = header.primary_source_file;
     ROSE_ASSERT(primary_source_file != NULL);

#if 0
  // DQ (9/17/2018): Added to support debugging of source file details.
     printf ("In sage_back_end(): calling summarize_source_file() \n");
     summarize_source_file(primary_source_file);
#endif

#if 0
  // DQ (6/15/2019): Output a message so that we can know the DOT file is being output.
  // Build a graph of the header files used and where they are included from.
     printf ("Output DOT file for graph headers \n");
     EDG_ROSE_Graph::graph_headers();
#endif
    
  // DQ (9/17/2018): Build a tree data structure that represents the header file tree.
  // This information will be used in the header file unparsing to note the differences 
  // between the header files that are listed in include directives, vs the header files 
  // that are included to form the AST.
  // DQ (8/18/2019): Add performance analysis support.
     TimingPerformance timer ("EDG-ROSE header file support in AST generation:");

     EDG_ROSE_Translation::build_header_file_tree(&sageFile,NULL,primary_source_file);
#if 0
     printf ("Number of include files processed: EDG_ROSE_Translation::edg_include_file_map.size() = %zu \n",EDG_ROSE_Translation::edg_include_file_map.size());
#endif

     if (SgProject::get_unparseHeaderFilesDebug() >= 2)
        {
            // DQ (4/28/2021): This is the correct default behavior. It is OK for the global scope to be 
            // shared because there are seperate lists (by file) which store the token sequence mapping.
            // set<SgGlobal*> globalScopeSet;

               printf ("EDG_ROSE_Translation::edg_include_file_map: (size = %zu) \n",EDG_ROSE_Translation::edg_include_file_map.size());
               for (std::map<std::string, SgIncludeFile*>::iterator i = EDG_ROSE_Translation::edg_include_file_map.begin(); 
                    i != EDG_ROSE_Translation::edg_include_file_map.end(); i++)
                  {
                    std::string filename = i->first;
                    SgIncludeFile* includeFile = i->second;
                    printf (" --- filename = %s includeFile = %p \n",filename.c_str(),includeFile);
#if 1
                    ROSE_ASSERT(includeFile != NULL);
#if 0
                    SgSourceFile* include_source_file = includeFile->get_source_file();

                    if (include_source_file != NULL)
                       {
                         if (globalScopeSet.find(include_source_file->get_globalScope()) == globalScopeSet.end())
                            {
                              globalScopeSet.insert(include_source_file->get_globalScope());
                            }
                           else
                            {
                           // DQ (4/28/2021): This is the correct default behavior. It is OK for the global scope to be 
                           // shared because there are seperate lists (by file) which store the token sequence mapping.
                              printf ("Error: global scope has been reused across multiple header files \n");
                              printf ("   --- include_source_file = %p filename = %s \n",include_source_file,include_source_file->getFileName().c_str());
                              printf ("   --- include_source_file->get_globalScope() = %p \n",include_source_file->get_globalScope());

                           // DQ (4/10/2021): This is allowed for the case where we have not yet fixed the global scope reuse across include files (this is the current default).
                           // ROSE_ABORT();
                            }
                       }
#endif
#if 0
                    if (include_source_file != NULL)
                       {
                      // DQ (3/28/2021): The source files will all be different (as it should be), but the global scopes are all the same.
                         printf (" --- include_source_file                    = %p filename = %s \n",include_source_file,include_source_file->getFileName().c_str());
                         printf ("Test 4 \n");
                         printf (" --- sageFile.get_globalScope()             = %p \n",sageFile.get_globalScope());
                         printf (" --- include_source_file->get_globalScope() = %p \n",include_source_file->get_globalScope());
                         printf ("Test 5 \n");
                       }
#endif
#endif
#if 0
                    printf (" --- --- first_source_sequence_number = %d \n",includeFile->get_first_source_sequence_number());
                    printf (" --- --- last_source_sequence_number  = %d \n",includeFile->get_last_source_sequence_number());
#endif
                  }
#if 0
            // DQ (3/23/2021): This is a bit redundant.
               printf ("EDG_ROSE_Translation::edg_include_file_using_names_as_written_map: (size = %zu) \n",
                    EDG_ROSE_Translation::edg_include_file_using_names_as_written_map.size());
               for (std::map<std::string, SgIncludeFile*>::iterator i = EDG_ROSE_Translation::edg_include_file_using_names_as_written_map.begin(); 
                    i != EDG_ROSE_Translation::edg_include_file_using_names_as_written_map.end(); i++)
                  {
                    string filename = i->first;
                    SgIncludeFile* includeFile = i->second;
                    printf (" --- filename = %s includeFile = %p \n",filename.c_str(),includeFile);
                  }
#endif
        }
}

#define DEBUG__sage_back_end 0

void sage_back_end ( SgSourceFile & sageFile )
   {
  // DQ (12/29/2008): Note that the input SgSourceFile object has a valid SgGlobal (global 
  // scope object) already. So we really should not be resetting the global scope.

  using namespace EDG_ROSE_Translation;

  // DQ (2/17/2013): Introduce tracking of performance of EDG before processing to ROSE AST.
     TimingPerformance timer ("ROSE AST generation (EDG/ROSE connection):");
       
#if DEBUG__sage_back_end
     printf ("Enter sage_back_end()\n");
#endif

     ROSE_ASSERT(Rose::Diagnostics::isInitialized() == true);
     if (EDG_ROSE_Translation::mlog[Rose::Diagnostics::DEBUG])
        {
          mprintf ("In EDG/ROSE: message logging is ON \n");
        }

  // DQ (12/28/2015): Implementing initial support to suppress the normalization of variable declarations 
  // (currently variable declarations with more than one variable are normalized to one variable per variable declarations).
     global_suppress_variable_declaration_normalization = sageFile.get_suppress_variable_declaration_normalization();

     if (global_suppress_variable_declaration_normalization == true)
        {
          mprintf ("**************************************************************************** \n");
          mprintf ("Testing experimental mode to suppress normalization of variable declarations \n");
          mprintf ("**************************************************************************** \n\n");
        }

     if (sageFile.get_edg_il_to_graphviz()) 
        {
          EDG_ROSE_Graph::graph_edg_ast();
        }
#if 0
  // DQ (1/3/2014): Added to support debugging of additional macro details (now turned on in EDG).
     printf ("In sage_back_end(): calling summarize_il_header() \n");
     summarize_il_header(il_header);
#endif

     const an_il_header & header = il_header;
     a_scope_ptr main_scope = header.primary_scope;

     analyse_header_files(sageFile, header);

     EDG_ROSE_Translation::initialize(&sageFile);

     SourceSequenceList sse(main_scope->source_sequence_list);

     bool skip_ast_translation = sageFile.get_skip_translation_from_edg_ast_to_rose_ast();
     if (skip_ast_translation == true)
        {
          printf ("Warning: skipping translation of EDG AST to ROSE AST (sageFile.get_skip_translation_from_edg_ast_to_rose_ast() == true) \n");
        }

  // DQ (2/20/2012): We want to reuse the SgGlobal scope that is built in ROSE before we get to this point.
  // This fixes a bug where there were always two SgGlobal scopes built.
     SgGlobal* existingGlobalScope = sageFile.get_globalScope();
     ROSE_ASSERT(existingGlobalScope != NULL);

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

     SgGlobal* g = EDG_ROSE_Translation::parse_global_scope(existingGlobalScope,sse,skip_ast_translation);

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

  // DQ (12/29/2008): Exchange the source file info from the original global scope.
     Sg_File_Info* temp = g->get_file_info(); // This Sg_File_Info is incorrectly marked as compiler generated!

  // DQ (12/18/2012): Added assertion.
     ROSE_ASSERT(temp != NULL);

     g->set_file_info(sageFile.get_globalScope()->get_file_info());
     sageFile.get_globalScope()->set_file_info(temp);

  // DQ (12/18/2012): Set the physical source code position information.
     temp->set_physical_source_position_to_match_logical_source_position();

#if 0
     g->get_startOfConstruct()->display("sage_back_end: global scope: START: debug");
     g->get_endOfConstruct  ()->display("sage_back_end: global scope: END: debug");
#endif

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

  // Reset the SgGlobal (global scope object)
     sageFile.set_globalScope(g);
     g->set_parent(&sageFile);

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

     SgNode::get_globalFunctionTypeTable()->set_parent(g); // FIXME: What should this be set to?

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

#ifdef DEBUG_CONVERSION
     printf ("Leaving sage_back_end() \n");
#endif

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

  // DQ (11/25/2012): Moved this to be after the processing of each file.
  // DQ (11/17/2012): To support multiple files on teh command line I think we have to clear the 
  // numerous caches of edg pointers to ROSE IR node pointers.
     EDG_ROSE_Translation::clear_EDG_ROSE_caches();

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

#if 0
     printf ("In sage_back_end(): Calling TestAstForCyclesInTypedefs() \n");
     TestAstForCyclesInTypedefs::test();
#endif

  // DQ (10/27/2015): Moved from astPostProcessing: reset all of the type references (to intermediately generated types).
     fixupTypeReferences();

#if 0
     printf ("In sage_back_end(): Output the symbol table from the global scope \n");
     g->get_symbol_table()->print();
     printf ("In sage_back_end(): DONE: Output the symbol table from the global scope \n");
#endif

#if 0
     printf ("In sage_back_end(): Output the type table from the global scope \n");
     g->get_type_table()->print_typetable();
     printf ("In sage_back_end(): DONE: Output the type table from the global scope \n");
#endif

#if 0
     printf ("In sage_back_end(): Output the global type table from the global scope \n");
     g->get_globalTypeTable()->print_typetable();
     printf ("In sage_back_end(): DONE: Output the global type table from the global scope \n");
#endif

#if 0
     printf ("In sage_back_end(): Calling TestAstForCyclesInTypedefs() \n");
     TestAstForCyclesInTypedefs::test();
#endif

#if 0
     debug_print("About to unparse\n");
     sageFile.unparse();
     debug_print("%s\n", g->unparseToCompleteString().c_str());
     debug_print("Unparsed\n");
     abort();
#endif

#if 0
  // DQ (4/26/2013): Debugging code.
     printf ("In sage_back_end(): Calling testFunctionCallArguments() \n");
     testFunctionCallArguments(&sageFile);
#endif

#if 0
     printf ("In sage_back_end(): calling outputFileIds() \n");

  // DQ (12/2/2019): Added to support debugging.
     SageInterface::outputFileIds(&sageFile);

     printf ("DONE: In sage_back_end(): calling outputFileIds() \n");
#endif

#if 0
     printf ("*********************** \n");
     printf ("Leaving sage_back_end() \n");
     printf ("*********************** \n");
#endif

  // DQ (4/5/2020): Added header file unparsing feature specific debug level.
     if (SgProject::get_unparseHeaderFilesDebug() >= 4)
        {
          printf ("*********************** \n");
          printf ("Leaving sage_back_end() \n");
          printf ("*********************** \n");
        }
   }

void
cfe_main(int argc, char *argv[], SgSourceFile & sageFile)
   {
  // This routine does the actual work to perform a compilation.  This is
  // called by the EDG_MAIN wrapper that performs error handling when
  // MAKE_FRONT_END_CALLABLE is TRUE.
     an_error_severity most_severe_diagnostic = es_none, diagnostic_level;
     a_timer	    start_time;
     a_timer	    fe_start_time;
     a_timer	    fe_end_time;
     a_timer	    be_start_time;
     a_timer	    be_end_time;
     a_timer	    end_time;

#ifdef DEBUG_CONVERSION
     printf ("Inside of cfe_main(): sageFile.get_sourceFileNameWithPath() = %s \n",sageFile.get_sourceFileNameWithPath().c_str());
     printf ("Inside of cfe_main(): sageFile.get_globalScope()            = %p \n",sageFile.get_globalScope());
     printf ("Inside of cfe_main(): sageFile.get_exit_after_parser()      = %s \n",sageFile.get_exit_after_parser() ? "true" : "false");

     printf ("Inside of cfe_main(): Set the mode in SageBuilder to build IR nodes using SageBuilder::e_sourcePositionFrontendConstruction semantics. \n");
#endif

     SageBuilder::setSourcePositionClassificationMode(SageBuilder::e_sourcePositionFrontendConstruction);

#if DEBUG
  // Initialize the file variable used for debug output.  This should be done before anything else that could potentially produce debug output.
     f_debug = stderr;
#endif /* DEBUG */

  // Initialize the file variable used for error output.  This should be done before anything else 
  // that could potentially produce error output (including an internal error). 
     f_error = stderr;

  // Do early (before command-line processing) initialization.
     fe_early_init();

  // Get the execution starting time.  Do this unconditionally because the timing command line option 
  // will not have been processed yet.  This must be done after the early initialization done above. 
     get_timer(&start_time);

  // Process the command line.
     proc_command_line(argc, argv);

  // Initialize values that apply to the entire compilation if multiple files are allowed.
     fe_one_time_init();
        
        {
       // DQ (2/17/2013): Introduce tracking of performance of EDG before processing to ROSE AST.
          TimingPerformance timer ("EDG AST generation:");

       // Get the front end starting time.
          if (display_compilation_time) get_timer(&fe_start_time);
       // Initialize the per-compilation variables related to translation unit processing.
          trans_unit_init();
       // Process the source file.
          process_translation_unit(primary_source_file_name, /*is_primary=*/TRUE, (an_exported_template_file_ptr)NULL);
       // Do wrap-up processing for the front end (before the back end).

          fe_wrapup();

          if (display_compilation_time)
             {
            // Get the front end ending time.
               get_timer(&fe_end_time);
            // Display the amount of time used by the front end.
               display_time_used((char*)"Front end time", &fe_start_time, &fe_end_time);
             }  /* if */
        }

       // Run the back end if required.
#ifndef CALL_BACK_END_EVEN_WITH_ERRORS
       // Do not run the back end if there were errors.
          if (diagnostic_counters.total.all_error_types() != 0) suppress_back_end = TRUE;
#endif

       // DQ (2/17/2011): Added more uniform support for exiting after parsing option (permits some UPC EDG testing).
          if (sageFile.get_exit_after_parser())
               suppress_back_end = TRUE;

          if (!suppress_back_end)
             {
               if (display_compilation_time) get_timer(&be_start_time);

            // ROSE modification
               sage_back_end(sageFile);

               if (display_compilation_time)
                  {
                 // Get the back end ending time.
                    get_timer(&be_end_time);
                 // Display the amount of time used by the back end.
                    display_time_used((char*)"Back end time", &be_start_time, &be_end_time);
                  }
             }
       // Do wrap-up processing for the front end (after the back end).
          fe_wrapup_part_2();

       // Determine the most severe diagnostic encountered.  Catastrophic errors do
       // not get here and therefore need not be checked for. Remarks don't count.
          if (diagnostic_counters.total.all_error_types() != 0)
             {
               diagnostic_level = es_error;
             }
            else
             {
               if (diagnostic_counters.total.warnings != 0)
                  {
                    diagnostic_level = es_warning;
                  }
                 else
                  {
                    diagnostic_level = es_none;
                  }
             }

     most_severe_diagnostic = diagnostic_level;

     if (display_compilation_time)
        {
       // Get the ending time.
          get_timer(&end_time);
       // Display the amount of time used by the entire compilation.
          display_time_used((char*)"Total compilation time", &start_time, &end_time);
        }

  // DQ (5/2/2012): Reset SageBuilder so that future IR node construction will be represented as a transformation.
#ifdef DEBUG_CONVERSION
     printf ("Inside of cfe_main(): Reset the mode in SageBuilder to build IR nodes using SageBuilder::e_sourcePositionTransformation semantics. \n");
#endif

     SageBuilder::setSourcePositionClassificationMode(SageBuilder::e_sourcePositionTransformation);

#ifdef DEBUG_CONVERSION
     printf ("Leaving cfe_main(): global scope = %p \n",sageFile.get_globalScope());
#endif

  // Exit with the return code appropriate to the highest severity error detected.
     exit_compilation(most_severe_diagnostic);

  // NOTREACHED
   }

}

int edg_main(int argc, char *argv[], SgSourceFile & sageFile)
   {

     if (setjmp(edg_main_setjmp_buffer) == 0)
        {
          EDG_ROSE_Translation::cfe_main(argc, argv, sageFile);
        }

     fe_cleanup();
     return exit_status;
   }
