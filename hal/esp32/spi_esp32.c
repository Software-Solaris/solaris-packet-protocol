#include "spi.h" 
#include "esp_log.h" 
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "macros_esp.h"

static const char *TAG = "SPP_HAL_SPI";

static int n_devices = 2;

static spi_device_handle_t device_ids[MAX_DEVICES] = {NULL}; //[0]-BMP [1]-ICM
static int device_state[MAX_DEVICES] = {EMPTY};

//---Init---
// Init del bus (comun)
retval_t SPP_HAL_SPI_BusInit(void)
{
    esp_err_t ret;

    spi_bus_config_t buscfg = 
    {
    .miso_io_num     = MISO_PIN,
    .mosi_io_num     = MOSI_PIN,
    .sclk_io_num     = CLK_PIN,
    .quadwp_io_num   = -1,
    .quadhd_io_num   = -1,
    .max_transfer_sz = 0
    };

    ret = spi_bus_initialize(USED_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return SPP_ERROR;

    return SPP_OK;
}

void* SPP_HAL_SPI_GetHandler(void)
{
    for (int i = 0; i < n_devices; ++i)
    {
        if (device_ids[i] == NULL)
        {
            void* p_dev = (void*)&device_ids[i];
            return p_dev;
        }
    }

    return NULL;
}

retval_t SPP_HAL_SPI_DeviceInit(void* p_handler)
{ 
    if (p_handler == NULL) return SPP_ERROR;

    spi_device_handle_t* p_handle = (spi_device_handle_t*)p_handler;

    int flag = -1;

    for (int j = 0; j < n_devices; ++j)
    {
        if (p_handle == &device_ids[j])
        {
            flag = j;
            break;
        }   
    }

    //if (*p_handle != NULL) return SPP_OK; // se xa está inicializado que o salte pero non pete

    if (flag < 0) return SPP_ERROR;

    if (device_state[flag] == READY) return SPP_OK;

    spi_device_interface_config_t devcfg = {0};

    if (flag == 0) // BMP
    {
        devcfg.clock_speed_hz = 500 * 1000;
        devcfg.mode           = 0;
        devcfg.spics_io_num   = CS_PIN_BMP;
        devcfg.queue_size     = 7;
        devcfg.command_bits   = 8;
        devcfg.dummy_bits     = 8;
        devcfg.flags          = SPI_DEVICE_HALFDUPLEX;
    } 
    else if (flag == 1) // ICM
    {         
        devcfg.clock_speed_hz = 1 * 1000 * 1000;
        devcfg.mode           = 0;
        devcfg.spics_io_num   = CS_PIN_ICM;
        devcfg.queue_size     = 7;
        devcfg.command_bits   = 0;
        devcfg.dummy_bits     = 0;
        devcfg.flags          = 0;
    }

    esp_err_t ret;
    ret = spi_bus_add_device(USED_HOST, &devcfg, p_handle);
    if (ret != ESP_OK) return SPP_ERROR;

    device_state[flag] = READY;

    return SPP_OK;
}
//---End Init---

retval_t SPP_HAL_SPI_Transmit(void* handler, void* data_to_send, void* data_to_recieve, spp_uint8_t length) {
    spi_device_handle_t p_handler = (spi_device_handle_t) handler;
    esp_err_t trans_result = ESP_OK;

    if (length <= 2) {
        // Only one transmission
        spi_transaction_t trans_desc = {0};
        trans_desc.length    = 8 * length;
        trans_desc.tx_buffer = data_to_send;
        trans_desc.rx_buffer = data_to_recieve;

        trans_result = spi_device_transmit(p_handler, &trans_desc);

    } else {
        spp_uint8_t* p_tx = (spp_uint8_t*) data_to_send;
        spp_uint8_t* p_rx = (spp_uint8_t*) data_to_recieve;
        
        int num_ops = length / 2;
        spi_transaction_t transactions[num_ops];

        for (int i = 0; i < num_ops; i++) {
            spi_transaction_t* p_transaction = &transactions[i];
            *p_transaction = (spi_transaction_t){0}; //We put all the struct to zero
            p_transaction->length    = 16; 
            p_transaction->tx_buffer = &p_tx[i * 2];
            p_transaction->rx_buffer = &p_rx[i * 2];

            trans_result = spi_device_queue_trans(p_handler, p_transaction, portMAX_DELAY);
            if (trans_result != ESP_OK) break;
        }

        for (int i = 0; i < num_ops && trans_result == ESP_OK; i++) {
            spi_transaction_t* ret_t;
            trans_result = spi_device_get_trans_result(p_handler, &ret_t, portMAX_DELAY);
            if (trans_result != ESP_OK) break;
        }
    }

    if (trans_result != ESP_OK) {
        return SPP_ERROR;
    }
    return SPP_OK;
}


