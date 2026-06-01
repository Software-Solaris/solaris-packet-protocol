/**
 * @file test_core_error.c
 * @brief BDD unit tests for SPP CORE Error functions.
 *
 */

#include <cgreen/cgreen.h>
#include "spp/core/error.h"
#include "spp/core/returnTypes.h"

#define FILE_EXAMPLE "Example.c"
#define LINE_EXAMPLE 67

SPP_RetVal_t unknown_error = (SPP_RetVal_t)9999;


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
    SPP_CORE_errSetCtx(K_SPP_ERROR, FILE_EXAMPLE, LINE_EXAMPLE);
    assert_that(SPP_CORE_errGet(), is_equal_to(K_SPP_ERROR));
}

/* ----------------------------------------------------------------
 * Describe: SPP_CORE_errToStringR
 * ---------------------------------------------------------------- */
Describe(SPP_CORE_errToStringR);
BeforeEach(SPP_CORE_errToStringR)
{
}
AfterEach(SPP_CORE_errToStringR)
{
}

Ensure(SPP_CORE_errToStringR, returns_ok_with_no_errors)
{
    assert_that(SPP_CORE_errToStringR(K_SPP_OK, NULL, 0), is_equal_to_string("OK"));
}

Ensure(SPP_CORE_errToStringR, returns_generic_error_with_error)
{
    assert_that(SPP_CORE_errToStringR(K_SPP_ERROR, NULL, 0), is_equal_to_string("generic error"));
}

Ensure(SPP_CORE_errToStringR, returns_unknown_error_with_no_SPP_type_error)
{
    assert_that(SPP_CORE_errToStringR(unknown_error, NULL, 0), is_equal_to_string("unknown error"));
}

Ensure(SPP_CORE_errToStringR, writes_buffer)
{
    char buf[25];
    assert_that(SPP_CORE_errToStringR(K_SPP_OK, buf, sizeof(buf)), is_equal_to(buf));
    assert_that(buf, is_equal_to_string("OK"));
}

Ensure(SPP_CORE_errToStringR, length_zero_does_not_modify_buffer)
{
    char buf[4] = {'A', 'B', 'C', '\0'};
    SPP_CORE_errToStringR(K_SPP_OK, buf, 0);

    assert_that(buf, is_equal_to_string("ABC"));
}

Ensure(SPP_CORE_errToStringR, unitary_length_buf_returns_empty_str)
{
    char buf[1] = {'A'};
    SPP_CORE_errToStringR(K_SPP_OK, buf, sizeof(buf));
    assert_that(buf[0], is_equal_to('\0'));
}
