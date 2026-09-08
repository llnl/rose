
#include <Rose/BinaryAnalysis/ByteCode/ClassHierarchy/CilHierarchyAnalysis.h>

namespace Rose::BinaryAnalysis::ByteCode::ClassHierarchy
{

using namespace Rose::Common::Analysis;

const CilCompatibility&
CilHierarchyAnalysis::compatibilityBridge() const
{
  return compat;
}

CilCompatibility CilHierarchyAnalysis::compat;

CilHierarchyAnalysis
analyzeCilClassesFromMemoryPool()
{
  CilHierarchyAnalysis classes;

  analyzeClassesFromMemoryPool(classes);
  return classes;
}

}
