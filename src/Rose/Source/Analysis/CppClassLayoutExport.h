#ifndef ROSE_Source_Analysis_CppClassLayoutExport_H
#define ROSE_Source_Analysis_CppClassLayoutExport_H 1

#include <RoseFirst.h>

#include <ostream>

#include <Rose/Common/Analysis/ClassHierarchyExport.h>
#include <Rose/Source/Analysis/CppLayoutAnalysis.h>

namespace Rose::Source::Analysis
{

/// Writes the virtual base class initialization order as text.
/// \param os destination stream.
/// \param className callback that maps class keys to display names.
/// \param include callback that selects which classes are included.
/// \param classes analyzed class hierarchy.
/// \return nothing.
void virtualBaseClassInitOrderTxt(std::ostream& os,
                                  Rose::Common::Analysis::ClassNameFn& className,
                                  Rose::Common::Analysis::ClassFilterFn include,
                                  const Rose::Common::Analysis::ClassAnalysis& classes);

/// Writes computed object layouts as text.
/// \param os destination stream.
/// \param className callback that maps class keys to display names.
/// \param varName callback that maps variable keys to display names.
/// \param include callback that selects which classes are included.
/// \param classLayout computed object layout analysis.
/// \return nothing.
void classLayoutTxt(std::ostream& os,
                    Rose::Common::Analysis::ClassNameFn& className,
                    Rose::Common::Analysis::VarNameFn& varName,
                    Rose::Common::Analysis::ClassFilterFn include,
                    const CppLayoutAnalysis& classLayout);

/// Writes computed object layouts as a DOT graph.
/// \param os destination stream.
/// \param className callback that maps class keys to display names.
/// \param varName callback that maps variable keys to display names.
/// \param include callback that selects which classes are included.
/// \param classLayout computed object layout analysis.
/// \return nothing.
void classLayoutDot(std::ostream& os,
                    Rose::Common::Analysis::ClassNameFn& className,
                    Rose::Common::Analysis::VarNameFn& varName,
                    Rose::Common::Analysis::ClassFilterFn include,
                    const CppLayoutAnalysis& classLayout);

/// Writes computed vtable layouts as text.
/// \param os destination stream.
/// \param className callback that maps class keys to display names.
/// \param funcName callback that maps function keys to display names.
/// \param include callback that selects which classes are included.
/// \param vtableLayout computed vtable layout analysis.
/// \return nothing.
void vtableLayoutTxt(std::ostream& os,
                     Rose::Common::Analysis::ClassNameFn& className,
                     Rose::Common::Analysis::FuncNameFn& funcName,
                     Rose::Common::Analysis::ClassFilterFn include,
                     const VTableLayoutAnalysis& vtableLayout);

/// Writes computed object layouts as JSON.
/// \param os destination stream.
/// \param className callback that maps class keys to display names.
/// \param varName callback that maps variable keys to display names.
/// \param include callback that selects which classes are included.
/// \param classLayout computed object layout analysis.
/// \return nothing.
void classLayoutJson(std::ostream& os,
                     Rose::Common::Analysis::ClassNameFn& className,
                     Rose::Common::Analysis::VarNameFn& varName,
                     Rose::Common::Analysis::ClassFilterFn include,
                     const CppLayoutAnalysis& classLayout);

/// Writes computed vtable layouts as JSON.
/// \param os destination stream.
/// \param className callback that maps class keys to display names.
/// \param funcName callback that maps function keys to display names.
/// \param include callback that selects which classes are included.
/// \param vtableLayout computed vtable layout analysis.
/// \return nothing.
void vtableLayoutJson(std::ostream& os,
                      Rose::Common::Analysis::ClassNameFn& className,
                      Rose::Common::Analysis::FuncNameFn& funcName,
                      Rose::Common::Analysis::ClassFilterFn include,
                      const VTableLayoutAnalysis& vtableLayout);

/// Writes the legacy CodeThorn-compatible object/vtable layout JSON projection.
///
/// All identifiers are produced by ExternalMangling; source names remain
/// display-only fields.  The function is an export projection over already
/// computed layouts and does not perform hierarchy or layout analysis.
void cppClassLayoutJson(std::ostream& os,
                        Rose::Common::Analysis::ClassFilterFn include,
                        const CppLayoutAnalysis& classLayout,
                        const VTableLayoutAnalysis& vtableLayout);

} // namespace Rose::Source::Analysis

#endif
