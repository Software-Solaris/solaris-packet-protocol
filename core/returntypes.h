/**
 * @file returntypes.h
 * @brief SPP return value enumeration for all public API functions.
 */

#ifndef RETURNTYPES_H
#define RETURNTYPES_H

#ifdef __cplusplus
extern "C"
{
#endif

    /* ---------------------------------------------------------------- */
    /*  Return Types                                                    */
    /* ---------------------------------------------------------------- */

    /** @brief Common return type for SPP functions. */
    typedef enum
    {
        /* Basic errors */
        SPP_OK,    /**< @brief Operation completed successfully. */
        SPP_ERROR, /**< @brief Generic error. */

        /* Packet-related errors */
        SPP_NOT_ENOUGH_PACKETS, /**< @brief Databank has no free packets. */
        SPP_NULL_PACKET,        /**< @brief Packet pointer is NULL. */

        /* Other errors for function returns */
        SPP_ERROR_ALREADY_INITIALIZED, /**< @brief Module was already initialized. */
        SPP_ERROR_NULL_POINTER,        /**< @brief A required pointer argument is NULL. */
        SPP_ERROR_NOT_INITIALIZED,     /**< @brief Module has not been initialized yet. */
        SPP_ERROR_INVALID_PARAMETER,   /**< @brief An argument value is out of range. */
        SPP_ERROR_ON_SPI_TRANSACTION   /**< @brief SPI transaction failed. */
    } retval_t;

#ifdef __cplusplus
}
#endif

#endif /* RETURNTYPES_H */
