#include <Sawyer/Assert.h>

#include <Rose/Common/Analysis/ClassHierarchyAnalysis.h>
#include <Rose/Common/Analysis/Utility.h>

#include <algorithm>
#include <unordered_set>
#include <cassert>
#include <iterator>

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>

namespace rca = Rose::Common::Analysis;
namespace adapt = boost::adaptors;


namespace
{
  [[noreturn]]
  void
  reportAndThrowError( const rca::AbstractCompatibilityBridge&,
                       rca::ClassKeyType key,
                       const rca::ClassAnalysis& m
                     )
  {
    msgError() << m.classInfo(key) << std::endl;

    if (m.size() == 0)
      msgError() << "\nNote, the class hierarchy database is empty." << std::endl;

    throw std::out_of_range("Unable to find key in ClassAnalysis.");
  }

  [[noreturn]]
  void
  reportAndThrowError( const rca::AbstractCompatibilityBridge& compat,
                       rca::FunctionKeyType key,
                       const rca::VirtualFunctionAnalysis& m
                     )
  {
    msgError() << "Function " << compat.nameOf(key) << " not found in VirtualFunctionAnalysis"
               << std::endl;

    if (m.size() == 0)
      msgError() << "\nNote, the virtual hierarchy database is empty." << std::endl;

    throw std::out_of_range("Unable to find key in VirtualFunctionAnalysis.");
  }

  // AnalysisMap = is either a const or non-const analysis (i.e., ClassAnalysis, VirtualFunctionAnalysis)
  template <class AnalysisMap>
  inline
  auto
  lookup( const rca::AbstractCompatibilityBridge& compat,
          AnalysisMap& m,
          const typename AnalysisMap::key_type& key
        ) -> decltype(*m.find(key))
  {
    auto pos = m.find(key);

    if (pos != m.end())
      return *pos;

    reportAndThrowError(compat, key, m);
  }
}

namespace Rose::Common::Analysis
{

ClassAnalysisInfo ClassAnalysis::classInfo(ClassKeyType classKey) const
{
  return { this, classKey, &compatibilityBridge() };
}

std::ostream& operator<<(std::ostream& os, ClassAnalysisInfo cai)
{
  const ClassAnalysis*               analysis = cai.analysis();
  const AbstractCompatibilityBridge* compat = cai.compat();
  ClassKeyType                       classkey = cai.key();

  if (analysis == nullptr)
    return os << "ClassAnalysisInfo::analysis() returns nullptr. Use a valid object."
              << std::endl;

  if (compat == nullptr)
    return os << "ClassAnalysisInfo::compat() returns nullptr. Use a valid object."
              << std::endl;

  auto pos = analysis->find(cai.key());
  if (pos != analysis->end())
    return os << "class " << compat->nameOf(classkey) << " exists in class hierarchy analysis. OK."
              << std::endl;

  os << "class " << compat->nameOf(classkey) << " was not found in the class hierarchy analysis.\n";

  if (!analysis->containsAllClasses())
    os << "  - The class hierarchy analysis was built incrementally.\n"
       << "    Possibly, " << compat->nameOf(classkey) << " has not been seen."
       << std::endl;

/*
  if (auto opt = missingDiagnostics(classkey))
    os << "\n    " << *opt
       << std::endl;
*/
  return os;
}

ClassAnalysis::mapped_type&
ClassAnalysis::at(const ClassAnalysis::key_type& k)
{
  const rca::AbstractCompatibilityBridge& compat = this->compatibilityBridge();

  return lookup(compat, *this, k).second;
}

const ClassAnalysis::mapped_type&
ClassAnalysis::at(const ClassAnalysis::key_type& k) const
{
  const rca::AbstractCompatibilityBridge& compat = this->compatibilityBridge();

  return lookup(compat, *this, k).second;
}

/*
const ClassAnalysis::mapped_type&
ClassAnalysis::at(const SgClassDefinition& clsdef) const
{
  return lookup(*this, &clsdef).second;
}
*/

void
ClassAnalysis::addInheritanceEdge(value_type& descendantEntry, ClassKeyType ancestorKey, bool virtualEdge, bool directEdge)
{
  ClassData&   ancestor = this->at(ancestorKey);
  ClassKeyType descendantKey = descendantEntry.first;
  ClassData&   descendant = descendantEntry.second;

  descendant.ancestors().emplace_back(ancestorKey,   virtualEdge, directEdge);
  ancestor.descendants().emplace_back(descendantKey, virtualEdge, directEdge);
}

void
ClassAnalysis::addInheritanceEdge(value_type& descendant, const InheritanceDesc& ancestor)
{
  addInheritanceEdge(descendant, ancestor.getClass(), ancestor.isVirtual(), ancestor.isDirect());
}

bool
ClassAnalysis::isBaseOf(ClassKeyType ancestorKey, ClassKeyType descendantKey) const
{
  using container = ClassData::AncestorContainer;

  const container&          ancestors = this->at(descendantKey).ancestors();
  container::const_iterator zz = ancestors.end();

  return zz != std::find_if( ancestors.begin(), zz,
                             [ancestorKey](const InheritanceDesc& desc) -> bool
                             {
                               return desc.getClass() == ancestorKey;
                             }
                           );
}

bool
ClassAnalysis::isVirtualBaseOf(ClassKeyType ancestorKey, ClassKeyType descendantKey) const
{
  using container = ClassData::AncestorContainer;

  const container&          ancestors = this->at(descendantKey).ancestors();
  container::const_iterator zz = ancestors.end();

  return zz != std::find_if( ancestors.begin(), zz,
                             [ancestorKey](const InheritanceDesc& desc) -> bool
                             {
                               return desc.isVirtual() && desc.getClass() == ancestorKey;
                             }
                           );
}

std::vector<InheritanceDesc>
ClassAnalysis::concreteDescendants(ClassKeyType classKey) const
{
  using container = ClassData::AncestorContainer;

  std::vector<InheritanceDesc> res;
  const container&             descendants = this->at(classKey).descendants();
  const auto&                  compat = this->compatibilityBridge();

  auto isConcreteClass =
      [&compat](const InheritanceDesc& desc) -> bool
      {
        return !compat.isAbstract(desc.getClass());
      };

  std::copy_if( descendants.begin(), descendants.end(),
                std::back_inserter(res),
                isConcreteClass
              );

  auto byDirectThenVirtual =
      [](const InheritanceDesc& lhs, const InheritanceDesc& rhs) -> bool
      {
        if (lhs.isDirect() != rhs.isDirect())
          return lhs.isDirect();

        if (lhs.isVirtual() != rhs.isVirtual())
          return !lhs.isVirtual();

        return lhs < rhs;
      };

  std::sort(res.begin(), res.end(), byDirectThenVirtual);
  return res;
}



}




namespace
{
  using RelationAccessor = std::vector<rca::InheritanceDesc>& (rca::ClassData::*)();
  using RelationAccessorConst = const std::vector<rca::InheritanceDesc>& (rca::ClassData::*)() const;

  struct GetClass
  {
    rca::ClassKeyType operator()(const rca::InheritanceDesc& desc) const
    {
      return desc.getClass();
    }
  };

  template <class AnalysisT, class MemFnSelector>
  auto
  inheritanceRelationships(AnalysisT& all, MemFnSelector selector)
  {
    return [selector, &all](auto& elem) // -> someRange
           {
             return (elem.second.*selector)() | adapt::filtered(rca::IsDirect{}) | adapt::transformed(GetClass{});
           };
  }

  template <class AnalysisT, class Fn, class Selector, class Set>
  void traversal(AnalysisT& all, Fn fn, rca::ClassKeyType curr, Selector selector, Set& explored)
  {
    const rca::AbstractCompatibilityBridge& compat = all.compatibilityBridge();

    if (!explored.insert(curr).second)
      return;

    auto& elem = lookup(compat, all, curr);

    for (rca::ClassKeyType clazz : selector(elem))
    {
      try
      {
        traversal(all, fn, clazz, selector, explored);
      }
      catch (...)
      {
        msgError() << "  .. required from " << compat.nameOf(elem.first)
                   << "\n     and dependency " << compat.nameOf(clazz) << " - " << typeid(clazz).name()
                   << "\n     CONSIDER: use the language-specific memory-pool analysis to find all classes."
                   << std::endl;

        throw;
      }
    }

    fn(elem);
  }


  template <class AnalysisT, class Fn, class MemFnSelector>
  void traversal(AnalysisT& all, Fn fn, MemFnSelector selector)
  {
    std::unordered_set<rca::ClassKeyType> explored;

    for (auto& elem : all)
      traversal(all, fn, elem.first, inheritanceRelationships(all, selector), explored);
  }
}

namespace Rose::Common::Analysis
{

void topDownTraversal(ClassAnalysis& all, ClassAnalysisFn fn)
{
  RelationAccessor ancestors = &ClassData::ancestors;

  traversal(all, fn, ancestors);
}

void topDownTraversal(const ClassAnalysis& all, ClassAnalysisConstFn fn)
{
  RelationAccessorConst ancestors = &ClassData::ancestors;

  traversal(all, fn, ancestors);
}

void bottomUpTraversal(ClassAnalysis& all, ClassAnalysisFn fn)
{
  RelationAccessor descendants = &ClassData::descendants;

  traversal(all, fn, descendants);
}

void bottomUpTraversal(const ClassAnalysis& all, ClassAnalysisConstFn fn)
{
  RelationAccessorConst descendants = &ClassData::descendants;

  traversal(all, fn, descendants);
}

void unorderedTraversal(ClassAnalysis& all, ClassAnalysisFn fn)
{
  for (ClassAnalysis::value_type& elem : all)
    fn(elem);
}

void unorderedTraversal(const ClassAnalysis& all, ClassAnalysisConstFn fn)
{
  for (const ClassAnalysis::value_type& elem : all)
    fn(elem);
}

bool ClassData::hasVirtualFunctions() const
{
  return declaresVirtualFunctions() || inheritsVirtualFunctions();
}

bool ClassData::hasVirtualInheritance() const
{
  ClassData::AncestorContainer::const_iterator aa = ancestors().begin();
  ClassData::AncestorContainer::const_iterator zz = ancestors().end();
  auto isVirtualInheritance = [](const InheritanceDesc& desc) -> bool
                              {
                                return desc.isVirtual();
                              };

  return zz != std::find_if(aa, zz, isVirtualInheritance);
}

bool ClassData::hasVirtualTable() const
{
  return hasVirtualFunctions() || hasVirtualInheritance();
}

}



namespace
{
  bool inheritsVirtualFunctions(const rca::ClassAnalysis& classes, const rca::ClassAnalysis::value_type& entry)
  {
    const std::vector<rca::InheritanceDesc>&          parents = entry.second.ancestors();
    std::vector<rca::InheritanceDesc>::const_iterator aa = parents.begin();
    std::vector<rca::InheritanceDesc>::const_iterator zz = parents.end();

    while (aa != zz && !classes.at(aa->getClass()).hasVirtualFunctions())
      ++aa;

    return aa != zz;
  }

  void integrateIndirectInheritance(rca::ClassAnalysis& classes, rca::ClassAnalysis::value_type& entry)
  {
    std::vector<rca::InheritanceDesc>  tmp;

    // collect additional ancestors
    for (rca::InheritanceDesc& parent : entry.second.ancestors())
      for (rca::InheritanceDesc ancestor : classes.at(parent.getClass()).ancestors())
      {
        // skip virtual bases (they have already been propagated to derived)
        if (ancestor.isVirtual()) continue;

        ancestor.setDirect(false);
        tmp.push_back(ancestor);
      }

    // add additional ancestors
    for (rca::InheritanceDesc ancestor : tmp)
    {
      classes.addInheritanceEdge(entry, ancestor);
    }
  }

  struct UniqueVirtualInheritancePredicate
  {
    bool operator()(const rca::InheritanceDesc& lhs, const rca::InheritanceDesc& rhs)
    {
      return lhs.getClass() == rhs.getClass();
    }
  };

  struct VirtualInheritanceComparator
  {
    bool operator()(const rca::InheritanceDesc& lhs, const rca::InheritanceDesc& rhs)
    {
      // only virtual inheritance is considered
      assert(lhs.isVirtual() && rhs.isVirtual());

      // at most one can be a direct base
      assert((rhs.isDirect() ^ rhs.isDirect()) <= 1);

      if (lhs.getClass() < rhs.getClass())
       return true;

      if (lhs.getClass() > rhs.getClass())
       return false;

      return lhs.isDirect();
    }
  };


  void copyVirtualInhertanceToDerived(rca::ClassAnalysis& classes, rca::ClassAnalysis::value_type& entry)
  {
    using iterator = std::vector<rca::InheritanceDesc>::iterator;

    std::vector<rca::InheritanceDesc> tmp;

    // collect addition ancestors
    for (rca::InheritanceDesc& parent : entry.second.ancestors())
    {
      if (parent.isVirtual())
        tmp.push_back(parent);

      for (rca::InheritanceDesc ancestor : classes.at(parent.getClass()).ancestors())
      {
        if (!ancestor.isVirtual()) continue;

        ancestor.setDirect(false);
        tmp.push_back(ancestor);
      }
    }

    std::sort(tmp.begin(), tmp.end(), VirtualInheritanceComparator{});

    iterator zz = std::unique(tmp.begin(), tmp.end(), UniqueVirtualInheritancePredicate{});

    // add additional ancestors
    std::for_each( tmp.begin(), zz,
                   [&classes, &entry](const rca::InheritanceDesc& ancestor) -> void
                   {
                     if (!ancestor.isDirect())
                       classes.addInheritanceEdge(entry, ancestor);
                   }
                 );
  }

  // TODO: Migrate the virtual base class initialization order to the C++
  // specific processing (CppCompatibility::languageSpecificProcessing).
  // Construction/destruction order of virtual bases is a C++ object model
  // concept.  For the bytecode languages the "virtual" edges model interfaces,
  // which have no instance state and no constructors (JVM interface
  // initialization is not even triggered by subclass initialization unless
  // default methods are present, JVMS 5.5), so the order computed here is
  // meaningless for JVM/CIL and should not be part of the common analysis.
  void computeVirtualBaseInitializationOrder(rca::ClassAnalysis& classes, rca::ClassAnalysis::value_type& entry)
  {
    std::unordered_set<rca::ClassKeyType> alreadySeen;
    std::vector<rca::ClassKeyType>&       initorder = entry.second.virtualBaseClassOrder();

    // traverse direct ancestors and collect their initialization orders
    for (rca::InheritanceDesc& parentDesc : entry.second.ancestors() | adapt::filtered(rca::IsDirect{}))
    {
      rca::ClassKeyType                     parent = parentDesc.getClass();
      const std::vector<rca::ClassKeyType>& parentInit = classes.at(parent).virtualBaseClassOrder();

      // add the virtual bases that have not been seen before
      for (rca::ClassKeyType vbase : parentInit)
        if (alreadySeen.insert(vbase).second)
          initorder.push_back(vbase);

      if (parentDesc.isVirtual() && alreadySeen.insert(parent).second)
        initorder.push_back(parent);
    }
  }

  void analyzeClassRelationships(rca::ClassAnalysis& all)
  {
    msgTrace() << "analyzing classes " << all.size() << std::endl;

    auto propagateVirtualInheritance =
           [&all](rca::ClassAnalysis::value_type& rep) -> void
           {
             copyVirtualInhertanceToDerived(all, rep);
           };
    topDownTraversal(all, propagateVirtualInheritance);

    // TODO: move to CppCompatibility::languageSpecificProcessing; see the note
    //       at computeVirtualBaseInitializationOrder.
    auto computeVirtualBaseClassInitializationOrder =
           [&all](rca::ClassAnalysis::value_type& rep) -> void
           {
             computeVirtualBaseInitializationOrder(all, rep);
           };
    topDownTraversal(all, computeVirtualBaseClassInitializationOrder);

    auto flattenInheritance =
           [&all](rca::ClassAnalysis::value_type& rep) -> void
           {
             integrateIndirectInheritance(all, rep);
           };
    topDownTraversal(all, flattenInheritance);

    auto analyzeInheritedVirtualMethod =
           [&all](rca::ClassAnalysis::value_type& rep) -> void
           {
             rep.second.inheritsVirtualFunctions(inheritsVirtualFunctions(all, rep));
           };
    topDownTraversal(all, analyzeInheritedVirtualMethod);
  }

  void inheritanceRelations(rca::ClassAnalysis& classes)
  {
    const rca::AbstractCompatibilityBridge& compat = classes.compatibilityBridge();

    for (rca::ClassAnalysis::value_type& elem : classes)
    {
      auto addEdge =
             [&elem, &classes](rca::ClassKeyType child, rca::ClassKeyType parent, bool virt) -> void
             {
               assert(elem.first == child);

               classes.addInheritanceEdge(elem, parent, virt, true /* direct ancestor */);
             };

      compat.inheritanceEdges(elem.first, addEdge);
    }
  }

  // returns true iff lhs < rhs
  struct VFNameTypeOrder
  {
    bool operator()(rca::FunctionKeyType lhs, rca::FunctionKeyType rhs) const
    {
      static constexpr bool firstDecisiveComparison = false;

      int res = 0;

      firstDecisiveComparison
      || (res = compat.compareNames(lhs, rhs))
      || (res = compat.compareFunctionsByType(lhs, rhs))
      ;

      return res < 0;
    }
/*
    template <class TupleWithFunctionKeyType>
    bool operator()(const TupleWithFunctionKeyType& lhs, const TupleWithFunctionKeyType& rhs) const
    {
      return (*this)(std::get<0>(lhs), std::get<0>(rhs));
    }
*/
    const rca::AbstractCompatibilityBridge& compat;
  };

  /// a signature match that the bridge rejected as direct override and that
  /// may still hold transitively through an intermediate overrider.
  struct DeferredOverride
  {
    rca::FunctionKeyType drv;
    rca::FunctionKeyType bas;
    rca::ClassKeyType    witness;
  };

  using DeferredOverrides = std::vector<DeferredOverride>;

  /// records \p drv as overrider of \p bas after checking the return types.
  void recordOverride( const rca::AbstractCompatibilityBridge& compat,
                       const rca::ClassAnalysis& classes,
                       rca::VirtualFunctionAnalysis& vfunAnalysis,
                       rca::FunctionKeyType drv,
                       rca::FunctionKeyType bas,
                       rca::ClassKeyType witness
                     )
  {
    const rca::FunctionTypeRelation rel = compat.haveSameOrCovariantReturn(classes, bas, drv);

    if (rel == rca::FunctionTypeRelation::unrelated)
    {
      msgError() << "oops, covariant return assertion failed: "
                 << compat.nameOf(bas) << " <> " << compat.nameOf(drv)
                 << std::endl;
      return;
    }

    ASSERT_require(rel != rca::FunctionTypeRelation::unrelated);
    //~ std::cerr << compat.nameOf(drv) << " overrides " << compat.nameOf(bas)
              //~ << std::endl;

    const bool covariant = rel == rca::FunctionTypeRelation::covariant;

    vfunAnalysis.at(drv).overridden().emplace_back(bas, covariant, witness);
    vfunAnalysis.at(bas).overriders().emplace_back(drv, covariant, witness);
  }

  struct ComputeVirtualFunctionRelation
  {
    void operator()(rca::FunctionKeyType drv, rca::FunctionKeyType bas) const
    {
      // implied overrides: the inherited function must qualify as an implementation
      if (rca::isValidKey(witness) && !compat.isInheritableImplementation(drv))
        return;

      // language rules beyond the signature (e.g., access); may still hold transitively
      if (!compat.canOverrideInContext(classes, drvClass, drv, basClass, bas))
      {
        deferred.push_back(DeferredOverride{drv, bas, witness});
        return;
      }

      recordOverride(compat, classes, vfunAnalysis, drv, bas, witness);
    }

    void operator()(rca::FunctionKeyType, unavailable_t) const {}

    void operator()(unavailable_t, rca::FunctionKeyType) const {}

    // data members
    const rca::AbstractCompatibilityBridge& compat;
    const rca::ClassAnalysis&       classes;
    rca::VirtualFunctionAnalysis&   vfunAnalysis;
    DeferredOverrides&             deferred;

    /// witness class of an implied override; invalid for declared overrides.
    rca::ClassKeyType               witness = rca::ClassKeyType{};

    /// class that declares the derived function; invalid for implied overrides.
    rca::ClassKeyType               drvClass = rca::ClassKeyType{};

    /// class that declares the base function.
    rca::ClassKeyType               basClass = rca::ClassKeyType{};
  };

  /// returns true if \p drv overrides a function whose recorded overridden
  /// functions include \p bas.
  /// \details
  ///   Classes are processed top-down, so the overridden() lists of functions in
  ///   ancestor classes are complete, including their own transitive entries.
  ///   One intermediate level therefore suffices.
  bool overridesTransitively( const rca::VirtualFunctionAnalysis& vfunAnalysis,
                              rca::FunctionKeyType drv,
                              rca::FunctionKeyType bas
                            )
  {
    auto isBas = [bas](const rca::OverrideDesc& desc) -> bool { return desc.function() == bas; };

    for (const rca::OverrideDesc& mid : vfunAnalysis.at(drv).overridden())
    {
      const rca::OverrideContainer& midOverridden = vfunAnalysis.at(mid.function()).overridden();

      if (std::find_if(midOverridden.begin(), midOverridden.end(), isBas) != midOverridden.end())
        return true;
    }

    return false;
  }

  /// records the deferred pairs that hold transitively (fixpoint, since a
  /// resolved pair can enable another one of the same derived function).
  void resolveDeferredOverrides( const rca::AbstractCompatibilityBridge& compat,
                                 const rca::ClassAnalysis& classes,
                                 rca::VirtualFunctionAnalysis& vfunAnalysis,
                                 DeferredOverrides& deferred
                               )
  {
    bool progress = true;

    while (progress && !deferred.empty())
    {
      DeferredOverrides remaining;

      progress = false;

      for (const DeferredOverride& cand : deferred)
      {
        if (overridesTransitively(vfunAnalysis, cand.drv, cand.bas))
        {
          recordOverride(compat, classes, vfunAnalysis, cand.drv, cand.bas, cand.witness);
          progress = true;
        }
        else
        {
          remaining.push_back(cand);
        }
      }

      deferred.swap(remaining);
    }
  }

  using VirtualFunctionContainer     = rca::ClassData::VirtualFunctionContainer;
  using SortedVirtualMemberFunctions = std::unordered_map<rca::ClassKeyType, VirtualFunctionContainer>;

  /// per-class function lists maintained during the override computation.
  struct VirtualFunctionTables
  {
    /// functions declared by a class, sorted by VFNameTypeOrder.
    SortedVirtualMemberFunctions declared;

    /// functions declared by a class plus the functions inherited along the
    /// non-virtual inheritance chain that the class does not override, sorted
    /// by VFNameTypeOrder (a vtable view). Only maintained when the bridge
    /// enables inheritedMethodsImplementVirtualBaseMethods.
    SortedVirtualMemberFunctions effective;
  };

  /// merge operation that builds the effective function list of a class from
  /// its declared functions and the effective functions of its direct
  /// non-virtual base. Declared functions take precedence; functions only
  /// present in the base are collected separately as inherited.
  struct MaterializeInheritedFunctions
  {
    void operator()(rca::FunctionKeyType declared, rca::FunctionKeyType /*overridden*/) const
    {
      effective.push_back(declared);
    }

    void operator()(rca::FunctionKeyType declared, unavailable_t) const
    {
      effective.push_back(declared);
    }

    void operator()(unavailable_t, rca::FunctionKeyType inheritedFn) const
    {
      effective.push_back(inheritedFn);
      inherited.push_back(inheritedFn);
    }

    VirtualFunctionContainer& effective;
    VirtualFunctionContainer& inherited;
  };

  std::vector<rca::ClassKeyType>
  directNonVirtualBases(const rca::ClassAnalysis::value_type& entry)
  {
    std::vector<rca::ClassKeyType> res;

    for (const rca::InheritanceDesc& desc : entry.second.ancestors())
      if (desc.isDirect() && !desc.isVirtual())
        res.push_back(desc.getClass());

    return res;
  }

  /// records implied overrides for \p entry: functions inherited along the
  /// non-virtual chain that implement functions of virtual bases (see
  /// AbstractCompatibilityBridge::inheritedMethodsImplementVirtualBaseMethods).
  /// \details
  ///   1. effective(C) = declared(C) ∪ (effective(B) \ overridden by C), with
  ///      B the direct non-virtual base; the functions only present in
  ///      effective(B) are the inherited functions of C.
  ///   2. the inherited functions are matched against the declared functions of
  ///      every virtual base of C that is not already a base of B. Virtual
  ///      bases known to B were handled when B (or one of its bases) was
  ///      processed, so each implied override is recorded exactly once, at the
  ///      class where the inherited function and the requirement first meet.
  void computeImpliedOverriders( const rca::ClassAnalysis& classes,
                                 rca::VirtualFunctionAnalysis& vfunAnalysis,
                                 const rca::ClassAnalysis::value_type& entry,
                                 VirtualFunctionTables& tables,
                                 DeferredOverrides& deferred
                               )
  {
    const rca::AbstractCompatibilityBridge& compat = classes.compatibilityBridge();
    const VirtualFunctionContainer&        declared = tables.declared.at(entry.first);
    VirtualFunctionContainer&              effective = tables.effective[entry.first];

    ASSERT_require(effective.empty());

    const std::vector<rca::ClassKeyType>    bases = directNonVirtualBases(entry);

    if (bases.size() != 1)
    {
      if (bases.size() > 1)
        msgWarn() << "class " << compat.nameOf(entry.first)
                  << " has " << bases.size() << " direct non-virtual bases;"
                  << " implied overrides are not computed for it."
                  << std::endl;

      effective = declared;
      return;
    }

    const rca::ClassKeyType                 base = bases.front();
    const VirtualFunctionContainer&        baseEffective = tables.effective.at(base);
    VirtualFunctionContainer               inherited;

    mergeOrderedSequences( declared.begin(), declared.end(),
                           baseEffective.begin(), baseEffective.end(),
                           MaterializeInheritedFunctions{effective, inherited},
                           VFNameTypeOrder{compat}
                         );

    if (inherited.empty())
      return;

    // virtual bases already known to the direct base
    std::unordered_set<rca::ClassKeyType>   knownToBase;

    for (const rca::InheritanceDesc& desc : classes.at(base).ancestors())
      knownToBase.insert(desc.getClass());

    for (const rca::InheritanceDesc& parentDesc : entry.second.ancestors())
    {
      if (!parentDesc.isVirtual() || knownToBase.count(parentDesc.getClass()))
        continue;

      const VirtualFunctionContainer& parentDeclared = tables.declared.at(parentDesc.getClass());

      mergeOrderedSequences( inherited.begin(), inherited.end(),
                             parentDeclared.begin(), parentDeclared.end(),
                             ComputeVirtualFunctionRelation{compat, classes, vfunAnalysis, deferred, entry.first /* witness */},
                             VFNameTypeOrder{compat}
                           );
    }
  }

  void computeOverriders( const rca::ClassAnalysis& classes,
                          rca::VirtualFunctionAnalysis& vfunAnalysis,
                          const rca::ClassAnalysis::value_type& entry,
                          bool /*normalizedSignature*/,
                          VirtualFunctionTables& tables
                        )
  {
    using AbstractCompatibilityBridge = rca::AbstractCompatibilityBridge;

    // create a new entry
    AbstractCompatibilityBridge const& compat = classes.compatibilityBridge();
    VirtualFunctionContainer&          vfunSorted = tables.declared[entry.first];

    ASSERT_require(vfunSorted.empty());

    vfunSorted = entry.second.virtualFunctions();
    std::sort(vfunSorted.begin(), vfunSorted.end(), VFNameTypeOrder{ compat });

    std::for_each( vfunSorted.begin(), vfunSorted.end(),
                   [&vfunAnalysis, &entry, &compat](rca::FunctionKeyType id) -> void
                   {
                     vfunAnalysis.emplace(id, rca::VirtualFunctionDesc{entry.first, compat.isPureVirtual(id)});
                     vfunAnalysis.at(id).isFinal(compat.isFinal(id));
                   }
                 );

    // signature matches rejected by the bridge's direct rule; resolved transitively below
    DeferredOverrides deferred;

    // TODO(CIL): CilCompatibility implements none of canOverride,
    //   inheritedMethodsImplementVirtualBaseMethods, and
    //   isInheritableImplementation, so name and signature matching alone
    //   decides and CIL slot semantics are not respected. canOverride(drv, bas)
    //   sees only the two functions. A reuse-slot method is related to every
    //   flattened ancestor with the same signature, so rejecting a pair that
    //   crosses a newslot or final method requires the bridge to walk the
    //   base chain itself, or this hook to receive the ClassAnalysis. The
    //   transitive rule below cannot restore hidden pairs, which is correct.
    //   Explicit overrides (MethodImpl) are never matched, because they do not
    //   rely on name equality and no hook can add a pair.
    //   \code{.cs}
    //   class A { public virtual void F() {} }
    //   class B : A { public new virtual void F() {} }  // newslot: hides A.F
    //   class C : B { public override void F() {} }     // overrides B.F only; related to A.F here
    //   \endcode
    // declared overrides: functions declared by this class against functions
    // declared by its ancestors; non-virtual ancestors first, then virtual ones.
    // PRIORITY: HIGH
    auto declaredOverrides =
        [&](bool virtualAncestors) -> void
        {
          for (const rca::InheritanceDesc& parentDesc : entry.second.ancestors())
          {
            if (parentDesc.isVirtual() != virtualAncestors)
              continue;

            const VirtualFunctionContainer& parentVFunSorted = tables.declared.at(parentDesc.getClass());

            mergeOrderedSequences( vfunSorted.begin(), vfunSorted.end(),
                                   parentVFunSorted.begin(), parentVFunSorted.end(),
                                   ComputeVirtualFunctionRelation{compat, classes, vfunAnalysis, deferred, rca::ClassKeyType{} /* no witness */, entry.first, parentDesc.getClass()},
                                   VFNameTypeOrder{compat}
                                 );
          }
        };

    declaredOverrides(false /* non-virtual ancestors */);
    declaredOverrides(true  /* virtual ancestors */);

    // implied overrides (language dependent)
    if (compat.inheritedMethodsImplementVirtualBaseMethods())
      computeImpliedOverriders(classes, vfunAnalysis, entry, tables, deferred);

    // transitive rule for pairs the bridge rejected directly
    resolveDeferredOverrides(compat, classes, vfunAnalysis, deferred);
  }
}

namespace Rose::Common::Analysis
{

VirtualFunctionAnalysis
analyzeVirtualFunctions(const ClassAnalysis& all, bool normalizedSignature)
{
  VirtualFunctionAnalysis      res{all.compatibilityBridge()};
  VirtualFunctionTables        tables;

  topDownTraversal( all,
                    [&all, &res, &tables, &normalizedSignature]
                    (const ClassAnalysis::value_type& clrep) -> void
                    {
                      computeOverriders(all, res, clrep, normalizedSignature, tables);
                    }
                  );

  return res;
}

void analyzeClassesCommon(ClassAnalysis& classes)
{
  const rca::AbstractCompatibilityBridge& compat = classes.compatibilityBridge();

  inheritanceRelations(classes);
  analyzeClassRelationships(classes);

  // Hook for analyses that need language-specific post-processing.
  compat.languageSpecificProcessing()(classes);
}

void analyzeClasses(ClassAnalysis& classes, ASTRootType n)
{
  const rca::AbstractCompatibilityBridge& compat = classes.compatibilityBridge();

  classes.containsAllClasses(true);
  compat.extractFromProject(classes, n);
  analyzeClassesCommon(classes);
}

void analyzeClassesFromMemoryPool(ClassAnalysis& classes)
{
  const rca::AbstractCompatibilityBridge& compat = classes.compatibilityBridge();

  classes.containsAllClasses(true);
  compat.extractFromMemoryPool(classes);
  analyzeClassesCommon(classes);
}


#if 0
ClassAnalysis analyzeClass(ClassKeyType n)
{
  AbstractCompatibilityBridge compat;
  ClassAnalysis       classes{false /* incomplete view */};

  // collect all classes reachable upwards from n
  compat.extractClassAndBaseClasses(classes, n);

  inheritanceRelations(classes);
  analyzeClassRelationships(classes);

  return classes;
}
#endif /*0*/


VirtualFunctionAnalysis::VirtualFunctionAnalysis(const AbstractCompatibilityBridge& compatibilityBridge)
: compat(compatibilityBridge)
{}

VirtualFunctionAnalysis::mapped_type&
VirtualFunctionAnalysis::at(const VirtualFunctionAnalysis::key_type& k)
{
  const rca::AbstractCompatibilityBridge& compat = this->compatibilityBridge();

  return lookup(compat, *this, k).second;
}

const VirtualFunctionAnalysis::mapped_type&
VirtualFunctionAnalysis::at(const VirtualFunctionAnalysis::key_type& k) const
{
  const rca::AbstractCompatibilityBridge& compat = this->compatibilityBridge();

  return lookup(compat, *this, k).second;
}

FunctionPredicate
VirtualFunctionAnalysis::virtualFunctionTest() const
{
  return [self = this](FunctionKeyType key) -> bool
         {
           return self->count(key) != 0;
         };
}
}
