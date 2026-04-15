/**
 * @file queue.h
 * @brief SPP message queue API — dispatches through the registered OSAL port.
 *
 * Naming conventions used in this file:
 * - Public functions: SPP_Osal_queue*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_OSAL_QUEUE_H
#define SPP_OSAL_QUEUE_H

#include "spp/core/types.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Create a message queue.
 *
 * @param[in] len       Maximum number of items the queue can hold.
 * @param[in] itemSize  Size of each item in bytes.
 *
 * @return Opaque queue handle on success, NULL on failure.
 */
void *SPP_Osal_queueCreate(spp_uint32_t len, spp_uint32_t itemSize);

/**
 * @brief Send (enqueue) an item — copy semantics.
 *
 * @param[in] p_q        Queue handle.
 * @param[in] p_item     Pointer to the item to copy.
 * @param[in] timeoutMs  Maximum time to wait if the queue is full (ms).
 *
 * @return SPP_OK on success, SPP_ERROR on timeout or full queue.
 */
retval_t SPP_Osal_queueSend(void *p_q, const void *p_item,
                              spp_uint32_t timeoutMs);

/**
 * @brief Receive (dequeue) an item — copy semantics.
 *
 * @param[in]  p_q        Queue handle.
 * @param[out] p_item     Buffer to copy the item into.
 * @param[in]  timeoutMs  Maximum time to wait if the queue is empty (ms).
 *
 * @return SPP_OK on success, SPP_ERROR on timeout or empty queue.
 */
retval_t SPP_Osal_queueRecv(void *p_q, void *p_item,
                              spp_uint32_t timeoutMs);

/**
 * @brief Return the number of items currently in the queue.
 *
 * @param[in] p_q  Queue handle.
 *
 * @return Item count.
 */
spp_uint32_t SPP_Osal_queueCount(void *p_q);

#endif /* SPP_OSAL_QUEUE_H */
