
#include <Rose/BinaryAnalysis/ByteCode/ClassHierarchy/JvmHierarchyAnalysis.h>

namespace Rose::BinaryAnalysis::ByteCode::ClassHierarchy
{

using namespace Rose::Common::Analysis;

const JvmCompatibility&
JvmHierarchyAnalysis::compatibilityBridge() const
{
  return compat;
}

JvmCompatibility JvmHierarchyAnalysis::compat;

JvmHierarchyAnalysis
analyzeJvmClassesFromMemoryPool()
{
  JvmHierarchyAnalysis classes;

  analyzeClassesFromMemoryPool(classes);
  return classes;
}

}
