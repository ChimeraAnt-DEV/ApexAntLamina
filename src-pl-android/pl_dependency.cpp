// pl::dependency_walker for Android.
//
// Dependency scanning is a PE/COFF concept; the Android dynamic linker
// reports missing imports itself. These stubs exist so that
// NativeModManager.cpp can compile and always report a clean dependency
// graph.

#include "pl/dependency/DependencyWalker.h"
#include "pl/dependency/IProvider.h"
#include "pl/dependency/LibrarySearcher.h"

#include <memory>

namespace ll::android {
void dummyFillDiagnosticDependency(pl::dependency_walker::DependencyIssueItem& item);
}

namespace pl::dependency_walker {

std::shared_ptr<LibrarySearcher> LibrarySearcher::getInstance() {
    static std::shared_ptr<LibrarySearcher> instance(new LibrarySearcher(""));
    return instance;
}

LibrarySearcher::LibrarySearcher(std::string_view envPath) {
    (void)envPath;
}

std::unordered_map<std::string, std::string> LibrarySearcher::parseApiSetSchema() { return {}; }

std::optional<std::filesystem::path> LibrarySearcher::getLibraryPath(std::string libName) const {
    (void)libName;
    return std::nullopt;
}

PortableExecutableProvider::PortableExecutableProvider(
    const std::shared_ptr<LibrarySearcher>& libSearcher,
    const std::u8string&                    systemRoot,
    const std::filesystem::path&            path
) {
    (void)libSearcher;
    (void)systemRoot;
    (void)path;
}

bool PortableExecutableProvider::queryExport(const std::string&) { return false; }

const IProvider::ImportContainer& PortableExecutableProvider::getImports() {
    static const ImportContainer empty{};
    return empty;
}

InternalSymbolProvider::InternalSymbolProvider() = default;

bool InternalSymbolProvider::queryExport(const std::string&) { return false; }

const IProvider::ImportContainer& InternalSymbolProvider::getImports() {
    static const ImportContainer empty{};
    return empty;
}

PLAPI std::unique_ptr<DependencyIssueItem, void (*)(DependencyIssueItem*)>
      pl_diagnostic_dependency_new(std::filesystem::path const& path) {
    std::unique_ptr<DependencyIssueItem, void (*)(DependencyIssueItem*)> result(
        new DependencyIssueItem{},
        [](DependencyIssueItem* item) { delete item; }
    );
    result->mPath = path;
    result->mContainsError = false;
    ll::android::dummyFillDiagnosticDependency(*result);
    return result;
}

PLAPI std::unique_ptr<DependencyIssueItem> pl_diagnostic_dependency(std::filesystem::path const& path) {
    auto result = std::make_unique<DependencyIssueItem>();
    result->mPath = path;
    ll::android::dummyFillDiagnosticDependency(*result);
    return result;
}

std::string pl_diagnostic_dependency_string(std::filesystem::path const& path) {
    (void)path;
    return {};
}

} // namespace pl::dependency_walker