# services/icm20948/

ICM20948 9-axis IMU service (3-axis accelerometer + 3-axis gyroscope + 3-axis magnetometer via AK09916). Reads the DMP FIFO on a data-ready interrupt and publishes a 9-float sensor packet via pub/sub.

APID: `0x0201`

---

## Files

| File | Description |
|---|---|
| `icm20948.h` | Public API, register map, config structs |
| `icm20948.c` | Driver implementation, DMP loading, service task |
| `dmp_image.h` | DMP firmware binary image (do not edit) |

---

## Key types

```c
typedef struct {
    spp_uint8_t  spiDevIdx;   // SPI device index (0 = ICM20948 on ESP32 port)
    spp_uint8_t  intPin;      // Data-ready interrupt GPIO
    spp_uint32_t intIntrType; // Interrupt trigger type (rising edge = 1)
    spp_uint32_t intPull;     // Pull resistor: 0=none 1=up 2=down
} ICM20948_ServiceCfg_t;

typedef struct {
    float      ax, ay, az;   // Accelerometer (g)
    float      gx, gy, gz;   // Gyroscope (dps)
    float      mx, my, mz;   // Magnetometer (µT)
    spp_bool_t dataReady;
} ICM20948_SensorData_t;

typedef struct {
    volatile spp_bool_t drdyFlag;   // Set by GPIO ISR, cleared by ServiceTask
    SPP_GpioIsrCtx_t    isr_ctx;
    spp_uint8_t         intPin;
    spp_uint32_t        intIntrType;
    spp_uint32_t        intPull;
} ICM20948_Data_t;

typedef struct {
    void                  *p_spi;
    ICM20948_Data_t        icmData;
    ICM20948_SensorData_t  lastData;
    spp_uint16_t           seq;
} ICM20948_ServiceCtx_t;
```

---

## Register banks

The ICM20948 has four register banks (0–3) selected via register `0x7F`. The driver handles bank switching transparently — do not write to `0x7F` directly.

---

## DMP support

The DMP firmware (`dmp_image.h`) is loaded into the sensor's RAM at init time. When DMP is enabled:
- Outputs: rotation quaternion (Q30 fixed-point), step count, activity classification
- DMP runs independently on the sensor — the host only reads results

Disable DMP at build time with `-DSPP_ICM20948_NO_DMP=1` to save ~14 KB of flash.

---

## Registration

```c
extern const SPP_ServiceDesc_t g_icm20948ServiceDesc;

static ICM20948_ServiceCtx_t s_icmCtx;
static ICM20948_ServiceCfg_t s_icmCfg = {
    .spiDevIdx   = 0U,
    .intPin      = 4U,
    .intIntrType = 1U,   // rising edge
    .intPull     = 0U,   // no pull
};

SPP_Service_register(&g_icm20948ServiceDesc, &s_icmCtx, &s_icmCfg);
```

---

## Superloop integration

```c
// In the application superloop:
if (s_icmCtx.icmData.drdyFlag)
{
    ICM20948_ServiceTask(&s_icmCtx);
}
```

`ICM20948_ServiceTask()` clears `drdyFlag`, drains the DMP FIFO, calls `SPP_Databank_getPacket()` → `SPP_Databank_packetData()` → `SPP_PubSub_publish()`.

---

## Packet payload layout (36 bytes)

| Offset | Type | Field |
|---|---|---|
| 0  | `float` | Accel X (g) |
| 4  | `float` | Accel Y (g) |
| 8  | `float` | Accel Z (g) |
| 12 | `float` | Gyro X (dps) |
| 16 | `float` | Gyro Y (dps) |
| 20 | `float` | Gyro Z (dps) |
| 24 | `float` | Mag X (µT) |
| 28 | `float` | Mag Y (µT) |
| 32 | `float` | Mag Z (µT) |

---

## Hardware configuration (ESP32-S3)

- SPI device index: 0 (CS GPIO 21, 1 MHz, MODE0)
- INT pin: 4 (configured via `ICM20948_ServiceCfg_t`)
