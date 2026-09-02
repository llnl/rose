#include "sage3basic.h"
#include <sstream>
#include <iostream>
#include "sageInterface.h"
#include <map>
#include <boost/foreach.hpp>
#include "Rose/AST/Utility.h"
#include <unordered_map>
#include "RoseAst.h"

using namespace std;
using namespace Rose;
#define foreach BOOST_FOREACH

#define DEBUG_QUADRATIC_BEHAVIOR 0

// originally from src/midend/astInlining/typeTraits.C
// src/midend/astUtil/astInterface/AstInterface.C
namespace SageInterface
{
  //! Internal map between SgType and the corresponding type string specific to GASnet
  // support mangled type names according to Itanium C++ ABI
  map <SgType*,string> type_string_map; 
  map <string, SgType*> string_type_map; 

  //! Check if a type is an integral type, only allowing signed/unsigned short, int, long, long long.
  ////!
  ////! There is another similar function named SgType::isIntegerType(), which allows additional types char, wchar, and bool.
  bool isStrictIntegerType(SgType* t)
  {
    ROSE_ASSERT(t != NULL);

    while (t->variant() == T_TYPEDEF)
    {
      t = ((SgTypedefType *) t)->get_base_type();
      ROSE_ASSERT(t != NULL);
    }

    switch (t->variant())
    {
      case T_SHORT:
      case T_SIGNED_SHORT:
      case T_UNSIGNED_SHORT:
      case T_INT:
      case T_SIGNED_INT:
      case T_UNSIGNED_INT:
      case T_LONG:
      case T_SIGNED_LONG:
      case T_UNSIGNED_LONG:
      case T_LONG_LONG:
      case T_SIGNED_LONG_LONG:
      case T_UNSIGNED_LONG_LONG:
        return true; // 1
        break;
      default:
        return false; // 0
    }
  }

  bool isScalarType( SgType* t)
  {
    ROSE_ASSERT(t);
    // We want to strip off typedef chain, and const modifiers etc.
    t = t->stripTypedefsAndModifiers();

    if (isSgFloatingPointType(t) || isSgIntegralType(t)) {
      return true;
    }
    else {
      switch(t->variantT()) {
//        case V_SgTypedefType:
//           return isScalarType(isSgTypedefType(t)->get_base_type());
        case V_SgTypeVoid :
        case V_SgTypeString:
          // DQ (8/27/2006): change name of SgComplex to SgTypeComplex (for consistency) and added SgTypeImaginary (C99 type).
        case V_SgTypeComplex:
        case V_SgTypeImaginary:
          return true;
        default: {}
      }
    }
    return false;
  }


  bool isStructType(SgType* t)
  {
    bool result = false;
    ROSE_ASSERT(t!=NULL);
    SgClassType * class_type = isSgClassType(t);
    if (class_type)
    {
      SgDeclarationStatement* decl = class_type->get_declaration();
      result = isStructDeclaration(decl);
    }
    return result;
  }
    
  bool isUnionType(SgType* t)
  {
      bool result = false;
      ROSE_ASSERT(t!=NULL);
      SgClassType * class_type = isSgClassType(t);
      if (class_type)
      {
          SgDeclarationStatement* decl = class_type->get_declaration();
          result = isUnionDeclaration(decl);
      }
      return result;
  }
    

  SgType* getBaseFromType(SgType* t) {

    SgType* deref = t->dereference();
    if (deref && deref != t)
      return getBaseFromType(deref);
    return t;
  }

  // Is this a const type?
  bool isConstType(SgType* t) {
    //  if (isSgReferenceType(t)) // reference type cannot be cv-qualified usually, 
    //    return isConstType(isSgReferenceType(t)->get_base_type());
    if (isSgTypedefType(t))
      return isConstType(isSgTypedefType(t)->get_base_type());
    if (isSgModifierType(t)) {
      SgTypeModifier& modifier = isSgModifierType(t)->get_typeModifier();
      SgConstVolatileModifier& cv = modifier.get_constVolatileModifier();
      if (cv.isConst())
        return true;
      else
        return isConstType(isSgModifierType(t)->get_base_type());
    }
    return false;
  }

  // Remove a top-level const from the type, if present
  SgType* removeConst(SgType* t) {
    if (isSgTypedefType(t))
      return removeConst(isSgTypedefType(t)->get_base_type());
    if (isSgModifierType(t)) {
      SgModifierType* newT = deepCopy(isSgModifierType(t));
      ROSE_ASSERT (newT);
      SgTypeModifier& modifier = newT->get_typeModifier();
      SgConstVolatileModifier& cv = modifier.get_constVolatileModifier();
      cv.unsetConst();
      return newT;
    }
    return t;
  }

  // Is this a volatile type?
  bool isVolatileType(SgType* t) {
    if (isSgTypedefType(t))
      return isVolatileType(isSgTypedefType(t)->get_base_type());
    if (isSgModifierType(t)) {
      SgTypeModifier& modifier = isSgModifierType(t)->get_typeModifier();
      SgConstVolatileModifier& cv = modifier.get_constVolatileModifier();
      if (cv.isVolatile())
        return true;
      else
        return isVolatileType(isSgModifierType(t)->get_base_type());
    }
    return false;
  }

  // Is this a restrict type?
  bool isRestrictType(SgType* t) {
    if (isSgTypedefType(t))
      return isRestrictType(isSgTypedefType(t)->get_base_type());
    if (isSgModifierType(t)) {
      SgTypeModifier& modifier = isSgModifierType(t)->get_typeModifier();
      if (modifier.isRestrict())
        return true;
      else
        return isRestrictType(isSgModifierType(t)->get_base_type());
    }
    return false;
  }


  // Is this type a non-constant reference type?
  bool isNonconstReference(SgType* t) {
    if (isSgReferenceType(t))
      return !isConstType(isSgReferenceType(t)->get_base_type());
    if (isSgTypedefType(t))
      return isNonconstReference(isSgTypedefType(t)->get_base_type());
    if (isSgModifierType(t)) {
      SgTypeModifier& modifier = isSgModifierType(t)->get_typeModifier();
      SgConstVolatileModifier& cv = modifier.get_constVolatileModifier();
      if (cv.isConst())
        return false;
      else
        return isNonconstReference(isSgModifierType(t)->get_base_type());
    }
    return false;
  }

  // Is this type a constant or non-constant reference type?
  bool isReferenceType(SgType* t) {
    if (isSgReferenceType(t))
      return true;
    if (isSgTypedefType(t))
      return isReferenceType(isSgTypedefType(t)->get_base_type());
    if (isSgModifierType(t)) {
      return isReferenceType(isSgModifierType(t)->get_base_type());
    }
    return false;
  }

  // Is this type a pointer type?
  bool isPointerType(SgType* t) {
    if (isSgPointerType(t))
      return true;
    if (isSgTypedefType(t))
      return isPointerType(isSgTypedefType(t)->get_base_type());
    if (isSgModifierType(t)) {
      return isPointerType(isSgModifierType(t)->get_base_type());
    }
    return false;
  }

bool isPointerToNonConstType(SgType* type)
{
        if (SgTypedefType* typeDef = isSgTypedefType(type))
                return isPointerToNonConstType(typeDef->get_base_type());
        else if (SgPointerType* pointerType = isSgPointerType(type))
                return !SageInterface::isConstType(pointerType->get_base_type());
        else if (SgModifierType* modifierType = isSgModifierType(type))
                return isPointerToNonConstType(modifierType->get_base_type());
        else
                return false;
}

  //! Check if an expression is an array access (SgPntrArrRefExp) . If so, return its name and subscripts if requested. 
  bool isArrayReference(SgExpression* ref, SgExpression** arrayName/*=NULL*/, vector<SgExpression*>** subscripts/*=NULL*/)
  {
    SgExpression* arrayRef=NULL;
    if (ref->variantT() == V_SgPntrArrRefExp) {
      if (subscripts != 0 || arrayName != 0) {
        SgExpression* n = ref;
        while (true) {
          SgPntrArrRefExp *arr = isSgPntrArrRefExp(n);
          if (arr == 0)
            break;
          n = arr->get_lhs_operand();
          // store left hand for possible reference exp to array variable
          if (arrayName!= 0)
            arrayRef = n;
          // right hand stores subscripts
          if (subscripts != 0) // must insert to be the first here !! The last visited rhs will be the first dimension!!
            (*subscripts)->insert( (*subscripts)->begin(),  arr->get_rhs_operand());
        } // end while
        if  (arrayName !=NULL)
        {
          *arrayName = arrayRef;
#if 0 // we decided to delegate this to convertRefToInitializedName()
          ROSE_ASSERT(arrayRef != NULL);
          if (isSgVarRefExp(arrayRef))
            *arrayName = isSgVarRefExp(arrayRef)->get_symbol()->get_declaration();
          else if (isSgDotExp(arrayRef))
          { // just return the parent object name for now 
             // this may not be a good choice,but we do this for collect referenced top variables
              // within outliner  
            SgVarRefExp* lhs = NULL;
            lhs = isSgVarRefExp(isSgDotExp(arrayRef)->get_lhs_operand());
            ROSE_ASSERT(lhs != NULL);
            *arrayName = lhs->get_symbol()->get_declaration();
          }
          else
           {
            cout<<"Unhandled case in isArrayReference():"<<arrayRef->class_name()<<" "
            arrayRef->unparseToString()<<endl;
            ROSE_ABORT();
           }
#endif 
        }
      }
      return true;
    }
    return false;
  }
 
 
  //! Calculate the number of elements of an array type
  size_t getArrayElementCount(SgArrayType* t)
  {
    ROSE_ASSERT(t);
    size_t result=1; 
    SgExpression * indexExp =  t->get_index();

    //strip off THREADS for UPC array with a dimension like dim*THREADS
    if (isUpcArrayWithThreads(t))
    {
      SgMultiplyOp* multiply = isSgMultiplyOp(indexExp);
      ROSE_ASSERT(multiply);
      indexExp = multiply->get_lhs_operand();
    }

    // assume dimension default to 1 if not specified ,such as a[] 
    if ((indexExp == NULL) || isSgNullExpression(indexExp)) 
      result = 1;
    else 
    { 
      //Take advantage of the fact that the value expression is always SgUnsignedLongVal in AST
      SgUnsignedLongVal * valExp = isSgUnsignedLongVal(indexExp);
      SgIntVal * valExpInt = isSgIntVal(indexExp);
      ROSE_ASSERT(valExp || valExpInt); // TODO: return -1 is better ?
      if (valExp)
        result = valExp->get_value(); 
      else 
        result = valExpInt->get_value(); 
    }

    // consider multi dimensional case 
    SgArrayType* arraybase = isSgArrayType(t->get_base_type());
    if (arraybase)
      result = result * getArrayElementCount(arraybase);

    return result;
  } // getArrayElementCount()

  SgType* getArrayElementType(SgType* t)
  {
    ROSE_ASSERT(t);
    SgType* current_type = t->stripType(SgType::STRIP_MODIFIER_TYPE | SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_REFERENCE_TYPE);
    while (isSgArrayType(current_type))
      current_type = isSgArrayType(current_type)->get_base_type()->stripTypedefsAndModifiers();
    if (current_type->variantT() == V_SgTypeString)
      return SgTypeChar::createType();
    if (isSgArrayType(current_type))
    {
      cerr<<"Error in getArrayElementType(): returning an array type for input type:"<<t->class_name()<<endl;
      ROSE_ABORT ();
    }
    return current_type;  
  }

  SgType* getElementType(SgType* t)
  {
    ROSE_ASSERT(t);
    t = t->stripType(SgType::STRIP_MODIFIER_TYPE | SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_REFERENCE_TYPE);
    VariantT vt = t->variantT();
    if (vt == V_SgPointerType)
      return static_cast<SgPointerType *>(t)->get_base_type();
    else if (vt == V_SgArrayType)
      return static_cast<SgArrayType *>(t)->get_base_type();
    else if (vt == V_SgTypeString)
      return SgTypeChar::createType();
    else
      return NULL;
  }

  //! Get the number of dimensions of an array type
  int getDimensionCount(SgType* mytype)
  {
    ROSE_ASSERT(mytype != NULL);
    int dimension = 0;
    SgType* current_type = mytype->stripType(SgType::STRIP_MODIFIER_TYPE | SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_REFERENCE_TYPE);
    while (isSgArrayType(current_type))
    {
      dimension++;
      current_type= isSgArrayType(current_type)->get_base_type()->stripTypedefsAndModifiers();
    }
    if (current_type->variantT() == V_SgTypeString)
      dimension++;
    return dimension;
  }                              
  // Returns true if args is a valid argument type list for decl
  // Requires the exact argument list given -- conversions not handled
  // Good enough for default and copy constructors, but not other functions
  static bool acceptsArguments(SgFunctionDeclaration* decl, 
      std::list<SgType*> args, bool allowDefaultArgs=true) {
    SgInitializedNamePtrList& params = decl->get_args();
    SgInitializedNamePtrList::iterator p = params.begin();
    std::list<SgType*>::iterator a = args.begin();
    for (; p != params.end() && a != args.end(); ++p, ++a) {
      if (getBaseFromType((*p)->get_type()) != getBaseFromType(*a))
        return false; // Type mismatch
      // Good enough for copy constructors, though
    }
     if (p == params.end() && a != args.end())
        return false; // Too many arguments
      
     if(allowDefaultArgs == false) {
          if (p != params.end())
              return false;
      }

      for (; p != params.end(); ++p) {
      if ((*p)->get_initializer() == 0)
        return false; // Non-defaulted parameter after end of arg list
    }
    return true;
  }

  // Is a type default constructible?  This may not quite work properly.
  bool isDefaultConstructible(SgType* type) {
    switch (type->variantT()) {
      case V_SgArrayType:
        return isDefaultConstructible(isSgArrayType(type)->get_base_type());
        break;

      case V_SgModifierType:
        return isDefaultConstructible(isSgModifierType(type)->get_base_type()); 
        break;

      case V_SgClassType: {
        SgClassDeclaration* decl =
          isSgClassDeclaration(isSgClassType(type)->get_declaration());
        assert (decl);
        decl= isSgClassDeclaration(decl->get_definingDeclaration ());
        assert (decl);
        SgClassDefinition* defn = decl->get_definition();
        assert (defn);
        bool hasDefaultConstructor = false, hasAnyConstructor = false;
        // Look for user-defined constructors
        SgDeclarationStatementPtrList& classMembers = defn->get_members();
        for (SgDeclarationStatementPtrList::iterator i = classMembers.begin();
            i != classMembers.end(); ++i) {
          if (isSgFunctionDeclaration(*i)) {
            SgFunctionDeclaration* fundecl = isSgFunctionDeclaration(*i);
            bool thisIsAConstructor = 
              fundecl->get_specialFunctionModifier().isConstructor();
            if (thisIsAConstructor)
              hasAnyConstructor = true;
            bool allArgsAreDefaulted = 
              acceptsArguments(fundecl, std::list<SgType*>());
            if (thisIsAConstructor && allArgsAreDefaulted &&
                (*i)->get_declarationModifier().get_accessModifier().isPublic())
              hasDefaultConstructor = true;
          }
        }
        if (hasDefaultConstructor) return true;
        if (hasAnyConstructor) return false;
        // Return true if all non-static data members are default constructible
        for (SgDeclarationStatementPtrList::iterator i = classMembers.begin();
            i != classMembers.end(); ++i) {
          if (isSgVariableDeclaration(*i)) {
            SgVariableDeclaration* decl = isSgVariableDeclaration(*i);
            if (decl->get_declarationModifier().get_storageModifier().isStatic())
              continue;
            SgInitializedNamePtrList& vars = decl->get_variables();
            for (SgInitializedNamePtrList::iterator j = vars.begin();
                j != vars.end(); ++j) {
              if (isDefaultConstructible((*j)->get_type()))
                return true;
            }
          }
        }
        return false;
      }
        break;

      case V_SgTypedefType:
        return isDefaultConstructible(isSgTypedefType(type)->get_base_type());
        break;

      case V_SgReferenceType:
        return false;
        break;

   // DQ (9/7/2016): Added support for new type now referenced as a result of using new automated generation of builtin functions for ROSE.
      case V_SgTypeSigned128bitInteger:

        // DQ (8/27/2006): Changed name of SgComplex to make it more consistant with other 
        // type names and added SgTypeImaginary IR node (for C99 complex support).
      case V_SgTypeComplex:
      case V_SgTypeImaginary:
      case V_SgEnumType:
      case V_SgPointerType:
      case V_SgPointerMemberType:
      case V_SgTypeBool:
      case V_SgTypeChar:
      case V_SgTypeDefault:
      case V_SgTypeDouble:
      case V_SgTypeFloat:
      case V_SgTypeGlobalVoid:
      case V_SgTypeInt:
      case V_SgTypeLong:
      case V_SgTypeLongDouble:
      case V_SgTypeLongLong:
      case V_SgTypeShort:
      case V_SgTypeSignedChar:
      case V_SgTypeSignedInt:
      case V_SgTypeSignedLong:
      case V_SgTypeSignedShort:
      case V_SgTypeString:
      case V_SgTypeUnsignedChar:
      case V_SgTypeUnsignedInt:
      case V_SgTypeUnsignedLong:
      case V_SgTypeUnsignedLongLong:
      case V_SgTypeUnsignedShort:
      case V_SgTypeVoid:
      case V_SgTypeWchar:

   // DQ (2/16/2018): Adding support for char16_t and char32_t (C99 and C++11 specific types).
      case V_SgTypeChar16:
      case V_SgTypeChar32:

   // PL (9/29/2026): Adding support for new float types
      case V_SgTypeFloat80:
      case V_SgTypeFloat128:
      case V_SgTypeFloat16:
      case V_SgTypeFp16:
      case V_SgTypeBFloat16:
      case V_SgTypeFloat32x:
      case V_SgTypeFloat64x:
      case V_SgTypeFloat32:
      case V_SgTypeFloat64:

        return true;
        break;

      default:
        assert (!"Unknown type in isDefaultConstructible()");
        return true;
    }
  }

  // Is a type copy constructible?  This may not quite work properly.

bool isCopyConstructible(SgType* type)
{
    switch (type->variantT())
    {
        case V_SgArrayType:
            return false; //Liao, array types are not copy constructible, 3/3/2009
            //return isCopyConstructible(isSgArrayType(type)->get_base_type());
            break;

        case V_SgModifierType:
            return isCopyConstructible(isSgModifierType(type)->get_base_type());
            break;

        case V_SgFunctionType:
        case V_SgMemberFunctionType:
        case V_SgTypeEllipse:
        case V_SgDeclType:
            return false;
            break;

        case V_SgClassType:
        {
            SgClassDeclaration* decl = isSgClassDeclaration(isSgClassType(type)->get_declaration());
            ROSE_ASSERT(decl);
            ROSE_ASSERT(decl->get_definingDeclaration());
            // from forward declaration to defining declaration 
            SgClassDefinition* defn = isSgClassDeclaration(decl->get_definingDeclaration())->get_definition();
            ROSE_ASSERT(defn);
            bool hasPublicCopyConstructor = false, hasAnyCopyConstructor = false;
            // Look for user-defined constructors
            SgDeclarationStatementPtrList& classMembers = defn->get_members();
            for (SgDeclarationStatementPtrList::iterator i = classMembers.begin(); i != classMembers.end(); ++i)
            {
                if (isSgFunctionDeclaration(*i))
                {
                    SgFunctionDeclaration* fundecl = isSgFunctionDeclaration(*i);
                    bool thisIsAConstructor = fundecl->get_specialFunctionModifier().isConstructor();
                    bool isCopyConstructor = false;
                    if (fundecl->get_args().size() == 1)
                    {
                        SgType* formalArg = fundecl->get_args().front()->get_type();
                        
                        //This must be a reference to the type or a const reference to the type.
                        //Let's remove the 'const'                     
                        formalArg = formalArg->stripTypedefsAndModifiers();
                        if (isSgReferenceType(formalArg))
                        {
                            SgType* refType = isSgReferenceType(formalArg)->get_base_type();
                            refType = refType->stripTypedefsAndModifiers();
                            isCopyConstructor = (refType == type);
                        }
                    }
                    
                    if (thisIsAConstructor && isCopyConstructor)
                    {
                        hasAnyCopyConstructor = true;
                        if ((*i)->get_declarationModifier().get_accessModifier().isPublic())
                            hasPublicCopyConstructor = true;
                    }
                    
                    //Check if the function is pure virtual. If so we can't copy the class.
                    if (fundecl->get_functionModifier().isPureVirtual())
                    {
                        return false;
                    }
                }
            }
                        
            if (hasPublicCopyConstructor) 
                return true;
            if (hasAnyCopyConstructor) 
                return false;
            
            // Return true if all non-static data members are copy constructible
            for (SgDeclarationStatementPtrList::iterator i = classMembers.begin(); i != classMembers.end(); ++i)
            {
                if (isSgVariableDeclaration(*i))
                {
                    SgVariableDeclaration* decl = isSgVariableDeclaration(*i);
                    if (decl->get_declarationModifier().get_storageModifier().isStatic())
                        continue;
                    SgInitializedNamePtrList& vars = decl->get_variables();
                    for (SgInitializedNamePtrList::iterator j = vars.begin(); j != vars.end(); ++j)
                    {
                        if (!isCopyConstructible((*j)->get_type()))
                            return false;
                    }
                }
            }
            
            //Check that all the bases classes are copy constructible
            foreach (SgBaseClass* baseClass, defn->get_inheritances())
            {
                SgClassDeclaration* baseClassDecl = baseClass->get_base_class();
                if (!isCopyConstructible(baseClassDecl->get_type()))
                    return false;
            }
            
            return true; // If no non-copy constructible data member, return true.
        }
            break;

        case V_SgTypedefType:
            return isCopyConstructible(isSgTypedefType(type)->get_base_type());
            break;

     // DQ (9/7/2016): Added support for new type now referenced as a result of using new automated generation of builtin functions for ROSE.
        case V_SgTypeSigned128bitInteger:

        case V_SgReferenceType:
            // DQ (8/27/2006): Changed name of SgComplex to make it more consistent with other 
            // type names and added SgTypeImaginary IR node (for C99 complex support).
        case V_SgTypeComplex:
        case V_SgTypeImaginary:
        case V_SgEnumType:
        case V_SgPointerType:
        case V_SgPointerMemberType:
        case V_SgTypeBool:
        case V_SgTypeChar:
        case V_SgTypeDefault:
        case V_SgTypeDouble:
        case V_SgTypeFloat:
        case V_SgTypeGlobalVoid:
        case V_SgTypeInt:
        case V_SgTypeLong:
        case V_SgTypeLongDouble:
        case V_SgTypeLongLong:
        case V_SgTypeNullptr:
        case V_SgTypeShort:
        case V_SgTypeSignedChar:
        case V_SgTypeSignedInt:
        case V_SgTypeSignedLong:
        case V_SgTypeSignedShort:
        case V_SgTypeString:
        case V_SgTypeUnsignedChar:
        case V_SgTypeUnsignedInt:
        case V_SgTypeUnsignedLong:
        case V_SgTypeUnsignedLongLong:
        case V_SgTypeUnsignedShort:
        case V_SgTypeVoid:
        case V_SgTypeWchar:

     // DQ (2/16/2018): Adding support for char16_t and char32_t (C99 and C++11 specific types).
        case V_SgTypeChar16:
        case V_SgTypeChar32:

     // PL (9/29/2026): Adding support for new float types
        case V_SgTypeFloat80:
        case V_SgTypeFloat128:
        case V_SgTypeFloat16:
        case V_SgTypeFp16:
        case V_SgTypeBFloat16:
        case V_SgTypeFloat32x:
        case V_SgTypeFloat64x:
        case V_SgTypeFloat32:
        case V_SgTypeFloat64:

            return true;
            break;

        default:
            std::cerr << "In SageInterface::isCopyConstructible(), type is a " << type->class_name() << std::endl;
            ROSE_ASSERT(!"Unknown type in isCopyConstructible()");
    }
    return true;
}


  // Is a type assignable?  This may not quite work properly.
  // Liao, 3/3/2009 based on the code for isCopyConstructible()
  // TYPE a, b; is a=b; allowed ?
  bool isAssignable(SgType* type) {
    switch (type->variantT()) {
      case V_SgArrayType:
        return false; 
        break;

      case V_SgModifierType:
        return isAssignable(isSgModifierType(type)->get_base_type()); 
        break;

      case V_SgFunctionType:
      case V_SgMemberFunctionType:
      case V_SgReferenceType: //I think C++ reference types cannot be reassigned. 
      case V_SgTypeEllipse:
      case V_SgDeclType:
        return false;
        break;

      case V_SgClassType: {
        SgClassDeclaration* decl = isSgClassDeclaration(isSgClassType(type)->get_declaration());
        assert (decl);
        assert (decl->get_definingDeclaration());
        // from forward declaration to defining declaration 
        SgClassDefinition* defn = isSgClassDeclaration(decl->get_definingDeclaration())->get_definition();
        assert (defn);

        bool hasPublicAssignOperator= false, hasAnyAssignOperator = false;
        // Look for user-defined operator=
        SgDeclarationStatementPtrList& classMembers = defn->get_members();
        for (SgDeclarationStatementPtrList::iterator i = classMembers.begin();
            i != classMembers.end(); ++i) {
          if (isSgFunctionDeclaration(*i)) {
            SgFunctionDeclaration* fundecl = isSgFunctionDeclaration(*i);
            bool thisIsAnAssignOperator= 
              (fundecl->get_specialFunctionModifier().isConstructor())&&
              (fundecl->get_name().getString()=="operator=");
            // check parameter list has TYPE&      
            std::list<SgType*> args;
            args.push_back(new SgReferenceType(type));
            bool isAssignOperator= acceptsArguments(fundecl, args);
            if (thisIsAnAssignOperator&& isAssignOperator) {
              hasAnyAssignOperator= true;
              if ((*i)->get_declarationModifier().
                  get_accessModifier().isPublic())
                hasPublicAssignOperator= true;
            }
          }
        }
        if (hasPublicAssignOperator) return true;
        if (hasAnyAssignOperator) return false;
        // Return true if all non-static data members are assignable
        for (SgDeclarationStatementPtrList::iterator i = classMembers.begin();
            i != classMembers.end(); ++i) {
          if (isSgVariableDeclaration(*i)) {
            SgVariableDeclaration* decl = isSgVariableDeclaration(*i);
            if (decl->get_declarationModifier().get_storageModifier().isStatic())
              continue;
            SgInitializedNamePtrList& vars = decl->get_variables();
            for (SgInitializedNamePtrList::iterator j = vars.begin();
                j != vars.end(); ++j) {
              if (!isAssignable((*j)->get_type()))
                return false;
            }
          }
        }
        return true; // If no non-assignable data member, return true.
      }
        break;

      case V_SgTypedefType:
        return isAssignable(isSgTypedefType(type)->get_base_type());
        break;

     // DQ (9/7/2016): Added support for new type now referenced as a result of using new automated generation of builtin functions for ROSE.
        case V_SgTypeSigned128bitInteger:

      case V_SgTypeComplex: // C99 complex is assignable
      case V_SgTypeImaginary:
      case V_SgEnumType:
      case V_SgPointerType:
      case V_SgPointerMemberType:
      case V_SgTypeBool:
      case V_SgTypeChar:
      case V_SgTypeDefault:
      case V_SgTypeDouble:
      case V_SgTypeFloat:
      case V_SgTypeGlobalVoid:
      case V_SgTypeInt:
      case V_SgTypeLong:
      case V_SgTypeLongDouble:
      case V_SgTypeLongLong:
      case V_SgTypeNullptr:
      case V_SgTypeShort:
      case V_SgTypeSignedChar:
      case V_SgTypeSignedInt:
      case V_SgTypeSignedLong:
      case V_SgTypeSignedShort:
      case V_SgTypeString:
      case V_SgTypeUnsignedChar:
      case V_SgTypeUnsignedInt:
      case V_SgTypeUnsignedLong:
      case V_SgTypeUnsignedLongLong:
      case V_SgTypeUnsignedShort:
      case V_SgTypeVoid:
      case V_SgTypeWchar:

   // DQ (2/16/2018): Adding support for char16_t and char32_t (C99 and C++11 specific types).
      case V_SgTypeChar16:
      case V_SgTypeChar32:

   // PL (9/29/2026): Adding support for new float types
      case V_SgTypeFloat80:
      case V_SgTypeFloat128:
      case V_SgTypeFloat16:
      case V_SgTypeFp16:
      case V_SgTypeBFloat16:
      case V_SgTypeFloat32x:
      case V_SgTypeFloat64x:
      case V_SgTypeFloat32:
      case V_SgTypeFloat64:

        return true;
        break;

      default:
        std::cerr << "In SageInterface::isAssignable(), type is a " << type->class_name() << std::endl;
        assert (!"Unknown type in isAssignable()");
    }
    return true;
  }

  // Does a type have a trivial (built-in) destructor?
  bool hasTrivialDestructor(SgType* t) {
    switch (t->variantT()) {

     // DQ (9/7/2016): Added support for new type now referenced as a result of using new automated generation of builtin functions for ROSE.
        case V_SgTypeSigned128bitInteger:

      // DQ (8/27/2006): Changed name of SgComplex to make it more consistant with other 
      // type names and added SgTypeImaginary IR node (for C99 complex support).
      case V_SgTypeComplex:
      case V_SgTypeImaginary:
      case V_SgPointerType:
      case V_SgReferenceType:
      case V_SgTypeBool:
      case V_SgTypeChar:
      case V_SgTypeDouble:
      case V_SgTypeEllipse:
      case V_SgTypeFloat:
      case V_SgTypeGlobalVoid:
      case V_SgTypeInt:
      case V_SgTypeLong:
      case V_SgTypeLongDouble:
      case V_SgTypeLongLong:
      case V_SgTypeShort:
      case V_SgTypeSignedChar:
      case V_SgTypeSignedInt:
      case V_SgTypeSignedLong:
      case V_SgTypeSignedShort:
      case V_SgTypeUnsignedChar:
      case V_SgTypeUnsignedInt:
      case V_SgTypeUnsignedLong:
      case V_SgTypeUnsignedLongLong:
      case V_SgTypeUnsignedShort:
      case V_SgTypeVoid:
      case V_SgTypeWchar:

   // DQ (2/16/2018): Adding support for char16_t and char32_t (C99 and C++11 specific types).
      case V_SgTypeChar16:
      case V_SgTypeChar32:

  // PL (9/29/2026): Adding support for new float types
      case V_SgTypeFloat80:
      case V_SgTypeFloat128:
      case V_SgTypeFloat16:
      case V_SgTypeFp16:
      case V_SgTypeBFloat16:
      case V_SgTypeFloat32x:
      case V_SgTypeFloat64x:
      case V_SgTypeFloat32:
      case V_SgTypeFloat64:

      case V_SgFunctionType:
      case V_SgTypeString:
      case V_SgEnumType:
        return true;

      case V_SgArrayType:
        return hasTrivialDestructor(isSgArrayType(t)->get_base_type());

      case V_SgModifierType: 
        return hasTrivialDestructor(isSgModifierType(t)->get_base_type());

      case V_SgTypedefType:
        return hasTrivialDestructor(isSgTypedefType(t)->get_base_type());

      default: 
        SgClassType* ct = isSgClassType(t);
        if (!ct) return false;
        SgDeclarationStatement* decl = ct->get_declaration();
        if (isSgClassDeclaration(decl)) {
          SgClassDeclaration* cd = isSgClassDeclaration(decl);
          SgClassDefinition* cdef = cd->get_definition();
          if (!cdef) return false;
          bool hasDestructor = false;
          SgDeclarationStatementPtrList& decls = cdef->get_members();
          for (SgDeclarationStatementPtrList::iterator i = decls.begin();
              i != decls.end(); ++i) {
            if (isSgFunctionDeclaration(*i)) {
              SgFunctionDeclaration* fundecl = isSgFunctionDeclaration(*i);
              if (fundecl->get_specialFunctionModifier().isDestructor())
                hasDestructor = true;
            }
          }
          if (hasDestructor)
            return false;
          else
            return true;
        } else if (isSgFunctionDeclaration(decl)) {
          return true;
        } else if (isSgEnumDeclaration(decl)) {
          return true;
        } else {
          return false;
        }
    }
  }

  bool isUpcSharedModifierType (SgModifierType* mod_type)
  {
    ROSE_ASSERT(mod_type);
    if (mod_type->get_typeModifier().get_upcModifier().get_isShared())
      return true;
    else 
      return false;
  }

  bool isUpcSharedArrayType (SgArrayType* array_type)
  {
    bool result = false;
    ROSE_ASSERT(array_type);
    SgType* base_type=getArrayElementType(array_type); 
    if (isSgModifierType(base_type))
      result = isUpcSharedModifierType(isSgModifierType(base_type));
    return result;
  }

  bool isUpcStrictSharedModifierType(SgModifierType* mod_type)
  {

    ROSE_ASSERT(isUpcSharedModifierType(mod_type));
    bool result = false;
    result = mod_type->get_typeModifier().get_upcModifier().isUPC_Strict();
    return result;
  }

  //! Get the block size of a UPC shared type, including Modifier types and array of modifier types (shared arrays)
  size_t getUpcSharedBlockSize(SgType* t)
  {
    SgModifierType * mod_type=NULL;  
    bool isUpc = isUpcSharedType(t,&mod_type);
    ROSE_ASSERT(isUpc);
    return getUpcSharedBlockSize(mod_type);
  }

  size_t getUpcSharedBlockSize(SgModifierType* mod_type)
  {
    ROSE_ASSERT(isUpcSharedModifierType(mod_type));

    int result =  mod_type->get_typeModifier().get_upcModifier().get_layout();
    if (result == -1 ) // unspecified block size is treated as size 1
      result = 1;
    return result;
  }

  //! Check if a type is a UPC shared type, including shared array, shared pointers etc.
  bool isUpcSharedType(SgType* t, SgModifierType ** mod_type_out/* = NULL*/)
  {
    return (hasUpcSharedType(t,mod_type_out) && !isUpcPrivateToSharedType(t));
  }

  //! Has a UPC shared type?
  bool hasUpcSharedType(SgType* sg_type,SgModifierType ** mod_type_out/*=NULL*/)
  {
    ROSE_ASSERT(sg_type);
    bool result = false;

    if (isSgModifierType(sg_type))
    {
      SgModifierType * mod_type = isSgModifierType(sg_type);
      if (isUpcSharedModifierType(mod_type))
      {  
        if (mod_type_out) *mod_type_out = mod_type;
        result = true;
      }  
      else 
        result = hasUpcSharedType( (isSgModifierType(sg_type))->get_base_type(), mod_type_out);
    }  
    else if (isSgPointerType(sg_type))
      result = hasUpcSharedType( (isSgPointerType(sg_type))->get_base_type(),mod_type_out );
    else if (isSgReferenceType(sg_type))
      result = hasUpcSharedType( (isSgReferenceType(sg_type))->get_base_type(),mod_type_out );
    else if (isSgArrayType(sg_type))
      result = hasUpcSharedType( (isSgArrayType(sg_type))->get_base_type(),mod_type_out );
    else if (isSgTypedefType(sg_type))
      result = hasUpcSharedType( (isSgTypedefType(sg_type))->get_base_type(), mod_type_out);

    if ((result == false) &&(mod_type_out))  *mod_type_out = NULL;
    return result;
  } //hasUpcSharedType

  //! Is UPC private-to-shared type? Judge the order of SgPointer and SgUPC_AccessModifier
  bool isUpcPrivateToSharedType(SgType* t)
  {
    bool check = hasUpcSharedType(t);
    if (check==false) return false;
    bool pointerFirst = false;
    SgType* currentType = t;

    while (true) 
    {
      if (isSgModifierType(currentType))
      {
        SgModifierType * mod_type = isSgModifierType(currentType);
        if (mod_type->get_typeModifier().get_upcModifier().get_isShared())
          break; //Reaches SgUPC_AccessModifier,exit the while loop
        else 
          currentType = isSgModifierType(currentType)->get_base_type();
      } // if SgModifierType
      else if (isSgPointerType(currentType))
      {
        pointerFirst = true; // reach pointer before reaching UPC modifier!
        currentType = isSgPointerType(currentType)->get_base_type();
      } // if Pointer
      else if (isSgReferenceType(currentType))
        currentType = isSgReferenceType(currentType)->get_base_type();
      else if (isSgArrayType(currentType))
        currentType =  isSgArrayType(currentType)->get_base_type();
      else if (isSgTypedefType(currentType))
        currentType =  isSgTypedefType(currentType)->get_base_type();
    } // while

    return pointerFirst;
  }// isUpcPrivateToSharedType()

  //! Phase-less means block size ==1, or 0, or unspecified(-1). 
  bool isUpcPhaseLessSharedType (SgType* t)
  {
    SgModifierType * mod_type_out;
    int block_size;

    bool check = hasUpcSharedType(t,&mod_type_out);
    //ROSE_ASSERT(hasUpcSharedType(t,mod)); // avoid side effect for assertion!!
    if (check==false) 
      return false;
    else
      ROSE_ASSERT(check&&mod_type_out); 
    block_size =  mod_type_out->get_typeModifier().get_upcModifier().get_layout();
    // cout<<"block size is "<<block_size<<endl;
    if ((block_size==1)||(block_size == -1)|| (block_size == 0))
      return true;
    else 
      return false;
  } // isUpcPhaseLessSharedType

  //! Is a UPC array with dimension of X*THREADS
  /*!
   * EDG-SAGE connection ensures that UPC array using THREADS dimension 
   *  has an index expression of type SgMultiplyOp
   * and operands (X and SgUpcThreads)
   * TODO multi dimensional arrays
   */
  bool isUpcArrayWithThreads(SgArrayType* t)
  {
    ROSE_ASSERT(t!=NULL);
    bool result = false;
    SgExpression * exp = t->get_index();
    SgMultiplyOp* multiply = isSgMultiplyOp(exp);

  // DQ (7/22/2014): Fixed to remove compiler warning.
  // if (multiply)
     if (multiply != NULL)
        {
          if (isSgUpcThreads(multiply->get_rhs_operand()))
             {
               result = true;
             }
            else
             {
            // DQ (9/26/2011): Added else case to handle static compilation of UPC threads to be integer values.
               if (isSgIntVal(multiply->get_rhs_operand()))
                  {
                    result = true;
                  }
             }
       }

    return result;  
  }

  SgType* getFirstVarType(SgVariableDeclaration* decl)
  {
    ROSE_ASSERT(decl);
    SgVariableSymbol * sym = getFirstVarSym(decl);
    ROSE_ASSERT(sym);
    return sym->get_type();
  }

  //! Generate a type string for a given type 
  //TODO put it into sageInterface when ready 
  string mangleType(SgType* type)
  {
    string result;
    result = type_string_map[type];
    if (result.empty())
    {
      if (isScalarType(type))
        result = mangleScalarType(type);
      else if (isSgModifierType(type))  
        result = mangleModifierType(isSgModifierType(type));
      else if (isSgPointerType(type))  
        result = 'P' + mangleType(isSgPointerType(type)->get_base_type());
      else if (isSgReferenceType(type))  
        result = 'R' + mangleType(isSgPointerType(type)->get_base_type());
      //rvalue reference is not yet supported in EDG/ROSE/GCC  
      else if (isSgTypedefType(type))
        result = mangleType(isSgTypedefType(type)->get_base_type());
      else if (isSgArrayType(type))  
      {  
        size_t element_count = getArrayElementCount(isSgArrayType(type));
        stringstream ss;
        ss<<element_count;
        string len=ss.str();
        if (isUpcArrayWithThreads(isSgArrayType(type))) len = len+"H";
        result = "A" + len + "_" + mangleType(isSgArrayType(type)->get_base_type());
      }  
      else
      {  
        //TODO function-type, class-enum-type, pointer-to-member-type, template-param, etc
        cerr<<"Unhandled type mangling for "<<type->sage_class_name()<<endl;
        result = "*Unhandled*";
      }

      type_string_map[type] =  result;
      if (string_type_map[result] == 0)
        string_type_map[result] = type; 
    }     // end if
    return result;

  } // mangeType()
  //! Mangle builtin scalar types
  /*
   * isScalarType() return true for:
   *   char, short, int, long , void, Wchar, Float, double, 
   *   long long, string, bool, complex, imaginary, 
   *   including their signed and unsigned variants, if any.
   *
   * Refer to http://www.codesourcery.com/public/cxx-abi/abi.html#mangling
   *  
   */
  string mangleScalarType(SgType* type)
  { 
    string result;
    ROSE_ASSERT(isScalarType(type)); //? Not exact match to specification

    switch (type->variantT())
    {
      /* Keep the order as the same as the specification */
      case V_SgTypeVoid :
        result += "v";
        break;
      case V_SgTypeWchar:
        result += "w";
        break;

   // DQ (2/16/2018): Adding support for char16_t and char32_t (C99 and C++11 specific types).
      case V_SgTypeChar16:
        result += "c16";
        break;

      case V_SgTypeChar32:
        result += "c32";
        break;

      case V_SgTypeBool:
        result += "b";
        break;
      case V_SgTypeChar :
        result += "c";
        break;
      case V_SgTypeSignedChar :
        result += "a";
        break;
      case V_SgTypeUnsignedChar :
        result += "h";
        break;
      case V_SgTypeShort :
      case V_SgTypeSignedShort: // ? 
        result += "s";
        break;
      case V_SgTypeUnsignedShort:
        result += "t";
        break;
      case V_SgTypeInt:
      case V_SgTypeSignedInt : //?
        result += "i";
        break;
      case V_SgTypeUnsignedInt :
        result += "j";
        break;
      case V_SgTypeLong :
      case V_SgTypeSignedLong :
        result += "l";
        break;
      case V_SgTypeUnsignedLong :
        result += "m";
        break;
      case V_SgTypeLongLong:
        result += "x";
        break;
      case V_SgTypeUnsignedLongLong:
        result += "y";
        break;
        // unsigned int128 // not exist in ROSE
      case V_SgTypeFloat:
        result += "f";
        break;
      case V_SgTypeDouble:
        result += "d";
        break;
      case V_SgTypeLongDouble:
        result += "e";
        break;
        /* Missing in ROSE
           n  # __int128
           o  # unsigned __int128
           g  # float 128, 
           z  # ellipsis
           Dd # IEEE 754r decimal floating point (64 bits)
           De # IEEE 754r decimal floating point (128 bits)
           Df # IEEE 754r decimal floating point (32 bits)
           Dh # IEEE 754r half-precision floating point (16 bits)
           u <source-name>      # vendor extended type
           */
      case V_SgTypeComplex:
        result += "C";
        break;
      case V_SgTypeImaginary:
        result += "G";
        break;
      case V_SgTypeString: //? TODO Not for char*: array of char ? or pointer to char ?
        // only for SgStringVal
        result = ".unhandledSgTypeString." ;
        break; 
      default:
        result = "Invalid.mtype"; //?
    } // switch

    return result;
  }
  //! <CV-qualifiers> ::= [r] [V] [K]   # restrict (C99), volatile, const
  // the order of handling const, volatile, and restric matters
  // Quote of the specification:
  // "In cases where multiple order-insensitive qualifiers are present, 
  // they should be ordered 'K' (closest to the base type), 'V', 'r', and 'U' (farthest from the base type)"
  // also UPC shared type
  string mangleModifierType(SgModifierType* type)
  {
    string result;
    if (isConstType(type))
      result = "K" ; 
    if (isVolatileType(type))
      result = "V" + result;
    if (isRestrictType(type))
      result = "r" + result;
    if (isUpcSharedModifierType(type))
    {
      size_t blocksize = getUpcSharedBlockSize(type);
      stringstream ss;
      ss << blocksize;
      result = ss.str() + "_" + result;
      if (isUpcStrictSharedModifierType(type))
        result = "S" + result;
      else
        result = "R" + result;
    }
    return result + mangleType(type->get_base_type());  
  }


SgType*
getAssociatedTypeFromFunctionTypeList(SgExpression* actual_argument_expression)
   {
  // DQ (7/22/2014): Added support for comparing expression types in actual arguments 
  // with those expected from the formal function parameter types.
  // Get the type of the associated argument expression from the function type.
     ROSE_ASSERT(actual_argument_expression != NULL);

#if 0
     printf ("In SageInterface::getAssociatedTypeFromFunctionTypeList(): actual_argument_expression = %p = %s \n",actual_argument_expression,actual_argument_expression->class_name().c_str());
#endif

     SgExprListExp* exprListExp = isSgExprListExp(actual_argument_expression->get_parent());
     if (exprListExp == NULL)
        {
          printf ("Error: input actual_argument_expression = %p = %s is not a direct argument of a function argument list \n",actual_argument_expression,actual_argument_expression->class_name().c_str());
        }
     ROSE_ASSERT(exprListExp != NULL);

     int index = 0;
     SgExpressionPtrList & expressionList = exprListExp->get_expressions();
     SgExpressionPtrList::iterator i = expressionList.begin(); 

  // Generate the associated index into the array of function parameters.
     while (*i != actual_argument_expression && i != expressionList.end())
        {
          index++;
          i++;
        }
     ROSE_ASSERT(i != expressionList.end());

  // Get the associated type list.
     SgFunctionCallExp* functionCallExp = isSgFunctionCallExp(exprListExp->get_parent());
     ROSE_ASSERT(functionCallExp != NULL);

  // Note that the result of functionCallExp->get_function() can sometime not be a SgFunctionRefExp, 
  // e.g. when called from a pointer to a function.  These cases are not handled.
     SgExpression* tmp_exp = functionCallExp->get_function();
     ROSE_ASSERT(tmp_exp != NULL);

     SgType* tmp_functionType = NULL;
     SgFunctionRefExp* functionRefExp = isSgFunctionRefExp(tmp_exp);
     if (functionRefExp != NULL)
        {
       // This is the most common case.
          ROSE_ASSERT(functionRefExp != NULL);
          tmp_functionType = functionRefExp->get_type();
        }
       else
        {
          SgPointerDerefExp* pointerDerefExp = isSgPointerDerefExp(tmp_exp);
          if (pointerDerefExp != NULL)
             {
            // This is the next most common case.
               tmp_functionType = pointerDerefExp->get_type();
#if 0
               printf ("tmp_functionType = %p = %s = %s \n",tmp_functionType,tmp_functionType->class_name().c_str(),tmp_functionType->unparseToString().c_str());
#endif
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ABORT();
#endif
             }
            else
             {
               tmp_functionType = NULL;

               printf ("Error: this is a case of a indirect reference to a function (not yet handled in SageInterface::getAssociatedTypeFromFunctionTypeList()) tmp_exp = %p = %s \n",tmp_exp,tmp_exp->class_name().c_str());
               functionCallExp->get_file_info()->display("Error: functionRefExp == NULL: functionCallExp: debug");
             }
        }

     ROSE_ASSERT(tmp_functionType != NULL);

     SgFunctionType* functionType = isSgFunctionType(tmp_functionType);
     ROSE_ASSERT(functionType != NULL);

     SgTypePtrList & typeList = functionType->get_arguments();

     SgType* indexed_function_parameter_type = typeList[index];
     ROSE_ASSERT(indexed_function_parameter_type != NULL);

     return indexed_function_parameter_type;
   }


#ifndef ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT
bool isPureVirtualClass(SgType* type, const ClassHierarchyWrapper& classHierarchy)
{
    SgClassType* classType = isSgClassType(type);
    if (classType == NULL)
        return false;
    
    SgClassDeclaration* classDeclaration = isSgClassDeclaration(classType->get_declaration());
    ROSE_ASSERT(classDeclaration != NULL);
    
    classDeclaration = isSgClassDeclaration(classDeclaration->get_definingDeclaration());
    if (classDeclaration == NULL)
    {
        //There's no defining declaration. We really can't tell
        return false;
    }
    SgClassDefinition* classDefinition = classDeclaration->get_definition();
        
    //Find all superclasses
    const ClassHierarchyWrapper::ClassDefSet& ancestors =  classHierarchy.getAncestorClasses(classDefinition);
    set<SgClassDefinition*> inclusiveAncestors(ancestors.begin(), ancestors.end());
    inclusiveAncestors.insert(classDefinition);
    
    //Find all virtual and concrete functions
    set<SgMemberFunctionDeclaration*> concreteFunctions, pureVirtualFunctions;
    foreach(SgClassDefinition* c, inclusiveAncestors)
    {
        foreach(SgDeclarationStatement* memberDeclaration, c->get_members())
        {
            if (SgMemberFunctionDeclaration* memberFunction = isSgMemberFunctionDeclaration(memberDeclaration))
            {
                if (memberFunction->get_functionModifier().isPureVirtual())
                {
                    pureVirtualFunctions.insert(memberFunction);
                }
                else
                {
                    concreteFunctions.insert(memberFunction);
                }
            }
        }
    }
    
    //Check if each virtual function is implemented somewhere
    foreach (SgMemberFunctionDeclaration* virtualFunction, pureVirtualFunctions)
    {
        bool foundConcrete = false;
        
        foreach (SgMemberFunctionDeclaration* concreteFunction, concreteFunctions)
        {
            if (concreteFunction->get_name() != virtualFunction->get_name())
                continue;
            
            if (concreteFunction->get_args().size() != virtualFunction->get_args().size())
                continue;
            
            bool argsMatch = true;
            for(size_t i = 0; i < concreteFunction->get_args().size(); i++)
            {
                if (concreteFunction->get_args()[i]->get_type() != virtualFunction->get_args()[i]->get_type())
                {
                    argsMatch = false;
                    break;
                }
            }
            
            if (!argsMatch)
                continue;
            
            foundConcrete = true;
            break;
        }
        
        //If there's a pure virtual function with no corresponding concrete function, the type is pure virtual
        if (!foundConcrete)
        {
            return true;
        }
    }
    
    return false;
}
#endif

    
#define CPP_TYPE_TRAITS
#ifdef CPP_TYPE_TRAITS
    /*    http://gcc.gnu.org/onlinedocs/gcc/Type-Traits.html
     
     7.10 Type Traits
     
     The C++ front end implements syntactic extensions that allow compile-time determination of various characteristics of a type (or of a pair of types).
     */
    
    
    
#define ENSURE_CLASS_TYPE(__type, ret) do { \
if(isSgPartialFunctionType(__type) || isSgTemplateType(__type) || isSgTypeUnknown(__type)){\
std::cout<<"\n Type = "<<__type->class_name();\
    ROSE_ASSERT(0 && "expected a complete type"); \
    return ret; \
}\
SgClassType * sgClassType = isSgClassType(__type);\
if (!sgClassType) { \
    return ret;\
}} while(0)



    // Walks over the call hierarchy and collects all member function in a vector.
    vector<SgMemberFunctionDeclaration*> GetAllMemberFunctionsInClassHierarchy(SgType *type){
#if DEBUG_QUADRATIC_BEHAVIOR
      printf ("In GetAllMemberFunctionsInClassHierarchy(): type = %p = %s \n",type,type->class_name().c_str());
#endif
        vector<SgMemberFunctionDeclaration*> allMemberFunctions;
        
        SgClassType * sgClassType = isSgClassType(type);
        if (!sgClassType) {
            ROSE_ASSERT(0 && "Requires a complete class type");
        }
        
        SgClassDeclaration* decl = isSgClassDeclaration(sgClassType->get_declaration());
        ROSE_ASSERT(decl && "Requires a complete class type");
        SgClassDeclaration * definingDeclaration = isSgClassDeclaration(decl->get_definingDeclaration ());
        ROSE_ASSERT(definingDeclaration && "Requires a complete class type");
        SgClassDefinition* classDef = definingDeclaration->get_definition();
        ROSE_ASSERT(classDef && "Requires a complete class type");
        
        //Find all superclasses
        ClassHierarchyWrapper classHierarchy(getProject());
        const ClassHierarchyWrapper::ClassDefSet& ancestors =  classHierarchy.getAncestorClasses(classDef);
        set<SgClassDefinition*> inclusiveAncestors(ancestors.begin(), ancestors.end());
        inclusiveAncestors.insert(classDef);
        
        //Find all member functions
        foreach(SgClassDefinition* c, inclusiveAncestors) {
            foreach(SgDeclarationStatement* memberDeclaration, c->get_members()) {
                if (SgMemberFunctionDeclaration* memberFunction = isSgMemberFunctionDeclaration(memberDeclaration)) {
                    allMemberFunctions.push_back(memberFunction);
                }
            }
        }
        
        return allMemberFunctions;
    }

    // Walks over the call hierarchy and collects all data members in a vector.
    vector<SgVariableDeclaration*> GetAllVariableDeclarationsInAncestors(SgType *type){
        vector<SgVariableDeclaration*> allVariableDeclarations;
        
        SgClassType * sgClassType = isSgClassType(type);
        if (!sgClassType) {
            ROSE_ASSERT(0 && "Requires a complete class type");
        }
        
        SgClassDeclaration* decl = isSgClassDeclaration(sgClassType->get_declaration());
        ROSE_ASSERT(decl && "Requires a complete class type");
        SgClassDeclaration * definingDeclaration = isSgClassDeclaration(decl->get_definingDeclaration ());
        ROSE_ASSERT(definingDeclaration && "Requires a complete class type");
        SgClassDefinition* classDef = definingDeclaration->get_definition();
        ROSE_ASSERT(classDef && "Requires a complete class type");
        
        //Find all superclasses
        ClassHierarchyWrapper classHierarchy(getProject());
        const ClassHierarchyWrapper::ClassDefSet& ancestors =  classHierarchy.getAncestorClasses(classDef);
        set<SgClassDefinition*> exclusiveAncestors(ancestors.begin(), ancestors.end());
        
        //Find all data members
        foreach(SgClassDefinition* c, exclusiveAncestors) {
            foreach(SgDeclarationStatement* memberDeclaration, c->get_members()) {
                if (SgVariableDeclaration * sgVariableDeclaration = isSgVariableDeclaration(memberDeclaration)) {
                    allVariableDeclarations.push_back(sgVariableDeclaration);
                }
            }
        }
        
        return allVariableDeclarations;
    }


    // Checks if the given member function accepts the given argument.
    // If the argument passed is null, then the argument is assumed to be of the same type as the class that the memberfunction belongs to.
    // The check ignores const and reference modifiers.
    static bool CheckIfFunctionAcceptsArgumentIgnoreConstRefAndTypedef(SgMemberFunctionDeclaration* decl, SgType * args = NULL) {
        SgDeclarationStatement * declstmt = decl->get_associatedClassDeclaration();
        SgClassDeclaration * xdecl = isSgClassDeclaration(declstmt);
        ROSE_ASSERT(xdecl != NULL);

        // if no args was passed, we will take the class type as the arg
        args = xdecl->get_type()->stripType(SgType::STRIP_REFERENCE_TYPE | SgType::STRIP_MODIFIER_TYPE | SgType::STRIP_TYPEDEF_TYPE);
        
        // Must not be const or typedef or ref type
        ROSE_ASSERT( isConstType(args) == 0 );
        ROSE_ASSERT( isReferenceType(args) == 0 );
        ROSE_ASSERT( isSgTypedefType(args) == 0 );
        
        SgInitializedNamePtrList& params = decl->get_args();
        SgInitializedNamePtrList::iterator p = params.begin();
        
        // can't be zero arguments
        if(p == params.end())
            return false;
        
        if ((*p)->get_type()->stripType(SgType::STRIP_REFERENCE_TYPE | SgType::STRIP_MODIFIER_TYPE | SgType::STRIP_TYPEDEF_TYPE) != args)
            return false; // Type mismatch
        
        // Must be single argument
        p++;
        if(p != params.end())
            return false;
        return true;
    }

    /*
     __has_nothrow_assign (type)
     If type is const qualified or is a reference type then the trait is false.
     Otherwise if __has_trivial_assign (type) is true then the trait is true, else if type is a cv class or union type with copy assignment operators that are known not to throw an exception then the trait is true, else it is false.
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     */

    bool HasNoThrowAssign(const SgType * const inputType){
        //Strip typedef and array
        SgType * type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE );
        
        if (isConstType(type))
            return false;
        
        if (isReferenceType(type))
            return false;
        
        if(HasTrivialAssign(type))
            return true;
        
        ENSURE_CLASS_TYPE(type, false);
        vector<SgMemberFunctionDeclaration*> allMemberFunctions =  GetAllMemberFunctionsInClassHierarchy(type);
#if DEBUG_QUADRATIC_BEHAVIOR
        printf ("In HasNoThrowAssign(): allMemberFunctions.size() = %zu \n",allMemberFunctions.size());
#endif
        foreach(SgMemberFunctionDeclaration* memberFunction,allMemberFunctions) {
            if (memberFunction->get_specialFunctionModifier().isOperator() && memberFunction->get_name().getString()=="operator="){
                // check if the function has nothrow attribute
                //TODO: C++11 accepts noexcept expression which we need to check when we support it.
                if(memberFunction->get_functionModifier ().isGnuAttributeNoThrow () == false)
                    return false;
            }
        }
        return true;
    }


    /*
     __has_nothrow_copy (type)
     If __has_trivial_copy (type) is true then the trait is true, else if type is a cv class or union type with copy constructors that are known not to throw an exception then the trait is true, else it is false.
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     */
    bool HasNoThrowCopy(const SgType * const inputType){
        //Strip typedef and array
        SgType * type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE );
        
        if(HasTrivialCopy(type))
            return true;
        
        ENSURE_CLASS_TYPE(type, false);
        vector<SgMemberFunctionDeclaration*> allMemberFunctions =  GetAllMemberFunctionsInClassHierarchy(type);
#if DEBUG_QUADRATIC_BEHAVIOR
        printf ("In HasNoThrowCopy(): allMemberFunctions.size() = %zu \n",allMemberFunctions.size());
#endif
        foreach(SgMemberFunctionDeclaration* memberFunction,allMemberFunctions) {
            if (memberFunction->get_specialFunctionModifier().isConstructor()){
                // function must have exactly one argument and the argument type must match class type after stripping typedef, const, and ref
                if(CheckIfFunctionAcceptsArgumentIgnoreConstRefAndTypedef(memberFunction)) {
                    //TODO: C++11 accepts noexcept expression which we need to check when we support it.
                    if(memberFunction->get_functionModifier().isGnuAttributeNoThrow () == false)
                        return false;
                }
            }
        }
        return true;
    }

    /*
     __has_nothrow_constructor (type)
     If __has_trivial_constructor (type) is true then the trait is true, else if type is a cv class or union type (or array thereof) with a default constructor that is known not to throw an exception then the trait is true, else it is false. 
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     */
    bool HasNoThrowConstructor(const SgType * const inputType){
        //Strip typedef and array
        SgType * type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE );
        
        if(HasTrivialConstructor(type))
            return true;
        
        ENSURE_CLASS_TYPE(type, false);
        vector<SgMemberFunctionDeclaration*> allMemberFunctions =  GetAllMemberFunctionsInClassHierarchy(type);
#if DEBUG_QUADRATIC_BEHAVIOR
        printf ("In HasNoThrowConstructor(): allMemberFunctions.size() = %zu \n",allMemberFunctions.size());
#endif
        foreach(SgMemberFunctionDeclaration* memberFunction,allMemberFunctions) {
            if (memberFunction->get_specialFunctionModifier().isConstructor()){
                // function must have no args
                if(memberFunction->get_args().size() == 0) {
                    //TODO: C++11 accepts noexcept expression which we need to check when we support it.
                    if(memberFunction->get_functionModifier().isGnuAttributeNoThrow () == false)
                        return false;
                }
            }
        }
        return true;
    }

    /*
     __has_trivial_assign (type)
     If type is const qualified or is a reference type then the trait is false. 
     Otherwise if __is_pod (type) is true then the trait is true, else if type is a cv class or union type with a trivial copy assignment ([class.copy]) then the trait is true, else it is false. 
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     */
    static bool HasTrivialAssign(const SgType * const inputType, bool checkPod){
        //Strip typedef and array
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE );
        
        if (isConstType(type))
            return false;
        
        if (isReferenceType(type))
            return false;
        
        if (checkPod && IsPod(type))
            return true;
        
        ENSURE_CLASS_TYPE(type, false);
        vector<SgMemberFunctionDeclaration*> allMemberFunctions =  GetAllMemberFunctionsInClassHierarchy(type);
#if DEBUG_QUADRATIC_BEHAVIOR
        printf ("In HasTrivialAssign(): allMemberFunctions.size() = %zu \n",allMemberFunctions.size());
#endif
        foreach(SgMemberFunctionDeclaration* memberFunction,allMemberFunctions) {
            if (memberFunction->get_specialFunctionModifier().isOperator() && memberFunction->get_name().getString()=="operator="){
                // function must have exactly one argument and the argument type after stripping typedef, const, and ref must match class type
                if(CheckIfFunctionAcceptsArgumentIgnoreConstRefAndTypedef(memberFunction))
                    return false;
            }
        }
        return true;
    }

    bool HasTrivialAssign(const SgType * const inputType){
        return HasTrivialAssign(inputType, true);
    }


    /*
     __has_trivial_copy (type)
     If __is_pod (type) is true or type is a reference type then the trait is true, else if type is a cv class or union type with a trivial copy constructor ([class.copy]) then the trait is true, else it is false. 
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     */
    static bool HasTrivialCopy(const SgType * const inputType, bool checkPod){
        //Strip typedef and array
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE );
        
        
        if (checkPod && IsPod(type))
            return true;
        
        if (isReferenceType(type))
            return true;
        
        ENSURE_CLASS_TYPE(type, false);
        vector<SgMemberFunctionDeclaration*> allMemberFunctions =  GetAllMemberFunctionsInClassHierarchy(type);
#if DEBUG_QUADRATIC_BEHAVIOR
        printf ("In HasTrivialCopy(): allMemberFunctions.size() = %zu \n",allMemberFunctions.size());
#endif
        foreach(SgMemberFunctionDeclaration* memberFunction,allMemberFunctions) {
            if (memberFunction->get_specialFunctionModifier().isConstructor()){
                // function must have exactly one argument and the argument type after stripping typedef, const, and ref must match passed-in type
                if(CheckIfFunctionAcceptsArgumentIgnoreConstRefAndTypedef(memberFunction))
                    return false;
            }
        }
        return true;
    }

    bool HasTrivialCopy(const SgType * const inputType){
        return HasTrivialCopy(inputType, true);
    }


    /*
     __has_trivial_constructor (type)
     If __is_pod (type) is true then the trait is true, else if type is a cv class or union type (or array thereof) with a trivial default constructor ([class.ctor]) then the trait is true, else it is false. 
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     */
    static bool HasTrivialConstructor(const SgType * const inputType, bool checkPod){
        //Strip typedef and array
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE );
        
        if (checkPod && IsPod(type))
            return true;
        
        ENSURE_CLASS_TYPE(type, false);
        vector<SgMemberFunctionDeclaration*> allMemberFunctions =  GetAllMemberFunctionsInClassHierarchy(type);
#if DEBUG_QUADRATIC_BEHAVIOR
        printf ("In HasTrivialConstructor(): allMemberFunctions.size() = %zu \n",allMemberFunctions.size());
#endif
        foreach(SgMemberFunctionDeclaration* memberFunction,allMemberFunctions) {
            if (memberFunction->get_specialFunctionModifier().isConstructor()){
                return false;
            }
        }
        return true;
    }

    bool HasTrivialConstructor(const SgType * const inputType){
        return HasTrivialConstructor(inputType, true);
    }

    /*
     __has_trivial_destructor (type)
     If __is_pod (type) is true or type is a reference type then the trait is true, else if type is a cv class or union type (or array thereof) with a trivial destructor ([class.dtor]) then the trait is true, else it is false.
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     */
    static bool HasTrivialDestructor(const SgType * const inputType, bool checkPod){
        //Strip typedef and array
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE );
        
        if (checkPod && IsPod(type))
            return true;
        
        if (isReferenceType(type))
            return true;
        
        ENSURE_CLASS_TYPE(type, false);
        vector<SgMemberFunctionDeclaration*> allMemberFunctions =  GetAllMemberFunctionsInClassHierarchy(type);
#if DEBUG_QUADRATIC_BEHAVIOR
        printf ("In HasTrivialDestructor(): allMemberFunctions.size() = %zu \n",allMemberFunctions.size());
#endif
        foreach(SgMemberFunctionDeclaration* memberFunction,allMemberFunctions) {
            if (memberFunction->get_specialFunctionModifier().isDestructor()){
                return false;
            }
        }
        return true;
        
    }
    
    bool HasTrivialDestructor(const SgType * const inputType){
        return HasTrivialDestructor(inputType, true);
    }

    /*
     __has_virtual_destructor (type)
     If type is a class type with a virtual destructor ([class.dtor]) then the trait is true, else it is false. 
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     */
    bool HasVirtualDestructor(const SgType * const inputType){
        //Strip typedef and array
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE );
        
        ENSURE_CLASS_TYPE(type, false);
        
        if(!IsClass(type)){
            return false;
        }
        
        vector<SgMemberFunctionDeclaration*> allMemberFunctions =  GetAllMemberFunctionsInClassHierarchy(type);
#if DEBUG_QUADRATIC_BEHAVIOR
        printf ("In HasVirtualDestructor(): allMemberFunctions.size() = %zu \n",allMemberFunctions.size());
#endif
        foreach(SgMemberFunctionDeclaration* memberFunction,allMemberFunctions) {
            if (memberFunction->get_functionModifier().isVirtual() && memberFunction->get_specialFunctionModifier().isDestructor())
                return true;
        }
        
        // No virtual destructor(s) found
        return false;
    }

    /*
     __is_abstract (type)
     If type is an abstract class ([class.abstract]) then the trait is true, else it is false. 
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     */
    bool IsAbstract(const SgType * const inputType) {
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE );
        ENSURE_CLASS_TYPE(type, false);
        // false if it is not a class
        if(!IsClass(type)){
            return false;
        }
        return SageInterface::isPureVirtualClass(type, ClassHierarchyWrapper(getProject()));
    }

    /*
     __is_base_of (base_type, derived_type)
     If base_type is a base class of derived_type ([class.derived]) then the trait is true, otherwise it is false. 
     Top-level cv qualifications of base_type and derived_type are ignored. For the purposes of this trait, a class type is considered is own base. 
     Requires: if __is_class (base_type) and __is_class (derived_type) are true and base_type and derived_type are not the same type (disregarding cv-qualifiers), derived_type shall be a complete type.
     Diagnostic is produced if this requirement is not met.
     
     http://www.cplusplus.com/reference/type_traits/is_base_of/
     
     Trait class that identifies whether Base is a base class of (or the same class as) Derived, without regard to their const and/or volatile qualification. 
     Only classes that are not unions are considered.
     */
    bool IsBaseOf(const SgType * const inputBaseType, const SgType * const inputDerivedType) {
        //Strip typedef and array
        SgType *baseType = inputBaseType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_MODIFIER_TYPE );
        SgType *derivedType = inputDerivedType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_MODIFIER_TYPE );
        
        if (!IsClass(baseType) || !IsClass(derivedType)) {
            return false;
        }
        
        // derivedType must be complete type
        ENSURE_CLASS_TYPE(derivedType, false);
        SgClassType * baseClassType = isSgClassType(baseType);
        SgClassType * derivedClassType = isSgClassType(derivedType);
        SgClassType * sgClassType = isSgClassType(derivedClassType);
        SgClassDeclaration* decl = isSgClassDeclaration(sgClassType->get_declaration());
        ROSE_ASSERT(decl);
        SgClassDeclaration * definingDeclaration = isSgClassDeclaration(decl->get_definingDeclaration ());
        ROSE_ASSERT(definingDeclaration && "Requires a complete class type");
        SgClassDefinition* classDef = definingDeclaration->get_definition();
        ROSE_ASSERT(classDef);
        
        
        //Find all superclasses
        ClassHierarchyWrapper classHierarchy(getProject());
        const ClassHierarchyWrapper::ClassDefSet& ancestors =  classHierarchy.getAncestorClasses(classDef);
        set<SgClassDefinition*> inclusiveAncestors(ancestors.begin(), ancestors.end());
        inclusiveAncestors.insert(classDef);
#if DEBUG_QUADRATIC_BEHAVIOR
        printf ("In HasVirtualDestructor(): inclusiveAncestors.size() = %zu \n",inclusiveAncestors.size());
#endif
        
        // Match types
        foreach(SgClassDefinition* c, inclusiveAncestors) {
            if(c->get_declaration()->get_type()->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE  | SgType::STRIP_MODIFIER_TYPE ) == baseClassType)
                return true;
        }
        return false;
        
        
    }

    /*
     __is_class (type)
     If type is a cv class type, and not a union type ([basic.compound]) the trait is true, else it is false.
     */

    bool IsClass(const SgType * const inputType) {
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_MODIFIER_TYPE );
        return isSgClassType(type) && (!IsUnion(type));
    }

    /*
     __is_empty (type)
     If __is_class (type) is false then the trait is false.
     Otherwise type is considered empty if and only if: type has no non-static data members, or all non-static data members,
     if any, are bit-fields of length 0, and type has no virtual members, and type has no virtual base classes,
     and type has no base classes base_type for which __is_empty (base_type) is false.
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     */

    bool IsEmpty(const SgType * const inputType, bool checkIsClass){
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_MODIFIER_TYPE);
        
        if(checkIsClass && !IsClass(type))
            return false;
        
        
        // Make sure the class is complete
        ENSURE_CLASS_TYPE(type, false);
        SgClassType * classType = isSgClassType(type);
        SgClassDefinition * classDefinition = isSgClassDeclaration(isSgClassDeclaration(classType->get_declaration()->get_definingDeclaration()))->get_definition();
        // All non static members must be bit fields of length 0.
        SgDeclarationStatementPtrList &members = classDefinition->get_members();
        for (SgDeclarationStatementPtrList::iterator it = members.begin(); it != members.end(); ++it) {
            SgDeclarationStatement *declarationStatement = *it;
            ROSE_ASSERT(declarationStatement != NULL);
            
            if (SgMemberFunctionDeclaration * memberFunctionDeclaration = isSgMemberFunctionDeclaration(declarationStatement)) {
                // Member function can't be virtual
                if (memberFunctionDeclaration->get_functionModifier().isVirtual())
                    return false;
            } else if(SgVariableDeclaration * sgVariableDeclaration = isSgVariableDeclaration(declarationStatement)) {
                // Non-static data memebers must be bit fields of length 0
                if (SageInterface::isStatic(sgVariableDeclaration))
                    continue;
                
                if (isScalarType(sgVariableDeclaration->get_definition()->get_type()))
                {
                 // DQ (1/20/2014): The bitfield is not stored as a SgValueExp.  As a result we can't get the value as directly.
                 // I think that this correction to the code below is what is required.
                 // if (sgVariableDeclaration->get_definition()->get_bitfield() == NULL || sgVariableDeclaration->get_definition()->get_bitfield()->get_value() != 0)
                   if (sgVariableDeclaration->get_definition()->get_bitfield() != NULL)
                    {
                        return false;
                    }
                } else {
                    // if it is not a class type, we will return false
                    if(!isSgClassType(sgVariableDeclaration->get_definition()->get_type()))
                        return false;
                    
                    // if it is a class or a union it should be empty:
                    if(!IsEmpty(sgVariableDeclaration->get_definition()->get_type(), false /* checkIsClass = false so that we can check unions too*/))
                        return false;
                }
            }
        }
        
        // No virtual base classes
        SgBaseClassPtrList  & baseClasses = classDefinition->get_inheritances();
        for(SgBaseClassPtrList::iterator it = baseClasses.begin(); it != baseClasses.end(); ++it)
           {
          // Base class can't be virtual

          // DQ (1/21/2019): get_baseClassModifier() uses ROSETTA generated access functions which return a pointer.
          // if((*it)->get_baseClassModifier().isVirtual())
             ROSE_ASSERT((*it)->get_baseClassModifier() != NULL);
             if ((*it)->get_baseClassModifier()->isVirtual())
                  return false;
            
          // Base classes must be empty as well
             SgClassDeclaration * baseClass = (*it)->get_base_class();
             ROSE_ASSERT(baseClass != NULL);
             if (!IsEmpty(baseClass->get_type()))
                  return false;
           }
        
        return true;
    }

    bool IsEmpty(const SgType * const inputType){
        return IsEmpty(inputType, true);
    }

    /*
     __is_enum (type)
     If type is a cv enumeration type ([basic.compound]) the trait is true, else it is false.
     */

    bool IsEnum(const SgType * const inputType){
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE| SgType::STRIP_MODIFIER_TYPE);
        return isSgEnumType(type);
    }

    /*
     __is_literal_type (type)
     If type is a literal type ([basic.types]) the trait is true, else it is false. 
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.

     A literal type is a type that can qualify as constexpr. This is true for scalar types, references, certain classes, and arrays of any such types.
     
     http://www.cplusplus.com/reference/type_traits/is_literal_type/
     A class that is a literal type is a class (defined with class, struct or union) that:
     has a trivial destructor,
     every constructor call and any non-static data member that has brace- or equal- initializers is a constant expression,
     is an aggregate type, or has at least one constexpr constructor or constructor template that is not a copy or move constructor, and
     has all non-static data members and base classes of literal types

     */
    bool IsLiteralType(const SgType * const inputType){
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE | SgType::STRIP_MODIFIER_TYPE);

        // Scalars, Pointers and Reference are literal
        if (isScalarType(type) || isReferenceType(type) || isPointerType(type)) {
            return true;
        }
        
        ROSE_ASSERT(0 && "NYI, don't know what type to accept for this API");
        abort();
        ENSURE_CLASS_TYPE(type, false);
    }

    /*
     __is_pod (type)
     If type is a cv POD type ([basic.types]) then the trait is true, else it is false. 
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     
     http://www.cplusplus.com/reference/type_traits/is_pod/
     
     A POD type (which stands for Plain Old Data type) is a type whose characteristics are supported by a data type in the C language, either cv-qualified or not. This includes scalar types, POD classes and arrays of any such types.
     
     A POD class is a class that is both trivial (can only be statically initialized) and standard-layout (has a simple data structure), and thus is mostly restricted to the characteristics of a class that are compatible with those of a C data structure declared with struct or union in that language, even though the expanded C++ grammar can be used in their declaration and can have member functions.
     
     */

    static bool IsTrivial(const SgType * const inputType, bool checkTrivial);

    bool IsPod(const SgType * const inputType) {
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE );
        if(IsTrivial(type,  /* checkPod = */ false) && IsStandardLayout(type))
            return true;
        return false;
    }

    /*
     __is_polymorphic (type)
     If type is a polymorphic class ([class.virtual]) then the trait is true, else it is false. 
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     
     http://www.cplusplus.com/reference/type_traits/is_polymorphic/
     Trait class that identifies whether T is a polymorphic class.
     A polymorphic class is a class that declares or inherits a virtual function.
     */
    bool IsPolymorphic(const SgType * const inputType) {
        SgType *type = inputType->stripType( SgType::STRIP_MODIFIER_TYPE | SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE);
        // What if it is a template at this point!!!!
        
        if(!IsClass(type)) {
            return false;
        }
        
        ENSURE_CLASS_TYPE(type, false);
        vector<SgMemberFunctionDeclaration*> allMemberFunctions =  GetAllMemberFunctionsInClassHierarchy(type);
#if DEBUG_QUADRATIC_BEHAVIOR
        printf ("In IsPolymorphic(): allMemberFunctions.size() = %zu \n",allMemberFunctions.size());
#endif
        foreach(SgMemberFunctionDeclaration* memberFunction,allMemberFunctions) {
            if (memberFunction->get_functionModifier().isVirtual())
                return true;
        }
        return false;
        
    }

    /*
     __is_standard_layout (type)
     If type is a standard-layout type ([basic.types]) the trait is true, else it is false. 
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     
     
     A standard-layout type is a type with a simple linear data structure and access control that can easily be used to communicate with code written in other programming languages, such as C, either cv-qualified or not. This is true for scalar types, standard-layout classes and arrays of any such types.
     
     A standard-layout class is a class (defined with class, struct or union) that:
     has no virtual functions and no virtual base classes.
     has the same access control (private, protected, public) for all its non-static data members.
     either has no non-static data members in the most derived class and at most one base class with non-static data members, or has no base classes with non-static data members.
     its base class (if any) is itself also a standard-layout class. And,
     has no base classes of the same type as its first non-static data member.
     */
    bool IsStandardLayout(const SgType * const inputType){
        SgType *type = inputType->stripType( SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE | SgType::STRIP_REFERENCE_TYPE /* not sure but looks ok*/);
        
        // isPointerType are scalar
        if(isScalarType(type) || isPointerType(type))
            return true;
        
        
        ENSURE_CLASS_TYPE(type, false);
        SgClassType * sgClassType = isSgClassType(type);
        SgClassDeclaration* decl = isSgClassDeclaration(sgClassType->get_declaration());
        ROSE_ASSERT(decl);
        SgClassDeclaration * definingDeclaration = isSgClassDeclaration(decl->get_definingDeclaration ());
        ROSE_ASSERT(definingDeclaration && "Requires a complete class type");
        SgClassDefinition* classDef = definingDeclaration->get_definition();
        ROSE_ASSERT(classDef);
        
        
        //If we have a virtual function, then it not OK
        foreach(SgDeclarationStatement* memberDeclaration, classDef->get_members()) {
            if (SgMemberFunctionDeclaration* memberFunction = isSgMemberFunctionDeclaration(memberDeclaration)) {
                if (memberFunction->get_functionModifier().isVirtual())
                    return false;
            }
        }
        
        // No virtual base classes
        SgBaseClassPtrList  & baseClasses = classDef->get_inheritances();
        for(SgBaseClassPtrList::iterator it = baseClasses.begin(); it != baseClasses.end(); ++it)
           {
          // Base class can't be virtual
          // DQ (1/21/2019): get_baseClassModifier() uses ROSETTA generated access functions which return a pointer.
          // if((*it)->get_baseClassModifier().isVirtual())
             ROSE_ASSERT((*it)->get_baseClassModifier() != NULL);
             if((*it)->get_baseClassModifier()->isVirtual())
                return false;
           }
        
        bool haveNonStaticData = false;
        SgType * firstNonStaticDataMember = NULL;
        
        //same access control (private, protected, public) for all its non-static data members
        bool first = true;
        SgAccessModifier::access_modifier_enum prevModifier=SgAccessModifier::e_unknown;
        foreach(SgDeclarationStatement* memberDeclaration, classDef->get_members()) {
            //has no non-static data members with brace- or equal- initializers.
            if (SgVariableDeclaration* memberVariable = isSgVariableDeclaration(memberDeclaration)) {
                if (memberVariable->get_declarationModifier().get_storageModifier().isStatic())
                    continue;
                
                SgAccessModifier::access_modifier_enum  mod = memberDeclaration->get_declarationModifier().get_accessModifier().get_modifier();
                if (!first && mod != prevModifier) {
                    return false;
                }
                
                if (first) {
                    first = false;
                    firstNonStaticDataMember = NULL;
                    foreach(SgInitializedName* name, memberVariable->get_variables ()) {
                        firstNonStaticDataMember = name->get_type();
                        break;
                    }
                }
                prevModifier = mod;
                haveNonStaticData = true;
            }
        }
        
        
        // If the most derived type has non-static data members, then none of the base classes can have non-static members.
        if (haveNonStaticData) {
            vector<SgVariableDeclaration *> allVariableDeclarations = GetAllVariableDeclarationsInAncestors(type);
#if DEBUG_QUADRATIC_BEHAVIOR
            printf ("In IsStandardLayout(): allVariableDeclarations.size() = %zu \n",allVariableDeclarations.size());
#endif
            foreach(SgVariableDeclaration* memberVariable, allVariableDeclarations) {
                if (memberVariable->get_declarationModifier().get_storageModifier().isStatic())
                    continue;
                // ok, we have atleast one non-static data member in the ancestry
                return false;
            }
        } else {
            // no non-static data members in the most derived class and at most one base class with non-static data members
            //Find all superclasses
            ClassHierarchyWrapper classHierarchy(getProject());
            
            const ClassHierarchyWrapper::ClassDefSet& ancestors =  classHierarchy.getAncestorClasses(classDef);
            set<SgClassDefinition*> exclusiveAncestors(ancestors.begin(), ancestors.end());
            
#if DEBUG_QUADRATIC_BEHAVIOR
            printf ("In IsStandardLayout(): exclusiveAncestors.size() = %zu \n",exclusiveAncestors.size());
#endif
            
            // There can be atmost 1 base class with non-static data members
            int baseClassesWithNonStaticData = 0;
            foreach(SgClassDefinition* c, exclusiveAncestors) {
                foreach(SgDeclarationStatement* baseMemberDeclaration, c->get_members()) {
                    if (SgVariableDeclaration* baseMemberVariable = isSgVariableDeclaration(baseMemberDeclaration)) {
                        if (!baseMemberVariable->get_declarationModifier().get_storageModifier().isStatic()) {
                            baseClassesWithNonStaticData ++;
                            if(baseClassesWithNonStaticData > 1)
                                return false;
                            break;
                        }
                    }
                }
            }
        }
        
        // All base classes have to be standard layout
        foreach(SgBaseClass* baseClass, classDef->get_inheritances()) {
            if(!IsStandardLayout(baseClass->get_base_class()->get_type()))
                return false;
        }
        
        // There should be no base classes of the same type as its first non-static data member
        if(firstNonStaticDataMember){
            //Find all superclasses
            ClassHierarchyWrapper classHierarchy(getProject());
            const ClassHierarchyWrapper::ClassDefSet& ancestors =  classHierarchy.getAncestorClasses(classDef);
            set<SgClassDefinition*> exclusiveAncestors(ancestors.begin(), ancestors.end());
            
#if DEBUG_QUADRATIC_BEHAVIOR
            printf ("In IsStandardLayout(): exclusiveAncestors.size() = %zu \n",exclusiveAncestors.size());
#endif
            foreach(SgClassDefinition* c, exclusiveAncestors) {
                if(c->get_declaration()->get_type()->stripType(SgType::STRIP_TYPEDEF_TYPE) ==  firstNonStaticDataMember->stripType(SgType::STRIP_TYPEDEF_TYPE))
                    return false;
            }
        }
        
        return true;
        
    }

    /*
     __is_trivial (type)
     If type is a trivial type ([basic.types]) the trait is true, else it is false. 
     Requires: type shall be a complete type, (possibly cv-qualified) void, or an array of unknown bound.
     http://www.cplusplus.com/reference/type_traits/is_trivial/
     
     A trivial type is a type whose storage is contiguous (trivially copyable) and which only supports static default initialization (trivially default constructible), either cv-qualified or not. It includes scalar types, trivial classes and arrays of any such types.
     
     A trivial class is a class (defined with class, struct or union) that is both trivially default constructible and trivially copyable, which implies that:
     uses the implicitly defined default, copy and move constructors, copy and move assignments, and destructor.
     has no virtual members.
     has no non-static data members with brace- or equal- initializers.
     its base class and non-static data members (if any) are themselves also trivial types.
     */

    static bool IsTrivial(const SgType * const inputType, bool checkTrivial){
        SgType *type = inputType->stripType( SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_ARRAY_TYPE);
        
        // Scalars or pointers are trivial types
        if(isScalarType(type) || isPointerType(type))
            return true;
        
        
        ENSURE_CLASS_TYPE(type, false);
        SgClassType * sgClassType = isSgClassType(type);
        if (!sgClassType) {
            //ROSE_ASSERT(0 && "IsTrivial requires a complete type");
            return false;
        }
        SgClassDeclaration* decl = isSgClassDeclaration(sgClassType->get_declaration());
        ROSE_ASSERT(decl);
        SgClassDeclaration * definingDeclaration = isSgClassDeclaration(decl->get_definingDeclaration());
        ROSE_ASSERT(definingDeclaration);
        SgClassDefinition* classDef = definingDeclaration->get_definition();
        ROSE_ASSERT(classDef);
        
        
        if(!HasTrivialConstructor(type, /* checkPod = */ checkTrivial) ||
           !HasTrivialCopy(type, /* checkPod = */ checkTrivial) ||
           !HasTrivialDestructor(type, /* checkPod = */ checkTrivial) ||
           !HasTrivialAssign(type, /* checkPod = */ checkTrivial)
           /* || HasTrivialMove!!! */)
            return false;
        
        
        foreach(SgDeclarationStatement* memberDeclaration, classDef->get_members()) {
            // No virtual functions
            if (SgMemberFunctionDeclaration* memberFunction = isSgMemberFunctionDeclaration(memberDeclaration)) {
                if (memberFunction->get_functionModifier().isVirtual())
                    return false;
            }
            //has no non-static data members with brace- or equal- initializers.
            if (SgVariableDeclaration* memberVariable = isSgVariableDeclaration(memberDeclaration)) {
                
                if (memberVariable->get_declarationModifier().get_storageModifier().isStatic())
                    continue;
                
                // each variable must be Trivial and must not have initializers
                foreach(SgInitializedName* name, memberVariable->get_variables ()) {
                    if(name->get_initializer())
                        return false;
                    if(!IsTrivial(name->get_type()))
                        return false;
                }
                
            }
        }
        
        // Base classes must be trivial too
        foreach(SgBaseClass* baseClass, classDef->get_inheritances()) {
            if(!IsTrivial(baseClass->get_base_class()->get_type()))
                return false;
        }
        
        return true;
        
    }

    bool IsTrivial(const SgType * const inputType){
        return IsTrivial(inputType, true);
    }

    /*
     __is_union (type)
     If type is a cv union type ([basic.compound]) the trait is true, else it is false.
     */
    bool IsUnion(const SgType * const inputType){
        SgType *type = inputType->stripType(SgType::STRIP_TYPEDEF_TYPE);
        return isUnionType(type);
    }

    /*
     __underlying_type (type)
     The underlying type of type. Requires: type shall be an enumeration type ([dcl.enum]).
     */
    SgType *  UnderlyingType(SgType *type){
        ROSE_ASSERT(IsEnum(type));
        ROSE_ASSERT( 0  && "Cxx11 feature NYI");
        return NULL;
    }

#endif //CPP_TYPE_TRAITS


} //end of namespace


