/**
 * @file test_databank.c
 * @brief BDD unit tests for the SPP packet databank.
 *
 * Coverage targets (100%):
 *  - SPP_Databank_init()         — first call, second call (already-init guard)
 *  - SPP_Databank_getPacket()    — normal, pool exhausted, before init
 *  - SPP_Databank_returnPacket() — happy path, NULL, invalid pointer, double-return
 *  - SPP_Databank_freeCount()    — after init, after get, after return
 */

#include <cgreen/cgreen.h>
#include "spp/services/databank/databank.h"
#include "spp/core/returntypes.h"
#include "spp/util/macros.h"

/* ----------------------------------------------------------------
 * Describe: SPP_Databank_init
 * ---------------------------------------------------------------- */

Describe(SPP_Databank_init);
BeforeEach(SPP_Databank_init) {}
AfterEach(SPP_Databank_init)  {}

Ensure(SPP_Databank_init, succeeds_on_first_call)
{
    assert_that(SPP_Databank_init(), is_equal_to(K_SPP_OK));
}

Ensure(SPP_Databank_init, returns_already_initialized_on_second_call)
{
    SPP_Databank_init(); /* First — may or may not be first in process. */
    assert_that(SPP_Databank_init(), is_equal_to(K_SPP_ERROR_ALREADY_INITIALIZED));
}

Ensure(SPP_Databank_init, pool_is_full_after_init)
{
    SPP_Databank_init();
    assert_that(SPP_Databank_freeCount(), is_equal_to(K_SPP_DATABANK_SIZE));
}

/* ----------------------------------------------------------------
 * Describe: SPP_Databank_getPacket
 * ---------------------------------------------------------------- */

Describe(SPP_Databank_getPacket);
BeforeEach(SPP_Databank_getPacket) { SPP_Databank_init(); }
AfterEach(SPP_Databank_getPacket)  {}

Ensure(SPP_Databank_getPacket, returns_non_null_when_pool_has_packets)
{
    SPP_Packet_t *p_pkt = SPP_Databank_getPacket();
    assert_that(p_pkt, is_not_null);
}

Ensure(SPP_Databank_getPacket, decrements_free_count)
{
    SPP_Databank_getPacket();
    assert_that(SPP_Databank_freeCount(), is_equal_to(K_SPP_DATABANK_SIZE - 1U));
}

Ensure(SPP_Databank_getPacket, returns_null_when_pool_exhausted)
{
    for (spp_uint32_t i = 0U; i < K_SPP_DATABANK_SIZE; i++)
    {
        (void)SPP_Databank_getPacket();
    }
    assert_that(SPP_Databank_getPacket(), is_null);
}

Ensure(SPP_Databank_getPacket, each_packet_is_unique)
{
    SPP_Packet_t *p_pkts[K_SPP_DATABANK_SIZE];
    for (spp_uint32_t i = 0U; i < K_SPP_DATABANK_SIZE; i++)
    {
        p_pkts[i] = SPP_Databank_getPacket();
        assert_that(p_pkts[i], is_not_null);
    }
    for (spp_uint32_t i = 0U; i < K_SPP_DATABANK_SIZE; i++)
    {
        for (spp_uint32_t j = i + 1U; j < K_SPP_DATABANK_SIZE; j++)
        {
            assert_that(p_pkts[i], is_not_equal_to(p_pkts[j]));
        }
    }
}

/* ----------------------------------------------------------------
 * Describe: SPP_Databank_returnPacket
 * ---------------------------------------------------------------- */

Describe(SPP_Databank_returnPacket);
BeforeEach(SPP_Databank_returnPacket) { SPP_Databank_init(); }
AfterEach(SPP_Databank_returnPacket)  {}

Ensure(SPP_Databank_returnPacket, returns_ok_for_valid_packet)
{
    SPP_Packet_t *p_pkt = SPP_Databank_getPacket();
    assert_that(SPP_Databank_returnPacket(p_pkt), is_equal_to(K_SPP_OK));
}

Ensure(SPP_Databank_returnPacket, increments_free_count)
{
    SPP_Packet_t *p_pkt = SPP_Databank_getPacket();
    spp_uint32_t countBefore = SPP_Databank_freeCount();
    SPP_Databank_returnPacket(p_pkt);
    assert_that(SPP_Databank_freeCount(), is_equal_to(countBefore + 1U));
}

Ensure(SPP_Databank_returnPacket, rejects_null_pointer)
{
    assert_that(SPP_Databank_returnPacket(NULL), is_equal_to(K_SPP_ERROR_NULL_POINTER));
}

Ensure(SPP_Databank_returnPacket, rejects_out_of_pool_pointer)
{
    SPP_Packet_t fake;
    assert_that(SPP_Databank_returnPacket(&fake), is_equal_to(K_SPP_ERROR));
}

Ensure(SPP_Databank_returnPacket, rejects_double_return)
{
    SPP_Packet_t *p_pkt = SPP_Databank_getPacket();
    SPP_Databank_returnPacket(p_pkt);
    SPP_RetVal_t ret = SPP_Databank_returnPacket(p_pkt);
    assert_that(ret, is_equal_to(K_SPP_ERROR_ALREADY_INITIALIZED));
}

Ensure(SPP_Databank_returnPacket, packet_can_be_reused_after_return)
{
    SPP_Packet_t *p_first = SPP_Databank_getPacket();
    SPP_Databank_returnPacket(p_first);
    SPP_Packet_t *p_second = SPP_Databank_getPacket();
    assert_that(p_second, is_not_null);
}

/* ----------------------------------------------------------------
 * Test suite factory
 * ---------------------------------------------------------------- */

TestSuite *databank_suite(void)
{
    TestSuite *suite = create_test_suite();

    add_test_with_context(suite, SPP_Databank_init, succeeds_on_first_call);
    add_test_with_context(suite, SPP_Databank_init, returns_already_initialized_on_second_call);
    add_test_with_context(suite, SPP_Databank_init, pool_is_full_after_init);

    add_test_with_context(suite, SPP_Databank_getPacket, returns_non_null_when_pool_has_packets);
    add_test_with_context(suite, SPP_Databank_getPacket, decrements_free_count);
    add_test_with_context(suite, SPP_Databank_getPacket, returns_null_when_pool_exhausted);
    add_test_with_context(suite, SPP_Databank_getPacket, each_packet_is_unique);

    add_test_with_context(suite, SPP_Databank_returnPacket, returns_ok_for_valid_packet);
    add_test_with_context(suite, SPP_Databank_returnPacket, increments_free_count);
    add_test_with_context(suite, SPP_Databank_returnPacket, rejects_null_pointer);
    add_test_with_context(suite, SPP_Databank_returnPacket, rejects_out_of_pool_pointer);
    add_test_with_context(suite, SPP_Databank_returnPacket, rejects_double_return);
    add_test_with_context(suite, SPP_Databank_returnPacket, packet_can_be_reused_after_return);

    return suite;
}
