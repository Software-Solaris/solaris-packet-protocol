/**
 * @file spp_log.h
 * @brief SPP logging service - level-filtered log output.
 *
 * Provides log macros (SPP_LOGE, SPP_LOGW, SPP_LOGI, SPP_LOGD, SPP_LOGV)
 * that check the current log level before emitting output.  A custom
 * output callback can be registered to redirect log messages.
 */

#ifndef SPP_LOG_H
#define SPP_LOG_H

#include "spp/core/types.h"
#include "spp/core/returntypes.h"

typedef enum
{
    SPP_LOG_NONE = 0,
    SPP_LOG_ERROR,
    SPP_LOG_WARN,
    SPP_LOG_INFO,
    SPP_LOG_DEBUG,
    SPP_LOG_VERBOSE
} spp_log_level_t;

/** @brief Initialise the logging system. */
retval_t SPP_LOG_Init(void);

/** @brief Set the active log level. */
void SPP_LOG_SetLevel(spp_log_level_t level);

/** @brief Get the current log level. */
spp_log_level_t SPP_LOG_GetLevel(void);

/** @brief Callback type for custom log output. */
typedef void (*spp_log_output_fn_t)(const char *tag, spp_log_level_t level, const char *format,
                                    ...);

/** @brief Register a custom output callback for log messages. */
void SPP_LOG_RegisterOutputCallback(spp_log_output_fn_t callback);

/* ------------------------------------------------------------------ */
/*  Logging macros                                                     */
/* ------------------------------------------------------------------ */

#define SPP_LOGE(tag, format, ...)                                    \
    do                                                                \
    {                                                                 \
        if (SPP_LOG_ERROR <= spp_log_current_level_get())             \
        {                                                             \
            spp_log_write(SPP_LOG_ERROR, tag, format, ##__VA_ARGS__); \
        }                                                             \
    } while (0)

#define SPP_LOGW(tag, format, ...)                                   \
    do                                                               \
    {                                                                \
        if (SPP_LOG_WARN <= spp_log_current_level_get())             \
        {                                                            \
            spp_log_write(SPP_LOG_WARN, tag, format, ##__VA_ARGS__); \
        }                                                            \
    } while (0)

#define SPP_LOGI(tag, format, ...)                                   \
    do                                                               \
    {                                                                \
        if (SPP_LOG_INFO <= spp_log_current_level_get())             \
        {                                                            \
            spp_log_write(SPP_LOG_INFO, tag, format, ##__VA_ARGS__); \
        }                                                            \
    } while (0)

#define SPP_LOGD(tag, format, ...)                                    \
    do                                                                \
    {                                                                 \
        if (SPP_LOG_DEBUG <= spp_log_current_level_get())             \
        {                                                             \
            spp_log_write(SPP_LOG_DEBUG, tag, format, ##__VA_ARGS__); \
        }                                                             \
    } while (0)

#define SPP_LOGV(tag, format, ...)                                      \
    do                                                                  \
    {                                                                   \
        if (SPP_LOG_VERBOSE <= spp_log_current_level_get())             \
        {                                                               \
            spp_log_write(SPP_LOG_VERBOSE, tag, format, ##__VA_ARGS__); \
        }                                                               \
    } while (0)

/* Internal functions (do not call directly) */
spp_log_level_t spp_log_current_level_get(void);
void spp_log_write(spp_log_level_t level, const char *tag, const char *format, ...);

#endif /* SPP_LOG_H */
