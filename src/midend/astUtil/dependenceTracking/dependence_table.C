#include "dependence_table.h"
#include "AstInterface_ROSE.h"

// Max number of characters where line wrap is inserted by GUI display.
#define MAX_WRAP_STRING 200

namespace {
    std::string  wrap_string (const std::string& s) {
       std::string new_string;
       unsigned wrap = 10, maxwrap = MAX_WRAP_STRING, index = 0;
       for (auto c : s) {
         ++index;
         if (index > maxwrap || (index > wrap && c == ':')) {
             new_string.push_back('\\');
             new_string.push_back('n');
             index = 0;
         } 
         new_string.push_back(c);
       }
       return new_string;
    };
};

namespace AstUtilInterface {

std::ostream& operator << (std::ostream& output, const DependenceEntry& dep) { 
        output << dep.first_entry() << " : " ;
        for (const auto& e : dep.type_entries()) {
            output << "[ " << e << " ] ";
        } 
        output << dep.second_entry() << " ;";
        return output;
}

/************************************/
/* Class for supporting clustering of dependences in GUI */
/************************************/
class ClusterDependences {
    // map each graph node to the name of its cluster.
    std::map<std::string, std::string> cluster_map; 
    // Maps the cluster name to all member nodes that belong to the  cluster.
    std::map<std::string,std::set<std::string> > clusters;
    // Maps each node to all the outgoing and incoming edges.
    std::map<std::string, std::set<DependenceEntry>> out_edge_map, in_edge_map;
    // The same of all nodes that are functions.
    std::set<std::string> functions;
    // the integer keeping track of new clusters created.
    int cluster_index_ = 0;
    DependenceTable& deptable;

    bool setupNamespace(const std::string& s) {
       auto namespace_pos = s.rfind("::");
       if (namespace_pos > 0 && namespace_pos < s.size()) {
           std::string cluster_name = s.substr(0, namespace_pos); 
           clusters[cluster_name].insert(s);
           return true;
       }
       return false;
    }
  public:
     ClusterDependences(DependenceTable& t) : deptable(t) {}
    // Call for each node of the graph to set up clustering.
    void setupNode (const std::string& s) {
       assert(s != "");
       // If s has been clustered previously, return.
       if (cluster_map.find(s) != cluster_map.end()) return;
       // set up preliminary clustering.
       if (setupNamespace(s)) return;

       // Try to cluster node s with itself (nobody else). 
       auto cluster_string = std::to_string(cluster_index_);
       cluster_index_ ++;
       clusters[cluster_string].insert(s);
       cluster_map[s] = cluster_string;
    };

    // Call for each edge of the graph to set up clustering.
    void setupEdge (const DependenceEntry& e) {
       assert(e.first_entry() != "");
       assert(e.second_entry() != "");
       // first entry is always a function.
       functions.insert(e.first_entry());
       if (e.type_entry() == "call") {
          // second entry is a function only if this is a call relation..
          functions.insert(e.second_entry());
       }
       // Group the edges incidenting to the node.
       out_edge_map[e.first_entry()].insert(e);
       in_edge_map[e.second_entry()].insert(e);
    }
    
    // Call to cluster the registered nodes and edges. 
    // Group nodes into single clusters if connected more than #threshold edges.
    void setupClusters (int threshold = 1) { 
       std::list<std::string> work; 
       for (auto m : cluster_map) {
          auto node = m.first;
          assert(m.second != "");
          assert(m.first != "");
          // do nothing if it's already includes other nodes.
          if (clusters[m.second].size() > 1)  continue;
          work.push_back(node);
       }
       for (std::string node : work) {
          std::map <std::string, int> connectivity;
          // Fuse into the edge sink if this is 
          for (auto e : in_edge_map[node]) {
            auto c = cluster_map[e.first_entry()];
            if (c == "") continue; // TODO: Not supposed to happen.
            if (connectivity.find(c) == connectivity.end()) 
               connectivity[c] = 0;
            else 
               connectivity[c] ++;
          }  
          for (auto e : out_edge_map[node]) {
            auto c = cluster_map[e.second_entry()];
            if (c == "") continue; // TODO: not supposed to happen.
            if (connectivity.find(c) == connectivity.end()) 
               connectivity[c] = 0;
            else 
               connectivity[c] ++;
          }  
          std::string cluster = "";
          // If node has only one connection, cluster it.
          if (connectivity.size() == 1) {
            cluster = (*connectivity.begin()).first;
            cluster_map[node] = cluster; 
            clusters[cluster].insert(node);
          } else {
            for (auto con : connectivity) {
              if (con.second >= threshold) {
                 if (cluster == "") {
                    cluster = con.first;
                    assert(cluster != "");
                    cluster_map[node] = cluster; 
                    clusters[cluster].insert(node);
                 } else {
                    for (auto n : clusters[con.first]) {
                       cluster_map[n] = cluster;
                       clusters[cluster].insert(n);
                    }
                    clusters[con.first].clear(); // Clear the old cluster.
           } } } } // else
      } // while
    };
 public:
    std::string edge_to_string (const std::string& s) {
       if (s != ""){
          return "[ label=\"" + s + "\" ]";
       }
       return "";
    };

    // Output clustering of nodes in different namespaces.
    void outputCluster(const std::string& cluster_name, std::ostream& output) {
      bool do_cluster = clusters[cluster_name].size() > 1;
      if (do_cluster) {
        output << "subgraph \"cluster_" << cluster_name << "\" {\n";
        output << "  style=invis;\n";
 //       output << "  style=filled;\n";
 //       output << "  color=lightgrey;\n";
 //       output << "  label=\"" << cluster_name << "\";\n";
      }
      for (auto cluster_member : clusters[cluster_name]) {
        if (functions.find(cluster_member) != functions.end()) {
          output << "\"" << wrap_string(cluster_member) << "\" [shape=box]";
        } else {
          output << "\"" << wrap_string(cluster_member) << "\" [shape=diamond]";
        }
        for (auto attr : deptable.get_nodeInfo(cluster_member)) {
           output << " [ " << attr << " ] ";
        }
        output << ";\n";
      }
      if (do_cluster) {
         output << "}\n";
      }
    };
    // This must be done after all the dependences have been output, which sets up the clusters.
    void output(std::ostream& output) {
     output << "digraph {\n";
       for (auto cluster_entry : clusters) {
         outputCluster(cluster_entry.first, output);
       };
       for (auto m : out_edge_map) {
         for (auto& e : m.second) {
            output << "\"" << wrap_string(e.first_entry()) << "\" -> \"" << wrap_string(e.second_entry()) << "\"" <<  edge_to_string(e.type_entry()) << " ;\n";
         }
       }
     output << "}";
    };
};
/************************************/

void DependenceTable :: OutputDependencesInGUI(std::ostream& output) {
    // Save all the nodes into different clusters based on their namespaces.
    ClusterDependences clusters(*this);
    CollectDependences([&clusters](const DependenceEntry& e) {
       clusters.setupNode(e.first_entry()); 
       clusters.setupNode(e.second_entry()); 
       clusters.setupEdge(e); 
      }); 
    clusters.setupClusters();
    clusters.output(output);
}

void DependenceTable:: ClearOperatorSideEffect(SgNode* op) {
  auto sig = GetVariableSignature(op);
  ClearDependence(sig);
}

bool DependenceTable::
     SaveOperatorSideEffect(SgNode* op, const AstNodePtr& varref, const AstUtilInterface::OperatorSideEffect& relation) {
  std::vector<std::string> rel_sig = relation.relations();
  const auto& op_sig = AstUtilInterface::GetVariableSignature(op);
  DependenceEntry e(op_sig, AstUtilInterface::GetVariableSignature(varref), rel_sig);
  SaveDependence(e);
  return true;
}

void CollectTransitiveDependences:: Compute(
                const std::string& input, 
                std::set<std::string>& result,
                const std::function<bool(const DependenceEntry&)>* what_to_do) {
    DebugLog Log("-debug-dep-table");
    if (result.find(input) != result.end()) {
      // Transitively collect more results only if it hasn't yet been done.
      Log.push("Skip collecting transitive dependence for " + input);
      return;
    }
    Log.push("Collect transitive dependence for " + input);
    result.insert(input);
    DependenceTable::InsertNode(input);
    const auto& dependences = DependenceTable::get_dependences(input);
    // Terminates if dependences are empty, with the loop below skipped.
    for (const auto& dependence : dependences) {
           if (what_to_do != 0 && (*what_to_do)(dependence)) {
              DependenceTable::SaveDependence(dependence);
           }
           if (current_start(dependence) == input) {
              Compute(next_start(dependence), result, what_to_do);
           }
    }
}

void CollectTransitiveDependences :: 
Compute(const std::vector<std::string>& input,
                const std::function<bool(const DependenceEntry&)>* what_to_do,
                std::set<std::string>* result) { 
    DebugLog Log("-debug-dep-table");
    Log.push("Output results of transitive dependence analysis");
    auto from = input;
    for (std::string e : from) {
          std::set<std::string> destinations;
          Compute(e, destinations, what_to_do);
          if (result != 0) {
            for (auto d : destinations) {
              result->insert(d);
            }
          }
    } 
}


};
