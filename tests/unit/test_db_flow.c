/**
 * @file test_db_flow.c
 * @brief BDD unit tests for the SPP db_flow FIFO.
 *
 * Coverage targets (100%):
 *  - SPP_DbFlow_init()       — first call, second call
 *  - SPP_DbFlow_pushReady()  — normal, NULL, full FIFO
 *  - SPP_DbFlow_popReady()   — normal, NULL out-pointer, empty FIFO
 *  - SPP_DbFlow_readyCount() — after push, after pop
 */

#include <cgreen/cgreen.h>
#include "spp/services/db_flow.h"
#include "spp/services/databank.h"
#include "spp/core/returntypes.h"
#include "spp/util/macros.h"

/* ----------------------------------------------------------------
 * Describe: SPP_DbFlow_init
 * ---------------------------------------------------------------- */

Describe(SPP_DbFlow_init);
BeforeEach(SPP_DbFlow_init) {}
AfterEach(SPP_DbFlow_init)  {}

Ensure(SPP_DbFlow_init, succeeds_on_first_call)
{
    assert_that(SPP_DbFlow_init(), is_equal_to(SPP_OK));
}

Ensure(SPP_DbFlow_init, returns_already_initialized_on_second_call)
{
    SPP_DbFlow_init();
    assert_that(SPP_DbFlow_init(), is_equal_to(SPP_ERROR_ALREADY_INITIALIZED));
}

Ensure(SPP_DbFlow_init, fifo_is_empty_after_init)
{
    SPP_DbFlow_init();
    assert_that(SPP_DbFlow_readyCount(), is_equal_to(0U));
}

/* ----------------------------------------------------------------
 * Describe: SPP_DbFlow_pushReady
 * ---------------------------------------------------------------- */

Describe(SPP_DbFlow_pushReady);
BeforeEach(SPP_DbFlow_pushReady)
{
    SPP_Databank_init();
    SPP_DbFlow_init();
}
AfterEach(SPP_DbFlow_pushReady) {}

Ensure(SPP_DbFlow_pushReady, returns_ok_for_valid_packet)
{
    SPP_Packet_t *p_pkt = SPP_Databank_getPacket();
    assert_that(SPP_DbFlow_pushReady(p_pkt), is_equal_to(SPP_OK));
}

Ensure(SPP_DbFlow_pushReady, increments_ready_count)
{
    SPP_Packet_t *p_pkt = SPP_Databank_getPacket();
    SPP_DbFlow_pushReady(p_pkt);
    assert_that(SPP_DbFlow_readyCount(), is_equal_to(1U));
}

Ensure(SPP_DbFlow_pushReady, rejects_null_packet)
{
    assert_that(SPP_DbFlow_pushReady(NULL), is_equal_to(SPP_ERROR_NULL_POINTER));
}

Ensure(SPP_DbFlow_pushReady, returns_error_when_fifo_full)
{
    /* Fill the FIFO using dummy stack-allocated packets. */
    SPP_Packet_t fakePackets[K_SPP_DBFLOW_READY_SIZE];
    for (spp_uint32_t i = 0U; i < K_SPP_DBFLOW_READY_SIZE; i++)
    {
        assert_that(SPP_DbFlow_pushReady(&fakePackets[i]), is_equal_to(SPP_OK));
    }
    /* One more should fail. */
    SPP_Packet_t extra;
    assert_that(SPP_DbFlow_pushReady(&extra), is_equal_to(SPP_ERROR));
}

/* ----------------------------------------------------------------
 * Describe: SPP_DbFlow_popReady
 * ---------------------------------------------------------------- */

Describe(SPP_DbFlow_popReady);
BeforeEach(SPP_DbFlow_popReady)
{
    SPP_Databank_init();
    SPP_DbFlow_init();
}
AfterEach(SPP_DbFlow_popReady) {}

Ensure(SPP_DbFlow_popReady, returns_pushed_packet)
{
    SPP_Packet_t *p_pushed = SPP_Databank_getPacket();
    SPP_DbFlow_pushReady(p_pushed);

    SPP_Packet_t *p_popped = NULL;
    retval_t ret = SPP_DbFlow_popReady(&p_popped);
    assert_that(ret, is_equal_to(SPP_OK));
    assert_that(p_popped, is_equal_to(p_pushed));
}

Ensure(SPP_DbFlow_popReady, decrements_ready_count)
{
    SPP_Packet_t dummy;
    SPP_DbFlow_pushReady(&dummy);
    SPP_Packet_t *p_out = NULL;
    SPP_DbFlow_popReady(&p_out);
    assert_that(SPP_DbFlow_readyCount(), is_equal_to(0U));
}

Ensure(SPP_DbFlow_popReady, rejects_null_out_pointer)
{
    SPP_Packet_t dummy;
    SPP_DbFlow_pushReady(&dummy);
    assert_that(SPP_DbFlow_popReady(NULL), is_equal_to(SPP_ERROR_NULL_POINTER));
}

Ensure(SPP_DbFlow_popReady, returns_not_enough_packets_when_empty)
{
    SPP_Packet_t *p_out = NULL;
    assert_that(SPP_DbFlow_popReady(&p_out), is_equal_to(SPP_NOT_ENOUGH_PACKETS));
    assert_that(p_out, is_null);
}

Ensure(SPP_DbFlow_popReady, fifo_is_fifo_ordered)
{
    SPP_Packet_t p1, p2, p3;
    SPP_DbFlow_pushReady(&p1);
    SPP_DbFlow_pushReady(&p2);
    SPP_DbFlow_pushReady(&p3);

    SPP_Packet_t *p_out = NULL;
    SPP_DbFlow_popReady(&p_out); assert_that(p_out, is_equal_to(&p1));
    SPP_DbFlow_popReady(&p_out); assert_that(p_out, is_equal_to(&p2));
    SPP_DbFlow_popReady(&p_out); assert_that(p_out, is_equal_to(&p3));
}

/* ----------------------------------------------------------------
 * Test suite factory
 * ---------------------------------------------------------------- */

TestSuite *db_flow_suite(void)
{
    TestSuite *suite = create_test_suite();

    add_test_with_context(suite, SPP_DbFlow_init, succeeds_on_first_call);
    add_test_with_context(suite, SPP_DbFlow_init, returns_already_initialized_on_second_call);
    add_test_with_context(suite, SPP_DbFlow_init, fifo_is_empty_after_init);

    add_test_with_context(suite, SPP_DbFlow_pushReady, returns_ok_for_valid_packet);
    add_test_with_context(suite, SPP_DbFlow_pushReady, increments_ready_count);
    add_test_with_context(suite, SPP_DbFlow_pushReady, rejects_null_packet);
    add_test_with_context(suite, SPP_DbFlow_pushReady, returns_error_when_fifo_full);

    add_test_with_context(suite, SPP_DbFlow_popReady, returns_pushed_packet);
    add_test_with_context(suite, SPP_DbFlow_popReady, decrements_ready_count);
    add_test_with_context(suite, SPP_DbFlow_popReady, rejects_null_out_pointer);
    add_test_with_context(suite, SPP_DbFlow_popReady, returns_not_enough_packets_when_empty);
    add_test_with_context(suite, SPP_DbFlow_popReady, fifo_is_fifo_ordered);

    return suite;
}
