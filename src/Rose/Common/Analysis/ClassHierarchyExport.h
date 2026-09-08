// An example ROSE plugin

#ifndef ROSE_Common_Analysis_ClassHierarchyExport_H
#define ROSE_Common_Analysis_ClassHierarchyExport_H 1

#include <RoseFirst.h>

#include <ostream>

#include <Rose/Common/Analysis/ClassHierarchyAnalysis.h>

namespace Rose::Common::Analysis
{

/// filter type for class hierarchy writer
///   returns true, if a class should be included in the graph
/// \param key class key to test.
/// \return true if \p key should be included in exported output.
using ClassFilterFn = std::function<bool(ClassKeyType)>;

/// Returns a no filter (always returning true).
/// \return predicate that includes every class key.
ClassFilterFn noClassFilter();

/// Returns a class filter that includes classes without excluding by base name.
/// \param classes class analysis used to evaluate base-name filtering.
/// \return predicate selecting classes for export.
/// \note The exact filtering rules are defined by the implementation.
ClassFilterFn baseName(const ClassAnalysis& classes);


/// Writes out a class analysis graph for all classes and casts.
/// \param os destination stream
/// \param nameOf callback that maps class keys to display names
/// \param include callback that selects which classes are included
/// \param classes analyzed class hierarchy
/// \return nothing.
void classHierarchyDot( std::ostream& os,
                        ClassNameFn& nameOf,
                        ClassFilterFn include,
                        const ClassAnalysis& classes
                      );

/// Writes out virtual function relationships as text.
/// \param os destination stream
/// \param className callback that maps class keys to display names
/// \param funcName callback that maps function keys to display names
/// \param classId callback that maps class keys to stable JSON identifiers
/// \param funcId callback that maps function keys to stable JSON identifiers
/// \param include callback that selects which classes are included
/// \param classes analyzed class hierarchy
/// \param vfuns analyzed virtual function relationships
/// \param withOverridden include overridden-function information when true
/// \return nothing.
void virtualFunctionsTxt( std::ostream& os,
                          ClassNameFn& className,
                          FuncNameFn& funcName,
                          ClassFilterFn include,
                          const ClassAnalysis& classes,
                          const VirtualFunctionAnalysis& vfuns,
                          bool withOverridden = false
                        );

/// Writes out class hierarchy and optional virtual function relationships as JSON.
/// \param os destination stream
/// \param className callback that maps class keys to display names
/// \param funcName callback that maps function keys to display names
/// \param include callback that selects which classes are included
/// \param classes analyzed class hierarchy
/// \param vfuns optional analyzed virtual function relationships
/// \return nothing.
void classHierarchyJson( std::ostream& os,
                         ClassNameFn& className,
                         FuncNameFn& funcName,
                         ClassNameFn& classId,
                         FuncNameFn& funcId,
                         ClassFilterFn include,
                         const ClassAnalysis& classes,
                         const VirtualFunctionAnalysis* vfuns = nullptr
                       );

}
#endif /* CLASS_HIERARCHY_EXPORT_H */
