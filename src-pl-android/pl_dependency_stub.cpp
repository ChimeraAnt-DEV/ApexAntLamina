// Shared stubs used by the dependency-walker compat layer.

#include "pl/dependency/DependencyWalker.h"

namespace ll::android {
void dummyFillDiagnosticDependency(pl::dependency_walker::DependencyIssueItem& item) {
    // Anchors the symbol so it is not optimised away and future porting can
    // report unresolved imports here.
    (void)item;
}
} // namespace ll::android