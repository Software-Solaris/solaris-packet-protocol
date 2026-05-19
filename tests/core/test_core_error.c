/**
 * @file test_core_error.c
 * @brief BDD unit tests for SPP CORE Error functions.
 *
 */

#include <cgreen/cgreen.h>
#include "spp/core/error.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * Describe: SPP_CORE_errGet
 * ---------------------------------------------------------------- */

Describe(SPP_CORE_errGet);
BeforeEach(SPP_CORE_errGet)
{
    SPP_CORE_errSet(K_SPP_OK);
}
AfterEach(SPP_CORE_errGet)
{
}

Ensure(SPP_CORE_errGet, returns_ok_after_init)
{
    assert_that(SPP_CORE_errGet(), is_equal_to(K_SPP_OK));
}

Ensure(SPP_CORE_errGet, returns_last_error_with_errSet)
{
    SPP_CORE_errSet(K_SPP_ERROR);
    assert_that(SPP_CORE_errGet(), is_equal_to(K_SPP_ERROR));
}

Ensure(SPP_CORE_errGet, returns_last_erro_with_errSetCtx)
{
    SPP_CORE_errSetCtx(K_SPP_ERROR, 1, 2);
    assert_that(SPP_CORE_errGet(), is_equal_to(K_SPP_ERROR));
}
