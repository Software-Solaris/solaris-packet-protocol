#ifndef SPP_CORE_TYPES_H_
#define SPP_CORE_TYPES_H_

typedef unsigned char  SPP_uint8_t;
typedef unsigned short SPP_uint16_t;
typedef unsigned long  Spp_uint32_t;

typedef Spp_uint32_t      SPP_size_t;
//---Init types---
typedef enum{
    SPP_SPI_MODE0 = 0,
    SPP_SPI_MODE1 = 1,
    SPP_SPI_MODE3 = 3,
}spp_spi_mode_t;

typedef enum{
    SPP_SPI_FULL_DUPLEX = 0,
    SPP_SPI_HALF_DUPLEX = 1
}spp_spi_duplex_t;

typedef struct{
    int bus_id;

    int pin_miso;
    int pin_mosi;
    int pin_sclk;
    int pin_cs;

    unsigned int max_hz;
    spp_spi_mode_t mode;
    spp_spi_duplex_t duplex;

    unsigned int queue_size;
}SPP_SPI_InitCfg;


/*--------------------------------------------OSAL-------------------------------------------------------------------*/
/** @brief Idle hook callback type */
typedef void (*SPP_OSAL_IdleHook)(void);
/**
 * @brief Task priority levels
 */
typedef enum {
    SPP_OSAL_PRIORITY_IDLE = 0,
    SPP_OSAL_PRIORITY_LOW = 1,
    SPP_OSAL_PRIORITY_NORMAL = 2,
    SPP_OSAL_PRIORITY_HIGH = 3,
    SPP_OSAL_PRIORITY_CRITICAL = 4
} SppPriority_t;

/**
 * @brief Task state enumeration
 */
typedef enum {
    SPP_OSAL_TASK_READY = 0,
    SPP_OSAL_TASK_RUNNING = 1,
    SPP_OSAL_TASK_BLOCKED = 2,
    SPP_OSAL_TASK_SUSPENDED = 3,
    SPP_OSAL_TASK_DELETED = 4
} SppTaskState;

//---End Init Types---
//hola
#endif /* SPP_CORE_TYPES_H_ */