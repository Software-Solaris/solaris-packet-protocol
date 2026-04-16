# services/bmp390/

BMP390 barometric pressure and temperature sensor service. Runs as an OSAL task, waits for the DRDY interrupt, reads raw ADC data over SPI, compensates it using the sensor's factory calibration, and pushes altitude / pressure / temperature packets into the `db_flow` FIFO.

APID: `0x0101`

---

## Files

| File | Description |
|---|---|
| `bmp390.h` | Public API, config and context structs |
| `bmp390.c` | Driver implementation, compensation formulas, service task |

---

## Key types

```c
typedef struct {
    uint8_t  csPin;       // SPI chip-select GPIO
    uint8_t  intPin;      // DRDY interrupt GPIO
    uint32_t intrType;    // Platform interrupt trigger type
    uint32_t pull;        // Pull resistor: 0=none 1=up 2=down
} BMP390_ServiceCfg_t;

typedef struct {
    void              *p_spiHandle;
    void              *p_eventGroup;
    SPP_GpioIsrCtx_t   isrCtx;
    BMP390_TempParams_t tempParams;
    BMP390_PressParams_t pressParams;
} BMP390_ServiceCtx_t;
```

---

## Registration

```c
extern const SPP_ServiceDesc_t g_bmp390ServiceDesc;

static BMP390_ServiceCtx_t s_bmpCtx;
static BMP390_ServiceCfg_t s_bmpCfg = {
    .csPin    = 18U,
    .intPin   = 5U,
    .intrType = GPIO_INTR_POSEDGE,
    .pull     = 0U,
};

SPP_Service_register(&g_bmp390ServiceDesc, &s_bmpCtx, &s_bmpCfg);
```

---

## Packet payload layout (12 bytes)

| Offset | Type | Field |
|---|---|---|
| 0 | `float` | Altitude (m) |
| 4 | `float` | Pressure (Pa) |
| 8 | `float` | Temperature (°C) |

---

## Hardware configuration (ESP32-S3)

- SPI bus: SPI2_HOST
- CS GPIO: 18
- DRDY interrupt GPIO: configured per application
- ODR: 50 Hz
- IIR filter: coefficient 2
- Over-sampling: default (OSR = 0)
- Chip ID register: `0x00`, expected value `0x60`
