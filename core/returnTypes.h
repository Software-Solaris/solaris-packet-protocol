/**
 * @file returnTypes.h
 * @brief Standard return-value type used across all SPP modules.
 *
 * Every public SPP function returns @ref SPP_RetVal_t.  Callers must check the
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
    K_SPP_OK = 0,                    /**< Operation completed successfully. */
    K_SPP_ERROR,                     /**< Generic unspecified error. */
    K_SPP_NOT_ENOUGH_PACKETS,        /**< Packet pool exhausted. */
    K_SPP_NULL_PACKET,               /**< Packet pointer is NULL. */
    K_SPP_ERROR_ALREADY_INITIALIZED, /**< Module already initialised; call ignored. */
    K_SPP_ERROR_NULL_POINTER,        /**< A required pointer argument is NULL. */
    K_SPP_ERROR_NOT_INITIALIZED,     /**< Module has not been initialised yet. */
    K_SPP_ERROR_INVALID_PARAMETER,   /**< An argument value is out of range. */
    K_SPP_ERROR_ON_SPI_TRANSACTION,  /**< SPI transaction failed. */
    K_SPP_ERROR_TIMEOUT,             /**< Operation timed out. */
    K_SPP_ERROR_NO_PORT,             /**< No HAL port has been registered. */
    K_SPP_ERROR_REGISTRY_FULL        /**< Service registry is full. */
} SPP_RetVal_t;

#endif /* SPP_RETURNTYPES_H */
