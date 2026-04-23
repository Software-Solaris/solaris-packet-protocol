/**
 * @file main_test.c
 * @brief Cgreen test runner — aggregates all SPP unit test suites.
 */

#include <cgreen/cgreen.h>

/* Suite factories declared in test_*.c files. */
TestSuite *core_suite(void);
TestSuite *SPP_SERVICES_SX1261_init(void);
TestSuite *kalmanInit(void);
TestSuite *serviceRegister(void);

int main(int argc, char **argv)
{
    TestSuite *suite = create_test_suite();

    // add_suite(suite, core_suite());
    // add_suite(suite, SPP_SERVICES_SX1261_init());
    // add_suite(suite, serviceRegister());
    add_suite(suite, kalmanInit());

    if (argc > 1)
    {
        return run_single_test(suite, argv[1], create_text_reporter());
    }
    return run_test_suite(suite, create_text_reporter());
}
