add_rules("mode.release", "mode.debug")

set_xmakever("3.0.0")

add_repositories("levimc-repo " .. (get_config("levimc_repo") or "https://github.com/LiteLDev/xmake-repo.git"))

local is_windows  = is_plat("windows")
local is_linux    = is_plat("linux")
local is_android  = is_plat("android")

local is_server = is_config("target_type", "server")



-- Dependencies from xmake-repo.
-- Header-only packages: safe for every platform, including the Android
-- cross-build (no host-arch static/shared libs get installed).
add_requires("ctre 3.8.1")
add_requires("entt v3.15.0")
add_requires("expected-lite v0.8.0")
add_requires("gsl v4.2.0")
add_requires("glm 1.0.1")
add_requires("parallel-hashmap v2.0.0")
add_requires("concurrentqueue v1.0.4")
add_requires("stb 2025.03.14")

if is_linux or (not is_android) then
    add_requires("type_safe v0.2.4")
end

if not is_android then
    -- Compiled libs from the official xmake-repo. On Android these would be
    -- installed as host-arch archives and fail the NDK cross-arch link
    -- (/usr/bin/ld: skipping incompatible libfmt.a). The preloader-android
    -- runtime vendors fmt/nlohmann_json/Boost::pfr/magic_enum into
    -- libpreloader.so (FetchContent in its CMakeLists.txt), so the mod must
    -- NOT compile/link its own copies.
    add_requires("fmt 11.2.0")
    add_requires("leveldb 1.23")
    add_requires("magic_enum v0.9.7")
    add_requires("nlohmann_json v3.12.0")
    add_requires("rapidjson 2025.02.05")
    add_requires("mimalloc v3.5.0")
    add_requires("cpr[ssl=y] 1.11.1")
end

-- Dependencies from levimc-repo.
add_requires("pcg_cpp v1.0.0")
add_requires("pfr 2.1.1")
add_requires("trampoline 2024.11.7")

-- levibuildscript / preloader / symbolprovider are desktop-only: the
-- Android mod gets its runtime (`symbolprovider` hooks, `preloader`
-- types) from the preloader_android package instead. Their source
-- (e.g. symbolprovider's SymbolProvider.cpp includes <windows.h>) does
-- not cross-compile.
if not is_android then
    add_requires("levibuildscript 0.6.1")
    add_requires("preloader v1.16.2")
    add_requires("symbolprovider v1.3.0")
    add_requires("demangler v17.0.7")
end

if is_windows then
add_requires("libhat 0.4.0")
set_toolchains("clang-cl")
end

if is_linux then
set_toolchains("clang")
end

if is_android then
-- Android NDK clang. Kept out of the clang toolchain block above to avoid
-- confusing the standard `is_linux` path.
set_toolchains("clang")
end

if has_config("tests") then
    add_requires("gtest")
end

if is_server then
    if is_windows then
        add_requires("bedrockdata v26.40.8-server.3")
    end
else
    if is_windows then
        add_requires("bedrockdata v26.40.5-client.3")
    end
end

if is_android then
    -- The preloader-android runtime (libpreloader.so) publishes the
    -- pl:: memory/hook/symbol API the LeviLamina shim forwards to.
    add_requires("preloader_android 0.2.3")
end

option("levimc_repo")
    set_default("https://github.com/LiteLDev/xmake-repo.git")
    set_showmenu(true)
    set_description("Set the levimc-repo path or url")
option_end()

option("publish")
    set_default(false)
    set_showmenu(true)
option_end()

option("tests")
    set_default(false)
    set_showmenu(true)
    set_description("Enable tests")
option_end()

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

if is_server then
    set_defaultarchs("windows|x64")
    set_allowedarchs("windows|x64", "linux|x86_64")
    if is_android then
        set_allowedarchs("android|arm64-v8a", "android|armeabi-v7a")
    end
else
    set_defaultarchs("windows|x64")
    set_allowedarchs("windows|x64")
end

if is_windows and not has_config("vs_runtime") then
    set_runtimes("MD")
end

if is_linux or is_android then
    set_runtimes("c++_shared")
end

target("LeviLamina")
    -- linkrule runs the Windows BDS prelink.exe; not applicable to Android
    -- (no bedrock_runtime_data/prelink.exe in the NDK). The Android mod's
    -- runtime image needs no relocations against the dedicated server DLL.
    if not is_android then
        add_rules("@levibuildscript/linkrule")
    end
    set_languages("c++20")
    set_kind("shared")
    set_symbols("debug")
    add_files("src/**.cpp")
    add_files("src/**.rc")
    set_configdir("$(builddir)/config")
    set_configvar("LL_WORKSPACE_FOLDER", "$(projectdir)")
    add_configfiles("src/(ll/core/Version.h.in)")
    add_headerfiles("src/(ll/api/**.h)", "src/(mc/**.h)")
    add_includedirs("src", "$(builddir)/config")
    set_pcxxheader("src/ll/api/Global.h")
    if is_android then
        -- The preloader-android runtime ships its own pl:: headers (Mod.hpp,
        -- Config.hpp, ...) which would collide with the C shim.  The mod
        -- interface used by ApexAntLamina is provided by src-pl-android/pl,
        -- so only the runtime/link hooks come from the package.
        add_packages("preloader_android")
        add_packages("ctre", "trampoline")
    else
        add_packages("demangler", "mimalloc", "ctre", "cpr", "trampoline", "preloader")
    end
    local common_packages = {
        "entt",
        "expected-lite",
        "gsl",
        "glm",
        "pcg_cpp",
        "pfr",
        "parallel-hashmap",
        "concurrentqueue",
        "stb",
        {public = true}
    }
    if not is_android then
        -- Compiled libs are desktop-only; on Android they are skipped at
        -- add_requires time and would otherwise break add_packages here.
        local desktop_packages = {
            "fmt",
            "leveldb",
            "magic_enum",
            "nlohmann_json",
            "rapidjson",
            "type_safe",
            "symbolprovider",
        }
        for _, p in ipairs(desktop_packages) do
            table.insert(common_packages, #common_packages, p)
        end
    end
    if is_windows then
        -- BDS runtime data (headers + bedrock_runtime_data) is a Windows-only
        -- artifact; it has no Android repackage.
        table.insert(common_packages, 14, "bedrockdata")
    end
    add_packages(common_packages)
    add_defines("LL_EXPORT")
    add_defines(
        "FMT_USE_FULL_CACHE_DRAGONBOX=1",
        "ENTT_PACKED_PAGE=128",
        "ENTT_SPARSE_PAGE=2048",
        {public = true}
    )

    if not is_windows then
        add_cxxflags("clang::-Wno-invalid-offsetof")
        add_defines(
            "__GCC_DESTRUCTIVE_SIZE=64",
            "__GCC_CONSTRUCTIVE_SIZE=64",
            {tools = {"clang"}}
        )
    end

    -- work around to enable c++23

    if is_windows then
        -- msstl
        add_defines("_HAS_CXX23=1")
    else
        -- libc++
        add_cxxflags("clang::-stdlib=libc++")
        add_defines("_LIBCPP_STD_VER=23")
    end

    if is_windows then
        add_packages("libhat")
        add_syslinks("Version", "DbgHelp", "dwrite")
        add_defines(
            "_AMD64_",
            "NOMINMAX",
            "UNICODE",
            "WIN32_LEAN_AND_MEAN"
        )
        set_exceptions("none")
        add_cxflags(
            "/utf-8",
            "/permissive-",
            "/EHa",
            "/W4",
            "/w44265",
            "/w44289",
            "/w44296",
            "/w45263",
            "/w44738"
        )
        add_cxflags(
            "/EHs",
            "-Wno-microsoft-cast",
            "-Wno-invalid-offsetof",
            "-Wno-c++2b-extensions",
            "-Wno-microsoft-include",
            "-Wno-overloaded-virtual",
            "-Wno-ignored-qualifiers",
            "-Wno-missing-field-initializers",
            "-Wno-potentially-evaluated-expression",
            "-Wno-pragma-system-header-outside-header",
            {tools = {"clang_cl"}}
        )
    end

    if is_server then
        add_headerfiles("src-server/(ll/api/**.h)", "src-server/(mc/**.h)")
        add_includedirs("src-server")
        add_files("src-server/**.cpp")
        add_defines("LL_PLAT_S")
    else
        if is_windows then
            add_syslinks("dxgi", "runtimeobject", "gdi32")
        end
        add_headerfiles("src-client/(ll/api/**.h)", "src-client/(mc/**.h)")
        add_includedirs("src-client")
        add_defines("LL_PLAT_C")
        add_files("src-client/**.cpp")
        add_cxflags("/wd4273")
        add_shflags("/IGNORE:4217")
    end

    if is_android then
        -- ApexAntLamina Android shim: pl:: preloader bridge + replaced platform files.
        add_defines("LL_ANDROID")
        add_includedirs("src-pl-android", "src-pl-export")
        add_headerfiles("src-pl-android/(pl/**.h)", "src-pl-android/(ll/**.h)")
        add_files("src-pl-android/**.cpp")
        -- PLGetModRegistration: ABI entry that the preloader calls with a
        -- pl::mod context; needs preloader_android's pl/Mod.hpp.
        add_files("src-pl-export/mod_registration.cpp")
        add_packages("preloader_android")
        add_syslinks("log", "dl", "android", "jnigraphics")
        add_cxflags("clang::-fno-aligned-allocation")
        -- LeviLauncher preloads the mod .so named `preload-<...>.so` and the
        -- manifest `entry` field maps to it (see src/pl/internal/ModManifest.cpp).
        -- The `lib` prefix follows the Android native library convention so
        -- AGP can bundle it directly into a host APK.
        set_filename("libpreload-levilamina.so")
        -- The desktop server registers game singletons through hooks whose
        -- target symbols (DedicatedServer, ServerScriptManager, ...) are not
        -- exported by libminecraftpe.so on Android. The shim replaces them.
        remove_files(
            "src-server/ll/api/service/TargetedBedrock.cpp",
            "src-server/ll/api/service/ServerInfo.cpp",
            "src-server/ll/api/event/command/ServerCommandRegisterEvent.cpp",
            "src-server/ll/core/command/BuiltinCommands.cpp"
        )
        -- Windows-only: console/AIO hooks, BDS entry.
        remove_files("src-server/ll/core/main_win.cpp", "src-server/ll/core/io/Output_win.cpp")
        -- Android replaces the shared self-mod bootstrap (getSelfModIns) with
        -- LeviLamina_android.cpp because the desktop layout derives the path
        -- from `getModsRoot()/LeviLamina`, which is not valid under a
        -- launcher-provided per-mod root.
        remove_files("src/ll/core/LeviLamina.cpp")
        -- Server statistics/behavior tweaks that require the BDS dedicated
        -- server loop symbols; not applicable inside the launcher process.
        remove_files(
            "src-server/ll/core/Statistics.cpp",
            "src-server/ll/core/tweak/ModifyBedrockLogInfo.cpp",
            "src-server/ll/core/tweak/SimpleServerLogger.cpp",
            "src-server/ll/core/tweak/ProtocolCompatibility.cpp",
            "src-server/ll/core/tweak/VulnerabilityFixes.cpp"
        )
        -- Shared Bedrock.cpp installs a DBStorage constructor hook carrying a
        -- reference to a Windows-only symbol; the Android shim supplies
        -- setDBStorage()/publishGamePointers() instead.
        remove_files("src/ll/api/service/Bedrock.cpp")
        -- Shared Config.cpp (LL_CONFIG_IMPL) resolves the config from the
        -- self mod's config dir; the Android shim adds a pre-launch fallback.
        -- Both define getLeviConfig()/saveLeviConfig(), so exclude the shared
        -- implementation.
        remove_files("src/ll/core/Config.cpp")
        -- SystemUtils_linux.cpp reads /proc/self/maps to find the module
        -- base; the Android shim uses dladdr() because the main executable
        -- is the game, not ApexAntLamina. Exclude the linux implementation.
        remove_files("src/ll/api/utils/SystemUtils_linux.cpp")
    end

    if has_config("tests") then
        add_defines("LL_DEBUG")
        add_packages("gtest")
            add_includedirs("src-test/common/")
            add_files("src-test/common/**.cpp")
        if is_server then
            add_includedirs("src-test/server/")
            add_files("src-test/server/**.cpp")
        else
            add_includedirs("src-test/client/")
            add_files("src-test/client/**.cpp")
        end
        before_build(function (target)
            headers = ""
            for _,x in ipairs(os.files("src/**.h")) do
                headers = headers.."#include \""..path.relative(x, "src/").."\"\n"
            end
            file = io.open("src-test/common/include_all.cpp", "w")
            file:write(headers)
            file:close()
        if is_server then
            headers = ""
            for _,x in ipairs(os.files("src-server/**.h")) do
                headers = headers.."#include \""..path.relative(x, "src-server/").."\"\n"
            end
            file = io.open("src-test/server/include_all.cpp", "w")
            file:write(headers)
            file:close()
        else
            headers = ""
            for _,x in ipairs(os.files("src-client/**.h")) do
                headers = headers.."#include \""..path.relative(x, "src-client/").."\"\n"
            end
            file = io.open("src-test/client/include_all.cpp", "w")
            file:write(headers)
            file:close()
        end
        end)
        after_build(function (target)
            io.writefile("src-test/common/include_all.cpp", "// auto gen when build test\n")
            if is_server then
                io.writefile("src-test/server/include_all.cpp", "// auto gen when build test\n")
            else
                io.writefile("src-test/client/include_all.cpp", "// auto gen when build test\n")
            end
        end)
    end

    if has_config("publish") then
        add_defines("LL_VERSION_PUBLISH")
    end

    if is_mode("debug") then
        add_defines("LL_DEBUG")
    end

    if not is_windows then
        remove_files("./**/*_win.*")
        remove_files("./**/*_windows.*")
        remove_files("./**/win/**.*")
        remove_headerfiles("./**/win/**.*")
    end
    if not is_linux then
        remove_files("./**/*_linux.*")
        remove_files("./**/linux/**.*")
        remove_headerfiles("./**/linux/**.*")
    end

    on_config(function (target)
        -- vs_runtime can not be set to MDd or MTd.
        if has_config("vs_runtime") and (get_config("vs_runtime"):endswith("d")) then
            raise("LeviLamina: vs_runtime can not be set to MDd or MTd")
        end
    end)

    on_load(function (target)
        import("core.base.json")
        local major, minor, patch, suffix
        local function parse_version(str)
            return str:match("v?%s*(%d+)%.(%d+)%.(%d+)(.*)")
        end
        -- CI clones are tag-less (no `git describe`); read the version from
        -- tooth.json and fall back to 0.0.0. Never shell out to git here.
        local tooth = json.loadfile("tooth.json")
        local version = tooth and tooth["version"] or ""
        major, minor, patch, suffix = parse_version(version)
        if not major then
            major, minor, patch, suffix = "0", "0", "0", nil
        end
        local versionStr =  major.."."..minor.."."..patch
        if suffix then
            prerelease = suffix:match("-(.*)")
            if prerelease then
                prerelease = prerelease:gsub("\n", "")
            end
            if prerelease then
                target:set("configvar", "LL_VERSION_PRERELEASE", prerelease)
                versionStr = versionStr.."-"..prerelease
            end
        end
        target:set("configvar", "LL_VERSION_MAJOR", major)
        target:set("configvar", "LL_VERSION_MINOR", minor)
        target:set("configvar", "LL_VERSION_PATCH", patch)

        local rule_config = {
            modVersion = versionStr,
        }
        if is_android then
            -- LeviLaunchroid mod packaging is done in CI (bin/ApexAntLamina/):
            -- the preloader reads `manifest.json` next to <entry> (see
            -- preloader-android ModManifest.cpp). No levibuildscript rule here:
            -- it is desktop-only and unavailable on Android.
            rule_config.modName     = "ApexAntLamina"
            rule_config.modFile     = "preload-levilamina.so"
            rule_config.modPlatform = "android-arm64"
            rule_config.modVersion  = versionStr
            -- Keep the rule_config values visible to CI packaging by writing
            -- them into the target data (CI can't read Lua locals).
            target:data_set("modpacker", rule_config)
        else
            target:add("rules", "@levibuildscript/modpacker", rule_config)
        end
    end)
