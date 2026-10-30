// DQ (2/10/2010): This is required for the OpenMP support in the EDG-ROSE connection (moved from above).
#include "OmpAttribute.h"

/* Don't know a better place for them 
 * Bison does not generate prototypes for them by default
 * */
extern int omp_parse();
extern OmpSupport::OmpAttribute* getParsedDirective();
extern void omp_parser_init(SgNode* aNode, const char* str);

// store omp pragmas first during conversion and generate 
// Two reasons to do this:
// a) Some variable symbols are not ready during conversion
// b) Some SgPragmaDeclaration nodes are deferred for insertion
static std::vector<SgPragmaDeclaration*> all_pragmas;


