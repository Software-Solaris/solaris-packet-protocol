/**
 * @file db_flow.c
 * @brief Databank flow service implementation.
 *
 * Implements a circular FIFO of spp_packet_t pointers used to route
 * filled packets from producers to consumers.
 */

#include "spp/services/db_flow/db_flow.h"
#include "spp/services/logging/spp_log.h"
#include <stddef.h>

/* ------------------------------------------------------------------ */
/*  Static variables                                                   */
/* ------------------------------------------------------------------ */

static const char *TAG = "DB_FLOW";

static spp_packet_t *s_ready[DB_FLOW_READY_SIZE];
static uint32_t s_head = 0;
static uint32_t s_tail = 0;
static uint32_t s_count = 0;
static uint8_t s_initialized = 0;

/* ------------------------------------------------------------------ */
/*  Public functions                                                    */
/* ------------------------------------------------------------------ */

/**
 * @brief Initialise the ready FIFO.
 *
 * @return SPP_OK on success.
 */
retval_t DB_FLOW_Init(void)
{
    s_head = 0;
    s_tail = 0;
    s_count = 0;
    for (uint32_t i = 0; i < DB_FLOW_READY_SIZE; i++)
    {
        s_ready[i] = NULL;
    }
    s_initialized = 1;
    SPP_LOGI(TAG, "Ready FIFO init size=%u", (unsigned)DB_FLOW_READY_SIZE);
    return SPP_OK;
}

/**
 * @brief Push a filled packet into the ready FIFO.
 *
 * @param[in] p_pkt Pointer to the packet to enqueue.
 * @return SPP_OK on success, SPP_ERROR if FIFO is full.
 */
retval_t DB_FLOW_PushReady(spp_packet_t *p_pkt)
{
    if (!s_initialized)
        return SPP_ERROR;
    if (p_pkt == NULL)
        return SPP_ERROR_NULL_POINTER;

    if (s_count >= DB_FLOW_READY_SIZE)
    {
        return SPP_ERROR; /* FIFO full */
    }

    s_ready[s_tail] = p_pkt;
    s_tail = (s_tail + 1u) % DB_FLOW_READY_SIZE;
    s_count++;
    return SPP_OK;
}

/**
 * @brief Pop the next packet from the ready FIFO.
 *
 * @param[out] pp_pkt Receives the dequeued packet pointer.
 * @return SPP_OK on success, SPP_ERROR if FIFO is empty.
 */
retval_t DB_FLOW_PopReady(spp_packet_t **pp_pkt)
{
    if (!s_initialized)
        return SPP_ERROR;
    if (pp_pkt == NULL)
        return SPP_ERROR_NULL_POINTER;

    if (s_count == 0u)
    {
        *pp_pkt = NULL;
        return SPP_ERROR; /* FIFO empty */
    }

    *pp_pkt = s_ready[s_head];
    s_ready[s_head] = NULL;
    s_head = (s_head + 1u) % DB_FLOW_READY_SIZE;
    s_count--;
    return SPP_OK;
}

/**
 * @brief Return the current ready-queue depth.
 *
 * @return Number of packets in the FIFO.
 */
uint32_t DB_FLOW_ReadyCount(void)
{
    return s_count;
}
