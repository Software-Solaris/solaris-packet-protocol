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
#include "spp/osal/event.h"

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
 * @brief Register an ISR handler for a GPIO pin using an SPP event group.
 *
 * The ISR will call @ref SPP_OSAL_eventSetFromIsr() with the bits defined
 * in @p p_isrCtx whenever the configured interrupt fires.
 *
 * @param[in] pin       GPIO pin number.
 * @param[in] p_isrCtx  Pointer to a @ref SPP_GpioIsrCtx_t that carries the
 *                      event group handle and bit mask.
 *
 * @return K_SPP_OK on success.
 */
SPP_RetVal_t SPP_HAL_gpioRegisterIsr(spp_uint32_t pin, void *p_isrCtx);

#endif /* SPP_HAL_GPIO_H */
