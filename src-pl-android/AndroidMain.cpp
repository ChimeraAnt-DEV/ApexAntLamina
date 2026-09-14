// Android bootstrap for the ApexAntLamina mod.
//
// Compiled only for the android platform. On Windows/Linux the preloader
// (main_win.cpp / its Linux equivalents) owns the launch lifecycle and
// drives the game hooks directly.
//
// On Android, LeviLamina is packaged as a "preload-native" mod loaded by
// LeviLauncher+libpreloader.so (or manually dlopen'ed on Termux). The
// preloader calls PLGetModRegistration() (see src-pl-export/
// mod_registration.cpp) then drives the lifecycle:
//
//   load   -> set the pl:: config globals (paths, log level) from the
//             preloader-provided mod root
//   enable -> initialize the LeviLamina core: self mod, i18n, config,
//             welcome banner, builtin command registration and mod loading
//   disable/unload -> disable all loaded mods and reset gaming status
//
// The full game-loop hooks (main(), ServerScriptManager::
// onServerThreadStarted etc.) are intentionally NOT hooked on Android: they
// are Windows-specific symbols that are absent from libminecraftpe.so. The
// vanilla server still runs inside the launcher process, so mods can
// install event listeners and commands once it starts.

#include "ll/api/Expected.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/io/LoggerRegistry.h"
#include "ll/api/mod/Mod.h"
#include "ll/api/mod/ModManagerRegistry.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/utils/ErrorUtils.h"
#include "ll/api/utils/StringUtils.h"
#include "ll/core/Config.h"
#include "ll/core/LeviLamina.h"
#include "ll/core/command/BuiltinCommands.h"
#include "ll/core/mod/ModRegistrar.h"

#include <jni.h>

#include <android/log.h>

#include <atomic>
#include <chrono>
#include <exception>
#include <filesystem>
#include <thread>

#include "pl/Config.h"

namespace ll::android {
void setAndroidPlConfig(std::filesystem::path const& root, bool colorLog, int logLevel);
std::filesystem::path const& getSelfModRoot() noexcept;
}

namespace {

constexpr const char* kLogTag = "ApexAntLamina";

void logException(std::exception_ptr eptr) noexcept {
    try {
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "%s", error_utils::makeExceptionString(eptr).c_str());
    } catch (...) {
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "unknown exception");
    }
}

std::atomic<bool> gLaunched{false};

} // namespace

namespace ll {

void registerAndroidCommands() {
    if (command::isRegisterCommandsAvailable()) {
        command::registerCommands();
    }
}

void launch() noexcept {
    try {
        setGamingStatus(GamingStatus::Starting);

        if (auto res = i18n::getInstance().load(getSelfModIns()->getLangDir()); !res) {
            getLogger().error("i18n load failed");
            res.error().log(getLogger());
        }

        auto& config = getLeviConfig();

        if (config.language != "system") {
            i18n::defaultLocaleCode() = config.language;
        }

        printWelcomeMsg();

        // The game process and its CommandRegistry may not exist yet inside
        // libminecraftpe.so; LeviLaunchroid publishes the pointers through
        // publishGamePointers() once the server object is constructed. We
        // spin here (short, polled) so the builtin commands are registered in
        // drained, explicit manner rather than racing the game init.
        registerAndroidCommands();
        auto const deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(5000);
        while (!command::isRegisterCommandsAvailable() && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (ll::service::getCommandRegistry(false)) {
                // registry appeared; re-run builtin registration
                registerAndroidCommands();
            }
        }

        mod::ModRegistrar::getInstance().loadAllMods();
    } catch (...) {
        logException(std::current_exception());
    }
}

void unload() noexcept {
    setGamingStatus(GamingStatus::Stopping);
    try {
        mod::ModRegistrar::getInstance().disableAllMods();
    } catch (...) {
        logException(std::current_exception());
    }
    setGamingStatus(GamingStatus::Default);
}

} // namespace ll

namespace ll::android {

void setAndroidRuntime(JavaVM* vm, std::filesystem::path const& modsRoot) noexcept {
    (void)vm;
    // The launcher passes the mod's own root directory. Shared code derives
    // the self-mod dir as `pl_mods_path / manifest.name`, so expose the
    // parent as the mods root and remember the exact self-mod root.
    auto selfRoot = modsRoot.lexically_normal();
    setAndroidPlConfig(selfRoot, false, 4);
    setSelfModRoot(selfRoot);
    __android_log_print(
        ANDROID_LOG_INFO,
        kLogTag,
        "pl_mods_path=%s pl_log_path=%s selfModRoot=%s",
        pl::pl_mods_path,
        pl::pl_log_path,
        selfRoot.string().c_str()
    );
}

} // namespace ll::android