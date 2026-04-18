/**
 * @file test_helpers.h
 * @brief Shared test fixture: registers posix ports and resets SPP state.
 */

#ifndef SPP_TEST_HELPERS_H
#define SPP_TEST_HELPERS_H

#include "spp/core/core.h"
#include "spp/services/databank/databank.h"
#include "spp/services/db_flow/db_flow.h"
#include "spp/services/log/log.h"

/* Declared in halStub.c */
extern const SPP_HalPort_t g_stubHalPort;

/**
 * @brief Register HAL port and silence logging before each test suite.
 *
 * Each cgreen-runner shared library gets its own translation unit, so module
 * state starts fresh per test binary.
 */
static inline void sppTestSetup(void)
{
    SPP_CORE_setHalPort(&g_stubHalPort);
    SPP_SERVICES_LOG_init();
    SPP_SERVICES_LOG_setLevel(K_SPP_LOG_NONE); /* Silence during tests. */
}

#endif /* SPP_TEST_HELPERS_H */
