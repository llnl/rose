
#include "edgRose.h"

using namespace std;
using namespace Rose;
using namespace SageInterface;
using namespace SageBuilder;
using namespace OmpSupport;

bool source_range_null(a_source_range r)
   {
     return r.start.seq == 0 && r.end.seq == 0;
   }

bool 
is_valid_source_sequence_value(a_source_position & edg_source_position )
   {
  // DQ (11/14/2016): This function is to support a work around in EDG 4.11 and 4.12 
  // which both appear to have source sequence values that are out of range.

     bool returnValue = true;

     a_source_file_ptr curr_file = il_header.primary_source_file;

     ROSE_ASSERT(curr_file != NULL);
     ROSE_ASSERT(edg_source_position.seq >= curr_file->first_seq_number);

#if 0
     printf ("In is_valid_source_sequence_value(): edg_source_position.seq = %u \n",edg_source_position.seq);
#endif
#if 0
  // DQ (11/14/2016): debugging info.
     printf("In is_valid_source_sequence_value(): edg_source_position.seq = %u curr_file->first_seq_number = %u curr_file->last_seq_number = %u \n",
          edg_source_position.seq,curr_file->first_seq_number,curr_file->last_seq_number);
#endif

     unsigned int max_alowable_value = curr_file->last_seq_number;

  // DQ (11/14/2016): Need to implement better support for this test.
     if (edg_source_position.seq > max_alowable_value)
        {
          returnValue = false;
        }

     return returnValue;
   }


Sg_File_Info*
make_compiler_file_info()
   {
     Sg_File_Info* fileInfo = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
     ROSE_ASSERT(fileInfo != NULL);

     ROSE_ASSERT(fileInfo->isTransformation() == false);

#if 0
     printf ("In make_compiler_file_info(): fileInfo = %p \n",fileInfo);
#endif

     return fileInfo;
   }

Sg_File_Info*
make_compiler_file_info_to_unparse()
   {
     Sg_File_Info* result = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();

     result->setOutputInCodeGeneration();

     ROSE_ASSERT(result->isTransformation() == false);

#if 0
     printf ("In make_compiler_file_info_to_unparse(): result = %p \n",result);
#endif

     return result;
   }

Sg_File_Info*
convert_source_position(const a_source_position & pos)
   {
  // DQ (10/20/2015): Note that EDG 4.9 changes the function argument types to conv_seq_to_file_and_line()
  // And makes the typedef represented by a_const_char be "const char" for C++11 and "char" for C++98.
     a_const_char* file_name_ptr_memory_location = NULL;
     a_const_char* full_name_ptr_memory_location = NULL;
     a_const_char** file_name = &file_name_ptr_memory_location;
     a_const_char** full_name = &full_name_ptr_memory_location;

     a_line_number ln = 0;
     a_boolean at_end = FALSE;

#if 0
     printf ("In convert_source_position(): pos = %p \n",&pos);
#endif

     conv_seq_to_file_and_line(pos.seq, file_name, full_name, &ln, &at_end);

     ROSE_ASSERT ((*file_name != NULL) || (*full_name != NULL));

  // DQ (12/4/2012): If we detect an empty filename then fill it with a valid name.
  // This is only required if we are not correctly interpreting this as a source 
  // position of a compiler generated node.
     string name = (*file_name != NULL) ? *file_name : *full_name;
     if (name.empty() == true)
        {
       // printf ("WARNING: In convert_source_position(): detected an empty file name: pos.seq = %" PRIuPTR " \n",pos.seq);
          printf ("WARNING: In convert_source_position(): detected an empty file name: pos.seq = %lu \n",(unsigned long)pos.seq);
          name = "compiler_generated_in_convert_source_position";

          printf ("ERROR: In convert_source_position(): making it an error to detect an empty file name \n");
          ROSE_ABORT();
        }

     Sg_File_Info* fileInfo = new Sg_File_Info(name, ln, pos.column);

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

#if 0
     printf ("In convert_source_position(): fileInfo = %p \n",fileInfo);
     printf ("In convert_source_position(): file_name = %s full_name = %s \n",*file_name,*full_name);
#endif
#if 0
     printf ("In convert_source_position(): calling ROSE::stripPathFromFileName(file_name) \n");
  // printf ("In convert_source_position(): stripPathFromFileName(file_name) = %s \n",ROSE::stripPathFromFileName(string(*file_name)).c_str());
     printf ("In convert_source_position(): stripPathFromFileName(file_name) = %s \n",StringUtility::stripPathFromFileName(string(*file_name)).c_str());
     printf ("In convert_source_position(): calling StringUtility::stripPathFromFileName(file_name) \n");
     printf ("In convert_source_position(): stripPathFromFileName(file_name) = %s \n",StringUtility::stripPathFromFileName(string(*file_name)).c_str());
#endif

  // DQ (12/17/2012): Added assertion.
  // ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

  // DQ (2/17/2013): This should be a cheaper alternative implementation to marking source position as being front-end specific.
     static int frontendSpecificFileId = 0;

  // Get the integer id for the current file.
     int currentFileId = fileInfo->get_file_id();

  // Do the string compare only once at the start of the AST construction.
     if ((frontendSpecificFileId == 0) && StringUtility::stripPathFromFileName(string(*file_name)) == string("rose_edg_required_macros_and_functions.h"))
        {
          frontendSpecificFileId = currentFileId;
        }

  // Make this an integer comparision instead of a string compare.
     if (currentFileId == frontendSpecificFileId)
        {
       // printf ("Mark as front-end specific \n");
          fileInfo->setFrontendSpecific();
        }

     ROSE_ASSERT(fileInfo->isTransformation() == false);

  // DQ (12/18/2012): Report the physical file and line number.
     a_seq_number  seq_number                = pos.seq;
     a_line_number physical_line_number      = 0;
     a_boolean     physical_at_end_of_source = FALSE;
     a_boolean     physical_line             = TRUE;

  // DQ (1/23/2013): Added support for source sequence numbering.
     unsigned int source_sequence_integer = (unsigned int) seq_number;
     fileInfo->set_source_sequence_number(source_sequence_integer);

#if 0
     printf ("In convert_source_position(): fileInfo->get_source_sequence_number() = %" PRIuPTR " \n",fileInfo->get_source_sequence_number());
#endif

     if (seq_number != 0)
        {
          a_source_file_ptr source_file_pointer = source_file_for_seq(seq_number,&physical_line_number,&physical_at_end_of_source,physical_line);
#if 0
          printf ("In convert_source_position(): seq_number  = %d \n",(int)seq_number);
          printf ("In convert_source_position(): source_file = %p \n",source_file_pointer);
#endif
          if (source_file_pointer != NULL)
             {
               string physical_file_name       = source_file_pointer->file_name;
               string physical_full_name       = source_file_pointer->full_name;
               string physical_name_as_written = source_file_pointer->name_as_written;
#if 0
               printf ("In convert_source_position(): physical_file_name        = %s \n",physical_file_name.c_str());
               printf ("In convert_source_position(): physical_full_name        = %s \n",physical_full_name.c_str());
               printf ("In convert_source_position(): physical_name_as_written  = %s \n",physical_name_as_written.c_str());
               printf ("In convert_source_position(): physical_line_number      = %d \n",(int)physical_line_number);
               printf ("In convert_source_position(): physical_at_end_of_source = %s \n",physical_at_end_of_source ? "true" : "false");
#endif
               fileInfo->set_physical_filename(physical_file_name);
               fileInfo->set_physical_line(physical_line_number);
#if 0
               printf ("In convert_source_position(): fileInfo->get_physical_filename() = %s \n",fileInfo->get_physical_filename().c_str());
#endif
               ROSE_ASSERT(fileInfo->get_physical_filename() == physical_file_name);
             }
            else
             {
               printf ("Skipping initialization of physical position for source code position (source_file_pointer == NULL) fileInfo = %p \n",fileInfo);
               ROSE_ASSERT(false);
             }
        }
       else
        {
          printf ("Skipping initialization of physical position for source code position (seq_number == 0) fileInfo = %p \n",fileInfo);
          ROSE_ASSERT(false);
        }

#if 0
     fileInfo->display("Leaving convert_source_position(): debug");
#endif

     return fileInfo;
   }


void
get_source_range_for_declaration_position_supplement(a_decl_position_supplement_ptr sup, a_source_position & start, a_source_position & end)
   {
#if DEBUG_SOURCE_POSITION
     printf ("In get_source_range_for_declaration_position_supplement() \n");
#endif

     ROSE_ASSERT(sup != NULL);
     if (!source_range_null(sup->identifier_range))
        {
#if 0
          printf ("Setting source position using the source_range_null(sup->identifier_range) \n");
#endif
          start = std::min(start, sup->identifier_range.start);
          end   = std::max(end, sup->identifier_range.end);
        }

     if (!source_range_null(sup->specifiers_range))
        {
#if 0
          printf ("Setting source position using the source_range_null(sup->specifiers_range) \n");
#endif
          start = std::min(start, sup->specifiers_range.start);
          end   = std::max(end, sup->specifiers_range.end);
        }

     if (!source_range_null(sup->variant.declarator_range))
        {
#if 0
          printf ("Setting source position using the source_range_null(sup->variant.declarator_range) \n");
#endif
          start = std::min(start, sup->variant.declarator_range.start);
          end   = std::max(end, sup->variant.declarator_range.end);
        }

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());
   }


void
get_source_range_for_declaration(a_source_correspondence sc, a_source_position & start, a_source_position & end)
   {
#if DEBUG_SOURCE_POSITION
     printf ("In get_source_range_for_declaration() \n");
#endif

     start = sc.decl_position;
     end   = sc.decl_position;
     a_decl_position_supplement_ptr sup = sc.decl_pos_info;

#if 0
     printf ("In get_source_range_for_declaration(): a_decl_position_supplement_ptr sup = %p \n",sup);
#endif

     if (sup != NULL)
        {
          get_source_range_for_declaration_position_supplement(sup,start,end);
        }
   }

SgInitializedName*
set_file_info_for_decl_pos(SgInitializedName* e, a_decl_position_supplement_ptr sc)
   {
  // FIXME: use the a_decl_position_supplement information better

#if DEBUG_SOURCE_POSITION
     printf ("In set_file_info_for_decl_pos(SgInitializedName* e = %p sc = %p) \n",e,sc);
#endif

     if (sc == NULL)
        {
          set_generated_file_info_to_unparse(e);
        }
       else
        {
       // DQ (12/4/2012): Fixed to handle cases of compiler generated IR nodes.
       // DQ (5/2/2012): Added support for setting the endOfConstruct (which was previously left NULL), 
       // this provides consistant support in ROSE for all SgLocatedNode objects.
          bool compiler_generated = (sc->identifier_range.start.seq == 0);
          Sg_File_Info* fi_start = compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(sc->identifier_range.start);
          Sg_File_Info* fi_end   = compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(sc->identifier_range.end);

          selectivelyReplaceSourcePositionInformation(e,fi_start,fi_end);
        }

     ROSE_ASSERT(e->get_startOfConstruct() != NULL);
     ROSE_ASSERT(e->get_endOfConstruct()   != NULL);

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);
     ROSE_ASSERT(e->get_endOfConstruct()  ->isTransformation() == false);

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

     return e;
   }

#define DEBUG__set_file_info_end DEBUG_SOURCE_POSITION

SgLocatedNode*
set_file_info_end(SgLocatedNode* locatedNode, const a_source_position & end_pos)
   {
  // DQ (9/1/2013): Function added to support setting the end of construct (used in resetting the 
  // end position of the SgAggregateInitializer from the where the variable declaration if built 
  // (using the a_variable_ptr) after the initializer is constructed.  The source position is only 
  // available in the a_variable structure in EDG, and not in the initializer structure.

     ROSE_ASSERT(locatedNode != NULL);
#if DEBUG__set_file_info_end
     printf ("Enter set_file_info_end()\n");
     printf ("  locatedNode = %p = %s\n", locatedNode, locatedNode->class_name().c_str());
     printf ("  end_pos.seq = %d\n", end_pos.seq);
#endif

     bool compiler_generated = (end_pos.seq == 0);
     Sg_File_Info* fi_end   = compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(end_pos);

     selectivelyReplaceSourcePositionInformation(locatedNode,NULL,fi_end);

  // DQ (9/1/2013): Note that we can (and should) still test both the starting and ending positions.
     ROSE_ASSERT(locatedNode->get_startOfConstruct() != NULL);
     ROSE_ASSERT(locatedNode->get_endOfConstruct()   != NULL);

     ROSE_ASSERT(locatedNode->get_startOfConstruct()->isTransformation() == false);
     ROSE_ASSERT(locatedNode->get_endOfConstruct()  ->isTransformation() == false);

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

#if DEBUG__set_file_info_end
     printf ("Leave set_file_info_end()\n");
#endif
     return locatedNode;
   }

// DQ (5/13/2012): Added function to access parst of the source code position from EDG.
string
source_position_filename(a_source_position pos)
   {
// DQ (2/13/2016): Fixing the EDG 4.8 support (because we are testing it and I would like to see the tests pass, but we are not generally interested in supporting it).
  // DQ (10/20/2015): Note that EDG 4.9 changes the function argument types to conv_seq_to_file_and_line()
  // And makes the typedef represented by a_const_char be "const char" for C++11 and "char" for C++98.
     a_const_char* file_name_ptr_memory_location = NULL;
     a_const_char* full_name_ptr_memory_location = NULL;
     a_const_char** file_name = &file_name_ptr_memory_location;
     a_const_char** full_name = &full_name_ptr_memory_location;

     a_line_number ln = 0;
     a_boolean at_end = FALSE;

  // conv_seq_to_file_and_line(pos.seq, &file_name, &full_name, &ln, &at_end);
     conv_seq_to_file_and_line(pos.seq, file_name, full_name, &ln, &at_end);

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

     return *file_name;
   }

// DQ (5/13/2012): Added function to access part of the source code position from EDG.
int
source_position_line_number(a_source_position pos)
   {
// DQ (2/13/2016): Fixing the EDG 4.8 support (because we are testing it and I would like to see the tests pass, but we are not generally interested in supporting it).
  // DQ (10/20/2015): Note that EDG 4.9 changes the function argument types to conv_seq_to_file_and_line()
  // And makes the typedef represented by a_const_char be "const char" for C++11 and "char" for C++98.
     a_const_char* file_name_ptr_memory_location = NULL;
     a_const_char* full_name_ptr_memory_location = NULL;
     a_const_char** file_name = &file_name_ptr_memory_location;
     a_const_char** full_name = &full_name_ptr_memory_location;

     a_line_number ln = 0;
     a_boolean at_end = FALSE;

  // conv_seq_to_file_and_line(pos.seq, &file_name, &full_name, &ln, &at_end);
     conv_seq_to_file_and_line(pos.seq, file_name, full_name, &ln, &at_end);

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

     return ln;
   }

// DQ (5/13/2012): Added function to access parst of the source code position from EDG.
int
source_position_column_number(a_source_position pos)
   {
     return pos.column;
   }


void
check_end_of_construct(an_il_entry_kind orig_kind, void* orig_ptr, SourceSequenceList& sse)
   {
     ROSE_ASSERT (sse->entity.kind == iek_src_seq_end_of_construct);
     a_src_seq_end_of_construct_ptr p = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);

#if 0
     printf ("In check_end_of_construct(): associated entry sse = %p = %s \n",p->entity.ptr,summarize_entry_kind_in_string_kind(p->entity).c_str());
#endif

     if (p->entity.ptr != orig_ptr)
        {
       // ERROR_MESSAGE(stderr, "In check_end_of_construct(): Found end of %p, expecting %p\n",p->entity.ptr,orig_ptr);
#ifdef DEBUG_CONVERSION
          printf ("In check_end_of_construct(): associated entry sse = %p = %s \n",p->entity.ptr,summarize_entry_kind_in_string_kind(p->entity).c_str());
          printf ("ERROR: In check_end_of_construct(): Found end of %p, expecting %p\n",p->entity.ptr,orig_ptr);
#endif
        }
   }


bool
resetSourcePostion(Sg_File_Info* existing_fi, Sg_File_Info* new_fi)
   {
  // DQ (10/30/2012): This function isolates the details of the logic behind the selection of either 
  // the existing or the new source code position information to be used at any IR node.  Ultimately, 
  // this function can be reduced to a simple logical expression. But in the mean time this better 
  // supports debugging.

#if 0
     if (existing_fi == NULL)
          printf ("In resetSourcePostion() \n");
       else
          printf ("In resetSourcePostion() for existing_fi->get_parent() = %p = %s \n",existing_fi->get_parent(),(existing_fi->get_parent() != NULL) ? existing_fi->get_parent()->class_name().c_str() : "null");
#endif

     ROSE_ASSERT(new_fi != NULL);

     bool resetSourcePosition_value = false;

     if (existing_fi != NULL)
        {
#if 0
       // printf ("resetSourcePostion(): Found an existing source code position (startOfConstruct): remove it \n");
          existing_fi->display("resetSourcePostion(): compute source code position information replacement logic: existing_fi: debug");
#endif

          bool poor_quality_existing_fi_predicate = ( // (existing_fi->isCompilerGenerated() == true) || 
                                                      (existing_fi->isSourcePositionUnavailableInFrontend() == true) ||
                                                      (existing_fi->get_file_id() < 0) );
                                                   // (existing_fi->get_filenameString().empty() == true) );

          bool good_quality_new_fi_predicate      = ( (new_fi->isCompilerGenerated()      == true) ||
                                                   // (new_fi->isSourcePositionUnavailableInFrontend() == true) ||
                                                      (new_fi->get_file_id() >= 0) );
                                                   // (new_fi->get_filenameString().empty() == true) );

          if ( poor_quality_existing_fi_predicate == true )
             {
            // The existing information is not very good, if the new info is better then use it instead.
               if ( good_quality_new_fi_predicate == true )
                  {
                 // Use the better source position information to replace the compiler generated information.
                    resetSourcePosition_value = true;
                  }
                 else
                  {
                 // Nothing to do here
                    resetSourcePosition_value = false;
                  }
             }
            else
             {
            // The existing information is valid and obtained from as low a level as possible, so it should not be replaced.
               if ( good_quality_new_fi_predicate == true )
                  {
                 // Don't overwrite good source position information.
                 // resetSourcePosition_value = false;

                 // DQ (1/23/2013): we need a better test.
                 // resetSourcePosition_value = true;
                    if (existing_fi->get_file_id() >= 0 && new_fi->isCompilerGenerated() == true)
                       {
                      // Don't replace a better source position with a compiler generated setting.
                      // See test2013_28.C where explicit const ref cast is over written to be marked as compiler generated (thus implicit).
                         resetSourcePosition_value = false;
                       }
                      else
                       {
                         resetSourcePosition_value = true;
                       }
                  }
                 else
                  {
                 // Some arbitration might be required here (give preference to the existing source position since it should be lower level).
                 // Alternatively, the newer one is maybe more accurate (non NULL FILE) node.
                    resetSourcePosition_value = false;
                  }
             }
        }
       else
        {
       // If there is no existing source position information then add some.
          resetSourcePosition_value = true;
        }

#if 0
     new_fi->display("resetSourcePostion(): compute source code position information replacement logic: new_fi: debug");
#endif

  // DQ (2/26/2019): If resetting the source position information to just a different file, then record the first file's id as an id to use in unparsing.
     if (resetSourcePosition_value == true)
        {
          if (existing_fi != NULL)
             {
               if ( (existing_fi->isSourcePositionUnavailableInFrontend() == false) && (existing_fi->get_file_id() >= 0) )
                  {
                    ROSE_ASSERT(new_fi->get_file_id() >= 0);
                    ROSE_ASSERT(new_fi->isSourcePositionUnavailableInFrontend() == false);

                 // The existing_fi may be associated with a valid source position from a different file.
                    if (existing_fi->get_file_id() != new_fi->get_file_id())
                       {
                         SgNode* parent = existing_fi->get_parent();
#if 0
                         printf ("Found overwrite of source file position for non-declaration: \n");
                         printf (" --- parent                                         = %p = %s \n",parent,parent->class_name().c_str());
                         SgDeclarationStatement* declarationStatement = isSgDeclarationStatement(parent);
                         if (declarationStatement != NULL)
                            {
                              bool is_defining_declaration = (declarationStatement == declarationStatement->get_definingDeclaration());
                              printf (" --- is_defining_declaration                        = %s \n",is_defining_declaration ? "true" : "false");
                            }
                         printf (" --- new_fi->get_filenameString()                           = %s \n",new_fi->get_filenameString().c_str());
                         printf (" --- --- new_fi                                             = %p \n",new_fi);
                         printf (" --- --- new_fi->get_file_id()                              = %d \n",new_fi->get_file_id());
                         printf (" --- --- new_fi->get_line()                                 = %d \n",new_fi->get_line());
                         printf (" --- --- new_fi->get_fileIDsToUnparse().size()              = %zu \n",new_fi->get_fileIDsToUnparse().size());
                         printf (" --- --- new_fi->get_fileLineNumbersToUnparse().size()      = %zu \n",new_fi->get_fileLineNumbersToUnparse().size());
                         printf (" --- existing_fi->get_filenameString()                      = %s \n",existing_fi->get_filenameString().c_str());
                         printf (" --- --- existing_fi                                        = %p \n",existing_fi);
                         printf (" --- --- existing_fi->get_file_id()                         = %d \n",existing_fi->get_file_id());
                         printf (" --- --- existing_fi->get_line()                            = %d \n",existing_fi->get_line());
                         printf (" --- --- existing_fi->get_fileIDsToUnparse().size()         = %zu \n",existing_fi->get_fileIDsToUnparse().size());
                         printf (" --- --- existing_fi->get_fileLineNumbersToUnparse().size() = %zu \n",existing_fi->get_fileLineNumbersToUnparse().size());
#endif
                      // Check that these are in sync.
                         if (new_fi->get_fileIDsToUnparse().size() != new_fi->get_fileLineNumbersToUnparse().size())
                            {
                              printf ("Error: fileIDsToUnparse and fileLineNumbersToUnparse lists are out of sync: \n");
                              printf (" --- new_fi->get_fileIDsToUnparse().size() != new_fi->get_fileLineNumbersToUnparse().size(): new_fi->get_fileIDsToUnparse().size()         = %zu \n",
                                   new_fi->get_fileIDsToUnparse().size());
                              printf (" --- new_fi->get_fileIDsToUnparse().size() != new_fi->get_fileLineNumbersToUnparse().size(): new_fi->get_fileLineNumbersToUnparse().size() = %zu \n",
                                   new_fi->get_fileLineNumbersToUnparse().size());
                            }
                         ROSE_ASSERT(new_fi->get_fileIDsToUnparse().size() == new_fi->get_fileLineNumbersToUnparse().size());

                         if (existing_fi->get_fileIDsToUnparse().empty() == false)
                            {
                              ROSE_ASSERT(existing_fi->get_fileLineNumbersToUnparse().empty() == false);
                              if (new_fi->get_fileIDsToUnparse().empty() == true)
                                 {
                                   ROSE_ASSERT(new_fi->get_fileLineNumbersToUnparse().empty() == true);
#if 0
                                   printf ("Assigning non-empty existing_fi->get_fileIDsToUnparse() (and fileLineNumbersToUnparse) to empty new_fi->get_fileIDsToUnparse() (and fileLineNumbersToUnparse) \n");
#endif
                                   new_fi->get_fileIDsToUnparse()         = existing_fi->get_fileIDsToUnparse();
                                   new_fi->get_fileLineNumbersToUnparse() = existing_fi->get_fileLineNumbersToUnparse();
                                 }
                                else
                                 {
                                   printf ("Need to add existing_fi->get_fileIDsToUnparse() to non-empty new_fi->get_fileIDsToUnparse() \n");
                                   printf (" --- existing_fi->get_fileIDsToUnparse().size() = %zu \n",existing_fi->get_fileIDsToUnparse().size());
                                   printf (" --- new_fi->get_fileIDsToUnparse().size()      = %zu \n",new_fi->get_fileIDsToUnparse().size());
#if 1
                                   printf ("Exiting as a test: not supported yet \n");
                                   ROSE_ASSERT(false);
#endif
                                 }
                            }

                      // Add this existing_fi->get_file_id() to the list of file id's that will permit the assocated language construct to be unparsed.
                         new_fi->get_fileIDsToUnparse().push_back(existing_fi->get_file_id());
                         new_fi->get_fileLineNumbersToUnparse().push_back(existing_fi->get_line());

                      // DQ (2/27/2019): Mark this as a shared IR node.
                         new_fi->setShared();

                      // Keep these in sync.
                         if (new_fi->get_fileIDsToUnparse().size() != new_fi->get_fileLineNumbersToUnparse().size())
                            {
                              printf ("Error: fileIDsToUnparse and fileLineNumbersToUnparse lists are out of sync: \n");
                              printf (" --- new_fi->get_fileIDsToUnparse().size() != new_fi->get_fileLineNumbersToUnparse().size(): new_fi->get_fileIDsToUnparse().size()         = %zu \n",
                                   new_fi->get_fileIDsToUnparse().size());
                              printf (" --- new_fi->get_fileIDsToUnparse().size() != new_fi->get_fileLineNumbersToUnparse().size(): new_fi->get_fileLineNumbersToUnparse().size() = %zu \n",
                                   new_fi->get_fileLineNumbersToUnparse().size());
                            }
                         ROSE_ASSERT(new_fi->get_fileIDsToUnparse().size() == new_fi->get_fileLineNumbersToUnparse().size());
                       }
                  }
             }
        }

#if 0
     printf ("Leaving resetSourcePostion(): resetSourcePosition_value = %s \n",resetSourcePosition_value ? "true" : "false");
#endif

     return resetSourcePosition_value;
   }

SgNamespaceDefinitionStatement*
set_file_info_for_seconary_namespace_definition(SgNamespaceDefinitionStatement* stmt, a_decl_position_supplement_ptr decl_pos_info)
   {
     ROSE_ASSERT(stmt != NULL);

#if DEBUG_SOURCE_POSITION || 0
     printf ("In set_file_info_for_seconary_namespace_definition(stmt = %p = %s decl_pos_info = %p) \n",stmt,stmt->class_name().c_str(),decl_pos_info);
#endif

     ROSE_ASSERT(decl_pos_info != NULL);

     bool compiler_generated = (decl_pos_info->variant.namespace_definition_range.start.seq == 0);

     Sg_File_Info* fi_start = compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(decl_pos_info->variant.namespace_definition_range.start);
     Sg_File_Info* fi_end = compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(decl_pos_info->variant.namespace_definition_range.end);
     selectivelyReplaceSourcePositionInformation(stmt,fi_start,fi_end);

     ROSE_ASSERT(stmt->get_startOfConstruct()->isTransformation() == false);

     return stmt;
   }




#ifdef DEBUG_CONVERSION
// Class to handle the indentation of debug information.
ScopedIndent::ScopedIndent(int& i): indent(i)
   {
     ++indent;
   }

ScopedIndent::~ScopedIndent()
   {
     --indent;
     debug_print("%2d %*s-\n", indent, 2*indent, "");
   }
#endif


