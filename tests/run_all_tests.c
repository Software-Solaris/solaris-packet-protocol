/**
 * @file main_test.c
 * @brief Cgreen test runner — aggregates all SPP unit test suites.
 */

#include <cgreen/cgreen.h>

/* Suite factories declared in test_*.c files. */
TestSuite *core_suite(void);
TestSuite *databank_suite(void);
TestSuite *db_flow_suite(void);
TestSuite *log_suite(void);
TestSuite *crc_suite(void);
TestSuite *service_suite(void);

int main(int argc, char **argv)
{
    TestSuite *suite = create_test_suite();

    add_suite(suite, core_suite());
    add_suite(suite, databank_suite());
    add_suite(suite, db_flow_suite());
    add_suite(suite, log_suite());
    add_suite(suite, crc_suite());
    add_suite(suite, service_suite());

    if (argc > 1)
    {
        return run_single_test(suite, argv[1], create_text_reporter());
    }
    return run_test_suite(suite, create_text_reporter());
}
