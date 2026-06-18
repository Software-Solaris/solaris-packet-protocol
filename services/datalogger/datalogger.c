/**
 * @file datalogger.c
 * @brief SD card packet logger — SPP consumer service implementation.
 *
 * This module is a pure consumer: it subscribes to K_SPP_APID_ALL through the
 * pub/sub router and writes every received packet to a text file on the SD card.
 * It never reads hardware directly and has no produce() function.
 *
 * Log format:
 *   Log messages:   "[I] TAG: message text"
 *   Sensor packets: "ts=12345 apid=0x0004 seq=7 len=12 payload_hex=44 9A ..."
 */

#include "spp/services/datalogger/datalogger.h"

#include "spp/hal/storage/storage.h"
#include "spp/hal/spi/spi.h"
#include "spp/core/packet.h"
#include "spp/services/log/log.h"
#include "spp/core/types.h"

#include <string.h>

/* ----------------------------------------------------------------
 * CONSTANTS
 * ---------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DECLARATIONS
 * ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_init(void);
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_deliverToMailbox(const SPP_Packet_t *p_pkt);
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_consumeData(void *p_data);
static SPP_RetVal_t SDC_cmdGoIdleState(void);
static SPP_RetVal_t SDC_cmdSendIfCond(void);

/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */
static const SPP_SERVICE_ConsumerContract_t g_dataloggerConsumerContract = {
    .consumerID = K_DATALOGGER_CONSUMER_ID,
    .priority = K_DATALOGGER_CONSUMER_PRIO,
    .p_nameConsumer = "datalogger",
    .tiemoutMs = K_DATALOGGER_TIMEOUT_MS,
    .suscribeToApid = 0U,
    .init = SPP_SERVICES_DATALOGGER_init,
    .deliverToMailbox = SPP_SERVICES_DATALOGGER_deliverToMailbox,
    .consumeData = SPP_SERVICES_DATALOGGER_consumeData,
};

static Datalogger_t s_datalogger;
static const char *const k_tag = "DATALOGGER";

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */
const SPP_SERVICE_ConsumerContract_t *SPP_SERVICES_DATALOGGER_getConsumerContract(void)
{
    return &g_dataloggerConsumerContract;
}

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * ---------------------------------------------------------------- */
/**
 * @brief Initialise the SD card and open the log file for writing.
 * @return K_SPP_OK on success, K_SPP_ERROR on mount or file-open failure.
 */
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_init(void)
{
    /* Init SPI for SD card */
    s_datalogger.p_spiHandler = SPP_HAL_SPI_getHandle(K_DATALOGGER_SPI_DEV_IDX);
    SPP_RetVal_t ret = SPP_HAL_SPI_deviceInit(s_datalogger.p_spiHandler);
    if (ret != K_SPP_OK)
    {
        SPP_LOGE(k_tag, "SPI device init failed");
        return ret;
    }

    ret = SDC_cmdGoIdleState();
    if (ret != K_SPP_OK)
    {
        SPP_LOGE(k_tag, "CMD0 failed");
        return ret;
    }

    ret = SDC_cmdSendIfCond();
    if (ret != K_SPP_OK)
    {
        SPP_LOGE(k_tag, "CMD8 failed");
        return ret;
    }

    return K_SPP_OK;
}

/**
 * @brief  Sends 10 dummy bytes (CS HIGH) then CMD0 and reads up to 8 bytes for R1.
 *         CMD0 resets the card and puts it in SPI mode (CS must be LOW during the command).
 * @return K_SPP_OK if R1 = 0x01 (in_idle_state), K_SPP_ERROR otherwise.
 */
static SPP_RetVal_t SDC_cmdGoIdleState(void)
{
    /* 10 dummy bytes with CS HIGH: ≥74 clock cycles, MOSI HIGH (spec §6.4.1) */
    spp_uint8_t dummy[10];
    memset(dummy, 0xFF, sizeof(dummy));
    SPP_RetVal_t ret = SPP_HAL_SPI_transmit(s_datalogger.p_spiHandler, dummy, sizeof(dummy));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    /* CMD0 + read 8 bytes for R1 (NCR: card may take up to 8 byte-clocks to respond) */
    spp_uint8_t buf[14] = {
        0x40, 0x00, 0x00, 0x00, 0x00, 0x95,            /* CMD0: GO_IDLE_STATE, CRC=0x95 */
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF /* dummy bytes to clock in R1 */
    };
    ret = SPP_HAL_SPI_transmit(s_datalogger.p_spiHandler, buf, sizeof(buf));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    /* R1 is the first non-0xFF byte in the NCR window (bytes 6-13).
     * Bytes 0-5 are the transmitted CMD0 and may contain MOSI coupling noise. */
    spp_uint8_t r1 = 0xFF;
    for (spp_uint8_t i = 6U; i < (spp_uint8_t)sizeof(buf); i++)
    {
        if (buf[i] != 0xFFU)
        {
            r1 = buf[i];
            break;
        }
    }

    SPP_LOGI(k_tag, "CMD0 R1=0x%02X", r1);
    return (r1 == 0x01U) ? K_SPP_OK : K_SPP_ERROR;
}

/**
 * @brief  Sends CMD8 (SEND_IF_COND) and parses the R7 response.
 *         Determines whether the card is SD v2+ (SDHC/SDXC capable) or SD v1/MMC.
 *         - R1=0x01 + echo match  → SD v2+, card supports 2.7-3.6V
 *         - R1=0x05 (illegal cmd) → SD v1 or MMC, continue with v1 init path
 * @return K_SPP_OK on success or v1 detection, K_SPP_ERROR on voltage mismatch.
 */
static SPP_RetVal_t SDC_cmdSendIfCond(void)
{
    /* CMD8: VHS=0x01 (2.7–3.6 V), check pattern=0xAA, CRC=0x87 (spec §7.3.2.6) */
    spp_uint8_t buf[20] = {
        0x48, 0x00, 0x00, 0x01, 0xAA, 0x87,             /* CMD8 */
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* NCR  */
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF              /* R7   */
    };
    SPP_RetVal_t ret = SPP_HAL_SPI_transmit(s_datalogger.p_spiHandler, buf, sizeof(buf));
    if (ret != K_SPP_OK)
    {
        return ret;
    }

    /* Find R1 in the NCR window (bytes 6-19); bytes 0-5 are CMD8 and may carry coupling noise */
    spp_uint8_t r1 = 0xFFU;
    spp_uint8_t r1Idx = 0xFFU;
    for (spp_uint8_t i = 6U; i < (spp_uint8_t)sizeof(buf); i++)
    {
        if (buf[i] != 0xFFU)
        {
            r1 = buf[i];
            r1Idx = i;
            if (r1 == 0x01U || r1 == 0x05U)
            {
                break;
            }
        }
    }

    SPP_LOGI(k_tag, "CMD8 R1=0x%02X", r1);

    if (r1 == 0x05U)
    {
        /* Illegal command → SD v1 or MMC card */
        SPP_LOGW(k_tag, "CMD8 illegal command — SD v1 or MMC detected");
        return K_SPP_OK;
    }
    if (r1 != 0x01U)
    {
        return K_SPP_ERROR;
    }

    /* R7 = R1 + 4 bytes: [reserved 2B][VHS 1B][check pattern 1B] */
    if ((spp_uint8_t)(r1Idx + 4U) >= (spp_uint8_t)sizeof(buf))
    {
        SPP_LOGE(k_tag, "CMD8 R7 truncated");
        return K_SPP_ERROR;
    }
    spp_uint8_t vhs = buf[r1Idx + 3U];
    spp_uint8_t pattern = buf[r1Idx + 4U];
    SPP_LOGI(k_tag, "CMD8 R7: VHS=0x%02X pattern=0x%02X", vhs, pattern);

    if ((vhs != 0x01U) || (pattern != 0xAAU))
    {
        SPP_LOGE(k_tag, "CMD8 voltage or pattern mismatch");
        return K_SPP_ERROR;
    }

    return K_SPP_OK;
}

/**
 * @brief Receive a packet from the pub/sub router into the consumer mailbox.
 * @param  p_pkt  Packet dispatched by the router.
 * @return K_SPP_OK on success, K_SPP_ERROR_NULL_POINTER if p_pkt is NULL.
 */
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_deliverToMailbox(const SPP_Packet_t *p_pkt)
{
    if (p_pkt == NULL)
    {
        return K_SPP_ERROR_NULL_POINTER;
    }

    return K_SPP_OK;
}

/**
 * @brief Consume a packet from the mailbox and write it to the SD card log file.
 * @param  p_data  Unused (context pointer reserved for future use).
 * @return K_SPP_OK on success, K_SPP_ERROR if the file is not open or write fails.
 */
static SPP_RetVal_t SPP_SERVICES_DATALOGGER_consumeData(void *p_data)
{
    (void)p_data;

    return K_SPP_OK;
}
