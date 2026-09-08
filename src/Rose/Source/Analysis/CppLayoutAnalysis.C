#include <Rose/Source/Analysis/CppLayoutAnalysis.h>

#include <algorithm>
#include <numeric>
#include <stdexcept>

#include <Sawyer/Assert.h>

namespace ga = Rose::Common::Analysis;

namespace
{
  void addDataMembers(Rose::Source::Analysis::ObjectLayout& ol, const ga::ClassAnalysis::value_type& clazz)
  {
    for (ga::VariableKeyType id : clazz.second.dataMembers())
      ol.emplace_back(0, Rose::Source::Analysis::Field{id});
  }

  struct PrimarySubobjectFinder
  {
    bool operator()(const ga::InheritanceDesc& cand) const
    {
      return !cand.isVirtual()
          && cand.isDirect()
          && all->at(cand.getClass()).hasVirtualTable();
    }

    const ga::ClassAnalysis* all;
  };

  Rose::Source::Analysis::ObjectLayout
  computeObjectLayout(const ga::ClassAnalysis& all,
                      const Rose::Source::Analysis::ObjectLayoutContainer&,
                      const ga::ClassAnalysis::value_type& clazz)
  {
    using Vec = std::vector<ga::InheritanceDesc>;

    static constexpr bool virtually = true;
    static constexpr bool directly  = true;

    Rose::Source::Analysis::ObjectLayout res;
    res.abstractClass(clazz.second.abstractClass());

    PrimarySubobjectFinder finder{&all};
    const Vec& parents = clazz.second.ancestors();
    const Vec::const_iterator aa = parents.begin();
    const Vec::const_iterator zz = parents.end();
    const Vec::const_iterator primary = std::find_if(aa, zz, finder);

    if (primary != zz)
      res.emplace_back(0, Rose::Source::Analysis::Subobject{primary->getClass(), !virtually, directly});
    else if (clazz.second.hasVirtualTable())
      res.emplace_back(0, Rose::Source::Analysis::VTable{clazz.first, true});

    for (Vec::const_iterator it = aa; it != zz; ++it)
    {
      if (it->isVirtual() || !it->isDirect() || all.at(it->getClass()).hasVirtualTable())
        continue;

      res.emplace_back(0, Rose::Source::Analysis::Subobject{it->getClass(), !virtually, directly});
    }

    addDataMembers(res, clazz);

    for (Vec::const_iterator it = (primary == zz ? zz : primary + 1); it != zz; ++it)
    {
      if (finder(*it))
        res.emplace_back(0, Rose::Source::Analysis::Subobject{it->getClass(), !virtually, directly});
    }

    for (Vec::const_iterator it = aa; it != zz; ++it)
    {
      if (it->isVirtual())
        res.emplace_back(0, Rose::Source::Analysis::Subobject{it->getClass(), virtually, it->isDirect()});
    }

    return res;
  }

  using OverriderInfo = std::unordered_map<ga::FunctionKeyType, Rose::Source::Analysis::VirtualFunctionEntry>;
  using FreshVirtualFunctions = std::unordered_map<ga::FunctionKeyType, bool>;

  using VirtualMemberFnSummaryBase = std::tuple<OverriderInfo, FreshVirtualFunctions>;
  struct VirtualMemberFnSummary : VirtualMemberFnSummaryBase
  {
    using base = VirtualMemberFnSummaryBase;
    using base::base;

    const OverriderInfo& overriderInfo() const { return std::get<0>(*this); }
    const FreshVirtualFunctions& freshVirtualFunctions() const { return std::get<1>(*this); }
    FreshVirtualFunctions& freshVirtualFunctions() { return std::get<1>(*this); }
  };

  VirtualMemberFnSummary
  createVirtualMemberFunctionSummary(const ga::VirtualFunctionAnalysis& vfa,
                                     const ga::ClassAnalysis::value_type& clazz)
  {
    FreshVirtualFunctions fresh;
    OverriderInfo over;

    for (ga::FunctionKeyType fn : clazz.second.virtualFunctions())
    {
      const ga::VirtualFunctionAnalysis::mapped_type& vfn = vfa.at(fn);

      for (const ga::OverrideDesc& ovrdsc : vfn.overridden())
      {
        const bool pureVirt = vfn.isPureVirtual();
        const bool retvalAdj = ovrdsc.adjustReturnObj();
        const bool needsGen = false;

        over.emplace(ovrdsc.function(),
                     Rose::Source::Analysis::VirtualFunctionEntry{fn, clazz.first, retvalAdj, pureVirt, needsGen});
      }

      fresh.emplace(fn, vfn.isPureVirtual());
    }

    return VirtualMemberFnSummary{std::move(over), std::move(fresh)};
  }

  void processInherited(Rose::Source::Analysis::VTableLayout& vt,
                        const Rose::Source::Analysis::VTableLayout& base,
                        const OverriderInfo& overriders,
                        Rose::Source::Analysis::VTableSection& sect)
  {
    using Sections = Rose::Source::Analysis::VTableLayout::VTableSections;

    const Sections& baseSects = base.vtableSections();
    Sections::const_iterator beg = baseSects.begin();
    Sections::const_iterator lim =
      std::find_if(beg, baseSects.end(),
                   [](const Rose::Source::Analysis::VTableSection& baseSec) { return baseSec.virtualBase(); });

    auto sumFn = [](std::size_t n, const Rose::Source::Analysis::VTableSection& s) -> std::size_t
                 {
                   return n + s.numTotal();
                 };
    const std::size_t numFn = std::accumulate(beg, lim, std::size_t{0}, sumFn);

    std::transform(base.begin(), base.begin() + numFn,
                   std::back_inserter(vt),
                   [&overriders](const Rose::Source::Analysis::VTableLayoutElement& el) -> Rose::Source::Analysis::VTableLayoutElement
                   {
                     const Rose::Source::Analysis::VirtualFunctionEntry& vfn =
                       boost::get<Rose::Source::Analysis::VirtualFunctionEntry>(el);
                     auto pos = overriders.find(vfn.function());

                     return pos == overriders.end() ? vfn : pos->second;
                   });

    sect.numInherited(numFn);
    sect.numTotal(numFn);
  }

  struct CheckedIter : std::tuple<Rose::Source::Analysis::VTableLayout::const_iterator, std::size_t>
  {
    using base = std::tuple<Rose::Source::Analysis::VTableLayout::const_iterator, std::size_t>;
    using base::base;

    Rose::Source::Analysis::VTableLayout::const_iterator iter() const { return std::get<0>(*this); }
    std::size_t size() const { return std::get<1>(*this); }
  };

  std::vector<CheckedIter>
  collectVirtualBaseSections(const Rose::Source::Analysis::VTableLayout& vt,
                             const Rose::Source::Analysis::VTableLayout& base,
                             std::size_t numVirtualFunctions,
                             const ga::ClassAnalysis& all,
                             const Rose::Source::Analysis::VTableLayoutContainer& layouts)
  {
    std::vector<CheckedIter> res;
    const ga::ClassAnalysis::mapped_type& thisClass = all.at(vt.getClass());

    res.emplace_back(base.begin(), numVirtualFunctions);

    for (ga::InheritanceDesc inh : thisClass.ancestors())
    {
      if (!inh.isDirect() && !inh.isVirtual()) continue;
      if (!all.isVirtualBaseOf(base.getClass(), inh.getClass())) continue;

      const Rose::Source::Analysis::VTableLayout& overLayout = layouts.at(inh.getClass());
      const Rose::Source::Analysis::VTableSection& virtBaseSect = overLayout.virtualBaseSection(base.getClass());

      ASSERT_require(virtBaseSect.numTotal() == numVirtualFunctions);
      res.emplace_back(overLayout.begin() + virtBaseSect.startOffset(), numVirtualFunctions);
    }

    return res;
  }

  void processInheritedVirtualBases(Rose::Source::Analysis::VTableLayout& vt,
                                    const ga::ClassAnalysis& all,
                                    const Rose::Source::Analysis::VTableLayout& base,
                                    const Rose::Source::Analysis::VTableLayoutContainer& layouts,
                                    const OverriderInfo& overriders,
                                    Rose::Source::Analysis::VTableSection& sect,
                                    const Rose::Source::Analysis::CppCompatibility& compat)
  {
    using Sections = Rose::Source::Analysis::VTableLayout::VTableSections;

    const Sections& baseSects = base.vtableSections();
    Sections::const_iterator beg = baseSects.begin();
    Sections::const_iterator lim =
      std::find_if(beg, baseSects.end(),
                   [](const Rose::Source::Analysis::VTableSection& baseSec) { return baseSec.virtualBase(); });

    auto sumFn = [](std::size_t n, const Rose::Source::Analysis::VTableSection& s) -> std::size_t
                 {
                   return n + s.numTotal();
                 };
    const std::size_t numFn = std::accumulate(beg, lim, std::size_t{0}, sumFn);

    std::vector<CheckedIter> candBaseSects =
      collectVirtualBaseSections(vt, base, numFn, all, layouts);
    ASSERT_require(!candBaseSects.empty());

    std::vector<std::vector<Rose::Source::Analysis::VTableLayoutElement> > candidates;
    candidates.reserve(numFn);

    for (std::size_t slot = 0; slot < numFn; ++slot)
    {
      std::vector<Rose::Source::Analysis::VTableLayoutElement> entries;
      entries.reserve(candBaseSects.size());

      for (CheckedIter candBaseBeg : candBaseSects)
      {
        ASSERT_require(slot < candBaseBeg.size());
        entries.push_back(*(candBaseBeg.iter() + slot));
      }

      auto entryLessThan =
        [](const Rose::Source::Analysis::VTableLayoutElement& lhs, const Rose::Source::Analysis::VTableLayoutElement& rhs) -> bool
        {
          const Rose::Source::Analysis::VirtualFunctionEntry& lhsFn =
            boost::get<Rose::Source::Analysis::VirtualFunctionEntry>(lhs);
          const Rose::Source::Analysis::VirtualFunctionEntry& rhsFn =
            boost::get<Rose::Source::Analysis::VirtualFunctionEntry>(rhs);
          return lhsFn.function() < rhsFn.function();
        };

      std::sort(entries.begin(), entries.end(), entryLessThan);

      auto entryEquals =
        [](const Rose::Source::Analysis::VTableLayoutElement& lhs, const Rose::Source::Analysis::VTableLayoutElement& rhs) -> bool
        {
          const Rose::Source::Analysis::VirtualFunctionEntry& lhsFn =
            boost::get<Rose::Source::Analysis::VirtualFunctionEntry>(lhs);
          const Rose::Source::Analysis::VirtualFunctionEntry& rhsFn =
            boost::get<Rose::Source::Analysis::VirtualFunctionEntry>(rhs);
          return lhsFn.function() == rhsFn.function();
        };

      auto unq = std::unique(entries.begin(), entries.end(), entryEquals);
      const std::size_t unqLen = std::distance(entries.begin(), unq);
      ASSERT_require(unqLen > 0);

      std::vector<Rose::Source::Analysis::VTableLayoutElement> revised;

      for (std::size_t i = 0; i < unqLen; ++i)
      {
        bool isDominated = false;
        const Rose::Source::Analysis::VirtualFunctionEntry& ciFn =
          boost::get<Rose::Source::Analysis::VirtualFunctionEntry>(entries[i]);

        for (std::size_t j = 0; j < unqLen; ++j)
        {
          if (i == j) continue;

          const Rose::Source::Analysis::VirtualFunctionEntry& cjFn =
            boost::get<Rose::Source::Analysis::VirtualFunctionEntry>(entries[j]);

          if (all.isVirtualBaseOf(ciFn.getClass(), cjFn.getClass()))
            isDominated = true;
        }

        if (!isDominated)
          revised.push_back(entries[i]);
      }

      ASSERT_require(!revised.empty());
      candidates.emplace_back(std::move(revised));
    }

    std::transform(candidates.begin(), candidates.end(),
                   std::back_inserter(vt),
                   [&overriders,&vt,&compat,&all]
                   (const std::vector<Rose::Source::Analysis::VTableLayoutElement>& els) -> Rose::Source::Analysis::VTableLayoutElement
                   {
                     Rose::Source::Analysis::VirtualFunctionEntry res{ga::FunctionKeyType{}, ga::ClassKeyType{}, false, false, false};

                     for (const Rose::Source::Analysis::VTableLayoutElement& el : els)
                     {
                       const Rose::Source::Analysis::VirtualFunctionEntry& vfn =
                         boost::get<Rose::Source::Analysis::VirtualFunctionEntry>(el);
                       auto pos = overriders.find(vfn.function());
                       const Rose::Source::Analysis::VirtualFunctionEntry& vtElem =
                         pos == overriders.end() ? vfn : pos->second;

                       if (!ga::isValidKey(res.function()))
                         res = vtElem;
                       else if (res.function() != vtElem.function())
                       {
                         if (!compat.isAutoGeneratable(all, vt.getClass(), res.function()))
                           msgError() << "No unique overrider for virtual inheritance." << std::endl;
                       }
                     }

                     ASSERT_require(ga::isValidKey(res.getClass()));
                     return res;
                   });

    sect.numInherited(numFn);
    sect.numTotal(numFn);
  }

  void processNonInherited(Rose::Source::Analysis::VTableLayout& vt,
                           const ga::ClassAnalysis::value_type& clazz,
                           const FreshVirtualFunctions& fresh,
                           Rose::Source::Analysis::VTableSection& sect)
  {
    std::transform(fresh.begin(), fresh.end(),
                   std::back_inserter(vt),
                   [&clazz](const FreshVirtualFunctions::value_type& el) -> Rose::Source::Analysis::VTableLayoutElement
                   {
                     return Rose::Source::Analysis::VirtualFunctionEntry{el.first, clazz.first, false, el.second, false};
                   });

    sect.numTotal(sect.numTotal() + fresh.size());
  }

  Rose::Source::Analysis::VTableLayout
  computeVTableLayout(const ga::ClassAnalysis& all,
                      const Rose::Source::Analysis::VTableLayoutContainer& layouts,
                      const ga::ClassAnalysis::value_type& clazz,
                      VirtualMemberFnSummary summary,
                      const Rose::Source::Analysis::CppCompatibility& compat)
  {
    using Vec = std::vector<ga::InheritanceDesc>;

    Rose::Source::Analysis::VTableLayout res{clazz.first};
    PrimarySubobjectFinder finder{&all};
    const Vec& parents = clazz.second.ancestors();
    const Vec::const_iterator aa = parents.begin();
    const Vec::const_iterator zz = parents.end();
    const Vec::const_iterator primary = std::find_if(aa, zz, finder);

    Rose::Source::Analysis::VTableSection& primarySect = res.createVTableSection();

    if (primary != zz)
    {
      primarySect.associatedClass(primary->getClass());
      processInherited(res, layouts.at(primary->getClass()), summary.overriderInfo(), primarySect);
    }
    else
    {
      primarySect.associatedClass(clazz.first);
    }

    for (const Rose::Source::Analysis::VTableLayoutElement& el : res)
    {
      const Rose::Source::Analysis::VirtualFunctionEntry& vfn = boost::get<Rose::Source::Analysis::VirtualFunctionEntry>(el);
      if (!vfn.adjustReturnObj())
        summary.freshVirtualFunctions().erase(vfn.function());
    }

    processNonInherited(res, clazz, summary.freshVirtualFunctions(), primarySect);

    for (Vec::const_iterator it = (primary == zz ? zz : primary + 1); it != zz; ++it)
    {
      if (!finder(*it))
        continue;

      Rose::Source::Analysis::VTableSection& nonprimarySect = res.createVTableSection();
      nonprimarySect.associatedClass(it->getClass());
      processInherited(res, layouts.at(it->getClass()), summary.overriderInfo(), nonprimarySect);
    }

    for (Vec::const_iterator it = aa; it != zz; ++it)
    {
      if (!it->isVirtual())
        continue;

      Rose::Source::Analysis::VTableSection& virtualSect = res.createVTableSection();
      virtualSect.virtualBase(true);
      virtualSect.associatedClass(it->getClass());
      processInheritedVirtualBases(res,
                                   all,
                                   layouts.at(it->getClass()),
                                   layouts,
                                   summary.overriderInfo(),
                                   virtualSect,
                                   compat);
    }

    bool isAbstract = false;

    for (Rose::Source::Analysis::VTableLayoutElement& el : res)
    {
      Rose::Source::Analysis::VirtualFunctionEntry& vfn = boost::get<Rose::Source::Analysis::VirtualFunctionEntry>(el);
      ASSERT_require(ga::isValidKey(vfn.getClass()));

      const bool willGen = clazz.first != vfn.getClass()
                        && compat.isAutoGeneratable(all, clazz.first, vfn.function());

      if (!isAbstract && vfn.isPureVirtual() && !willGen)
        isAbstract = true;

      if (willGen)
      {
        vfn.setClass(clazz.first);
        vfn.needsCompilerGeneration(true);
      }
    }

    res.isAbstractClass(isAbstract);
    return res;
  }

  Rose::Source::Analysis::VTableLayout
  computeVTableLayout(const ga::ClassAnalysis& all,
                      const Rose::Source::Analysis::VTableLayoutContainer& layouts,
                      const ga::ClassAnalysis::value_type& clazz,
                      const ga::VirtualFunctionAnalysis& vfa,
                      const Rose::Source::Analysis::CppCompatibility& compat)
  {
    return computeVTableLayout(all, layouts, clazz,
                               createVirtualMemberFunctionSummary(vfa, clazz),
                               compat);
  }

  template <class TLayoutAnalysis>
  auto lookupAux(TLayoutAnalysis& m, const typename TLayoutAnalysis::key_type& key, const char* code)
    -> decltype(*m.find(key))
  {
    auto pos = m.find(key);
    if (pos != m.end())
      return *pos;

    Rose::Source::Analysis::CppCompatibility compat;
    msgError() << "\nUnable to find representation for class " << compat.nameOf(key)
               << "\n  * Number of known classes (in " << code << "): " << m.size()
               << std::endl;

    throw std::out_of_range("Unable to find class in layout analysis.");
  }
}

namespace Rose::Source::Analysis
{

CppLayoutAnalysis
computeObjectLayouts(const ga::ClassAnalysis& all, bool onlyClassesWithVTable)
{
  CppLayoutAnalysis res;

  auto objectLayoutComputation =
    [&all, &res, onlyClassesWithVTable](const ga::ClassAnalysis::value_type& clazz)
    {
      if (onlyClassesWithVTable && !clazz.second.hasVirtualTable())
        return;

      res.emplace(clazz.first, computeObjectLayout(all, res, clazz));
    };

  topDownTraversal(all, objectLayoutComputation);
  return res;
}

VTableLayoutAnalysis
computeVTableLayouts(const ga::ClassAnalysis& all,
                     const ga::VirtualFunctionAnalysis& vfa,
                     const CppCompatibility& compat)
{
  VTableLayoutAnalysis res;

  auto vtableLayoutComputation =
    [&all, &vfa, &compat, &res](const ga::ClassAnalysis::value_type& clazz)
    {
      if (!clazz.second.hasVirtualTable())
        return;

      res.emplace(clazz.first, computeVTableLayout(all, res, clazz, vfa, compat));
    };

  topDownTraversal(all, vtableLayoutComputation);
  return res;
}

VTableLayoutAnalysis
computeVTableLayouts(const ga::ClassAnalysis& all,
                     const ga::VirtualFunctionAnalysis& vfa)
{
  CppCompatibility compat;
  return computeVTableLayouts(all, vfa, compat);
}

VTableLayout::VTableLayout(ga::ClassKeyType cl)
: base(), clazz(cl), sections(), isAbstract(false)
{}

const VTableSection&
VTableLayout::virtualBaseSection(ga::ClassKeyType key) const
{
  auto pos = std::find_if(sections.begin(), sections.end(),
                          [key](const VTableSection& sec)
                          {
                            return sec.virtualBase() && sec.associatedClass() == key;
                          });
  ASSERT_require(pos != sections.end());
  return *pos;
}

VTableSection&
VTableLayout::createVTableSection()
{
  sections.emplace_back(ga::ClassKeyType{}, 0, 0, size(), false);
  return sections.back();
}

std::size_t
VTableLayout::vtableAddress() const
{
  return 0;
}

VTableLayoutElement&
VTableLayout::at(const VTableSection& sec, std::size_t i)
{
  return this->base::at(sec.startOffset() + i);
}

const VTableLayoutElement&
VTableLayout::at(const VTableSection& sec, std::size_t i) const
{
  return this->base::at(sec.startOffset() + i);
}

const VTableLayoutAnalysis::mapped_type&
VTableLayoutAnalysis::at(const VTableLayoutAnalysis::key_type& k) const
{
  return lookupAux(*this, k, "VTableLayoutAnalysis").second;
}

VTableLayoutAnalysis::mapped_type&
VTableLayoutAnalysis::at(const VTableLayoutAnalysis::key_type& k)
{
  return lookupAux(*this, k, "VTableLayoutAnalysis").second;
}

const CppLayoutAnalysis::mapped_type&
CppLayoutAnalysis::at(const CppLayoutAnalysis::key_type& k) const
{
  return lookupAux(*this, k, "CppLayoutAnalysis").second;
}

CppLayoutAnalysis::mapped_type&
CppLayoutAnalysis::at(const CppLayoutAnalysis::key_type& k)
{
  return lookupAux(*this, k, "CppLayoutAnalysis").second;
}

} // namespace Rose::Source::Analysis
