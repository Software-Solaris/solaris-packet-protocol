/**
 * @file port.h
 * @brief SPP OSAL port contract — the interface every OS port must implement.
 *
 * To port SPP to a new OS (FreeRTOS, POSIX, ThreadX, bare-metal…):
 *   1. Allocate a static @ref SPP_OsalPort_t.
 *   2. Fill every function pointer with your OS implementation.
 *   3. Call @ref SPP_Core_setOsalPort() before @ref SPP_Core_init().
 *
 * All function pointers are mandatory unless documented as optional (NULL-safe).
 *
 * Naming conventions used in this file:
 * - Types: SPP_OsalPort_t
 * - Pointer parameters: p_*
 */

#ifndef SPP_OSAL_PORT_H
#define SPP_OSAL_PORT_H

#include "spp/core/types.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * OSAL port struct
 * ---------------------------------------------------------------- */

/**
 * @brief OS abstraction layer port descriptor.
 *
 * Register one instance per build target via @ref SPP_Core_setOsalPort().
 */
typedef struct
{
    /* ---- Tasks ------------------------------------------------- */

    /**
     * @brief Create an OS task / thread.
     *
     * @param[in] p_fn         Task entry function.
     * @param[in] p_name       Human-readable name (may be ignored on some OS).
     * @param[in] stackWords   Stack size in words (not bytes).
     * @param[in] p_arg        Argument passed to @p p_fn.
     * @param[in] prio         Priority (0 = lowest; use K_SPP_OSAL_PRIO_* values).
     *
     * @return Opaque task handle, or NULL on failure.
     */
    void *(*taskCreate)(void (*p_fn)(void *), const char *p_name,
                        spp_uint32_t stackWords, void *p_arg,
                        spp_uint32_t prio);

    /**
     * @brief Delete a task.  NULL deletes the calling task.
     *
     * @param[in] p_handle  Task handle returned by @c taskCreate, or NULL.
     */
    void (*taskDelete)(void *p_handle);

    /**
     * @brief Delay the calling task for at least @p ms milliseconds.
     *
     * @param[in] ms  Delay duration in milliseconds.
     */
    void (*taskDelayMs)(spp_uint32_t ms);

    /**
     * @brief Return elapsed system time in milliseconds.
     *
     * @return Monotonic tick count in ms (wraps at UINT32_MAX).
     */
    spp_uint32_t (*getTickMs)(void);

    /* ---- Queues ------------------------------------------------ */

    /**
     * @brief Create a message queue.
     *
     * @param[in] len       Maximum number of items in the queue.
     * @param[in] itemSize  Size of each item in bytes.
     *
     * @return Opaque queue handle, or NULL on failure.
     */
    void *(*queueCreate)(spp_uint32_t len, spp_uint32_t itemSize);

    /**
     * @brief Send an item to a queue (copy semantics).
     *
     * @param[in] p_q        Queue handle.
     * @param[in] p_item     Pointer to the item to copy in.
     * @param[in] timeoutMs  Max wait time; 0 = non-blocking.
     *
     * @return SPP_OK on success, SPP_ERROR on timeout or full queue.
     */
    retval_t (*queueSend)(void *p_q, const void *p_item,
                          spp_uint32_t timeoutMs);

    /**
     * @brief Receive an item from a queue (copy semantics).
     *
     * @param[in]  p_q        Queue handle.
     * @param[out] p_item     Buffer to copy the item into.
     * @param[in]  timeoutMs  Max wait time; 0 = non-blocking.
     *
     * @return SPP_OK on success, SPP_ERROR on timeout or empty queue.
     */
    retval_t (*queueRecv)(void *p_q, void *p_item, spp_uint32_t timeoutMs);

    /**
     * @brief Return the number of items currently waiting in the queue.
     *
     * @param[in] p_q  Queue handle.
     *
     * @return Item count.
     */
    spp_uint32_t (*queueCount)(void *p_q);

    /* ---- Mutexes ----------------------------------------------- */

    /**
     * @brief Create a non-recursive mutex.
     *
     * @return Opaque mutex handle, or NULL on failure.
     */
    void *(*mutexCreate)(void);

    /**
     * @brief Acquire a mutex.
     *
     * @param[in] p_m        Mutex handle.
     * @param[in] timeoutMs  Max wait time; 0 = non-blocking.
     *
     * @return SPP_OK on success, SPP_ERROR on timeout.
     */
    retval_t (*mutexLock)(void *p_m, spp_uint32_t timeoutMs);

    /**
     * @brief Release a mutex.
     *
     * @param[in] p_m  Mutex handle.
     *
     * @return SPP_OK on success.
     */
    retval_t (*mutexUnlock)(void *p_m);

    /* ---- Event groups ------------------------------------------ */

    /**
     * @brief Create an event group (bitfield of up to 24 event bits).
     *
     * @return Opaque event group handle, or NULL on failure.
     */
    void *(*eventCreate)(void);

    /**
     * @brief Wait for one or more event bits to be set.
     *
     * @param[in]  p_e          Event group handle.
     * @param[in]  bits         Bitmask of bits to wait for.
     * @param[in]  clearOnExit  Clear matched bits before returning.
     * @param[in]  waitAll      Wait for ALL bits (true) or ANY bit (false).
     * @param[in]  timeoutMs    Max wait time; 0 = non-blocking.
     * @param[out] p_actualBits Bits set at the time of return (may be NULL).
     *
     * @return SPP_OK when the condition is satisfied, SPP_ERROR_TIMEOUT otherwise.
     */
    retval_t (*eventWait)(void *p_e, spp_uint32_t bits,
                          spp_bool_t clearOnExit, spp_bool_t waitAll,
                          spp_uint32_t timeoutMs,
                          spp_uint32_t *p_actualBits);

    /**
     * @brief Set event bits from an ISR context.
     *
     * @param[in]  p_e      Event group handle.
     * @param[in]  bits     Bitmask of bits to set.
     * @param[out] p_prev   Previous bit value (may be NULL).
     * @param[out] p_yield  Set to true if a higher-priority task was unblocked.
     *
     * @return SPP_OK on success.
     */
    retval_t (*eventSetFromIsr)(void *p_e, spp_uint32_t bits,
                                spp_uint32_t *p_prev,
                                spp_bool_t   *p_yield);

} SPP_OsalPort_t;

/* ----------------------------------------------------------------
 * Recommended priority levels
 * ---------------------------------------------------------------- */

#define K_SPP_OSAL_PRIO_IDLE     (0U)  /**< Idle / background priority. */
#define K_SPP_OSAL_PRIO_LOW      (1U)  /**< Below-normal priority.      */
#define K_SPP_OSAL_PRIO_NORMAL   (2U)  /**< Normal priority.            */
#define K_SPP_OSAL_PRIO_HIGH     (3U)  /**< Above-normal priority.      */
#define K_SPP_OSAL_PRIO_CRITICAL (4U)  /**< Highest user priority.      */

#endif /* SPP_OSAL_PORT_H */
