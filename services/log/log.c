/**
 * @file log.c
 * @brief SPP logging service implementation.
 */

#include "spp/services/log/log.h"

#include <stdio.h>
#include <stdarg.h>

#define K_LOG_BUF_SIZE (256U)

static SPP_LogLevel_t    s_level = K_SPP_LOG_VERBOSE;
static SPP_LogOutputFn_t s_outFn = NULL;

/* ----------------------------------------------------------------
 * Default output: print to stdout
 * ---------------------------------------------------------------- */

static void defaultOutput(const char *p_tag, SPP_LogLevel_t level,
                           const char *p_message)
{
    static const char k_lvl[] = "?EWID V";
    char c = k_lvl[(unsigned)level < sizeof(k_lvl) ? (unsigned)level : 0U];
    printf("[%c] %s: %s\n", c, p_tag, p_message);
}

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_SERVICES_LOG_init(void)
{
    s_level = K_SPP_LOG_VERBOSE;
    s_outFn = defaultOutput;
    return K_SPP_OK;
}

void SPP_SERVICES_LOG_setLevel(SPP_LogLevel_t level)
{
    s_level = level;
}

SPP_LogLevel_t SPP_SERVICES_LOG_getLevel(void)
{
    return s_level;
}

void SPP_SERVICES_LOG_setOutput(SPP_LogOutputFn_t p_fn)
{
    s_outFn = p_fn;
}

void SPP_SERVICES_LOG_emit(const char *p_tag, SPP_LogLevel_t level,
                            const char *p_fmt, ...)
{
    if ((level > s_level) || (s_outFn == NULL))
    {
        return;
    }

    char buf[K_LOG_BUF_SIZE];
    va_list args;
    va_start(args, p_fmt);
    (void)vsnprintf(buf, sizeof(buf), p_fmt, args);
    va_end(args);

    s_outFn(p_tag, level, buf);
}
