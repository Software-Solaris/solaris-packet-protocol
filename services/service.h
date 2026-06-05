/**
 * @file service.h
 * @brief SPP module descriptor and registry API.
 */

#ifndef SPP_SERVICE_H
#define SPP_SERVICE_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"
#include "spp/util/macros.h"
#include "spp/services/pubsub/pubsub.h"

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
    spp_uint8_t producerID;
    const char *p_nameProducer;                /**< Human-readable module name (for logging). */
    spp_uint16_t tiemoutMs;                    /**< Timeout for the producer to send a packet */
    SPP_RetVal_t (*init)();                    /**< Initialise the module. */
    SPP_RetVal_t (*acquireData)(void *p_data); /**< Acquire data from the module. */
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
    spp_uint8_t consumerID;
    const char *p_nameConsumer;                /**< Human-readable module name (for logging). */
    spp_uint16_t tiemoutMs;                    /**< Timeout for the consumer to receive a packet */
    SPP_RetVal_t (*init)();                    /**< Initialise the module. */
    SPP_RetVal_t (*consumeData)(void *p_data); /**< Consume data from the module. */
} SPP_SERVICE_ConsumerContract_t;
/* ----------------------------------------------------------------
 * Registry API
 * ---------------------------------------------------------------- */

/**
 * @brief Register a module: runs init(), runs start(), and wires up pub/sub.
 *
 * @param[in] p_module  Pointer to the static module descriptor.
 * @param[in] p_ctx     Pointer to the caller-allocated context buffer.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_REGISTRY_FULL if the registry is full.
 */
SPP_RetVal_t SPP_SERVICES_register(const SPP_Module_t *p_module, void *p_ctx);

/**
 * @brief Call @c produce on every registered module that has one.
 *
 * Replaces per-sensor DRDY checks in the superloop.  Each module's produce
 * is responsible for checking its own DRDY flag and returning immediately when
 * no data is ready.
 *
 * @return K_SPP_OK always.
 */
SPP_RetVal_t SPP_SERVICES_callProducers(void);

/**
 * @brief Dispatch the next pending deferred subscriber.
 *
 * Thin wrapper around @ref SPP_SERVICES_PUBSUB_callConsumers().  Call once per
 * superloop iteration alongside @ref SPP_SERVICES_callProducers().
 */
void SPP_SERVICES_callConsumers(void);

/**
 * @brief Return the number of currently registered modules.
 *
 * @return Module count.
 */
spp_uint32_t SPP_SERVICES_count(void);

#endif /* SPP_SERVICE_H */
