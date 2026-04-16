/**
 * @file databank.c
 * @brief Static packet pool implementation.
 */

#include "spp/services/databank/databank.h"
#include "spp/core/returntypes.h"
#include "spp/core/error.h"

#include <string.h>

/* ----------------------------------------------------------------
 * Private state
 * ---------------------------------------------------------------- */

/** @brief Static storage for all packets. */
static SPP_Packet_t s_packets[K_SPP_DATABANK_SIZE];

/** @brief Control structure (free-list stack). */
static SPP_Databank_t s_databank;

/** @brief Tracks whether the pool has been initialised. */
static spp_bool_t s_initialized = false;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

SPP_RetVal_t SPP_Databank_init(void)
{
    if (s_initialized)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_ALREADY_INITIALIZED);
    }

    memset(s_packets, 0, sizeof(s_packets));
    s_databank.freeCount = 0U;

    for (spp_uint32_t i = 0U; i < K_SPP_DATABANK_SIZE; i++)
    {
        s_databank.p_freePackets[i] = &s_packets[K_SPP_DATABANK_SIZE - 1U - i];
        s_databank.freeCount++;
    }

    s_initialized = true;
    return K_SPP_OK;
}

SPP_Packet_t *SPP_Databank_getPacket(void)
{
    if (!s_initialized || (s_databank.freeCount == 0U))
    {
        return NULL;
    }

    s_databank.freeCount--;
    return s_databank.p_freePackets[s_databank.freeCount];
}

SPP_RetVal_t SPP_Databank_returnPacket(SPP_Packet_t *p_packet)
{
    if (p_packet == NULL)
    {
        SPP_ERR_RETURN(K_SPP_ERROR_NULL_POINTER);
    }

    /* Validate that the pointer belongs to the static pool. */
    if ((p_packet < &s_packets[0]) ||
        (p_packet > &s_packets[K_SPP_DATABANK_SIZE - 1U]))
    {
        SPP_ERR_RETURN(K_SPP_ERROR);
    }

    /* Guard against double-return. */
    for (spp_uint32_t i = 0U; i < s_databank.freeCount; i++)
    {
        if (s_databank.p_freePackets[i] == p_packet)
        {
            SPP_ERR_RETURN(K_SPP_ERROR_ALREADY_INITIALIZED); /* Already in free list. */
        }
    }

    if (s_databank.freeCount >= K_SPP_DATABANK_SIZE)
    {
        SPP_ERR_RETURN(K_SPP_ERROR); /* Pool is already full — should not happen. */
    }

    s_databank.p_freePackets[s_databank.freeCount] = p_packet;
    s_databank.freeCount++;
    return K_SPP_OK;
}

spp_uint32_t SPP_Databank_freeCount(void)
{
    return s_databank.freeCount;
}
