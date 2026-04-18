/**
 * @file test_crc.c
 * @brief BDD unit tests for CRC-16/CCITT.
 */

#include <cgreen/cgreen.h>
#include "spp/util/crc.h"

Describe(SPP_UTIL_crc16);
BeforeEach(SPP_UTIL_crc16) {}
AfterEach(SPP_UTIL_crc16)  {}

Ensure(SPP_UTIL_crc16, returns_init_value_for_empty_buffer)
{
    /* CRC of zero bytes = initial value 0xFFFF. */
    assert_that(SPP_UTIL_crc16(NULL, 0U), is_equal_to(K_SPP_CRC_INIT));
}

Ensure(SPP_UTIL_crc16, known_vector_0x31_0x32_0x33)
{
    /* CRC-16/CCITT of "123" = 0x3B0C (known test vector). */
    const spp_uint8_t data[] = {0x31U, 0x32U, 0x33U};
    assert_that(SPP_UTIL_crc16(data, 3U), is_equal_to(0x3B0CU));
}

Ensure(SPP_UTIL_crc16, single_byte_zero)
{
    const spp_uint8_t data[] = {0x00U};
    spp_uint16_t crc = SPP_UTIL_crc16(data, 1U);
    assert_that(crc, is_not_equal_to(0U)); /* Non-zero for any non-trivial data. */
}

Ensure(SPP_UTIL_crc16, same_data_gives_same_result)
{
    const spp_uint8_t data[] = {0xABU, 0xCDU, 0xEFU};
    spp_uint16_t crc1 = SPP_UTIL_crc16(data, 3U);
    spp_uint16_t crc2 = SPP_UTIL_crc16(data, 3U);
    assert_that(crc1, is_equal_to(crc2));
}

Ensure(SPP_UTIL_crc16, different_data_gives_different_result)
{
    const spp_uint8_t d1[] = {0x01U, 0x02U};
    const spp_uint8_t d2[] = {0x03U, 0x04U};
    assert_that(SPP_UTIL_crc16(d1, 2U), is_not_equal_to(SPP_UTIL_crc16(d2, 2U)));
}

TestSuite *crc_suite(void)
{
    TestSuite *suite = create_named_test_suite("crc");
    add_test_with_context(suite, SPP_UTIL_crc16, returns_init_value_for_empty_buffer);
    add_test_with_context(suite, SPP_UTIL_crc16, known_vector_0x31_0x32_0x33);
    add_test_with_context(suite, SPP_UTIL_crc16, single_byte_zero);
    add_test_with_context(suite, SPP_UTIL_crc16, same_data_gives_same_result);
    add_test_with_context(suite, SPP_UTIL_crc16, different_data_gives_different_result);
    return suite;
}
