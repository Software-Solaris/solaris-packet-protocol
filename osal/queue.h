// Aquí se definen las funciones que quieres en el queue.c



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

#include <stdint.h>
#include <stdbool.h>
#include "core/returntypes.h"

#ifdef __cplusplus // Es un wrapper de compatibilidad entre C y C++
extern "C" {
#endif

/**
 * @brief OSAL Queue handle type
 */
typedef void* osal_queue_handle_t;


/**
 * @brief Crea una nueva cola OSAL
 * 
 * @param queue_length Número máximo de elementos en la cola
 * @param item_size Tamaño en bytes de cada elemento
 * 
 * @return Puntero void a handle de la cola creada
 * @retval NULL si hubo error al crear la cola
 * 
 * @note El handle debe ser usado con las demás funciones OSAL
 */
void* SPP_OSAL_QueueCreate(uint32_t queue_length, uint32_t item_size);

/**
 * @brief Crea una nueva cola OSAL con memoria estática
 * 
 * @param queue_length Número máximo de elementos en la cola
 * @param item_size Tamaño en bytes de cada elemento
 * @param queue_storage
 * @param queue_buffer
 * 
 * @return Puntero void a handle de la cola creada
 * @retval NULL si hubo error al crear la cola
 * 
 * @note El handle debe ser usado con las demás funciones OSAL
 */
void* SPP_OSAL_QueueCreateStatic(uint32_t queue_length, uint32_t item_size, uint8_t* queue_storage, void* queue_buffer);

/**
 * @brief Obtiene el número de elementos en cola
 * 
 * @param queue_handle Handle de la cola
 * @return uint32_t Número de elementos en cola
 */
uint32_t SPP_OSAL_QueueMessagesWaiting(void* queue_handle);

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
 * @param item Pointer to item to send
 * @param timeout_ms Timeout in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t OSAL_QueueSend(osal_queue_handle_t queue_handle, const void* item, uint32_t timeout_ms);

/**
 * @brief Send item to queue from ISR
 * 
 * @param queue_handle Queue handle
 * @param item Pointer to item to send
 * @param higher_priority_task_woken Pointer to flag indicating if higher priority task was woken
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t OSAL_QueueSendFromISR(osal_queue_handle_t queue_handle, const void* item, bool* higher_priority_task_woken);

/**
 * @brief Receive item from queue
 * 
 * @param queue_handle Queue handle
 * @param item Pointer to buffer for received item
 * @param timeout_ms Timeout in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t OSAL_QueueReceive(osal_queue_handle_t queue_handle, void* item, uint32_t timeout_ms);

/**
 * @brief Receive item from queue from ISR
 * 
 * @param queue_handle Queue handle
 * @param item Pointer to buffer for received item
 * @param higher_priority_task_woken Pointer to flag indicating if higher priority task was woken
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t OSAL_QueueReceiveFromISR(osal_queue_handle_t queue_handle, void* item, bool* higher_priority_task_woken);

/**
 * @brief Peek at item in queue without removing it
 * 
 * @param queue_handle Queue handle
 * @param item Pointer to buffer for peeked item
 * @param timeout_ms Timeout in milliseconds
 * @return retval_t SPP_OK on success, error code otherwise
 */
retval_t OSAL_QueuePeek(osal_queue_handle_t queue_handle, void* item, uint32_t timeout_ms);

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

#ifdef __cplusplus
}
#endif

#endif /* SPP_OSAL_QUEUE_H */ 