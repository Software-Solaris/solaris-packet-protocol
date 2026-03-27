/**
 * @file db_flow.h
 * @brief Databank flow service - circular FIFO for ready packets.
 *
 * Provides a lightweight ring buffer that routes filled spp_packet_t
 * pointers from producers to consumers.
 */

#ifndef DB_FLOW_H
#define DB_FLOW_H

#include "spp/core/returntypes.h"
#include "spp/core/packet.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Ready-queue depth (tunable, small for debug) */
#ifndef DB_FLOW_READY_SIZE
#define DB_FLOW_READY_SIZE 16
#endif

    /**
     * @brief Initialise the ready FIFO.
     *
     * Resets head, tail and count to zero and NULLs every slot.
     *
     * @return SPP_OK on success.
     */
    retval_t DB_FLOW_Init(void);

    /**
     * @brief Push a filled packet into the ready FIFO.
     *
     * @param[in] p_pkt Pointer to the packet to enqueue.
     * @return SPP_OK on success, SPP_ERROR_NULL_POINTER if p_pkt is NULL,
     *         SPP_ERROR if the FIFO is full or not initialised.
     */
    retval_t DB_FLOW_PushReady(spp_packet_t *p_pkt);

    /**
     * @brief Pop the next packet from the ready FIFO.
     *
     * @param[out] pp_pkt Receives the dequeued packet pointer.
     * @return SPP_OK on success, SPP_ERROR_NULL_POINTER if pp_pkt is NULL,
     *         SPP_ERROR if the FIFO is empty or not initialised.
     */
    retval_t DB_FLOW_PopReady(spp_packet_t **pp_pkt);

    /**
     * @brief Return the number of packets currently in the ready FIFO.
     *
     * @return Current packet count.
     */
    uint32_t DB_FLOW_ReadyCount(void);

#ifdef __cplusplus
}
#endif

#endif /* DB_FLOW_H */
