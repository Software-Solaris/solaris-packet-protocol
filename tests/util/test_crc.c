/**
 * @file test_crc.c
 * @brief BDD unit tests for CRC-16 utility.
 */

#include <cgreen/cgreen.h>
#include "spp/util/crc.h"

/* ----------------------------------------------------------------
 * Describe: SPP_UTIL_crc
 * ---------------------------------------------------------------- */

Describe(SPP_UTIL_crc);
BeforeEach(SPP_UTIL_crc)
{
}
AfterEach(SPP_UTIL_crc)
{
}

Ensure(SPP_UTIL_crc, calculates_correct_crc16_for_standard_test_string)
{
    const spp_uint8_t data[] = "123456789";
    /* sizeof("123456789") is 10 (includes null byte), so we explicitly pass 9 */
    const spp_uint32_t length = 9U;
    
    /* Standard CCITT-FALSE CRC-16 for "123456789" is 0x29B1 */
    spp_uint16_t result = SPP_UTIL_crc16(data, length);
    
    assert_that(result, is_equal_to(0x29B1U));
}

Ensure(SPP_UTIL_crc, calculates_correct_crc16_for_single_byte_zero)
{
    const spp_uint8_t data[] = { 0x00U };
    spp_uint16_t result = SPP_UTIL_crc16(data, 1U);
    /* 0xFFFF ^ (0x00 << 8) = 0xFFFF -> after 8 bits shift with poly 0x1021 -> 0xE1F0 */
    assert_that(result, is_equal_to(0xE1F0U));
}
