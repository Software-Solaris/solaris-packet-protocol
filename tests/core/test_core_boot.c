/**
 * @file test_core.c
 * @brief BDD unit tests for SPP CORE Boot function.
 *
 */

#include <cgreen/cgreen.h>
#include "spp/core/core.h"
#include "spp/core/returnTypes.h"

SPP_HalPort_t g_stubHalPort = {.spiBusInit = NULL,
                               .spiGetHandle = NULL,
                               .spiDeviceInit = NULL,
                               .spiTransmit = NULL,
                               .gpioConfigInterrupt = NULL,
                               .gpioRegisterIsr = NULL,
                               .storageMount = NULL,
                               .getTimeMs = NULL,
                               .delayMs = NULL};

/* ----------------------------------------------------------------
 * Describe: SPP_CORE_boot
 * ---------------------------------------------------------------- */

Describe(SPP_CORE_boot);
BeforeEach(SPP_CORE_boot)
{
}
AfterEach(SPP_CORE_boot)
{
}

Ensure(SPP_CORE_boot, rejects_null_pointer)
{
    assert_that(SPP_CORE_boot(NULL), is_equal_to(K_SPP_ERROR_NULL_POINTER));
}

Ensure(SPP_CORE_boot, succeeds_with_hal_port_registered)
{
    assert_that(SPP_CORE_boot(&g_stubHalPort), is_equal_to(K_SPP_OK));
}