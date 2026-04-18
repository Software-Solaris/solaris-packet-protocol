/**
 * @file databank.h
 * @brief Static packet pool manager.
 *
 * The databank maintains a fixed pool of @ref SPP_Packet_t objects.
 * Producers call @ref SPP_SERVICES_DATABANK_getPacket() to acquire a free packet,
 * fill it, and pass it to the @c db_flow FIFO.  Consumers return packets
 * via @ref SPP_SERVICES_DATABANK_returnPacket() after processing.
 *
 * The pool size is controlled by @ref K_SPP_DATABANK_SIZE (default 5).
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_SPP_DATABANK_*
 * - Types: SPP_Databank_t
 * - Public functions: SPP_SERVICES_DATABANK_*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_DATABANK_H
#define SPP_DATABANK_H

#include "spp/core/packet.h"
#include "spp/core/returnTypes.h"
#include "spp/core/types.h"
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
 * @c K_SPP_ERROR_ALREADY_INITIALIZED.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_ALREADY_INITIALIZED if already done.
 */
SPP_RetVal_t SPP_SERVICES_DATABANK_init(void);

/**
 * @brief Acquire a free packet from the pool.
 *
 * The returned packet is removed from the free list until returned via
 * @ref SPP_SERVICES_DATABANK_returnPacket().
 *
 * @return Pointer to a free @ref SPP_Packet_t, or NULL if the pool is empty.
 */
SPP_Packet_t *SPP_SERVICES_DATABANK_getPacket(void);

/**
 * @brief Return a packet to the pool after processing.
 *
 * @param[in] p_packet  Pointer previously returned by @ref SPP_SERVICES_DATABANK_getPacket().
 *
 * @return K_SPP_OK on success.
 * @return K_SPP_ERROR_NULL_POINTER if @p p_packet is NULL.
 * @return K_SPP_ERROR if @p p_packet is not a valid pool address (double-return guard).
 * @return K_SPP_ERROR_ALREADY_INITIALIZED if the packet is already in the free list.
 */
SPP_RetVal_t SPP_SERVICES_DATABANK_returnPacket(SPP_Packet_t *p_packet);

/**
 * @brief Return the number of free packets currently available.
 *
 * @return Free packet count (0 … K_SPP_DATABANK_SIZE).
 */
spp_uint32_t SPP_SERVICES_DATABANK_freeCount(void);

/**
 * @brief Fill a packet with data and compute its CRC.
 *
 * Zeroes the entire packet (including any struct padding), writes all header
 * fields, copies @p p_data into the payload, and computes a CRC-16/CCITT
 * checksum over every byte of the packet except the @c crc field itself.
 *
 * The timestamp is captured automatically via @ref SPP_HAL_getTimeMs().
 *
 * @param[out] p_packet  Packet previously acquired from @ref SPP_SERVICES_DATABANK_getPacket().
 * @param[in]  apid      Application Process Identifier.
 * @param[in]  seq       Packet sequence counter (maintained by the caller).
 * @param[in]  p_data    Pointer to the payload data to copy.
 * @param[in]  dataLen   Number of bytes to copy (must be ≤ K_SPP_PKT_PAYLOAD_MAX).
 *
 * @return K_SPP_OK on success.
 * @return K_SPP_ERROR_NULL_POINTER if @p p_packet or @p p_data is NULL.
 * @return K_SPP_ERROR_INVALID_PARAMETER if @p dataLen exceeds K_SPP_PKT_PAYLOAD_MAX.
 */
SPP_RetVal_t SPP_SERVICES_DATABANK_packetData(SPP_Packet_t *p_packet, spp_uint16_t apid,
                                      spp_uint16_t seq, const void *p_data,
                                      spp_uint16_t dataLen);

#endif /* SPP_DATABANK_H */
