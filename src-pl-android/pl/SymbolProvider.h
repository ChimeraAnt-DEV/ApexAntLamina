#pragma once

#include <cstddef>

#include "pl/Macro.h"

#ifdef __cplusplus
namespace pl::inline symbol_provider {
#endif

/**
 * @brief Resolve a symbol name to a function address.
 *
 * The Android port resolves symbols lazily from the game module at runtime.
 * If the symbol is not exported by any loaded module, nullptr is returned.
 *
 * @return result The function address. nullptr if the function is not found.
 */
PLCAPI void* pl_resolve_symbol(const char* symbolName);

/**
 * @brief Resolve a symbol name to a function address.
 *
 * @param symbolName [in] The symbol name.
 * @param symbolName [in] The name size.
 * @return result The function address. nullptr if the function is not found.
 */
PLCAPI void* pl_resolve_symbol_n(const char* symbolName, size_t size);

/**
 * @brief Resolve a symbol name to a function address.
 *
 * @note This function will not print error message if the function is not found.
 */
PLCAPI void* pl_resolve_symbol_silent(const char* symbolName);

/**
 * @brief Resolve a symbol name to a function address.
 *
 * @param symbolName [in] The symbol name.
 * @param symbolName [in] The name size.
 * @return result The function address. nullptr if the function is not found.
 *
 * @note This function will not print error message if the function is not found.
 */
PLCAPI void* pl_resolve_symbol_silent_n(const char* symbolName, size_t size);

/* DEPRECATED DONT USE*/
PLCAPI const char* const* pl_lookup_symbol(void* func, size_t* resultLength);

/* DEPRECATED DONT USE*/
PLCAPI const char* const* pl_lookup_symbol_disp(void* func, size_t* resultLength, unsigned int* displacement);

/* DEPRECATED DONT USE*/
PLCAPI void pl_free_lookup_result(const char* const* result);

#ifdef __cplusplus
} // namespace pl::symbol_provider
#endif