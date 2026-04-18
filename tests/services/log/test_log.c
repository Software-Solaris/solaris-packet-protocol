/**
 * @file test_log.c
 * @brief BDD unit tests for the SPP logging service.
 *
 * Coverage targets (100%):
 *  - SPP_SERVICES_LOG_init()             — initialises, sets defaults
 *  - SPP_SERVICES_LOG_setLevel()         — sets level
 *  - SPP_SERVICES_LOG_getLevel()         — returns current level
 *  - SPP_SERVICES_LOG_registerOutput()   — NULL callback silences, custom callback fires
 *  - SPP_SERVICES_LOG_emit()             — filtered by level, calls callback with correct args
 */

#include <cgreen/cgreen.h>
#include "spp/services/log/log.h"
#include "spp/core/returnTypes.h"

/* ----------------------------------------------------------------
 * Test callback infrastructure
 * ---------------------------------------------------------------- */

static int    s_callCount   = 0;
static SPP_LogLevel_t s_lastLevel = K_SPP_LOG_NONE;
static char   s_lastTag[64]  = {0};
static char   s_lastMsg[256] = {0};

static void captureOutput(const char *p_tag, SPP_LogLevel_t level,
                           const char *p_message)
{
    s_callCount++;
    s_lastLevel = level;
    __builtin_strncpy(s_lastTag,  p_tag,    sizeof(s_lastTag)  - 1U);
    __builtin_strncpy(s_lastMsg,  p_message, sizeof(s_lastMsg) - 1U);
}

static void resetCapture(void)
{
    s_callCount = 0;
    s_lastLevel = K_SPP_LOG_NONE;
    s_lastTag[0] = '\0';
    s_lastMsg[0] = '\0';
}

/* ----------------------------------------------------------------
 * Describe: SPP_SERVICES_LOG_init
 * ---------------------------------------------------------------- */

Describe(SPP_SERVICES_LOG_init);
BeforeEach(SPP_SERVICES_LOG_init) { resetCapture(); }
AfterEach(SPP_SERVICES_LOG_init)  {}

Ensure(SPP_SERVICES_LOG_init, returns_ok)
{
    assert_that(SPP_SERVICES_LOG_init(), is_equal_to(K_SPP_OK));
}

Ensure(SPP_SERVICES_LOG_init, sets_verbose_level_by_default)
{
    SPP_SERVICES_LOG_init();
    assert_that(SPP_SERVICES_LOG_getLevel(), is_equal_to(K_SPP_LOG_VERBOSE));
}

/* ----------------------------------------------------------------
 * Describe: SPP_SERVICES_LOG_setLevel / getLevel
 * ---------------------------------------------------------------- */

Describe(SPP_SERVICES_LOG_setLevel);
BeforeEach(SPP_SERVICES_LOG_setLevel) { SPP_SERVICES_LOG_init(); }
AfterEach(SPP_SERVICES_LOG_setLevel)  {}

Ensure(SPP_SERVICES_LOG_setLevel, get_returns_set_value)
{
    SPP_SERVICES_LOG_setLevel(K_SPP_LOG_ERROR);
    assert_that(SPP_SERVICES_LOG_getLevel(), is_equal_to(K_SPP_LOG_ERROR));
}

Ensure(SPP_SERVICES_LOG_setLevel, can_set_to_none)
{
    SPP_SERVICES_LOG_setLevel(K_SPP_LOG_NONE);
    assert_that(SPP_SERVICES_LOG_getLevel(), is_equal_to(K_SPP_LOG_NONE));
}

/* ----------------------------------------------------------------
 * Describe: SPP_SERVICES_LOG_emit
 * ---------------------------------------------------------------- */

Describe(SPP_SERVICES_LOG_emit);
BeforeEach(SPP_SERVICES_LOG_emit)
{
    SPP_SERVICES_LOG_init();
    SPP_SERVICES_LOG_registerOutput(captureOutput);
    SPP_SERVICES_LOG_setLevel(K_SPP_LOG_VERBOSE);
    resetCapture();
}
AfterEach(SPP_SERVICES_LOG_emit) {}

Ensure(SPP_SERVICES_LOG_emit, calls_output_callback)
{
    SPP_LOGI("TEST", "hello");
    assert_that(s_callCount, is_equal_to(1));
}

Ensure(SPP_SERVICES_LOG_emit, passes_correct_tag)
{
    SPP_LOGI("MY_TAG", "msg");
    assert_that(s_lastTag, is_equal_to_string("MY_TAG"));
}

Ensure(SPP_SERVICES_LOG_emit, passes_correct_level)
{
    SPP_LOGE("T", "err");
    assert_that(s_lastLevel, is_equal_to(K_SPP_LOG_ERROR));
}

Ensure(SPP_SERVICES_LOG_emit, formats_message)
{
    SPP_LOGI("T", "val=%d", 42);
    assert_that(s_lastMsg, is_equal_to_string("val=42"));
}

Ensure(SPP_SERVICES_LOG_emit, suppressed_when_level_below_filter)
{
    SPP_SERVICES_LOG_setLevel(K_SPP_LOG_ERROR);
    SPP_LOGI("T", "should be suppressed");
    assert_that(s_callCount, is_equal_to(0));
}

Ensure(SPP_SERVICES_LOG_emit, not_suppressed_at_exact_filter_level)
{
    SPP_SERVICES_LOG_setLevel(K_SPP_LOG_INFO);
    SPP_LOGI("T", "at boundary");
    assert_that(s_callCount, is_equal_to(1));
}

Ensure(SPP_SERVICES_LOG_emit, silent_when_callback_is_null)
{
    SPP_SERVICES_LOG_registerOutput(NULL);
    SPP_LOGI("T", "silent");
    assert_that(s_callCount, is_equal_to(0)); /* captureOutput not called. */
}

Ensure(SPP_SERVICES_LOG_emit, all_macros_reach_callback)
{
    SPP_LOGE("T", "e");
    SPP_LOGW("T", "w");
    SPP_LOGI("T", "i");
    SPP_LOGD("T", "d");
    SPP_LOGV("T", "v");
    assert_that(s_callCount, is_equal_to(5));
}

/* ----------------------------------------------------------------
 * Test suite factory
 * ---------------------------------------------------------------- */

TestSuite *log_suite(void)
{
    TestSuite *suite = create_named_test_suite("log");

    add_test_with_context(suite, SPP_SERVICES_LOG_init, returns_ok);
    add_test_with_context(suite, SPP_SERVICES_LOG_init, sets_verbose_level_by_default);

    add_test_with_context(suite, SPP_SERVICES_LOG_setLevel, get_returns_set_value);
    add_test_with_context(suite, SPP_SERVICES_LOG_setLevel, can_set_to_none);

    add_test_with_context(suite, SPP_SERVICES_LOG_emit, calls_output_callback);
    add_test_with_context(suite, SPP_SERVICES_LOG_emit, passes_correct_tag);
    add_test_with_context(suite, SPP_SERVICES_LOG_emit, passes_correct_level);
    add_test_with_context(suite, SPP_SERVICES_LOG_emit, formats_message);
    add_test_with_context(suite, SPP_SERVICES_LOG_emit, suppressed_when_level_below_filter);
    add_test_with_context(suite, SPP_SERVICES_LOG_emit, not_suppressed_at_exact_filter_level);
    add_test_with_context(suite, SPP_SERVICES_LOG_emit, silent_when_callback_is_null);
    add_test_with_context(suite, SPP_SERVICES_LOG_emit, all_macros_reach_callback);

    return suite;
}
