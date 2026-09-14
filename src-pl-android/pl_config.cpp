// pl compat globals owned by the ApexAntLamina Android shim.
//
// pl_log_path distinguishes Termux from a LeviLauncher-embedded installation:
// inside LeviLauncher the log goes to the launcher's files dir; on Termux
// it follows the conventional $HOME/Library/Logs layout. The values are set
// by ll::android::setAndroidPlConfig before the LeviLamina core starts.

#include "pl/Config.h"

#include <cstdlib>
#include <ctime>

#include <filesystem>
#include <string>

namespace ll::android {
void setAndroidPlConfig(std::filesystem::path const& root, bool colorLog, int logLevel);
std::filesystem::path const& getSelfModRoot() noexcept;
void setSelfModRoot(std::filesystem::path root) noexcept;
}

namespace {
bool        g_colorLog    = false;
int         g_logLevel    = 4; // io::LogLevel::Info
std::string g_logPath     = ".";
std::string g_modsPath    = ".";
std::string g_managerName = "native"; // matches ll::mod::NativeModManagerName

std::filesystem::path g_selfModRoot;
} // namespace

extern "C" {

PLAPI char const* pl_mod_manager_name = g_managerName.c_str();
PLAPI bool        pl_color_log         = false;
PLAPI int         pl_log_level         = 4;
PLAPI char const* pl_log_path          = "";
PLAPI char const* pl_mods_path         = "";
}

namespace ll::android {

void syncPlGlobals() noexcept {
    pl_color_log = g_colorLog;
    pl_log_level = g_logLevel;
    pl_log_path  = g_logPath.c_str();
    pl_mods_path = g_modsPath.c_str();
}

void setAndroidPlConfig(std::filesystem::path const& modRoot, bool colorLog, int logLevel) {
    g_logLevel = logLevel;
    g_colorLog = colorLog;

    // Distinguish Termux from a LeviLauncher-embedded install: under Termux
    // $PREFIX is set and a conventional logs dir exists under $HOME; inside
    // the launcher we keep logs next to the mod root.
    std::filesystem::path logsDir;
    char const* prefix = std::getenv("PREFIX");
    char const* home   = std::getenv("HOME");
    if (prefix && home) {
        logsDir = std::filesystem::path(home) / u8"Library" / u8"Logs";
    } else {
        logsDir = modRoot / u8"logs";
    }

    std::error_code ec;
    std::filesystem::create_directories(logsDir, ec);

    g_logPath  = logsDir.string();
    // pl_mods_path is the mods *root* (parent of the individual mod
    // folders). Shared Mod::Impl derives each mod dir as
    // `<pl_mods_path>/<manifest.name>`.
    g_modsPath = modRoot.parent_path().string();

    syncPlGlobals();
}

std::filesystem::path const& getSelfModRoot() noexcept { return g_selfModRoot; }

void setSelfModRoot(std::filesystem::path root) noexcept { g_selfModRoot = std::move(root); }
} // namespace ll::android