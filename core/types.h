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

typedef uint8_t spp_uint8_t;   /**< Unsigned 8-bit integer.  */
typedef int8_t spp_int8_t;     /**< Signed 8-bit integer.    */
typedef uint16_t spp_uint16_t; /**< Unsigned 16-bit integer. */
typedef int16_t spp_int16_t;   /**< Signed 16-bit integer.   */
typedef uint32_t spp_uint32_t; /**< Unsigned 32-bit integer. */
typedef int32_t spp_int32_t;   /**< Signed 32-bit integer.   */
typedef uint64_t spp_uint64_t; /**< Unsigned 64-bit integer. */
typedef int64_t spp_int64_t;   /**< Signed 64-bit integer.   */
typedef bool spp_bool_t;       /**< Boolean type.            */


#endif /* SPP_TYPES_H */
