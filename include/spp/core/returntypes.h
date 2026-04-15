/**
 * @file returntypes.h
 * @brief Standard return-value type used across all SPP modules.
 *
 * Every public SPP function returns @ref retval_t.  Callers must check the
 * return value; ignoring it is a programming error.
 *
 * Naming conventions used in this file:
 * - Return-code values: SPP_* (no K_ prefix — cross-module, not hardware constants)
 * - Types: lowercase snake (foundation types shared with external components)
 */

#ifndef SPP_RETURNTYPES_H
#define SPP_RETURNTYPES_H

/* ----------------------------------------------------------------
 * Return-value enumeration
 * ---------------------------------------------------------------- */

/**
 * @brief Unified return type for all SPP API functions.
 */
typedef enum
{
    SPP_OK,                        /**< Operation completed successfully. */
    SPP_ERROR,                     /**< Generic unspecified error. */
    SPP_NOT_ENOUGH_PACKETS,        /**< Packet pool exhausted. */
    SPP_NULL_PACKET,               /**< Packet pointer is NULL. */
    SPP_ERROR_ALREADY_INITIALIZED, /**< Module already initialised; call ignored. */
    SPP_ERROR_NULL_POINTER,        /**< A required pointer argument is NULL. */
    SPP_ERROR_NOT_INITIALIZED,     /**< Module has not been initialised yet. */
    SPP_ERROR_INVALID_PARAMETER,   /**< An argument value is out of range. */
    SPP_ERROR_ON_SPI_TRANSACTION,  /**< SPI transaction failed. */
    SPP_ERROR_TIMEOUT,             /**< Operation timed out. */
    SPP_ERROR_NO_PORT,             /**< No OSAL/HAL port has been registered. */
    SPP_ERROR_REGISTRY_FULL        /**< Service registry is full. */
} retval_t;

#endif /* SPP_RETURNTYPES_H */
