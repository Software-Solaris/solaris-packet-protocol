/**
 * @file bmp390_sim.c
 * @brief BMP390 simulated producer — model-rocket flight profile.
 *
 * Drop-in replacement for bmp390.c.  Implements the same
 * @ref SPP_SERVICE_ProducerContract_t interface but generates
 * deterministic, noise-corrupted barometric data that mimics a
 * small-scale model-rocket flight:
 *
 *   IDLE (5 s) → IGNITION (2 s) → ASCENT (20 s) →
 *   APOGEE (2 s) → DESCENT (40 s) → LANDED
 *
 * Physical reference (launch site at 726 m ASL, ~929 hPa, ~29.5 °C):
 *   - Motor burn peaks at +180 m during the first ~5 s of ascent.
 *   - Parachute deployment at apogee; descent rate ≈ 4.5 m/s.
 *   - Pressure and temperature are derived from altitude via the
 *     standard barometric formula and ISA lapse rate.
 *
 * No SPI, GPIO, or RTOS dependencies — suitable for hardware-in-the-loop
 * testing when the physical sensor is absent.
 *
 * Naming conventions:
 *   - Constants/macros: K_BMP390_SIM_*
 *   - Types:            BMP390Sim_*_t
 *   - Static functions: SPP_SERVICES_BMP390_SIM_*
 */

#include "spp/services/bmp390/bmp390.h"

#include "spp/services/databank/databank.h"
#include "spp/services/log/log.h"
#include "spp/services/pubsub/pubsub.h"
#include "spp/core/packet.h"

#include <math.h>
#include <stdio.h>

/* ----------------------------------------------------------------
 * SIMULATION CONSTANTS
 * ---------------------------------------------------------------- */

/** @brief Sample rate of the simulated sensor (Hz). */
#define K_BMP390_SIM_SAMPLE_RATE_HZ (5U)

/** @brief Altitude of the launch site above sea level (m). */
#define K_BMP390_SIM_ALT_BASE_M (726.0f)

/** @brief Peak altitude gain above launch site (m). */
#define K_BMP390_SIM_ALT_GAIN_M (180.0f)

/** @brief Sea-level pressure used in the barometric formula (Pa). */
#define K_BMP390_SIM_P0_PA (101325.0f)

/** @brief Ground temperature at launch site (°C). */
#define K_BMP390_SIM_TEMP_BASE_C (29.47f)

/** @brief ISA temperature lapse rate (°C/m). */
#define K_BMP390_SIM_LAPSE_RATE (0.0065f)

/* Phase durations in samples (at K_BMP390_SIM_SAMPLE_RATE_HZ). */
#define K_BMP390_SIM_SAMPLES_IDLE     (25U)  /**<  5 s — on pad, stable. */
#define K_BMP390_SIM_SAMPLES_IGNITION (10U)  /**<  2 s — ignitor + motor light-off, vibration. */
#define K_BMP390_SIM_SAMPLES_ASCENT   (100U) /**< 20 s — powered ascent + coast to apogee. */
#define K_BMP390_SIM_SAMPLES_APOGEE   (10U)  /**<  2 s — at peak, ejection charge fires. */
#define K_BMP390_SIM_SAMPLES_DESCENT  (200U) /**< 40 s — parachute descent ≈ 4.5 m/s. */

#define K_BMP390_SIM_TASK_TIMEOUT_MS (5000U)

/* ----------------------------------------------------------------
 * TYPES
 * ---------------------------------------------------------------- */

/**
 * @brief Flight phase state machine.
 */
typedef enum
{
    K_BMP390_SIM_PHASE_IDLE,     /**< On the launch pad, pre-ignition.     */
    K_BMP390_SIM_PHASE_IGNITION, /**< Ignitor firing, motor light-off.     */
    K_BMP390_SIM_PHASE_ASCENT,   /**< Powered ascent and unpowered coast.  */
    K_BMP390_SIM_PHASE_APOGEE,   /**< Apogee dwell before chute deploys.   */
    K_BMP390_SIM_PHASE_DESCENT,  /**< Parachute descent.                   */
    K_BMP390_SIM_PHASE_LANDED,   /**< Rocket on the ground, post-flight.   */
} BMP390Sim_Phase_t;

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS DECLARATIONS
 * ---------------------------------------------------------------- */
static SPP_RetVal_t SPP_SERVICES_BMP390_SIM_init(void);
static SPP_RetVal_t SPP_SERVICES_BMP390_SIM_acquireData(void);
static float SPP_SERVICES_BMP390_SIM_computeAltitude(void);
static float SPP_SERVICES_BMP390_SIM_noise(spp_uint16_t tick, float amplitude);
static float SPP_SERVICES_BMP390_SIM_altToPressure(float alt_m);
static float SPP_SERVICES_BMP390_SIM_altToTemperature(float alt_m);

/* ----------------------------------------------------------------
 * VARIABLES
 * ---------------------------------------------------------------- */
static const SPP_SERVICE_ProducerContract_t g_bmp390ProducerContract = {
    .p_nameProducer = "bmp390_sim",
    .tiemoutMs = K_BMP390_SIM_TASK_TIMEOUT_MS,
    .init = SPP_SERVICES_BMP390_SIM_init,
    .acquireData = SPP_SERVICES_BMP390_SIM_acquireData,
};

static BMP390Sim_Phase_t s_phase = K_BMP390_SIM_PHASE_IDLE;
static spp_uint16_t s_tick = 0U;      /**< Samples elapsed in the current phase. */
static spp_uint16_t s_totalTick = 0U; /**< Global sample counter (used for noise). */
static spp_uint16_t s_seq = 0U;       /**< SPP packet sequence counter. */

static const char *const k_svcTag = "BMP_SIM";

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief  Returns the producer contract for the simulated BMP390 service.
 * @return Pointer to the static @ref SPP_SERVICE_ProducerContract_t.
 */
const SPP_SERVICE_ProducerContract_t *SPP_SERVICES_BMP390_getProducerContract(void)
{
    return &g_bmp390ProducerContract;
}

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * ---------------------------------------------------------------- */

/**
 * @brief  Initialises the simulated BMP390 service.
 *
 * Resets the state machine and all counters to their initial values.
 *
 * @return K_SPP_OK always.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_SIM_init(void)
{
    s_phase = K_BMP390_SIM_PHASE_IDLE;
    s_tick = 0U;
    s_totalTick = 0U;
    s_seq = 0U;
    return K_SPP_OK;
}

/**
 * @brief  Generates band-limited pseudo-noise for sensor realism.
 *
 * Combines three sinusoids at incommensurable frequencies so the result
 * is not periodic over the flight duration.
 *
 * @param  tick       Global sample counter (drives the phase of the sinusoids).
 * @param  amplitude  Peak-to-peak amplitude of the noise (half on each side).
 * @return Noise value in [-amplitude, +amplitude].
 */
static float SPP_SERVICES_BMP390_SIM_noise(spp_uint16_t tick, float amplitude)
{
    float t = (float)tick;
    float n = sinf(t * 0.31f) + 0.4f * sinf(t * 1.73f) + 0.15f * sinf(t * 5.17f);
    return amplitude * (n / 1.55f); /* normalise to [-1, 1] then scale */
}

/**
 * @brief  Converts altitude to pressure using the barometric formula.
 *
 * P = P0 * (1 - h / 44330)^5.255
 *
 * @param  alt_m  Altitude above sea level in metres.
 * @return Pressure in Pa.
 */
static float SPP_SERVICES_BMP390_SIM_altToPressure(float alt_m)
{
    return K_BMP390_SIM_P0_PA * powf(1.0f - alt_m / 44330.0f, 5.255f);
}

/**
 * @brief  Converts altitude to temperature using the ISA lapse rate.
 *
 * T = T_base - lapse_rate * (h - h_base)
 *
 * @param  alt_m  Altitude above sea level in metres.
 * @return Temperature in °C.
 */
static float SPP_SERVICES_BMP390_SIM_altToTemperature(float alt_m)
{
    return K_BMP390_SIM_TEMP_BASE_C - K_BMP390_SIM_LAPSE_RATE * (alt_m - K_BMP390_SIM_ALT_BASE_M);
}

/**
 * @brief  Computes the simulated altitude for the current flight phase.
 *
 * Phase profiles:
 *   - IDLE / LANDED  : constant at K_BMP390_SIM_ALT_BASE_M.
 *   - IGNITION       : base altitude with amplified vibration noise.
 *   - ASCENT         : quadratic rise — fast off the pad, slowing to apogee.
 *   - APOGEE         : constant at peak altitude.
 *   - DESCENT        : linear drop from peak to base.
 *
 * @return Simulated true altitude in metres above sea level.
 */
static float SPP_SERVICES_BMP390_SIM_computeAltitude(void)
{
    float alt = K_BMP390_SIM_ALT_BASE_M;

    switch (s_phase)
    {
        case K_BMP390_SIM_PHASE_IDLE:
        case K_BMP390_SIM_PHASE_LANDED:
            alt = K_BMP390_SIM_ALT_BASE_M;
            break;

        case K_BMP390_SIM_PHASE_IGNITION:
            /* Rocket shakes on the pad — no net altitude change yet. */
            alt = K_BMP390_SIM_ALT_BASE_M;
            break;

        case K_BMP390_SIM_PHASE_ASCENT:
        {
            /* Quadratic profile: fast early rise, slowing toward apogee. */
            float t = (float)s_tick / (float)K_BMP390_SIM_SAMPLES_ASCENT;
            alt = K_BMP390_SIM_ALT_BASE_M + K_BMP390_SIM_ALT_GAIN_M * (2.0f * t - t * t);
            break;
        }

        case K_BMP390_SIM_PHASE_APOGEE:
            alt = K_BMP390_SIM_ALT_BASE_M + K_BMP390_SIM_ALT_GAIN_M;
            break;

        case K_BMP390_SIM_PHASE_DESCENT:
        {
            float t = (float)s_tick / (float)K_BMP390_SIM_SAMPLES_DESCENT;
            alt = (K_BMP390_SIM_ALT_BASE_M + K_BMP390_SIM_ALT_GAIN_M) * (1.0f - t) + K_BMP390_SIM_ALT_BASE_M * t;
            break;
        }

        default:
            break;
    }

    return alt;
}

/**
 * @brief  Acquisition callback invoked by the SPP superloop on each cycle.
 *
 * Advances the flight-phase state machine, computes altitude, derives
 * pressure and temperature, adds sensor noise, packs the three floats
 * into an SPP packet, and publishes it to the pubsub bus.
 *
 * @return K_SPP_OK on success, K_SPP_ERROR if no free packet is available.
 */
static SPP_RetVal_t SPP_SERVICES_BMP390_SIM_acquireData(void)
{
    /* --- advance state machine --- */
    switch (s_phase)
    {
        case K_BMP390_SIM_PHASE_IDLE:
            if (s_tick >= K_BMP390_SIM_SAMPLES_IDLE)
            {
                s_phase = K_BMP390_SIM_PHASE_IGNITION;
                s_tick = 0U;
            }
            break;

        case K_BMP390_SIM_PHASE_IGNITION:
            if (s_tick >= K_BMP390_SIM_SAMPLES_IGNITION)
            {
                s_phase = K_BMP390_SIM_PHASE_ASCENT;
                s_tick = 0U;
            }
            break;

        case K_BMP390_SIM_PHASE_ASCENT:
            if (s_tick >= K_BMP390_SIM_SAMPLES_ASCENT)
            {
                s_phase = K_BMP390_SIM_PHASE_APOGEE;
                s_tick = 0U;
            }
            break;

        case K_BMP390_SIM_PHASE_APOGEE:
            if (s_tick >= K_BMP390_SIM_SAMPLES_APOGEE)
            {
                s_phase = K_BMP390_SIM_PHASE_DESCENT;
                s_tick = 0U;
            }
            break;

        case K_BMP390_SIM_PHASE_DESCENT:
            if (s_tick >= K_BMP390_SIM_SAMPLES_DESCENT)
            {
                s_phase = K_BMP390_SIM_PHASE_LANDED;
                s_tick = 0U;
            }
            break;

        case K_BMP390_SIM_PHASE_LANDED:
        default:
            break;
    }

    /* --- compute physical quantities --- */
    float altitude = SPP_SERVICES_BMP390_SIM_computeAltitude();
    float noiseAlt = (s_phase == K_BMP390_SIM_PHASE_IGNITION) ? 2.5f : 0.3f;
    altitude += SPP_SERVICES_BMP390_SIM_noise(s_totalTick, noiseAlt);

    float pressure = SPP_SERVICES_BMP390_SIM_altToPressure(altitude);
    pressure += SPP_SERVICES_BMP390_SIM_noise(s_totalTick + 7U, 30.0f);

    float temperature = SPP_SERVICES_BMP390_SIM_altToTemperature(altitude);
    temperature += SPP_SERVICES_BMP390_SIM_noise(s_totalTick + 13U, 0.1f);

    s_tick++;
    s_totalTick++;

    /* --- pack and publish --- */
    SPP_Packet_t *p_packet = SPP_SERVICES_DATABANK_getPacket();
    if (p_packet == NULL)
    {
        SPP_LOGW(k_svcTag, "No free packet");
        return K_SPP_ERROR;
    }

#ifdef SPP_DEBUG_PRINT
    printf("[BMP_SIM] phase=%d alt=%.1fm P=%.1fhPa T=%.2fC\n", (int)s_phase, altitude, pressure / 100.0f, temperature);
#endif

    float payload[3] = {altitude, pressure, temperature};
    SPP_RetVal_t ret =
        SPP_SERVICES_DATABANK_packetData(p_packet, K_SPP_KPID_BMP390, s_seq++, payload, (spp_uint16_t)sizeof(payload));

    if (ret != K_SPP_OK)
    {
        SPP_LOGE(k_svcTag, "packetData failed ret=%d", (int)ret);
        (void)SPP_SERVICES_DATABANK_returnPacket(p_packet);
        return ret;
    }

    (void)SPP_SERVICES_PUBSUB_publish(p_packet);
    return K_SPP_OK;
}
