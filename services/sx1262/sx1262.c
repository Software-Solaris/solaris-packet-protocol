/**
 * @file sx1262_service.c
 * @brief Driver para el transceptor SX1262
 */

#include "spp/services/sx1262/sx1262.h"
#include "spp/hal/spi.h"
#include "spp/hal/gpio.h"
#include "spp/hal/time.h"
#include <string.h>

/**
 * @brief Espera a que el pin BUSY baje (nivel 0).
 * El SX1262 ignora comandos SPI mientras BUSY está en nivel alto.
 */
static void wait_for_busy(spp_uint32_t busyPin)
{
    while (SPP_HAL_gpioRead(busyPin) == 1U) // hay que crear esta funcion
    {
        /* Posible timeout aqui*/
    }
}

/* --- Inicialización del Driver --- */

void SPP_SERVICES_SX1262_init(void *p_data)
{
    SX1262_Data_t *p_sx = (SX1262_Data_t *)p_data;

    p_sx->drdyFlag = false;
    p_sx->isr_ctx.p_flag = &p_sx->drdyFlag;

    SPP_HAL_gpioConfigInterrupt(p_sx->intPin, p_sx->intIntrType, p_sx->intPull);
    SPP_HAL_gpioRegisterIsr(p_sx->intPin, (void *)&p_sx->isr_ctx);
}

/* --- Implementación del Servicio --- */

static SPP_RetVal_t SPP_SERVICES_SX1262_serviceInit(void *p_ctx, const void *p_cfg)
{
    SX1262_ServiceCtx_t *ctx = (SX1262_ServiceCtx_t *)p_ctx;
    const SX1262_ServiceCfg_t *cfg = (const SX1262_ServiceCfg_t *)p_cfg;
    SPP_RetVal_t ret;

    ctx->p_cfg = cfg;
    ctx->sxData.p_handler_spi = SPP_HAL_spiGetHandle(cfg->spiDevIdx);
    ctx->seq = 0U;

    ctx->sxData.intPin = cfg->intPin;
    ctx->sxData.intIntrType = cfg->intIntrType;
    ctx->sxData.intPull = cfg->intPull;

    SPP_SERVICES_SX1262_init(&ctx->sxData);


    // Set Standby (Opcode 0x80 + Config 0x00 para STDBY_RC)
    spp_uint8_t stdby_cmd[2] = {0x80U, 0x00U}; // 00 Oscilador Interno
    ret = SPP_HAL_spiTransmit(ctx->sxData.p_handler_spi, stdby_cmd, 2U);
    wait_for_busy(cfg->busyPin);

    // Set Regulator Mode (Opcode 0x96 + Modo)
    spp_uint8_t reg_cmd[2] = {0x96U, cfg->regulatorMode}; // LDO - mejor DC-DC -- menos consumo
    ret = SPP_HAL_spiTransmit(ctx->sxData.p_handler_spi, reg_cmd, 2U);
    wait_for_busy(cfg->busyPin);

    //  seguir aqui !!!

    return ret;
}

static SPP_RetVal_t SPP_SERVICES_SX1262_serviceStart(void *p_ctx)
{
    (void)p_ctx;
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_SERVICES_SX1262_serviceStop(void *p_ctx)
{
    (void)p_ctx;
    return K_SPP_OK;
}

static SPP_RetVal_t SPP_SERVICES_SX1262_serviceDeinit(void *p_ctx)
{
    (void)p_ctx;
    return K_SPP_OK;
}

/* --- Descriptor del Servicio --- */

const SPP_ServiceDesc_t g_sx1262ServiceDesc = {
    .p_name = "sx1262",
    .ctxSize = sizeof(SX1262_ServiceCtx_t),
    .init = SPP_SERVICES_SX1262_serviceInit,
    .start = SPP_SERVICES_SX1262_serviceStart,
    .stop = SPP_SERVICES_SX1262_serviceStop,
    .deinit = SPP_SERVICES_SX1262_serviceDeinit,
};