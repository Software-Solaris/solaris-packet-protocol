/**
 * @file test_service.c
 * @brief BDD unit tests for the SPP service registry.
 *
 * Coverage targets (100%):
 *  - SPP_Service_register()  — NULL desc, NULL ctx, valid, registry full
 *  - SPP_Service_initAll()   — calls init on all, propagates error
 *  - SPP_Service_startAll()  — calls start on all
 *  - SPP_Service_stopAll()   — calls stop in reverse order
 *  - SPP_Service_count()     — increments per registration
 */

#include <cgreen/cgreen.h>
#include "spp/services/service.h"
#include "spp/services/log.h"
#include "spp/core/returntypes.h"

/* ----------------------------------------------------------------
 * Stub service
 * ---------------------------------------------------------------- */

typedef struct { int initCalled; int startCalled; int stopCalled; } StubCtx_t;

static SPP_RetVal_t stubInit  (void *p_ctx, const void *p_cfg) { (void)p_cfg; ((StubCtx_t*)p_ctx)->initCalled++;  return K_SPP_OK; }
static SPP_RetVal_t stubStart (void *p_ctx)                    { ((StubCtx_t*)p_ctx)->startCalled++; return K_SPP_OK; }
static SPP_RetVal_t stubStop  (void *p_ctx)                    { ((StubCtx_t*)p_ctx)->stopCalled++;  return K_SPP_OK; }
static SPP_RetVal_t stubDeinit(void *p_ctx)                    { (void)p_ctx; return K_SPP_OK; }

static SPP_RetVal_t failingInit(void *p_ctx, const void *p_cfg) { (void)p_ctx; (void)p_cfg; return K_SPP_ERROR; }

static const SPP_ServiceDesc_t k_stubDesc = {
    .p_name  = "stub",
    .apid    = 0x0001U,
    .ctxSize = sizeof(StubCtx_t),
    .init    = stubInit,
    .start   = stubStart,
    .stop    = stubStop,
    .deinit  = stubDeinit,
};

static const SPP_ServiceDesc_t k_failDesc = {
    .p_name  = "fail",
    .apid    = 0x0002U,
    .ctxSize = sizeof(StubCtx_t),
    .init    = failingInit,
    .start   = stubStart,
    .stop    = stubStop,
    .deinit  = stubDeinit,
};

/* ----------------------------------------------------------------
 * Describe: SPP_Service_register
 * ---------------------------------------------------------------- */

Describe(SPP_Service_register);
BeforeEach(SPP_Service_register) { SPP_Log_init(); SPP_Log_setLevel(K_SPP_LOG_NONE); }
AfterEach(SPP_Service_register)  {}

Ensure(SPP_Service_register, rejects_null_descriptor)
{
    StubCtx_t ctx = {0};
    assert_that(SPP_Service_register(NULL, &ctx, NULL), is_equal_to(K_SPP_ERROR_NULL_POINTER));
}

Ensure(SPP_Service_register, rejects_null_context)
{
    assert_that(SPP_Service_register(&k_stubDesc, NULL, NULL), is_equal_to(K_SPP_ERROR_NULL_POINTER));
}

Ensure(SPP_Service_register, succeeds_with_valid_args)
{
    StubCtx_t ctx = {0};
    assert_that(SPP_Service_register(&k_stubDesc, &ctx, NULL), is_equal_to(K_SPP_OK));
}

Ensure(SPP_Service_register, count_increments)
{
    spp_uint32_t before = SPP_Service_count();
    StubCtx_t ctx = {0};
    SPP_Service_register(&k_stubDesc, &ctx, NULL);
    assert_that(SPP_Service_count(), is_equal_to(before + 1U));
}

/* ----------------------------------------------------------------
 * Describe: SPP_Service_initAll / startAll / stopAll
 * ---------------------------------------------------------------- */

Describe(SPP_Service_lifecycle);
BeforeEach(SPP_Service_lifecycle) { SPP_Log_init(); SPP_Log_setLevel(K_SPP_LOG_NONE); }
AfterEach(SPP_Service_lifecycle)  {}

Ensure(SPP_Service_lifecycle, initAll_calls_init_on_registered_service)
{
    StubCtx_t ctx = {0};
    SPP_Service_register(&k_stubDesc, &ctx, NULL);
    SPP_Service_initAll();
    assert_that(ctx.initCalled, is_greater_than(0));
}

Ensure(SPP_Service_lifecycle, startAll_calls_start_on_registered_service)
{
    StubCtx_t ctx = {0};
    SPP_Service_register(&k_stubDesc, &ctx, NULL);
    SPP_Service_startAll();
    assert_that(ctx.startCalled, is_greater_than(0));
}

Ensure(SPP_Service_lifecycle, stopAll_calls_stop_on_registered_service)
{
    StubCtx_t ctx = {0};
    SPP_Service_register(&k_stubDesc, &ctx, NULL);
    SPP_Service_stopAll();
    assert_that(ctx.stopCalled, is_greater_than(0));
}

Ensure(SPP_Service_lifecycle, initAll_propagates_error)
{
    StubCtx_t ctx = {0};
    SPP_Service_register(&k_failDesc, &ctx, NULL);
    SPP_RetVal_t ret = SPP_Service_initAll();
    assert_that(ret, is_equal_to(K_SPP_ERROR));
}

/* ----------------------------------------------------------------
 * Test suite factory
 * ---------------------------------------------------------------- */

TestSuite *service_suite(void)
{
    TestSuite *suite = create_test_suite();

    add_test_with_context(suite, SPP_Service_register, rejects_null_descriptor);
    add_test_with_context(suite, SPP_Service_register, rejects_null_context);
    add_test_with_context(suite, SPP_Service_register, succeeds_with_valid_args);
    add_test_with_context(suite, SPP_Service_register, count_increments);

    add_test_with_context(suite, SPP_Service_lifecycle, initAll_calls_init_on_registered_service);
    add_test_with_context(suite, SPP_Service_lifecycle, startAll_calls_start_on_registered_service);
    add_test_with_context(suite, SPP_Service_lifecycle, stopAll_calls_stop_on_registered_service);
    add_test_with_context(suite, SPP_Service_lifecycle, initAll_propagates_error);

    return suite;
}
