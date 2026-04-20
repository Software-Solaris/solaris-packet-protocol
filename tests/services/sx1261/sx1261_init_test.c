/**
 * @file test_core.c
 * @brief BDD unit tests for SPP core initialisation and port registration.
 *
 * Coverage targets:
 *  - SPP_CORE_setHalPort()  — NULL guard, happy path
 *  - SPP_CORE_init()        — missing port, port set
 *  - SPP_CORE_getHalPort()  — returns registered port
 */

#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>
#include "spp/core/core.h"
#include "spp/hal/spi.h"
#include "spp/core/returnTypes.h"
#include "spp/services/sx1261/sx1261.h"
#include "spp/services/log/log.h"

extern const SPP_HalPort_t g_stubHalPort;

/* ----------------------------------------------------------------
 * Describe: SPP_CORE_setHalPort
 * ---------------------------------------------------------------- */

Describe(SX1261_init);
BeforeEach(SX1261_init)
{
}
AfterEach(SX1261_init)
{
}

Ensure(SX1261_init, sends_an_spi_message_and_return_okay_if_everything_went_fine)
{
    spp_uint8_t fakeTransmitData[2] = {K_SPP_SX1261_READ_OPCODE, SPP_SX1261_STDBY_XOSC};
    expect(SPP_HAL_spiTransmit, when(p_handle, is_not_null),
           when(p_data, is_equal_to_contents_of(fakeTransmitData, sizeof(fakeTransmitData))),
           will_return(K_SPP_OK));
    assert_that(SPP_SERVICES_SX1261_init(), is_equal_to(K_SPP_OK));
}

Ensure(SX1261_init, when_spi_transmit_fails_prints_the_error)
{
    expect(SPP_HAL_spiTransmit, will_return(K_SPP_ERROR));
    expect(SPP_LOGE);
    assert_that(SPP_SERVICES_SX1261_init(), is_equal_to(K_SPP_ERROR));
}


Ensure(SX1261_init, returns_ok_when_everything_goes_fine)
{
    expect(SPP_HAL_spiTransmit, will_return(K_SPP_OK));
    assert_that(SPP_SERVICES_SX1261_init(), is_equal_to(K_SPP_OK));
}


/* ----------------------------------------------------------------
 * Test suite factory
 * ---------------------------------------------------------------- */

TestSuite *SX1261_init(void)
{
    TestSuite *suite = create_named_test_suite("core");

    add_test_with_context(suite, SX1261_init,
                          sends_an_spi_message_and_return_okay_if_everything_went_fine);
    add_test_with_context(suite, SX1261_init, when_spi_transmit_fails_prints_the_error);
    add_test_with_context(suite, SX1261_init, returns_ok_when_everything_goes_fine);

    return suite;
}
