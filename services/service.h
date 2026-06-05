/**
 * @file service.h
 * @brief SPP module descriptor and registry API.
 *
 * A module is any producer or consumer of @ref SPP_Packet_t data (sensor
 * driver, telemetry downlink, SD logger, antenna control, …).  Each module
 * describes itself with a static @ref SPP_Module_t and registers via
 * @ref SPP_SERVICES_register().
 *
 * Registration automatically wires up pub/sub subscriptions: if a module
 * sets @c onPacket != NULL, @ref SPP_SERVICES_register() calls
 * @ref SPP_SERVICES_PUBSUB_subscribe() with @c consumesApid and @c onPacketPrio.
 *
 * @ref SPP_SERVICES_callProducers() iterates every registered module and calls its
 * @c produce, replacing the per-sensor DRDY checks in the superloop.
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_SPP_*
 * - Types: SPP_Module_t
 * - Public functions: SPP_SERVICES_*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_SERVICE_H
#define SPP_SERVICE_H

#include "spp/core/types.h"
#include "spp/core/returnTypes.h"
#include "spp/util/macros.h"
#include "spp/services/pubsub/pubsub.h"

/* ----------------------------------------------------------------
 * Module descriptor
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
    const char *p_nameProducer;         /**< Human-readable module name (for logging). */
    spp_uint16_t tiemoutMs;             /**< Timeout for the producer to send a packet */
    SPP_RetVal_t (*init)(void *p_data); /**< Initialise the module. */
    SPP_RetVal_t (*acquireData)(void);  /**< Acquire data from the module. */
} SPP_SERVICE_ProducerContract_t;
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
