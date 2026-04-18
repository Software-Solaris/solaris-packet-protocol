/**
 * @file macrosEsp32.h
 * @brief ESP32 hardware pin definitions and SPI constants.
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_ESP32_*
 */

#ifndef SPP_MACROS_ESP32_H
#define SPP_MACROS_ESP32_H

#include "driver/spi_common.h"

/* ----------------------------------------------------------------
 * SPI bus pin assignments
 * ---------------------------------------------------------------- */

/** @brief MISO (CIPO) GPIO pin. */
#define K_ESP32_PIN_MISO (47)

/** @brief MOSI (COPI) GPIO pin. */
#define K_ESP32_PIN_MOSI (38)

/** @brief SPI clock GPIO pin. */
#define K_ESP32_PIN_CLK  (48)

/** @brief SPI peripheral to use. */
#define K_ESP32_SPI_HOST SPI2_HOST

/* ----------------------------------------------------------------
 * Chip-select pin assignments
 * ---------------------------------------------------------------- */

/** @brief BMP390 barometer chip-select GPIO. */
#define K_ESP32_PIN_CS_BMP (18)

/** @brief ICM20948 IMU chip-select GPIO. */
#define K_ESP32_PIN_CS_ICM (21)

/** @brief SD card chip-select GPIO. */
#define K_ESP32_PIN_CS_SDC (8)

/* ----------------------------------------------------------------
 * Device table constants
 * ---------------------------------------------------------------- */

/** @brief Maximum number of SPI device slots. */
#define K_ESP32_MAX_SPI_DEVICES (4U)

/* ----------------------------------------------------------------
 * Device index assignments
 * ---------------------------------------------------------------- */

/** @brief SPI device index for the ICM20948. */
#define K_ESP32_SPI_IDX_ICM (0U)

/** @brief SPI device index for the BMP390. */
#define K_ESP32_SPI_IDX_BMP (1U)

#endif /* SPP_MACROS_ESP32_H */
