/**
 * @file db_flow.h
 * @brief Circular FIFO for routing filled packets from producers to consumers.
 *
 * Producers push packets obtained from @ref SPP_Databank_getPacket() into
 * this FIFO after filling them.  Consumers pop from it, process the packet,
 * and return it to the databank.
 *
 * The FIFO depth is controlled by @ref K_SPP_DBFLOW_READY_SIZE (default 16,
 * must be a power of two).
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_SPP_DBFLOW_*
 * - Types: SPP_DbFlow_t
 * - Public functions: SPP_DbFlow_*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_DB_FLOW_H
#define SPP_DB_FLOW_H

#include "spp/core/packet.h"
#include "spp/core/returntypes.h"
#include "spp/util/macros.h"

/* ----------------------------------------------------------------
 * DbFlow state type
 * ---------------------------------------------------------------- */

/**
 * @brief Internal db_flow control structure.
 *
 * Treat all fields as private; use only the public API.
 */
typedef struct
{
    SPP_Packet_t *p_ready[K_SPP_DBFLOW_READY_SIZE]; /**< Circular packet buffer. */
    spp_uint32_t  head;                              /**< Write index.            */
    spp_uint32_t  tail;                              /**< Read index.             */
    spp_uint32_t  count;                             /**< Number of items stored. */
} SPP_DbFlow_t;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Initialise the db_flow FIFO.
 *
 * Safe to call multiple times; subsequent calls are no-ops and return
 * @c SPP_ERROR_ALREADY_INITIALIZED.
 *
 * @return SPP_OK on success, SPP_ERROR_ALREADY_INITIALIZED if already done.
 */
retval_t SPP_DbFlow_init(void);

/**
 * @brief Push a filled packet into the ready FIFO.
 *
 * @param[in] p_pkt  Pointer to the filled packet.
 *
 * @return SPP_OK on success.
 * @return SPP_ERROR_NULL_POINTER if @p p_pkt is NULL.
 * @return SPP_ERROR if the FIFO is full.
 */
retval_t SPP_DbFlow_pushReady(SPP_Packet_t *p_pkt);

/**
 * @brief Pop the next available packet from the ready FIFO.
 *
 * @param[out] pp_pkt  Set to the dequeued packet pointer.
 *
 * @return SPP_OK on success.
 * @return SPP_ERROR_NULL_POINTER if @p pp_pkt is NULL.
 * @return SPP_NOT_ENOUGH_PACKETS if the FIFO is empty.
 */
retval_t SPP_DbFlow_popReady(SPP_Packet_t **pp_pkt);

/**
 * @brief Return the number of packets currently in the FIFO.
 *
 * @return Packet count (0 … K_SPP_DBFLOW_READY_SIZE).
 */
spp_uint32_t SPP_DbFlow_readyCount(void);

#endif /* SPP_DB_FLOW_H */
