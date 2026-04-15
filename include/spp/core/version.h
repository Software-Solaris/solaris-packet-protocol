/**
 * @file version.h
 * @brief SPP library version constants.
 *
 * Naming conventions used in this file:
 * - Constants/macros: K_SPP_VERSION_*
 */

#ifndef SPP_VERSION_H
#define SPP_VERSION_H

/** @brief SPP library major version number. */
#define K_SPP_VERSION_MAJOR (2U)

/** @brief SPP library minor version number. */
#define K_SPP_VERSION_MINOR (0U)

/** @brief SPP library patch version number. */
#define K_SPP_VERSION_PATCH (0U)

/** @brief Encoded version as a single 32-bit value (MAJOR<<16 | MINOR<<8 | PATCH). */
#define K_SPP_VERSION \
    ((K_SPP_VERSION_MAJOR << 16U) | (K_SPP_VERSION_MINOR << 8U) | K_SPP_VERSION_PATCH)

#endif /* SPP_VERSION_H */
