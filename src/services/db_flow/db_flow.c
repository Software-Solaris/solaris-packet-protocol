/**
 * @file db_flow.c
 * @brief Circular FIFO packet router implementation.
 */

#include "spp/services/db_flow.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

/** @brief Global db_flow FIFO control structure. */
static SPP_DbFlow_t s_dbFlow;

/** @brief Tracks whether the FIFO has been initialised. */
static spp_bool_t s_initialized = false;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

retval_t SPP_DbFlow_init(void)
{
    if (s_initialized)
    {
        return SPP_ERROR_ALREADY_INITIALIZED;
    }

    s_dbFlow.head  = 0U;
    s_dbFlow.tail  = 0U;
    s_dbFlow.count = 0U;

    s_initialized = true;
    return SPP_OK;
}

retval_t SPP_DbFlow_pushReady(SPP_Packet_t *p_pkt)
{
    if (p_pkt == NULL)
    {
        return SPP_ERROR_NULL_POINTER;
    }

    if (s_dbFlow.count >= K_SPP_DBFLOW_READY_SIZE)
    {
        return SPP_ERROR; /* FIFO full. */
    }

    s_dbFlow.p_ready[s_dbFlow.head] = p_pkt;
    s_dbFlow.head = (s_dbFlow.head + 1U) & (K_SPP_DBFLOW_READY_SIZE - 1U);
    s_dbFlow.count++;
    return SPP_OK;
}

retval_t SPP_DbFlow_popReady(SPP_Packet_t **pp_pkt)
{
    if (pp_pkt == NULL)
    {
        return SPP_ERROR_NULL_POINTER;
    }

    if (s_dbFlow.count == 0U)
    {
        *pp_pkt = NULL;
        return SPP_NOT_ENOUGH_PACKETS;
    }

    *pp_pkt = s_dbFlow.p_ready[s_dbFlow.tail];
    s_dbFlow.tail = (s_dbFlow.tail + 1U) & (K_SPP_DBFLOW_READY_SIZE - 1U);
    s_dbFlow.count--;
    return SPP_OK;
}

spp_uint32_t SPP_DbFlow_readyCount(void)
{
    return s_dbFlow.count;
}
