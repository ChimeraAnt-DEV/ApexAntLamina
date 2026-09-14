#pragma once

#include "ll/api/base/Macro.h"

namespace ll {

enum class GamingStatus : int {
    Default = 0,
    Starting,
    Running,
    Stopping,
};

LLNDAPI GamingStatus getGamingStatus();

LLNDAPI void setGamingStatus(GamingStatus value);

} // namespace ll
