#ifndef SPP_HAL_GPIO_H
#define SPP_HAL_GPIO_H

#include "core/types.h"
#include "core/returntypes.h"
#include "osal/eventgroups.h"   // osal_eventbits_t

#ifdef __cplusplus
extern "C" {
#endif

/* Contexto que consume la ISR interna fija del port */
typedef struct {
    void*            event_group;
    osal_eventbits_t bits;
} spp_gpio_isr_ctx_t;

/*
 * intr_type y pull son enteros “agnósticos”:
 *  - intr_type: se castea dentro del port a gpio_int_type_t (ESP-IDF)
 *  - pull: 0 none, 1 pullup, 2 pulldown
 */
retval_t SPP_HAL_GPIO_ConfigInterrupt(spp_uint32_t pin, spp_uint32_t intr_type, spp_uint32_t pull);

/*
 * Registra la ISR interna fija del port para ese pin.
 * isr_context debe apuntar a un spp_gpio_isr_ctx_t persistente.
 */
retval_t SPP_HAL_GPIO_RegisterISR(spp_uint32_t pin, void* isr_context);

#ifdef __cplusplus
}
#endif

#endif /* SPP_HAL_GPIO_H */
