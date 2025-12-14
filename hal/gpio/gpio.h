#include <stdint.h>
#include <stdbool.h>
#include "core/returntypes.h"
#include "types.h"

typedef struct {
    void*            event_group;
    osal_eventbits_t bits;
} spp_gpio_isr_ctx_t;

retval_t SPP_HAL_GPIO_ConfigInterrupt(spp_uint32_t pin, spp_uint32_t intr_type, spp_uint32_t pull);
retval_t SPP_HAL_GPIO_RegisterISR(spp_uint32_t pin, void* isr_context);
