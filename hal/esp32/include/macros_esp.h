#include "spi.h" 
#include "esp_log.h" 
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include <string.h>

#define MISO_PIN 47
#define MOSI_PIN 38
#define CLK_PIN  48
#define USED_HOST SPI2_HOST

#define CS_PIN_BMP 18
#define CS_PIN_ICM 19
#define MAX_DEVICES 4

enum
{
    EMPTY = 0,
    READY = 1
};
