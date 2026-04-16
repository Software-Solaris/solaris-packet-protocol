# hal/

Hardware Abstraction Layer. Defines the **contract** that any hardware port must fulfill — a struct of function pointers (`SPP_HalPort_t`) covering SPI bus management, GPIO interrupts, storage mounting, and system time. SPP core and services never call ESP-IDF or any MCU SDK directly; they go through these abstractions.

`hal/` contains only headers and the thin dispatch layer (`dispatch.c`). The actual implementations live in [`ports/hal/`](../ports/hal/).

---

## Files

| File | Description |
|---|---|
| `port.h` | `SPP_HalPort_t` — the full contract struct with all function pointer signatures |
| `spi.h` | `SPP_Hal_spiBusInit()`, `SPP_Hal_spiGetHandle()`, `SPP_Hal_spiDeviceInit()`, `SPP_Hal_spiTransmit()` |
| `gpio.h` | `SPP_Hal_gpioConfigInterrupt()`, `SPP_Hal_gpioRegisterIsr()` and `SPP_GpioIsrCtx_t` |
| `storage.h` | `SPP_Hal_storageMount()`, `SPP_Hal_storageUnmount()` |
| `dispatch.c` | Routes every `SPP_Hal_*()` call through the port registered via `SPP_Core_setHalPort()` |

---

## The port contract

```c
typedef struct {
    // SPI bus
    SPP_RetVal_t  (*spiBusInit)(void);
    void         *(*spiGetHandle)(uint8_t deviceIdx);
    SPP_RetVal_t  (*spiDeviceInit)(void *handle);
    SPP_RetVal_t  (*spiTransmit)(void *handle, uint8_t *data, uint8_t len);

    // GPIO interrupts
    SPP_RetVal_t  (*gpioConfigInterrupt)(uint32_t pin, uint32_t intrType, uint32_t pull);
    SPP_RetVal_t  (*gpioRegisterIsr)(uint32_t pin, void *isrCtx);

    // Storage (optional — may be NULL if unused)
    SPP_RetVal_t  (*storageMount)(void *cfg);
    SPP_RetVal_t  (*storageUnmount)(void *cfg);

    // Time
    uint32_t      (*getTimeMs)(void);
} SPP_HalPort_t;
```

`storageMount` and `storageUnmount` are the only optional fields — set them to NULL if your target has no SD card.

---

## SPI device indexing

The ESP32 port maps device indices as follows:

| Index | Device |
|---|---|
| 0 | ICM20948 IMU (CS GPIO 21, 1 MHz, MODE0) |
| 1 | BMP390 pressure sensor (CS GPIO 18, 500 kHz, MODE0) |
| 2 | SD card (CS GPIO 8) |

Your port defines its own mapping inside `spiGetHandle()`.

---

## GPIO ISR context

Sensor drivers that use data-ready interrupts pass an `SPP_GpioIsrCtx_t` to `gpioRegisterIsr()`. The ISR sets the event group bits, waking the waiting sensor task:

```c
SPP_GpioIsrCtx_t isrCtx = {
    .p_eventGroup = p_eventGroup,
    .bits         = K_DRDY_BIT,
};
SPP_Hal_gpioRegisterIsr(BMP390_INT_PIN, &isrCtx);

// In the sensor task:
SPP_Osal_eventWait(p_eventGroup, K_DRDY_BIT, true, false, 5000U, NULL);
```

---

## Available ports

| Port | Location | Notes |
|---|---|---|
| ESP32 (FreeRTOS) | `ports/hal/esp32/hal_esp32.c` | SPI2_HOST, DMA, FreeRTOS-aware |
| ESP32 (baremetal) | `ports/hal/esp32/hal_esp32_baremetal.c` | Polling SPI, no FreeRTOS dependency |
| Stub | `ports/hal/stub/hal_stub.c` | No-op, always returns `K_SPP_OK` — for host tests |

---

## Implementing a new HAL port

1. Create `ports/hal/<target>/hal_<target>.c`
2. Implement every non-optional field in `SPP_HalPort_t`
3. Declare a `const SPP_HalPort_t g_<target>HalPort = { ... }`
4. Register it at startup: `SPP_Core_setHalPort(&g_<target>HalPort)`

See `ports/hal/esp32/hal_esp32.c` as the reference implementation.
