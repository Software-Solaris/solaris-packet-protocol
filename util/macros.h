/**
 * @file macros.h
 * @brief Compile-time configuration flags and utility macros for SPP.
 *
 * Feature flags follow the pattern @c SPP_NO_<FEATURE> — define them to 1
 * (e.g. in your CMakeLists.txt) to disable the corresponding feature and
 * reduce code size on constrained targets.
 *
 * Naming conventions used in this file:
 * - Feature-disable flags: SPP_NO_*
 * - Capacity constants:    K_SPP_*
 */

#ifndef SPP_MACROS_H
#define SPP_MACROS_H

/* ----------------------------------------------------------------
 * Feature-disable flags
 * (Define to 1 in CMake to disable the feature)
 * ---------------------------------------------------------------- */

/**
 * @brief Disable dynamic memory allocation (malloc/free).
 *
 * When set, the databank and service registry use only static storage.
 * This is the default mode for bare-metal targets.
 * Currently unused.
 */
#ifndef SPP_NO_MALLOC
#define SPP_NO_MALLOC 0
#endif

/**
 * @brief Disable SD card / filesystem storage support.
 *
 * When set, the storage HAL stubs return K_SPP_OK without doing anything.
 * Currently unused.
 */
#ifndef SPP_NO_STORAGE
#define SPP_NO_STORAGE 0
#endif

/* ----------------------------------------------------------------
 * Capacity constants
 * ---------------------------------------------------------------- */

/** @brief Number of packets in the static databank pool. */
#ifndef K_SPP_DATABANK_SIZE
#define K_SPP_DATABANK_SIZE (50U)
#endif

/* Subscriber dispatch priorities — defined in pubsub.h */

#endif /* SPP_MACROS_H */
