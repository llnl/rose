// These functions have been separated from Support.code as a way of
// both simplifying the development and also permitting more specialized
// optimizations (file specific compiler flags).

// tps (01/14/2010) : Switching from rose.h to sage3.
#include "sage3basic.h"

// DQ (10/14/2010):  This should only be included by source files that require it.
// This fixed a reported bug which caused conflicts with autoconf macros (e.g. PACKAGE_BUGREPORT).
#include "rose_config.h"

using namespace std;

// DQ (5/3/2009): We might want a version of this function expressed in terms of the node_index and edge_index (integer) values.
// This might not be possible since the rose_graph_integer_edge_hash_multimap is (node index --> SgGraphEdge*).

//! Generate a set of SgGraphEdge pointers associated with a given SgGraphNode.
std::set<SgGraphEdge*>
SgGraph::computeEdgeSet( SgGraphNode* node )
   {
     ASSERT_this();
     ROSE_ASSERT(node != NULL);

     std::set<SgGraphEdge*>  returnSet;



  // printf ("In SgGraph::computeEdgeSet(node=%p=%d) p_node_index_to_edge_multimap.size() = %" PRIuPTR " \n",node,node->get_index(),p_node_index_to_edge_multimap.size());

     typedef std::pair<rose_graph_integer_edge_hash_multimap::const_iterator,rose_graph_integer_edge_hash_multimap::const_iterator> equal_range_type;
     equal_range_type equal_range_pair = p_node_index_to_edge_multimap.equal_range(node->get_index());

     for (rose_graph_integer_edge_hash_multimap::const_iterator i = equal_range_pair.first; i != equal_range_pair.second; i++)
        {
       // int edge_index = i->second;
       // SgGraphEdge* edge = p_node_index_to_edge_multimap[edge_index];
          SgGraphEdge* edge = i->second;

          int edge_node_first  = edge->get_node_A()->get_index();
          int edge_node_second = edge->get_node_B()->get_index();

          ROSE_ASSERT(edge_node_first == node->get_index() || edge_node_second == node->get_index());

          //printf ("Building set with edge = %d between nodes (%d,%d) \n",edge->get_index(),edge_node_first,edge_node_second);

          returnSet.insert(edge);
        }


     return returnSet;
   }

std::set<int>
SgGraph::computeEdgeSet( int node_index )
   {
     ASSERT_this();

     std::set<int>  returnSet;

// #ifdef ROSE_USE_NEW_GRAPH_NODES
     std::set<SgGraphEdge*> edgeSet = computeEdgeSet( p_node_index_to_node_map[node_index] );

     std::set<SgGraphEdge*>::iterator i = edgeSet.begin();
     while (i != edgeSet.end())
        {
          returnSet.insert((*i)->get_index());
          i++;
        }
// #endif

     return returnSet;
   }


//! Generate a set of SgGraphEdge pointers associated with a given SgGraphNode.
std::set< std::pair<int,int> >
SgGraph::computeNodeIndexPairSet( int node_index )
   {
     ASSERT_this();

     std::set< std::pair<int,int> >  returnSet;

// #ifdef ROSE_USE_NEW_GRAPH_NODES
     typedef std::pair<rose_graph_integer_edge_hash_multimap::const_iterator,rose_graph_integer_edge_hash_multimap::const_iterator> equal_range_type;
     equal_range_type equal_range_pair = p_node_index_to_edge_multimap.equal_range(node_index);

     for (rose_graph_integer_edge_hash_multimap::const_iterator i = equal_range_pair.first; i != equal_range_pair.second; i++)
        {
          SgGraphEdge* edge = i->second;

          ROSE_ASSERT(edge != NULL);

          int edge_node_first  = edge->get_node_A()->get_index();
          int edge_node_second = edge->get_node_B()->get_index();

          ROSE_ASSERT(edge_node_first == node_index || edge_node_second == node_index);

          //printf ("Building set with edge = %d between nodes (%d,%d) \n",edge->get_index(),edge_node_first,edge_node_second);

          returnSet.insert(std::pair<int,int>(edge_node_first,edge_node_second));
        }
// #endif

     return returnSet;
   }


//! Generate a set of SgGraphNode pointers associated with a given string label.
std::set<SgGraphNode*>
SgGraph::computeNodeSet()
   {
     ASSERT_this();
     std::set<SgGraphNode*>  returnSet;

     rose_graph_integer_node_hash_map::iterator i = p_node_index_to_node_map.begin();
     while (i != p_node_index_to_node_map.end())
        {
          returnSet.insert(i->second);
          i++;
        }

     return returnSet;
   }

//! Generate a set of SgGraphNode pointers associated with a given string label.
std::set<SgGraphNode*>
SgGraph::computeNodeSet( const string & label )
   {
     ASSERT_this();
     std::set<SgGraphNode*>  returnSet;

     std::set<int> nodeIndexSet = computeNodeIndexSet( label );

     std::set<int>::iterator i = nodeIndexSet.begin();
     while (i != nodeIndexSet.end())
        {
          returnSet.insert(p_node_index_to_node_map[*i]);
          i++;
        }

     return returnSet;
   }

//! Generate a set of node index values associated with a given string label.
std::set<int>
SgGraph::computeNodeIndexSet( const string & label )
   {
     ASSERT_this();

#if 0
     typedef std::pair<rose_graph_string_integer_hash_multimap::const_iterator,rose_graph_string_integer_hash_multimap::const_iterator> equal_range_type;
     equal_range_type equal_range_pair = p_string_to_node_index_multimap.equal_range(node->get_name());
     rose_graph_string_integer_hash_multimap::const_iterator lower_bound = equal_range_pair.first;
     rose_graph_string_integer_hash_multimap::const_iterator upper_bound = equal_range_pair.second;
     ROSE_ASSERT(lower_bound != upper_bound);
#endif

     std::set<int>  returnSet;

     typedef std::pair<rose_graph_string_integer_hash_multimap::const_iterator,rose_graph_string_integer_hash_multimap::const_iterator> equal_range_type;
     equal_range_type equal_range_pair = p_string_to_node_index_multimap.equal_range(label);

  // DQ (3/25/2017): Eliminated Clange warning about unused variables.
  // rose_graph_string_integer_hash_multimap::const_iterator lower_bound = equal_range_pair.first;
  // rose_graph_string_integer_hash_multimap::const_iterator upper_bound = equal_range_pair.second;

  // DQ (8/18/2009): This CAN be an empty set so we have to allow this.
  // ROSE_ASSERT(lower_bound != upper_bound);

     for (rose_graph_string_integer_hash_multimap::const_iterator i = equal_range_pair.first; i != equal_range_pair.second; i++)
        {
          int node_index = i->second;
       // printf ("Found node %d associated with label = %s \n",node_index,label.c_str());
          returnSet.insert(node_index);
        }


     return returnSet;
   }


size_t
SgGraph::numberOfGraphNodes() const
   {
  // Number of nodes in graph.
  // Note that there is a static function SgNode::numberOfNodes() which can't be overloaded!

// DQ (8/18/2009): Commented out consitional compilation
// #ifdef ROSE_USE_NEW_GRAPH_NODES
     return p_node_index_to_node_map.size();
// #else
//      return 0;
// #endif
   }


size_t
SgGraph::numberOfGraphEdges() const
   {
  // Number of edges in graph.

// DQ (8/18/2009): Commented out consitional compilation
// #ifdef ROSE_USE_NEW_GRAPH_NODES
     return p_edge_index_to_edge_map.size();
// #else
//      return 0;
// #endif
   }


bool
SgGraph::exists( SgGraphNode* node )
   {
     ASSERT_this();
     ROSE_ASSERT(node != NULL);

     return (p_node_index_to_node_map.find(node->get_index()) != p_node_index_to_node_map.end());
   }

bool
SgGraph::exists( SgGraphEdge* edge )
   {
     ASSERT_this();
     ROSE_ASSERT(edge != NULL);

     return (p_edge_index_to_edge_map.find(edge->get_index()) != p_edge_index_to_edge_map.end());
   }



// DQ (4/28/2009): Added support to build node (automaticly added to SgGraph).
SgGraphNode*
SgGraph::addNode( const std::string & name, SgNode* internal_node )
   {
     ASSERT_this();

     SgGraphNode* node = new SgGraphNode(name);
     ROSE_ASSERT(node != NULL);

  // Set the reference to the associated AST node
     node->set_SgNode(internal_node);
     node->set_name(name);
     return addNode(node);
   }

SgGraphNode*
SgGraph::addNode( SgGraphNode* node )
   {
     ASSERT_this();
     ROSE_ASSERT(node != NULL);

     node->set_parent(this);

     if (exists(node) == false)
        {
          int node_index = node->get_index();
          p_node_index_to_node_map[node_index] = node;

       // This fails for test_graph_5.C
       // ROSE_ASSERT(p_string_to_node_index_multimap.find(node->get_name()) == p_string_to_node_index_multimap.end());

       // Adding the label to the label map (we might decide to not put empty labels into the map).
       // p_string_to_node_index_multimap[node->get_name()] = node_index;
          if (node->get_name().empty() == false)
               p_string_to_node_index_multimap.insert(std::pair<std::string,int>(node->get_name(),node_index));
        }
       else
        {
          printf ("This node already exists in this graph \n");
          ROSE_ABORT();
        }
     return node;
   }


SgGraphEdge*
SgGraph::addEdge( SgGraphNode* a, SgGraphNode* b, const std::string & name )
   {
     ASSERT_this();

     SgGraphEdge* edge = NULL;

     edge = new SgGraphEdge(a,b,name);
     ROSE_ASSERT(edge != NULL);
     return addEdge(edge);
   }


// DQ (4/28/2009): Added support to build node (automaticly added to SgGraph).
SgGraphEdge*
SgGraph::addEdge( SgGraphEdge* edge )
   {
     ASSERT_this();

     edge->set_parent(this);

     if (exists(edge) == false)
        {
          int edge_index = edge->get_index();
          p_edge_index_to_edge_map[edge_index] = edge;

          ROSE_ASSERT(edge->get_node_A() != NULL);
          ROSE_ASSERT(edge->get_node_B() != NULL);

          int node_index_first  = edge->get_node_A()->get_index();
          int node_index_second = edge->get_node_B()->get_index();

       // Note that this significantly slows down the performance of the new graph support (appears to be about a factor of 10X).
       // Is there a better (faster) way to build the p_node_index_pair_to_edge_multimap?

       // DQ (5/2/2009): Note that operator[] is not available for the multimap container.
       // p_node_index_pair_to_edge_multimap[std::pair<int,int>(edge->get_node_A(),edge->get_node_B())] = edge;
#if 1
       // p_node_index_pair_to_edge_multimap.insert(std::pair<std::pair<int,int>,SgGraphEdge*>(std::pair<int,int>(edge->get_node_A()->get_index(),edge->get_node_B()->get_index()),edge));
       //
// CH (4/9/2010): Use boost::unordered instead 
//#ifdef _MSCx_VER
#if 0
// tps (12/09/09) : Cannot compile this right now.
//#pragma message("rose_graph_support.C: Problem compiling multimap")
#else
                  p_node_index_pair_to_edge_multimap.insert(std::pair<std::pair<int,int>,SgGraphEdge*>(std::pair<int,int>(node_index_first,node_index_second),edge));
#endif
#endif

       // Initialize the node index --> SgGraphEdge* multimap.
       // printf ("In SgGraph::addEdge(): Insert edge (%d,%d) on node %d \n",node_index_first,node_index_second,node_index_first);
          p_node_index_to_edge_multimap.insert(std::pair<int,SgGraphEdge*>(node_index_first,edge));
        }
       else
        {
          printf ("This node already exists in this graph \n");
          ROSE_ABORT();
        }

     return edge;
   }


SgDirectedGraphEdge*
SgIncidenceDirectedGraph::addDirectedEdge( SgGraphNode* a, SgGraphNode* b, const std::string & name )
   {
     ASSERT_this();

     ROSE_ASSERT( a != NULL && b != NULL ) ;

     SgDirectedGraphEdge* edge = NULL;

     edge = new SgDirectedGraphEdge(a,b,name);

     ROSE_ASSERT(edge != NULL);


     return addDirectedEdge(edge);
   }

// DQ (4/28/2009): Added support to build node (automaticly added to SgGraph).
SgDirectedGraphEdge*
SgIncidenceDirectedGraph::addDirectedEdge( SgDirectedGraphEdge* edge )
   {
     ASSERT_this();

     edge->set_parent(this);


  // This is using the new Graph IR node API.
     if (exists(edge) == false)
        {
          //      std::cerr << " >>>>>>>>>> Edge does not exist .. adding edge : " << edge->get_node_A()->get_name() << 
          //  " - " << edge->get_node_B()->get_name() << std::endl;
#if 0
     rose_graph_integer_edge_hash_multimap::iterator edgeIt = get_node_index_to_edge_multimap_edgesOut().find(edge->get_node_A()->get_index());
     bool matchingEdgeIsNew = (edgeIt == get_node_index_to_edge_multimap_edgesOut().end());
     if (matchingEdgeIsNew == false)
        {
       // printf ("The node has a valid edge, now check the edge's other end \n");
          matchingEdgeIsNew = ( edgeIt->second->get_node_B() != edge->get_node_B());
        }

     if (matchingEdgeIsNew)
        {
#endif
          int edge_index = edge->get_index();
          p_edge_index_to_edge_map[edge_index] = edge;


          ROSE_ASSERT(edge->get_node_A() != NULL);
          ROSE_ASSERT(edge->get_node_B() != NULL);

          int node_index_first  = edge->get_node_A()->get_index();
          int node_index_second = edge->get_node_B()->get_index();

       // Note that this significantly slows down the performance of the new graph support (appears to be about a factor of 10X).
       // Is there a better (faster) way to build the p_node_index_pair_to_edge_multimap? Yes, increase the size of the hash table (DONE).
#if 1

// CH (4/9/2010): Use boost::unordered instead 
//#ifndef _MSCx_VER
#if 1
                  p_node_index_pair_to_edge_multimap.insert(std::pair<std::pair<int,int>,SgGraphEdge*>(std::pair<int,int>(node_index_first,node_index_second),edge));
#else
// tps (12/09/09) Does not work under Windows right now.
//#pragma message ("rose_graph_support.C: multimap does currently not work.")
#endif
                  //     p_node_index_pair_to_edge_multimap.insert(std::pair<std::pair<int,int>,SgGraphEdge*>(std::pair<int,int>(node_index_second,node_index_first),redge));
#endif
                  
       // Initialize the node index --> SgGraphEdge* multimap.
       // printf ("In SgGraph::addEdge(): Insert edge %p = (%d,%d) on node %d (p_node_index_to_edge_multimap size = %" PRIuPTR ") \n",edge,node_index_first,node_index_second,node_index_first,p_node_index_to_edge_multimap.size());

       // p_node_index_to_edge_multimap.insert(std::pair<int,SgGraphEdge*>(node_index_first,edge));
       //   p_node_index_to_edge_multimap.insert(rose_graph_integer_edge_hash_multimap::value_type(node_index_first,edge));
          get_node_index_to_edge_multimap_edgesOut().insert(pair<int,SgDirectedGraphEdge*>(node_index_first,edge));
          get_node_index_to_edge_multimap_edgesIn().insert(pair<int,SgDirectedGraphEdge*>(node_index_second,edge));

       // printf ("    After adding edge (p_node_index_to_edge_multimap size = %" PRIuPTR ") bucket_count() = %" PRIuPTR " \n",p_node_index_to_edge_multimap.size(),p_node_index_to_edge_multimap.bucket_count());

       // Adding the label to the label map (we might decide to not put empty labels into the map).
          if (edge->get_name().empty() == false)
               p_string_to_edge_index_multimap.insert(std::pair<std::string,int>(edge->get_name(),edge_index));

       // Debugging code
       // display_node_index_to_edge_multimap();
        }
       else
        {
          printf ("This edge already exists in this graph \n");
          ROSE_ABORT();
        }
     return edge;
   }

bool SgIncidenceDirectedGraph::removeDirectedEdge( SgDirectedGraphEdge* edge  ) {
    
    if(exists(edge) ) {
        
        int edge_index = edge->get_index();
        p_edge_index_to_edge_map.erase(edge_index);
        
        int node_index_first  = edge->get_node_A()->get_index();

     // DQ (3/25/2017): Eliminated Clange warning about unused variables.
     // int node_index_second = edge->get_node_B()->get_index();
        
        for(rose_graph_integerpair_edge_hash_multimap::const_iterator it = p_node_index_pair_to_edge_multimap.begin() ;
                it != p_node_index_pair_to_edge_multimap.end(); it++) {
            if(it->second == edge) {
                p_node_index_pair_to_edge_multimap.erase(it);
                break;
            }
        }


        for(rose_graph_integer_edge_hash_multimap::const_iterator it = get_node_index_to_edge_multimap_edgesOut().begin() ;
                it != get_node_index_to_edge_multimap_edgesOut().end(); it++) {
            if(it->first == node_index_first && it->second == edge) {
                get_node_index_to_edge_multimap_edgesOut().erase(it);
                break;
            }
        }
        
        for(rose_graph_integer_edge_hash_multimap::const_iterator it = get_node_index_to_edge_multimap_edgesIn().begin() ;
                it != get_node_index_to_edge_multimap_edgesIn().end(); it++) {
            if(it->first == node_index_first && it->second == edge) {
                get_node_index_to_edge_multimap_edgesIn().erase(it);
                break;
            }
        }
        
        
        if(edge->get_name().empty() == false)
            p_string_to_edge_index_multimap.erase(edge->get_name());
        
        edge->set_parent(NULL);
        
        delete edge; 
        return true;
        
    }   
 
    return false;
}

void
SgGraph::display_node_index_to_node_map() const
   {
     printf ("Inside of SgGraph::display_node_index_to_node_multimap(): \n");
// #ifdef ROSE_USE_NEW_GRAPH_NODES
     rose_graph_integer_node_hash_map::const_iterator i = p_node_index_to_node_map.begin();
     while (i != p_node_index_to_node_map.end())
        {
          printf ("   node index: i->first = %d SgGraphNode: i->second = %p = %d \n",i->first,i->second,i->second->get_index());
          i++;
        }
// #endif
   }

void
SgGraph::display_edge_index_to_edge_map() const
   {
     printf ("Inside of SgGraph::display_edge_index_to_edge_map(): \n");
// #ifdef ROSE_USE_NEW_GRAPH_NODES
     rose_graph_integer_edge_hash_map::const_iterator i = p_edge_index_to_edge_map.begin();
     while (i != p_edge_index_to_edge_map.end())
        {
          printf ("   edge index: i->first = %d SgGraphEdge: i->second = %p = %d \n",i->first,i->second,i->second->get_index());
          i++;
        }
// #endif
   }

void
SgGraph::display_node_index_pair_to_edge_multimap() const
   {
     printf ("Inside of SgGraph::display_node_index_pair_to_edge_multimap(): \n");
// #ifdef ROSE_USE_NEW_GRAPH_NODES
     rose_graph_integerpair_edge_hash_multimap::const_iterator i = p_node_index_pair_to_edge_multimap.begin();
     while (i != p_node_index_pair_to_edge_multimap.end())
        {
// CH (4/9/2010): Use boost::unordered instead 
//#ifndef _MSCx_VER
#if 1
// tps (12/08/09) : Does not work under windows:  error C2039: 'first' : is not a member of 'System::UInt32'
                        printf ("   node pair: (i->first.first = %d,i->first.second = %d) SgGraphEdge: i->second = %p = %d \n",i->first.first,i->first.second,i->second,i->second->get_index());
#endif
                  i++;
        }
// #endif
   }

void
SgGraph::display_string_to_node_index_multimap() const
   {
     printf ("Inside of SgGraph::display_string_to_node_index_multimap(): \n");
// #ifdef ROSE_USE_NEW_GRAPH_NODES
     rose_graph_string_integer_hash_multimap::const_iterator i = p_string_to_node_index_multimap.begin();
     while (i != p_string_to_node_index_multimap.end())
        {
          printf ("   string: i->first = %s node: i->second = %d \n",i->first.c_str(),i->second);
          i++;
        }
// #endif
   }

void
SgGraph::display_string_to_edge_index_multimap() const
   {
     printf ("Inside of SgGraph::display_string_to_edge_index_multimap(): \n");
// #ifdef ROSE_USE_NEW_GRAPH_NODES
     rose_graph_string_integer_hash_multimap::const_iterator i = p_string_to_edge_index_multimap.begin();
     while (i != p_string_to_edge_index_multimap.end())
        {
          printf ("   string: i->first = %s edge: i->second = %d \n",i->first.c_str(),i->second);
          i++;
        }
// #endif
   }

void
SgGraph::display_node_index_to_edge_multimap() const
   {
     printf ("Inside of SgGraph::display_node_index_to_edge_multimap(): \n");
// #ifdef ROSE_USE_NEW_GRAPH_NODES
     rose_graph_integer_edge_hash_multimap::const_iterator i = p_node_index_to_edge_multimap.begin();
     while (i != p_node_index_to_edge_multimap.end())
        {
          printf ("   node index: i->first = %d SgGraphEdge: i->second = %p = %d count(i->first) = %" PRIuPTR " \n",i->first,i->second,i->second->get_index(),p_node_index_to_edge_multimap.count(i->first));
          i++;
        }
// #endif
   }

#ifdef ROSE_USING_GRAPH_IR_NODES_FOR_BACKWARD_COMPATABILITY
void display_nodes();
void display_edges();
#endif


void
SgGraph::resize_hash_maps( size_t numberOfNodes, size_t numberOfEdges )
   {
  // This function makes the hash_maps and hash_multimaps internal hash table 
  // be set to an explicit size (the largest prime number greater than the
  // input specified size is used (I think)).  The reduces the hash collisions
  // and improves the performance of usign the hash table.

  // printf ("Inside of SgGraph::resize_hash_maps(numberOfNodes = %" PRIuPTR " numberOfEdges = %" PRIuPTR "): \n",numberOfNodes,numberOfEdges);

// #ifdef ROSE_USE_NEW_GRAPH_NODES
  // For maps and multimaps which deal with edges, make those hash 
  // tables larger by the expected average degree of the nodes.
  // Note that the next larger prime number will be used by the 
  // hash_map and hash_multimap for the internal table size.

// CH (4/9/2010): Use boost::unordered instead 
//#ifdef _MSCx_VER
#if 0
//#pragma message ("WARNING: std::hash_map::resize() function not available in MSVC.")
         printf ("std::hash_map::resize() function not available in MSVC. \n");
         ROSE_ABORT();
#else
     // CH (4/9/2010): boost::unordered_map uses 'rehash' instead of 'resize'  
#if 1    
     p_node_index_to_node_map.rehash(numberOfNodes);
     p_edge_index_to_edge_map.rehash(numberOfEdges);

     p_node_index_to_edge_multimap.rehash(numberOfEdges);
     p_node_index_pair_to_edge_multimap.rehash(numberOfEdges);

     p_string_to_node_index_multimap.rehash(numberOfNodes);
     p_string_to_edge_index_multimap.rehash(numberOfEdges);

#else
     p_node_index_to_node_map.resize(numberOfNodes);
     p_edge_index_to_edge_map.resize(numberOfEdges);

     p_node_index_to_edge_multimap.resize(numberOfEdges);
     p_node_index_pair_to_edge_multimap.resize(numberOfEdges);

     p_string_to_node_index_multimap.resize(numberOfNodes);
     p_string_to_edge_index_multimap.resize(numberOfEdges);
#endif

#endif
// #endif
   }

size_t
SgGraph::memory_usage()
   {
  // This function makes the hash_maps and hash_multimaps internal hash table 
  // be set to an explicit size (the largest prime number greater than the
  // input specified size is used (I think)).  The reduces the hash collisions
  // and improves the performance of using the hash table.

  // printf ("Inside of SgGraph::memory_usage(): \n");

  // For maps and multimaps which deal with edges, make those hash 
  // tables larger by the expected average degree of the nodes.
  // Note that the next larger prime number will be used by the 
  // hash_map and hash_multimap for the internal table size.
     size_t memory_usage = 0;

// #ifdef ROSE_USE_NEW_GRAPH_NODES
     memory_usage += p_node_index_to_node_map.size() * sizeof(rose_graph_integer_node_hash_map::value_type);
     memory_usage += p_edge_index_to_edge_map.size() * sizeof(rose_graph_integer_edge_hash_map::value_type);

     memory_usage += p_node_index_to_edge_multimap.size() * sizeof(rose_graph_integer_edge_hash_multimap::value_type);
     memory_usage += p_node_index_pair_to_edge_multimap.size() * sizeof(rose_graph_integerpair_edge_hash_multimap::value_type);

     memory_usage += p_string_to_edge_index_multimap.size() * sizeof(rose_graph_string_integer_hash_multimap::value_type);
     memory_usage += p_string_to_node_index_multimap.size() * sizeof(rose_graph_string_integer_hash_multimap::value_type);
// #endif

     printf ("Inside of SgGraph::memory_usage(): memory_usage = %" PRIuPTR " \n",memory_usage);

     return memory_usage;
   }


// #ifdef ROSE_USE_NEW_GRAPH_NODES
// Required for Boost Spanning Tree support.
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>

int
source(boost::detail::edge_desc_impl<boost::undirected_tag, long unsigned int>& edge_descriptor, SgGraph&)
   {
     int edge_source_node = edge_descriptor.m_source;
     return edge_source_node;
   }

int
target(boost::detail::edge_desc_impl<boost::undirected_tag, long unsigned int>& edge_descriptor, SgGraph&)
   {
     int edge_target_node = edge_descriptor.m_target;
     return edge_target_node;
   }
// #endif



// std::vector <SgGraph::BoostEdgeDescriptor>
std::vector <SgGraphEdge*>
SgGraph::generateSpanningTree()
   {
     ASSERT_this();

  // return rose_spanning_tree;
     std::vector <SgGraphEdge*> returnEdgeList;



#ifdef ROSE_USING_GRAPH_IR_NODES_FOR_BACKWARD_COMPATABILITY
     ROSE_ABORT();
     // tps , commented this out because it is baded on the old graph structure
     // Dan needs to look at this later
#if 0
     ROSE_ASSERT(get_edges() != NULL);

  // Assemble the representation of the edges for Boost.
     SgGraphEdgeList::iterator edgeIt = get_edges()->get_edges().begin();
     while(edgeIt != get_edges()->get_edges().end())
        {
       // We want the edges to have an index ordering (could be computed or saved, for now we will save it explicitly).
          int i = edgeIt->second->get_node_A()->get_index();
          int j = edgeIt->second->get_node_B()->get_index();

       // Generate the edge vector for MST (required for many BGL algorithms)
          p_boost_edges.push_back(BoostEdgeType(i,j));

       // Use constant edge weight for now (I think that the edge weights are required by the MST algorithm)
          p_boost_edge_weights.push_back(1);

          edgeIt++;
        }

     ROSE_ASSERT(get_nodes() != NULL);

  // DQ (4/29/2009): Boost Graph Type used to represent ROSE graphs within Boost Graph Algorithms.
     typedef boost::adjacency_list < boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, boost::property < boost::edge_weight_t, int > > BoostGraphType;

  // Supporting graph type required by Boost Graph Library.
     typedef boost::graph_traits < BoostGraphType >::edge_descriptor   BoostEdgeDescriptor;

     BoostGraphType rose_BGL_graph(p_boost_edges.begin(), p_boost_edges.end(), p_boost_edge_weights.begin(), get_nodes()->get_nodes().size());

  // If we want to add edge weights then this is now we have to do it.
  // property_map < Graph, edge_weight_t >::type weight = get(edge_weight, g);

  // Result
     std::vector < BoostEdgeDescriptor > rose_spanning_tree;

#if 1
  // printf ("Computing the kruskal_minimum_spanning_tree() \n");
     boost::kruskal_minimum_spanning_tree(rose_BGL_graph, std::back_inserter(rose_spanning_tree));
  // printf ("DONE: Computing the kruskal_minimum_spanning_tree() \n");

     printf ("rose_spanning_tree.size() = %" PRIuPTR " \n",rose_spanning_tree.size());
#endif

#if 0
     std::cout << "Print the edges in the MST:" << std::endl;
     for (std::vector < Edge >::iterator ei = spanning_tree.begin(); ei != spanning_tree.end(); ++ei)
        {
          cout << source(*ei, g) << " <--> " << target(*ei, g)
               << " with weight of " << weight[*ei]
               << endl;

          int s = source(*ei, g);
          int t = target(*ei, g);
          SgGraphNode* nodeA = get_nodes()->get_nodes()[s];
          SgGraphNode* nodeB = get_nodes()->get_nodes()[t];

       // Now to we find the edge?
       // SgGraphEdge* edge = nodeA->get_edges()[s];

       // How to I get the edge that ends at t?

          returnEdgeList.push_back(*ei);
        }
#endif

#else
  // This implementation uses the future ROSE Graph API.
  // ROSE_USING_GRAPH_IR_NODES_FOR_BACKWARD_COMPATABILITY is undefined

  // A BoostEdgeType is a "std::pair<int, int>"
  // A SgBoostEdgeList is a "std::vector<BoostEdgeType>"
     SgBoostEdgeList boost_edges;
     SgBoostEdgeWeightList boost_edge_weights;
     rose_graph_integerpair_edge_hash_multimap::iterator i = p_node_index_pair_to_edge_multimap.begin();
     while (i != p_node_index_pair_to_edge_multimap.end())
        {
       // printf ("Building up the boost_edges (%d,%d) \n",i->first.first,i->first.second);

          boost_edges.push_back(i->first);
          boost_edge_weights.push_back(1);
          i++;
        }

  // DQ (4/29/2009): Boost Graph Type used to represent ROSE graphs within Boost Graph Algorithms.
     typedef boost::adjacency_list < boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, boost::property < boost::edge_weight_t, int > > BoostGraphType;

  // Supporting graph type required by Boost Graph Library.
     typedef boost::graph_traits < BoostGraphType >::edge_descriptor BoostEdgeDescriptor;

     BoostGraphType rose_BGL_graph(p_boost_edges.begin(), p_boost_edges.end(), p_boost_edge_weights.begin(), numberOfGraphNodes());

  // If we want to add edge weights then this is now we have to do it.
  // property_map < Graph, edge_weight_t >::type weight = get(edge_weight, g);

  // Result
     std::vector < BoostEdgeDescriptor > rose_spanning_tree;

#if 1
  // printf ("Computing the kruskal_minimum_spanning_tree() \n");
     boost::kruskal_minimum_spanning_tree(rose_BGL_graph, std::back_inserter(rose_spanning_tree));
  // printf ("DONE: Computing the kruskal_minimum_spanning_tree() \n");

     printf ("rose_spanning_tree.size() = %" PRIuPTR " \n",rose_spanning_tree.size());
#endif

#if 1
     std::cout << "Print the edges in the MST:" << std::endl;
     for (std::vector < BoostEdgeDescriptor >::iterator ei = rose_spanning_tree.begin(); ei != rose_spanning_tree.end(); ++ei)
        {
#if 1
       // Use the source() and target() functions defined above to extract the node index values from the edge.
          int source_node = source(*ei, *this);
          int target_node = target(*ei, *this);

          printf ("rose_spanning_tree: source_node = %d target_node = %d \n",source_node,target_node);
#endif
        }
#endif


// endif for ROSE_USING_GRAPH_IR_NODES_FOR_BACKWARD_COMPATABILITY
#endif

// endif for ROSE_USING_GRAPH_IR_NODES_FOR_BACKWARD_COMPATABILITY
#endif


     return returnEdgeList;
   }




//! Generate a set of SgGraphEdge pointers associated with a given SgGraphNode.
std::set<SgDirectedGraphEdge*>
SgIncidenceDirectedGraph::computeEdgeSetIn( SgGraphNode* node )
   {
     ASSERT_this();
     ROSE_ASSERT(node != NULL);

     std::set<SgDirectedGraphEdge*> returnSet;

  // printf ("In SgIncidenceDirectedGraph::computeEdgeSetIn(node=%p=%d) p_node_index_to_edge_multimap_edgesIn.size() = %" PRIuPTR " \n",node,node->get_index(),p_node_index_to_edge_multimap_edgesIn.size());

     typedef std::pair<rose_graph_integer_edge_hash_multimap::const_iterator,rose_graph_integer_edge_hash_multimap::const_iterator> equal_range_type;
     equal_range_type equal_range_pair = p_node_index_to_edge_multimap_edgesIn.equal_range(node->get_index());

     for (rose_graph_integer_edge_hash_multimap::const_iterator i = equal_range_pair.first; i != equal_range_pair.second; i++)
        {
       // int edge_index = i->second;
       // SgGraphEdge* edge = p_node_index_to_edge_multimap[edge_index];
          SgDirectedGraphEdge* edge = (SgDirectedGraphEdge*) i->second;

          int edge_node_first  = edge->get_node_A()->get_index();
          int edge_node_second = edge->get_node_B()->get_index();

          ROSE_ASSERT(edge_node_first == node->get_index() || edge_node_second == node->get_index());

          //printf ("Building set with edge = %d between nodes (%d,%d) \n",edge->get_index(),edge_node_first,edge_node_second);

          returnSet.insert(edge);
        }

     return returnSet;
   }

std::set<int>
SgIncidenceDirectedGraph::computeEdgeSetIn( int node_index )
   {
     ASSERT_this();

     std::set<int>  returnSet;

     std::set<SgDirectedGraphEdge*> edgeSet = computeEdgeSetIn( p_node_index_to_node_map[node_index] );

     std::set<SgDirectedGraphEdge*>::iterator i = edgeSet.begin();
     while (i != edgeSet.end())
        {
          returnSet.insert((*i)->get_index());
          i++;
        }

     return returnSet;
   }

//! Generate a set of SgGraphEdge pointers associated with a given SgGraphNode.
std::set<SgDirectedGraphEdge*>
SgIncidenceDirectedGraph::computeEdgeSetOut( SgGraphNode* node )
   {
     ASSERT_this();
     ROSE_ASSERT(node != NULL);

     std::set<SgDirectedGraphEdge*> returnSet;

  // printf ("In SgIncidenceDirectedGraph::computeEdgeSetOut(node=%p=%d) p_node_index_to_edge_multimap_edgesOut.size() = %" PRIuPTR " \n",node,node->get_index(),p_node_index_to_edge_multimap_edgesOut.size());

     typedef std::pair<rose_graph_integer_edge_hash_multimap::const_iterator,rose_graph_integer_edge_hash_multimap::const_iterator> equal_range_type;
     equal_range_type equal_range_pair = p_node_index_to_edge_multimap_edgesOut.equal_range(node->get_index());

     for (rose_graph_integer_edge_hash_multimap::const_iterator i = equal_range_pair.first; i != equal_range_pair.second; i++)
        {
       // int edge_index = i->second;
       // SgGraphEdge* edge = p_node_index_to_edge_multimap[edge_index];
          SgDirectedGraphEdge* edge = (SgDirectedGraphEdge*) i->second;

          int edge_node_first  = edge->get_node_A()->get_index();
          int edge_node_second = edge->get_node_B()->get_index();

          ROSE_ASSERT(edge_node_first == node->get_index() || edge_node_second == node->get_index());

          //printf ("Building set with edge = %d between nodes (%d,%d) \n",edge->get_index(),edge_node_first,edge_node_second);

          returnSet.insert(edge);
        }

     return returnSet;
   }

std::set<int>
SgIncidenceDirectedGraph::computeEdgeSetOut( int node_index )
   {
     ASSERT_this();

     std::set<int>  returnSet;

     std::set<SgDirectedGraphEdge*> edgeSet = computeEdgeSetOut( p_node_index_to_node_map[node_index] );

     std::set<SgDirectedGraphEdge*>::iterator i = edgeSet.begin();
     while (i != edgeSet.end())
        {
          returnSet.insert((*i)->get_index());
          i++;
        }

     return returnSet;
   }

