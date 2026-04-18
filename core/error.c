/**
 * @file error.c
 * @brief SPP error reporting implementation.
 *
 * Thread-local (or static global for baremetal) storage for the last error
 * code and extended context.  String conversion via a compile-time switch.
 */

#include "spp/core/error.h"
#include "spp/util/macros.h"

#include <string.h>

/* ----------------------------------------------------------------
 * Internal buffer size for error strings
 * ---------------------------------------------------------------- */

#define ERR_STR_SIZE (64U)

/* ----------------------------------------------------------------
 * Thread-local (or global) state
 * ---------------------------------------------------------------- */

static SPP_THREAD_LOCAL SPP_RetVal_t    s_lastErr = K_SPP_OK;
static SPP_THREAD_LOCAL SPP_ErrCtx_t s_lastCtx =
{
    .code   = K_SPP_OK,
    .p_file = NULL,
    .line   = 0,
};

/* ----------------------------------------------------------------
 * Last-error get / set
 * ---------------------------------------------------------------- */

SPP_RetVal_t
SPP_CORE_errGet(void)
{
    return s_lastErr;
}

void
SPP_CORE_errSet(SPP_RetVal_t err)
{
    s_lastErr       = err;
    s_lastCtx.code  = err;
}

/* ----------------------------------------------------------------
 * Extended context get / set
 * ---------------------------------------------------------------- */

SPP_ErrCtx_t
SPP_CORE_errGetCtx(void)
{
    return s_lastCtx;
}

void
SPP_CORE_errSetCtx(SPP_RetVal_t code, const char *p_file, int line)
{
    s_lastErr       = code;
    s_lastCtx.code  = code;
    s_lastCtx.p_file = p_file;
    s_lastCtx.line  = line;
}

/* ----------------------------------------------------------------
 * String conversion
 * ---------------------------------------------------------------- */

const char *
SPP_CORE_errToString(SPP_RetVal_t err, char *p_buf, size_t bufLen)
{
    static SPP_THREAD_LOCAL char s_buf[ERR_STR_SIZE];

    const char *p_str;

    switch (err)
    {
        case K_SPP_OK:
            p_str = "OK";
            break;
        case K_SPP_ERROR:
            p_str = "generic error";
            break;
        case K_SPP_NOT_ENOUGH_PACKETS:
            p_str = "packet pool exhausted";
            break;
        case K_SPP_NULL_PACKET:
            p_str = "null packet pointer";
            break;
        case K_SPP_ERROR_ALREADY_INITIALIZED:
            p_str = "already initialized";
            break;
        case K_SPP_ERROR_NULL_POINTER:
            p_str = "null pointer argument";
            break;
        case K_SPP_ERROR_NOT_INITIALIZED:
            p_str = "not initialized";
            break;
        case K_SPP_ERROR_INVALID_PARAMETER:
            p_str = "invalid parameter";
            break;
        case K_SPP_ERROR_ON_SPI_TRANSACTION:
            p_str = "SPI transaction failed";
            break;
        case K_SPP_ERROR_TIMEOUT:
            p_str = "operation timed out";
            break;
        case K_SPP_ERROR_NO_PORT:
            p_str = "no OSAL/HAL port registered";
            break;
        case K_SPP_ERROR_REGISTRY_FULL:
            p_str = "service registry full";
            break;
        default:
            p_str = "unknown error";
            break;
    }

    if (p_buf != NULL)
    {
        strncpy(p_buf, p_str, bufLen - 1U);
        p_buf[bufLen - 1U] = '\0';
        return p_buf;
    }

    strncpy(s_buf, p_str, ERR_STR_SIZE - 1U);
    s_buf[ERR_STR_SIZE - 1U] = '\0';
    return s_buf;
}
