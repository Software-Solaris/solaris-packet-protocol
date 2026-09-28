/**
 * @file macrosRp2350.h
 * @brief RP2350 hardware pin definitions and peripheral constants for Solaris.
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_RP2350_*
 */

#ifndef SPP_MACROS_RP2350_H
#define SPP_MACROS_RP2350_H

/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

/* ----------------------------------------------------------------
 * SPI BUS DEFINITIONS
 * ---------------------------------------------------------------- */

/** @brief SPI peripheral instance to use. */
#define K_RP2350_SPI_INST           spi0

/** @brief SPI MISO (RX) GPIO pin. */
#define K_RP2350_PIN_MISO           (16U)

/** @brief SPI Clock (SCK) GPIO pin. */
#define K_RP2350_PIN_CLK            (18U)

/** @brief SPI MOSI (TX) GPIO pin. */
#define K_RP2350_PIN_MOSI           (19U)

/** @brief SPI default clock frequency (10 MHz). */
#define K_RP2350_SPI_DEFAULT_BAUD   (10000000U)

/** @brief SPI low-speed clock frequency for SD card init (400 kHz). */
#define K_RP2350_SPI_INIT_BAUD      (400000U)

/* ----------------------------------------------------------------
 * CHIP-SELECT (CS) PIN ASSIGNMENTS
 * ---------------------------------------------------------------- */

/** @brief BMP390 barometer chip-select GPIO. */
#define K_RP2350_PIN_CS_BMP         (17U)

/** @brief ICM20948 IMU chip-select GPIO. */
#define K_RP2350_PIN_CS_ICM         (20U)

/** @brief SD card chip-select GPIO. */
#define K_RP2350_PIN_CS_SDC         (21U)

/** @brief SX1262 / E22 LoRa transceiver chip-select GPIO. */
#define K_RP2350_PIN_CS_LORA        (22U)

/* ----------------------------------------------------------------
 * GPIO DATA-READY (DRDY) INTERRUPT PINS
 * ---------------------------------------------------------------- */

/** @brief BMP390 data-ready interrupt GPIO. */
#define K_RP2350_PIN_DRDY_BMP       (14U)

/** @brief ICM20948 data-ready interrupt GPIO. */
#define K_RP2350_PIN_DRDY_ICM       (15U)

/* ----------------------------------------------------------------
 * DEVICE TABLE CONSTANTS
 * ---------------------------------------------------------------- */

/** @brief Maximum number of SPI device slots. */
#define K_RP2350_MAX_SPI_DEVICES    (4U)

/* ----------------------------------------------------------------
 * DEVICE INDEX ASSIGNMENTS
 * ---------------------------------------------------------------- */

/** @brief SPI device index for the BMP390. */
#define K_RP2350_SPI_IDX_BMP        (0U)

/** @brief SPI device index for the ICM20948. */
#define K_RP2350_SPI_IDX_ICM        (1U)

/** @brief SPI device index for the SD card. */
#define K_RP2350_SPI_IDX_SDC        (2U)

/** @brief SPI device index for the LoRa transceiver. */
#define K_RP2350_SPI_IDX_LORA       (3U)

/* ----------------------------------------------------------------
 * UART PORT ASSIGNMENTS
 * ---------------------------------------------------------------- */

/** @brief UART peripheral instance for telemetry / GNSS. */
#define K_RP2350_UART_INST          uart0

/** @brief Default baudrate for UART. */
#define K_RP2350_UART_BAUD_RATE     (9600U)

/** @brief UART Transmit (TX) GPIO pin. */
#define K_RP2350_UART_TX_PIN        (0U)

/** @brief UART Receive (RX) GPIO pin. */
#define K_RP2350_UART_RX_PIN        (1U)

/** @brief UART receive software buffer size. */
#define K_RP2350_UART_RX_BUFFER_SIZE (1024U)

/** @brief UART transmit software buffer size. */
#define K_RP2350_UART_TX_BUFFER_SIZE (1024U)

#endif /* SPP_MACROS_RP2350_H */
