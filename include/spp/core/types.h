/**
 * @file types.h
 * @brief Portable integer types and SPI/storage configuration types for SPP.
 *
 * Provides width-exact integer typedefs derived from <stdint.h> and <stdbool.h>
 * so that SPP code is independent of compiler-specific type widths.  Also
 * defines configuration structs shared between the HAL interface and port
 * implementations.
 *
 * Naming conventions used in this file:
 * - Portable base types:  spp_<sign><width>_t   (lowercase snake)
 * - Config struct types:  SPP_<Name>_t           (uppercase prefix, CamelCase)
 * - Enum / constant values: K_SPP_*
 */

#ifndef SPP_TYPES_H
#define SPP_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ----------------------------------------------------------------
 * Portable integer aliases
 * ---------------------------------------------------------------- */

typedef uint8_t  spp_uint8_t;   /**< Unsigned 8-bit integer.  */
typedef int8_t   spp_int8_t;    /**< Signed 8-bit integer.    */
typedef uint16_t spp_uint16_t;  /**< Unsigned 16-bit integer. */
typedef int16_t  spp_int16_t;   /**< Signed 16-bit integer.   */
typedef uint32_t spp_uint32_t;  /**< Unsigned 32-bit integer. */
typedef int32_t  spp_int32_t;   /**< Signed 32-bit integer.   */
typedef uint64_t spp_uint64_t;  /**< Unsigned 64-bit integer. */
typedef int64_t  spp_int64_t;   /**< Signed 64-bit integer.   */
typedef bool     spp_bool_t;    /**< Boolean type.            */

/* ----------------------------------------------------------------
 * SPI configuration types
 * ---------------------------------------------------------------- */

/**
 * @brief SPI clock phase/polarity mode.
 */
typedef enum
{
    K_SPP_SPI_MODE0 = 0U, /**< CPOL=0, CPHA=0. */
    K_SPP_SPI_MODE1 = 1U, /**< CPOL=0, CPHA=1. */
    K_SPP_SPI_MODE3 = 3U  /**< CPOL=1, CPHA=1. */
} SPP_SpiMode_t;

/**
 * @brief SPI duplex mode.
 */
typedef enum
{
    K_SPP_SPI_FULL_DUPLEX = 0U, /**< Simultaneous TX and RX. */
    K_SPP_SPI_HALF_DUPLEX = 1U  /**< Shared TX/RX line.      */
} SPP_SpiDuplex_t;

/**
 * @brief SPI bus and device initialisation configuration.
 *
 * Passed to the HAL port's @c spiDeviceInit callback to describe one
 * physical SPI device.
 */
typedef struct
{
    int          busId;      /**< SPI bus / host identifier (platform-defined). */
    int          pinMiso;    /**< MISO (CIPO) GPIO number.                       */
    int          pinMosi;    /**< MOSI (COPI) GPIO number.                       */
    int          pinSclk;    /**< Clock GPIO number.                             */
    int          pinCs;      /**< Chip-select GPIO number.                       */
    unsigned int maxHz;      /**< Maximum SPI clock frequency in Hz.             */
    SPP_SpiMode_t   mode;    /**< SPI clock mode.                                */
    SPP_SpiDuplex_t duplex;  /**< Duplex mode.                                   */
    unsigned int queueSize;  /**< Transaction queue depth.                       */
} SPP_SpiInitCfg_t;

/* ----------------------------------------------------------------
 * Storage configuration type
 * ---------------------------------------------------------------- */

/**
 * @brief SD card / filesystem mount configuration.
 *
 * Passed to @ref SPP_Hal_storageMount().
 */
typedef struct
{
    const char   *p_basePath;           /**< VFS mount point (e.g. "/sdcard").  */
    int           spiHostId;            /**< SPI host used by the card.         */
    int           pinCs;                /**< Chip-select GPIO for the SD card.  */
    spp_uint32_t  maxFiles;             /**< Maximum simultaneously open files. */
    spp_uint32_t  allocationUnitSize;   /**< FAT allocation unit size in bytes. */
    spp_bool_t    formatIfMountFailed;  /**< Auto-format if mount fails.        */
} SPP_StorageInitCfg_t;

#endif /* SPP_TYPES_H */
