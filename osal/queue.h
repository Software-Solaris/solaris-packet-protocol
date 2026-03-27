/**
 * @file queue.h
 * @brief OSAL Queue Management Interface
 * @version 1.0.0
 * @date 2024
 *
 * This file provides the queue management interface for the OSAL layer.
 */

#ifndef SPP_OSAL_QUEUE_H
#define SPP_OSAL_QUEUE_H

#include "spp/core/returntypes.h"
#include "spp/core/types.h"

#ifdef __cplusplus /* C/C++ compatibility wrapper */
extern "C"
{
#endif

    /**
 * @brief OSAL Queue handle type
 */
    typedef void *osal_queue_handle_t;


    /**
 * @brief Create a new OSAL queue
 *
 * @param queue_length Maximum number of items in the queue
 * @param item_size    Size in bytes of each item
 *
 * @return void* Handle to the created queue
 * @retval NULL if queue creation failed
 *
 * @note The handle must be used with the other OSAL queue functions
 */
    void *SPP_OSAL_QueueCreate(uint32_t queue_length, uint32_t item_size);

    /**
 * @brief Create a new OSAL queue with static memory
 *
 * @param queue_length  Maximum number of items in the queue
 * @param item_size     Size in bytes of each item
 * @param queue_storage Pointer to the static storage area for queue items
 * @param queue_buffer  Pointer to the static queue control structure
 *
 * @return void* Handle to the created queue
 * @retval NULL if queue creation failed
 *
 * @note The handle must be used with the other OSAL queue functions
 */
    void *SPP_OSAL_QueueCreateStatic(uint32_t queue_length, uint32_t item_size,
                                     uint8_t *queue_storage, void *queue_buffer);

    /**
 * @brief Get the number of messages waiting in a queue
 *
 * @param queue_handle Handle of the queue
 * @return uint32_t Number of items currently in the queue
 */
    uint32_t SPP_OSAL_QueueMessagesWaiting(void *queue_handle);

    /**
 * @brief Delete a queue
 *
 * @param queue_handle Queue handle
 * @return retval_t SPP_OK on success, error code otherwise
 */
    retval_t OSAL_QueueDelete(osal_queue_handle_t queue_handle);

    /**
 * @brief Send item to queue
 *
 * @param queue_handle Queue handle
 * @param p_item       Pointer to item to send
 * @param timeout_ms   Timeout in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
    retval_t SPP_OSAL_QueueSend(void *queue_handle, const void *p_item, uint32_t timeout_ms);

    /**
 * @brief Send item to queue from ISR
 *
 * @param queue_handle              Queue handle
 * @param item                      Pointer to item to send
 * @param higher_priority_task_woken Pointer to flag indicating if higher priority task was woken
 * @return retval_t SPP_OK on success, error code otherwise
 */
    retval_t OSAL_QueueSendFromISR(osal_queue_handle_t queue_handle, const void *item,
                                   bool *higher_priority_task_woken);

    /**
 * @brief Receive item from queue
 *
 * @param queue_handle Queue handle
 * @param p_out_item   Pointer to buffer for received item
 * @param timeout_ms   Timeout in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
    retval_t SPP_OSAL_QueueReceive(void *queue_handle, void *p_out_item, uint32_t timeout_ms);

    /**
 * @brief Receive item from queue from ISR
 *
 * @param queue_handle              Queue handle
 * @param item                      Pointer to buffer for received item
 * @param higher_priority_task_woken Pointer to flag indicating if higher priority task was woken
 * @return retval_t SPP_OK on success, error code otherwise
 */
    retval_t OSAL_QueueReceiveFromISR(osal_queue_handle_t queue_handle, void *item,
                                      bool *higher_priority_task_woken);

    /**
 * @brief Peek at item in queue without removing it
 *
 * @param queue_handle Queue handle
 * @param item         Pointer to buffer for peeked item
 * @param timeout_ms   Timeout in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
    retval_t OSAL_QueuePeek(osal_queue_handle_t queue_handle, void *item, uint32_t timeout_ms);

    /**
 * @brief Get number of items in queue
 *
 * @param queue_handle Queue handle
 * @return uint32_t Number of items in queue
 */
    uint32_t OSAL_QueueGetCount(osal_queue_handle_t queue_handle);

    /**
 * @brief Get available space in queue
 *
 * @param queue_handle Queue handle
 * @return uint32_t Available space in queue
 */
    uint32_t OSAL_QueueGetSpace(osal_queue_handle_t queue_handle);

    /**
 * @brief Check if queue is full
 *
 * @param queue_handle Queue handle
 * @return bool true if queue is full, false otherwise
 */
    bool OSAL_QueueIsFull(osal_queue_handle_t queue_handle);

    /**
 * @brief Check if queue is empty
 *
 * @param queue_handle Queue handle
 * @return bool true if queue is empty, false otherwise
 */
    bool OSAL_QueueIsEmpty(osal_queue_handle_t queue_handle);

    /**
 * @brief Reset queue (remove all items)
 *
 * @param queue_handle Queue handle
 * @return retval_t SPP_OK on success, error code otherwise
 */
    retval_t OSAL_QueueReset(osal_queue_handle_t queue_handle);

    retval_t SPP_OSAL_QueueReset(void *queue_handle);


#ifdef __cplusplus
}
#endif

#endif /* SPP_OSAL_QUEUE_H */
