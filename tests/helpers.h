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

/* Declared in osal_posix.c and hal_stub.c */
extern const SPP_OsalPort_t g_posixOsalPort;
extern const SPP_HalPort_t  g_stubHalPort;

/**
 * @brief Reset internal SPP module state between tests.
 *
 * Accesses private static flags via compile-time hack: recompile source
 * with -DSPP_TEST_RESET to expose a reset function, OR use the approach
 * below where each test suite re-links a fresh translation unit.
 *
 * For simplicity in these unit tests we call each module's init() and rely
 * on the K_SPP_ERROR_ALREADY_INITIALIZED guard, then reset by relinking.
 * The CMakeLists compiles each test binary with its own object copy.
 */
static inline void sppTestSetup(void)
{
    SPP_Core_setOsalPort(&g_posixOsalPort);
    SPP_Core_setHalPort(&g_stubHalPort);
    SPP_Log_init();
    SPP_Log_setLevel(K_SPP_LOG_NONE); /* Silence during tests. */
}

#endif /* SPP_TEST_HELPERS_H */
