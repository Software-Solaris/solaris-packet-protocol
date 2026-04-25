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

Describe(serviceRegister);
BeforeEach(serviceRegister)
{
}
AfterEach(serviceRegister)
{
}

Ensure(serviceRegister, nothing)
{
}


/* ----------------------------------------------------------------
 * Test suite factory
 * ---------------------------------------------------------------- */

TestSuite *serviceRegister(void)
{
    TestSuite *suite = create_named_test_suite("core");
    add_test_with_context(suite, serviceRegister, nothing);

    return suite;
}
