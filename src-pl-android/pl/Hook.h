#pragma once

#include "pl/Macro.h"

#ifdef __cplusplus
namespace pl::inline hook {
#endif

typedef void* FuncPtr;

enum Priority {
    PriorityHighest = 0,
    PriorityHigh    = 100,
    PriorityNormal  = 200,
    PriorityLow     = 300,
    PriorityLowest  = 400,
};

enum RegisterSaveOptions {
    SaveNone = 0,
    SaveR10  = 1 << 0,
    SaveR11  = 1 << 1,
    SaveXmm4 = 1 << 2,
    SaveXmm5 = 1 << 3,
    SaveAll  = SaveR10 | SaveR11 | SaveXmm4 | SaveXmm5
};

/**
 * @brief Hook a function with a detour.
 *
 * @return int 0 on success, non-zero on failure.
 */
PLCAPI int pl_hook(FuncPtr target, FuncPtr detour, FuncPtr* originalFunc, Priority priority);

/**
 * @brief Hook a function with a detour, preserving extra registers.
 *
 * @return int 0 on success, non-zero on failure.
 */
PLCAPI int
pl_hook_ex(FuncPtr target, FuncPtr detour, FuncPtr* originalFunc, Priority priority, RegisterSaveOptions options);

/**
 * @brief Unhook a function.
 *
 * @return true on success, false on failure.
 */
PLCAPI bool pl_unhook(FuncPtr target, FuncPtr detour);

/**
 * @brief Set register save options for a hook.
 *
 * @return true on success, false on failure.
 */
PLCAPI bool pl_hook_options(FuncPtr target, RegisterSaveOptions options, bool replace);

#ifdef __cplusplus
} // namespace pl::inline hook
#endif