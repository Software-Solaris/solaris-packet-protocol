/**
 * @file test_sx1262.c
 * @brief Placeholder Cgreen test for the SX1262 driver.
 */

#include <cgreen/assertions.h>
#include <cgreen/cgreen.h>
#include <cgreen/constraint_syntax_helpers.h>
#include "spp/services/service.h"
#include "spp/services/sx1262/sx1262.h"

Describe(SX1262);
BeforeEach(SX1262)
{
}
AfterEach(SX1262)
{
}

Ensure(SX1262, returns_the_consumer_constract_correctly)
{
    SPP_SERVICE_ConsumerContract_t sx1262Contract = SPP_SERVICES_SX1262_getConsumerContract();
    assert_that(sx1262Contract.init, is_non_null);
    assert_that(sx1262Contract.p_nameConsumer, is_equal_to_string("sx1262"));
}
