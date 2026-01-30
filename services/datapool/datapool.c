#include "databank.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Static variables for data pool
static spp_packet_t data_bank[DATA_BANK_SIZE];
static data_pool_t data_pool;
static bool is_initialized = false;

retval_t DataBank_Init(void){
    if (is_initialized) {
        return SPP_OK; // Already initialized
    }
    
    printf("Initializing Data Bank\n");
    
    // Clear data bank
    memset(data_bank, 0, sizeof(data_bank));
    
    // Initialize array of pointers
    for (int i = 0; i < DATA_BANK_SIZE; i++){
        data_pool.free_packets[i] = &data_bank[i];
    }

    data_pool.number_of_free_packets = DATA_BANK_SIZE;
    is_initialized = true;
    
    printf("Data Bank initialized with %d packets\n", DATA_BANK_SIZE);
    return SPP_OK;
}

spp_packet_t* DataBank_GetPacket(void){
    if (!is_initialized) {
        printf("Error: Data Bank not initialized\n");
        return NULL;
    }
    
    if(data_pool.number_of_free_packets == 0){
        printf("Error: No free packets available\n");
        return NULL;
    }
    
    // Get the last available packet
    data_pool.number_of_free_packets--;
    spp_packet_t* p_packet = data_pool.free_packets[data_pool.number_of_free_packets];
    data_pool.free_packets[data_pool.number_of_free_packets] = NULL;
    
    return p_packet;
}

retval_t DataBank_ReturnPacket(spp_packet_t* p_packet){
    if (!is_initialized) {
        printf("Error: Data Bank not initialized\n");
        return SPP_ERROR;
    }
    
    if(p_packet == NULL){
        printf("Error: Packet is NULL\n");
        return SPP_NULL_PACKET;
    }
    
    if(data_pool.number_of_free_packets >= DATA_BANK_SIZE){
        printf("Error: Data pool is full\n");
        return SPP_ERROR;
    }
    
    // Clear the packet
    memset(p_packet, 0, sizeof(spp_packet_t));
    
    // Return packet to pool
    data_pool.free_packets[data_pool.number_of_free_packets] = p_packet;
    data_pool.number_of_free_packets++;
    
    return SPP_OK;
}