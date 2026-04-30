/**
 * @file log.h
 * @brief SPP logging service.
 *
 * Usage:
 *   SPP_LOGI("MY_MODULE", "sensor value = %.2f", value);
 *
 * Each macro formats the message and calls the active output function.
 * By default that function prints to stdout.  SPP_CORE_boot() replaces
 * it with one that also publishes each message as a pub/sub packet so
 * subscribers (e.g. the SD card logger) receive log lines automatically.
 *
 * To silence everything: SPP_SERVICES_LOG_setLevel(K_SPP_LOG_NONE)
 * To change destination: SPP_SERVICES_LOG_setOutput(myFn)
 */

#ifndef SPP_LOG_H
#define SPP_LOG_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * Log levels  (higher value = more output)
 * ---------------------------------------------------------------- */

typedef enum
{
    K_SPP_LOG_NONE    = 0, /* Silence everything        */
    K_SPP_LOG_ERROR   = 1, /* Errors only               */
    K_SPP_LOG_WARN    = 2, /* Errors + warnings         */
    K_SPP_LOG_INFO    = 3, /* Normal operational output */
    K_SPP_LOG_DEBUG   = 4, /* Extra detail              */
    K_SPP_LOG_VERBOSE = 5  /* Everything                */
} SPP_LogLevel_t;

/* ----------------------------------------------------------------
 * Output function type
 *
 * A function that receives a fully-formatted log message and does
 * something with it (print it, write it to SD, publish it, …).
 * ---------------------------------------------------------------- */

typedef void (*SPP_LogOutputFn_t)(const char *p_tag, SPP_LogLevel_t level,
                                   const char *p_message);

/* ----------------------------------------------------------------
 * API
 * ---------------------------------------------------------------- */

SPP_RetVal_t   SPP_SERVICES_LOG_init(void);

void           SPP_SERVICES_LOG_setLevel(SPP_LogLevel_t level);
SPP_LogLevel_t SPP_SERVICES_LOG_getLevel(void);

/* Replace the active output function.  Pass NULL to silence all output. */
void           SPP_SERVICES_LOG_setOutput(SPP_LogOutputFn_t p_fn);

/* Internal — called by the macros below, do not call directly. */
void           SPP_SERVICES_LOG_emit(const char *p_tag, SPP_LogLevel_t level,
                                     const char *p_fmt, ...);

/* ----------------------------------------------------------------
 * Macros  — use these everywhere
 * ---------------------------------------------------------------- */

#define SPP_LOGE(tag, fmt, ...) SPP_SERVICES_LOG_emit((tag), K_SPP_LOG_ERROR,   (fmt), ##__VA_ARGS__)
#define SPP_LOGW(tag, fmt, ...) SPP_SERVICES_LOG_emit((tag), K_SPP_LOG_WARN,    (fmt), ##__VA_ARGS__)
#define SPP_LOGI(tag, fmt, ...) SPP_SERVICES_LOG_emit((tag), K_SPP_LOG_INFO,    (fmt), ##__VA_ARGS__)
#define SPP_LOGD(tag, fmt, ...) SPP_SERVICES_LOG_emit((tag), K_SPP_LOG_DEBUG,   (fmt), ##__VA_ARGS__)
#define SPP_LOGV(tag, fmt, ...) SPP_SERVICES_LOG_emit((tag), K_SPP_LOG_VERBOSE, (fmt), ##__VA_ARGS__)

#endif /* SPP_LOG_H */
