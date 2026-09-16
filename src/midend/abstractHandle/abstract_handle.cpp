/*!
 * Generic implementation for abstract handles of language constructs
 */

#include "abstract_handle.h"
#include <utility>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <vector>
#include <map>
#include <cstring>
#include <ROSE_ABORT.h>
#include <ROSE_ASSERT.h>

using namespace std;

/* A local helper function to convert integers to strings */
template <typename T>
static string numberToString ( T x )
{
     std::stringstream ss;
     ss << x;
     return ss.str();
}

namespace AbstractHandle{
  // maintain a map between nodes and handles,
  // used for fast indexing and avoid redundant creation of handles for nodes
  // Should update the map after each creation
  // TODO Do we allow multiple handles for one node?
  std::map<abstract_node*, abstract_handle*> handle_map;

 //construct a handle from a handle string  within the scope of an existing handle
//reverse look up for a handle from a string within a scope specified by phandle
    abstract_handle::abstract_handle(abstract_handle* phandle, const std::string& handle_string)
    {
      init();// avoid garbage
      this->fromString(phandle,handle_string);
    }

  void abstract_handle::init()
  {
    parent_handle= NULL;
    m_node=NULL;
    m_specifier=NULL;
  }
  abstract_handle::abstract_handle()
  {
    init();
  }

  bool isEqual(const source_position &pos1, const source_position &pos2)
  {
    // We allow inexact comparison here since source position information can be only partial
    // So comparison only matters if both pos information have non-empty information
    // Otherwise we treat them as equal.
    bool lineResult = true, colResult = true;
    if ((pos1.line !=0 ) && (pos2.line !=0))
      lineResult = (pos1.line == pos2.line);
    if ((pos1.column !=0 ) && (pos2.column !=0))
      lineResult = (pos1.column== pos2.column);
    return (lineResult && colResult);
    // exact match cannot handle the case that a compiler provides full source position but
    // user only provides a line number.
   // return(( pos1.line == pos2.line )&&(pos1.column == pos2.column ));
  }
  bool isEqual(const source_position_pair &pair1, const source_position_pair &pair2)
  {
    // the line number of the first part (begin position) must match at least.
    // Avoid the situation that empty file info. matches any position info.
    if (pair1.first.line != pair2.first.line)
      return false;
    else
      return (isEqual(pair1.first, pair2.first) && isEqual(pair2.second,pair2.second));
  }

  //Create a handle using the source position as the specifier by default
  // Set parent handle to the file's handle.
  //If source position is not available,then use name or numbering when possible.
  //TODO add label specifiers
  abstract_handle::abstract_handle(abstract_node* node)
  {
    init();
    m_node = node;
    specifier_value_t svalue;
    abstract_node* p_node =NULL;

    if (node->hasSourcePos())
    {
      source_position_pair spair = node->getSourcePos();
      svalue.positions=spair;
      m_specifier = new specifier(e_position,svalue);

      //a file handle is treated as the parent handle here
      //even other nodes may be the real parent
      p_node = node->getFileNode();
      assert(p_node!=NULL); // a node with source pos info. should have a file node
    }
    else if (node->hasName())
    {
      strcpy(svalue.str_v, (node->getName()).c_str());
      m_specifier = new specifier(e_name, svalue);
      p_node  = node->getParent();
    } else // last resort is numbering, which always exists
    {
      p_node  = node->getParent();
      // parent can be NULL here
      svalue.int_v = node->getNumbering(p_node);
      m_specifier = new specifier(e_numbering, svalue);
    }

    //set up upstream parent/file handles
    //for all cases
     if (p_node!=NULL)
     {
       abstract_handle* p_handle = handle_map[p_node];
       if (p_handle==NULL)
       {
         p_handle = new abstract_handle(p_node);
         handle_map[p_node] = p_handle;
       }
       parent_handle = p_handle;
     }
  }

    // Create a handle from an optional parent handle, with type and numbering information
   abstract_handle::abstract_handle(abstract_node* node, specifier_type_t stype, \
           specifier_value_t svalue, abstract_handle* p_handle/*=NULL*/)
    {
      m_node= node;
      m_specifier = new specifier(stype, svalue);
      //parent_handle->set_next_handle(this);
      //will have problem with shared parent handle!!
      handle_map[node]=this;
    // recursively find parent handles if not provided via parameters
     if (p_handle == NULL)
     {
       abstract_node* p_node = m_node->getParent();
       if (p_node)
       {
         p_handle = handle_map[p_node];
         if (p_handle==NULL)
         {
           p_handle = new abstract_handle(p_node,stype);
           handle_map[p_node] = p_handle;
         }
       }
     } // end if p_handle
      parent_handle = p_handle;
    }

  //Create a handle using specified type, automatically fill out the corresponding value
  //Most useful to create a numbering or named specifier within a scope
   abstract_handle::abstract_handle(abstract_node* node, \
                 specifier_type_t stype, abstract_handle* p_handle/*=NULL*/)
   {
     specifier_value_t svalue;
     switch (stype)
     {
       case e_numbering:
         {
           if (p_handle != NULL)
           {
             svalue.int_v = node->getNumbering(p_handle->getNode());
             m_specifier = new specifier (stype, svalue);
           }
           else
           {
             cerr<<"Error: NULL p_handle when trying to create a numbering handle."<<endl;
             ROSE_ABORT ();
           }
           break;
         }
       case e_name:
         strcpy(svalue.str_v,(node->getName()).c_str());
         m_specifier = new specifier (stype, svalue);
         break;
       case e_position://TODO, factor the code into a member function, called by multiple constructors then
         cerr<<"error:please use the default constructor for position specifier"<<endl;
         ROSE_ABORT();
       default:
         cerr<<"error:unrecognized specifier type:"<<stype<<endl;
         ROSE_ABORT();
     }
     m_node = node;
     handle_map[node] = this;
     // recursively find parent handles if not provided via parameters
     if (p_handle == NULL)
     {
       abstract_node* p_node = m_node->getParent();
       if (p_node)
       {
         p_handle = handle_map[p_node];
         if (p_handle==NULL)
         {
           p_handle = new abstract_handle(p_node,stype);
           handle_map[p_node] = p_handle;
         }
       }
     }
     parent_handle = p_handle;
   }

    // the language construct type name
    std::string abstract_handle::get_construct_type_name()
    {
      return m_node->getConstructTypeName();
    }

   //Generate string starting from the top parent handle to the current one.
   std::string abstract_handle::toString()
   {
     std::string result;
     // generate a vector of handles from the current handle to the root parent handle
     std::vector<abstract_handle*> handle_vec;
     abstract_handle* current_handle= this;
     do
     {
       handle_vec.push_back(current_handle);
       current_handle = current_handle->get_parent_handle();
     }
     while (current_handle!=NULL);

      //generate strings for handles from top parent to the current handle
      //using :: as delimiter in between
      vector<abstract_handle*>::reverse_iterator riter;
      for (riter = handle_vec.rbegin(); riter!=handle_vec.rend(); riter++)
      {
        if (riter!=handle_vec.rbegin())
          result +="::";
        result+= (*riter)->toStringSelf();
      }
      return result;
   }

    // Output only the current handle to a string ,excluding any parent handles
    std::string abstract_handle::toStringSelf()
    {
      string result;
      result += get_construct_type_name();
      if (m_specifier!=NULL)
        result += m_specifier->toString();
      return result;
    }

 std::string specifier::toString()
 {
     string result;
      result +="<";
      specifier_type_t stype =  this->get_type();
      specifier_value_t svalue = this->get_value();
     switch (stype)
      {
        case e_name:
         result +="name,";
         result +=svalue.str_v;
          break;
        case e_position:
         result +="position,";
         result +=AbstractHandle::toString(svalue.positions);
         break;
        case e_numbering:
         result +="numbering,";
         result +=numberToString<size_t>(svalue.int_v);
          break;
        case e_int_label:
          result +="intLabel,";
          break;
        case e_str_label:
          result +="stringLabel,";
          break;
        default:
          cerr<<"Unrecognized specifier type:"<<stype<<endl;
          ROSE_ABORT();
      }//end switch
      result+=">";
      return result;
 }

 std::string toString(const source_position &pos)
  {
    std::string result;
    result +=numberToString<size_t>(pos.line);
    if (pos.column!=0)
       result +="."+numberToString<size_t>(pos.column);
    return result;
  }

 std::string toString(const source_position_pair &pospair)
 {
  std::string result;
  result +=toString(pospair.first);
  if (pospair.second.line!=0)
    result +="-"+toString(pospair.second);
  return result;
 }

  // Parse a string (such as 12.10, 13, 0 etc )into source position data structure
  void fromString(source_position& pos, const std::string& input)
  {
    //avoid garbage number here
    pos.line=0;
    pos.column=0;
    istringstream buffer (input);
    if ((buffer >> std::dec >>pos.line).fail())
    {
      //cerr<<"error: fromString(pos,input) when input is:"<<input<<endl;
      //assert(false);// we allow calling this with empty input
      return;
    }

   char dot;
   dot = buffer.get();
   if (dot=='.') // can be extended to other delimiter here
   {
     if ((buffer>>std::dec>>pos.column).fail())
     {
        //cerr<<"warning: no column number is found in fromString(pos,input) when input is:"<<input<<endl;
     }
   }
  }

  // Parse a string into source position pair data structure
  // 1234.34-2134.234
  void fromString(source_position_pair& pos_pair, const std::string &input)
  {
    istringstream buffer(input);
    char part1[256], part2[256];

    buffer.getline(part1,256,'-');
    buffer.getline(part2,256);

    fromString(pos_pair.first,part1);
    fromString(pos_pair.second,part2);
    //cout<<"debug: fromString(pos_pair,input):"<<toString(pos_pair)<<endl;
  }

  // Parse a string into a specifier
  // such as <numbering,1> <position,1.1-1.9> <name,/home/liao6/svnrepos/mycode/rose/abstracthandle/variable.c> etc
  void fromString(specifier& result, const std::string &input)
  {
    istringstream buffer(input);
    char type_str[256], value_str[PATH_MAX+256];

    specifier_type_t stype;
    specifier_value_t svalue;

    // Consume and validate the leading delimiter in every build mode.
    char current = buffer.get();
    if (current != '<')
    {
      cerr << "error, expecting '<' in fromString(specifier&,string)" << endl;
      ROSE_ABORT();
    }
    // get type string and value string
    buffer.getline(type_str,256,',');
    buffer.getline(value_str,PATH_MAX+256, '>');

   // cout<<"type_str:"<<type_str<<endl;
   // cout<<"value_str:"<<value_str<<endl;
    if (strcmp(type_str,"name")==0)
    {
      stype= e_name;
      strcpy(svalue.str_v,value_str);
    } else
    if (strcmp(type_str,"position")==0)
    {
      stype = e_position;
      fromString(svalue.positions,value_str);
    } else
    if (strcmp(type_str,"numbering")==0)
    {
      stype= e_numbering;
      istringstream iss(value_str);
      if ((iss >>std::dec>>svalue.int_v).fail())
      {
        cerr<<"error, expecting a number in fromString(specifier&,string) but get "
           <<value_str<<endl;
        ROSE_ABORT();
      }
    } else
    {
      //TODO labels
      cerr<<"error, unhandled specifier type in fromString(specifier&,string) "<<endl;
      ROSE_ABORT();
    }
    result.set_type(stype);
    result.set_value(svalue);

    //cout<<"debug inside..."<<result.toString()<<endl;
  }

  //------------- abstract_node:default implementation --------------

  std::string abstract_node::getConstructTypeName() const
  {
    return "";
  }
  /*two abstract nodes are the same if both of them point to the same internal node*/

  bool abstract_node::hasSourcePos() const
  {
    return false;
  }

  bool abstract_node::hasName() const
  {
    return false;
  }

  std::string abstract_node::getName() const
  {
    return "";
  }

  abstract_node* abstract_node::getFileNode() const
  {
    return NULL;
  }

  abstract_node* abstract_node::getParent() const
  {
    return NULL;
  }

  abstract_node* abstract_node::findNode(std::string input) const
  {
    assert(input.size()>0);

    istringstream buffer(input);
    char type_str[256], specifier_str[PATH_MAX+512];
    AbstractHandle::specifier mspecifier;

    buffer.getline(type_str,256,'<');
    buffer.unget(); // put back '<'
    buffer.getline(specifier_str,PATH_MAX+512,'>');
    // obtain the specifier first from the specifier string
    fromString(mspecifier,specifier_str);

    // then use the specifier to guide the find
    return findNode(type_str,mspecifier);

  }

  //Delegate this to users unless we provide another interface to query subtree
  abstract_node* abstract_node::findNode(std::string /*construct_type_str*/, specifier /*mspecifier*/) const
  {
    return nullptr;
  }


  std::string abstract_node::getFileName() const
  {
    abstract_node* file_node = getFileNode();
    if (file_node)
      return file_node->getName();
    else
      return "";
  }

  source_position_pair abstract_node::getSourcePos() const
  {
     source_position_pair result;
     result.first = getStartPos();
     result.second= getEndPos();
     return result;
  }

  source_position abstract_node::getStartPos() const
  {
     source_position pos;
     pos.line=0;
     pos.column=0;
     return pos;
  }
  source_position abstract_node::getEndPos() const
  {
     source_position pos;
     pos.line=0;
     pos.column=0;
     return pos;
  }
  std::string abstract_node::toString() const
  {
    return "";
  }

   //Get integer label
  size_t abstract_node::getIntLabel() const
  {
    return 0;
  }
  //Get string label
  std::string abstract_node::getStringLabel() const
  {
    return "";
  }

  size_t abstract_node::getNumbering(const abstract_node* /*another_node*/) const
  {
    return 0;
  }

  //----- abstract handle -------------------------
   //set internal data according a handle item constructName<specifier_type, specifier_value>
   void abstract_handle::fromStringSelf(abstract_handle* p_handle, const string& input)
   {
     assert(p_handle!=NULL);
     assert(input.size()>0);

     abstract_node* root = p_handle->getNode();
     istringstream buffer(input);
     char type_str[256], specifier_str[PATH_MAX+512];

     buffer.getline(type_str,256,'<');
     buffer.unget(); // put back '<'
     buffer.getline(specifier_str,PATH_MAX+512,'>');
     if (m_specifier==NULL)
       m_specifier = new specifier();
     else
     {
       cerr<<"warning:abstract_handle::fromStringSelf(): overwriting existing m_specifier"<<endl;
     }

     // set specifier and node then
     AbstractHandle::fromString((*m_specifier),specifier_str);
     m_node = root->findNode(type_str,*m_specifier);

     handle_map[m_node]=this;
   }

   // Initialize an empty handle from a handle string (including full parent handles)
   // set up parent pointer accordingly
   bool abstract_handle::fromString(abstract_handle* inParent_handle, const std::string& handle_str_full)
    {
      assert(inParent_handle!=NULL);
      assert(handle_str_full.size()>0);
      vector<string> handle_str_vec;
      istringstream buffer(handle_str_full);

      abstract_node* inParent = inParent_handle->getNode();

      char handle_item[256+PATH_MAX+512]; //TODO define max handle length etc in header,
      do {
        buffer.getline(handle_item, 256+PATH_MAX+512, ':');
        handle_str_vec.push_back(string(handle_item));
        // TODO change to a better delimiter! or configurable!
        buffer.get();// skip the second ':'
        //buffer.peek();
      } while (!buffer.eof());

     // cout<<"vec size is "<<handle_str_vec.size()<<endl;

     // set up all parent handles
      for (int i=0;i<(int)(handle_str_vec.size())-1; i++)
      {
        //cout<<handle_str_vec[i]<<endl;
        abstract_node* node = inParent ->findNode(handle_str_vec[i]);
        if(node == NULL) return false;
        //assert (node != NULL);
        if (handle_map[node]==NULL)
        {
          abstract_handle* phandle = new abstract_handle();
          handle_map[node] = phandle;
          phandle->fromStringSelf(inParent_handle,handle_str_vec[i]);
          parent_handle = phandle;
        }
        else
          parent_handle= handle_map[node];

        if (i==0)  // connect the link between the root and the top parent of the string
        {
          if (parent_handle->get_parent_handle() == NULL)
            parent_handle->set_parent_handle(inParent_handle);
        }
      } // end for

      // patch up parent handle, if not yet set
      if (parent_handle == NULL)
        parent_handle=inParent_handle;
      // set up the handle for itself
      fromStringSelf(parent_handle,handle_str_vec.back());
      return true;
    }
}// end namespace
