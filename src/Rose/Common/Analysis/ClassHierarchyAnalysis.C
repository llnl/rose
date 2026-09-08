#include <Sawyer/Assert.h>

#include <Rose/Common/Analysis/ClassHierarchyAnalysis.h>
#include <Rose/Common/Analysis/Utility.h>

#include <algorithm>
#include <unordered_set>
#include <cassert>
#include <iterator>

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>

namespace ga = Rose::Common::Analysis;
namespace adapt = boost::adaptors;


namespace
{
  [[noreturn]]
  void
  reportAndThrowError( const ga::AbstractCompatibilityBridge&,
                       ga::ClassKeyType key,
                       const ga::ClassAnalysis& m
                     )
  {
    msgError() << m.classInfo(key) << std::endl;

    if (m.size() == 0)
      msgError() << "\nNote, the class hierarchy database is empty." << std::endl;

    throw std::out_of_range("Unable to find key in ClassAnalysis.");
  }

  [[noreturn]]
  void
  reportAndThrowError( const ga::AbstractCompatibilityBridge& compat,
                       ga::FunctionKeyType key,
                       const ga::VirtualFunctionAnalysis& m
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
  lookup( const ga::AbstractCompatibilityBridge& compat,
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
  const ga::AbstractCompatibilityBridge& compat = this->compatibilityBridge();

  return lookup(compat, *this, k).second;
}

const ClassAnalysis::mapped_type&
ClassAnalysis::at(const ClassAnalysis::key_type& k) const
{
  const ga::AbstractCompatibilityBridge& compat = this->compatibilityBridge();

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
  using RelationAccessor = std::vector<ga::InheritanceDesc>& (ga::ClassData::*)();
  using RelationAccessorConst = const std::vector<ga::InheritanceDesc>& (ga::ClassData::*)() const;

  struct GetClass
  {
    ga::ClassKeyType operator()(const ga::InheritanceDesc& desc) const
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
             return (elem.second.*selector)() | adapt::filtered(ga::IsDirect{}) | adapt::transformed(GetClass{});
           };
  }

  template <class AnalysisT, class Fn, class Selector, class Set>
  void traversal(AnalysisT& all, Fn fn, ga::ClassKeyType curr, Selector selector, Set& explored)
  {
    const ga::AbstractCompatibilityBridge& compat = all.compatibilityBridge();

    if (!explored.insert(curr).second)
      return;

    auto& elem = lookup(compat, all, curr);

    for (ga::ClassKeyType clazz : selector(elem))
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
    std::unordered_set<ga::ClassKeyType> explored;

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
  bool inheritsVirtualFunctions(const ga::ClassAnalysis& classes, const ga::ClassAnalysis::value_type& entry)
  {
    const std::vector<ga::InheritanceDesc>&          parents = entry.second.ancestors();
    std::vector<ga::InheritanceDesc>::const_iterator aa = parents.begin();
    std::vector<ga::InheritanceDesc>::const_iterator zz = parents.end();

    while (aa != zz && !classes.at(aa->getClass()).hasVirtualFunctions())
      ++aa;

    return aa != zz;
  }

  void integrateIndirectInheritance(ga::ClassAnalysis& classes, ga::ClassAnalysis::value_type& entry)
  {
    std::vector<ga::InheritanceDesc>  tmp;

    // collect additional ancestors
    for (ga::InheritanceDesc& parent : entry.second.ancestors())
      for (ga::InheritanceDesc ancestor : classes.at(parent.getClass()).ancestors())
      {
        // skip virtual bases (they have already been propagated to derived)
        if (ancestor.isVirtual()) continue;

        ancestor.setDirect(false);
        tmp.push_back(ancestor);
      }

    // add additional ancestors
    for (ga::InheritanceDesc ancestor : tmp)
    {
      classes.addInheritanceEdge(entry, ancestor);
    }
  }

  struct UniqueVirtualInheritancePredicate
  {
    bool operator()(const ga::InheritanceDesc& lhs, const ga::InheritanceDesc& rhs)
    {
      return lhs.getClass() == rhs.getClass();
    }
  };

  struct VirtualInheritanceComparator
  {
    bool operator()(const ga::InheritanceDesc& lhs, const ga::InheritanceDesc& rhs)
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


  void copyVirtualInhertanceToDerived(ga::ClassAnalysis& classes, ga::ClassAnalysis::value_type& entry)
  {
    using iterator = std::vector<ga::InheritanceDesc>::iterator;

    std::vector<ga::InheritanceDesc> tmp;

    // collect addition ancestors
    for (ga::InheritanceDesc& parent : entry.second.ancestors())
    {
      if (parent.isVirtual())
        tmp.push_back(parent);

      for (ga::InheritanceDesc ancestor : classes.at(parent.getClass()).ancestors())
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
                   [&classes, &entry](const ga::InheritanceDesc& ancestor) -> void
                   {
                     if (!ancestor.isDirect())
                       classes.addInheritanceEdge(entry, ancestor);
                   }
                 );
  }

  void computeVirtualBaseInitializationOrder(ga::ClassAnalysis& classes, ga::ClassAnalysis::value_type& entry)
  {
    std::unordered_set<ga::ClassKeyType> alreadySeen;
    std::vector<ga::ClassKeyType>&       initorder = entry.second.virtualBaseClassOrder();

    // traverse direct ancestors and collect their initialization orders
    for (ga::InheritanceDesc& parentDesc : entry.second.ancestors() | adapt::filtered(ga::IsDirect{}))
    {
      ga::ClassKeyType                     parent = parentDesc.getClass();
      const std::vector<ga::ClassKeyType>& parentInit = classes.at(parent).virtualBaseClassOrder();

      // add the virtual bases that have not been seen before
      for (ga::ClassKeyType vbase : parentInit)
        if (alreadySeen.insert(vbase).second)
          initorder.push_back(vbase);

      if (parentDesc.isVirtual() && alreadySeen.insert(parent).second)
        initorder.push_back(parent);
    }
  }

  void analyzeClassRelationships(ga::ClassAnalysis& all)
  {
    msgTrace() << "analyzing classes " << all.size() << std::endl;

    auto propagateVirtualInheritance =
           [&all](ga::ClassAnalysis::value_type& rep) -> void
           {
             copyVirtualInhertanceToDerived(all, rep);
           };
    topDownTraversal(all, propagateVirtualInheritance);

    auto computeVirtualBaseClassInitializationOrder =
           [&all](ga::ClassAnalysis::value_type& rep) -> void
           {
             computeVirtualBaseInitializationOrder(all, rep);
           };
    topDownTraversal(all, computeVirtualBaseClassInitializationOrder);

    auto flattenInheritance =
           [&all](ga::ClassAnalysis::value_type& rep) -> void
           {
             integrateIndirectInheritance(all, rep);
           };
    topDownTraversal(all, flattenInheritance);

    auto analyzeInheritedVirtualMethod =
           [&all](ga::ClassAnalysis::value_type& rep) -> void
           {
             rep.second.inheritsVirtualFunctions(inheritsVirtualFunctions(all, rep));
           };
    topDownTraversal(all, analyzeInheritedVirtualMethod);
  }

  void inheritanceRelations(ga::ClassAnalysis& classes)
  {
    const ga::AbstractCompatibilityBridge& compat = classes.compatibilityBridge();

    for (ga::ClassAnalysis::value_type& elem : classes)
    {
      auto addEdge =
             [&elem, &classes](ga::ClassKeyType child, ga::ClassKeyType parent, bool virt) -> void
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
    bool operator()(ga::FunctionKeyType lhs, ga::FunctionKeyType rhs) const
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
    const ga::AbstractCompatibilityBridge& compat;
  };

  struct ComputeVirtualFunctionRelation
  {
    void operator()(ga::FunctionKeyType drv, ga::FunctionKeyType bas) const
    {
      const ga::FunctionTypeRelation rel = compat.haveSameOrCovariantReturn(classes, bas, drv);

      if (rel == ga::FunctionTypeRelation::unrelated)
      {
        msgError() << "oops, covariant return assertion failed: "
                   << compat.nameOf(bas) << " <> " << compat.nameOf(drv)
                   << std::endl;
        return;
      }

      ASSERT_require(rel != ga::FunctionTypeRelation::unrelated);
      //~ std::cerr << compat.nameOf(drv) << " overrides " << compat.nameOf(bas)
                //~ << std::endl;

      const bool               covariant = rel == ga::FunctionTypeRelation::covariant;

      vfunAnalysis.at(drv).overridden().emplace_back(bas, covariant);
      vfunAnalysis.at(bas).overriders().emplace_back(drv, covariant);
    }

    void operator()(ga::FunctionKeyType, unavailable_t) const {}

    void operator()(unavailable_t, ga::FunctionKeyType) const {}

    // data members
    const ga::AbstractCompatibilityBridge& compat;
    const ga::ClassAnalysis&       classes;
    ga::VirtualFunctionAnalysis&   vfunAnalysis;
    ga::ClassKeyType               ancestor;
    ga::ClassKeyType               descendant;
  };

  using SortedVirtualMemberFunctions = std::unordered_map<ga::ClassKeyType, ga::ClassData::VirtualFunctionContainer>;

  void computeOverriders( const ga::ClassAnalysis& classes,
                          ga::VirtualFunctionAnalysis& vfunAnalysis,
                          const ga::ClassAnalysis::value_type& entry,
                          bool /*normalizedSignature*/,
                          SortedVirtualMemberFunctions& sortedVFunMap
                        )
  {
    using VirtualFunctionContainer    = ga::ClassData::VirtualFunctionContainer;
    using AbstractCompatibilityBridge = ga::AbstractCompatibilityBridge;

    // create a new entry
    AbstractCompatibilityBridge const& compat = classes.compatibilityBridge();
    VirtualFunctionContainer&          vfunSorted = sortedVFunMap[entry.first];

    ASSERT_require(vfunSorted.empty());

    vfunSorted = entry.second.virtualFunctions();
    std::sort(vfunSorted.begin(), vfunSorted.end(), VFNameTypeOrder{ compat });

    std::for_each( vfunSorted.begin(), vfunSorted.end(),
                   [&vfunAnalysis, &entry, &compat](ga::FunctionKeyType id) -> void
                   {
                     vfunAnalysis.emplace(id, ga::VirtualFunctionDesc{entry.first, compat.isPureVirtual(id)});
                   }
                 );

    for (const ga::InheritanceDesc& parentDesc : entry.second.ancestors())
    {
      using ComputeRelation = ComputeVirtualFunctionRelation;

      VirtualFunctionContainer& parentVFunSorted = sortedVFunMap.at(parentDesc.getClass());

      mergeOrderedSequences( vfunSorted.begin(), vfunSorted.end(),
                             parentVFunSorted.begin(), parentVFunSorted.end(),
                             ComputeRelation{compat, classes, vfunAnalysis, entry.first, parentDesc.getClass()},
                             VFNameTypeOrder{compat}
                           );
    }
  }
}

namespace Rose::Common::Analysis
{

VirtualFunctionAnalysis
analyzeVirtualFunctions(const ClassAnalysis& all, bool normalizedSignature)
{
  VirtualFunctionAnalysis      res{all.compatibilityBridge()};
  SortedVirtualMemberFunctions tmpSorted;

  topDownTraversal( all,
                    [&all, &res, &tmpSorted, &normalizedSignature]
                    (const ClassAnalysis::value_type& clrep) -> void
                    {
                      computeOverriders(all, res, clrep, normalizedSignature, tmpSorted);
                    }
                  );

  return res;
}

void analyzeClassesCommon(ClassAnalysis& classes)
{
  const ga::AbstractCompatibilityBridge& compat = classes.compatibilityBridge();

  inheritanceRelations(classes);
  analyzeClassRelationships(classes);

  // Hook for analyses that need language-specific post-processing.
  compat.languageSpecificProcessing()(classes);
}

void analyzeClasses(ClassAnalysis& classes, ASTRootType n)
{
  const ga::AbstractCompatibilityBridge& compat = classes.compatibilityBridge();

  classes.containsAllClasses(true);
  classes.languageStandard(compat.languageStandard());
  compat.extractFromProject(classes, n);
  analyzeClassesCommon(classes);
}

void analyzeClassesFromMemoryPool(ClassAnalysis& classes)
{
  const ga::AbstractCompatibilityBridge& compat = classes.compatibilityBridge();

  classes.containsAllClasses(true);
  classes.languageStandard(compat.languageStandard());
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
  const ga::AbstractCompatibilityBridge& compat = this->compatibilityBridge();

  return lookup(compat, *this, k).second;
}

const VirtualFunctionAnalysis::mapped_type&
VirtualFunctionAnalysis::at(const VirtualFunctionAnalysis::key_type& k) const
{
  const ga::AbstractCompatibilityBridge& compat = this->compatibilityBridge();

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
