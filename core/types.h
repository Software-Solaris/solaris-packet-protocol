/**
 * @file types.h
 * @brief SPP core type definitions for portable integer types, SPI
 *        configuration, and OSAL primitives.
 */

#ifndef SPP_CORE_TYPES_H_
#define SPP_CORE_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

    /* ---------------------------------------------------------------- */
    /*  Portable Integer Types                                          */
    /* ---------------------------------------------------------------- */

    /** @brief Unsigned 8-bit integer. */
    typedef unsigned char spp_uint8_t;

    /** @brief Unsigned 16-bit integer. */
    typedef unsigned short spp_uint16_t;

    /** @brief Unsigned 32-bit integer. */
    typedef unsigned long spp_uint32_t;

    /** @brief Signed 8-bit integer. */
    typedef signed char spp_int8_t;

    /** @brief Signed 16-bit integer. */
    typedef signed short spp_int16_t;

    /** @brief Signed 32-bit integer. */
    typedef signed long spp_int32_t;

    /** @brief Unsigned 64-bit integer. */
    typedef unsigned long long spp_uint64_t;

    /** @brief Signed 64-bit integer. */
    typedef signed long long spp_int64_t;

    /** @brief Boolean type. */
    typedef bool spp_bool_t;

    /** @brief Size type. */
    typedef spp_uint32_t spp_size_t;

    /* ---------------------------------------------------------------- */
    /*  OSAL Types                                                      */
    /* ---------------------------------------------------------------- */

    /** @brief Function pointer type for OSAL tasks. */
    typedef void (*spp_task_func_t)(void *p_arg);

    /** @brief Opaque handle for an OSAL task. */
    typedef void *spp_task_handle_t;

    /* ---------------------------------------------------------------- */
    /*  SPI Init Types                                                  */
    /* ---------------------------------------------------------------- */

    /** @brief SPI clock polarity / phase mode. */
    typedef enum
    {
        SPP_SPI_MODE0 = 0,
        SPP_SPI_MODE1 = 1,
        SPP_SPI_MODE3 = 3,
    } spp_spi_mode_t;

    /** @brief SPI duplex mode. */
    typedef enum
    {
        SPP_SPI_FULL_DUPLEX = 0,
        SPP_SPI_HALF_DUPLEX = 1
    } spp_spi_duplex_t;

    /** @brief SPI device initialization configuration. */
    typedef struct
    {
        int bus_id;

        int pin_miso;
        int pin_mosi;
        int pin_sclk;
        int pin_cs;

        unsigned int max_hz;
        spp_spi_mode_t mode;
        spp_spi_duplex_t duplex;

        unsigned int queue_size;
    } SPP_SPI_InitCfg;

#ifdef __cplusplus
}
#endif

#endif /* SPP_CORE_TYPES_H_ */
