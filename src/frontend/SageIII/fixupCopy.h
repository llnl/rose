// DQ (10/15/2007): This controls the output of debugging information within the fixupCopy() member function of many IR nodes.
// These member functions are used to handle custom IR node specific details of correcting references made by the AST copy 
// mechanism between the original AST and the copy of the AST.
// DQ (11/29/2009): Note that MSVC does not support use of "false" in macros, so use "0" instead.
#define DEBUG_FIXUP_COPY 0
#define DEBUG_FIXUP_COPY_OUTPUT_MAP 0

void outputMap ( SgCopyHelp & help );

// DQ (10/16/2007): This is a macro to simplify the code associated with fixing up data member in the AST copy.
#if DEBUG_FIXUP_COPY
#define FixupCopyDataMemberMacro(IR_node_copy,IR_node_type,get_accessFunctionName,set_accessFunctionName)                \
     ASSERT_this();                                                                                          \
     if (IR_node_copy->get_accessFunctionName() == this->get_accessFunctionName())                                       \
        {                                                                                                                \
          SgCopyHelp::copiedNodeMapTypeIterator i = help.get_copiedNodeMap().find(this->get_accessFunctionName());       \
          printf ("SgCopyHelp::copiedNodeMapTypeIterator i != help.get_copiedNodeMap().end() = %s (using %s()) \n",i != help.get_copiedNodeMap().end() ? "true" : "false",#get_accessFunctionName); \
          if (i != help.get_copiedNodeMap().end())                                                                       \
             {                                                                                                           \
               SgNode* associated_node_copy = i->second;                                                                 \
               ROSE_ASSERT(associated_node_copy != NULL);                                                                \
               IR_node_type* local_copy = is##IR_node_type(associated_node_copy);                                        \
               ROSE_ASSERT(local_copy != NULL);                                                                          \
               printf ("Resetting using %s->%s(local_copy) = %p = %s \n",#IR_node_copy,#set_accessFunctionName,local_copy,local_copy->class_name().c_str()); \
               IR_node_copy->set_accessFunctionName(local_copy);                                                         \
             }                                                                                                           \
          ROSE_ASSERT(IR_node_copy != NULL);                                                                             \
          ROSE_ASSERT(IR_node_copy->get_accessFunctionName() != NULL);                                                   \
          ROSE_ASSERT(this->get_accessFunctionName() != NULL);                                                           \
          ROSE_ASSERT(IR_node_copy->get_accessFunctionName()->variantT() == this->get_accessFunctionName()->variantT()); \
        }                                                                                                                \
       else                                                                                                              \
        {                                                                                                                \
           printf ("%s->%s() = %p != this->%s() = %p (so %s is already set for %p) \n",#IR_node_copy,#get_accessFunctionName,IR_node_copy,#get_accessFunctionName,this,#get_accessFunctionName,IR_node_copy); \
        }
#else
#define FixupCopyDataMemberMacro(IR_node_copy,IR_node_type,get_accessFunctionName,set_accessFunctionName)                \
     if (IR_node_copy->get_accessFunctionName() == this->get_accessFunctionName())                                       \
        {                                                                                                                \
          SgCopyHelp::copiedNodeMapTypeIterator i = help.get_copiedNodeMap().find(this->get_accessFunctionName());       \
          if (i != help.get_copiedNodeMap().end())                                                                       \
             {                                                                                                           \
               SgNode* associated_node_copy = i->second;                                                                 \
               ROSE_ASSERT(associated_node_copy != NULL);                                                                \
               IR_node_type* local_copy = is##IR_node_type(associated_node_copy);                                        \
               ROSE_ASSERT(local_copy != NULL);                                                                          \
               IR_node_copy->set_accessFunctionName(local_copy);                                                         \
             }                                                                                                           \
          ROSE_ASSERT(IR_node_copy->get_accessFunctionName()->variantT() == this->get_accessFunctionName()->variantT()); \
        }
#endif

