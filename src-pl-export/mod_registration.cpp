// LeviLauncher / preloader-android ABI bridge for ApexAntLamina.
//
// LeviLauncher loads a "preload-native" mod by dlopen'ing its entry .so and
// then calling PLGetModRegistration() to obtain the C++ lifecycle dispatch
// table. This translation unit implements that entry point and drives the
// LeviLamina lifecycle:
//
//   load   -> make LeviLamina loadable in the launcher process
//   enable -> launch the LeviLamina core (self mod, config, mod loading)
//   unload -> tear down the LeviLamina core
//
// LeviLamina is designed for a single-instance lifetime, so the state
// machine is intentionally simple and idempotent.

#include <jni.h>

#include <filesystem>
#include <memory>
#include <new>
#include <string>

#include "pl/Mod.hpp"

namespace ll {
void launch();
void unload();
} // namespace ll

namespace ll::android {
void setAndroidRuntime(JavaVM* vm, std::filesystem::path const& modsRoot) noexcept;
}

namespace {

class ApexAntLaminaMod {
    bool mLoaded  = false;
    bool mEnabled = false;

public:
    bool load(pl::mod::ModContext& context) {
        if (mLoaded) {
            return true;
        }
        try {
            auto& info = context.info();
            ll::android::setAndroidRuntime(context.javaVm(), info.modRootPath);
            mLoaded = true;
            return true;
        } catch (...) {
            return false;
        }
    }

    bool enable(pl::mod::ModContext& context) {
        (void)context;
        if (!mLoaded || mEnabled) {
            return mEnabled;
        }
        try {
            ll::launch();
            mEnabled = true;
        } catch (...) {
            mEnabled = false;
        }
        return mEnabled;
    }

    bool disable(pl::mod::ModContext& context) {
        (void)context;
        if (!mEnabled) {
            return true;
        }
        try {
            ll::unload();
            mEnabled = false;
        } catch (...) {
            return false;
        }
        return true;
    }

    bool unload(pl::mod::ModContext& context) {
        (void)context;
        disable(context);
        mLoaded = false;
        return true;
    }
};

ApexAntLaminaMod& instance() {
    static ApexAntLaminaMod mod;
    return mod;
}
} // namespace

extern "C" PL_EXPORT ::pl::mod::ModRegistration* PLGetModRegistration() {
    static ::pl::mod::ModRegistration registration{
        .instance = &instance(),
        .load     = +[](void* mod, pl::mod::ModContext& context) -> bool {
            return static_cast<ApexAntLaminaMod*>(mod)->load(context);
        },
        .enable = +[](void* mod, pl::mod::ModContext& context) -> bool {
            return static_cast<ApexAntLaminaMod*>(mod)->enable(context);
        },
        .disable = +[](void* mod, pl::mod::ModContext& context) -> bool {
            return static_cast<ApexAntLaminaMod*>(mod)->disable(context);
        },
        .unload = +[](void* mod, pl::mod::ModContext& context) -> bool {
            return static_cast<ApexAntLaminaMod*>(mod)->unload(context);
        },
    };
    return &registration;
}