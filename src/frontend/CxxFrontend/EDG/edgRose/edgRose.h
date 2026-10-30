
// ROSE Headers

#include "rose_config.h"
#include "sage3basic.h"

// EDG Headers

#include "basic_hdrs.h"
#include "fe_common.h"
#include "decls.h"
#include "macro.h"

// 

#include "macro_defs.h"
#include "debugging.h"

//

#include "SourceSequenceList.h"
#include "SourcePosition.h"

#include "edgRoseAttributes.h"
#include "edgRoseOMP.h"
#include "edgRoseFunctions.h"

#include "edgGraph.h"

// 

#undef DEBUG
#include "Rose/Diagnostics.h"

#undef mprintf
#define mprintf Rose::Diagnostics::mfprintf(EDG_ROSE_Translation::mlog[Rose::Diagnostics::DEBUG])

using namespace std;
using namespace Rose;
using namespace SageInterface;
using namespace SageBuilder;
using namespace OmpSupport;
using namespace Rose::Diagnostics;

#include "edgRose.tpp"
