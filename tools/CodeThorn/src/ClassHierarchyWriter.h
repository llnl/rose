// An example ROSE plugin

#ifndef CODETHORN_CLASS_HIERARCHY_WRITER_H
#define CODETHORN_CLASS_HIERARCHY_WRITER_H 1

#include <ostream>

#include "ClassHierarchyAnalysis.h"
#include "ObjectLayoutAnalysis.h"


namespace CodeThorn
{

/// filter type for class hierarchy writer
///   returns true, if a class should be included in the graph
using ClassFilterFn = std::function<bool (ClassKeyType)>;

/// writes out a class analysis graph for all classes and casts
void classHierarchyDot( std::ostream& os,
                        ClassNameFn& nameOf,
                        ClassFilterFn include,
                        const ClassAnalysis& classes,
                        const CastAnalysis& casts
                      );

/// writes out the virtual functions relationships as text
void virtualFunctionsTxt( std::ostream& os,
                          ClassNameFn& className,
                          FuncNameFn& funcName,
                          ClassFilterFn include,
                          const ClassAnalysis& classes,
                          const VirtualFunctionAnalysis& vfuns,
                          bool withOverridden = false
                        );

/// writes out the virtual base class initialization order as text for
///   classes that have virtual base classes.
void virtualBaseClassInitOrderTxt( std::ostream& os,
                                   ClassNameFn& className,
                                   ClassFilterFn include,
                                   const ClassAnalysis& classes
                                 );

/// writes out the class layout as text
void classLayoutTxt( std::ostream& os,
                     ClassNameFn& className,
                     VarNameFn& varName,
                     ClassFilterFn include,
                     const ObjectLayoutContainer& classLayout
                   );

/// writes out the class layout as dot graph
void classLayoutDot( std::ostream& os,
                     ClassNameFn& className,
                     VarNameFn& varName,
                     ClassFilterFn include,
                     const ObjectLayoutContainer& classLayout
                   );

/// writes out the vtable layout as text
void vtableLayoutTxt( std::ostream& os,
                      ClassNameFn& className,
                      FuncNameFn& funcName,
                      ClassFilterFn include,
                      const VTableLayoutContainer& vtableLayout
                    );
}
#endif /* CLASS_HIERARCHY_WRITER_H */
