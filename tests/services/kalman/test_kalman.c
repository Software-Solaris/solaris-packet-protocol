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


/* ----------------------------------------------------------------
 * Describe: SPP_SERVICES_KALMAN_ekfINIT
 * ---------------------------------------------------------------- */


Describe(SPP_SERVICES_KALMAN_ekfInit);
BeforeEach(SPP_SERVICES_KALMAN_ekfInit)
{
}
AfterEach(SPP_SERVICES_KALMAN_ekfInit)
{
}

Ensure()
{
    assert_that();
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


TestSuite *our_tests()
{
    TestSuite *suite = create_test_suite();

    add_test_with_context(suite);

    return suite;
}
