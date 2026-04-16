/**
 * @file service.h
 * @brief SPP service descriptor and registry API.
 *
 * A service is any producer or consumer of @ref SPP_Packet_t data (sensor
 * driver, telemetry downlink, SD logger, antenna control, …).  Each service
 * describes itself with a static @ref SPP_ServiceDesc_t and registers via
 * @ref SPP_Service_register().
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_SPP_*
 * - Types: SPP_ServiceDesc_t
 * - Public functions: SPP_Service_*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_SERVICE_H
#define SPP_SERVICE_H

#include "spp/core/types.h"
#include "spp/core/returntypes.h"
#include "spp/util/macros.h"

/* ----------------------------------------------------------------
 * Service descriptor
 * ---------------------------------------------------------------- */

/**
 * @brief Describes a service — filled in by the service implementer.
 *
 * Declare one @c const instance per service and pass a pointer to
 * @ref SPP_Service_register().
 *
 * @code
 * // In bmp390_service.c:
 * const SPP_ServiceDesc_t g_bmp390ServiceDesc = {
 *     .p_name  = "bmp390",
 *     .apid    = 0x0101U,
 *     .ctxSize = sizeof(BMP390_ServiceCtx_t),
 *     .init    = BMP390_Service_init,
 *     .start   = BMP390_Service_start,
 *     .stop    = BMP390_Service_stop,
 *     .deinit  = BMP390_Service_deinit,
 * };
 * @endcode
 */
typedef struct
{
    const char   *p_name;  /**< Human-readable service name (for logging).  */
    spp_uint16_t  apid;    /**< Application Process ID assigned to this service. */
    size_t        ctxSize; /**< sizeof(service-private context struct).      */

    /**
     * @brief Initialise the service.
     *
     * @param[out] p_ctx  Pointer to the service's opaque context buffer.
     * @param[in]  p_cfg  Pointer to the service configuration struct.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*init)(void *p_ctx, const void *p_cfg);

    /**
     * @brief Start the service (e.g. create its task).
     *
     * @param[in] p_ctx  Service context.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*start)(void *p_ctx);

    /**
     * @brief Stop the service gracefully.
     *
     * @param[in] p_ctx  Service context.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*stop)(void *p_ctx);

    /**
     * @brief Release all resources held by the service.
     *
     * @param[in] p_ctx  Service context.
     *
     * @return K_SPP_OK on success.
     */
    SPP_RetVal_t (*deinit)(void *p_ctx);

} SPP_ServiceDesc_t;

/* ----------------------------------------------------------------
 * Registry API
 * ---------------------------------------------------------------- */

/**
 * @brief Register a service with the SPP service registry.
 *
 * The registry stores the descriptor pointer and the caller-provided context
 * and configuration pointers.  No memory is allocated; all buffers are
 * caller-managed.
 *
 * @param[in] p_desc  Pointer to the static service descriptor.
 * @param[in] p_ctx   Pointer to the caller-allocated context buffer
 *                    (must be at least @c p_desc->ctxSize bytes).
 * @param[in] p_cfg   Pointer to the caller-allocated configuration struct.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR_REGISTRY_FULL if the registry is full.
 */
SPP_RetVal_t SPP_Service_register(const SPP_ServiceDesc_t *p_desc,
                               void *p_ctx, const void *p_cfg);

/**
 * @brief Call @c init on all registered services in registration order.
 *
 * @return K_SPP_OK if all services initialised successfully; the first error
 *         code encountered otherwise (remaining services are still attempted).
 */
SPP_RetVal_t SPP_Service_initAll(void);

/**
 * @brief Call @c start on all registered services in registration order.
 *
 * @return K_SPP_OK if all services started successfully.
 */
SPP_RetVal_t SPP_Service_startAll(void);

/**
 * @brief Call @c stop on all registered services in reverse order.
 *
 * @return K_SPP_OK if all services stopped successfully.
 */
SPP_RetVal_t SPP_Service_stopAll(void);

/**
 * @brief Return the number of currently registered services.
 *
 * @return Service count.
 */
spp_uint32_t SPP_Service_count(void);

#endif /* SPP_SERVICE_H */
