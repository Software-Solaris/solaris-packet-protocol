/**
 * @file log.c
 * @brief SPP logging service implementation.
 */

#include "spp/services/log/log.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* ----------------------------------------------------------------
 * Private constants
 * ---------------------------------------------------------------- */

/** @brief Maximum length of a single formatted log message. */
#define K_LOG_BUF_SIZE (256U)

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

static SPP_LogLevel_t    s_level    = K_SPP_LOG_VERBOSE;
static SPP_LogOutputFn_t s_p_outFn  = NULL;
static spp_bool_t        s_initialized = false;

/* ----------------------------------------------------------------
 * Default stdout output callback
 * ---------------------------------------------------------------- */

static void defaultOutput(const char *p_tag, SPP_LogLevel_t level,
                           const char *p_message)
{
    static const char *const k_levelStr[] = {
        "", "E", "W", "I", "D", "V"
    };

    spp_uint32_t idx = (spp_uint32_t)level;
    if (idx >= (sizeof(k_levelStr) / sizeof(k_levelStr[0])))
    {
        idx = 0U;
    }

    printf("[%s] %s: %s\n", k_levelStr[idx], p_tag, p_message);
}

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_Log_init(void)
{
    s_level       = K_SPP_LOG_VERBOSE;
    s_p_outFn     = defaultOutput;
    s_initialized = true;
    return K_SPP_OK;
}

void SPP_Log_setLevel(SPP_LogLevel_t level)
{
    s_level = level;
}

SPP_LogLevel_t SPP_Log_getLevel(void)
{
    return s_level;
}

void SPP_Log_registerOutput(SPP_LogOutputFn_t p_fn)
{
    s_p_outFn = p_fn;
}

void SPP_Log_emit(const char *p_tag, SPP_LogLevel_t level,
                  const char *p_fmt, ...)
{
    if (!s_initialized || (level > s_level) || (s_p_outFn == NULL))
    {
        return;
    }

    char buf[K_LOG_BUF_SIZE];
    va_list args;
    va_start(args, p_fmt);
    (void)vsnprintf(buf, sizeof(buf), p_fmt, args);
    va_end(args);

    s_p_outFn(p_tag, level, buf);
}
