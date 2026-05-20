# services/icm20948/

ICM20948 9-axis IMU service (3-axis accelerometer + 3-axis gyroscope + 3-axis magnetometer via AK09916). Reads the DMP FIFO on a data-ready interrupt and publishes a sensor packet via pub/sub.

APID: `K_ICM20948_SERVICE_APID` (`0x0002`)

---

## Files

| File | Description |
|---|---|
| `icm20948.h` | Public API, register map, `ICM20948_t` context struct |
| `icm20948.c` | Driver implementation, DMP loading, FIFO parsing, service task |
| `dmpImage.h` | TDK InvenSense DMP firmware binary image (do not edit) |

---

## References and credits

The DMP firmware binary passed to the sensor is the vendor firmware image from TDK InvenSense for the ICM20948 DMP. It is treated as a binary blob and is not modified manually.

The driver implementation is written for the Solaris Packet Protocol architecture. The SparkFun ICM-20948 Arduino Library was used as a public technical reference for understanding the ICM20948 DMP initialization flow, register sequence and DMP configuration approach:

`https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary`

No SparkFun source code is copied into this service.

---

## Key types

```c
typedef struct {
    /* Config — set at declaration */
    spp_uint8_t  spiDevIdx;    // SPI device index (0 = ICM20948 on ESP32 port)
    spp_uint32_t intPin;       // Data-ready interrupt GPIO
    spp_uint32_t intIntrType;  // Interrupt trigger type (rising edge = 1)
    spp_uint32_t intPull;      // Pull resistor: 0=none 1=up 2=down

    /* Runtime — filled by init, do not set manually */
    void                  *p_spi;
    ICM20948_Data_t        icmData;    // drdyFlag + ISR context
    ICM20948_SensorData_t  lastData;
    spp_uint16_t           seq;
} ICM20948_t;

typedef struct {
    float      ax, ay, az;   // Accelerometer (g)
    float      gx, gy, gz;   // Gyroscope (dps)
    float      mx, my, mz;   // Magnetometer (µT)
    spp_bool_t dataReady;
} ICM20948_SensorData_t;
```

---

## Register banks

The ICM20948 has four register banks (0–3) selected via register `0x7F`. The driver handles bank switching transparently — do not write to `0x7F` directly.

---

## DMP support

The DMP firmware (`dmpImage.h`) is loaded into the sensor's RAM at init time. Current config:

- **DATA_OUT_CTL1 = 0xE400** → Accel + Gyro + Compass + Quat9
- **MOTION_EVENT_CTL = 0x03C0**
- **DMP FIFO packet size = 42 bytes**
- FIFO frame parsed by the driver:
  - Header: 2 bytes
  - Accelerometer: 6 bytes
  - Gyroscope raw: 6 bytes
  - Gyroscope bias: 6 bytes
  - Compass / magnetometer: 6 bytes
  - Quat9: 12 bytes
  - Heading accuracy: 2 bytes
  - Footer: 2 bytes

The driver currently publishes accel + gyro + magnetometer through SPP. Quat9 is parsed/debugged internally but is not part of the published SPP payload.

---

## Registration

```c
extern const SPP_Module_t g_icm20948Module;

static ICM20948_t s_icm = {
    .spiDevIdx   = 0U,
    .intPin      = 10U,
    .intIntrType = 1U,   // rising edge
    .intPull     = 0U,   // no pull
};

SPP_SERVICES_register(&g_icm20948Module, &s_icm);
```

`register()` calls `init` (loads DMP firmware, installs ISR) and `start` immediately.

---

## Packet payload layout (36 bytes — Accel + Gyro + Mag)

The published SPP payload is composed of 9 floats:

```c
float payload[9] = {
    ax, ay, az,
    gx, gy, gz,
    mx, my, mz
};
```

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

Important: the DMP FIFO frame is 42 bytes, but the SPP payload published by this service is 36 bytes.

---

## Hardware configuration (ESP32-S3)

- SPI device index: 0 (CS GPIO 21, 1 MHz, MODE0)
- INT pin: 10 (configured via `ICM20948_t`)