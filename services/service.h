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
 * @ref SPP_SERVICES_pollAll() iterates every registered module and calls its
 * @c serviceTask, replacing the per-sensor DRDY checks in the superloop.
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
 * @brief Describes a module — filled in by the module implementer.
 *
 * Declare one @c const instance per module and pass a pointer to
 * @ref SPP_SERVICES_register().
 *
 * @code
 * // In bmp390.c:
 * const SPP_Module_t g_bmp390Module = {
 *     .p_name       = "bmp390",
 *     .apid         = K_BMP390_SERVICE_APID,
 *     .ctxSize      = sizeof(BMP390_t),
 *     .init         = bmp390Init,
 *     .start        = SPP_SERVICES_BMP390_serviceStart,
 *     .stop         = SPP_SERVICES_BMP390_serviceStop,
 *     .deinit       = SPP_SERVICES_BMP390_serviceDeinit,
 *     .serviceTask  = SPP_SERVICES_BMP390_serviceTask,
 *     .consumesApid = K_SPP_APID_NONE,
 *     .onPacket     = NULL,
 *     .onPacketPrio = 0U,
 * };
 * @endcode
 */
typedef struct
{
    const char   *p_name;  /**< Human-readable module name (for logging). */
    spp_uint16_t  apid;    /**< APID bitmask produced by this module (single bit, or K_SPP_APID_NONE). */
    size_t        ctxSize; /**< sizeof(module-private context struct). */

    /**
     * @brief Initialise the module.
     * @param[out] p_ctx  Module's opaque context buffer.
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*init)(void *p_ctx);

    /**
     * @brief Start the module.
     * @param[in] p_ctx  Module context.
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*start)(void *p_ctx);

    /**
     * @brief Stop the module gracefully.
     * @param[in] p_ctx  Module context.
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*stop)(void *p_ctx);

    /**
     * @brief Release all resources held by the module.
     * @param[in] p_ctx  Module context.
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*deinit)(void *p_ctx);

    /**
     * @brief Per-iteration producer task — called by @ref SPP_SERVICES_pollAll().
     *
     * For sensor modules: check the DRDY flag and, if set, read the sensor
     * and call publish().  Must return quickly when no data is ready.
     *
     * @param[in] p_ctx  Module context.
     */
    void (*serviceTask)(void *p_ctx);

    /**
     * @brief APID bitmask this module subscribes to.
     *
     * Use @ref K_SPP_APID_ALL for all packets, @ref K_SPP_APID_NONE if this
     * module does not consume packets.  Ignored when @c onPacket is NULL.
     */
    spp_uint16_t consumesApid;

    /**
     * @brief Subscription handler (NULL if this module is a producer only).
     *
     * When non-NULL, @ref SPP_SERVICES_register() automatically registers
     * this handler with the pub/sub bus using @c consumesApid and @c onPacketPrio.
     *
     * @param[in] p_packet  Received packet (read-only, valid for call duration).
     * @param[in] p_ctx     Module context pointer.
     */
    SPP_PubSub_Handler_t onPacket;

    /** @brief Dispatch priority for @c onPacket (@ref K_SPP_PUBSUB_PRIO_CRITICAL … @ref K_SPP_PUBSUB_PRIO_LOW). */
    spp_uint8_t onPacketPrio;

} SPP_Module_t;

/* ----------------------------------------------------------------
 * Registry API
 * ---------------------------------------------------------------- */

/**
 * @brief Register a module, initialise it, and wire up its pub/sub subscription.
 *
 * Calls @c p_module->init(p_ctx) immediately.  If @c p_module->onPacket != NULL,
 * also calls @ref SPP_SERVICES_PUBSUB_subscribe() automatically.
 *
 * @param[in] p_module  Pointer to the static module descriptor.
 * @param[in] p_ctx     Pointer to the caller-allocated context buffer
 *                      (at least @c p_module->ctxSize bytes).
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_REGISTRY_FULL if the registry is full.
 */
SPP_RetVal_t SPP_SERVICES_register(const SPP_Module_t *p_module, void *p_ctx);

/**
 * @brief Call @c start on all registered modules in registration order.
 *
 * @return K_SPP_OK if all succeeded.
 */
SPP_RetVal_t SPP_SERVICES_startAll(void);

/**
 * @brief Call @c stop on all registered modules in reverse order.
 *
 * @return K_SPP_OK if all succeeded.
 */
SPP_RetVal_t SPP_SERVICES_stopAll(void);

/**
 * @brief Call @c serviceTask on every registered module that has one.
 *
 * Replaces per-sensor DRDY checks in the superloop.  Each module's serviceTask
 * is responsible for checking its own DRDY flag and returning immediately when
 * no data is ready.
 *
 * @return K_SPP_OK always.
 */
SPP_RetVal_t SPP_SERVICES_pollAll(void);

/**
 * @brief Return the number of currently registered modules.
 *
 * @return Module count.
 */
spp_uint32_t SPP_SERVICES_count(void);

#endif /* SPP_SERVICE_H */
