// ll::io::defaultOutputImpl for Android.
//
// The logger post-processes every line through this function. On Android
// inside LeviLauncher there is no attached console for stdout (logcat owns
// the diagnostics stream), so lines are forwarded to both: fwrite keeps
// Termux/pipes working, and logcat gives launcher users a way to inspect
// output through the system log.

#include "ll/api/io/FileUtils.h"
#include "ll/core/io/Output.h"

#include "ll/api/utils/StringUtils.h"

#include <cstdio>
#include <string_view>

#include <android/log.h>

namespace ll::io {
void defaultOutputImpl(std::string_view sv) {
    fwrite(sv.data(), sizeof(char), sv.size(), stdout);
    fflush(stdout);
    __android_log_print(
        ANDROID_LOG_INFO,
        "LeviLamina",
        "%s",
        string_utils::tou8str(sv).c_str()
    );
}
} // namespace ll::io