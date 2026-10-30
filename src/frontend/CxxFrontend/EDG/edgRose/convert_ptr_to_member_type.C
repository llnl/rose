
#include "edgRose.h"

SgType* EDG_ROSE_Translation::convert_ptr_to_member_type(a_type_ptr type)
   {
     SgType* result = NULL;

     ROSE_ASSERT(type->kind == tk_ptr_to_member);

  // DQ (11/6/2011): Adding support for pointers to member functions. Maybe this would be better as a seperate function!.
     a_type_ptr basePointer = type->variant.ptr_to_member.type;

  // DQ (7/22/2007): get the pointer to the associated class
  // a_type_ptr classPointer = type->variant.ptr_to_member.class_of_which_a_member;
  // ROSE_ASSERT(classPointer != NULL);

     ROSE_ASSERT(basePointer != NULL);

#if 0
     printf ("In convert_ptr_to_member_type(): summarize_type_kind_in_string_kind(basePointer->kind = %d) = %s \n",basePointer->kind,summarize_type_kind_in_string_kind(basePointer->kind));
#endif

#if 0
  // DQ (12/12/2011): Older code rewritten below.
  // DQ (12/12/2011): Added test since we don't want to return NULL pinters (fails for test2011_183.C).
     ROSE_ASSERT(basePointer->kind == tk_routine);

  // Everything but function pointers is handled elsewhere, so we need only handle pointers to member functions!
     if (basePointer->kind == tk_routine)
        {
          printf ("Handling the case of a pointer to a member (data member or function member) \n");

       // SgSymbol* existingSymbol = get_symbol_ptr(type->variant.ptr_to_member.class_of_which_a_member->source_corresp);
          a_type_ptr edg_class_type = type->variant.ptr_to_member.class_of_which_a_member;
          ROSE_ASSERT(edg_class_type != NULL);
          SgType* existingType = type_translation_cache[edg_class_type];

          SgType* memberFunctionType = convert_type(basePointer);
          ROSE_ASSERT(memberFunctionType != NULL);

          SgClassType* classType = NULL;

          if (existingType == NULL)
             {
            // specify use of e_class, e_struct, e_union explicitly.
            // SgClassDeclaration* memberFunctionClassDeclarationPointer = convert_struct_secondary(edg_class_type,SgClassDeclaration::e_class);

            // Build the class type (incomplete implementation).

               ROSE_ASSERT(classType != NULL);
             }
            else
             {
               classType = isSgClassType(existingType);
             }
          ROSE_ASSERT(classType != NULL);

       // SgPointerMemberType (SgType *base_type=NULL, SgType *class_type=NULL);
          SgPointerMemberType* pointerToMemberType = new SgPointerMemberType(memberFunctionType,classType);
          ROSE_ASSERT(pointerToMemberType != NULL);

          result = pointerToMemberType;
        }
#else
  // DQ (12/12/2011): Rewrote this code to be more general purpose.
     a_type_ptr edg_class_type = type->variant.ptr_to_member.class_of_which_a_member;
     ROSE_ASSERT(edg_class_type != NULL);
#if 0
     printf ("In convert_ptr_to_member_type(): edg_class_type = %p \n",edg_class_type);
#endif

  // Only reference type_translation_cache[edg_class_type] if we have a valid entry (else this just adds a NULL pointer to the map).
     SgType* existingType = NULL;
     if (type_translation_cache.find(edg_class_type) != type_translation_cache.end())
        {
          existingType = type_translation_cache[edg_class_type];
        }

     if (existingType == NULL)
        {
#if 0
          printf ("In convert_ptr_to_member_type(): ERROR we should have cached this class type --- calling summarize_type() \n");
          summarize_type(edg_class_type);
#endif
#if 0
          printf ("In convert_ptr_to_member_type(): ERROR we should have cached this class type \n");
#endif
       // See test2011_183.C for where a class type has not yet been build, so build one.
          existingType = convert_type(edg_class_type);
          ROSE_ASSERT(existingType != NULL);

       // DQ (8/12/2012): Added test.
          ROSE_ASSERT(type_translation_cache.find(edg_class_type) != type_translation_cache.end());

          ROSE_ASSERT(existingType == type_translation_cache[edg_class_type]);
        }
     ROSE_ASSERT(existingType != NULL);

  // DQ (8/12/2012): Added tests.
     ROSE_ASSERT(type_translation_cache.find(edg_class_type) != type_translation_cache.end());
     ROSE_ASSERT(type_translation_cache[edg_class_type] == existingType);

  // TV (04/10/2018): dealing with nonreal classes
     SgClassType* classType = isSgClassType(existingType);
     SgNonrealType* nrType = isSgNonrealType(existingType);
     ROSE_ASSERT(classType != NULL || nrType != NULL);
     SgDeclarationStatement * decl = NULL;
     if (classType != NULL) decl = classType->get_declaration();
     if (nrType    != NULL) decl = nrType->get_declaration();
     ROSE_ASSERT (decl != NULL);
     ROSE_ASSERT (decl->get_firstNondefiningDeclaration() != NULL);
     ROSE_ASSERT (decl->get_firstNondefiningDeclaration() == decl);

#if 0
     printf ("In convert_ptr_to_member_type(): basePointer = %p \n",basePointer);
#endif

     SgType* memberType = convert_type(basePointer,/* pointerToMember = */ true);
     ROSE_ASSERT(memberType != NULL);
  // printf ("In convert_ptr_to_member_type(): memberType = %p = %s \n",memberType,memberType->class_name().c_str());

#if 0
  // DQ (12/18/2011): 
     if (isSgMemberFunctionType(memberType) == NULL)
        {
          printf ("In convert_ptr_to_member_type(type=%p): ERROR we should have generated a SgMemberFunctionType --- calling summarize_type() \n",type);
          summarize_type(type);
          printf ("In convert_ptr_to_member_type(basePointer=%p): ERROR we should have generated a SgMemberFunctionType --- calling summarize_type() \n",basePointer);
          summarize_type(basePointer);
        }
#endif
  // DQ (12/18/2011): test2011_183.C demonstrates where this need not be a member function (reference to integer member data).
  // ROSE_ASSERT(isSgMemberFunctionType(memberType) != NULL);

     SgPointerMemberType* pointerToMemberType = new SgPointerMemberType(memberType,existingType);
     ROSE_ASSERT(pointerToMemberType != NULL);

     result = pointerToMemberType;
#endif

  // DQ (12/12/2011): Added test since we don't want to return NULL pinters.
     ROSE_ASSERT(result != NULL);

#if 0
     printf ("Leaving convert_ptr_to_member_type(): result = %p \n",result);
#endif

     return result;
   }
