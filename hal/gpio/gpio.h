/**
 * @file gpio.h
 * @brief GPIO Hardware Abstraction Layer
 * @version 1.0.0
 * @date 2024
 *
 * This file provides the GPIO Hardware Abstraction Layer (HAL) interface
 * for interrupt configuration and ISR registration.
 */

#ifndef SPP_HAL_GPIO_H
#define SPP_HAL_GPIO_H

/* ---------------------------------------------------------------- */
/*  Includes                                                        */
/* ---------------------------------------------------------------- */

#include "spp/core/types.h"
#include "spp/core/returntypes.h"
#include "spp/osal/eventgroups.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* ---------------------------------------------------------------- */
    /*  Types                                                           */
    /* ---------------------------------------------------------------- */

    /**
     * @brief  Context consumed by the fixed internal ISR in the port layer.
     */
    typedef struct
    {
        void *p_event_group;   /**< Opaque event-group handle. */
        osal_eventbits_t bits; /**< Bits to set from the ISR.  */
    } spp_gpio_isr_ctx_t;

    /* ---------------------------------------------------------------- */
    /*  Public function declarations                                    */
    /* ---------------------------------------------------------------- */

    /**
     * @brief  Configure a GPIO pin as an interrupt source.
     *
     * @param[in] pin        GPIO pin number.
     * @param[in] intr_type  Platform-agnostic interrupt type
     *                       (cast to gpio_int_type_t inside the port).
     * @param[in] pull       Pull mode: 0 = none, 1 = pull-up, 2 = pull-down.
     *
     * @return retval_t  SPP_OK on success, error code otherwise.
     */
    retval_t SPP_HAL_GPIO_ConfigInterrupt(spp_uint32_t pin, spp_uint32_t intr_type,
                                          spp_uint32_t pull);

    /**
     * @brief  Register the fixed internal ISR for a given pin.
     *
     * @param[in] pin             GPIO pin number.
     * @param[in] p_isr_context   Pointer to a persistent spp_gpio_isr_ctx_t.
     *
     * @return retval_t  SPP_OK on success, error code otherwise.
     */
    retval_t SPP_HAL_GPIO_RegisterISR(spp_uint32_t pin, void *p_isr_context);

#ifdef __cplusplus
}
#endif

#endif /* SPP_HAL_GPIO_H */
