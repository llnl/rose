
#ifndef ROSE_BinaryAnalysis_ByteCode_ClassHierarchy_CilHierarchyAnalysis_H
#define ROSE_BinaryAnalysis_ByteCode_ClassHierarchy_CilHierarchyAnalysis_H 1

#include <Rose/Common/Analysis/ClassHierarchyAnalysis.h>
#include <Rose/BinaryAnalysis/ByteCode/ClassHierarchy/CilCompatibility.h>

namespace Rose::BinaryAnalysis::ByteCode::ClassHierarchy
{

/// Binds the abstract Rose::Common::Analysis::ClassAnalysis to the Cil representation in ROSE
///   by defining the compatibility class.
class CilHierarchyAnalysis final : public Rose::Common::Analysis::ClassAnalysis
{
  public:

    /// Base class-analysis type.
    using base = Rose::Common::Analysis::ClassAnalysis;
    using base::base;

    /// Returns the CIL compatibility bridge.
    /// \return compatibility bridge used by this CIL analysis.
    const CilCompatibility& compatibilityBridge() const override;

  private:
    static CilCompatibility compat;
};

/// Generates the CIL class hierarchy from the ROSE memory pool.
/// \return populated CIL hierarchy analysis.
CilHierarchyAnalysis
analyzeCilClassesFromMemoryPool();

}

#endif /* ROSE_BinaryAnalysis_ByteCode_ClassHierarchy_CilHierarchyAnalysis_H */
