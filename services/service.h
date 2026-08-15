/**
* @file service.h
* @brief SPP module descriptor and registry API.
*/

#ifndef SPP_SERVICE_H
#define SPP_SERVICE_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"
#include "spp/core/packet.h"
#include "spp/services/kpid.h"

/* ----------------------------------------------------------------
 * STRUCTS AND ENUMS
 * ---------------------------------------------------------------- */

/**

* @brief Describes the contract of a producer module. This module needs to have:
*   - a producer ID
*   - a name
*   - a timeout
*   - an init function
*   - an acquireData function
*/
typedef struct
{
    const char *p_nameProducer;    /**< Human-readable module name (for logging). */
    spp_uint16_t tiemoutMs;        /**< Timeout for the producer to send a packet */
    SPP_RetVal_t (*init)();        /**< Initialise the module. */
    SPP_RetVal_t (*acquireData)(); /**< Acquire data from the module. */
} SPP_SERVICE_ProducerContract_t;

/**
* @brief Describes the contract of a consumer module. This module needs to have:
*   - a consumer ID
*   - a name
*   - a timeout (optional)
*   - a consumeData function
*/
typedef struct
{
    spp_int8_t priority;         /**< Priority of the consumer */
    const char *p_nameConsumer;  /**< Human-readable module name (for logging). */
    spp_uint16_t tiemoutMs;      /**< Timeout for the consumer to receive a packet */
    spp_uint16_t suscribeToApid; /**< APID subscription bitmask */
    SPP_RetVal_t (*init)();      /**< Initialise the module. */
    SPP_RetVal_t (*deliverToMailbox)(const SPP_Packet_t packet);
    SPP_RetVal_t (*consumeData)(void *p_data); /**< Consume data from the module. */
} SPP_SERVICE_ConsumerContract_t;


#endif /* SPP_SERVICE_H */
