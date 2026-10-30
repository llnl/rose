
#include "edgRose.h"

using namespace std;
using namespace Rose;
using namespace SageInterface;
// using namespace SageBuilder;
// using namespace OmpSupport;

// File for output for generated graph.
std::ofstream EDG_ROSE_Graph::file;

// DQ (9/7/2013): We need a list to make sure that we don't add nodes twice (once when traversing the source sequence lists and a second time when traversing the AST).
// We need to show the AST plus the edges that are specific to the source sequence lists (and which node's source sequence list they are associated with).
std::set<void*> graphNodeSet;

#define DEBUG_EDG_DOT_GRAPH_SUPPPORT 0

#define DEBUG_HEADER_GRAPH_SUPPPORT 0


#if 0
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
static
void
printGraphNodeSet()
   {
     int counter = 0;
     std::set<void*>::iterator i = graphNodeSet.begin();

     printf ("In printGraphNodeSet(): size() = %" PRIuPTR " \n",graphNodeSet.size());
     while (i != graphNodeSet.end())
        {
          printf ("   --- graphNodeSet[%d] = %p \n",counter,*i);
          i++;
          counter++;
        }
   }
#endif
#endif

void
EDG_ROSE_Graph::insertGraphNode (void* node)
   {
#if 0
     printf ("In insertGraphNode(): node = %p \n",node);
#endif

     ROSE_ASSERT(node != NULL);

     graphNodeSet.insert(node);
   }

void
EDG_ROSE_Graph::graph ( a_scope_ptr scope, const string & scope_name )
   {
     ROSE_ASSERT(scope != NULL);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
  // DQ (7/15/2017): Added debugging data.
     printf ("In graph(a_scope_ptr): scope = %p scope->kind = %s scope_name = %s \n",scope,summarize_scope_in_string(scope),scope_name.c_str());
#endif

  // DQ (1/26/2019): Make sure this has been given a name.
     ROSE_ASSERT(scope_name.empty() == false);

  // SourceSequenceList sse(scope->source_sequence_list);

     if (graphNodeSet.find(scope) == graphNodeSet.end())
        {
       // DQ (10/11/2015): We might want to test if the name was filled in and generate a name when it was not (e.g. from class for the parent scope).

          file << "\"" << StringUtility::numberToString(scope) << "\"[" << "label=\"" << scope_name << "\\n" << StringUtility::numberToString(scope) << "\" color=\"blue\",fillcolor=cyan4,fontname=\"7x13bold\",fontcolor=black,style=filled];" << endl;

          insertGraphNode(scope);
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_scope_ptr): top (scope = %p): graphNodeSet.size() = %" PRIuPTR " \n",scope,graphNodeSet.size());
#endif
        }
       else
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("WARNING: this scope has been previously traversed to build the graph (return) \n");
#endif
          return;
#if 0
          printf ("ERROR: We should not be able to have traversed a scope more than once: scope = %p \n",scope);
          ROSE_ASSERT(false);
#endif
        }

  // DQ (account for when this is a NULL pointer.
  // DQ (1/5/2014): Modified to call this refactored function.
  // graph(scope->source_sequence_list);
     if (scope->source_sequence_list != NULL)
        {
          graph(scope->source_sequence_list);
        }

  // DQ (1/19/2016): Adding support for output of constructor preinitialization list.
     if (scope->kind == sck_function)
        {
          a_constructor_init_ptr ctor_init = scope->variant.routine.constructor_inits;
          int counter = 0;
          if (ctor_init != NULL)
             {
#if 0
               printf ("Build edge from scope %p to %p \n",scope,ctor_init);
#endif
#if 1
               file << "\"" << StringUtility::numberToString(scope) << "\" -> \"" << StringUtility::numberToString(ctor_init)
                    << "\"[label=\"" << "ctor preinit member : " + StringUtility::numberToString(counter) << ":" 
                    << StringUtility::numberToString(ctor_init) << "\" color=\"blue\" labelfontcolor=\"blue4\" weight=1 len=2];" << endl;
#endif
             }

          while (ctor_init != NULL)
             {
#if 0
               printf ("Graph the constructor preinitialization list entry: ctor_init = %p \n",ctor_init);
#endif
               graph(ctor_init);

               counter++;
               if (ctor_init->next != NULL)
                  {
#if 0
                    printf ("Build edge from ctor_init = %p to ctor_init->next = %p \n",ctor_init,ctor_init->next);
#endif
#if 1
                    file << "\"" << StringUtility::numberToString(ctor_init) << "\" -> \"" << StringUtility::numberToString(ctor_init->next)
                         << "\"[label=\"" << "ctor preinit member : " + StringUtility::numberToString(counter) << ":" 
                         << StringUtility::numberToString(ctor_init) << "\" color=\"blue\" labelfontcolor=\"blue4\" weight=1 len=2];" << endl;
#endif
                  }

               ctor_init = ctor_init->next;
             }
        }
       else
        {
          if (scope->kind == sck_class_struct_union)
             {
               a_type_ptr assoc_type = scope->variant.assoc_type;
               ROSE_ASSERT(assoc_type != NULL);

               graph(assoc_type);

               file << "\"" << StringUtility::numberToString(scope) << "\" -> \"" << StringUtility::numberToString(assoc_type) << "\"[label=\"" << "assoc_type" << "\"];" << endl;

             }
            else
             {
#if 0
            // DQ (6/16/2016): Added debugging data.
               printf ("In graph(a_scope_ptr): Unhandled scope: scope = %p scope->kind = %s scope_name = %s \n",scope,summarize_scope_in_string(scope),scope_name.c_str());
#endif
             }
        }

  // DQ (1/31/2018): Adding more support for scopes.
     if (scope->assoc_block != NULL)
        {
          graph(scope->assoc_block);

          file << "\"" << StringUtility::numberToString(scope) << "\" -> \"" << StringUtility::numberToString(scope->assoc_block) << "\"[label=\"" << "assoc_block" << "\"];" << endl;
        }

  // DQ (2/10/2018): Adding more support for vla types.
     if (scope->vla_dimensions != NULL)
        {
          a_vla_dimension_ptr vla_dimension = scope->vla_dimensions;

          while(vla_dimension != NULL)
             {
               graph(vla_dimension);

               file << "\"" << StringUtility::numberToString(scope) << "\" -> \"" << StringUtility::numberToString(vla_dimension) << "\"[label=\"" << "vla_dimension" << "\"];" << endl;

               vla_dimension = vla_dimension->next;
             }
        }

     if (scope->parent != NULL) {
       graph(scope->parent,"parent scope");

       file << "\"" << StringUtility::numberToString(scope) << "\" -> \"" << StringUtility::numberToString(scope->parent) << "\"[label=\"parent_scope\" color=\"lightgreen\"];" << endl;
     }
   }


void
EDG_ROSE_Graph::graph ( a_constructor_init_ptr ctor_init )
   {
     ROSE_ASSERT(ctor_init != NULL);

#if 1
     file << "\"" << StringUtility::numberToString(ctor_init) << "\"[" << "label=\"" << "constructor-preinitialization-entry" << "\\n" << StringUtility::numberToString(ctor_init) << "\" color=\"blue\",fillcolor=cyan4,fontname=\"7x13bold\",fontcolor=black,style=filled];" << endl;
#endif

     if (ctor_init->kind == cik_field)
        {
       // Add edge to associated field variable.
#if 1
          file << "\"" << StringUtility::numberToString(ctor_init) << "\" -> \"" << StringUtility::numberToString(ctor_init->variant.field)
                       << "\"[label=\"" << "field "
                       << "\" color=\"blue\" labelfontcolor=\"blue4\" weight=1 len=2];" << endl;
#endif
        }


   }


void
EDG_ROSE_Graph::graph ( a_using_decl_ptr udp )
   {
     ROSE_ASSERT(udp != NULL);

     string inheriting_constructor = "";
     if (udp->is_inheriting_ctor == true)
        {
          printf ("This is the case of an inheriting constructor \n");
          inheriting_constructor = "   is inheriting constructor    \n";
        }

     file << "\"" << StringUtility::numberToString(udp) << "\"[" << "label=\"using-decl\\n" << inheriting_constructor << StringUtility::numberToString(udp) << "\" color=\"blue\"];" << endl;

#if 0
     printf ("In graph ( a_using_decl_ptr udp ): TOP: udp->entity.kind != iek_type: udp->entity.kind = %s \n",summarize_entry_kind_in_string_kind(udp->entity).c_str());
#endif

  // DQ (1/10/2019): Even when this is an inheriting constructor, the entity.kind is always the class.
     if (udp->entity.kind == iek_type)
        {
          a_type_ptr edg_type = (a_type_ptr)(udp->entity.ptr);
          graph(edg_type);
          file << "\"" << StringUtility::numberToString(udp) << "\" -> \"" << StringUtility::numberToString(edg_type)
                       << "\"[label=\"" << "type" << "\"];" << endl;
        }
       else
        {
       // DQ (1/30/2019): Adding support for routines.
          if (udp->entity.kind == iek_routine)
             {
               a_routine_ptr edg_routine = (a_routine_ptr)(udp->entity.ptr);
               graph(edg_routine);
               file << "\"" << StringUtility::numberToString(udp) << "\" -> \"" << StringUtility::numberToString(edg_routine)
                            << "\"[label=\"" << "routine" << "\"];" << endl;
             }
            else
             {
               if (udp->entity.kind == iek_namespace)
                  {
                    a_namespace_ptr edg_namespace = (a_namespace_ptr)(udp->entity.ptr);
                    graph(edg_namespace);
                    file << "\"" << StringUtility::numberToString(udp) << "\" -> \"" << StringUtility::numberToString(edg_namespace)
                                 << "\"[label=\"" << "namespace" << "\"];" << endl;
                  }
                 else
                  {
                 // DQ (1/10/2019): What kind is this!
                 // printf ("What is this sse_kind = %s \n",summarize_entry_kind_in_string_kind(kind));

                 // printf ("In graph ( a_using_decl_ptr udp ): udp->entity.kind != iek_type: udp->entity.kind = %s \n",summarize_entry_kind_in_string_kind(udp->entity.kind));
                    printf ("In graph ( a_using_decl_ptr udp ): udp->entity.kind != iek_type: udp->entity.kind = %s \n",summarize_entry_kind_in_string_kind(udp->entity).c_str());

                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
                  }
             }
        }

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif
   }


void
EDG_ROSE_Graph::graph ( a_source_sequence_entry_ptr src_seq_entry )
   {
     ROSE_ASSERT(src_seq_entry != NULL);

     if (graphNodeSet.find(src_seq_entry) == graphNodeSet.end())
        {
       // printf ("In graph(a_source_sequence_entry_ptr src_seq_entry = %p ): not implemented! \n",src_seq_entry);
          file << "\"" << StringUtility::numberToString(src_seq_entry) << "\"[" << "label=\"" << "source_sequence_entry list" << "\\n" << StringUtility::numberToString(src_seq_entry) << "\" color=\"blue\",fillcolor=cyan4,fontname=\"7x13bold\",fontcolor=black,style=filled];" << endl;

          insertGraphNode(src_seq_entry);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_source_sequence_entry_ptr): top (src_seq_entry = %p): graphNodeSet.size() = %" PRIuPTR " \n",src_seq_entry,graphNodeSet.size());
#endif
        }
       else
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("WARNING: In graph(a_source_sequence_entry_ptr): this source sequence list has been previously traversed to build the graph (return) \n");
#endif
          return;
#if 0
          printf ("ERROR: We should not be able to have traversed a src_seq_entry list more than once: src_seq_entry (head of list) = %p \n",src_seq_entry);
          ROSE_ASSERT(false);
#endif
        }

  // SourceSequenceList sse(scope->source_sequence_list);
     SourceSequenceList sse(src_seq_entry);

  // EDG uses a char* for the pointer type to all IR nodes (entities).
  // char* previousSourceSequenceEntity = (char*)scope;
     char* previousSourceSequenceEntity = (char*)src_seq_entry;

     int counter = 0;
     while (sse)
        {
#if 0
          printf ("In graph(a_source_sequence_entry_ptr): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if 0
          if (outputInGraph(sse) == false)
             {
            // Ignore the rest of the loop for IR nodes that are not in the input source file.
               ++sse;
               continue;
             }
#else
#if 0
          printf ("In graph(a_source_sequence_entry_ptr): Skipping the code to skip node not output in graph: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#endif
#if 1
       // Output scope member edges
       // file << "\"" << StringUtility::numberToString(scope) << "\" -> \"" << StringUtility::numberToString(sse->entity.ptr) 
       //      << "\"[label=\"" << "scope member : " + StringUtility::numberToString(counter)<< "\" color=\"blue\" decorate labelfontcolor=\"blue4\" weight=1 len=2];" << endl;
       // DQ (3/19/2014): removed "decorate" keyword since it is an error for latest version of zgrviewer (or maybe dot).
       // file << "\"" << StringUtility::numberToString(src_seq_entry) << "\" -> \"" << StringUtility::numberToString(sse->entity.ptr) 
       //      << "\"[label=\"" << "list member : " + StringUtility::numberToString(counter)<< "\" color=\"blue\" decorate labelfontcolor=\"blue4\" weight=1 len=2];" << endl;
          file << "\"" << StringUtility::numberToString(src_seq_entry) << "\" -> \"" << StringUtility::numberToString(sse->entity.ptr) 
            // << "\"[label=\"" << "list member : " + StringUtility::numberToString(counter) << "\" color=\"blue\" labelfontcolor=\"blue4\" weight=1 len=2];" << endl;
               << "\"[label=\"" << "list member : " + StringUtility::numberToString(counter) << ":" << StringUtility::numberToString(src_seq_entry) << "\" color=\"blue\" labelfontcolor=\"blue4\" weight=1 len=2];" << endl;
#endif
#if 1
       // Output the edges of the source sequence list (restricted to those that pass: outputInGraph(sse) == true).
       // DQ (3/19/2014): removed "decorate" keyword since it is an error for latest version of zgrviewer (or maybe dot).
       // file << "\"" << StringUtility::numberToString(previousSourceSequenceEntity) << "\" -> \"" << StringUtility::numberToString(sse->entity.ptr) 
       //      << "\"[label=\"" << "source seq: " + StringUtility::numberToString(counter)<< "\" color=\"red\" decorate labelfontcolor=\"red\" weight=1];" << endl;

#if 0
       // DQ (11/19/2017): This version will output more information with edge lables (useful to build a more detailed dot graph).
          file << "\"" << StringUtility::numberToString(previousSourceSequenceEntity) << "\" -> \"" << StringUtility::numberToString(sse->entity.ptr) 
               << "\"[label=\"" << "source seq: " + StringUtility::numberToString(counter)<< "\" color=\"red\" labelfontcolor=\"red\" weight=1];" << endl;
#else
       // DQ (11/19/2017): This version will output less information with empty edge lables (useful to build a less crowded dot graph).
          file << "\"" << StringUtility::numberToString(previousSourceSequenceEntity) << "\" -> \"" << StringUtility::numberToString(sse->entity.ptr) 
               << "\"[label=\"\" color=\"red\" labelfontcolor=\"red\" weight=1];" << endl;
#endif
#endif

       // This will be the previous enity that for which (outputInGraph(sse) == true).
          previousSourceSequenceEntity = sse->entity.ptr;

          an_il_entry_kind kind = (an_il_entry_kind) (sse->entity.kind);
       // printf ("What is this sse_kind = %s \n",summarize_entry_kind_in_string_kind(kind));

          switch (kind)
             {
               case iek_routine:
                  {
                 // This code could be refactored into a function "graph(a_variable_ptr variable)".
                    a_routine_ptr routine = (a_routine_ptr)(sse->entity.ptr);

                    graph(routine);
                 // file << "\"" << StringUtility::numberToString(scope) << "\" -> \"" << StringUtility::numberToString(routine) << "\"[label=\"" << "function : " + StringUtility::numberToString(counter)<< "\"];" << endl;

                    a_scope_ptr            function_scope = il_header.function_def_table[routine->function_def_number].scope;
#if 0
                    printf ("In graph(a_source_sequence_entry_ptr): case iek_routine: Calling graph(a_scope_ptr) on function_scope = %p \n",function_scope);
#endif

                 // DQ (2/10/2018): This is one of two edges that are generated, I supressed the second edge.
                    string function_name = routine->source_corresp.name;
                    function_name += " : function scope";
                    if (function_scope != NULL)
                       {
                         graph(function_scope, function_name);
                         file << "\"" << StringUtility::numberToString(routine) << "\" -> \"" << StringUtility::numberToString(function_scope) << "\"[label=\"" << "function scope" << "\"];" << endl;
                       }

                    break;
                  }

            // DQ (7/31/2014): Added more support for EDG AST graphing.
               case iek_constant:
                  {
                 // This code could be refactored into a function "graph(a_constant_ptr constant)".
                    a_constant_ptr constant = (a_constant_ptr)(sse->entity.ptr);
                    graph(constant);
                    break;
                  }


               case iek_variable:
                  {
                 // This code could be refactored into a function "graph(a_variable_ptr variable)".

                    a_variable_ptr variable = (a_variable_ptr)(sse->entity.ptr);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                    printf ("In graph(a_source_sequence_entry_ptr): case iek_variable: variable = %p \n",variable);
#endif
                    graph(variable);
                    break;
                  }

               case iek_statement:
                  {
                    a_statement_ptr statement = (a_statement_ptr)(sse->entity.ptr);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                    printf ("In graph(a_source_sequence_entry_ptr): case iek_statement: statement = %p \n",statement);
#endif
                    graph(statement);
                    break;
                  }

               case iek_type:
                  {
                    a_type_ptr type = (a_type_ptr)(sse->entity.ptr);
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                    printf ("In graph(a_source_sequence_entry_ptr): case iek_type: type = %p \n",type);
#endif
                    graph(type);
                    break;
                  }

               case iek_src_seq_secondary_decl:
                  {
                 // This code could be refactored into a function "graph(a_variable_ptr variable)".

                    a_src_seq_secondary_decl_ptr src_seq_secondary_decl = (a_src_seq_secondary_decl_ptr)(sse->entity.ptr);

                 // Output nodes
                    if (graphNodeSet.find(src_seq_secondary_decl) == graphNodeSet.end())
                       {
                         bool includePointerValue = false;
                         file << "\"" << StringUtility::numberToString(src_seq_secondary_decl) << "\"[" << "label=\"" << summarize_entry_kind_in_string_kind(sse->entity,includePointerValue) << "\\n" 
                           // << StringUtility::numberToString(src_seq_secondary_decl) + " : " + StringUtility::numberToString(counter) << "\" color=\"blue\" ];" << endl;
                              << StringUtility::numberToString(src_seq_secondary_decl) << "\" color=\"blue\" ];" << endl;

                         insertGraphNode(sse->entity.ptr);
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                         printf ("In graph(a_source_sequence_entry_ptr): Relevant nodes not yet handled: (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",sse->entity.ptr,graphNodeSet.size());
#endif
                       }
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                    printf ("In graph(a_source_sequence_entry_ptr): case iek_src_seq_secondary_decl \n");
#endif

                 // DQ (9/25/2015): We can't assume this is non-NULL (see test2015_83.C).
                 // DQ (9/21/2015): Added info about the declared type in the secondary declaration.
                 // ROSE_ASSERT(src_seq_secondary_decl->declared_type != NULL);
                    if (src_seq_secondary_decl->declared_type != NULL)
                       {
                         graph(src_seq_secondary_decl->declared_type);
                         file << "\"" << StringUtility::numberToString(src_seq_secondary_decl) << "\" -> \"" << StringUtility::numberToString(src_seq_secondary_decl->declared_type) << "\"[label=\"" << "declared_type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
                       }

                    an_il_entry_kind inner_kind = (an_il_entry_kind)(((a_src_seq_secondary_decl_ptr)(sse->entity.ptr))->entity.kind);
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                    printf ("case iek_src_seq_secondary_decl: il_entry_kind_names[inner_kind] = %s \n",il_entry_kind_names[inner_kind]);
#endif
                    char* inner_ptr = ((a_src_seq_secondary_decl_ptr)(sse->entity.ptr))->entity.ptr;
                    ROSE_ASSERT(inner_ptr != NULL);

                    switch (inner_kind)
                       {
                         case iek_variable:
                            {
                              a_variable_ptr variable = (a_variable_ptr)inner_ptr;
                              graph(variable);
                              file << "\"" << StringUtility::numberToString(src_seq_secondary_decl) << "\" -> \"" << StringUtility::numberToString(variable) << "\"[label=\"" << "secondary variable decl" << "\"];" << endl;
#if 0
                              printf ("In graph(a_source_sequence_entry_ptr): case iek_src_seq_secondary_decl: case iek_variable: not implemented yet! \n");
                              ROSE_ASSERT(false);
#endif
                              break;
                            }

                         case iek_type:
                            {
                              a_type_ptr type = (a_type_ptr)inner_ptr;
                              graph(type);
                              file << "\"" << StringUtility::numberToString(src_seq_secondary_decl) << "\" -> \"" << StringUtility::numberToString(type) << "\"[label=\"" << "secondary type decl" << "\"];" << endl;
#if 0
                              printf ("In graph(a_source_sequence_entry_ptr): case iek_src_seq_secondary_decl: case iek_type: not implemented yet! \n");
                              ROSE_ASSERT(false);
#endif
                              break;
                            }

                         case iek_routine:
                            {
                           // This is the case of a C++ member function
                              a_routine_ptr routine = (a_routine_ptr)inner_ptr;
                              ROSE_ASSERT(routine != NULL);
#if 0
                              printf ("In parse_structlike(): calling summarize_routine() \n");
                              summarize_routine(routine);
#endif
                              graph(routine);
                              file << "\"" << StringUtility::numberToString(src_seq_secondary_decl) << "\" -> \"" << StringUtility::numberToString(routine) << "\"[label=\"" << "secondary function decl" << "\"];" << endl;

                           // DQ (7/6/2017): Note that the zeroth entry is not used.
                           // a_memory_region_number scope_region_number = 0L;
                           // a_scope_ptr            function_scope = il_header.region_scope_entry[scope_region_number];
                           // a_scope_ptr            function_scope = il_header.function_def_table[routine->function_def_number].scope;
                              a_scope_ptr            function_scope = NULL;
                              if (routine->function_def_number > 0)
                                 {
                                   function_scope = il_header.function_def_table[routine->function_def_number].scope;
                                 }

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                              printf ("In graph(a_source_sequence_entry_ptr): case iek_routine: Calling graph(a_scope_ptr) on function_scope = %p \n",function_scope);
#endif
                           // There might not be a primary declaration (e.g. scope, etc.).
                              if (function_scope != NULL)
                                 {
                                   string function_name = routine->source_corresp.name;
                                   function_name += " : function scope";

                                   graph(function_scope, function_name);

                                   file << "\"" << StringUtility::numberToString(sse->entity.ptr) << "\" -> \"" << StringUtility::numberToString(routine) << "\"[label=\"" << "routine" << "\"];" << endl;
                                 }
#if 0
                              printf ("In graph(a_source_sequence_entry_ptr): case iek_src_seq_secondary_decl: case iek_routine: not implemented yet! \n");
                              ROSE_ASSERT(false);
#endif
                              break;
                            }
                            
                         case iek_template:
                            {
                              a_template_ptr template_node = (a_template_ptr)inner_ptr;
                              graph (template_node);
                              file << "\"" << StringUtility::numberToString(src_seq_secondary_decl) << "\" -> \"" << StringUtility::numberToString(template_node) << "\"[label=\"" << "secondary template function decl" << "\"];" << endl;
#if 0
                              printf ("In graph(a_source_sequence_entry_ptr): case iek_src_seq_secondary_decl: case iek_template: not implemented yet! \n");
                              ROSE_ASSERT(false);
#endif
                              break;
                            }

                         case iek_namespace:
                            {
                              a_namespace_ptr namespace_node = (a_namespace_ptr)inner_ptr;
#if 0
                              printf ("In case iek_namespace: Calling graph(a_namespace_ptr): namespace_node = %p \n",namespace_node);
#endif
                              graph (namespace_node);
                              file << "\"" << StringUtility::numberToString(src_seq_secondary_decl) << "\" -> \"" << StringUtility::numberToString(namespace_node) << "\"[label=\"" << "secondary namespace decl" << "\"];" << endl;
#if 0
                              printf ("In graph(a_source_sequence_entry_ptr): case iek_src_seq_secondary_decl: case iek_namespace: not implemented yet! \n");
                              ROSE_ASSERT(false);
#endif
                              break;
                            }

                          default:
                            {
                              printf ("ERROR: In graph(a_source_sequence_entry_ptr): case iek_src_seq_secondary_decl: default reached \n");
                              printf ("   --- case iek_src_seq_secondary_decl: il_entry_kind_names[inner_kind] = %s \n",il_entry_kind_names[inner_kind]);
                              ROSE_ASSERT(false);
                            }
                       }

                 // DQ (8/20/2020): Adding support for attributes
                    an_attribute_ptr attributes = src_seq_secondary_decl->attributes;
                    if (attributes != NULL)
                       {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                         printf ("In graph(a_src_seq_secondary_decl_ptr): found attributes \n");
#endif
                         graph(attributes);
                         file << "\"" << StringUtility::numberToString(src_seq_secondary_decl) << "\" -> \"" << StringUtility::numberToString(attributes) << "\"[label=\"" << "attributes" << "\"];" << endl;
#if 0
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                       }
                      else
                       {
#if 0
                         printf ("In graph(a_src_seq_secondary_decl_ptr): NO attributes \n");
#endif
                       }
#if 0
                    printf ("Exiting in In graph(a_source_sequence_entry_ptr): case iek_src_seq_secondary_decl: \n");
                    ROSE_ASSERT(false);
#endif
                    break;
                  }

               case iek_template:
                  {
                    a_template_ptr template_node = (a_template_ptr)(sse->entity.ptr);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                    printf ("In graph(a_source_sequence_entry_ptr): case iek_template: template_node = %p \n",template_node);
#endif
                    graph(template_node);

#if 0
                 // DQ (10/20/2016): This is redundant in specification of a lable for the template node.
                 // file << "\"" << StringUtility::numberToString(src_seq_secondary_decl) << "\" -> \"" << StringUtility::numberToString(namespace_node) << "\"[label=\"" << "secondary template decl" << "\"];" << endl;
                    bool includePointerValue = false;
                    file << "\"" << StringUtility::numberToString(sse->entity.ptr) << "\"[" << "label=\"" << summarize_entry_kind_in_string_kind(sse->entity,includePointerValue) << "\\n" 
                         << StringUtility::numberToString(sse->entity.ptr) + " : " + StringUtility::numberToString(counter) << "\" color=\"blue\" ];" << endl;
#endif
#if 0
                    printf ("In graph(a_scope_ptr): case iek_template: (incomplete) \n");
                    ROSE_ASSERT(false);
#endif
                    break;
                  }

            // DQ (1/10/2019): Seperate out the case of the using declaration.
               case iek_using_decl:
                  {
                 // Output nodes
                    a_using_decl_ptr using_decl_node = (a_using_decl_ptr)(sse->entity.ptr);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                    printf ("In graph(a_source_sequence_entry_ptr): case iek_using_decl: using_decl_node = %p \n",using_decl_node);
#endif
                    graph(using_decl_node);
#if 0
                    if (graphNodeSet.find(sse->entity.ptr) == graphNodeSet.end())
                       {
                         bool includePointerValue = false;
                         file << "\"" << StringUtility::numberToString(sse->entity.ptr) << "\"[" << "label=\"" << summarize_entry_kind_in_string_kind(sse->entity,includePointerValue) << "\\n" 
                              << StringUtility::numberToString(sse->entity.ptr) + " : " + StringUtility::numberToString(counter) << "\" color=\"blue\" ];" << endl;

                         insertGraphNode(sse->entity.ptr);
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                         printf ("In graph(a_source_sequence_entry_ptr): Relevant nodes not yet handled: (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",sse->entity.ptr,graphNodeSet.size());
#endif
                       }
#endif

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

                    break;
                  }

               case iek_instantiation_directive:
                  {
                 // Output nodes
                    if (graphNodeSet.find(sse->entity.ptr) == graphNodeSet.end())
                       {
                         bool includePointerValue = false;
                         file << "\"" << StringUtility::numberToString(sse->entity.ptr) << "\"[" << "label=\"" << summarize_entry_kind_in_string_kind(sse->entity,includePointerValue) << "\\n" 
                              << StringUtility::numberToString(sse->entity.ptr) + " : " + StringUtility::numberToString(counter) << "\" color=\"blue\" ];" << endl;

                         insertGraphNode(sse->entity.ptr);
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                         printf ("In graph(a_source_sequence_entry_ptr): Relevant nodes not yet handled: (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",sse->entity.ptr,graphNodeSet.size());
#endif

                      // DQ (1/18/2019): Adding support for template instantiation dire4ctives.
                         an_instantiation_directive_ptr idp = (an_instantiation_directive_ptr)(sse->entity.ptr);
                      // if (idp->entity != NULL)
                            {
                              an_il_entry_kind kind = (an_il_entry_kind)idp->entity.kind;
                              switch (kind)
                                 {
                                   case iek_routine:
                                      {
                                        a_routine_ptr p = (a_routine_ptr)idp->entity.ptr;
                                        ROSE_ASSERT(p != NULL);

                                        printf ("Found case iek_routine \n");

                                        graph(p);
                                        file << "\"" << StringUtility::numberToString(idp) << "\" -> \"" << StringUtility::numberToString(p) << "\"[label=\"" << "function" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

                                        break;
                                      }

                                   case iek_variable:
                                      {
                                        a_variable_ptr p = (a_variable_ptr)idp->entity.ptr;
                                        ROSE_ASSERT(p != NULL);

                                        printf ("Found case iek_variable \n");

                                        graph(p);
                                        file << "\"" << StringUtility::numberToString(idp) << "\" -> \"" << StringUtility::numberToString(p) << "\"[label=\"" << "variable" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

                                        break;
                                      }

                                   case iek_type:
                                      {
                                        a_type_ptr p = (a_type_ptr)idp->entity.ptr;
                                        ROSE_ASSERT(p != NULL);

                                        printf ("Found case iek_type \n");

                                        graph(p);
                                        file << "\"" << StringUtility::numberToString(idp) << "\" -> \"" << StringUtility::numberToString(p) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

                                        break;
                                      }

                                   default:
                                      {
                                        printf ("Error: default reached in case iek_instantiation_directive \n");
                                        ROSE_ASSERT(false);
                                      }
                                 }
                            }

                         if (idp->attributes != NULL)
                            {
                           // DQ (8/20/2020): We can support this now.
                              graph(idp->attributes);
                              file << "\"" << StringUtility::numberToString(idp) << "\" -> \"" << StringUtility::numberToString(idp->attributes) << "\"[label=\"" << "attributes" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
                            }

#if 0
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                       }

                    break;
                  }


            // Relevant nodes not yet handled.
            // case iek_src_seq_secondary_decl:
               case iek_macro:
               case iek_pragma:
               case iek_asm_entry:
            // case iek_namespace:
            // case iek_template:
               case iek_field:
            // case iek_using_decl:
            // case iek_instantiation_directive:
                  {
                 // Output nodes
                    if (graphNodeSet.find(sse->entity.ptr) == graphNodeSet.end())
                       {
                         bool includePointerValue = false;
                         file << "\"" << StringUtility::numberToString(sse->entity.ptr) << "\"[" << "label=\"" << summarize_entry_kind_in_string_kind(sse->entity,includePointerValue) << "\\n" 
                              << StringUtility::numberToString(sse->entity.ptr) + " : " + StringUtility::numberToString(counter) << "\" color=\"blue\" ];" << endl;

                         insertGraphNode(sse->entity.ptr);
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                         printf ("In graph(a_source_sequence_entry_ptr): Relevant nodes not yet handled: (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",sse->entity.ptr,graphNodeSet.size());
#endif
                       }

                    break;
                  }

            // DQ (4/7/2018): Added support for namespaces.
               case iek_namespace:
                  {
                    a_namespace_ptr namespace_node = (a_namespace_ptr)(sse->entity.ptr);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                    printf ("In graph(a_namespace_ptr): case iek_namespace: namespace_node = %p \n",namespace_node);
#endif
                    graph(namespace_node);
                    break;
                  }

               case iek_src_seq_sublist:
                  {
                 // DQ (1/5/2014): This is a list in the function scope to entities that are a part of the file scope.
                 // test2014_16.c demonstrates this IR node.
#if 1
                    if (graphNodeSet.find(sse->entity.ptr) == graphNodeSet.end())
                       {
                         bool includePointerValue = false;
                         file << "\"" << StringUtility::numberToString(sse->entity.ptr) << "\"[" << "label=\"" << summarize_entry_kind_in_string_kind(sse->entity,includePointerValue) << "\\n" 
                              << StringUtility::numberToString(sse->entity.ptr) + " : " + StringUtility::numberToString(counter) << "\" color=\"blue\" ];" << endl;

                         insertGraphNode(sse->entity.ptr);
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                         printf ("In graph(a_source_sequence_entry_ptr): iek_src_seq_sublist: (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",sse->entity.ptr,graphNodeSet.size());
#endif

#if 1
                         a_src_seq_sublist_ptr tmp_src_seq_sublist = (a_src_seq_sublist_ptr)(sse->entity.ptr);
                         while (tmp_src_seq_sublist->next != NULL)
                            {
                           // graph(src_seq_sublist->next);

                              file << "\"" << StringUtility::numberToString(tmp_src_seq_sublist) << "\" -> \"" << StringUtility::numberToString(tmp_src_seq_sublist->next) 
                                   << "\"[label=\"" << "src_seq_sublist->next" << "\"];" << endl;

                              tmp_src_seq_sublist = tmp_src_seq_sublist->next;
                            }
#endif
#if 1
                         a_src_seq_sublist_ptr src_seq_sublist = (a_src_seq_sublist_ptr)(sse->entity.ptr);
                         a_source_sequence_entry_ptr src_seq_entry = src_seq_sublist->source_sequence_list;
                      // while (src_seq_entry != NULL && src_seq_entry != src_seq_sublist->last_source_sequence_entry)
                         while (src_seq_entry != NULL)
                            {
                           // graph(src_seq_sublist);
                              graph(src_seq_entry);

                              file << "\"" << StringUtility::numberToString(src_seq_sublist) << "\" -> \"" << StringUtility::numberToString(src_seq_entry) 
                                   << "\"[label=\"" << "src_seq_sublist->source_sequence_list" << "\"];" << endl;

                           // Only traverse a subrange of the source dequence list pointed to by the src_seq_sublist.
                              if (src_seq_entry == src_seq_sublist->last_source_sequence_entry)
                                 {
                                   src_seq_entry = NULL;
                                 }
                              else
                                 {
                                   src_seq_entry = src_seq_entry->next;
                                 }
                              
                            }
#endif
                       }
#else
                    printf ("In graph(a_source_sequence_entry_ptr): case iek_src_seq_sublist: I don't think we see these entries \n");
                    ROSE_ASSERT(false);
#endif
                    break;
                  }

               case iek_src_seq_end_of_construct:
                  {
                 // We should never see one of these since the sse operator++() takes care of these, i think. We do find then when we got into the nested scopes.
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                    printf ("In graph(a_source_sequence_entry_ptr): case iek_src_seq_end_of_construct: Processing case iek_src_seq_end_of_construct \n");
#endif
                 // Output nodes
                    if (graphNodeSet.find(sse->entity.ptr) == graphNodeSet.end())
                       {
                         bool includePointerValue = false;
                         file << "\"" << StringUtility::numberToString(sse->entity.ptr) << "\"[" << "label=\"" << summarize_entry_kind_in_string_kind(sse->entity,includePointerValue) << "\\n" 
                              << StringUtility::numberToString(sse->entity.ptr) + " : " + StringUtility::numberToString(counter) << "\" color=\"blue\" ];" << endl;

                         insertGraphNode(sse->entity.ptr);
#if 0
                         printf ("In graph(a_source_sequence_entry_ptr): case iek_src_seq_end_of_construct: (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",sse->entity.ptr,graphNodeSet.size());
#endif
                       }

                    a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);

                    file << "\"" << StringUtility::numberToString(src_seq_end_of_construct) << "\" -> \"" << StringUtility::numberToString(src_seq_end_of_construct->entity.ptr) 
                         << "\"[label=\"" << "src_seq_end_of_construct->entity" << "\"];" << endl;

                    break;
                  }

            // TV (05/01/2018): [TODO] Case seen in Kripke
               case iek_static_assertion:
                  {
                    break;
                  }

            // DQ (12/29/2008): Added EDG 4.0 cases not handled
               case iek_none:
               case iek_source_file:
            // case iek_constant:
               case iek_param_type:
               case iek_routine_type_supplement:
               case iek_based_type_list_member:
               case iek_exception_specification:
               case iek_exception_specification_type:
               case iek_label:
               case iek_expr_node:
               case iek_for_loop:
               case iek_switch_case_entry:
               case iek_switch_stmt_descr:
               case iek_handler:
               case iek_try_supplement:
#if MICROSOFT_EXTENSIONS_ALLOWED
               case iek_microsoft_try_supplement:
               case iek_ms_attribute:
               case iek_ms_attribute_arg:
               case iek_ms_if_exists:
#endif
               case iek_block:
            // case iek_statement:
               case iek_object_lifetime:
               case iek_scope:
               case iek_id_name:
               case iek_string_text:
               case iek_other_text:
               case iek_internal_complex_value:
            // case iek_using_decl:
               case iek_dynamic_init:
               case iek_local_static_variable_init:
               case iek_vla_dimension:
               case iek_overriding_virtual_function:
               case iek_derivation_step:
               case iek_base_class_derivation:
               case iek_base_class:
               case iek_class_list_entry:
               case iek_routine_list_entry:
               case iek_class_type_supplement:
               case iek_template_param_type_supplement:
               case iek_constructor_init:
#if GNU_EXTENSIONS_ALLOWED
               case iek_asm_operand:
#if !RECORD_RAW_ASM_OPERAND_DESCRIPTIONS
               case iek_asm_operand_constraint:
#endif
               case iek_named_register_list:
#endif
               case iek_template_arg:
               case iek_new_delete_supplement:
               case iek_throw_supplement:
               case iek_condition_supplement:
               case iek_source_sequence_entry:
#if !HANDLE_SOURCE_SEQUENCE_END
            // case iek_src_seq_end_of_construct:
#endif
            // case iek_src_seq_sublist:
            // case iek_instantiation_directive:
               case iek_scope_orphaned_list_header:
               case iek_decl_position_supplement:
               case iek_template_decl:
               case iek_template_parameter:
               case iek_seq_number_lookup_entry:
               case iek_local_expr_node_ref:
               case iek_local_scope_ref:
               case iek_il_entity_list_entry:

            // DQ (7/21/2012): Added lambda cases (but might not be required here).
               case iek_lambda:
               case iek_lambda_capture:
               case iek_name_reference:
               case iek_name_qualifier:
               case iek_last:
                  {
                    printf ("In graph(a_source_sequence_entry_ptr): Case not handled! kind = %s \n",il_entry_kind_names[kind]);
                    ROSE_ASSERT(false);
                    break;
                  }

                default:
                  {
                    printf ("In graph(a_source_sequence_entry_ptr): Unhandled sse kind: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
                  }
             }

          ++sse;

          counter++;
        }
   }


void
EDG_ROSE_Graph::graph ( a_statement_ptr statement )
   {
     bool previoudlyProcessedBlock = false;

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_statement_ptr): statement = %p kind = %s \n",statement,summarize_statement_in_string(statement));
#endif

     if (graphNodeSet.find(statement) == graphNodeSet.end())
        {
          file << "\"" << StringUtility::numberToString(statement) << "\"[" << "label=\"" << summarize_statement_in_string(statement) << "\\n" << StringUtility::numberToString(statement) << "\"];" << endl;

          insertGraphNode(statement);
#if 0
          printf ("In graph(a_statement_ptr): top (statement = %p): graphNodeSet.size() = %" PRIuPTR " \n",statement,graphNodeSet.size());
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_statement_ptr): graphNodeSet previously processed in graphNodeSet: statement = %p kind = %s \n",statement,summarize_statement_in_string(statement));
#endif
          return;
        }

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_statement_ptr): statement = %p kind = %s \n",statement,summarize_statement_in_string(statement));
#endif

  // Use this code after the test for existence in graphNodeSet.  This permits nodes to be generated for unhandled leaf nodes that are compiler generated (e.g. non-explicit return-stmt)
  // Since these are not frequent statements this is OK, were as doing the same in the graph(acope_ptr) function would generate too large of a graph (e.g. declarations from header files).
     if (outputInGraph(statement) == false)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_statement_ptr): Statement will not be output in graph statement = %p kind = %s \n",statement,summarize_statement_in_string(statement));
#endif
       // Premature return for test2012_02.C (c++11 example).
       // return;
        }

     a_statement_kind kind = (a_statement_kind) (statement->kind);
     switch (kind)
        {
          case stmk_if:
             {
            // if statment test
               ROSE_ASSERT(statement->expr != NULL);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
               printf ("In graph(a_statement_ptr): case stmk_if: processing test: statement->expr = %p \n",statement->expr);
#endif
               graph(statement->expr);

               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(statement->expr) << "\"[label=\"" << "predicate : 1" << "\" weight=1000];" << endl;

            // if statment true block
               ROSE_ASSERT(statement->variant.if_stmt.then_statement != NULL);
#if 0
               printf ("In graph(a_statement_ptr): case stmk_if: processing TRUE branch: statement->variant.if_stmt.then_statement = %p \n",statement->variant.if_stmt.then_statement);
#endif
               graph(statement->variant.if_stmt.then_statement);

               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(statement->variant.if_stmt.then_statement) << "\"[label=\"" << "true case : 2" << "\" weight=1000];" << endl;

            // if statment false block
               if (statement->variant.if_stmt.else_statement != NULL)
                  {
#if 0
                    printf ("In graph(a_statement_ptr): case stmk_if: processing FALSE branch: statement->variant.if_stmt.else_statement = %p \n",statement->variant.if_stmt.else_statement);
#endif
                    graph(statement->variant.if_stmt.else_statement);

                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(statement->variant.if_stmt.else_statement) << "\"[label=\"" << "false case : 3" << "\" weight=1000];" << endl;
                  }
               break;
             }

          case stmk_block:
             {
            // block statment test

            // The block can be empty.
            // ROSE_ASSERT(statement->variant.block.statements != NULL);
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
               printf ("In graph(a_statement_ptr): case stmk_block: processing test: statement->variant.block.statements = %p \n",statement->variant.block.statements);
#endif
            // DQ (9/8/2013): I think this is true and so we can later simplify the code below.
               ROSE_ASSERT(previoudlyProcessedBlock == false);

            // Loop over statements in the block.
               a_statement_ptr s = statement->variant.block.statements;
               int counter = 0;
               while(s != NULL)
                  {
#if 0
                 // Debug by puting out the elements in the set.
                    printGraphNodeSet();
#endif
                 // graph(s);
                 // This will likely have already been processed or will be processed as part of the processing this statements parent block (so check).
                    if (graphNodeSet.find(s) == graphNodeSet.end())
                       {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                         printf ("In graph(a_statement_ptr): case stmk_block: block = %p processing block member s = %p kind = %s counter = %d \n",statement,s,summarize_statement_in_string(s),counter);
#endif
#if 0
                         printf ("****** Skipping case of adding statement s = %p for case stmk_block \n",s);
#endif
                      // DQ (11/19/2017): Uncommented as a test!  We might want to restrict this to specific kinds of nodes, 
                      // or make sure that the cases are seperately visited in each switch statement.
                         graph(s);
                       }
                      else
                       {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                         printf ("WARNING: In graph(a_statement_ptr): case stmk_block: block = %p s = %p kind = %s WAS PREVIOUSLY PROCESSED counter = %d \n",statement,s,summarize_statement_in_string(s),counter);
#endif
                       }

                 // file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(s) << "\"[label=\"" << "block member : " + StringUtility::numberToString(counter) << "\" weight=1000];" << endl;

                    string counter_string = StringUtility::numberToString(counter);
                    string label_string = "member #"  + counter_string;
                 // file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(s) << "\"[label=\"" << "member \" weight=1000];" << endl;
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(s) << "\"[label=\"" << label_string << " \" weight=1000];" << endl;

                    s = s->next;
                    counter++;
                  }

            // DQ (1/31/2018): Add scope information to graph.
               a_block_ptr block = statement->variant.block.extra_info;
               ROSE_ASSERT(block != NULL);

               graph(block);

               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(block) << "\"[label=\"" << "extra_info" << "\"];" << endl;

               a_scope_ptr scope = block->assoc_scope;
            // ROSE_ASSERT(scope != NULL);
               if (scope != NULL)
                  {
                    graph(scope,"block scope");

                    file << "\"" << StringUtility::numberToString(block) << "\" -> \"" << StringUtility::numberToString(scope) << "\"[label=\"" << "assoc_scope" << "\"];" << endl;
                  }

               break;
             }

          case stmk_expr:
             {
               graph(statement->expr);
               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(statement->expr) << "\"[label=\"" << "expression-statement" << "\" weight=1000];" << endl;
               break;
             }

          case stmk_decl:
             {
               an_il_entity_list_entry_ptr entities = statement->variant.decl.entities;
               int counter = 0;

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
               printf ("In graph(a_statement_ptr): case stmk_decl: entities = %p \n",entities);
#endif
               while (entities != NULL)
                  {
#if 0
                    printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
#endif
                    a_tagged_pointer entity = entities->entity;

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                    bool tmp_includePointerValue = false;
                    printf ("In graph(a_statement_ptr): case stmk_decl: before test in graphNodeSet: entity.ptr = %p = %s \n",entity.ptr,summarize_entry_kind_in_string_kind(entity,tmp_includePointerValue).c_str());
#endif
                    if (graphNodeSet.find(entity.ptr) == graphNodeSet.end())
                       {
                         bool includePointerValue = false;
                         file << "\"" << StringUtility::numberToString(entity.ptr) << "\"[" << "label=\"" << summarize_entry_kind_in_string_kind(entity,includePointerValue) << "\\n" 
                              << StringUtility::numberToString(entity.ptr) + " : " + StringUtility::numberToString(counter) << "\" color=\"green\" ];" << endl;
#if 0
                         printf ("In graph(a_statement_ptr): case stmk_decl: entity.ptr = %p = %s \n",entity.ptr,summarize_entry_kind_in_string_kind(entity,includePointerValue).c_str());
#endif
                         an_il_entry_kind entry_kind = (an_il_entry_kind)entity.kind;
#if 1
                         string name = il_entry_kind_names[entry_kind];
#endif
#if 0
                         printf ("In graph(a_statement_ptr): case stmk_decl: name = %s \n",name.c_str());
                         printf ("In graph(a_statement_ptr): case stmk_decl: entity.ptr = %p = %s \n",entity.ptr,summarize_entry_kind_in_string_kind(entity,true).c_str());
#endif
                      // insertGraphNode(entity.ptr);
                      // if (entry_kind == iek_expr_node)
                      // if (entry_kind == enk_variable)
                         if (true)
                            {
                           // an_expr_node_ptr expression = (an_expr_node_ptr)(entity.ptr);
                           // ROSE_ASSERT(expression != NULL);
                           // an_expr_node_kind kind = (an_expr_node_kind) (expression->kind);

                           // if ((an_expr_node_ptr)(entity.ptr))->kind == enk_variable)
                           // if (kind == enk_variable)
                           // if (entry_kind == enk_variable)
                              if (name == "variable")
                                 {
                                // a_variable_ptr variable = (a_variable_ptr)((an_expr_node_ptr)(entity.ptr));
                                   a_variable_ptr variable = (a_variable_ptr)(entity.ptr);
#if 0
                                   printf ("Found a variable: variable = %p \n",variable);
#endif
                                   graph(variable);
                                 }
                                else
                                 {
#if 0
                                   printf ("Variable NOT found \n");
#endif
                                // DQ (1/26/2019): Supress premature insertion into the list of already processed nodes.
                                // insertGraphNode(entity.ptr);
                                 }
                            }
                           else
                            {
#if 0
                              printf ("expression NOT found \n");
#endif
                              insertGraphNode(entity.ptr);
                            }
#if 0
                         printf ("In graph(a_statement_ptr): case stmk_decl: (entity.ptr = %p): graphNodeSet.size() = %" PRIuPTR " \n",entity.ptr,graphNodeSet.size());
#endif
                       }

                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(entity.ptr) << "\"[label=\"" << "stmk_decl:entity" + StringUtility::numberToString(counter) << "\" weight=1000];" << endl;

                    entities = entities->next;
                    counter++;
                  }
#if 0
               printf ("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");
               printf ("Leaving graph(a_statement_ptr): case stmk_decl: entities = %p \n",entities);
               printf ("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");
#endif
               break;
             }


          case stmk_set_vla_size:
             {
               a_vla_dimension_ptr vla_dimension = statement->variant.vla_dimension;

               while (vla_dimension != NULL)
                  {
                    graph(vla_dimension);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(vla_dimension) << "\"[label=\"" << "vla_dimension" << "\" weight=1000];" << endl;

                    vla_dimension = vla_dimension->next;
                  }

               break;
             }

          case stmk_for:
             {
               a_for_loop_ptr for_loop = statement->variant.for_loop.extra_info;

            // Generate graph for for_init_scope.
               a_scope_ptr for_init_scope = for_loop->for_init_scope;
            // ROSE_ASSERT(for_init_scope != NULL);
               if (for_init_scope != NULL)
                  {

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
                    printf ("In graph(a_statement_ptr): case stmk_for: processing for_init_scope: for_init_scope = %p \n",for_init_scope);
#endif
                    graph(for_init_scope,"for_init_scope");
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(for_init_scope) << "\"[label=\"" << "for_init_scope" << "\" weight=1000];" << endl;
#if 0
                    printf ("In graph(a_statement_ptr): case stmk_for: calling summarize_scope(): for_init_scope = %p \n",for_init_scope);
                    summarize_scope(for_init_scope);
#endif
                  }

            // initialization
               a_statement_ptr initialization = for_loop->initialization;
               if (initialization != NULL)
                  {
                    graph(initialization);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(initialization) << "\"[label=\"" << "initialization" << "\" weight=1000];" << endl;
#if 0
                    printf ("In graph(a_statement_ptr): case stmk_for: calling summarize_statement(): initialization = %p \n",initialization);
                    summarize_statement(initialization);
#endif
                  }

            // Generate graph for the for_loop's test statement (BTW, it is a statement and not an expression)
               if (statement->expr != NULL)
                  {
#if 0
                    printf ("In graph(a_statement_ptr): case stmk_for: processing test: statement->expr = %p \n",statement->expr);
#endif
                    graph(statement->expr);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(statement->expr) << "\"[label=\"" << "predicate" << "\" weight=1000];" << endl;
                  }

            // Generate graph for the for_loop's increment.
               an_expr_node_ptr increment = for_loop->increment;
               if (increment != NULL)
                  {
                    graph(increment);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(increment) << "\"[label=\"" << "increment" << "\" weight=1000];" << endl;
                  }

            // Generate graph for the for_loop's body
               if (statement->variant.for_loop.statement != NULL)
                  {
#if 0
                    printf ("In graph(a_statement_ptr): case stmk_for: processing for loop body: statement->variant.for_loop.statement = %p \n",statement->variant.for_loop.statement);
#endif
                    graph(statement->variant.for_loop.statement);

                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(statement->variant.for_loop.statement) << "\"[label=\"" << "body" << "\" weight=1000];" << endl;
                  }
               break;
             }

          case stmk_range_based_for:
             {
            // a_for_loop_ptr for_loop = statement->variant.for_loop.extra_info;

            // SgStatement* body = build_basic_block_if_multiple_statements(parse_statement(sse));
            // ROSE_ASSERT(body != NULL);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
               printf ("In graph(a_statement_ptr): case stmk_range_based_for: statement = %p \n",statement);
#endif
               a_range_based_for_loop_ptr extra_info = statement->variant.range_based_for_loop.extra_info;
               ROSE_ASSERT(extra_info != NULL);

               a_variable_ptr iterator = extra_info->iterator;
               if (iterator != NULL)
                  {
                    graph(iterator);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(iterator) << "\"[label=\"" << "iterator" << "\" weight=1000];" << endl;
                  }

               a_variable_ptr range = extra_info->range;
               if (range != NULL)
                  {
                    graph(range);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(range) << "\"[label=\"" << "range" << "\" weight=1000];" << endl;
                  }

               a_scope_ptr   range_based_for_scope = extra_info->range_based_for_scope;
               if (range_based_for_scope != NULL)
                  {
                    graph(range_based_for_scope,"range scope");
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(range_based_for_scope) << "\"[label=\"" << "range_based_for_scope" << "\"  color=\"lightgreen\" weight=1000];" << endl;
                  }

               a_scope_ptr   iterator_scope = extra_info->iterator_scope;
               if (iterator_scope != NULL)
                  {
                    graph(iterator_scope,"iterator scope");
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(iterator_scope) << "\"[label=\"" << "iterator_scope" << "\"  color=\"lightgreen\" weight=1000];" << endl;
                  }

               a_variable_ptr begin = extra_info->begin;
               if (begin != NULL)
                  {
                    graph(begin);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(begin) << "\"[label=\"" << "begin" << "\" weight=1000];" << endl;
                  }

               a_variable_ptr end = extra_info->end;
               if (end != NULL)
                  {
                    graph(end);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(end) << "\"[label=\"" << "end" << "\" weight=1000];" << endl;
                  }

               an_expr_node_ptr ne_call_expr = extra_info->ne_call_expr;
               if (ne_call_expr != NULL)
                  {
                    graph(ne_call_expr);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(ne_call_expr) << "\"[label=\"" << "ne_call_expr" << "\" weight=1000];" << endl;
                  }

               an_expr_node_ptr incr_call_expr = extra_info->incr_call_expr;
               if (incr_call_expr != NULL)
                  {
                    graph(incr_call_expr);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(incr_call_expr) << "\"[label=\"" << "incr_call_expr" << "\" weight=1000];" << endl;
                  }

               a_statement_ptr body = statement->variant.range_based_for_loop.statement;
               if (body != NULL)
                  {
                    graph(body);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(body) << "\"[label=\"" << "body" << "\" weight=1000];" << endl;
                  }

               break;
             }

          case stmk_init:
             {
               a_dynamic_init_ptr dynamic_init = statement->variant.dynamic_init;
               graph(dynamic_init);
               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(dynamic_init) << "\"[label=\"" << "dynamic_init" << "\" weight=1000];" << endl;
               break;
             }

          case stmk_return:
             {
               if (statement->variant.return_dynamic_init != NULL)
                  {
                    graph(statement->variant.return_dynamic_init);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(statement->variant.return_dynamic_init) << "\"[label=\"" << "return_expr" << "\" weight=1000];" << endl;
                  }
                 else
                  {
                    if (statement->expr != NULL)
                       {
                         graph(statement->expr);
                         file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(statement->expr) << "\"[label=\"" << "return_expr" << "\" weight=1000];" << endl;
                       }
                  }
               break;
             }

          case stmk_while:
             {
               graph(statement->expr);
               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(statement->expr) << "\"[label=\"" << "expression-statement" << "\" weight=1000];" << endl;

               a_statement_ptr loop_body = statement->variant.loop_statement;
               if (loop_body != NULL)
                  {
                    graph(loop_body);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(loop_body) << "\"[label=\"" << "while-body" << "\" weight=1000];" << endl;
                  }
               break;
             }

          case stmk_switch:
             {
               graph(statement->expr);
               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(statement->expr) << "\"[label=\"" << "expression-statement" << "\" weight=1000];" << endl;

            // DQ (8/19/2020): This is incorrect.
            // a_statement_ptr body = statement->variant.loop_statement;
               a_statement_ptr body = statement->variant.switch_stmt.body_statement;
               if (body != NULL)
                  {
                    graph(body);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(body) << "\"[label=\"" << "switch-statement-body" << "\" weight=1000];" << endl;
                  }

               a_switch_stmt_descr_ptr extra_info = statement->variant.switch_stmt.extra_info;
               ROSE_ASSERT(extra_info != NULL);
               graph(extra_info);
               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(extra_info) << "\"[label=\"" << "extra_info" << "\" weight=1000];" << endl;

               break;
             }

       // DQ (1/14/2014): Added case to support debugging C99 test2014_01.c (switch statement details).
          case stmk_switch_case:
             {
               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(statement->variant.switch_case.switch_statement) << "\"[label=\"" << "expression-statement" << "\" color=\"purple\" weight=1000];" << endl;

            // DQ (8/19/2020): Adding support for switch cases.
            // a_statement_ptr switch_statement;
            // a_switch_case_entry_ptr extra_info;
            // a_statement_ptr body = statement->variant.loop_statement;
               a_statement_ptr switch_statement = statement->variant.switch_case.switch_statement;
               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(switch_statement) << "\"[label=\"" << "switch-statement" << "\" weight=1000];" << endl;

               a_switch_case_entry_ptr extra_info = statement->variant.switch_case.extra_info;
#if 1
            // DQ (8/19/2020): This appears to only point to itself.
               a_statement_ptr switch_case_statement = extra_info->stmt;
               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(switch_case_statement) << "\"[label=\"" << "associated-switch-statement" << "\" weight=1000];" << endl;
#endif
               a_constant_ptr constant = extra_info->case_value;

            // DQ (3/25/2021): Adding assertion (where false, this is likely a default case).
            // ROSE_ASSERT(constant != NULL);
               if (constant != NULL)
                  {
                    graph(constant);
                  }

            // graph(statement->expr);
               file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(constant) << "\"[label=\"" << "case-constant" << "\" weight=1000];" << endl;

#if GNU_EXTENSIONS_ALLOWED
               a_constant_ptr range_end = extra_info->range_end;
               if (range_end != NULL)
                  {
                    graph(constant);
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(range_end) << "\"[label=\"" << "range-end-constant" << "\" weight=1000];" << endl;
                  }
#endif
#if 0
               a_switch_case_entry_ptr next = extra_info->next;
               if (next != NULL)
                  {
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(next) << "\"[label=\"" << "next-case" << "\" weight=1000];" << endl;
                  }

               a_switch_case_entry_ptr next_on_sorted_list = extra_info->next_on_sorted_list;
               if (next_on_sorted_list != NULL)
                  {
                    file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(next_on_sorted_list) << "\"[label=\"" << "next-sorted-case" << "\" weight=1000];" << endl;
                  }
#endif
#if 0
               printf ("Detected case of stmk_switch_case \n");
               ROSE_ASSERT(false);
#endif
               break;
             }

          default:
             {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
               printf ("In graph(a_statement_ptr): Unhandled statement kind: statement = %p kind = %s \n",statement,summarize_statement_in_string(statement));
#endif
             }
        }

  // DQ (8/20/2020): Look for attributes (edg does not appear to use this data member).
     if (statement->is_fallthrough_statement == true)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_statement_ptr): Detected fallthrough attribute \n");
#endif
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_statement_ptr): fallthrough == false \n");
#endif
        }

  // DQ (8/20/2020): Look for attributes.
     an_attribute_ptr attributes = statement->attributes;
     if (attributes != NULL)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_statement_ptr): found attributes \n");
#endif
          graph(attributes);
          file << "\"" << StringUtility::numberToString(statement) << "\" -> \"" << StringUtility::numberToString(attributes) << "\"[label=\"" << "attributes" << "\"];" << endl;
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_statement_ptr): NO attributes \n");
#endif
        }

   }


// DQ (8/19/2020): Adding more graph support for the switch_case_entry.
void
EDG_ROSE_Graph::graph ( a_switch_case_entry_ptr switch_case_entry )
   {
     ROSE_ASSERT(switch_case_entry != NULL);

     if (graphNodeSet.find(switch_case_entry) == graphNodeSet.end())
        {
       // Output nodes
          file << "\"" << StringUtility::numberToString(switch_case_entry) << "\"[" << "label=\"" << "switch_case_entry" << "\\n" << StringUtility::numberToString(switch_case_entry) << "\"];" << endl;

          insertGraphNode(switch_case_entry);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_switch_case_entry_ptr): top (switch_case_entry = %p): graphNodeSet.size() = %" PRIuPTR " \n",switch_case_entry,graphNodeSet.size());
#endif
        }
       else
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_switch_case_entry_ptr): node was already processed (return): switch_case_entry = %p \n",switch_case_entry);
#endif
          return;
        }

  // graph(switch_case_entry->case_value);
     if (switch_case_entry->case_value != NULL)
        {
          graph(switch_case_entry->case_value);
        }

     file << "\"" << StringUtility::numberToString(switch_case_entry) << "\" -> \"" << StringUtility::numberToString(switch_case_entry->case_value) << "\"[label=\"" << "case_value" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

#if GNU_EXTENSIONS_ALLOWED
  // DQ (8/22/2020): This data member is only present under GNU, and not under MS Windows (caught by Jenkins).
     if (switch_case_entry->range_end)
        {
          graph (switch_case_entry->range_end);
          file << "\"" << StringUtility::numberToString(switch_case_entry) << "\" -> \"" << StringUtility::numberToString(switch_case_entry->range_end) << "\"[label=\"" << "range_end" << "\" weight=1000];" << endl;
        }
#endif

     a_switch_case_entry_ptr next = switch_case_entry->next;
     if (next != NULL)
        {
          graph(next);
          file << "\"" << StringUtility::numberToString(switch_case_entry) << "\" -> \"" << StringUtility::numberToString(next) << "\"[label=\"" << "next-case" << "\" weight=1000];" << endl;
        }

     a_switch_case_entry_ptr next_on_sorted_list = switch_case_entry->next_on_sorted_list;
     if (next_on_sorted_list != NULL)
        {
          graph(next_on_sorted_list);
          file << "\"" << StringUtility::numberToString(switch_case_entry) << "\" -> \"" << StringUtility::numberToString(next_on_sorted_list) << "\"[label=\"" << "next-sorted-case" << "\" weight=1000];" << endl;
        }

#if 0
     if (switch_case_entry->original_dimension)
        {
          graph (switch_case_entry->original_dimension);
          file << "\"" << StringUtility::numberToString(switch_case_entry) << "\" -> \"" << StringUtility::numberToString(switch_case_entry->original_dimension) << "\"[label=\"" << "original_dimension" << "\" weight=1000];" << endl;
        }
#endif
#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif
   }




// DQ (8/19/2020): Adding more graph support for the switch_stmt_descriptor.
void
EDG_ROSE_Graph::graph ( a_switch_stmt_descr_ptr switch_stmt_descr )
   {
     ROSE_ASSERT(switch_stmt_descr != NULL);

     if (graphNodeSet.find(switch_stmt_descr) == graphNodeSet.end())
        {
       // Output nodes
          file << "\"" << StringUtility::numberToString(switch_stmt_descr) << "\"[" << "label=\"" << "switch_stmt_descr" << "\\n" << StringUtility::numberToString(switch_stmt_descr) << "\"];" << endl;

          insertGraphNode(switch_stmt_descr);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_switch_stmt_descr_ptr): top (switch_stmt_descr = %p): graphNodeSet.size() = %" PRIuPTR " \n",switch_stmt_descr,graphNodeSet.size());
#endif
        }
       else
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_switch_stmt_descr_ptr): node was already processed (return): switch_stmt_descr = %p \n",switch_stmt_descr);
#endif
          return;
        }

     void* previous_case_ptr = (void*)(switch_stmt_descr);
     a_switch_case_entry_ptr tmp_ptr = switch_stmt_descr->cases;
     while(tmp_ptr != NULL)
        {
          graph(tmp_ptr);
       // file << "\"" << StringUtility::numberToString(switch_stmt_descr) << "\" -> \"" << StringUtility::numberToString(tmp_ptr) << "\"[label=\"" << "cases" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
          file << "\"" << StringUtility::numberToString(previous_case_ptr) << "\" -> \"" << StringUtility::numberToString(tmp_ptr) << "\"[label=\"" << "cases" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

          previous_case_ptr = (void*)(tmp_ptr);

          tmp_ptr = tmp_ptr->next;
        }

     previous_case_ptr = (void*)(switch_stmt_descr);
     a_switch_case_entry_ptr tmp_sorted_ptr = switch_stmt_descr->sorted_cases;
     while(tmp_sorted_ptr != NULL)
        {
          graph(tmp_sorted_ptr);
       // file << "\"" << StringUtility::numberToString(switch_stmt_descr) << "\" -> \"" << StringUtility::numberToString(tmp_sorted_ptr) << "\"[label=\"" << "sorted--case" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
          file << "\"" << StringUtility::numberToString(previous_case_ptr) << "\" -> \"" << StringUtility::numberToString(tmp_sorted_ptr) << "\"[label=\"" << "sorted--case" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

          previous_case_ptr = (void*)(tmp_sorted_ptr);

          tmp_sorted_ptr = tmp_sorted_ptr->next;
        }

     if (switch_stmt_descr->default_case)
        {
          graph (switch_stmt_descr->default_case);
          file << "\"" << StringUtility::numberToString(switch_stmt_descr) << "\" -> \"" << StringUtility::numberToString(switch_stmt_descr->default_case) << "\"[label=\"" << "default-case" << "\" weight=1000];" << endl;
        }

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif
   }


// DQ (8/20/2020): Adding more graph support for attributes.
// void graph ( an_attribute_ptr attributes );

void
EDG_ROSE_Graph::graph ( an_attribute_ptr attributes )
   {
     ROSE_ASSERT(attributes != NULL);

     if (graphNodeSet.find(attributes) == graphNodeSet.end())
        {
       // Output nodes
          file << "\"" << StringUtility::numberToString(attributes) << "\"[" << "label=\"" << "attributes" << "\\n" << StringUtility::numberToString(attributes) << "\"];" << endl;

          insertGraphNode(attributes);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(an_attribute_ptr): top (attributes = %p): graphNodeSet.size() = %" PRIuPTR " \n",attributes,graphNodeSet.size());
#endif
        }
       else
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(an_attributes_ptr): node was already processed (return): attributes = %p \n",attributes);
#endif
          return;
        }

     an_attribute_ptr next = attributes->next;
     if (next != NULL)
        {
          graph(next);
          file << "\"" << StringUtility::numberToString(attributes) << "\" -> \"" << StringUtility::numberToString(next) << "\"[label=\"" << "next" << "\" weight=1000];" << endl;
        }

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif
   }



// DQ (2/10/2018): Adding more graph support for vla typed paramters.
void
EDG_ROSE_Graph::graph ( a_vla_dimension_ptr vla_dimension )
   {
     ROSE_ASSERT(vla_dimension != NULL);

     if (graphNodeSet.find(vla_dimension) == graphNodeSet.end())
        {
       // Output nodes
          file << "\"" << StringUtility::numberToString(vla_dimension) << "\"[" << "label=\"" << "vla_dimension" << "\\n" << StringUtility::numberToString(vla_dimension) << "\"];" << endl;

          insertGraphNode(vla_dimension);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_vla_dimension_ptr): top (vla_dimension = %p): graphNodeSet.size() = %" PRIuPTR " \n",vla_dimension,graphNodeSet.size());
#endif
        }
       else
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_vla_dimension_ptr): node was already processed (return): vla_dimension = %p \n",vla_dimension);
#endif
          return;
        }

     graph(vla_dimension->type);
     file << "\"" << StringUtility::numberToString(vla_dimension) << "\" -> \"" << StringUtility::numberToString(vla_dimension->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

     if (vla_dimension->dimension_expr)
        {
          graph (vla_dimension->dimension_expr);
          file << "\"" << StringUtility::numberToString(vla_dimension) << "\" -> \"" << StringUtility::numberToString(vla_dimension->dimension_expr) << "\"[label=\"" << "dimension_expr" << "\" weight=1000];" << endl;
        }

     if (vla_dimension->original_dimension)
        {
          graph (vla_dimension->original_dimension);
          file << "\"" << StringUtility::numberToString(vla_dimension) << "\" -> \"" << StringUtility::numberToString(vla_dimension->original_dimension) << "\"[label=\"" << "original_dimension" << "\" weight=1000];" << endl;
        }

#if 0
  // This is handled by the caller.
     if (vla_dimension->next)
        {
          printf ("Recursive call to output next VLA entry \n");

          graph (vla_dimension->next);
          file << "\"" << StringUtility::numberToString(vla_dimension) << "\" -> \"" << StringUtility::numberToString(vla_dimension->next) << "\"[label=\"" << "original_dimension" << "\" weight=1000];" << endl;
#if 1
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
#endif
   }


void
EDG_ROSE_Graph::graph ( an_expr_node_ptr expression )
   {

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(an_expr_node_ptr): expression = %p kind = %s \n",expression,summarize_expression_in_string(expression).c_str());
#endif

  // DQ (2/10/2016): Added assertion.
     ROSE_ASSERT(expression != NULL);

     if (graphNodeSet.find(expression) == graphNodeSet.end())
        {
       // Output nodes
          file << "\"" << StringUtility::numberToString(expression) << "\"[" << "label=\"" << summarize_expression_in_string(expression) << "\\n" << StringUtility::numberToString(expression) << "\"];" << endl;

          insertGraphNode(expression);
#if 0
          printf ("In graph(an_expr_node_ptr): top (expression = %p): graphNodeSet.size() = %" PRIuPTR " \n",expression,graphNodeSet.size());
#endif
        }

     graph(expression->type);

  // DQ (3/19/2014): removed "decorate" keyword since it is an error for latest version of zgrviewer (or maybe dot).
  // file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(expression->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" decorate labelfontcolor=\"goldenrod\" weight=1];" << endl;
     file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(expression->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

     an_expr_node_kind kind = (an_expr_node_kind) (expression->kind);
     switch (kind)
        {
          case enk_operation:
             {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
               an_expr_operator_kind_tag inner_kind = (an_expr_operator_kind_tag)(expression->variant.operation.kind);
               printf ("In graph(an_expr_node_ptr): case enk_operation: Calling graph(an_expr_node_ptr) on operands in case enk_operation = %p = %s \n",expression,summarize_expr_operator(inner_kind).c_str());
#endif
#if 1
            // loop over operands.
               int counter = 0;
               for (an_expr_node_ptr operand = expression->variant.operation.operands; operand != NULL; operand = operand->next)
                  {
                    graph(operand);

                 // Generate an edge from expression to the operand.
                    file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(operand) << "\"[label=\"" << "operand : " + StringUtility::numberToString(counter) << "\"];" << endl;

                    counter++;
                  }
#endif
               break;
             }

           case enk_variable:
             {
               a_variable_ptr variable = expression->variant.variable.ptr;
               ROSE_ASSERT(variable != NULL);

               graph(variable);
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(variable) << "\"[label=\"" << "variable" << "\"];" << endl;
               break;
             }

           case enk_constant:
             {
               a_constant_ptr constant = expression->variant.constant.ptr;
               ROSE_ASSERT(constant != NULL);

               graph(constant);
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(constant) << "\"[label=\"" << "constant" << "\"];" << endl;
               break;
             }

           case enk_lambda:
             {
               a_lambda_ptr lambda = expression->variant.init.source.lambda;
               ROSE_ASSERT(lambda != NULL);

            // Output the lambda initialization
               a_dynamic_init_ptr lambda_initialization_dip = expression->variant.init.dynamic_init;
               graph(lambda_initialization_dip);
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(lambda_initialization_dip) << "\"[label=\"" << "lambda initialization" << "\"];" << endl;

            // Output the lambda routine
               a_routine_ptr lambda_routine = lambda->lambda_routine;
               ROSE_ASSERT(lambda_routine != NULL);
               graph(lambda_routine);
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(lambda_routine) << "\"[label=\"" << "lambda routine" << "\"];" << endl;
               
#if 0
               graph(lambda);
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(lambda) << "\"[label=\"" << "variable" << "\"];" << endl;
#endif
               break;
             }

           case enk_routine:
             {
               a_routine_ptr routine = expression->variant.routine.ptr;
               ROSE_ASSERT(routine != NULL);

               graph(routine);
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(routine) << "\"[label=\"" << "variable" << "\"];" << endl;
               break;
             }

           case enk_temp_init:
             {
               a_dynamic_init_ptr dynamic_initializer = expression->variant.init.dynamic_init;
               ROSE_ASSERT(dynamic_initializer != NULL);

               graph(dynamic_initializer);
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(dynamic_initializer) << "\"[label=\"" << "dynamic initializer" << "\"];" << endl;
               break;
             }

        // DQ (10/20/2016): Adding support for conditionals.
           case enk_condition:
             {
               a_condition_supplement_ptr condition_supplement = expression->variant.condition;
               ROSE_ASSERT(condition_supplement != NULL);

            // printf ("Skipping output of condition_supplement \n");

               ROSE_ASSERT(condition_supplement->dynamic_init != NULL);
               graph(condition_supplement->dynamic_init);
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(condition_supplement->dynamic_init) << "\"[label=\"" << "dynamic_init" << "\"];" << endl;

               ROSE_ASSERT(condition_supplement->scope != NULL);
               graph(condition_supplement->scope,"condition scope");
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(condition_supplement->scope) << "\"[label=\"" << "condition scope" << "\"];" << endl;


               ROSE_ASSERT(condition_supplement->expr != NULL);

               graph(condition_supplement->expr);
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(condition_supplement->expr) << "\"[label=\"" << "expr" << "\"];" << endl;

               break;
             }

#if GNU_EXTENSIONS_ALLOWED
        // DQ (7/6/2019): Adding support for GNU statement expressions.
           case enk_statement:
             {
               a_statement_ptr associated_statement = expression->variant.statement;
               ROSE_ASSERT(associated_statement != NULL);

               graph(associated_statement);
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(associated_statement) << "\"[label=\"" << "inner statement" << "\"];" << endl;
               break;
             }
#endif

          case enk_new_delete: 
             {
            // new or delete operation
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
               printf ("In graph(an_expr_node_ptr): case enk_new_delete \n");
#endif
               a_new_delete_supplement_ptr new_delete_supplement = expression->variant.new_delete;
               ROSE_ASSERT(new_delete_supplement != NULL);

               graph(new_delete_supplement);
               file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(new_delete_supplement) << "\"[label=\"" << "new_delete_supplement" << "\"];" << endl;

#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
               break;
             }

          default:
             {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
               printf ("In graph(an_expr_node_ptr): Unhandled expression kind: expression = %p kind = %s \n",expression,summarize_expression_in_string(expression).c_str());
#endif
             }
        }
   }



void
EDG_ROSE_Graph::graph ( a_new_delete_supplement_ptr new_delete_supplement )
   {
  // DQ (1/19/2020): Adding support for new and delete operators.

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_new_delete_supplement_ptr): new_delete_supplement = %p \n",new_delete_supplement);
#endif

     ROSE_ASSERT(new_delete_supplement != NULL);

     if (graphNodeSet.find(new_delete_supplement) == graphNodeSet.end())
        {
       // Output nodes
          file << "\"" << StringUtility::numberToString(new_delete_supplement) << "\"[" << "label=\"" << "new_delete_supplement" << "\\n" << StringUtility::numberToString(new_delete_supplement) << "\"];" << endl;

          insertGraphNode(new_delete_supplement);
#if 0
          printf ("In graph(a_new_delete_supplement_ptr): top (new_delete_supplement = %p): graphNodeSet.size() = %" PRIuPTR " \n",new_delete_supplement,graphNodeSet.size());
#endif
        }

     graph(new_delete_supplement->type);

  // DQ (3/19/2014): removed "decorate" keyword since it is an error for latest version of zgrviewer (or maybe dot).
  // file << "\"" << StringUtility::numberToString(expression) << "\" -> \"" << StringUtility::numberToString(expression->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" decorate labelfontcolor=\"goldenrod\" weight=1];" << endl;
     file << "\"" << StringUtility::numberToString(new_delete_supplement) << "\" -> \"" << StringUtility::numberToString(new_delete_supplement->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;


     if (new_delete_supplement->routine != NULL)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_new_delete_supplement_ptr): Add support for routine = %p \n",new_delete_supplement->routine);
#endif
          graph(new_delete_supplement->routine);

          file << "\"" << StringUtility::numberToString(new_delete_supplement) << "\" -> \"" << StringUtility::numberToString(new_delete_supplement->routine) << "\"[label=\"" << "routine" << "\"];" << endl;
        }

     if (new_delete_supplement->arg != NULL)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_new_delete_supplement_ptr): Add support for arg = %p \n",new_delete_supplement->arg);
#endif
          graph(new_delete_supplement->arg);

          file << "\"" << StringUtility::numberToString(new_delete_supplement) << "\" -> \"" << StringUtility::numberToString(new_delete_supplement->arg) << "\"[label=\"" << "arg" << "\"];" << endl;
        }

   }


void
EDG_ROSE_Graph::graph ( a_constant_ptr constant )
   {
     ROSE_ASSERT(constant != NULL);

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_constant_ptr): constant = %p kind = %s \n",constant,summarize_constant_kind_in_string(constant));
#endif

     if (constant->type != NULL) {
       graph(constant->type);
       file << "\"" << StringUtility::numberToString(constant) << "\" -> \"" << StringUtility::numberToString(constant->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
     }

     if (constant->orig_type != NULL) {
       graph(constant->orig_type);
       file << "\"" << StringUtility::numberToString(constant) << "\" -> \"" << StringUtility::numberToString(constant->orig_type) << "\"[label=\"" << "orig_type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
     }
  // Handle the original expression tree (unfolded constant expression).
     if (constant->expr != NULL)
        {
          graph(constant->expr);
       // DQ (7/13/2020): I think the label should not be "original expression tree" here.
       // file << "\"" << StringUtility::numberToString(constant) << "\" -> \"" << StringUtility::numberToString(constant->expr) << "\"[label=\"" << "original expression tree" << "\"];" << endl;
          file << "\"" << StringUtility::numberToString(constant) << "\" -> \"" << StringUtility::numberToString(constant->expr) << "\"[label=\"" << "original constant expression" << "\"];" << endl;
        }

     a_scope_ptr scope = constant->source_corresp.parent_scope;
     if (scope != NULL) {
       graph(scope,"parent scope");
       file << "\"" << StringUtility::numberToString(constant) << "\" -> \"" << StringUtility::numberToString(scope) << "\"[label=\"" << "parent_scope" << "\" color=\"lightgreen\"];" << endl;
     }

     file << "\"" << StringUtility::numberToString(constant) << "\"[" << "label=\"" << string(summarize_constant_kind_in_string(constant)) << "\\n" << StringUtility::numberToString(constant);

     switch (constant->kind) {
       case ck_error:
       case ck_void:
         {
           // EMPTY
           file << "\"];" << endl;
           break;
         }
       case ck_integer:
         {
           file << "\\ninteger value: " << (int)constant->variant.integer_value;
           file << "\"];" << endl;
           break;
         }
#if FIXED_POINT_ALLOWED
       case ck_fixed_point:
         {
           // a_fixed_point_value constant->variant.fixed_point_value;
           file << "\"];" << endl;
           break;
         }
#endif /* FIXED_POINT_ALLOWED */
       case ck_string:
         {
           // a_targ_size_t constant->variant.string.length;
           // a_const_char * constant->variant.string.value;
#if DO_IL_LOWERING && ASSIGN_STRING_LITERAL_SEQUENCE_NUMBERS
           // unsigned long constant->variant.string.sequence_number;
#endif /* DO_IL_LOWERING && ASSIGN_STRING_LITERAL_SEQUENCE_NUMBERS */
           // a_string_or_char_literal_kind constant->variant.string.literal_kind;
           file << "\"];" << endl;
           break;
         }
       case ck_float:
         {
           // an_internal_float_value constant->variant.float_value;
           file << "\"];" << endl;
           break;
         }
#if C99_IL_EXTENSIONS_SUPPORTED
       case ck_complex:
         {
           // an_internal_complex_value_ptr constant->variant.complex_value;
           file << "\"];" << endl;
           break;
         }
       case ck_imaginary:
         {
           // an_internal_float_value constant->variant.float_value;
           file << "\"];" << endl;
           break;
         }
#endif /* C99_IL_EXTENSIONS_SUPPORTED */
       case ck_address:
         {
           // a_bit_field constant->variant.address.from_reference_variable:1;
           switch (constant->variant.address.kind) {
#if MICROSOFT_EXTENSIONS_ALLOWED
             case abk_cli_array:
               {
                 // EMPTY
                 file << "\"];" << endl;
                 break;
               }
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
             case abk_routine:
               {
                 // a_routine_ptr constant->variant.address.variant.routine;
                 file << "\"];" << endl;
                 break;
               }
             case abk_variable:
               {
                 // a_variable_ptr constant->variant.address.variant.variable;
                 file << "\"];" << endl;
                 break;
               }
             case abk_constant:
               {
                 // a_constant_ptr constant->variant.address.variant.constant;
                 file << "\"];" << endl;
                 break;
               }
             case abk_temporary:
               {
                 // a_constant_ptr constant->variant.address.variant.constant;
                 file << "\"];" << endl;
                 break;
               }
             case abk_uuidof:
               {
                 // a_type_ptr constant->variant.address.variant.type;
                 file << "\"];" << endl;
                 break;
               }
             case abk_typeid:
               {
                 // a_type_ptr constant->variant.address.variant.type;
                 file << "\"];" << endl;
                 break;
               }
#if MICROSOFT_EXTENSIONS_ALLOWED
             case abk_cli_typeid:
               {
                 // a_type_ptr constant->variant.address.variant.type;
                 file << "\"];" << endl;
                 break;
               }
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
             case abk_label:
               {
                 // a_label_ptr constant->variant.address.variant.label;
                 file << "\"];" << endl;
                 break;
               }
             default:
               {
                 ROSE_ASSERT(false);
               }
           }
           // a_targ_ptrdiff_t constant->variant.address.offset;
           // a_dynamic_init_ptr constant->variant.address.assoc_dyn_init;
           break;
         }
       case ck_ptr_to_member:
         {
           // a_base_class_ptr constant->variant.ptr_to_member.casting_base_class;
           // a_name_reference_ptr constant->variant.ptr_to_member.name_reference;
           // a_bit_field constant->variant.ptr_to_member.cast_to_base:1;
           if (constant->variant.ptr_to_member.is_function_ptr) {
             // a_routine_ptr constant->variant.ptr_to_member.variant.routine;
           } else {
             // a_field_ptr constant->variant.ptr_to_member.variant.field;
           }
           file << "\"];" << endl;
           break;
         }
#if GNU_EXTENSIONS_ALLOWED
       case ck_label_difference:
         {
           // a_constant_ptr constant->variant.label_difference.from_address;
           // a_constant_ptr constant->variant.label_difference.to_address;
           file << "\"];" << endl;
           break;
         }
#endif /* GNU_EXTENSIONS_ALLOWED */
#if DO_IL_LOWERING && GENERATE_EH_TABLES && !DO_FULL_PORTABLE_EH_LOWERING
       case ck_stack_offset:
         {
           // a_variable_ptr constant->variant.stack_offset.variable;
           // a_targ_size_t constant->variant.stack_offset.offset;
           file << "\"];" << endl;
           break;
         }
#endif /* DO_IL_LOWERING && ... */
       case ck_dynamic_init:
         {
           // a_dynamic_init_ptr constant->variant.dynamic_init;
           file << "\"];" << endl;
           break;
         }
       case ck_aggregate:
         {
           // a_constant_ptr constant->variant.aggregate.first_constant;
           // a_constant_ptr constant->variant.aggregate.last_constant;
           // a_bit_field constant->variant.aggregate.has_dynamic_init_component:1;
           file << "\"];" << endl;
           break;
         }
       case ck_init_repeat:
         {
           // a_constant_ptr constant->variant.init_repeat.constant;
           // a_targ_size_t constant->variant.init_repeat.count;
           // a_byte_boolean constant->variant.init_repeat.multidimensional_aggr_tail_not_repeated;
           file << "\"];" << endl;
           break;
         }
       case ck_template_param:
         {
           // a_bit_field constant->variant.template_param.is_qualified_name:1;
           // a_bit_field constant->variant.template_param.is_pack:1;
           // a_bit_field constant->variant.template_param.has_generic_cast_for_nontype_template_param:1;
#if PROTOTYPE_INSTANTIATIONS_IN_IL
           // a_bit_field constant->variant.template_param.local_expr_ref:1;
#endif /* PROTOTYPE_INSTANTIATIONS_IN_IL */
           switch (constant->variant.template_param.kind) {
             case tpck_param:
               {
                 // a_template_param_coordinate constant->variant.template_param.variant.coordinates;
                 file << "\\ntpl param position: " << constant->variant.template_param.variant.coordinates.position;
                 file << "\\ntpl param depth: " << constant->variant.template_param.variant.coordinates.depth;
                 file << "\"];" << endl;
                 break;
               }
             case tpck_expression:
               {
                 file << "\"];" << endl;
                 an_expr_node_ptr expr = constant->variant.template_param.variant.expr;
                 if (expr != NULL) {
                   graph(expr);
                   file << "\"" << StringUtility::numberToString(constant) << "\" -> \"" << StringUtility::numberToString(expr) << "\"[label=\"" << "template_param.variant.expr" << "\"];" << endl;
                 }
                 break;
               }
             case tpck_member:
               {
                 file << "\\nMember of template parameter\"];" << endl;
                 break;
               }
             case tpck_unknown_function:
               {
                 file << "\"];" << endl;
                 // a_type_ptr constant->variant.template_param.variant.unknown_function.conversion_type;
#if MICROSOFT_EXTENSIONS_ALLOWED
                 // a_property_or_event_descr_ptr constant->variant.template_param.variant.unknown_function.property_or_event_descr;
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
                 // struct a_symbol * constant->variant.template_param.variant.unknown_function.symbol;
                 // an_opname_kind constant->variant.template_param.variant.unknown_function.opname_kind;
#if MICROSOFT_EXTENSIONS_ALLOWED
                 // a_special_function_kind constant->variant.template_param.variant.unknown_function.special_kind;
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
                 break;
               }
             case tpck_address:
               {
                 // a_constant_ptr constant->variant.template_param.variant.constant;
                 file << "\"];" << endl;
                 a_constant_ptr cst = constant->variant.template_param.variant.constant;
                 if (cst != NULL) {
                   graph(cst);
                   file << "\"" << StringUtility::numberToString(constant) << "\" -> \"" << StringUtility::numberToString(cst) << "\"[label=\"" << "template_param.variant.constant" << "\"];" << endl;
                 }
                 break;
               }
             case tpck_sizeof:
             case tpck_alignof:
             case tpck_uuidof:
             case tpck_typeid:
             case tpck_noexcept:
               {
                 // a_type_ptr constant->variant.template_param.variant.templ_sizeof.type;
                 // an_expr_node_ptr constant->variant.template_param.variant.templ_sizeof.expr;
                 // a_bit_field constant->variant.template_param.variant.templ_sizeof.is_std_alignof:1;
                 file << "\"];" << endl;
                 break;
               }
             case tpck_template_ref:
               {
                 file << "\"];" << endl;
                 a_constant_ptr tmpcst = constant->variant.template_param.variant.template_ref.con;
                 if (tmpcst != NULL) {
                   graph(tmpcst);
                   file << "\"" << StringUtility::numberToString(constant) << "\" -> \"" << StringUtility::numberToString(tmpcst) << "\"[label=\"" << "template_param.variant.template_ref.con" << "\"];" << endl;
                 }
                 a_template_arg_ptr arg_list = constant->variant.template_param.variant.template_ref.arg_list;
                 if (arg_list != NULL) {
                   graph(arg_list);
                   file << "\"" << StringUtility::numberToString(constant) << "\" -> \"" << StringUtility::numberToString(arg_list) << "\"[label=\"" << "template_param.variant.template_ref.arg_list" << "\"];" << endl;
                 }
                 break;
               }
             case tpck_destructor:
               {
                 // a_type_ptr constant->variant.template_param.variant.destructor.type;
                 // a_bit_field constant->variant.template_param.variant.destructor.unqualified:1;
                 file << "\"];" << endl;
                 break;
               }
             default:
               {
                 ROSE_ASSERT(false);
               }
           }
           break;
         }
       case ck_designator:
         {
           if (constant->variant.designator.is_field_designator) {
             if (constant->variant.designator.is_generic) {
               // a_const_char * constant->variant.designator.variant.field_name
             } else {
               // a_field_ptr constant->variant.designator.variant.field;
             }
           } else {
             if (constant->variant.designator.is_generic) {
               // a_constant_ptr constant->variant.designator.variant.subscript
             } else {
               // a_targ_size_t constant->variant.designator.variant.array_element;
             }
           }
           file << "\"];" << endl;
           break;
         }
#if UPC_EXTENSIONS_ALLOWED
       case ck_upc_threads:
         {
           // an_integer_value variant.integer_value;
           file << "\"];" << endl;
           break;
         }
       case ck_upc_mythread:
         {
           // EMPTY
           file << "\"];" << endl;
           break;
         }
#endif /* UPC_EXTENSIONS_ALLOWED */
       default:
         {
           ROSE_ASSERT(false);
         }
     }

  // DQ (8/20/2020): Adding support for attributes.
     an_attribute_ptr attributes = constant->source_corresp.attributes;
     if (attributes != NULL)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_constant_ptr): found attributes \n");
#endif
          graph(attributes);
          file << "\"" << StringUtility::numberToString(constant) << "\" -> \"" << StringUtility::numberToString(attributes) << "\"[label=\"" << "attributes" << "\"];" << endl;
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_constant_ptr): NO attributes \n");
#endif
        }
   }

void
EDG_ROSE_Graph::graph ( a_dynamic_init_ptr dynamic_initializer )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_dynamic_init_ptr): dynamic_initializer = %p kind = %s \n",dynamic_initializer,summarizeDynamicInitialization(dynamic_initializer->kind).c_str());
#endif

  // string s = (dynamic_initializer->is_compound_literal == true) ? "\\nIS compound_literal" : "";
     string s = string("\\ncompound_literal = ") + string((dynamic_initializer->is_compound_literal == true) ? "true" : "false");
  // file << "\"" << StringUtility::numberToString(dynamic_initializer) << "\"[" << "label=\"" << summarizeDynamicInitialization(dynamic_initializer->kind) << "\\n" << StringUtility::numberToString(dynamic_initializer) << "\"];" << endl;
     file << "\"" << StringUtility::numberToString(dynamic_initializer) << "\"[" << "label=\"" << summarizeDynamicInitialization(dynamic_initializer->kind) +  s << "\\n" << StringUtility::numberToString(dynamic_initializer) << "\"];" << endl;

     a_dynamic_init_kind kind = dynamic_initializer->kind;

#if 0
     printf ("In graph(a_dynamic_init_ptr): kind = %s \n",summarizeDynamicInitialization(kind).c_str());
#endif

#if 0
     printf ("In convert_dynamic_initializer(): Calling summarize_dynamic_init(dynamic_initializer = %p) \n",dynamic_initializer);
     summarize_dynamic_init(dynamic_initializer);
#endif

     switch (kind)
        {
          case dik_none:
             {
            // DQ (1/25/2019): This is not a case to support because it has not fields that are valid.
            // ROSE_ASSERT(!"case dik_none: not implemented");
            // graph(dynamic_initializer->variant.constant);
            // file << "\"" << StringUtility::numberToString(dynamic_initializer) << "\" -> \"" << StringUtility::numberToString(dynamic_initializer->variant.constant) << "\"[label=\"" << "none" << "\"];" << endl;
               break;
             }

          case dik_zero:
             {
            // TV (05/01/2018): that happens in Kripke. No associated field in the 'variant' union
            // ROSE_ASSERT(!"case dik_zero: not implemented");
               break;
             }

          case dik_constant:
             {
               graph(dynamic_initializer->variant.constant.ptr);
               file << "\"" << StringUtility::numberToString(dynamic_initializer) << "\" -> \"" << StringUtility::numberToString(dynamic_initializer->variant.constant.ptr) << "\"[label=\"" << "constant" << "\"];" << endl;
               break;
             }

          case dik_expression:
             {
               graph(dynamic_initializer->variant.expression);
               file << "\"" << StringUtility::numberToString(dynamic_initializer) << "\" -> \"" << StringUtility::numberToString(dynamic_initializer->variant.expression) << "\"[label=\"" << "expression" << "\"];" << endl;
               break;
             }

          case dik_class_result_via_ctor:
             {
               printf ("In graph(a_dynamic_init_ptr): Missing case dik_class_result_via_ctor\n");
               break;
             }

          case dik_constructor:
             {
            // DQ (10/20/2016): Fixed to account for when this is NULL internally within some templates (see test2016_88.C).
               if (dynamic_initializer->variant.constructor.ptr != NULL)
                  {
                 // DQ (12/27/2013): Added support for constructor calls.
                    graph(dynamic_initializer->variant.constructor.ptr);
                    file << "\"" << StringUtility::numberToString(dynamic_initializer) << "\" -> \"" << StringUtility::numberToString(dynamic_initializer->variant.constructor.ptr) << "\"[label=\"" << "constructor" << "\"];" << endl;
                  }
            // ROSE_ASSERT(!"case dik_constructor: not implemented");
               break;
             }

          case dik_nonconstant_aggregate:
             {
               graph(dynamic_initializer->variant.constant.ptr);
               file << "\"" << StringUtility::numberToString(dynamic_initializer) << "\" -> \"" << StringUtility::numberToString(dynamic_initializer->variant.constant.ptr) << "\"[label=\"" << "nonconstant aggregate" << "\"];" << endl;
               break;
             }

          case dik_bitwise_copy:
             {
               ROSE_ASSERT(!"case dik_bitwise_copy: not implemented");
               break;
             }

          default: 
             {
               ERROR_MESSAGE(stderr, "Bad dynamic initializer kind %d\n", kind); 
               ROSE_ASSERT(false);
               break;
             }
        }
   }



void
EDG_ROSE_Graph::graph ( a_param_type_ptr param_type )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
  // printf ("In graph(a_param_type_ptr): param_type = %p kind = %s \n",param_type,summarize_type_kind_in_string(param_type));
     printf ("In graph(a_param_type_ptr): param_type = %p \n",param_type);
#endif

#if 0
     printf ("In graph ( a_param_type_ptr ): calling summarize_param_type(): param_type = %p \n",param_type);
     summarize_param_type(param_type);
#endif
     string name;
     if (param_type->name != NULL)
        {
          name = param_type->name;
        }

  // Output nodes
     if (graphNodeSet.find(param_type) == graphNodeSet.end())
        {
          file << "\"" << StringUtility::numberToString(param_type) << "\"[" << "label=\"" << "parameter type: \\n" << name << "\\n" << StringUtility::numberToString(param_type) << "\"];" << endl;
          insertGraphNode(param_type);
        }
       else
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_param_type_ptr): node was already processed (return): type = %p \n",param_type);
#endif
          return;
        }

#if 1
  // DQ (9/21/2015): Output more information to debug handling of C++11 functions and their types.
  // Process the internal types: output the type and declared type
     ROSE_ASSERT(param_type->type != NULL);
     graph(param_type->type);
  // file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(base_type) << "\"[label=\"" << "base type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
     file << "\"" << StringUtility::numberToString(param_type) << "\" -> \"" << StringUtility::numberToString(param_type->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

  // DQ (7/7/2017): This has to be allowed to be NULL.
  // ROSE_ASSERT(param_type->declared_type != NULL);

     if (param_type->declared_type != NULL)
        {
          graph(param_type->declared_type);
          file << "\"" << StringUtility::numberToString(param_type) << "\" -> \"" << StringUtility::numberToString(param_type->declared_type) << "\"[label=\"" << "declared_type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
        }
#endif
   }



void
EDG_ROSE_Graph::graph ( a_type_ptr type )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_type_ptr): type = %p kind = %s \n",type,summarize_type_kind_in_string(type));
#endif

  // Output nodes
     if (graphNodeSet.find(type) == graphNodeSet.end())
        {
          string assoc_type_name;
          if (type->source_corresp.name != NULL)
             {
               assoc_type_name = type->source_corresp.name;
             }
            else
             {
            // DQ (3/2/2018): This can be the cae of a char, int, long, etc. and it would be helpful to name these more precisely.
               assoc_type_name = "un-named";

               if (type->kind == tk_integer)
                  {
                    assoc_type_name += string(": ") + summarize_integer_string(type->variant.integer.int_kind);
                  }

            // DQ (3/3/2018): Adding support for different kinds of floating point types.
               if (type->kind == tk_float)
                  {
                    assoc_type_name += string(": floating point: ");
                    a_float_kind kind = type->variant.float_kind;
                    switch (kind)
                       {
                         case fk_float:        assoc_type_name += "float";       break;
                         case fk_double:       assoc_type_name += "double";      break;
                         case fk_long_double:  assoc_type_name += "long double"; break;
                         case fk_float80:      assoc_type_name += "float80";     break;
                         case fk_float128:     assoc_type_name += "float128";    break;
                         case fk_std_bfloat16: assoc_type_name += "bfloat16";    break;
                         case fk_std_float16:  assoc_type_name += "float16";     break;
                         case fk_std_float32:  assoc_type_name += "float32";     break;
                         case fk_std_float64:  assoc_type_name += "float64";     break;
                         default:
                            {
                              printf ("Error: default reached for type->kind == tk_float \n");
                              ROSE_ASSERT(false);
                            }
                       }
                  }

            // DQ (3/3/2018): Adding support for different kinds of type modifiers.
               if (type->kind == tk_typeref)
                  {
                 // assoc_type_name += string(": ") + "what kind of modifier?";
                    assoc_type_name += string(": ") + "modifier: ";

#if 0
     if (qualifiers & TQ_CONST)    result = buildConstType(result);
     if (qualifiers & TQ_VOLATILE) result = buildVolatileType(result);
     if (qualifiers & TQ_RESTRICT) result = buildRestrictType(result);
#if UPC_EXTENSIONS_ALLOWED
     if (qualifiers & TQ_UPC_SHARED)
        {
          if (block_size == UPC_BLOCK_SIZE_NONE || block_size == 1)
          { // default block size is 1, Liao 11/29/2012
               result = buildUpcSharedType(result,1);
          } else if (block_size == UPC_BLOCK_SIZE_INDEFINITE /* which is 0 */) {
               result = buildUpcBlockIndefiniteType(result);
          } else if (block_size == UPC_BLOCK_SIZE_BLOCK) {
               result = buildUpcBlockStarType(result);
          } else {
               result = buildUpcBlockNumberType(result, block_size);
          }
        }
     if (qualifiers & TQ_UPC_STRICT) result = buildUpcStrictType(result);
     if (qualifiers & TQ_UPC_RELAXED) result = buildUpcRelaxedType(result);
#endif
#endif
                    a_bit_field qualifiers = type->variant.typeref.qualifiers;

                    if (qualifiers & TQ_CONST)    assoc_type_name += "const";
                    if (qualifiers & TQ_VOLATILE) assoc_type_name += "volatile";
                    if (qualifiers & TQ_RESTRICT) assoc_type_name += "restrict";
                  }
             }
       // assoc_type_name += " : type : " + string(summarize_type_kind_in_string(type));
          assoc_type_name = "type : " + string(summarize_type_kind_in_string(type)) + " : " + assoc_type_name;

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_type_ptr): type = %p assoc_type_name = %s \n",type,assoc_type_name.c_str());
#endif

       // DQ (7/14/2017): Tracking use of non-real classes in EDG.
          if (type->kind == tk_class && type->variant.class_struct_union.is_nonreal_class == true)
             {
               assoc_type_name += "\\n is_nonreal_class ";
             }

          if (type->kind == tk_class && type->variant.class_struct_union.originally_unnamed == true) {
            assoc_type_name += "\\n originally_unnamed ";
          }

       // file << "\"" << StringUtility::numberToString(type) << "\"[" << "label=\"" << string(summarize_type_kind_in_string(type)) << "\\n" << StringUtility::numberToString(type) << "\"];" << endl;
          file << "\"" << StringUtility::numberToString(type) << "\"[" << "label=\"" << assoc_type_name << "\\n" << StringUtility::numberToString(type) << "\"];" << endl;
          insertGraphNode(type);
        }
       else
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_type_ptr): node was already processed (return): type = %p \n",type);
#endif
          return;
        }

  // TV (03/29/2018): Traverse the symbols
     a_symbol_ptr assoc_info = (a_symbol_ptr)type->source_corresp.assoc_info;
     if (assoc_info != NULL) {
        file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(assoc_info)
             << "\"[label=\"" << "symbol" << "\" color=\"purple\" weight=1];" << endl;
        graph(assoc_info);
     }

  // TV (05/21/2018): Traverse base classes
     if (type->kind == tk_class || type->kind == tk_struct || type->kind == tk_union) {
          a_class_type_supplement_ptr ctsp = type->variant.class_struct_union.extra_info;
          ROSE_ASSERT(ctsp != NULL);

          a_base_class_ptr bcp = ctsp->base_classes;
          unsigned int cnt = 0;
          while (bcp != NULL) {
            graph(bcp);
            file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(bcp) << "\"[label=\"" << "base_class[" << cnt << "]" << "\" weight=1];" << endl;

            bcp = bcp->next;
            cnt++;
          }
        }

     if (type->kind == tk_integer) 
        {
#if 0
          printf ("Found type->kind == tk_integer: type = %p \n",type);
          printf ("type->variant.integer.enum_type      = %s \n",type->variant.integer.enum_type ? "true" : "false");
          printf ("type->variant.integer.is_scoped_enum = %s \n",type->variant.integer.is_scoped_enum ? "true" : "false");
#endif
          if (type->variant.integer.enum_type && type->variant.integer.is_scoped_enum)
             {
               a_scope_ptr scope = type->variant.integer.enum_info.assoc_scope;
#if 0
               printf ("Found C++11 scoped enum: scope = %p \n",scope);
#endif
               file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(scope) << "\"[label=\"enum scope\"];" << endl;
               graph(scope,"scoped enum scope");
             }
            else 
             {
#if 0
               printf ("Found NON C++11 scoped enum: type = %p \n",type);
               printf ("type->variant.integer.enum_type      = %s \n",type->variant.integer.enum_type ? "true" : "false");
               printf ("type->variant.integer.is_scoped_enum = %s \n",type->variant.integer.is_scoped_enum ? "true" : "false");
#endif
               if (type->variant.integer.enum_type && !type->variant.integer.is_scoped_enum) 
                  {
#if 0
                    printf ("Found type->variant.integer.enum_type && !type->variant.integer.is_scoped_enum: type = %p \n",type);
#endif
                    a_constant_ptr cst = type->variant.integer.enum_info.constant_list;
                    size_t cnt = 0;
                    while (cst != NULL)
                       {
                         file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(cst) << "\"[label=\"enumerator[" << cnt++ << "]\"];" << endl;
                         graph(cst);
                         cst = cst->next;
                       }
                  }
             }
        }

     switch (type->kind)
        {
          case tk_void:
          case tk_integer:
          case tk_float:
          case tk_imaginary:
          case tk_complex:
             {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
               printf ("Found type kind for which there is nothing to do! \n");
#endif
            // Nothing to do for these cases.
               break;
             }

          case tk_pointer:
             {
               a_type_ptr base_type = type->variant.pointer.type;
               ROSE_ASSERT(base_type != NULL);

            // DQ (9/21/2015): Added base type and size expression.
               graph(base_type);
               file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(base_type) << "\"[label=\"" << "base type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

               break;
             }

          case tk_routine:
             {
#if 1
            // a_type_ptr function_type = type;
               a_type_ptr return_type   = type->variant.routine.return_type;

            // graph(function_type);
            // file << "\"" << StringUtility::numberToString(function_type) << "\" -> \"" << StringUtility::numberToString(function_type) << "\"[label=\"" << "function_type" << "\"];" << endl;

               graph(return_type);
            // file << "\"" << StringUtility::numberToString(variable) << "\" -> \"" << StringUtility::numberToString(variable->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" decorate labelfontcolor=\"goldenrod\" weight=1];" << endl;
               file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(return_type) << "\"[label=\"" << "return type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
#endif

#if 1
            // DQ (9/25/2015): We can't assume this is non-NULL (see test2015_83.C).
            // DQ (9/21/2015): Added support for parameter types.
            // ROSE_ASSERT(type->variant.routine.extra_info->param_type_list != NULL);
               if (type->variant.routine.extra_info->param_type_list != NULL)
                  {
                    a_param_type_ptr temp_param = type->variant.routine.extra_info->param_type_list;
                    int counter = 0;
                    while (temp_param != NULL)
                       {
                         graph(temp_param);
                         file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(temp_param) << "\"[label=\"" << "param:" << StringUtility::numberToString(counter) << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

                         temp_param = temp_param->next;
                         counter++;
                       }
                  }
#endif

            // DQ (2/10/2018): Adding support to find VLA information.
               if (type->variant.routine.extra_info->prototype_scope != NULL)
                  {
                    a_scope_ptr prototype_scope = type->variant.routine.extra_info->prototype_scope;

                    graph(prototype_scope,"prototype scope");
                    file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(prototype_scope) << "\"[label=\"" << "prototype scope" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

                  }

               break;
             }

          case tk_array:
             {
               a_type_ptr base_type = type->variant.array.element_type;
               ROSE_ASSERT(base_type != NULL);

            // DQ (9/21/2015): Added base type and size expression.
               graph(base_type);
               file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(base_type) << "\"[label=\"" << "base type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

#if 0
               printf ("type->variant.array.is_template_dependent_size_array = %s \n",type->variant.array.is_template_dependent_size_array ? "true" : "false");
               printf ("type->variant.array.is_variable_size_array           = %s \n",type->variant.array.is_variable_size_array ? "true" : "false");
               printf ("type->variant.array.bound_is_zero                    = %s \n",type->variant.array.bound_is_zero ? "true" : "false");
               printf ("type->variant.array.variant.element_count_expr       = %p \n",type->variant.array.variant.element_count_expr);

               printf ("type->variant.array.variant.number_of_elements       = %zu \n",(size_t)type->variant.array.variant.number_of_elements);
               printf ("type->variant.array.bound_constant                   = %p \n",type->variant.array.bound_constant);
#endif
            // size_t array_size = 0;
               if (type->variant.array.is_variable_size_array == false && type->variant.array.is_template_dependent_size_array == false)
                  {
#if 0
                    printf ("          array: number_of_elements                            = %" PRIuPTR " \n",(size_t)type->variant.array.variant.number_of_elements);
#endif
                 // array_size = (size_t)type->variant.array.variant.number_of_elements;

                    if (type->variant.array.variant.number_of_elements == 0 && !type->variant.array.bound_is_zero)
                       {
#if 0
                         printf ("unspecified array bound \n");
#endif
                       }
                      else
                       {
#if 0
                         printf ("type->variant.array.variant.number_of_elements = %d \n",(int)type->variant.array.variant.number_of_elements);
                         printf ("type->variant.array.bound_is_zero = %s \n",type->variant.array.bound_is_zero ? "true" : "false");
#endif
                         if (type->variant.array.bound_constant != NULL)
                            {
                           // ROSE_ASSERT(type->variant.array.bound_constant != NULL);
                              graph(type->variant.array.bound_constant);
                              file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(type->variant.array.bound_constant) << "\"[label=\"" << "size" << "\"];" << endl;
                            }
                           else
                            {
#if 0
                              printf ("Not clear what kind of array bound this is \n");
#endif
                            }
                       }
                  }
                 else
                  {
                 // if (type->variant.array.is_variable_size_array == true)
                    if (type->variant.array.is_variable_size_array == true && type->variant.array.variant.element_count_expr != NULL)
                       {
#if 0
                         printf ("          array: element_count_expr                        = %p \n",type->variant.array.variant.element_count_expr);
#endif
                      // DQ (2/10/2016): Added assertion.
                         ROSE_ASSERT(type->variant.array.variant.element_count_expr != NULL);

                         graph(type->variant.array.variant.element_count_expr);
                         file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(type->variant.array.variant.element_count_expr) << "\"[label=\"" << "size" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
                       }

                    if (type->variant.array.is_template_dependent_size_array == true)
                       {
#if 0
                         printf ("          array: element_count_constant                    = %p \n",type->variant.array.variant.element_count_constant);
#endif
                      // DQ (2/10/2016): Added assertion.
                         ROSE_ASSERT(type->variant.array.variant.element_count_constant != NULL);

                         graph(type->variant.array.variant.element_count_constant);
                         file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(type->variant.array.variant.element_count_constant) << "\"[label=\"" << "size" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
                       }
                  }

               break;
             }

          case tk_class:
          case tk_struct:
          case tk_union:
             {
            // loop over operands.
               int counter = 0;
               for (a_field_ptr field = type->variant.class_struct_union.field_list; field != NULL; field = field->next)
                  {
                    graph(field);

                 // Generate an edge from type to the field.
                    file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(field) << "\"[label=\"" << "field : " + StringUtility::numberToString(counter) << "\"];" << endl;

                    counter++;
                  }

            // DQ (10/11/2015): Need to add more detail here.

               string assoc_type_name;
               if (type->source_corresp.name != NULL)
                  {
                    assoc_type_name = type->source_corresp.name;
                  }
                 else
                  {
                    assoc_type_name = "un-named";
                  }

#if 0
               printf ("assoc_type_name = %s \n",assoc_type_name.c_str());
#endif
            // This is an example of something that could be extracted to a function that added graph edged for any a_source_correspondence struct.
               a_scope_ptr parent_scope = type->source_corresp.parent_scope;
            // ROSE_ASSERT(parent_scope != NULL);
               if (parent_scope != NULL)
                  {
                    graph(parent_scope,"parent scope");
                    file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(parent_scope) 
                         << "\"[label=\"" << "parent_scope" << "\" color=\"lightgreen\" weight=1];" << endl;
                  }
                 else
                  {
                    printf ("Note: In graph(a_type_ptr type): case tk_union: parent_scope == NULL \n");
                  }

               a_class_type_supplement_ptr extra_info = type->variant.class_struct_union.extra_info;
               if (extra_info != NULL)
                  {
                 // anonymous_union_field
                    if (extra_info->anonymous_union_field != NULL)
                       {
                         graph(extra_info->anonymous_union_field);
                         file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(extra_info->anonymous_union_field) 
                              << "\"[label=\"" << "anonymous_union_field \"];" << endl;
                       }

                 // assoc_scope
                 // ROSE_ASSERT(extra_info->assoc_scope != NULL);
                    if (extra_info->assoc_scope != NULL)
                       {
                         string assoc_scope_name;
                         if (type->source_corresp.name != NULL)
                            {
                              assoc_scope_name = type->source_corresp.name;
                            }
                           else
                            {
                              assoc_scope_name = "un-named";
                            }
                         assoc_scope_name += " : assoc scope";
                         graph(extra_info->assoc_scope,assoc_scope_name);
                         file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(extra_info->assoc_scope) 
                              << "\"[label=\"" << "assoc_scope" << "\" color=\"lightgreen\" weight=1];" << endl;
                       }

                    if (extra_info->assoc_template != NULL)
                       {
#if 0
                         string assoc_template_name;
                         if (type->source_corresp.name != NULL)
                            {
                              assoc_template_name = type->source_corresp.name;
                            }
                           else
                            {
                              assoc_template_name = "un-named";
                            }
                         assoc_template_name += " : assoc template";
#endif
                         graph(extra_info->assoc_template);
                         file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(extra_info->assoc_template) 
                              << "\"[label=\"" << "assoc_template" << "\" color=\"yellow\" weight=1];" << endl;
                       }
#if 0
                    printf ("In graph(a_type_ptr): Process the template argument/parameter list \n");
#endif
                    if (extra_info->template_arg_list != NULL)
                       {
                         a_template_arg_ptr template_argument = extra_info->template_arg_list;
                         while (template_argument != NULL)
                            {
#if 0
                              printf ("In graph(a_type_ptr): Process the template argument = %p \n",template_argument);
#endif
                              graph(template_argument);
#if 0
                              printf ("In graph(a_type_ptr): Process the template argument = %p \n",template_argument);
#endif
                              file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(template_argument) 
                                   << "\"[label=\"" << "tpl_arg" << "\" color=\"yellow\" weight=1];" << endl;

                              template_argument = template_argument->next;
                            }
                       }
                  }

               break;
             }

          case tk_typeref:
             {
               a_type_ptr base_type = type->variant.typeref.type;
               ROSE_ASSERT(base_type != NULL);

               graph(base_type);
               file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(base_type) << "\"[label=\"" << "base type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

#if 0
               printf ("Calling summarize_type() \n");
               summarize_type(type);
#endif

            // DQ (6/18/2016): Added support for template arguments.
               a_typeref_type_supplement_ptr extra_info = type->variant.typeref.extra_info;
               if (extra_info != NULL)
                  {
                    if (extra_info->template_arg_list != NULL)
                       {
                         a_template_arg_ptr template_argument = extra_info->template_arg_list;
                         while (template_argument != NULL)
                            {
                              graph(template_argument);
                              file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(template_argument) 
                                   << "\"[label=\"" << "arg" << "\" color=\"yellow\" weight=1];" << endl;

                              template_argument = template_argument->next;
                            }
                       }
                  }

               if (type->variant.typeref.extra_info->expr) {
                 an_expr_node_ptr expr = type->variant.typeref.extra_info->expr;
                 graph(expr);
                 file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(expr) << "\"[label=\"";
                 // TODO EDG 6.5 defines that in il_def.h at line 9204
                 file << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
               }

#if DO_IL_LOWERING
            // This is un-used code by default.
               a_type_ptr orig_type = type->variant.typeref.orig_type;
               if (orig_type != NULL)
                  {
#if 1
                    printf ("Found original type in typedef: orig_type = %p \n",orig_type);
#endif
                    graph(orig_type);
                    file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(orig_type) 
                         << "\"[label=\"" << "arg" << "\" color=\"brown\" weight=1];" << endl;
                  }
#endif

               break;
             }

          case tk_ptr_to_member: 
             {
            // DQ (7/15/2017): Added support for pointer to member types.
            // printf ("Unimplemented case: case tk_ptr_to_member \n");
               a_type_ptr class_of_which_a_member = type->variant.ptr_to_member.class_of_which_a_member;
               ROSE_ASSERT(class_of_which_a_member != NULL);

               graph(class_of_which_a_member);
               file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(class_of_which_a_member) 
                    << "\"[label=\"" << "class_of_which_a_member" << "\" color=\"brown\" weight=1];" << endl;

               a_type_ptr class_member_type = type->variant.ptr_to_member.type;
               ROSE_ASSERT(class_member_type != NULL);

               graph(class_member_type);
               file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(class_member_type) 
                    << "\"[label=\"" << "class_member_type" << "\" color=\"brown\" weight=1];" << endl;

               break;
             }

          case tk_template_param:
             {
            // printf ("Unimplemented case: case tk_template_param \n");
               printf ("In graph(a_type_ptr): type = %p case tk_template_param \n",type);

               a_scope_ptr parent_scope = type->source_corresp.parent_scope;
               if (parent_scope != NULL) {
                 graph(parent_scope,"parent scope");
                 file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(parent_scope) << "\"[label=\"" << "parent_scope" << "\" color=\"lightgreen\" weight=1];" << endl;
               }

            // Output the a_template_param_type_supplement_ptr extra_info;
               a_template_param_type_supplement_ptr extra_info = type->variant.template_param.extra_info;

               printf ("In graph(a_type_ptr): type = %p case tk_template_param: extra_info = %p \n",type,extra_info);

               if (extra_info != NULL)
                  {
                    graph(extra_info);
                    file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(extra_info) << "\"[label=\"" << "extra info" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
                  }

               break;
             }
          

       // DQ (12/29/2008): Added cases not handled (these are OK to default to error as in default clause, I think).
          case tk_error:
#if FIXED_POINT_ALLOWED
          case tk_fixed_point:
#endif
          case tk_unknown:
             {
               printf ("Case not handled! kind = %s \n",output_type_enum(type->kind).c_str());
               ROSE_ASSERT(false);
               break;
             }

          case tk_nullptr:
             {
#if 0
               file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(base_type) << "\"[label=\"" << "nullptr type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
#else
               printf ("Case tk_nullptr not handled! kind = %s \n",output_type_enum(type->kind).c_str());
#endif
#if 0
               printf ("Case tk_nullptr not handled! kind = %s \n",output_type_enum(type->kind).c_str());
               ROSE_ASSERT(false);
#endif
               break;
             }

#if GNU_EXTENSIONS_ALLOWED && GNU_VECTOR_TYPES_ALLOWED
       // DQ (11/13/2017): Need to disable tk_vector from the Windows mode.

       // DQ (7/7/2017): Adding support for vector types.
          case tk_vector:
             {
               a_type_ptr base_type = NULL; // type->variant.pointer.type;
            // ROSE_ASSERT(base_type != NULL);

            // DQ (9/21/2015): Added base type and size expression.
            // graph(base_type);
               file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(base_type) << "\"[label=\"" << "base type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

               break;
             }
#endif

          default:
             {
            // printf ("Error: unhandled type kind = %d\n", type->kind);
               printf ("Error: unhandled type type = %p kind = %s \n",type,summarize_type_kind_in_string(type));
               ROSE_ASSERT(false);
             }
        }

  // DQ (8/20/2020): Adding support for attributes.
     an_attribute_ptr attributes = type->source_corresp.attributes;
     if (attributes != NULL)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_type_ptr): found attributes \n");
#endif
          graph(attributes);
          file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(attributes) << "\"[label=\"" << "attributes" << "\"];" << endl;
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_type_ptr): NO attributes \n");
#endif
        }
   }


void
EDG_ROSE_Graph::graph ( a_field_ptr field )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_field_ptr): field = %p \n",field);
#endif

  // DQ (10/11/2015): Added assertion.
     ROSE_ASSERT(field != NULL);

     file << "\"" << StringUtility::numberToString(field) << "\"[" << "label=\"" << "field" << "\\n" << StringUtility::numberToString(field);
     if (field->has_initializer) {
       file << "has_initializer\\n";
     }
     file << "\"];" << endl;

     if (field->type) {
       graph(field->type);
       file << "\"" << StringUtility::numberToString(field) << "\" -> \"" << StringUtility::numberToString(field->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
     }

     if (field->initializer) {
       graph(field->initializer);
       file << "\"" << StringUtility::numberToString(field) << "\" -> \"" << StringUtility::numberToString(field->initializer) << "\"[label=\"" << "initializer" << "\" weight=1];" << endl;
     }

  // DQ (8/20/2020): Adding support for attributes.
     an_attribute_ptr attributes = field->source_corresp.attributes;
     if (attributes != NULL)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_field_ptr): found attributes \n");
#endif
          graph(attributes);
          file << "\"" << StringUtility::numberToString(field) << "\" -> \"" << StringUtility::numberToString(attributes) << "\"[label=\"" << "attributes" << "\"];" << endl;
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_field_ptr): NO attributes \n");
#endif
        }
   }


void
EDG_ROSE_Graph::graph ( a_variable_template_info_ptr vtpl_info )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_variable_template_info_ptr): vtpl_info = %p \n", vtpl_info);
#endif
     if (graphNodeSet.find(vtpl_info) != graphNodeSet.end()) return;

     file << "\"" << StringUtility::numberToString(vtpl_info) << "\"[" << "label=\"" << "vtpl_info" << "\\n" << StringUtility::numberToString(vtpl_info) << "\"];" << endl;

     if (vtpl_info->assoc_template) {
       graph(vtpl_info->assoc_template);
       file << "\"" << StringUtility::numberToString(vtpl_info) << "\" -> \"" << StringUtility::numberToString(vtpl_info->assoc_template) << "\"[label=\"" << "assoc_template" << "\" weight=1];" << endl;
     }

     a_template_arg_ptr tplarg = vtpl_info->template_arg_list;
     int cnt = 0;
     while (tplarg) {
       graph(tplarg);
       file << "\"" << StringUtility::numberToString(vtpl_info) << "\" -> \"" << StringUtility::numberToString(tplarg) << "\"[label=\"" << "template_arg_list[" << cnt++ << "]" << "\" weight=1];" << endl;
       tplarg = tplarg->next;
     }
     tplarg = vtpl_info->partial_spec_template_arg_list;
     cnt = 0;
     while (tplarg) {
       graph(tplarg);
       file << "\"" << StringUtility::numberToString(vtpl_info) << "\" -> \"" << StringUtility::numberToString(tplarg) << "\"[label=\"" << "partial_spec_template_arg_list[" << cnt++ << "]" << "\" weight=1];" << endl;
       tplarg = tplarg->next;
     }
   }

void
EDG_ROSE_Graph::graph ( a_variable_ptr variable )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT || 0
     printf ("In graph(a_variable_ptr): variable = %p \n",variable);
#endif
     ROSE_ASSERT(variable != NULL);

#if 0
  // Debug by puting out the elements in the set.
     printGraphNodeSet();
#endif

  // DQ (9/8/2013): This is the variable I was looking for to support GNU statement expressions.
     if (variable->embedded_source_sequence_entries == true)
        {
          printf ("variable->embedded_source_sequence_entries == true: variable = %p \n",variable);
        }
  // ROSE_ASSERT(variable->embedded_source_sequence_entries == false);

  // Output nodes
     if (graphNodeSet.find(variable) == graphNodeSet.end())
        {
          string variableName = string("variable : ");
          if (variable->source_corresp.name != NULL)
             {
               variableName += variable->source_corresp.name;
             }

          file << "\"" << StringUtility::numberToString(variable) << "\"[" << "label=\"" << variableName << "\\n" 
               << StringUtility::numberToString(variable) + " : " << "\" color=\"blue\" ];" << endl;

          insertGraphNode(variable);

       // DQ (11/23/2016): Added assertion.
          ROSE_ASSERT(variable->type != NULL);

          graph(variable->type);

       // DQ (3/19/2014): removed "decorate" keyword since it is an error for latest version of zgrviewer (or maybe dot).
       // file << "\"" << StringUtility::numberToString(variable) << "\" -> \"" << StringUtility::numberToString(variable->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" decorate labelfontcolor=\"goldenrod\" weight=1];" << endl;
          file << "\"" << StringUtility::numberToString(variable) << "\" -> \"" << StringUtility::numberToString(variable->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

#if GENERATE_SOURCE_SEQUENCE_LISTS
       // DQ (2/14/2018): Add information specific to the declared type (since this is what the source-to-source support needs.
       // ROSE_ASSERT(variable->declared_type != NULL);
          if (variable->declared_type != NULL)
             {
               graph(variable->declared_type);
               file << "\"" << StringUtility::numberToString(variable) << "\" -> \"" << StringUtility::numberToString(variable->declared_type) << "\"[label=\"" << "declared_type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
             }
#endif

#if 0
          printf ("In graph(a_variable_ptr): variable (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",variable,graphNodeSet.size());
#endif
        }
       else
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT || 0
          printf ("In graph(a_variable_ptr): node was already processed (return): variable = %p \n",variable);
#endif
          return;
        }

     // Output a_template_symbol_supplement
     if (variable->template_info != NULL)
        {
          graph(variable->template_info);
#if 1
          file << "\"" << StringUtility::numberToString(variable) << "\" -> \"" << StringUtility::numberToString(variable->template_info) << "\"[label=\"" << "template_info" << "\"];" << endl;
#endif
        }

  // Draw an edge to the variable's parent scope (more useful for C++ than C).
     a_scope_ptr variable_scope = variable->source_corresp.parent_scope;
     if (variable_scope != NULL)
        {
          graph(variable_scope,"variable scope");
          file << "\"" << StringUtility::numberToString(variable) << "\" -> \"" << StringUtility::numberToString(variable_scope) 
               << "\"[label=\"" << "variable scope" << "\" color=\"lightgreen\" weight=1];" << endl;
        }

     an_init_kind initializer_kind = variable->init_kind;
     switch(initializer_kind)
        {
       // Nothing to do for initk_none, initk_zero, or initk_function_local:
          case initk_none:
          case initk_zero:
          case initk_function_local:
             {
               break;
             }

          case initk_static:
             {
               graph(variable->initializer.constant);
               file << "\"" << StringUtility::numberToString(variable) << "\" -> \"" << StringUtility::numberToString(variable->initializer.constant) 
                    << "\"[label=\"" << "constant initializer" << "\"];" << endl;
               break;
             }

          case initk_dynamic:
             {
               graph(variable->initializer.dynamic);

               file << "\"" << StringUtility::numberToString(variable) << "\" -> \"" << StringUtility::numberToString(variable->initializer.dynamic) 
                    << "\"[label=\"" << "dynamic initializer" << "\"];" << endl;
               break;
             }

          default:
             {
               printf ("Error: case not handled \n");
               ROSE_ASSERT(false);
             }
        }

  // string variableName = variable->source_corresp.name;
  // file << "\"" << StringUtility::numberToString(variable) << "\"[" << "label=\"" << variableName << "\\n" << StringUtility::numberToString(variable) << "\"];" << endl;

  // DQ (8/20/2020): Adding support for attributes.
     an_attribute_ptr attributes = variable->source_corresp.attributes;
     if (attributes != NULL)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_variable_ptr): found attributes \n");
#endif
          graph(attributes);
          file << "\"" << StringUtility::numberToString(variable) << "\" -> \"" << StringUtility::numberToString(attributes) << "\"[label=\"" << "attributes" << "\"];" << endl;
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_variable_ptr): NO attributes \n");
#endif
        }
   }

void
EDG_ROSE_Graph::graph ( a_routine_ptr routine )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_routine_ptr): routine = %p \n",routine);
#endif

     ROSE_ASSERT(routine != NULL);

  // Output nodes
     if (graphNodeSet.find(routine) == graphNodeSet.end())
        {
          ROSE_ASSERT(routine->source_corresp.name != NULL);
          string functionName = string("routine : ") + routine->source_corresp.name;
          file << "\"" << StringUtility::numberToString(routine) << "\"[" << "label=\"" << functionName << "\\n" 
               << StringUtility::numberToString(routine) << "\" color=\"blue\" ];" << endl;

          insertGraphNode(routine);
#if 0
          printf ("In graph(a_routine_ptr): routine (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",routine,graphNodeSet.size());
#endif
       }
      else
       {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
         printf ("In graph(a_routine_ptr): routine = %p returning without processing this not (already processed) \n",routine);
#endif
         return;
       }

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_routine_ptr): routine = %p output information about types \n",routine);
#endif

#if 1
  // DQ (9/21/2015): Output more information to debug handling of C++11 functions and their types.
  // Output the type and declarted type
     ROSE_ASSERT(routine->type != NULL);
     graph(routine->type);
  // file << "\"" << StringUtility::numberToString(type) << "\" -> \"" << StringUtility::numberToString(base_type) << "\"[label=\"" << "base type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
     file << "\"" << StringUtility::numberToString(routine) << "\" -> \"" << StringUtility::numberToString(routine->type) << "\"[label=\"" << "type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;

  // DQ (9/25/2015): We can't assume this is non-NULL (see test2015_83.C).
  // ROSE_ASSERT(routine->declared_type != NULL);
     if (routine->declared_type != NULL)
        {
          graph(routine->declared_type);
          file << "\"" << StringUtility::numberToString(routine) << "\" -> \"" << StringUtility::numberToString(routine->declared_type) << "\"[label=\"" << "declared_type" << "\" color=\"goldenrod\" labelfontcolor=\"goldenrod\" weight=1];" << endl;
        }
#endif

  // ROSE_ASSERT(routine->routine.extra_info.param_type_list != NULL);

  // DQ (2/10/2018): Turn this on as part of searching for VLA information.
     if (routine->assoc_template != NULL)
        {
          graph(routine->assoc_template);
          file << "\"" << StringUtility::numberToString(routine) << "\" -> \"" << StringUtility::numberToString(routine->assoc_template) << "\"[label=\"" << "assoc_template" << "\" weight=1];" << endl;
        }


     if (routine->source_corresp.parent_scope != NULL) {
       graph(routine->source_corresp.parent_scope,"routine scope");
       file << "\"" << StringUtility::numberToString(routine) << "\" -> \"" << StringUtility::numberToString(routine->source_corresp.parent_scope) << "\"[label=\"" << "parent_scope" << "\" weight=1];" << endl;
     }

  // DQ (10/20/2016): When this is from a template, it will not be in the source sequence list, yet we want to output the graph.
  // DQ (7/6/2017): Note that the zero entry is not used.
  // ROSE_ASSERT(routine->function_def_number > 0);
  // DQ (10/20/2016): This variable appears to have been moded to specific variants as part of changes to the  4.11 IR.
  // a_scope_ptr            function_scope = il_header.function_def_table[routine->function_def_number].scope;
     a_scope_ptr            function_scope = NULL;
  // printf ("In graph(a_routine_ptr): routine = %p routine->function_def_number = %d \n",routine,routine->function_def_number);
     if (routine->function_def_number > 0)
        {
          function_scope = il_header.function_def_table[routine->function_def_number].scope;
        }

#if 0
     printf ("In graph(a_routine_ptr): case iek_routine: Calling graph(a_scope_ptr) on function_scope = %p \n",function_scope);
#endif

#if 1
  // DQ (2/10/2018): This appears to be redundant as an edge.
     string function_name = routine->source_corresp.name;
     function_name += " : function scope";
     if (function_scope != NULL)
        {
          graph(function_scope, function_name);
          file << "\"" << StringUtility::numberToString(routine) << "\" -> \"" << StringUtility::numberToString(function_scope) << "\"[label=\"" << "function scope B" << "\"];" << endl;
        }
#endif

  // DQ (8/20/2020): Adding support for attributes.
     an_attribute_ptr attributes = routine->source_corresp.attributes;
     if (attributes != NULL)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_routine_ptr): found attributes \n");
#endif
          graph(attributes);
          file << "\"" << StringUtility::numberToString(routine) << "\" -> \"" << StringUtility::numberToString(attributes) << "\"[label=\"" << "attributes" << "\"];" << endl;
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_routine_ptr): NO attributes \n");
#endif
        }
   }


void
EDG_ROSE_Graph::graph ( a_template_symbol_supplement_ptr template_sup )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_template_symbol_supplement_ptr): template_sup = %p \n",template_sup);
#endif

  // Output nodes
     if (graphNodeSet.find(template_sup) == graphNodeSet.end())
        {
          file << "\"" << StringUtility::numberToString(template_sup) << "\"[" << "label=\"" << "template_symbol_supplement" << "\\n" 
               << StringUtility::numberToString(template_sup) << "\" color=\"blue\" ];" << endl;

          insertGraphNode(template_sup);
#if 0
          printf ("In graph(a_template_symbol_supplement_ptr): template_node (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",template_sup,graphNodeSet.size());
#endif
        }
       else
        {
          return;
        }

  // output befriending classes
     a_class_list_entry_ptr befriending_class = template_sup->befriending_classes;
     if (befriending_class != NULL)
        {
#if 1
      // DQ (9/27/2015): This is a problem for the GNU g++ version 4.2.4 compiler.
          while (befriending_class != NULL)
             {
#if 0
               printf ("befriending_class = %p \n",befriending_class);
#endif
               graph(befriending_class->class_type);

            // DQ (9/27/2015): This is a problem for the GNU g++ version 4.2.4 compiler
            // (the "\]" substring is reported as an error: undefined escape sequence).
            // file << "\"" << StringUtility::numberToString(template_sup) << "\" -> \"" << StringUtility::numberToString(befriending_class->class_type) << "\"[label=\"" << "befriending_class" << "\];" << endl;
               file << "\"" << StringUtility::numberToString(template_sup) << "\" -> \"" << StringUtility::numberToString(befriending_class->class_type) << "\"[label=\"" << "befriending_class" << "\" ];" << endl;

               befriending_class = befriending_class->next;
             }
#endif
#if 0
          printf ("After output of befriending classes \n");
          ROSE_ASSERT(false);
#endif
        }

#if 0
     printf ("Leaving graph(a_template_symbol_supplement_ptr) \n");
//   ROSE_ASSERT(false);
#endif
   }


void
EDG_ROSE_Graph::graph ( a_template_decl_ptr template_decl )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_template_decl_ptr): template_decl = %p \n",template_decl);
#endif

  // Output nodes
     if (graphNodeSet.find(template_decl) == graphNodeSet.end())
        {
          file << "\"" << StringUtility::numberToString(template_decl) << "\"[" << "label=\"" << "template_decl" << "\\n" 
               << StringUtility::numberToString(template_decl) << "\" color=\"blue\" ];" << endl;

          insertGraphNode(template_decl);
#if 0
          printf ("In graph(a_template_decl_ptr): template_decl (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",template_decl,graphNodeSet.size());
#endif
        }
       else
        {
          return;
        }

     // TV
     if (template_decl->scope != NULL) {
    // DQ (1/26/2019): Needed to add name of the scope to make this more clear.
    // graph(template_decl->scope);
       graph(template_decl->scope,"template_decl scope");
       file << "\"" << StringUtility::numberToString(template_decl) << "\" -> \"" << StringUtility::numberToString(template_decl->scope)
                         << "\"[label=\"" << "template_decl scope" << "\" color=\"lightgreen\" weight=1];" << endl;
     }

     printf ("In graph(a_template_decl_ptr): process the template parameters \n");

     a_template_parameter_ptr param_list = template_decl->param_list;
  // int counter = 0;

  // DQ (7/16/2017): This is a bug that prevented all but the first template pramter from being processed.
  // if (param_list != NULL)
     while (param_list != NULL)
        {
#if 0
          printf ("In graph(a_template_decl_ptr): counter = %d param_list = %p \n",counter,param_list);
#endif
          graph(param_list);
#if 0
          printf ("DONE: In graph(a_template_decl_ptr): counter = %d param_list = %p \n",counter,param_list);
#endif
#if 1
          file << "\"" << StringUtility::numberToString(template_decl) << "\" -> \"" << StringUtility::numberToString(param_list) << "\"[label=\"" << "template parameter" << "\"];" << endl;
#endif
          param_list = param_list->next;
        }

   }


void
EDG_ROSE_Graph::graph ( a_template_param_type_supplement_ptr extra_info )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_template_param_type_supplement_ptr): extra_info = %p \n",extra_info);
#endif

  // Output nodes
     if (graphNodeSet.find(extra_info) == graphNodeSet.end())
        {
          file << "\"" << StringUtility::numberToString(extra_info) << "\"[" << "label=\"" << "extra template parameter info" << "\\n" 
               << StringUtility::numberToString(extra_info) << "\" color=\"blue\" ];" << endl;

          insertGraphNode(extra_info);
#if 0
          printf ("In graph(a_template_param_type_supplement_ptr): extra_info (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",extra_info,graphNodeSet.size());
#endif
        }
       else
        {
          return;
        }

     printf ("In graph(a_template_param_type_supplement_ptr): extra_info->orig_nested_type = %p \n",extra_info->orig_nested_type);

  // DQ (11/21/2015): Added more info to the graph.
     if (extra_info->orig_nested_type != NULL)
        {
          graph(extra_info->orig_nested_type);
          file << "\"" << StringUtility::numberToString(extra_info) << "\" -> \"" << StringUtility::numberToString(extra_info->orig_nested_type) << "\"[label=\"" << "orig_nested_type" << "\"];" << endl;
        }

     printf ("In graph(a_template_param_type_supplement_ptr): extra_info->class_type = %p \n",extra_info->class_type);

  // DQ (11/21/2015): Added more info to the graph.
     if (extra_info->class_type != NULL)
        {
          graph(extra_info->class_type);
          file << "\"" << StringUtility::numberToString(extra_info) << "\" -> \"" << StringUtility::numberToString(extra_info->class_type) << "\"[label=\"" << "class_type" << "\"];" << endl;
        }

  // TV (03/30/2018):
     if (extra_info->template_symbol != NULL) {
       graph(extra_info->template_symbol);
       file << "\"" << StringUtility::numberToString(extra_info) << "\" -> \"" << StringUtility::numberToString(extra_info->template_symbol) << "\"[label=\"" << "template_symbol" << "\"];" << endl;
     }
   }


void
EDG_ROSE_Graph::graph ( a_template_parameter_ptr template_parameter )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_template_parameter_ptr): template_parameter = %p \n",template_parameter);
#endif

  // Output nodes
     if (graphNodeSet.find(template_parameter) == graphNodeSet.end())
        {
          file << "\"" << StringUtility::numberToString(template_parameter) << "\"[" << "label=\"" << "template_parameter" << "\\n" 
               << StringUtility::numberToString(template_parameter) << "\" color=\"blue\" ];" << endl;

          insertGraphNode(template_parameter);
#if 0
          printf ("In graph(a_template_parameter_ptr): template_parameter (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",template_parameter,graphNodeSet.size());
#endif
        }
       else
        {
          return;
        }

     a_scope_ptr parent_scope = template_parameter->source_corresp.parent_scope;
     if (parent_scope != NULL) {
       graph(parent_scope,"parent scope");
       file << "\"" << StringUtility::numberToString(template_parameter) << "\" -> \"" << StringUtility::numberToString(parent_scope) << "\"[label=\"" << "parent_scope" << "\" color=\"lightgreen\" weight=1];" << endl;
     }

  // DQ (7/15/2017): Graph the different parts of the template parameter.
     if (template_parameter->kind == tpk_type)
        {
          a_type_ptr type = template_parameter->variant.type.ptr;
          ROSE_ASSERT(type != NULL);

          graph(type);
          file << "\"" << StringUtility::numberToString(template_parameter) << "\" -> \"" << StringUtility::numberToString(type) << "\"[label=\"" << "type" << "\"];" << endl;

          a_type_ptr default_arg_type = template_parameter->variant.type.default_arg_type;
          if (default_arg_type != NULL)
             {
               graph(default_arg_type);
               file << "\"" << StringUtility::numberToString(template_parameter) << "\" -> \"" << StringUtility::numberToString(default_arg_type) << "\"[label=\"" << "default_arg_type" << "\"];" << endl;
             }
        }
       else
        {
          if (template_parameter->kind == tpk_nontype)
             {
               a_constant_ptr constant = template_parameter->variant.nontype.constant;
               ROSE_ASSERT(constant != NULL);

               graph(constant);
               file << "\"" << StringUtility::numberToString(template_parameter) << "\" -> \"" << StringUtility::numberToString(constant) << "\"[label=\"" << "constant" << "\"];" << endl;

               a_constant_ptr default_arg_constant = template_parameter->variant.nontype.default_arg_constant;
               if (default_arg_constant != NULL)
                  {
                    graph(default_arg_constant);
                    file << "\"" << StringUtility::numberToString(template_parameter) << "\" -> \"" << StringUtility::numberToString(default_arg_constant) << "\"[label=\"" << "default_arg_constant" << "\"];" << endl;
                  }
             }
            else
             {
               if (template_parameter->kind == tpk_template)
                  {
                    a_template_ptr class_template = template_parameter->variant.templ.class_template;
                    ROSE_ASSERT(class_template != NULL);

                    graph(class_template);
                    file << "\"" << StringUtility::numberToString(template_parameter) << "\" -> \"" << StringUtility::numberToString(class_template) << "\"[label=\"" << "class_template" << "\"];" << endl;

                    a_template_ptr default_arg_template = template_parameter->variant.templ.default_arg_template;
                    if (default_arg_template != NULL)
                       {
                         graph(default_arg_template);
                         file << "\"" << StringUtility::numberToString(template_parameter) << "\" -> \"" << StringUtility::numberToString(default_arg_template) << "\"[label=\"" << "default_arg_template" << "\"];" << endl;
                       }
                  }
                 else
                  {
                    printf ("Error: template parameter must be either a type, nontype, or template \n");
                    ROSE_ASSERT(false);
                  }
             }
        }

  // DQ (8/20/2020): Adding support for attributes.
     an_attribute_ptr attributes = template_parameter->source_corresp.attributes;
     if (attributes != NULL)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_template_parameter_ptr): found attributes \n");
#endif
          graph(attributes);
          file << "\"" << StringUtility::numberToString(template_parameter) << "\" -> \"" << StringUtility::numberToString(attributes) << "\"[label=\"" << "attributes" << "\"];" << endl;
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_template_parameter_ptr): NO attributes \n");
#endif
        }

#if 0
     printf ("Leaving graph(a_template_parameter_ptr): template_parameter = %p \n",template_parameter);
#endif
   }


void
EDG_ROSE_Graph::graph ( a_template_arg_ptr template_argument )
   {
  // DQ (6/16/2016): Adding template arguments to edg graphs.

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_template_arg_ptr): template_argument = %p \n",template_argument);
#endif
#if 1
     printf ("In graph(a_template_arg_ptr): template_argument->kind = %s \n",summarize_templ_arg_kind_in_string(template_argument->kind));
#endif

     ROSE_ASSERT(template_argument != NULL);

  // Output nodes
     if (graphNodeSet.find(template_argument) == graphNodeSet.end())
        {
          file << "\"" << StringUtility::numberToString(template_argument) << "\"[" << "label=\"" << "template_argument" << "\\n" 
               << StringUtility::numberToString(template_argument) << "\" color=\"blue\" ];" << endl;

          insertGraphNode(template_argument);
#if 1
          printf ("In graph(a_template_arg_ptr): template_argument (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",template_argument,graphNodeSet.size());
#endif
        }
       else
        {
          return;
        }

#if 1
     printf ("In graph(a_template_arg_ptr): template_argument->kind = %s \n",summarize_templ_arg_kind_in_string(template_argument->kind));
#endif

     switch (template_argument->kind) {
       case tak_type:
         {
         ROSE_ASSERT(template_argument->variant.type != NULL);

          a_type_ptr type = template_argument->variant.type;
          graph(type);

          file << "\"" << StringUtility::numberToString(template_argument) << "\" -> \"" << StringUtility::numberToString(type) << "\"[label=\"" << "type" << "\"];" << endl;

          break;
         }
       case tak_nontype:
         {
           if (template_argument->is_array_bound_of_unknown_type) {
           // DQ (1/10/2019): This is an unused variable (commented out to clean up compiler warnings).
           // a_targ_size_t integer_value = template_argument->variant.integer_value;
           } else {
             a_constant_ptr constant = template_argument->variant.constant;
             if (constant != NULL) {
               graph(constant);
               file << "\"" << StringUtility::numberToString(template_argument) << "\" -> \"" << StringUtility::numberToString(constant) << "\"[label=\"" << "constant" << "\"];" << endl;
             }
           }
           break;
         }
       case tak_template:
         {
           a_template_ptr tpl = template_argument->variant.templ.ptr;
           if (tpl != NULL) {
             graph(tpl);
             file << "\"" << StringUtility::numberToString(template_argument) << "\" -> \"" << StringUtility::numberToString(tpl) << "\"[label=\"" << "template.ptr" << "\"];" << endl;
           }
           a_template_ptr tpl_sub = template_argument->variant.templ.substituted_param_template;
           if (tpl_sub != NULL) {
             graph(tpl_sub);
             file << "\"" << StringUtility::numberToString(template_argument) << "\" -> \"" << StringUtility::numberToString(tpl_sub) << "\"[label=\"" << "templ.substituted_param_template" << "\"];" << endl;
           }

           break;
         }
       case tak_start_of_pack_expansion:
         {
           break;
         }
       default:
         {
#if 1
          printf ("In graph(a_template_arg_ptr): support not implemented for template_argument->kind = %s \n",summarize_templ_arg_kind_in_string(template_argument->kind));
#endif
         }
     }
   }


void
EDG_ROSE_Graph::graph ( a_template_ptr template_node )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_template_ptr): template_node = %p \n",template_node);
#endif

  // Output nodes
     if (graphNodeSet.find(template_node) == graphNodeSet.end())
        {
          string name = string("template : ");
          if (template_node->source_corresp.name != NULL)
            name += string(template_node->source_corresp.name);

       // DQ (9/25/2015): Added kind of template info.
          name += "\\n kind:";
          name += summarize_template_kind_in_string(template_node->kind);
#if 0
          printf ("In graph (a_template_ptr = %p): name = %s \n",template_node,name.c_str());
#endif
          file << "\"" << StringUtility::numberToString(template_node) << "\"[" << "label=\"" << name << "\\n" 
               << StringUtility::numberToString(template_node) << "\" color=\"blue\" ];" << endl;

          insertGraphNode(template_node);
#if 0
          printf ("In graph(a_template_ptr): template_node (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",template_node,graphNodeSet.size());
#endif
        }
       else
        {
          return;
        }

     // Parent scope
     a_scope_ptr parent_scope = template_node->source_corresp.parent_scope;
     if (parent_scope != NULL)
        {
          graph(parent_scope,"template scope");
          file << "\"" << StringUtility::numberToString(template_node) << "\" -> \"" << StringUtility::numberToString(parent_scope) << "\"[label=\"" << "parent_scope" << "\" color=\"lightgreen\" weight=1];" << endl;
        }

     // Output a_template_symbol_supplement
     if (template_node->template_info != NULL)
        {
          graph(template_node->template_info);
#if 1
          file << "\"" << StringUtility::numberToString(template_node) << "\" -> \"" << StringUtility::numberToString(template_node->template_info) << "\"[label=\"" << "template_info" << "\"];" << endl;
#endif
        }

     // Output a_template_decl_ptr
     if (template_node->template_decl != NULL)
        {
          graph(template_node->template_decl);
#if 1
          file << "\"" << StringUtility::numberToString(template_node) << "\" -> \"" << StringUtility::numberToString(template_node->template_decl) << "\"[label=\"" << "template_decl" << "\"];" << endl;
#endif
        }

     switch (template_node->kind) {
       case templk_function:
       case templk_member_function:
         {
           ROSE_ASSERT(template_node->prototype_instantiation.routine != NULL);
           graph(template_node->prototype_instantiation.routine);
#if 1
           file << "\"" << StringUtility::numberToString(template_node) << "\" -> \"" << StringUtility::numberToString(template_node->prototype_instantiation.routine) << "\"[label=\"" << "routine instantiation" << "\"];" << endl;
#endif
#if 0
           printf ("Calling summerize_template(): \n");
           summarize_template(template_node);
           printf ("Calling summerize_routine(): \n");
           summarize_routine(template_node->prototype_instantiation.routine);
#endif
           break;
         }
       case templk_class:
       case templk_member_class:
       case templk_member_enum:
         {
           if (template_node->prototype_instantiation.type != NULL) {
             graph(template_node->prototype_instantiation.type);
             file << "\"" << StringUtility::numberToString(template_node) << "\" -> \"" << StringUtility::numberToString(template_node->prototype_instantiation.type) << "\"[label=\"" << "class instantiation" << "\"];" << endl;
           }
           break;
         }
       case templk_variable:
       case templk_static_data_member:
         {
           ROSE_ASSERT(template_node->prototype_instantiation.variable != NULL);
           graph(template_node->prototype_instantiation.variable);
           file << "\"" << StringUtility::numberToString(template_node) << "\" -> \"" << StringUtility::numberToString(template_node->prototype_instantiation.variable) << "\"[label=\"" << "variable instantiation" << "\"];" << endl;
           break;
         }
       case templk_template_template_param:
         {
           // No `variant` field
           break;
         }
       default:
         {
           printf("Error: must have been at least function, class, or variable \n");
           ROSE_ASSERT(false);
         }
     }

     // Output a_template_ptr canonical_template
     if (template_node->canonical_template != NULL)
        {
          graph(template_node->canonical_template);
#if 1
          file << "\"" << StringUtility::numberToString(template_node) << "\" -> \"" << StringUtility::numberToString(template_node->canonical_template) << "\"[label=\"" << "canonical" << "\"];" << endl;
#endif
        }

     // Output a_template_ptr definition_template
     if (template_node->definition_template != NULL)
        {
          graph(template_node->definition_template);
#if 1
          file << "\"" << StringUtility::numberToString(template_node) << "\" -> \"" << StringUtility::numberToString(template_node->definition_template) << "\"[label=\"" << "definition" << "\"];" << endl;
#endif
        }

     // Output a_template_ptr prototype_template
     if (template_node->prototype_template != NULL)
        {
          graph(template_node->prototype_template);
#if 1
          file << "\"" << StringUtility::numberToString(template_node) << "\" -> \"" << StringUtility::numberToString(template_node->prototype_template) << "\"[label=\"" << "prototype" << "\"];" << endl;
#endif
        }


  // DQ (8/20/2020): Adding support for attributes.
     an_attribute_ptr attributes = template_node->source_corresp.attributes;
     if (attributes != NULL)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_template_ptr): found attributes \n");
#endif
          graph(attributes);
          file << "\"" << StringUtility::numberToString(template_node) << "\" -> \"" << StringUtility::numberToString(attributes) << "\"[label=\"" << "attributes" << "\"];" << endl;
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_template_ptr): NO attributes \n");
#endif
        }
   }


void
EDG_ROSE_Graph::graph ( a_namespace_ptr namespace_node )
   {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_namespace_ptr): namespace_node = %p \n",namespace_node);
#endif

     string name = string("namespace : ");
     if (namespace_node->source_corresp.name != NULL) {
       name += string(namespace_node->source_corresp.name);
     } else {
       name += string("__unnamed__");
     }

  // DQ (4/7/2018): Adding information about namespace aliasing.
     if (namespace_node->is_namespace_alias == true)
        {
          name += "\\n namespace alias ";
        }

  // Output nodes
     if (graphNodeSet.find(namespace_node) == graphNodeSet.end())
        {
#if 1
          printf ("In graph(a_namespace_ptr): name = %s \n",name.c_str());
#endif

          file << "\"" << StringUtility::numberToString(namespace_node) << "\"[" << "label=\"" << name << "\\n" 
               << StringUtility::numberToString(namespace_node) << "\" color=\"blue\" ];" << endl;

          insertGraphNode(namespace_node);
#if 0
          printf ("In graph(a_namespace_ptr): namespace_node (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",namespace_node,graphNodeSet.size());
#endif
        }
       else
        {
#if 1
          printf ("In graph(a_namespace_ptr): graphNodeSet previously processed in graphNodeSet: namespace_node = %p \n",namespace_node);
#endif
          return;
        }

#if 1
  // DQ (4/7/2018): Adding edges to the namespace support.
     if (namespace_node->is_namespace_alias == false && namespace_node->variant.assoc_scope != NULL)
        {
          graph(namespace_node->variant.assoc_scope,name);

          file << "\"" << StringUtility::numberToString(namespace_node) << "\" -> \"" << StringUtility::numberToString(namespace_node->variant.assoc_scope) << "\"[label=\"" << "assoc_scope" << "\"];" << endl;
        }
#endif

#if 1
  // DQ (4/7/2018): Adding edges to the namespace support.
     if (namespace_node->is_namespace_alias == true && namespace_node->variant.assoc_namespace != NULL)
        {
          graph(namespace_node->variant.assoc_namespace);

          file << "\"" << StringUtility::numberToString(namespace_node) << "\" -> \"" << StringUtility::numberToString(namespace_node->variant.assoc_namespace) << "\"[label=\"" << "assoc_namespace" << "\"];" << endl;
        }
#endif

  // DQ (8/20/2020): Adding support for attributes.
     an_attribute_ptr attributes = namespace_node->source_corresp.attributes;
     if (attributes != NULL)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_namespace_ptr): found attributes \n");
#endif
          graph(attributes);
          file << "\"" << StringUtility::numberToString(namespace_node) << "\" -> \"" << StringUtility::numberToString(attributes) << "\"[label=\"" << "attributes" << "\"];" << endl;
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_namespace_ptr): NO attributes \n");
#endif
        }
   }


void
EDG_ROSE_Graph::graph ( a_block_ptr block )
   {
  // DQ (1/31/2018): Adding more support for scopes and blocks.

  // bool previoudlyProcessedBlock = false;

// #if 0
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_block_ptr): block = %p \n",block);
#endif

     if (graphNodeSet.find(block) == graphNodeSet.end())
        {
          file << "\"" << StringUtility::numberToString(block) << "\"[" << "label=\"" << "block" << "\\n" << StringUtility::numberToString(block) << "\"];" << endl;

          insertGraphNode(block);
#if 0
          printf ("In graph(a_block_ptr): top (block = %p): graphNodeSet.size() = %" PRIuPTR " \n",block,graphNodeSet.size());
#endif
        }
       else
        {
#if 0
          printf ("In graph(a_block_ptr): graphNodeSet previously processed in graphNodeSet: block = %p \n",block);
#endif
          return;
        }

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph(a_block_ptr): block = %p \n",block);
#endif

  // Use this code after the test for existence in graphNodeSet.  This permits nodes to be generated for unhandled leaf nodes that are compiler generated (e.g. non-explicit return-stmt)
  // Since these are not frequent statements this is OK, were as doing the same in the graph(acope_ptr) function would generate too large of a graph (e.g. declarations from header files).
     if (outputInGraph(block) == false)
        {
#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
          printf ("In graph(a_block_ptr): Statement will not be output in graph block = %p \n",block);
#endif
       // Premature return for test2012_02.C (c++11 example).
       // return;
        }

#if 0
  // DQ (1/31/2018): This appears to be redundant (and thus builds a redundant edge).
     if (block->assoc_scope != NULL)
        {
          graph(block->assoc_scope);

          file << "\"" << StringUtility::numberToString(block) << "\" -> \"" << StringUtility::numberToString(block->assoc_scope) << "\"[label=\"" << "assoc_scope_xxx" << "\"];" << endl;
        }
#endif
   }

void
EDG_ROSE_Graph::graph ( a_symbol_ptr symbol )
   {
  // TV (03/29/2018): adding support for symbols
     if (graphNodeSet.find(symbol) == graphNodeSet.end())
        {
          file << "\"" << StringUtility::numberToString(symbol) << "\"[" << "label=\"" << "symbol" << "\\n" << StringUtility::numberToString(symbol);
          if (symbol->is_class_member) file << "\\nis_class_member";
          if (symbol->is_template_param) file << "\\nis_template_param";
          if (symbol->is_nonreal_nested_type) file << "\\nis_nonreal_nested_type";
          if (symbol->is_nonreal_member) file << "\\nis_nonreal_member";
          file << "\"];" << endl;

          insertGraphNode(symbol);
        }
       else
        {
          return;
        }

       if (symbol->is_class_member) {
         a_type_ptr type = symbol->parent.class_type;
         ROSE_ASSERT(type != NULL);

         file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(type) << "\"[label=\"" << "parent_class_type" << "\"];" << endl;

         graph(type);
       }
       else if (symbol->parent.namespace_ptr != NULL) {
         a_namespace_ptr nsp = symbol->parent.namespace_ptr;
         ROSE_ASSERT(nsp != NULL);

         file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(nsp) << "\"[label=\"" << "parent_namespace" << "\"];" << endl;

         graph(nsp);
       }

       if (symbol->corresp_nonreal_or_nested_type != NULL) {
         a_symbol_ptr corresp_nonreal_or_nested_type = symbol->corresp_nonreal_or_nested_type;
         ROSE_ASSERT(corresp_nonreal_or_nested_type != NULL);

         file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(corresp_nonreal_or_nested_type) << "\"[label=\"" << "corresp_nonreal_or_nested_type" << "\"];" << endl;

         graph(corresp_nonreal_or_nested_type);
       }

       switch (symbol->kind) {
         case sk_constant:
           {
             a_constant_ptr constant = symbol->variant.constant;
             if (constant != NULL) {
               graph(constant);
               file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(constant) << "\"[label=\"" << "constant" << "\"];" << endl;
             }
             break;
           }
         case sk_type:
           {
             a_type_ptr type = symbol->variant.type.ptr;
             if (type != NULL) {
               graph(type);
               file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(type) << "\"[label=\"" << "type" << "\"];" << endl;
             }
             break;
           }
         case sk_enum_tag:
           {
             a_type_ptr type = symbol->variant.enumeration.type;
             if (type != NULL) {
               graph(type);
               file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(type) << "\"[label=\"" << "enum" << "\"];" << endl;
             }
             break;
           }
         case sk_class_or_struct_tag:
         case sk_union_tag:
           {
             a_type_ptr type = symbol->variant.class_struct_union.type;
             if (type != NULL) {
               graph(type);
               file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(type) << "\"[label=\"" << "class_struct_union" << "\"];" << endl;
             }
             break;
           }
         case sk_variable:
           {
             a_variable_ptr variable = symbol->variant.variable.ptr;
             if (variable != NULL) {
               graph(variable);
               file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(variable) << "\"[label=\"" << "variable" << "\"];" << endl;
             }
             break;
           }
         case sk_static_data_member:
           {
             a_variable_ptr variable = symbol->variant.static_data_member.variable;
             if (variable != NULL) {
               graph(variable);
               file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(variable) << "\"[label=\"" << "static_data_member" << "\"];" << endl;
             }
             break;
           }
         case sk_field:
           {
             a_field_ptr field = symbol->variant.field.ptr;
             if (field != NULL) {
               graph(field);
               file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(field) << "\"[label=\"" << "field" << "\"];" << endl;
             }
             break;
           }
         case sk_routine:
         case sk_member_function:
           {
             a_routine_ptr routine = symbol->variant.routine.ptr;
             if (routine != NULL) {
               graph(routine);
               file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(routine) << "\"[label=\"" << "routine" << "\"];" << endl;
             }
             break;
           }
         case sk_class_template:
         case sk_function_template:
         case sk_variable_template:
           {
             a_template_symbol_supplement_ptr template_info = symbol->variant.template_info;
             if (template_info != NULL) {
               graph(template_info);
               file << "\"" << StringUtility::numberToString(symbol) << "\" -> \"" << StringUtility::numberToString(template_info) << "\"[label=\"" << "template_info" << "\"];" << endl;
             }
             break;
           }
         default:
           break;
       }
   }

void
EDG_ROSE_Graph::graph ( a_base_class_ptr base_class )
   {
     if (base_class->type != NULL) {
       graph(base_class->type);
       file << "\"" << StringUtility::numberToString(base_class) << "\" -> \"" << StringUtility::numberToString(base_class->type) << "\"[label=\"type\"];" << endl;
     }
     if (base_class->orig_type != NULL) {
       graph(base_class->orig_type);
       file << "\"" << StringUtility::numberToString(base_class) << "\" -> \"" << StringUtility::numberToString(base_class->orig_type) << "\"[label=\"orig_type\"];" << endl;
     }
     if (base_class->derived_class != NULL) {
       graph(base_class->derived_class);
       file << "\"" << StringUtility::numberToString(base_class) << "\" -> \"" << StringUtility::numberToString(base_class->derived_class) << "\"[label=\"derived_class\"];" << endl;
     }
   }


// ************************************************************************
// ************************************************************************
// ************************************************************************

bool
EDG_ROSE_Graph::outputInGraph ( SourceSequenceList & sse )
   {
#if 1
     printf ("In outputInGraph(SourceSequenceList): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
     an_il_entry_kind kind = (an_il_entry_kind) (sse->entity.kind);
  // printf ("What is this sse_kind = %s \n",summarize_entry_kind_in_string_kind(kind));

     bool result = false;
     switch (kind)
        {
          case iek_src_seq_sublist:
             {
               printf ("In outputInGraph(): case iek_src_seq_sublist: I don't think we see these entries \n");
               ROSE_ASSERT(false);
               break;
             }

          case iek_src_seq_secondary_decl:
             {
               result = outputInGraph(((a_src_seq_secondary_decl_ptr)(sse->entity.ptr))->decl_position);
               break;
             }

          case iek_type:
             {
               result = outputInGraph(((a_type_ptr)(sse->entity.ptr))->source_corresp.decl_position);
               break;
             }

          case iek_routine:
             {
               result = outputInGraph(((a_routine_ptr)(sse->entity.ptr))->source_corresp.decl_position);
               break;
             }

          case iek_variable:
             {
               result = outputInGraph(((a_variable_ptr)(sse->entity.ptr))->source_corresp.decl_position);
               break;
             }

          case iek_macro:
             {
               result = outputInGraph(((a_macro_ptr)(sse->entity.ptr))->source_corresp.decl_position);
               break;
             }

          case iek_pragma:
             {
               result = outputInGraph(((a_pragma_ptr)(sse->entity.ptr))->position);
               break;
             }

          case iek_asm_entry:
             {
               result = outputInGraph(((an_asm_entry_ptr)(sse->entity.ptr))->source_corresp.decl_position);
               break;
             }

          case iek_namespace:
             {
#if 1
               printf ("Processing case iek_namespace: \n");
#endif
               result = outputInGraph(((a_namespace_ptr)(sse->entity.ptr))->source_corresp.decl_position);
               break;
             }

          case iek_template:
             {
               result = outputInGraph(((a_template_ptr)(sse->entity.ptr))->source_corresp.decl_position);
               break;
             }

          case iek_field:
             {
               result = outputInGraph(((a_field_ptr)(sse->entity.ptr))->source_corresp.decl_position);
               break;
             }

          case iek_using_decl:
             {
               result = outputInGraph(((a_using_decl_ptr)(sse->entity.ptr))->position);
               break;
             }

          case iek_instantiation_directive:
             {
               result = outputInGraph(((an_instantiation_directive_ptr)(sse->entity.ptr))->position);
               break;
             }

       // DQ (9/7/2013): Adding more cases (because these are entries that don't show up in global or namespace scope).
          case iek_block:
             {
               result = outputInGraph(((a_block_ptr)(sse->entity.ptr))->final_position);
#if 1
               printf ("In outputInGraph(): case of iek_block: result = %s \n",(result == true) ? "true" : "false");
#endif
               break;
             }

          case iek_statement:
             {
               result = outputInGraph(((a_statement_ptr)(sse->entity.ptr))->position);
#if 1
               printf ("In outputInGraph(): case of iek_statement: result = %s \n",(result == true) ? "true" : "false");
#endif
               break;
             }

       // DQ (9/9/2013): Added to support test2009_08.c
          case iek_constant:
             {
               result = false; // outputInGraph(((a_statement_ptr)(sse->entity.ptr))->position);
               break;
             }

#if 1
          case iek_src_seq_end_of_construct:
             {
            // We should never see one of these since the sse operator++() takes care of these, i think.
#if 0
               printf ("In outputInGraph(): case iek_src_seq_end_of_construct: Processing case iek_src_seq_end_of_construct (part of new template support) \n");
               ROSE_ASSERT(false);
#endif
            // We want to always include the iek_src_seq_end_of_construct nodes into the graph (structurally significant).
            // We can also later (if there are too many) narrow the focus to those that are associated with blocks that are in the graph.
               result = true;

            // Don't allow the outputInGraph() to have an effect on the source sequence list.
            // ++sse;
               break;
             }
#endif
   // DQ (12/29/2008): Added EDG 4.0 cases not handled
          case iek_none:
          case iek_source_file:
       // case iek_constant:
          case iek_param_type:
          case iek_routine_type_supplement:
          case iek_based_type_list_member:
          case iek_exception_specification:
          case iek_exception_specification_type:
          case iek_label:
          case iek_expr_node:
          case iek_for_loop:
          case iek_switch_case_entry:
          case iek_switch_stmt_descr:
          case iek_handler:
          case iek_try_supplement:
#if MICROSOFT_EXTENSIONS_ALLOWED
          case iek_microsoft_try_supplement:
          case iek_ms_attribute:
          case iek_ms_attribute_arg:
          case iek_ms_if_exists:
#endif
//        case iek_block:
//        case iek_statement:
          case iek_object_lifetime:
          case iek_scope:
          case iek_id_name:
          case iek_string_text:
          case iek_other_text:
          case iek_internal_complex_value:
       // case iek_using_decl:
          case iek_dynamic_init:
          case iek_local_static_variable_init:
          case iek_vla_dimension:
          case iek_overriding_virtual_function:
          case iek_derivation_step:
          case iek_base_class_derivation:
          case iek_base_class:
          case iek_class_list_entry:
          case iek_routine_list_entry:
          case iek_class_type_supplement:
          case iek_template_param_type_supplement:
          case iek_constructor_init:
#if GNU_EXTENSIONS_ALLOWED
          case iek_asm_operand:
#if !RECORD_RAW_ASM_OPERAND_DESCRIPTIONS
          case iek_asm_operand_constraint:
#endif
          case iek_named_register_list:
#endif
          case iek_template_arg:
          case iek_new_delete_supplement:
          case iek_throw_supplement:
          case iek_condition_supplement:
          case iek_source_sequence_entry:
#if !HANDLE_SOURCE_SEQUENCE_END
//        case iek_src_seq_end_of_construct:
#endif
//        case iek_src_seq_sublist:
       // case iek_instantiation_directive:
          case iek_scope_orphaned_list_header:
          case iek_decl_position_supplement:
          case iek_template_decl:
          case iek_template_parameter:
          case iek_seq_number_lookup_entry:
          case iek_local_expr_node_ref:
          case iek_static_assertion:
          case iek_local_scope_ref:
          case iek_il_entity_list_entry:

       // DQ (7/21/2012): Added lambda cases (but might not be required here).
          case iek_lambda:
          case iek_lambda_capture:
          case iek_name_reference:
          case iek_name_qualifier:
          case iek_last:
             {
               printf ("In outputInGraph(): Case not handled! kind = %s \n",il_entry_kind_names[kind]);
               ROSE_ASSERT(false);
               break;
             }

          default:
             {
               printf ("In outputInGraph(): Unhandled sse kind: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
             }
        }

     return result;
   }

bool
EDG_ROSE_Graph::outputInGraph( const a_source_position & pos )
   {
     const an_il_header & header   = il_header;
     a_source_file_ptr source_file = header.primary_source_file;

     string filenameFromHeader = source_file->file_name;
     string filenameFromNode   = source_position_filename(pos);

#if 0
     printf ("In outputInGraph(): filenameFromHeader = %s filenameFromNode = %s \n",filenameFromHeader.c_str(),filenameFromNode.c_str());
#endif

     bool match = (filenameFromNode == filenameFromHeader);

     return match;
   }

bool
EDG_ROSE_Graph::outputInGraph( a_statement_ptr statement )
   {
     bool match = outputInGraph(statement->position);
     return match;
   }

bool
EDG_ROSE_Graph::outputInGraph( a_block_ptr block )
   {
     bool match = outputInGraph(block->final_position);
     return match;
   }

// ************************************************************************
// ************************************************************************
// ************************************************************************

void
EDG_ROSE_Graph::graph_edg_ast()
   {
  // DQ (9/6/2013): Build a dot graph of the EDG AST.

  // Build filename...
     string filename = "edgGraph";

     string dot_header = filename;
     filename += ".dot";

#if DEBUG_EDG_DOT_GRAPH_SUPPPORT
     printf ("In graph_edg_ast(): filename = %s \n",filename.c_str());
#endif

  // std::set<void*> graphNodeSet;
     if (graphNodeSet.empty() == false)
        {
#if 1
          printf ("In graph_edg_ast(): calling graphNodeSet.clear() to support multiple files \n");
#endif
          graphNodeSet.clear();
        }

  // Open file...(file is declared in the EDG_ROSE_Graph namespace).
     file.open(filename.c_str());

  // Output the opening header for a DOT file.
     file << "digraph \"" << dot_header << "\" {" << endl;

  // Calling recursive function to build dot file for scope, starting at the top level scope.
     graph (il_header.primary_scope,"EDG Global Scope");

  // Function "main" is stored seperately.
     if (il_header.main_routine != NULL)
        {
          graph (il_header.main_routine);
        }

  // Close off the DOT file.
     file << endl;
     file << "} " << endl;
     file.close();

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif
   }


void
EDG_ROSE_Graph::graph_headers( a_source_file_ptr edg_file )
   {
  // DQ (9/17/2018): Build a dot graph of the header files used and where they are included from (should be a tree).

  // DQ (6/15/2019): Output a message so that we can know the DOT file is being output.
     printf ("Output DOT file in graph_headers() \n");

#if DEBUG_HEADER_GRAPH_SUPPPORT
     printf ("In graph_headers(): file_name = %s \n",edg_file->file_name);
#endif

     if (graphNodeSet.find(edg_file) == graphNodeSet.end())
        {
          string name = string("file : ") + edg_file->file_name;

       // DQ (4/7/2018): Adding information about namespace aliasing.
          if (edg_file->is_include_file == true)
             {
               name += "\\n included file ";
             }
#if DEBUG_HEADER_GRAPH_SUPPPORT
          printf ("In graph_headers(): name = %s \n",name.c_str());
#endif

          file << "\"" << StringUtility::numberToString(edg_file) << "\"[" << "label=\"" << name << "\\n" 
               << StringUtility::numberToString(edg_file) << "\" color=\"blue\" ];" << endl;

          insertGraphNode(edg_file);
#if DEBUG_HEADER_GRAPH_SUPPPORT
          printf ("In graph_headers(): edg_file (node = %p): graphNodeSet.size() = %" PRIuPTR " \n",edg_file,graphNodeSet.size());
#endif
        }
       else
        {
#if DEBUG_HEADER_GRAPH_SUPPPORT
          printf ("In graph_headers(): graph_headers previously processed, edg_file in graphNodeSet: edg_file = %p \n",edg_file);
#endif
          return;
        }

     a_source_file_ptr temp_file = edg_file->first_child_file;
  // while (temp_file != ptr->last_child_file)
     while (temp_file != NULL)
        {
       // printf ("temp_file = %p \n",temp_file);

          graph_headers(temp_file);

          file << "\"" << StringUtility::numberToString(edg_file) << "\" -> \"" << StringUtility::numberToString(temp_file) << "\"[label=\"" << "include_file" << "\"];" << endl;

          temp_file = temp_file->next;
        }
   }


void
EDG_ROSE_Graph::graph_headers()
   {
  // DQ (9/17/2018): Build a dot graph of the header files used and where they are included from (should be a tree).

  // Build filename...
     string filename = "edgHeaderGraph";

     string dot_header = filename;
     filename += ".dot";

#if DEBUG_HEADER_GRAPH_SUPPPORT
     printf ("In graph_headers(): filename = %s \n",filename.c_str());
#endif

  // Open file...(file is declared in the EDG_ROSE_Graph namespace).
     file.open(filename.c_str());

  // Output the opening header for a DOT file.
     file << "digraph \"" << dot_header << "\" {" << endl;

  // Calling recursive function to build dot file for scope, starting at the top level scope.
     graph_headers (il_header.primary_source_file);

  // Function "main" is stored seperately.
     if (il_header.main_routine != NULL)
        {
          graph_headers (il_header.primary_source_file);
        }

  // Close off the DOT file.
     file << endl;
     file << "} " << endl;
     file.close();
   }






