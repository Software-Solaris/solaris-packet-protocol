/**
 * @file databank.h
 * @brief Static packet pool manager.
 *
 * The databank maintains a fixed pool of @ref SPP_Packet_t objects.
 * Producers call @ref SPP_Databank_getPacket() to acquire a free packet,
 * fill it, and pass it to the @c db_flow FIFO.  Consumers return packets
 * via @ref SPP_Databank_returnPacket() after processing.
 *
 * The pool size is controlled by @ref K_SPP_DATABANK_SIZE (default 5).
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_SPP_DATABANK_*
 * - Types: SPP_Databank_t
 * - Public functions: SPP_Databank_*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_DATABANK_H
#define SPP_DATABANK_H

#include "spp/core/packet.h"
#include "spp/core/returntypes.h"
#include "spp/util/macros.h"

/* ----------------------------------------------------------------
 * Databank state type
 * ---------------------------------------------------------------- */

/**
 * @brief Internal databank control structure.
 *
 * Exposed here only to allow static allocation by the caller if needed;
 * treat all fields as private — use only the public API.
 */
typedef struct
{
    SPP_Packet_t  *p_freePackets[K_SPP_DATABANK_SIZE]; /**< Free packet stack.  */
    spp_uint32_t   freeCount;                          /**< Free packets count. */
} SPP_Databank_t;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Initialise the packet pool.
 *
 * Zeroes all packets and rebuilds the free stack.  Safe to call multiple
 * times; subsequent calls are no-ops and return
 * @c SPP_ERROR_ALREADY_INITIALIZED.
 *
 * @return SPP_OK on success, SPP_ERROR_ALREADY_INITIALIZED if already done.
 */
retval_t SPP_Databank_init(void);

/**
 * @brief Acquire a free packet from the pool.
 *
 * The returned packet is removed from the free list until returned via
 * @ref SPP_Databank_returnPacket().
 *
 * @return Pointer to a free @ref SPP_Packet_t, or NULL if the pool is empty.
 */
SPP_Packet_t *SPP_Databank_getPacket(void);

/**
 * @brief Return a packet to the pool after processing.
 *
 * @param[in] p_packet  Pointer previously returned by @ref SPP_Databank_getPacket().
 *
 * @return SPP_OK on success.
 * @return SPP_ERROR_NULL_POINTER if @p p_packet is NULL.
 * @return SPP_ERROR if @p p_packet is not a valid pool address (double-return guard).
 * @return SPP_ERROR_ALREADY_INITIALIZED if the packet is already in the free list.
 */
retval_t SPP_Databank_returnPacket(SPP_Packet_t *p_packet);

/**
 * @brief Return the number of free packets currently available.
 *
 * @return Free packet count (0 … K_SPP_DATABANK_SIZE).
 */
spp_uint32_t SPP_Databank_freeCount(void);

#endif /* SPP_DATABANK_H */
