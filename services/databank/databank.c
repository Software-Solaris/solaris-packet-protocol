/**
 * @file databank.c
 * @brief SPP Databank service implementation.
 *
 * Manages a static pool of spp_packet_t packets backed by a fixed-size
 * array.  Packets are handed out stack-style (last returned is first
 * acquired) and validated on return.
 */

#include "spp/services/databank/databank.h"
#include "spp/core/macros.h"
#include "spp/core/types.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* ------------------------------------------------------------------ */
/*  Static variables                                                   */
/* ------------------------------------------------------------------ */

static spp_packet_t s_dataBank[DATA_BANK_SIZE];
static SPP_Databank_t s_dataPool;
static spp_packet_t *s_initialState[DATA_BANK_SIZE];
static bool s_isInitialized = false;

static const char *TAG = "DATABANK";

/* ------------------------------------------------------------------ */
/*  Public functions                                                    */
/* ------------------------------------------------------------------ */

/**
 * @brief Initialise the databank packet pool.
 *
 * Clears the backing array, populates the free-pointer list and records
 * the original addresses so that returned pointers can be validated.
 *
 * @return SPP_OK on success.
 */
retval_t SPP_DATABANK_init(void)
{
    if (s_isInitialized)
    {
        return SPP_OK; /* Already initialised */
    }

    SPP_LOGI(TAG, "Initializing Data Bank\n");

    /* Clear data bank */
    memset(s_dataBank, 0, sizeof(s_dataBank));

    /* Initialise array of pointers */
    for (int i = 0; i < DATA_BANK_SIZE; i++)
    {
        s_dataPool.freePackets[i] = &s_dataBank[i];
        s_initialState[i] = &s_dataBank[i];
    }

    s_dataPool.numberOfFreePackets = DATA_BANK_SIZE;
    s_isInitialized = true;
    SPP_LOGI(TAG, "Data Bank initialized with %d packets\n", DATA_BANK_SIZE);
    return SPP_OK;
}

/**
 * @brief Acquire a free packet from the pool.
 *
 * Pops the last available pointer from the free list and returns it
 * to the caller.
 *
 * @return Pointer to a free packet, or NULL on error.
 */
spp_packet_t *SPP_DATABANK_getPacket(void)
{
    if (!s_isInitialized)
    {
        SPP_LOGE(TAG, "Error: Data Bank not initialized\n");
        return NULL;
    }

    if (s_dataPool.numberOfFreePackets == 0)
    {
        SPP_LOGE(TAG, "Error: No free packets available\n");
        return NULL;
    }

    /* Get the last available packet */
    s_dataPool.numberOfFreePackets--;

    /* Retrieve the last packet in the array */
    spp_packet_t *p_packet = s_dataPool.freePackets[s_dataPool.numberOfFreePackets];

    /* Mark that slot as empty */
    s_dataPool.freePackets[s_dataPool.numberOfFreePackets] = NULL;

    return p_packet;
}

/**
 * @brief Return a previously acquired packet to the pool.
 *
 * Validates that the pointer belongs to the original pool and has not
 * already been returned, then zeroes the packet and appends it to the
 * free list.
 *
 * @param[in] p_packet Pointer to the packet to return.
 * @return SPP_OK on success, SPP_NULL_PACKET if p_packet is NULL,
 *         SPP_ERROR on any other failure.
 */
retval_t SPP_DATABANK_returnPacket(spp_packet_t *p_packet)
{
    if (!s_isInitialized)
    {
        SPP_LOGE(TAG, "Error: Data Bank not initialized\n");
        return SPP_ERROR;
    }

    if (p_packet == NULL)
    {
        SPP_LOGE(TAG, "Error: Packet is NULL\n");
        return SPP_NULL_PACKET;
    }

    if (s_dataPool.numberOfFreePackets >= DATA_BANK_SIZE)
    {
        SPP_LOGE(TAG, "Error: Data pool is full\n");
        return SPP_ERROR;
    }

    /* Verify that this pointer has not already been returned */
    for (int i = 0; i < DATA_BANK_SIZE; i++)
    {
        if (s_dataPool.freePackets[i] == p_packet)
        {
            return SPP_ERROR;
        }
    }

    /* Verify the address belongs to the original pool */
    spp_bool_t foundCoincidence = false;
    for (int i = 0; i < DATA_BANK_SIZE; i++)
    {
        if (p_packet == s_initialState[i])
        {
            foundCoincidence = true;
            break;
        }
    }
    if (foundCoincidence == false)
    {
        /* Cannot return an address that was never created */
        return SPP_ERROR;
    }

    /* Clear the packet */
    memset(p_packet, 0, sizeof(spp_packet_t));

    /* Return the packet to the last index position */
    s_dataPool.freePackets[s_dataPool.numberOfFreePackets] = p_packet;

    /* Update the counter */
    s_dataPool.numberOfFreePackets++;

    return SPP_OK;
}
