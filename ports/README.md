# ports/

Concrete platform implementations of the HAL contract. Each port implements `SPP_HalPort_t` for a specific MCU or host environment.

```
ports/
└── hal/                Hardware implementations
    ├── esp32/          ESP32-S3 SPI2, GPIO ISR, SD card via FATFS
    └── stub/           No-op stubs returning K_SPP_OK — for host tests
```

There is no OSAL layer. SPP runs in a bare-metal superloop — ISRs set `volatile` flags, the superloop polls them. No tasks, queues, or event groups are needed.

---

## Available HAL ports

### `hal/esp32/`

| File | Description |
|---|---|
| `halEsp32.c` | Polling SPI (`spi_device_polling_transmit`), no FreeRTOS dependencies — primary target |
| `macrosEsp32.h` | Pin definitions: MISO=47, MOSI=38, CLK=48, CS_ICM=21, CS_BMP=18, CS_SDC=8 |

Exports: `const SPP_HalPort_t g_esp32HalPort`

Register at startup:
```c
SPP_CORE_setHalPort(&g_esp32HalPort);
```

### `hal/stub/`

Every function returns `K_SPP_OK` immediately. Used for host-side unit tests where no hardware is present.

Exports: `const SPP_HalPort_t g_stubHalPort`

---

## Implementing a new HAL port

### New HAL port (new MCU)

1. Create `ports/hal/<target>/hal_<target>.c`
2. Fill all mandatory fields of `SPP_HalPort_t`
3. Declare `const SPP_HalPort_t g_<target>HalPort = { ... }`
4. At startup: `SPP_CORE_setHalPort(&g_<target>HalPort)`

Minimum checklist:
- [ ] `spiBusInit` / `spiGetHandle` / `spiDeviceInit` / `spiTransmit`
- [ ] `gpioConfigInterrupt` / `gpioRegisterIsr`
- [ ] `getTimeMs`
- [ ] `storageMount` / `storageUnmount` (or leave NULL if no storage)

See `hal/esp32/halEsp32.c` as the complete reference implementation.

---

## GPIO ISR pattern

The ISR sets a `volatile` flag through `SPP_GpioIsrCtx_t`:

```c
typedef struct {
    volatile spp_bool_t *p_flag;
} SPP_GpioIsrCtx_t;
```

The port ISR handler reads `p_flag` from the context pointer and sets `*p_flag = true`. No RTOS calls. No yield. The superloop detects the flag on the next iteration.
