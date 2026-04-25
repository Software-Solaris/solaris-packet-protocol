/**
 * @file main_test.c
 * @brief Cgreen test runner — aggregates all SPP unit test suites.
 */

#include <cgreen/cgreen.h>

/* Suite factories declared in test_*.c files. */
// TestSuite *core_suite(void);
// TestSuite *SPP_SERVICES_SX1261_init(void);
// TestSuite *serviceRegister(void);
TestSuite *kalman_suite();

int main(int argc, char **argv)
{
    TestSuite *suite = create_test_suite();

    // add_suite(suite, core_suite());
    // add_suite(suite, SPP_SERVICES_SX1261_init());
    // add_suite(suite, serviceRegister());
    add_suite(suite, kalman_suite()); // Addition

    if (argc > 1)
    {
        return run_single_test(suite, argv[1], create_text_reporter());
    }
    return run_test_suite(suite, create_text_reporter());
}


/*

#include <cgreen/cgreen.h>

// Tienes que declarar la función de tu archivo (o incluir su .h si lo tiene)
extern TestSuite *kalman_tests(); 
// extern TestSuite *otro_modulo_tests(); // Esto ya estará ahí seguramente

int main(int argc, char **argv) {
    TestSuite *suite_principal = create_test_suite();
    
    // Aquí es donde se van sumando todos los módulos
    add_suite(suite_principal, kalman_tests()); 
    // add_suite(suite_principal, otro_modulo_tests());

    return run_test_suite(suite_principal, create_text_reporter());
}

*/
