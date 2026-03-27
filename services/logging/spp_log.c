/**
 * @file spp_log.c
 * @brief SPP logging service implementation.
 *
 * Provides level-filtered logging with an optional custom output callback.
 * When no callback is registered, messages are printed to stdout.
 */

#include "spp/services/logging/spp_log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Static variables                                                   */
/* ------------------------------------------------------------------ */

static spp_log_level_t s_currentLogLevel = SPP_LOG_INFO;
static spp_log_output_fn_t s_outputCallback = NULL;
static spp_bool_t s_isInitialized = false;

/** Human-readable names for each log level */
static const char *s_logLevelStrings[] = {
    [SPP_LOG_NONE] = "NONE", [SPP_LOG_ERROR] = "ERROR", [SPP_LOG_WARN] = "WARN",
    [SPP_LOG_INFO] = "INFO", [SPP_LOG_DEBUG] = "DEBUG", [SPP_LOG_VERBOSE] = "VERBOSE"};

/* ------------------------------------------------------------------ */
/*  Private helpers                                                    */
/* ------------------------------------------------------------------ */

/**
 * @brief Default log output handler (console).
 *
 * Prints messages to stdout in the format: [LEVEL] [TAG] message
 *
 * @param[in] p_tag   Log tag string.
 * @param[in] level   Log level of the message.
 * @param[in] p_format printf-style format string.
 * @param[in] args    Variadic argument list.
 */
static void default_log_output(const char *p_tag, spp_log_level_t level, const char *p_format,
                               va_list args)
{
    const char *p_levelStr = s_logLevelStrings[level];

    /* Format: [LEVEL] [TAG] message */
    printf("[%s] [%s] ", p_levelStr, p_tag);
    vprintf(p_format, args);
    printf("\n");
}

/* ------------------------------------------------------------------ */
/*  Public functions                                                    */
/* ------------------------------------------------------------------ */

/**
 * @brief Initialise the logging system.
 *
 * Sets the default log level to INFO and clears any registered callback.
 *
 * @return SPP_OK on success, SPP_ERROR_ALREADY_INITIALIZED if already
 *         initialised.
 */
retval_t SPP_LOG_Init(void)
{
    if (s_isInitialized)
    {
        return SPP_ERROR_ALREADY_INITIALIZED;
    }

    s_currentLogLevel = SPP_LOG_INFO;
    s_outputCallback = NULL;
    s_isInitialized = true;
    SPP_LOGI("LOG", "Logging system initialized");

    return SPP_OK;
}

/**
 * @brief Set the active log level.
 *
 * Messages below this level will be suppressed.
 *
 * @param[in] level Desired log level.
 */
void SPP_LOG_SetLevel(spp_log_level_t level)
{
    if (level <= SPP_LOG_VERBOSE)
    {
        s_currentLogLevel = level;
        SPP_LOGI("LOG", "Log level set to: %s", s_logLevelStrings[level]);
    }
}

/**
 * @brief Get the current log level.
 *
 * @return Active log level.
 */
spp_log_level_t SPP_LOG_GetLevel(void)
{
    return s_currentLogLevel;
}

/**
 * @brief Register a custom output callback for log messages.
 *
 * When set, all log output is routed through this callback instead of
 * the default console handler.
 *
 * @param[in] p_callback Function pointer to the custom output handler.
 */
void SPP_LOG_RegisterOutputCallback(spp_log_output_fn_t p_callback)
{
    s_outputCallback = p_callback;
    SPP_LOGI("LOG", "Custom output callback registered");
}

/**
 * @brief Internal helper to retrieve the current log level.
 *
 * Used by the SPP_LOG* macros; do not call directly.
 *
 * @return Current log level.
 */
spp_log_level_t spp_log_current_level_get(void)
{
    return s_currentLogLevel;
}

/**
 * @brief Internal log writer.
 *
 * Formats and emits a log message through the registered callback or
 * the default console handler.  Silently returns if the system has not
 * been initialised.
 *
 * @param[in] level   Log level of the message.
 * @param[in] p_tag   Log tag string.
 * @param[in] p_format printf-style format string.
 * @param[in] ...     Additional arguments for the format string.
 */
void spp_log_write(spp_log_level_t level, const char *p_tag, const char *p_format, ...)
{
    if (!s_isInitialized)
    {
        /* Silent fallback when not yet initialised */
        return;
    }

    if (p_tag == NULL || p_format == NULL)
    {
        /* Silent error to avoid null dereference */
        return;
    }

    va_list args;
    va_start(args, p_format);

    if (s_outputCallback != NULL)
    {
        /* Use custom callback if registered */
        s_outputCallback(p_tag, level, p_format, args);
    }
    else
    {
        /* Use default console output */
        default_log_output(p_tag, level, p_format, args);
    }

    va_end(args);
}
