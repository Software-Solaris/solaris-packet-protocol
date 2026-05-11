#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>
#include "spp/services/sx1262/sx1262.h"
#include "spp/hal/spi.h"
#include "spp/hal/gpio.h"
#include "spp/hal/time.h"


// Mocks para HAL
SPP_RetVal_t SPP_HAL_gpioConfigInterrupt(spp_uint32_t pin, spp_uint32_t intrType, spp_uint32_t pull)
{
    return (SPP_RetVal_t)mock(pin, intrType, pull);
}

SPP_RetVal_t SPP_HAL_gpioRegisterIsr(spp_uint32_t pin, void *isr_ctx)
{
    return (SPP_RetVal_t)mock(pin, isr_ctx);
}

// Test
Describe(SX1262);

BeforeEach(SX1262)
{
}
AfterEach(SX1262)
{
}

Ensure(SX1262, init_configures_interrupt_and_registers_isr)
{
    SX1262_Data_t sx_data;
    sx_data.intPin = 5;
    sx_data.intIntrType = 1;
    sx_data.intPull = 2;

    expect(SPP_HAL_gpioConfigInterrupt, when(pin, is_equal_to(5)), when(intrType, is_equal_to(1)),
           when(pull, is_equal_to(2)), will_return(K_SPP_OK));
    expect(SPP_HAL_gpioRegisterIsr, when(pin, is_equal_to(5)), will_return(K_SPP_OK));

    SPP_SERVICES_SX1262_init(&sx_data);

    assert_that(sx_data.drdyFlag, is_equal_to(false));
    assert_that(sx_data.isr_ctx.p_flag, is_equal_to(&sx_data.drdyFlag));
}

int main(int argc, char **argv)
{
    TestSuite *test_s = create_test_suite();
    add_test_with_context(test_s, SX1262, init_configures_interrupt_and_registers_isr);
    return run_test_suite(test_s, create_text_reporter());
}
