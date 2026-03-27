/**
 * @file databank.h
 * @brief SPP Databank service - static packet pool manager.
 *
 * Provides a fixed-size pool of spp_packet_t packets. Consumers acquire
 * packets with SPP_DATABANK_getPacket() and release them back with
 * SPP_DATABANK_returnPacket().
 */

#ifndef DATABANK_H
#define DATABANK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "spp/core/macros.h"
#include "spp/core/packet.h"
#include "spp/core/returntypes.h"
#include "spp/services/logging/spp_log.h"
#include <stdint.h>

    /**
     * @brief Databank control structure.
     *
     * Holds an array of pointers to free packets and a count of how many
     * are currently available.
     */
    typedef struct
    {
        spp_packet_t *freePackets[DATA_BANK_SIZE]; /**< Array of free packet pointers */
        uint32_t numberOfFreePackets;              /**< Number of packets still available */
    } SPP_Databank_t;

    /**
 * @brief Initialise the databank packet pool.
 *
 * Zeroes every packet in the pool and populates the free-pointer array.
 * Safe to call more than once; subsequent calls return SPP_OK immediately.
 *
 * @return SPP_OK on success.
 */
    retval_t SPP_DATABANK_init(void);

    /**
 * @brief Acquire a free packet from the pool.
 *
 * Returns a pointer to the next available packet and removes it from the
 * free list.  The caller owns the packet until it is returned via
 * SPP_DATABANK_returnPacket().
 *
 * @return Pointer to a free packet, or NULL if the pool is exhausted or
 *         the databank has not been initialised.
 */
    spp_packet_t *SPP_DATABANK_getPacket(void);

    /**
 * @brief Return a previously acquired packet to the pool.
 *
 * The packet is zeroed before being placed back in the free list.
 * Double-returns and foreign pointers are detected and rejected.
 *
 * @param[in] p_packet Pointer to the packet to return.
 * @return SPP_OK on success, SPP_NULL_PACKET if p_packet is NULL,
 *         SPP_ERROR on any other failure.
 */
    retval_t SPP_DATABANK_returnPacket(spp_packet_t *p_packet);

#ifdef __cplusplus
}
#endif

#endif /* DATABANK_H */
