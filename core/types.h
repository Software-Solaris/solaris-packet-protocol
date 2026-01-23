#ifndef SPP_CORE_TYPES_H_
#define SPP_CORE_TYPES_H_

typedef unsigned char  spp_uint8_t;
typedef unsigned short spp_uint16_t;
typedef unsigned long  spp_uint32_t;
typedef bool spp_bool_t;

typedef spp_uint32_t      spp_size_t;

// OSAL
typedef void (*spp_task_func_t)(void *arg);
typedef void* spp_task_handle_t;

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

//---End Init Types---
#endif /* SPP_CORE_TYPES_H_ */