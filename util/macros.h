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
 */
#ifndef SPP_NO_MALLOC
#define SPP_NO_MALLOC 0
#endif

/**
 * @brief Disable RTOS task and synchronisation primitives.
 *
 * When set, the cooperative bare-metal scheduler is used instead.
 * Services must not block; they yield via SPP_OSAL_taskDelayMs().
 */
#ifndef SPP_NO_RTOS
#define SPP_NO_RTOS 0
#endif

/**
 * @brief Disable SD card / filesystem storage support.
 *
 * When set, the storage HAL stubs return K_SPP_OK without doing anything.
 */
#ifndef SPP_NO_STORAGE
#define SPP_NO_STORAGE 0
#endif

/* ----------------------------------------------------------------
 * Capacity constants
 * ---------------------------------------------------------------- */

/** @brief Number of packets in the static databank pool. */
#ifndef K_SPP_DATABANK_SIZE
#define K_SPP_DATABANK_SIZE (5U)
#endif

/** @brief Depth of the db_flow ready FIFO (must be a power of two). */
#ifndef K_SPP_DBFLOW_READY_SIZE
#define K_SPP_DBFLOW_READY_SIZE (16U)
#endif

/**
 * @brief Default task stack size in bytes for OSAL task creation.
 *
 * The FreeRTOS port divides this by sizeof(StackType_t) to get words.
 * 4096 bytes = 1024 StackType_t words — adequate for typical sensor tasks.
 * Override per project: -DK_SPP_STACK_SIZE=8192 for heavier tasks.
 */
#ifndef K_SPP_STACK_SIZE
#define K_SPP_STACK_SIZE (4096U)
#endif

/**
 * @brief Maximum number of concurrent tasks (bare-metal + FreeRTOS static pool).
 *
 * Each slot costs K_SPP_STACK_SIZE + sizeof(StaticTask_t) bytes of SRAM.
 * Keep this at the real maximum for your application to avoid wasting DRAM.
 */
#ifndef K_SPP_MAX_TASKS
#define K_SPP_MAX_TASKS (8U)
#endif

/** @brief Maximum number of event groups that can be created. */
#ifndef K_SPP_MAX_EVENT_GROUPS
#define K_SPP_MAX_EVENT_GROUPS (4U)
#endif

/** @brief Maximum number of services that can be registered. */
#ifndef K_SPP_MAX_SERVICES
#define K_SPP_MAX_SERVICES (16U)
#endif

#endif /* SPP_MACROS_H */
