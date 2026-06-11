/**
 * @file test_core.c
 * @brief BDD unit tests for SPP CORE Boot function.
 *
 */

#include <cgreen/cgreen.h>
#include "spp/core/core.h"
#include "spp/core/returnTypes.h"

extern const SPP_HalPort_t g_stubHalPort;

/* ----------------------------------------------------------------
 * SPP_CORE_boot
 * ---------------------------------------------------------------- */

Describe(SppCoreBoot);
BeforeEach(SppCoreBoot)
{
}
AfterEach(SppCoreBoot)
{
}

Ensure(SppCoreBoot, returns_ok_when_passing_the_hal_port_correctly)
{
    const SPP_HalPort_t testHalPort = {
        .spiBusInit = NULL,
        .spiGetHandle = NULL,
        .spiDeviceInit = NULL,
        .spiTransmit = NULL,
        .gpioConfigInterrupt = NULL,
        .gpioRegisterIsr = NULL,
        .storageMount = NULL,
        .storageUnmount = NULL,
        .getTimeMs = NULL,
        .delayMs = NULL,
    };
    assert_that(SPP_CORE_boot(&testHalPort), is_equal_to(K_SPP_OK));
}