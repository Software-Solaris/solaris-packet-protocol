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


/* ----------------------------------------------------------------
 * Describe: SPP_SERVICES_KALMAN_ekfPredict
 * ---------------------------------------------------------------- */


/* ----------------------------------------------------------------
 * Describe: SPP_SERVICES_KALMAN_ekfUpdate
 * ---------------------------------------------------------------- */


Describe(kalmanInit);
BeforeEach(kalmanInit)
{
}
AfterEach(kalmanInit)
{
}

Ensure(kalmanInit, passes_trivial_test)
{
    assert_that(1 + 1, is_equal_to(2));
}


/* ----------------------------------------------------------------
 * Test suite factory
 * ---------------------------------------------------------------- */


TestSuite *kalman_suite()
{
    TestSuite *suite = create_test_suite();

    add_test_with_context(suite, kalmanInit, passes_trivial_test);

    return suite;
}


/*


#include <cgreen/cgreen.h>

// 1. Define tus tests
Ensure(kalman_filter_inicializa_correctamente) {
    // Aquí irá tu lógica de Kalman después
    assert_that(1, is_equal_to(1)); 
}

Ensure(kalman_prediccion_actualiza_estado) {
    assert_that(0, is_equal_to(0));
}

// 2. Crea la función que agrupa estos tests
TestSuite *kalman_tests() {
    TestSuite *suite = create_test_suite();
    add_test(suite, kalman_filter_inicializa_correctamente);
    add_test(suite, kalman_prediccion_actualiza_estado);
    return suite;
}



TestSuite *SX1261_init(void)
{
    TestSuite *suite = create_named_test_suite("core");

    add_test_with_context(suite, SX1261_init,
                          sends_an_spi_message_and_return_okay_if_everything_went_fine);
    add_test_with_context(suite, SX1261_init, when_spi_transmit_fails_prints_the_error);
    add_test_with_context(suite, SX1261_init, returns_ok_when_everything_goes_fine);

    return suite;
}


int main(int argc, char **argv) {
    TestSuite *suite = create_test_suite();
    add_test_with_context(suite, CrashExample, seg_faults_for_null_dereference);
    return run_test_suite(suite, create_text_reporter());
}

*/
