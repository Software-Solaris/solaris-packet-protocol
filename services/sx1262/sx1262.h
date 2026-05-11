#ifndef SPP_SX1262_H
#define SPP_SX1262_H

#include <stdint.h>
#include "spp/core/returnTypes.h"
#include "spp/core/types.h"
#include "spp/hal/gpio.h"

/* Opcodes Principales */
#define SX126X_CMD_SET_SLEEP          0x84
#define SX126X_CMD_SET_STANDBY        0x80
#define SX126X_CMD_SET_RX             0x82
#define SX126X_CMD_SET_TX             0x83
#define SX126X_CMD_SET_DIO_IRQ_PARAMS 0x08
#define SX126X_CMD_GET_IRQ_STATUS     0x12
#define SX126X_CMD_CLEAR_IRQ_STATUS   0x02

/* Bits de IRQ */
#define SX126X_IRQ_TX_DONE (1 << 0)
#define SX126X_IRQ_RX_DONE (1 << 1)
#define SX126X_IRQ_CRC_ERR (1 << 6)
#define SX126X_IRQ_TIMEOUT (1 << 9)

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        spp_uint8_t spiDevIdx;    /**< SPI device handle index.        */
        spp_uint32_t busyPin;     /**< Nos dice si está ocupado*/
        spp_uint32_t resetPin;    /**< Pin NRESET para reset de fabrica*/
        spp_uint32_t intPin;      /**< GPIO pin for DRDY interrupt (DIO1)    */
        spp_uint32_t intIntrType; /**< Interrupt edge type (1=rising). */
        spp_uint32_t intPull;     /**< Pull resistor (0=none,1=up).    */

        spp_uint8_t regulatorMode; /**< 0: LDO, 1: DC-DC */
        spp_bool_t useTcxo;        /**< Indica si se usa un oscilador TCXO. */
        spp_uint8_t tcxoVoltage;   /**< Voltaje para DIO3 */
        spp_uint32_t
            tcxoTimeout; /**< Tiempo de espera para estabilización del TCXO. ver DataSheet TCX0 */
    } SX1262_ServiceCfg_t;

    typedef struct
    {
        void *p_handler_spi;          /**< SPI device handle.                         */
        volatile spp_bool_t drdyFlag; /**< Set by ISR when data-ready fires.          */
        SPP_GpioIsrCtx_t isr_ctx;     /**< ISR context.          */
        spp_uint32_t intPin;          /**< GPIO pin for DRDY interrupt (DIO1)    */
        spp_uint32_t intIntrType;     /**< Interrupt edge type (1=rising). */
        spp_uint32_t intPull;         /**< Pull resistor (0=none,1=up).    */
    } SX1262_Data_t;

    typedef struct
    {
        SX1262_Data_t sxData;             /**< Sensor driver context.            */
        spp_uint16_t seq;                 /**< Packet sequence counter.          */
        const SX1262_ServiceCfg_t *p_cfg; /**< Back-pointer to config struct.    */
    } SX1262_ServiceCtx_t;

    void SPP_SERVICES_SX1262_init(void *p_data);

    typedef struct {
        const char *p_name;
        size_t ctxSize;
        SPP_RetVal_t (*init)(void *p_ctx, const void *p_cfg);
        SPP_RetVal_t (*start)(void *p_ctx);
        SPP_RetVal_t (*stop)(void *p_ctx);
        SPP_RetVal_t (*deinit)(void *p_ctx);
    } SPP_ServiceDesc_t;

    extern const SPP_ServiceDesc_t g_sx1262ServiceDesc;

#ifdef __cplusplus
}
#endif

#endif /* SPP_SX1262_H */