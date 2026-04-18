/**
 * @file log.h
 * @brief SPP filtered logging service.
 *
 * Provides level-filtered log macros (@ref SPP_LOGE … @ref SPP_LOGV) that
 * dispatch through a user-registered output callback.  The default output
 * callback prints to stdout; replace it for embedded targets.
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_SPP_LOG_*, SPP_LOG*
 * - Types: SPP_LogLevel_t, SPP_LogOutputFn_t
 * - Public functions: SPP_SERVICES_LOG_*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_LOG_H
#define SPP_LOG_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * Log level enumeration
 * ---------------------------------------------------------------- */

/**
 * @brief Log verbosity levels (higher value = more output).
 */
typedef enum
{
    K_SPP_LOG_NONE    = 0, /**< Disable all log output.  */
    K_SPP_LOG_ERROR   = 1, /**< Critical errors only.    */
    K_SPP_LOG_WARN    = 2, /**< Warnings and errors.     */
    K_SPP_LOG_INFO    = 3, /**< Informational messages.  */
    K_SPP_LOG_DEBUG   = 4, /**< Debug detail.            */
    K_SPP_LOG_VERBOSE = 5  /**< Full trace output.       */
} SPP_LogLevel_t;

/* ----------------------------------------------------------------
 * Output callback type
 * ---------------------------------------------------------------- */

/**
 * @brief Signature of the log output callback.
 *
 * @param[in] p_tag     Module tag string (e.g. "SPP_CORE").
 * @param[in] level     Log level of this message.
 * @param[in] p_message Formatted message string.
 */
typedef void (*SPP_LogOutputFn_t)(const char *p_tag, SPP_LogLevel_t level,
                                   const char *p_message);

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Initialise the logging service.
 *
 * Sets the default level to K_SPP_LOG_VERBOSE and registers a stdout
 * callback.  Safe to call multiple times.
 *
 * @return K_SPP_OK on success.
 */
SPP_RetVal_t SPP_SERVICES_LOG_init(void);

/**
 * @brief Set the global log level filter.
 *
 * Messages with a level higher than @p level are suppressed.
 *
 * @param[in] level  New log level.
 */
void SPP_SERVICES_LOG_setLevel(SPP_LogLevel_t level);

/**
 * @brief Return the current global log level.
 *
 * @return Current @ref SPP_LogLevel_t.
 */
SPP_LogLevel_t SPP_SERVICES_LOG_getLevel(void);

/**
 * @brief Register a custom log output callback.
 *
 * Replaces the default stdout callback.  Pass NULL to silence all output.
 *
 * @param[in] p_fn  Callback function pointer (may be NULL).
 */
void SPP_SERVICES_LOG_registerOutput(SPP_LogOutputFn_t p_fn);

/**
 * @brief Internal log emit function — use macros instead of calling directly.
 *
 * @param[in] p_tag   Module tag.
 * @param[in] level   Message level.
 * @param[in] p_fmt   printf-style format string.
 * @param[in] ...     Format arguments.
 */
void SPP_SERVICES_LOG_emit(const char *p_tag, SPP_LogLevel_t level,
                  const char *p_fmt, ...);

/* ----------------------------------------------------------------
 * Log macros
 * ---------------------------------------------------------------- */

/** @brief Log an error message. */
#define SPP_LOGE(tag, fmt, ...) \
    SPP_SERVICES_LOG_emit((tag), K_SPP_LOG_ERROR,   (fmt), ##__VA_ARGS__)

/** @brief Log a warning message. */
#define SPP_LOGW(tag, fmt, ...) \
    SPP_SERVICES_LOG_emit((tag), K_SPP_LOG_WARN,    (fmt), ##__VA_ARGS__)

/** @brief Log an informational message. */
#define SPP_LOGI(tag, fmt, ...) \
    SPP_SERVICES_LOG_emit((tag), K_SPP_LOG_INFO,    (fmt), ##__VA_ARGS__)

/** @brief Log a debug message. */
#define SPP_LOGD(tag, fmt, ...) \
    SPP_SERVICES_LOG_emit((tag), K_SPP_LOG_DEBUG,   (fmt), ##__VA_ARGS__)

/** @brief Log a verbose trace message. */
#define SPP_LOGV(tag, fmt, ...) \
    SPP_SERVICES_LOG_emit((tag), K_SPP_LOG_VERBOSE, (fmt), ##__VA_ARGS__)

#endif /* SPP_LOG_H */
