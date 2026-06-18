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
#define K_ESP32_PIN_CLK (48)

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
#define K_ESP32_PIN_CS_SDC (9)

/* ----------------------------------------------------------------
 * Device table constants
 * ---------------------------------------------------------------- */

/** @brief Maximum number of SPI device slots. */
#define K_ESP32_MAX_SPI_DEVICES (4U)

/* ----------------------------------------------------------------
 * Device index assignments
 * ---------------------------------------------------------------- */

/** @brief SPI device index for the BMP390. */
#define K_ESP32_SPI_IDX_BMP (0U)

/** @brief SPI device index for the ICM20948. */
#define K_ESP32_SPI_IDX_ICM (1U)

/** @brief SPI device index for the SD card. */
#define K_ESP32_SPI_IDX_SDC (2U)

/* ----------------------------------------------------------------
 * UART port assignments
 * ---------------------------------------------------------------- */
#define K_ESP32_UART_PORT_ID   UART_NUM_2
#define K_ESP32_UART_BAUD_RATE 115200

#define K_ESP32_UART_TX_PIN  1
#define K_ESP32_UART_RX_PIN  0
#define K_ESP32_UART_RTS_PIN -1 // Not used
#define K_ESP32_UART_CTS_PIN -1 // Not used

#define K_ESP32_UART_RX_BUFFER_SIZE 1024
#define K_ESP32_UART_TX_BUFFER_SIZE 1024

#endif /* SPP_MACROS_ESP32_H */
