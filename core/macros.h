/**
 * @file macros.h
 * @brief SPP core compile-time configuration macros.
 */

#ifndef MACROS_H
#define MACROS_H

#ifdef __cplusplus
extern "C"
{
#endif

/* ---------------------------------------------------------------- */
/*  Configuration Macros                                            */
/* ---------------------------------------------------------------- */

/** @brief Number of packets in the static databank pool. */
#define DATA_BANK_SIZE 5

/** @brief Enable static allocation behaviour for functions. */
#define STATIC

/** @brief Default stack size in bytes for OSAL tasks. */
#define STACK_SIZE 8192

#ifdef __cplusplus
}
#endif

#endif /* MACROS_H */
