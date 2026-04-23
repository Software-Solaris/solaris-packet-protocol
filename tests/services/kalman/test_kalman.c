/**
 * @file test_kalman.c
 * @brief BDD unit tests for SPP kalman filter implementation.
 *
 * Coverage targets:
 *  - SPP_SERVICES_KALMAN_ekfInit()
 *  - SPP_SERVICES_KALMAN_ekfPredict()
 *  - SPP_SERVICES_KALMAN_ekfUpdate()
 */


#include <cgreen/cgreen.h>
#include "spp/services/kalman/kalman.h"
#include "spp/core/returnTypes.h"


/* ----------------------------------------------------------------
 * Describe: SPP_SERVICES_KALMAN_ekfINIT
 * ---------------------------------------------------------------- */


Describe(kalmanInit);
BeforeEach(kalmanInit)
{
}
AfterEach(kalmanInit)
{
}

Ensure(kalmanInit, returns_ok_when_everything_goes_ok)
{
    assert_that(SPP_SERVICES_KALMAN_vladikTest(), is_equal_to(K_SPP_OK));
}


/* ----------------------------------------------------------------
 * Describe: SPP_SERVICES_KALMAN_ekfPredict
 * ---------------------------------------------------------------- */


/* ----------------------------------------------------------------
 * Describe: SPP_SERVICES_KALMAN_ekfUpdate
 * ---------------------------------------------------------------- */


/* ----------------------------------------------------------------
 * Test suite factory
 * ---------------------------------------------------------------- */


TestSuite *kalmanInit(void)
{
    TestSuite *suite = create_named_test_suite("kalman init tests");

    add_test_with_context(suite, kalmanInit, returns_ok_when_everything_goes_ok);

    return suite;
}
