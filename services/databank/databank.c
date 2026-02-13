#include "databank.h"
#include "macros.h"
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Static variables for data pool
static spp_packet_t data_bank[DATA_BANK_SIZE];
static SPP_Databank_t data_pool;
static spp_packet_t* initial_state[DATA_BANK_SIZE];
static bool is_initialized = false;

char *TAG = "DATABANK";

retval_t SPP_DATABANK_init(void){
    if (is_initialized) {
        return SPP_OK; // Already initialized
    }
    
    SPP_LOGI(TAG, "Initializing Data Bank\n");
    
    // Clear data bank
    memset(data_bank, 0, sizeof(data_bank));
    
    // Initialize array of pointers
    for (int i = 0; i < DATA_BANK_SIZE; i++){
        data_pool.free_packets[i] = &data_bank[i];
        initial_state[i] = &data_bank[i];
    }

    data_pool.number_of_free_packets = DATA_BANK_SIZE;
    is_initialized = true;    
    SPP_LOGI(TAG, "Data Bank initialized with %d packets\n", DATA_BANK_SIZE);
    return SPP_OK;
}

spp_packet_t* SPP_DATABANK_getPacket(void){
    if (!is_initialized) {
        SPP_LOGE(TAG, "Error: Data Bank not initialized\n");
        return NULL;
    }
    
    if(data_pool.number_of_free_packets == 0){
        SPP_LOGE(TAG, "Error: No free packets available\n");
        return NULL;
    }
    
    // Get the last available packet
    data_pool.number_of_free_packets--;

    //We get the last packet in the array
    spp_packet_t* p_packet = data_pool.free_packets[data_pool.number_of_free_packets];

    // We put that element to NULL to know that place is empty
    data_pool.free_packets[data_pool.number_of_free_packets] = NULL;
    
    return p_packet;
}

retval_t SPP_DATABANK_returnPacket(spp_packet_t* p_packet){
    if (!is_initialized) {
        SPP_LOGE(TAG, "Error: Data Bank not initialized\n");
        return SPP_ERROR;
    }
    
    if(p_packet == NULL){
        SPP_LOGE(TAG, "Error: Packet is NULL\n");
        return SPP_NULL_PACKET;
    }
    
    if(data_pool.number_of_free_packets >= DATA_BANK_SIZE){
        SPP_LOGE(TAG, "Error: Data pool is full\n");
        return SPP_ERROR;
    }

    // We need to verify if that pointer has been already returned
    for (int i = 0; i < DATA_BANK_SIZE; i++){
        if(data_pool.free_packets[i] == p_packet){
            return SPP_ERROR;
        }
    }

    // We also need to verify they are reurning an address that was created
    spp_bool_t found_coincidence = false;
    for (int i = 0; i < DATA_BANK_SIZE; i++){
        if (p_packet == initial_state[i]){
            found_coincidence = true;
            break;
        }
    }
    if (found_coincidence == false){
        //You cannot return an address that was never created
        return SPP_ERROR;
    }
    
    // Clear the packet
    memset(p_packet, 0, sizeof(spp_packet_t));

    // Return the packet to the last index position
    data_pool.free_packets[data_pool.number_of_free_packets] = p_packet;
    
    // Update the counter
    data_pool.number_of_free_packets++;    
    
    return SPP_OK;
}