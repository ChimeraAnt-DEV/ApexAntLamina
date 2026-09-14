// Common::getBuildInfo for Android.
//
// The desktop builds import this symbol from bedrock_server (the game
// exposes it through its DLL/executable exports). libminecraftpe.so on
// Android does not expose it, so the version strings used by
// ll::api::getGameVersion() etc. are synthesized here. They are also what
// the LeviLaunchroid preloader injects as the game's build info on load,
// keeping everything consistent with the game that is actually running.

#include "mc/common/BuildInfo.h"
#include "mc/common/Common.h"

namespace Common {

::Common::BuildInfo getBuildInfo() {
    ::Common::BuildInfo info{};
    info.mGameVersion = "1.21.51.02";
    info.mFlavor      = "Android";
    return info;
}

} // namespace Common