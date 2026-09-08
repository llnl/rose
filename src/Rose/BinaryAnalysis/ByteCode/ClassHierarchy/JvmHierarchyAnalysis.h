
#ifndef ROSE_BinaryAnalysis_ByteCode_ClassHierarchy_JvmHierarchyAnalysis_H
#define ROSE_BinaryAnalysis_ByteCode_ClassHierarchy_JvmHierarchyAnalysis_H 1

#include <Rose/Common/Analysis/ClassHierarchyAnalysis.h>
#include <Rose/BinaryAnalysis/ByteCode/ClassHierarchy/JvmCompatibility.h>

namespace Rose::BinaryAnalysis::ByteCode::ClassHierarchy
{

/// Binds the abstract Rose::Common::Analysis::ClassAnalysis to the JVM representation in ROSE
///   by defining the compatibility class.
class JvmHierarchyAnalysis final : public Rose::Common::Analysis::ClassAnalysis
{
  public:

    /// Base class-analysis type.
    using base = Rose::Common::Analysis::ClassAnalysis;
    using base::base;

    /// Returns the JVM compatibility bridge.
    /// \return compatibility bridge used by this JVM analysis.
    const JvmCompatibility& compatibilityBridge() const override;

  private:
    static JvmCompatibility compat;
};

/// Generates the JVM class hierarchy from the ROSE memory pool.
/// \return populated JVM hierarchy analysis.
JvmHierarchyAnalysis
analyzeJvmClassesFromMemoryPool();

}

#endif /* ROSE_BinaryAnalysis_ByteCode_ClassHierarchy_JvmHierarchyAnalysis_H */
