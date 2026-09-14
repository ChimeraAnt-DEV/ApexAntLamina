// ll::core::getLeviConfig for Android.
//
// The shared LL_CONFIG_IMPL macro resolves the config path relative to the
// self mod's config dir, which requires the self mod to already be
// registered. On Android the self mod is registered during launch, before
// the first getLeviConfig() call, but there are pre-launch uses (e.g. the
// welcome banner) that must not depend on it. This implementation uses the
// self mod's config dir when available and falls back to the pl:: mods
// root, which is set at load time by the preloader.

#include "ll/api/Config.h"
#include "ll/core/Config.h"

#include <filesystem>

#include "ll/api/i18n/I18n.h"
#include "ll/api/mod/Mod.h"
#include "ll/api/utils/ErrorUtils.h"
#include "ll/core/LeviLamina.h"

#include "pl/Config.h"

namespace ll {

LeviConfig& getLeviConfig() {
    static LeviConfig config = []() {
        LeviConfig res;
        auto        getPath = [] {
            namespace fs = std::filesystem;
            try {
                return getSelfModIns()->getConfigDir() / u8"Config.json";
            } catch (...) {
                return fs::path{std::string_view{pl::pl_mods_path}} / u8"config" / u8"Config.json";
            }
        };
        try {
            if (config::loadConfig(res, getPath())) {
                return res;
            }
        } catch (...) {
            getLogger().error("{0} load failed"_tr("LeviConfig"));
            error_utils::printCurrentException(getLogger());
        }
        return res;
    }();
    return config;
}

bool saveLeviConfig() {
    bool res{};
    try {
        namespace fs = std::filesystem;
        fs::path path;
        try {
            path = getSelfModIns()->getConfigDir() / u8"Config.json";
        } catch (...) {
            path = fs::path{std::string_view{pl::pl_mods_path}} / u8"config" / u8"Config.json";
        }
        res = config::saveConfig(getLeviConfig(), path);
    } catch (...) {
        res = false;
        error_utils::printCurrentException(getLogger());
    }
    if (!res) {
        getLogger().error("{0} failed to save"_tr("LeviConfig"));
        return false;
    }
    return true;
}

} // namespace ll