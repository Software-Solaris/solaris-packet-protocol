#include <stdio.h>
#include <inttypes.h>
#include "core/core.h"
#include "services/databank/databank.h"

int main() {
    printf("Initializing Solaris Packet Protocol...\n");
    
    // Initialize SPP core
    retval_t result = Core_Init();
    if (result != SPP_OK) {
        printf("Error initializing SPP: %d\n", result);
        return -1;
    }
    
    printf("SPP initialized successfully.\n");
    
    // Get a packet from databank
    spp_packet_t* packet = DataBank_GetPacket();
    if (packet != NULL) {
        printf("Packet obtained from databank.\n");
        
        // Use the packet...
        packet->packet_id = 42;
        packet->timestamp = 1234567890;
        packet->data = 0xDEADBEEF;
        printf("Packet configured: ID=%d, Timestamp=%" PRIu32 ", Data=0x%" PRIX32 "\n", 
               packet->packet_id, packet->timestamp, packet->data);
        
        // Return the packet to databank
        DataBank_ReturnPacket(packet);
        printf("Packet returned to databank.\n");
    } else {
        printf("Could not obtain a packet from databank.\n");
    }
    
    printf("SPP example completed successfully.\n");
    return 0;
} 