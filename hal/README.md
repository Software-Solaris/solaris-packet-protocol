# hal/

Hardware Abstraction Layer. Defines the **contract** that any hardware port must fulfill — a struct of function pointers (`SPP_HalPort_t`) covering SPI bus management, GPIO interrupts, storage mounting, and system time. SPP core and services never call ESP-IDF or any MCU SDK directly; they go through these abstractions.

`hal/` contains only headers and the thin dispatch layer (`dispatch.c`). The actual implementations live in [`ports/hal/`](../ports/hal/).

---

## Files

| File | Description |
|---|---|
| `port.h` | `SPP_HalPort_t` — the full contract struct with all function pointer signatures |
| `spi.h` | `SPP_HAL_spiBusInit()`, `SPP_HAL_spiGetHandle()`, `SPP_HAL_spiDeviceInit()`, `SPP_HAL_spiTransmit()` |
| `gpio.h` | `SPP_HAL_gpioConfigInterrupt()`, `SPP_HAL_gpioRegisterIsr()` and `SPP_GpioIsrCtx_t` |
| `storage.h` | `SPP_HAL_storageMount()`, `SPP_HAL_storageUnmount()` |
| `time.h` | `SPP_HAL_getTimeMs()` — monotonic millisecond counter |
| `dispatch.c` | Routes every `SPP_HAL_*()` call through the port registered via `SPP_Core_setHalPort()` |

---

## The port contract

```c
typedef struct {
    // SPI bus
    SPP_RetVal_t  (*spiBusInit)(void);
    void         *(*spiGetHandle)(spp_uint8_t deviceIdx);
    SPP_RetVal_t  (*spiDeviceInit)(void *p_handle);
    SPP_RetVal_t  (*spiTransmit)(void *p_handle, spp_uint8_t *p_data, spp_uint8_t len);

    // GPIO interrupts
    SPP_RetVal_t  (*gpioConfigInterrupt)(spp_uint32_t pin, spp_uint32_t intrType, spp_uint32_t pull);
    SPP_RetVal_t  (*gpioRegisterIsr)(spp_uint32_t pin, void *p_isrCtx);

    // Storage (optional — may be NULL if unused)
    SPP_RetVal_t  (*storageMount)(void *p_cfg);
    SPP_RetVal_t  (*storageUnmount)(void *p_cfg);

    // Time
    spp_uint32_t  (*getTimeMs)(void);
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

Sensor services that use data-ready interrupts pass an `SPP_GpioIsrCtx_t` to `gpioRegisterIsr()`. The ISR sets a `volatile` flag; the superloop polls it. No RTOS primitives are involved.

```c
SPP_GpioIsrCtx_t isrCtx = {
    .p_flag = &myData.drdyFlag,  // volatile spp_bool_t
};
SPP_HAL_gpioRegisterIsr(INT_PIN, &isrCtx);

// In the superloop:
if (myData.drdyFlag)
{
    MyService_ServiceTask(&s_ctx);
}
```

---

## Available ports

| Port | Location | Notes |
|---|---|---|
| ESP32 | `ports/hal/esp32/hal_esp32.c` | Polling SPI, no FreeRTOS dependency — primary target |
| Stub | `ports/hal/stub/hal_stub.c` | No-op, always returns `K_SPP_OK` — for host tests |

---

## Implementing a new HAL port

1. Create `ports/hal/<target>/hal_<target>.c`
2. Implement every non-optional field in `SPP_HalPort_t`
3. Declare a `const SPP_HalPort_t g_<target>HalPort = { ... }`
4. Register it at startup: `SPP_Core_setHalPort(&g_<target>HalPort)`

See `ports/hal/esp32/hal_esp32.c` as the reference implementation.
