// Support for setting the source position information during the translation.
// these functions act as a filter to add the source position information to any
// IR node as it is passed in a parameter and returned as a value.

Sg_File_Info* make_compiler_file_info();
Sg_File_Info* make_compiler_file_info_to_unparse();

bool source_range_null(a_source_range r);

SgInitializedName* set_file_info_for_decl_pos(SgInitializedName* e, a_decl_position_supplement_ptr sc);

void get_source_range_for_declaration(a_source_correspondence sc, a_source_position& start, a_source_position& end);

void get_source_range_for_secondary_declaration(a_src_seq_secondary_decl_ptr secondary_decl, a_source_position & start, a_source_position & end);

void print_source_pos(a_source_position pos);
void print_source_corresp(a_source_correspondence sc);
void print_source_range(a_source_range r);
void check_end_of_construct(an_il_entry_kind orig_kind, void* orig_ptr, SourceSequenceList& sse);

// DQ (3/3/2013): Added more debug information.
void print_statement_position ( a_statement_ptr stmt );

// DQ (5/13/2012): Added function to access parst of the source code position from EDG.
std::string source_position_filename(a_source_position pos);
std::string source_position_filename_with_path(a_source_position pos);
int source_position_line_number(a_source_position pos);
int source_position_column_number(a_source_position pos);

// Template functions supporting the specification of source file position information...
template <typename Expr> static Expr* set_file_info_for_expr(Expr* e, an_expr_node_ptr orig_e);
template <typename Expr> static Expr* set_file_info_for_enumerator(Expr* e, a_constant_ptr c);
template <typename Stmt> static Stmt* set_file_info_for_stmt(Stmt* e, a_statement_ptr orig_e);
template <typename Node> static Node* set_file_info_for_corresp(Node* e, a_source_correspondence sc);
template <typename Node> static Node* set_file_info_for_corresp_and_end(Node* e, a_source_correspondence sc, const SourceSequenceList& end_entry);
template <typename Node> static Node* set_basic_file_info_for_expr(Node* e, an_expr_node_ptr orig_e);
template <typename Node> static Node* set_generated_file_info(Node* e);
template <typename Node> static Node* set_generated_file_info_to_unparse(Node* e);

// DQ (7/19/2020): Added to support EDG 6.0 source position for lambda expressions.
template <typename Expr> static Expr* set_starting_file_info_for_lambda(Expr* e, a_lambda_ptr orig_e);

// DQ (1/29/2015): Added better function for computing source position information (required for token-based unparsing).
SgNamespaceDefinitionStatement* set_file_info_for_seconary_namespace_definition(SgNamespaceDefinitionStatement* stmt, a_decl_position_supplement_ptr decl_pos_info);

// DQ (9/1/2013): This is added to support setting the end of the construct (when it exists in EDG).
SgLocatedNode* set_file_info_end(SgLocatedNode* locatedNode, const a_source_position & end_pos);

// Sg_File_Info* convert_source_position(a_source_position pos);
Sg_File_Info* convert_source_position(const a_source_position & pos);

// This function contains the logic for when existing source position information is replaced with new (better) source position information.
bool resetSourcePostion(Sg_File_Info* existing_fi, Sg_File_Info* new_fi);


#if USE_DEBUG_VERSION_OF_PRINTF
// DQ (8/24/2012): Move this from header file to the source file.
void debug_print(const char* format, ...) __attribute__((format(printf, 1, 2)));
#endif

// DQ (11/14/2016): Added to support bug fix in EDG 4.11 and 4.12.
bool is_valid_source_sequence_value(a_source_position & edg_source_position );

#define DEBUG_SOURCE_POSITION 0

#ifdef DEBUG_CONVERSION
struct ScopedIndent
   {
  // Support for debugging information to be output with indentation matching the scope within the AST.
     int & indent;
     ScopedIndent(int& i);
    ~ScopedIndent();
   };
#endif


// DQ (8/24/2012): Move this from header file to the source file.
// static inline bool operator< (a_source_position a, a_source_position b)
static inline bool operator< (a_source_position a, a_source_position b)
   {
  // This function is used to support std::min() and std::max().
  // It must be defined in global scope to compile with STL.

  // This operator evaluates the order in the source sequence list that 
  // EDG uses to describe the IR nodes that correspond to the source code.

  // Use the sequence numbers first
     if (a.seq < b.seq) return true;
     if (a.seq > b.seq) return false;

  // If the sequence numbers are equal, then use the column information.
  // Unclear why line numbers should not take a higher precedence than 
  // column numbers (unless they are equal when the sequence numbers are 
  // equal, this should be tested).
     return a.column < b.column;
   }


template <typename Node> void selectivelyReplaceSourcePositionInformation(Node* e, Sg_File_Info* fi_operator );

template <typename Node>
void selectivelyReplaceSourcePositionInformation(Node* e, Sg_File_Info* fi_operator )
   {
#if 0
      printf ("In selectivelyReplaceSourcePositionInformation(Node* e = %p = %s,fi_operator = %p) (operator version) \n",e,e->class_name().c_str(),fi_operator);
      printf ("   --- e->get_operatorPosition() = %p \n",e->get_operatorPosition());
#endif

  // DQ (10/20/2012): Simplified logic and refactored code to know when to replace exisiting information with new information.
     SgExpression* expression = isSgExpression(e);
     ROSE_ASSERT(expression != NULL);

  // DQ (11/2/2012): If there is no source code position then don't do any replacement.
     ROSE_ASSERT(expression->get_operatorPosition() != NULL);
     bool resetSourcePosition_operator = resetSourcePostion(expression->get_operatorPosition(),fi_operator);

#if 0
     printf ("In selectivelyReplaceSourcePositionInformation(Node*, Sg_File_Info*): resetSourcePosition_operator = %s \n",resetSourcePosition_operator ? "true" : "false");
#endif

     if (resetSourcePosition_operator == true)
        {
       // Overwrite or use the new source file information.
          if (expression->get_operatorPosition() != NULL)
             {
               delete expression->get_operatorPosition();
               expression->set_operatorPosition(NULL);
             }

          ROSE_ASSERT(fi_operator != NULL);
          expression->set_operatorPosition(fi_operator);
          fi_operator->set_parent(expression);
        }
       else
        {
       // Delete the input SgFileInfo object that we would have used since we don't want to replace 
       // the existing one (or this IR node does not use source position information, e.g. SgValueExp).

          delete fi_operator;
          fi_operator = NULL;
        }

     if (expression->get_operatorPosition()->get_file_id() >= 0)
        {
          ROSE_ASSERT(expression->get_operatorPosition()->get_physical_file_id() >= 0);
          ROSE_ASSERT(Sg_File_Info::get_fileidtoname_map().find(expression->get_operatorPosition()->get_physical_file_id()) != Sg_File_Info::get_fileidtoname_map().end());
        }
   }


template <typename Node> void selectivelyReplaceSourcePositionInformation(Node* e, Sg_File_Info* fi_start, Sg_File_Info* fi_end );

template <typename Node>
void selectivelyReplaceSourcePositionInformation(Node* e, Sg_File_Info* fi_start, Sg_File_Info* fi_end )
   {
     ROSE_ASSERT(e != NULL);

#if 0
     printf ("In selectivelyReplaceSourcePositionInformation(Node* e = %p = %s,fi_start =%p,fi_end = %p) \n",e,e->class_name().c_str(),fi_start,fi_end);
     printf ("   --- e->get_startOfConstruct() = %p \n",e->get_startOfConstruct());
     printf ("   --- e->get_endOfConstruct()   = %p \n",e->get_endOfConstruct());
#endif

  // DQ (11/2/2012): We should have source position info by default (might be "NULL_FILE" style).
     ROSE_ASSERT(e->get_startOfConstruct() != NULL);
     ROSE_ASSERT(e->get_endOfConstruct()   != NULL);

  // DQ (9/1/2013): Allow this function to be called with NULL pointers for either fi_start or fi_end so that it can be used to reset
  // only the end position in: SgLocatedNode* set_file_info_end(SgLocatedNode* locatedNode, const a_source_position & end_pos).
  // DQ (10/20/2012): Simplified logic and refactored code to know when to replace exisiting information with new information.
     bool resetSourcePosition_start = (fi_start == NULL) ? false : resetSourcePostion(e->get_startOfConstruct(),fi_start);
     bool resetSourcePosition_end   = (fi_end   == NULL) ? false : resetSourcePostion(e->get_endOfConstruct(),fi_end);

#if 0
     printf ("In selectivelyReplaceSourcePositionInformation(Node*,Sg_File_Info*,Sg_File_Info*): resetSourcePosition_start = %s resetSourcePosition_end = %s \n",resetSourcePosition_start ? "true" : "false",resetSourcePosition_end ? "true" : "false");
#endif

     if (resetSourcePosition_start == true)
        {
#if 0
          fi_start->display("In selectivelyReplaceSourcePositionInformation(Node*,Sg_File_Info*,Sg_File_Info*): fi_start: debug");
#endif
          if (e->get_startOfConstruct() != NULL)
             {
               delete e->get_startOfConstruct();
               e->set_startOfConstruct(NULL);
             }

          ROSE_ASSERT(fi_start != NULL);
          e->set_startOfConstruct(fi_start);
          fi_start->set_parent(e);
        }
       else
        {
       // NOTE: It is allowed to call delete on a NULL valued pointer.
          delete fi_start;
          fi_start = NULL;
        }

     if (resetSourcePosition_end == true)
        {
#if 0
          fi_end->display("In selectivelyReplaceSourcePositionInformation(Node*,Sg_File_Info*,Sg_File_Info*): fi_end: debug");
#endif
          if (e->get_endOfConstruct() != NULL)
             {
               delete e->get_endOfConstruct();
               e->set_endOfConstruct(NULL);
             }

          ROSE_ASSERT(fi_end != NULL);
          e->set_endOfConstruct(fi_end);
          fi_end->set_parent(e);
        }
       else
        {
       // NOTE: It is allowed to call delete on a NULL valued pointer.
          delete fi_end;
          fi_end = NULL;
        }

     if (e->get_startOfConstruct()->get_file_id() >= 0)
        {
          ROSE_ASSERT(e->get_startOfConstruct()->get_physical_file_id() >= 0);
          ROSE_ASSERT(Sg_File_Info::get_fileidtoname_map().find(e->get_startOfConstruct()->get_physical_file_id()) != Sg_File_Info::get_fileidtoname_map().end());

       // DQ (10/11/2016): Work around bug in EDG 4.12.
          if (e->get_endOfConstruct()->get_file_id() >= 0)
             {
               ROSE_ASSERT(Sg_File_Info::get_fileidtoname_map().find(e->get_endOfConstruct()->get_physical_file_id())   != Sg_File_Info::get_fileidtoname_map().end());
             }
        }
   }


// Set everything except the operator position so we can use this for expr statements.
template <typename Node>
Node* set_basic_file_info_for_expr(Node* e, an_expr_node_ptr orig_e)
   {
#if DEBUG_SOURCE_POSITION
     printf ("In set_basic_file_info_for_expr(Node* e = %p = %s orig_e = %p) \n",e,e->class_name().c_str(),orig_e);
#endif

  // DQ (9/10/2013): In the case of a lanbda expression there is only the starting position of the range, 
  // so we need to detect this case in EDG 4.7 and make the end match the start where this happens.
     bool compiler_generated_start = (orig_e->compiler_generated) || (orig_e->expr_range.start.seq == 0);
     bool compiler_generated_end   = (orig_e->compiler_generated) || (orig_e->expr_range.end.seq == 0);

     bool fixup_end_to_match_start  = (compiler_generated_start == false) && (compiler_generated_end == true);
     bool compiler_generated = compiler_generated_start;

#if 0
  // DQ (4/24/2013): This is another way that expressions can be marked as compiler generated.
     if (orig_e->generated_default_arg == true)
        {
          compiler_generated = true;
        }
#endif
#if 0
     if (compiler_generated == true)
        {
          printf ("In set_basic_file_info_for_expr(): Calling make_compiler_file_info_to_unparse() e = %p = %s \n",e,e->class_name().c_str());
        }
#endif

     Sg_File_Info* fi_start = compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(orig_e->expr_range.start);

  // DQ (9/10/2013): The new EDG 4.7 lambda support has only supported the start of a source position range in the expression, 
  // so we have to compinsate for this by detecing this special case and making the end a copy of the start.
  // Sg_File_Info* fi_end   = compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(orig_e->expr_range.end);
     Sg_File_Info* fi_end = NULL;
     if (fixup_end_to_match_start == true)
        {
          fi_end = compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(orig_e->expr_range.start);
        }
       else
        {
          fi_end = compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(orig_e->expr_range.end);
        }

     selectivelyReplaceSourcePositionInformation(e,fi_start,fi_end);

#if 0
     printf ("set_basic_file_info_for_expr(): Newly built source code position (startOfConstruct): \n");
     e->get_endOfConstruct()->display("set_basic_file_info_for_expr(): Newly built source code position (startOfConstruct): debug");
     printf ("set_basic_file_info_for_expr(): Newly built source code position (endOfConstruct): \n");
     e->get_endOfConstruct()->display("set_basic_file_info_for_expr(): Newly built source code position (endOfConstruct): debug");
#endif

  // DQ (4/24/2013): This is another way that expressions can be marked as compiler generated.
     if (orig_e->generated_default_arg == true)
        {
          e->get_startOfConstruct()->setDefaultArgument();
          e->get_endOfConstruct  ()->setDefaultArgument();
        }

     return e;
   }


template <typename Expr>
Expr* set_file_info_for_expr(Expr* e, an_expr_node_ptr orig_e)
   {
     bool compiler_generated = (orig_e->compiler_generated) || (orig_e->expr_range.start.seq == 0);


  // DQ (12/5/2012): We always want to use the best available information or fall back to having the operator position be a copy fo the startOfConstruct 
  // source position information.  So if the operator position information is unavaiable in EDG then don't consider this an operator and do the next best 
  // thing (builds a copy of the startOfConstruct source position information).  If the startOfConstruct is marked as compiler generated then this is fine,
  // but it need not be just because the operator position is not set in EDG.
     bool isOperator         = (orig_e->kind == enk_operation && orig_e->position.seq != 0);

#if DEBUG_SOURCE_POSITION
     printf ("In set_file_info_for_expr(e = %p = %s orig_e = %p) compiler_generated = %s isOperator = %s \n",e,e->class_name().c_str(),orig_e,compiler_generated ? "true" : "false",isOperator ? "true" : "false");
#endif

  // This sets the startOfConstruct and the endOfConstruct source position information.
     set_basic_file_info_for_expr(e, orig_e);

     Sg_File_Info* fi_operator = isOperator ? (compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(orig_e->position)) : new Sg_File_Info(*(e->get_startOfConstruct()));

     selectivelyReplaceSourcePositionInformation(e,fi_operator);

#if 0
     printf ("set_file_info_for_expr(): Newly built source code position (operatorPosition): \n");
     ROSE_ASSERT(e->get_operatorPosition() != NULL);
     e->get_operatorPosition()->display("set_file_info_for_expr(): Newly built source code position (operatorPosition): debug");
#endif

     ROSE_ASSERT(e->get_file_info()->isCompilerGenerated() == e->get_startOfConstruct()->isCompilerGenerated());

  // DQ (4/24/2013): This is another way that expressions can be marked as compiler generated.
     if (orig_e->generated_default_arg == true)
        {
          e->get_operatorPosition()->setDefaultArgument();
        }

     return e;
   }


template <typename Expr>
Expr* set_file_info_for_enumerator(Expr* e, a_constant_ptr c)
   {
     a_source_correspondence & corresp = c->source_corresp;

     bool is_generated = false;

     a_decl_position_supplement_ptr sup = corresp.decl_pos_info;

     if (sup == NULL)
        {
          is_generated = true;
        }
       else
        {
          is_generated = source_range_null(sup->variant.enum_value_range);
        }

#if DEBUG_SOURCE_POSITION
     printf ("In set_file_info_for_enumerator(e = %p = %s c = %p) is_generated = %s \n",e,e->class_name().c_str(),c,is_generated ? "true" : "false");
#endif

     Sg_File_Info* fi_start    = is_generated ? make_compiler_file_info() : convert_source_position(sup->variant.enum_value_range.start);
     Sg_File_Info* fi_end      = is_generated ? make_compiler_file_info() : convert_source_position(sup->variant.enum_value_range.end);
     Sg_File_Info* fi_operator = is_generated ? make_compiler_file_info() : convert_source_position(sup->variant.enum_value_range.start);

     selectivelyReplaceSourcePositionInformation(e,fi_start,fi_end);
     selectivelyReplaceSourcePositionInformation(e,fi_operator);

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     ROSE_ASSERT(e->get_file_info()->isCompilerGenerated() == e->get_startOfConstruct()->isCompilerGenerated());

     return e;
   }

template <typename Stmt>
Stmt* set_file_info_for_stmt(Stmt* e, a_statement_ptr orig_e)
   {
#if DEBUG_SOURCE_POSITION
     printf ("In set_file_info_for_stmt(e = %p = %s orig_e = %p) \n",e,e->class_name().c_str(),orig_e);
#endif

     bool compiler_generated = (orig_e->position.seq == 0);

#if 0
     if (compiler_generated == true)
        {
          printf ("In set_file_info_for_stmt(): Calling make_compiler_file_info_to_unparse() e = %p = %s \n",e,e->class_name().c_str());
        }
#endif

     Sg_File_Info* fi_start = compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(orig_e->position);

  // DQ (3/3/2013): UPC statements don't have a valid ending source position so this causes an error in convert_source_position().
  // We have to handle this case where the orig_e->end_position.seq == 0 explicitly. What we do is reuse the starting source position.
     Sg_File_Info* fi_end = compiler_generated ? make_compiler_file_info_to_unparse() : ((orig_e->end_position.seq == 0) ? convert_source_position(orig_e->position) : convert_source_position(orig_e->end_position));

#if DEBUG_SOURCE_POSITION
  // DQ (3/3/2013): Output a warning when this happens.
     if (compiler_generated == false && orig_e->end_position.seq == 0)
        {
          printf ("In set_file_info_for_stmt(e = %p = %s orig_e = %p): compiler_generated == false, but the ending position for statement not available, so reusing the starting position for the ending position \n",e,e->class_name().c_str(),orig_e);
        }
#endif

     selectivelyReplaceSourcePositionInformation(e,fi_start,fi_end);

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     return e;
   }


template <typename Stmt>
Stmt* set_file_info_for_case_option_stmt(Stmt* e, a_switch_case_entry_ptr orig_e)
   {
#if DEBUG_SOURCE_POSITION
     printf ("In set_file_info_for_case_option_stmt(e = %p = %s orig_e = %p) \n",e,e->class_name().c_str(),orig_e);
#endif

     bool compiler_generated = (orig_e->position.seq == 0);

     Sg_File_Info* fi_start = nullptr;
     if (compiler_generated) {
       fi_start = make_compiler_file_info_to_unparse();
     } else {
       fi_start = convert_source_position(orig_e->position);
     }

     Sg_File_Info * fi_end = nullptr;
     if (compiler_generated) {
       fi_end = make_compiler_file_info_to_unparse();
     } else if (orig_e->colon_position.seq == 0) {
       fi_end = convert_source_position(orig_e->position);
     } else {
       fi_end = convert_source_position(orig_e->colon_position);
     }

     selectivelyReplaceSourcePositionInformation(e,fi_start,fi_end);

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     return e;
   }

template <typename Stmt>
Stmt* set_file_info_for_template(Stmt* e, a_template_ptr orig_e)
   {
#if DEBUG_SOURCE_POSITION
     printf ("In set_file_info_for_template(e = %p = %s orig_e = %p) \n",e,e->class_name().c_str(),orig_e);
#endif

     bool compiler_generated_start = (orig_e->source_corresp.decl_position.seq == 0);
     ROSE_ASSERT(compiler_generated_start == false);

     Sg_File_Info* fi_start = nullptr;
     if (compiler_generated_start) {
       fi_start = make_compiler_file_info_to_unparse();
     } else {
       fi_start = convert_source_position(orig_e->source_corresp.decl_position);
     }

     auto orig_decl_pos_info = orig_e->source_corresp.decl_pos_info;
     bool compiler_generated_identifier_end = (orig_decl_pos_info->identifier_range.end.seq == 0);
     bool compiler_generated_specifiers_end = (orig_decl_pos_info->specifiers_range.end.seq == 0);

     Sg_File_Info* fi_end = nullptr;
     if (compiler_generated_identifier_end) {
       fi_end = make_compiler_file_info_to_unparse();
     } else if (!compiler_generated_specifiers_end) {
       fi_end = convert_source_position(orig_decl_pos_info->specifiers_range.end);
     } else {
       fi_end = convert_source_position(orig_decl_pos_info->identifier_range.end);
     }

     selectivelyReplaceSourcePositionInformation(e,fi_start,fi_end);

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     return e;
   }


template <typename Stmt>
Stmt* set_starting_file_info_for_template(Stmt* e, a_template_ptr orig_e)
   {
#if DEBUG_SOURCE_POSITION
     printf ("In set_starting_file_info_for_template(e = %p = %s orig_e = %p) \n",e,e->class_name().c_str(),orig_e);
#endif

     bool compiler_generated_start = (orig_e->source_corresp.decl_position.seq == 0);
     ROSE_ASSERT(compiler_generated_start == false);

#if 0
     if (compiler_generated_start == true)
        {
          printf ("In set_starting_file_info_for_case_option_stmt(): Calling make_compiler_file_info_to_unparse() e = %p = %s \n",e,e->class_name().c_str());
        }
#endif

     Sg_File_Info* fi_start = compiler_generated_start ? make_compiler_file_info_to_unparse() : convert_source_position(orig_e->source_corresp.decl_position);

#if 0
     printf ("In set_starting_file_info_for_template(): e = %p \n",e);
     fi_start->display("fi_start: startOfConstruct: debug");
#endif

     if (e->get_startOfConstruct() != NULL)
        {
          delete e->get_startOfConstruct();
          e->set_startOfConstruct(NULL);
        }

     e->set_startOfConstruct(fi_start); 

     fi_start->set_parent(e);

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     return e;
   }

#define DEBUG__set_file_info_for_corresp DEBUG_SOURCE_POSITION

template <typename Node>
Node* set_file_info_for_corresp(Node* e, a_source_correspondence sc)
   {
#if DEBUG__set_file_info_for_corresp
     printf ("Enter set_file_info_for_corresp()\n");
     printf ("  e = %p = %s\n", e, e->class_name().c_str());
#endif

     a_source_position start, end;
     get_source_range_for_declaration(sc, start, end);

#if DEBUG__set_file_info_for_corresp
     printf ("  start.seq = %d\n", start.seq);
#endif

     bool compiler_generated = (start.seq == 0);

  // DQ (2/14/2015): Lambda functions can have source position hidden elsewhere in 
  // the a_source_correspondence data structure.
     if (compiler_generated == true)
        {
          if (sc.decl_position.seq != 0)
             {
               start = sc.decl_position;
               end   = sc.decl_position;
               a_decl_position_supplement_ptr sup = sc.decl_pos_info;
#if DEBUG__set_file_info_for_corresp
               printf ("In set_file_info_for_corresp(): compiler_generated == true: sc.decl_position.seq != 0: a_decl_position_supplement_ptr sup = %p \n",sup);
#endif
               if (sup != NULL)
                  {
                    end = std::max(end, sup->variant.declarator_range.end);
                  }

               compiler_generated = false;
             }
        }

     Sg_File_Info* fi_start = compiler_generated ? make_compiler_file_info_to_unparse() : convert_source_position(start);

  // DQ (11/14/2016): EDG 4.11 and 4.12 have a bug in the values of the source sequence numbers that
  // are used to lookup the source position information.  So we need to filter the invalid values.
     Sg_File_Info* fi_end   = NULL;
     if (compiler_generated == true) 
        {
          fi_end = make_compiler_file_info_to_unparse();
        }
       else
        {
          if (is_valid_source_sequence_value(end) == true)
             {
               fi_end = convert_source_position(end);
             }
            else
             {
#if DEBUG__set_file_info_for_corresp
               printf ("In set_file_info_for_corresp(): Working around bug in EDG 4.12 \n");
#endif
               fi_end = new Sg_File_Info(*fi_start);
             }
       }

     selectivelyReplaceSourcePositionInformation(e,fi_start,fi_end);

     SgExpression* expression = isSgExpression(e);
     if (expression != NULL)
        {
          ROSE_ASSERT(e->get_startOfConstruct() != NULL);
          Sg_File_Info* fi_operator = new Sg_File_Info(*(e->get_startOfConstruct()));

          selectivelyReplaceSourcePositionInformation(expression,fi_operator);
        }

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

#if DEBUG__set_file_info_for_corresp
     printf ("Leave set_file_info_for_corresp()\n");
#endif
     return e;
   }

#define DEBUG_SET_FILE_INFO_FOR_SECONDARY_DECLARATION 0

// DQ (2/20/2012): Added support for secondary declarations (see test2004_64.C).
template <typename Node>
Node* set_file_info_for_secondary_declaration(Node* e, a_src_seq_secondary_decl_ptr secondary)
   {
  // DQ (11/3/2012): Added test.
     ROSE_ASSERT(e != NULL);

#if DEBUG_SOURCE_POSITION
     printf ("In set_file_info_for_secondary_declaration(e = %p = %s secondary = %p) \n",e,e->class_name().c_str(),secondary);
#endif

  // DQ (8/10/2012): We need to remember a few things about the existing Sg_File_Info objects if present.
     bool markedAsCompilerGenerated       = false;
     bool markedForOutputInCodeGeneration = false;

  // DQ (7/29/2012): Support resetting the source position information for secondary declarations.
     if (e->get_startOfConstruct() != NULL || e->get_endOfConstruct() != NULL)
        {
#if DEBUG_SET_FILE_INFO_FOR_SECONDARY_DECLARATION
          printf ("Warning: In set_file_info_for_secondary_declaration(): (part 2): found an existing file info object already attached to this IR node = %p = %s \n",e,e->class_name().c_str());
#endif
          if (e->get_startOfConstruct() != NULL)
             {
               markedAsCompilerGenerated       = e->get_startOfConstruct()->isCompilerGenerated();
               markedForOutputInCodeGeneration = e->get_startOfConstruct()->isOutputInCodeGeneration();

               delete e->get_startOfConstruct();
               e->set_startOfConstruct(NULL);
             }

          if (e->get_endOfConstruct() != NULL)
             {
               delete e->get_endOfConstruct();
               e->set_endOfConstruct(NULL);
             }
        }

     Sg_File_Info* fi_start = NULL;

     if (secondary->decl_pos_info == NULL) {
       fi_start = convert_source_position(secondary->decl_position);
     } else if (secondary->decl_pos_info->identifier_range.start.seq == 0) {
       fi_start = make_compiler_file_info_to_unparse();
     } else if (secondary->decl_pos_info->specifiers_range.start.seq != 0) {
       fi_start = convert_source_position(secondary->decl_pos_info->specifiers_range.start);
     } else {
       fi_start = convert_source_position(secondary->decl_pos_info->identifier_range.start);
     }

     e->set_startOfConstruct(fi_start);
     fi_start->set_parent(e);

#if DEBUG_SET_FILE_INFO_FOR_SECONDARY_DECLARATION
     printf ("   --- fi_start->get_line() = %d  column = %d \n",fi_start->get_line(),fi_start->get_col());
#endif

     Sg_File_Info* fi_end   = NULL;

     if (secondary->decl_pos_info == NULL) {
       fi_end = convert_source_position(secondary->decl_position);
     } else if (secondary->decl_pos_info->identifier_range.end.seq == 0) {
       fi_end = make_compiler_file_info_to_unparse();
     } else if (secondary->decl_pos_info->variant.declarator_range.end.seq != 0) {
       fi_end = convert_source_position(secondary->decl_pos_info->variant.declarator_range.end);
     } else if (secondary->decl_pos_info->specifiers_range.end.seq != 0) {
       fi_end = convert_source_position(secondary->decl_pos_info->specifiers_range.end);
     } else {
       fi_end = convert_source_position(secondary->decl_pos_info->identifier_range.end);
     }

     e->set_endOfConstruct(fi_end);
     fi_end->set_parent(e);

#if DEBUG_SET_FILE_INFO_FOR_SECONDARY_DECLARATION
     printf ("   --- fi_end->get_line() = %d  column = %d \n",fi_end->get_line(),fi_end->get_col());
#endif

  // DQ (1/13/2020): This is a problem for Cxx11_tests/test2020_47.C (friend function being makred incorectly as compiler generated).
  // But if we comment it out it is a larger issue for more test codes.
     if (markedAsCompilerGenerated == true)
        {
#if DEBUG_SET_FILE_INFO_FOR_SECONDARY_DECLARATION
          printf ("Transfering isCompilerGenerated-ness from old source position info to new secondary source position info \n");
#endif
          fi_start->setCompilerGenerated();
          fi_end->setCompilerGenerated();
        }

  // DQ (5/19/2013): IR nodes build using default rules and reset with valid source position from a 
  // different file should not be using setOutputInCodeGeneration().  See test2004_48.C, where 
  // namespace declarations from header files are being marked for output in the source file.
     if (markedAsCompilerGenerated == true && markedForOutputInCodeGeneration == true)
        {
#if DEBUG_SET_FILE_INFO_FOR_SECONDARY_DECLARATION
          printf ("Transfering isOutputInCodeGeneration-ness from old source position info to new secondary source position info \n");
#endif
          fi_start->setOutputInCodeGeneration();
          fi_end->setOutputInCodeGeneration();
        }

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     return e;
   }


template <typename Node>
Node* set_file_info_for_corresp_and_end(Node* e, a_source_correspondence sc, const SourceSequenceList & end_entry)
   {
  // DQ (11/3/2012): Added test.
     ROSE_ASSERT(e != NULL);

#if DEBUG_SOURCE_POSITION
     printf ("In set_file_info_for_corresp_and_end(e = %p = %s) \n",e,e->class_name().c_str());
#endif

     ROSE_ASSERT (end_entry->entity.kind == iek_src_seq_end_of_construct);
     a_source_position start, end;
     get_source_range_for_declaration(sc, start, end);

     Sg_File_Info* fi_start = convert_source_position(start);
     Sg_File_Info* fi_end   = convert_source_position(((a_src_seq_end_of_construct_ptr)(end_entry->entity.ptr))->position);

     selectivelyReplaceSourcePositionInformation(e,fi_start,fi_end);

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     return e;
   }


// DQ (8/18/2012): Added function to set secondaries with the correct end position.
template <typename Node>
Node* set_file_info_for_secondary_and_end(Node* e, a_src_seq_secondary_decl_ptr secondary_decl, const SourceSequenceList & end_entry)
   {
  // DQ (11/3/2012): Added test.
     ROSE_ASSERT(e != NULL);

#if DEBUG_SOURCE_POSITION
     printf ("In set_file_info_for_secondary_and_end(e = %p = %s) \n",e,e->class_name().c_str());
#endif

     ROSE_ASSERT (end_entry->entity.kind == iek_src_seq_end_of_construct);
     a_source_position start, end;
     get_source_range_for_secondary_declaration(secondary_decl, start, end);

     Sg_File_Info* fi_start = convert_source_position(start);
     Sg_File_Info* fi_end   = convert_source_position(((a_src_seq_end_of_construct_ptr)(end_entry->entity.ptr))->position);

     selectivelyReplaceSourcePositionInformation(e,fi_start,fi_end);

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     return e;
   }


template <typename Expr>
Expr* set_starting_file_info_for_lambda(Expr* e, a_lambda_ptr orig_e)
   {
  // DQ (7/19/2020): Added to support EDG 6.0 source position for lambda expressions.

#if DEBUG_SOURCE_POSITION
     printf ("In set_starting_file_info_for_lambda(e = %p = %s orig_e = %p) \n",e,e->class_name().c_str(),orig_e);
#endif

     bool compiler_generated_start = (orig_e->start_position.seq == 0);
     ROSE_ASSERT(compiler_generated_start == false);

     Sg_File_Info* fi_start = compiler_generated_start ? make_compiler_file_info_to_unparse() : convert_source_position(orig_e->start_position);


     if (e->get_startOfConstruct() != NULL)
        {
          delete e->get_startOfConstruct();
          e->set_startOfConstruct(NULL);
        }

     e->set_startOfConstruct(fi_start); 

     fi_start->set_parent(e);

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     return e;
   }


template <typename Node>
Node*
set_generated_file_info(Node* e)
   {
  // FIXME: This is a template because not everything that has a source position is a SgLocatedNode

  // DQ (11/3/2012): Added test.
     ROSE_ASSERT(e != NULL);

#if DEBUG_SOURCE_POSITION
     printf ("In set_generated_file_info(e = %p = %s) \n",e,e->class_name().c_str());
#endif

     Sg_File_Info* fi_start = make_compiler_file_info();
     Sg_File_Info* fi_end   = make_compiler_file_info();

     selectivelyReplaceSourcePositionInformation(e,fi_start,fi_end);

     if (isSgExpression(e) != NULL)
        {
          Sg_File_Info* fi_op = make_compiler_file_info();

          selectivelyReplaceSourcePositionInformation(e,fi_op);
        }

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     return e;
   }


template <typename Node>
Node*
set_generated_file_info_to_unparse(Node* e)
   {
     ROSE_ASSERT(e != NULL);

#if DEBUG_SOURCE_POSITION
     printf ("In set_generated_file_info_to_unparse(e = %p = %s) \n",e,e->class_name().c_str());
#endif

     set_generated_file_info(e);

     e->get_startOfConstruct()->setOutputInCodeGeneration();
     e->get_endOfConstruct()->setOutputInCodeGeneration();

     if (isSgExpression(e) != NULL)
        {
          isSgExpression(e)->get_operatorPosition()->setOutputInCodeGeneration();
        }

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     return e;
   }

template <typename Node>
Node* set_file_info_for_where_corresp_is_unavailable(Node* e, a_source_position & start)
   {
#if DEBUG_SOURCE_POSITION
     printf ("In set_file_info_for_where_corresp_is_unavailable(e = %p = %s) \n",e,e->class_name().c_str());
#endif

     Sg_File_Info* fi_start = convert_source_position(start);

  // Make the end position equal to the start, since no end position is available.
     Sg_File_Info* fi_end = convert_source_position(start);

     selectivelyReplaceSourcePositionInformation(e,fi_start,fi_end);

     ROSE_ASSERT(e->get_startOfConstruct()->isTransformation() == false);

     return e;
   }

