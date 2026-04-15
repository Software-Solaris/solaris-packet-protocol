/**
 * @file test_log.c
 * @brief BDD unit tests for the SPP logging service.
 *
 * Coverage targets (100%):
 *  - SPP_Log_init()             — initialises, sets defaults
 *  - SPP_Log_setLevel()         — sets level
 *  - SPP_Log_getLevel()         — returns current level
 *  - SPP_Log_registerOutput()   — NULL callback silences, custom callback fires
 *  - SPP_Log_emit()             — filtered by level, calls callback with correct args
 */

#include <cgreen/cgreen.h>
#include "spp/services/log.h"
#include "spp/core/returntypes.h"

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
 * Describe: SPP_Log_init
 * ---------------------------------------------------------------- */

Describe(SPP_Log_init);
BeforeEach(SPP_Log_init) { resetCapture(); }
AfterEach(SPP_Log_init)  {}

Ensure(SPP_Log_init, returns_ok)
{
    assert_that(SPP_Log_init(), is_equal_to(K_SPP_OK));
}

Ensure(SPP_Log_init, sets_verbose_level_by_default)
{
    SPP_Log_init();
    assert_that(SPP_Log_getLevel(), is_equal_to(K_SPP_LOG_VERBOSE));
}

/* ----------------------------------------------------------------
 * Describe: SPP_Log_setLevel / getLevel
 * ---------------------------------------------------------------- */

Describe(SPP_Log_setLevel);
BeforeEach(SPP_Log_setLevel) { SPP_Log_init(); }
AfterEach(SPP_Log_setLevel)  {}

Ensure(SPP_Log_setLevel, get_returns_set_value)
{
    SPP_Log_setLevel(K_SPP_LOG_ERROR);
    assert_that(SPP_Log_getLevel(), is_equal_to(K_SPP_LOG_ERROR));
}

Ensure(SPP_Log_setLevel, can_set_to_none)
{
    SPP_Log_setLevel(K_SPP_LOG_NONE);
    assert_that(SPP_Log_getLevel(), is_equal_to(K_SPP_LOG_NONE));
}

/* ----------------------------------------------------------------
 * Describe: SPP_Log_emit
 * ---------------------------------------------------------------- */

Describe(SPP_Log_emit);
BeforeEach(SPP_Log_emit)
{
    SPP_Log_init();
    SPP_Log_registerOutput(captureOutput);
    SPP_Log_setLevel(K_SPP_LOG_VERBOSE);
    resetCapture();
}
AfterEach(SPP_Log_emit) {}

Ensure(SPP_Log_emit, calls_output_callback)
{
    SPP_LOGI("TEST", "hello");
    assert_that(s_callCount, is_equal_to(1));
}

Ensure(SPP_Log_emit, passes_correct_tag)
{
    SPP_LOGI("MY_TAG", "msg");
    assert_that(s_lastTag, is_equal_to_string("MY_TAG"));
}

Ensure(SPP_Log_emit, passes_correct_level)
{
    SPP_LOGE("T", "err");
    assert_that(s_lastLevel, is_equal_to(K_SPP_LOG_ERROR));
}

Ensure(SPP_Log_emit, formats_message)
{
    SPP_LOGI("T", "val=%d", 42);
    assert_that(s_lastMsg, is_equal_to_string("val=42"));
}

Ensure(SPP_Log_emit, suppressed_when_level_below_filter)
{
    SPP_Log_setLevel(K_SPP_LOG_ERROR);
    SPP_LOGI("T", "should be suppressed");
    assert_that(s_callCount, is_equal_to(0));
}

Ensure(SPP_Log_emit, not_suppressed_at_exact_filter_level)
{
    SPP_Log_setLevel(K_SPP_LOG_INFO);
    SPP_LOGI("T", "at boundary");
    assert_that(s_callCount, is_equal_to(1));
}

Ensure(SPP_Log_emit, silent_when_callback_is_null)
{
    SPP_Log_registerOutput(NULL);
    SPP_LOGI("T", "silent");
    assert_that(s_callCount, is_equal_to(0)); /* captureOutput not called. */
}

Ensure(SPP_Log_emit, all_macros_reach_callback)
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
    TestSuite *suite = create_test_suite();

    add_test_with_context(suite, SPP_Log_init, returns_ok);
    add_test_with_context(suite, SPP_Log_init, sets_verbose_level_by_default);

    add_test_with_context(suite, SPP_Log_setLevel, get_returns_set_value);
    add_test_with_context(suite, SPP_Log_setLevel, can_set_to_none);

    add_test_with_context(suite, SPP_Log_emit, calls_output_callback);
    add_test_with_context(suite, SPP_Log_emit, passes_correct_tag);
    add_test_with_context(suite, SPP_Log_emit, passes_correct_level);
    add_test_with_context(suite, SPP_Log_emit, formats_message);
    add_test_with_context(suite, SPP_Log_emit, suppressed_when_level_below_filter);
    add_test_with_context(suite, SPP_Log_emit, not_suppressed_at_exact_filter_level);
    add_test_with_context(suite, SPP_Log_emit, silent_when_callback_is_null);
    add_test_with_context(suite, SPP_Log_emit, all_macros_reach_callback);

    return suite;
}
