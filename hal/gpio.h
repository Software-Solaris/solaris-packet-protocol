/**
 * @file gpio.h
 * @brief SPP GPIO HAL API — dispatches through the registered HAL port.
 *
 * Naming conventions used in this file:
 * - Public functions: SPP_HAL_gpio*()
 * - Pointer parameters: p_*
 */

#ifndef SPP_HAL_GPIO_H
#define SPP_HAL_GPIO_H

#include "spp/core/types.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * ISR context
 * ---------------------------------------------------------------- */

/**
 * @brief Context passed to the GPIO ISR handler.
 *
 * The ISR sets @c *p_flag to @c SPP_TRUE when the interrupt fires.
 * The application superloop polls this flag to detect the event.
 */
typedef struct
{
    volatile spp_bool_t *p_flag; /**< Flag set by the ISR on interrupt. */
} SPP_GpioIsrCtx_t;

/* ----------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------- */

/**
 * @brief Configure a GPIO pin as an interrupt input.
 *
 * @param[in] pin       GPIO pin number.
 * @param[in] intrType  Platform-specific interrupt trigger type value.
 * @param[in] pull      Pull resistor: 0 = none, 1 = pull-up, 2 = pull-down.
 *
 * @return K_SPP_OK on success.
 */
SPP_RetVal_t SPP_HAL_gpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t intrType,
                                      spp_uint32_t pull);

/**
 * @brief Register an ISR handler for a GPIO pin.
 *
 * The ISR sets @c *p_isrCtx->p_flag to @c SPP_TRUE whenever the configured
 * interrupt fires.
 *
 * @param[in] pin       GPIO pin number.
 * @param[in] p_isrCtx  Pointer to a @ref SPP_GpioIsrCtx_t for this pin.
 *
 * @return K_SPP_OK on success.
 */
SPP_RetVal_t SPP_HAL_gpioRegisterIsr(spp_uint32_t pin, void *p_isrCtx);

#endif /* SPP_HAL_GPIO_H */
