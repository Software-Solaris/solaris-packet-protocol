# services/bmp390/

BMP390 barometric pressure and temperature sensor service. Waits for the DRDY interrupt (via a `volatile` flag set by the GPIO ISR), reads raw ADC data over SPI, compensates it using the sensor's factory calibration, and publishes altitude / pressure / temperature packets via pub/sub.

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
    spp_uint8_t  spiDevIdx;   // SPI device index (1 = BMP390 on ESP32 port)
    spp_uint8_t  intPin;      // DRDY interrupt GPIO
    spp_uint32_t intIntrType; // Platform interrupt trigger type (rising edge = 1)
    spp_uint32_t intPull;     // Pull resistor: 0=none 1=up 2=down
} BMP390_ServiceCfg_t;

typedef struct {
    void                *p_spi;
    BMP390_Data_t        bmpData;   // drdyFlag + ISR context
    BMP390_TempParams_t  tempParams;
    BMP390_PressParams_t pressParams;
    spp_uint16_t         seq;
} BMP390_ServiceCtx_t;
```

`BMP390_Data_t` contains a `volatile spp_bool_t drdyFlag` that the GPIO ISR sets. The superloop polls this flag and calls `BMP390_ServiceTask()`.

---

## Registration

```c
extern const SPP_ServiceDesc_t g_bmp390ServiceDesc;

static BMP390_ServiceCtx_t s_bmpCtx;
static BMP390_ServiceCfg_t s_bmpCfg = {
    .spiDevIdx   = 1U,
    .intPin      = 5U,
    .intIntrType = 1U,   // rising edge
    .intPull     = 0U,   // no pull
};

SPP_Service_register(&g_bmp390ServiceDesc, &s_bmpCtx, &s_bmpCfg);
```

---

## Superloop integration

```c
// In the application superloop:
if (s_bmpCtx.bmpData.drdyFlag)
{
    BMP390_ServiceTask(&s_bmpCtx);
}
```

`BMP390_ServiceTask()` clears `drdyFlag`, reads the sensor FIFO, calls `SPP_Databank_getPacket()` → `SPP_Databank_packetData()` → `SPP_PubSub_publish()`.

---

## Packet payload layout (12 bytes)

| Offset | Type | Field |
|---|---|---|
| 0 | `float` | Altitude (m) |
| 4 | `float` | Pressure (Pa) |
| 8 | `float` | Temperature (°C) |

---

## Hardware configuration (ESP32-S3)

- SPI device index: 1 (CS GPIO 18, 500 kHz, MODE0)
- DRDY interrupt GPIO: 5 (configured via `BMP390_ServiceCfg_t`)
- ODR: 50 Hz
- IIR filter: coefficient 2
- Over-sampling: default (OSR = 0)
- Chip ID register: `0x00`, expected value `0x60`
