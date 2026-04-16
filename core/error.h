/**
 * @file error.h
 * @brief SPP error reporting — get/set last error, convert to string.
 *
 * Mirrors the pattern from lley-core's errnum.h:
 *
 *   SPP_ERR_get() / SPP_ERR_set()   — thread-local last error
 *                                     (static global when SPP_NO_RTOS=1)
 *   SPP_ERR_toString()              — pointer to internal buffer (not reentrant)
 *   SPP_ERR_toString_r()            — caller-supplied buffer (reentrant)
 *   SPP_ErrCtx_t                    — extended context: code + file + line
 *   SPP_ERR_SET(code)               — set context with __FILE__ / __LINE__
 *
 * Naming conventions used in this file:
 *   - Types:     SPP_Err*_t
 *   - Functions: SPP_Err_*()
 *   - Macros:    SPP_ERR_*
 */

#ifndef SPP_ERROR_H
#define SPP_ERROR_H

#include "spp/core/returntypes.h"
#include <stddef.h>

/* ----------------------------------------------------------------
 * Thread-local keyword — absent on baremetal (single-threaded)
 * ---------------------------------------------------------------- */

#if defined(SPP_NO_RTOS) && (SPP_NO_RTOS == 1)
#  define SPP_THREAD_LOCAL /* empty — single-threaded baremetal */
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define SPP_THREAD_LOCAL _Thread_local
#else
#  define SPP_THREAD_LOCAL /* fallback: static global */
#endif

/* ----------------------------------------------------------------
 * Extended error context
 * ---------------------------------------------------------------- */

/**
 * @brief Extended error context — error code plus source location.
 *
 * Set via SPP_ERR_SET(); retrieved via SPP_ERR_getCtx().
 */
typedef struct
{
    SPP_RetVal_t    code;    /**< The error code.                        */
    const char *p_file;  /**< Source file name (__FILE__), or NULL.  */
    int         line;    /**< Source line number (__LINE__), or 0.   */
} SPP_ErrCtx_t;

/* ----------------------------------------------------------------
 * Last-error get / set
 * ---------------------------------------------------------------- */

/**
 * @brief Returns the last error code set on the calling thread.
 *
 * Equivalent to lley-core's get_errc().
 *
 * @return The last @ref SPP_RetVal_t value stored by SPP_ERR_set() or
 *         SPP_ERR_SET(), or K_SPP_OK if none has been set.
 */
SPP_RetVal_t SPP_ERR_get(void);

/**
 * @brief Sets the last error code for the calling thread.
 *
 * Equivalent to lley-core's set_errc().
 *
 * @param[in] err Error code to store.
 */
void SPP_ERR_set(SPP_RetVal_t err);

/* ----------------------------------------------------------------
 * Extended context get / set
 * ---------------------------------------------------------------- */

/**
 * @brief Returns the extended error context for the calling thread.
 *
 * @return A copy of the last @ref SPP_ErrCtx_t stored by SPP_ERR_setCtx()
 *         or SPP_ERR_SET().
 */
SPP_ErrCtx_t SPP_ERR_getCtx(void);

/**
 * @brief Sets the extended error context for the calling thread.
 *
 * Prefer the SPP_ERR_SET() macro, which fills @p p_file and @p line
 * automatically via the preprocessor.
 *
 * @param[in] code   Error code.
 * @param[in] p_file Source file name (typically __FILE__).
 * @param[in] line   Source line number (typically __LINE__).
 */
void SPP_ERR_setCtx(SPP_RetVal_t code, const char *p_file, int line);

/* ----------------------------------------------------------------
 * String conversion
 * ---------------------------------------------------------------- */

/**
 * @brief Returns a human-readable string for @p err.
 *
 * Uses an internal thread-local (or static) buffer.  The pointer is
 * valid until the next call to SPP_ERR_toString() on the same thread.
 * Equivalent to lley-core's errc2str().
 *
 * @param[in] err Error code.
 * @return Null-terminated ASCII string, never NULL.
 */
static inline const char *SPP_ERR_toString(SPP_RetVal_t err);

/**
 * @brief Reentrant variant — copies the error string into @p p_buf.
 *
 * Equivalent to lley-core's errc2str_r().
 *
 * @param[in]  err     Error code.
 * @param[out] p_buf   Destination buffer, or NULL to use the internal buffer.
 * @param[in]  bufLen  Size of @p p_buf in bytes. Ignored when @p p_buf is NULL.
 * @return @p p_buf when not NULL; internal buffer otherwise.
 */
const char *SPP_ERR_toString_r(SPP_RetVal_t err, char *p_buf, size_t bufLen);

/* ----------------------------------------------------------------
 * Inline implementation of SPP_ERR_toString
 * ---------------------------------------------------------------- */

static inline const char *
SPP_ERR_toString(SPP_RetVal_t err)
{
    return SPP_ERR_toString_r(err, NULL, 0U);
}

/* ----------------------------------------------------------------
 * Convenience macro — set context with automatic file / line
 * ---------------------------------------------------------------- */

/**
 * @brief Set the last error with source location captured automatically.
 *
 * @code
 *     if (p_data == NULL)
 *         SPP_ERR_RETURN(K_SPP_ERROR_NULL_POINTER);
 * @endcode
 */
#define SPP_ERR_SET(code) SPP_ERR_setCtx((code), __FILE__, __LINE__)

/**
 * @brief Set error context and return in one step.
 *
 * Equivalent to calling SPP_ERR_SET() followed by return. Ensures every
 * error path captures the source location so callers can retrieve it via
 * SPP_ERR_getCtx() and print it with SPP_ERR_toString().
 *
 * @code
 *     if (p_buf == NULL)
 *         SPP_ERR_RETURN(K_SPP_ERROR_NULL_POINTER);
 * @endcode
 */
#define SPP_ERR_RETURN(code) \
    do { SPP_ERR_SET(code); return (code); } while (0)

#endif /* SPP_ERROR_H */
