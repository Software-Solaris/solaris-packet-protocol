/**
 * @file bmp390_dummy.c
 * @brief Minimal BMP390 dummy producer for SPP performance measurements.
 *
 * Drop-in replacement for bmp390.c. Implements the same
 * SPP_SERVICE_ProducerContract_t interface without using GPIO, SPI,
 * interrupts or physical hardware.
 *
 * The dummy publishes a constant payload equivalent to the real BMP390
 * service so the SPP Core path can be measured independently from the
 * sensor service and HAL overhead.
 */


/* ----------------------------------------------------------------
 * INCLUDES
 * ---------------------------------------------------------------- */
#include "spp/services/bmp390/bmp390.h"

#include "spp/services/databank/databank.h"
#include "spp/core/pubsub/pubsub.h"


/* ----------------------------------------------------------------
 * CONSTANTS
 * ---------------------------------------------------------------- */
#define K_BMP390_DUMMY_TASK_TIMEOUT_MS (5000U)


/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DECLARATIONS
 * ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_SERVICES_BMP390_DUMMY_init(void);
static SPP_RetVal_t SPP_SERVICES_BMP390_DUMMY_acquireData(SPP_Kpid_t kpid);


/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */
static const SPP_SERVICE_ProducerContract_t g_bmp390ProducerContract = {
    .p_nameProducer = "bmp390_dummy",
    .tiemoutMs = K_BMP390_DUMMY_TASK_TIMEOUT_MS,
    .init = SPP_SERVICES_BMP390_DUMMY_init,
    .acquireData = SPP_SERVICES_BMP390_DUMMY_acquireData,
};

static const spp_float32_t s_payload[3] = {
    0.0f,
    101325.0f,
    25.0f,
};

static spp_uint16_t s_seq = 0U;


/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_BMP390_getProducerContract(void)
{
    return &g_bmp390ProducerContract;
}


/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief Initialise the dummy BMP390 producer.
 *
 * Resets the packet sequence counter. No hardware is initialised.
 *
 * @return K_SPP_OK always.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_DUMMY_init(void)
{
    s_seq = 0U;

    return K_SPP_OK;
}


/**
 * @brief Publish a constant BMP390-equivalent packet.
 *
 * Exercises the normal DataBank and PubSub path without performing
 * sensor calculations, GPIO handling, SPI transactions or HAL calls.
 *
 * @param kpid Producer KPID assigned by PubSub.
 *
 * @return K_SPP_OK on success or the error returned by DataBank.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_DUMMY_acquireData(SPP_Kpid_t kpid)
{
    SPP_Packet_t *p_packet = SPP_SERVICES_DATABANK_getPacket();
    if (p_packet == NULL)
    {
        return K_SPP_ERROR;
    }

    SPP_RetVal_t ret =
        SPP_SERVICES_DATABANK_packetData(p_packet, kpid.value, s_seq++, s_payload, (spp_uint16_t)sizeof(s_payload));
    if (ret != K_SPP_OK)
    {
        (void)SPP_SERVICES_DATABANK_returnPacket(p_packet);
        return ret;
    }

    (void)SPP_SERVICES_PUBSUB_publish(p_packet);

    return K_SPP_OK;
}