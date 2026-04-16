# ports/

Concrete platform implementations of the HAL and OSAL contracts. HAL and OSAL are **independent axes** — pick one from each and combine them freely.

```
ports/
├── osal/               OS implementations (independent of hardware)
│   ├── freertos/       FreeRTOS tasks, queues, mutexes, event groups
│   ├── baremetal/      Cooperative scheduler — no OS required
│   └── posix/          POSIX pthreads — for host-side unit tests
└── hal/                Hardware implementations (independent of OS)
    ├── esp32/          ESP32-S3 SPI2, GPIO ISR, SD card via FATFS
    └── stub/           No-op stubs returning K_SPP_OK — for host tests
```

---

## Mixing ports

| OSAL | HAL | Use case |
|---|---|---|
| `freertos` | `esp32` | ESP32-S3 in production |
| `baremetal` | `esp32` | ESP32-S3 without FreeRTOS |
| `posix` | `stub` | Host unit tests (no hardware needed) |
| `freertos` | *(your new HAL)* | New MCU, reuse FreeRTOS OSAL |
| *(your new OSAL)* | `esp32` | Alternative OS on ESP32 |

---

## Available OSAL ports

### `osal/freertos/` — FreeRTOS

Maps SPP primitives to FreeRTOS APIs using static allocation where possible:

| SPP | FreeRTOS |
|---|---|
| `taskCreate` | `xTaskCreateStatic` |
| `taskDelayMs` | `vTaskDelay(pdMS_TO_TICKS(ms))` |
| `queueCreate` | `xQueueCreate` |
| `queueSend` | `xQueueSend` |
| `mutexCreate` | `xSemaphoreCreateMutex` |
| `eventCreate` | `xEventGroupCreate` |
| `eventSetFromIsr` | `xEventGroupSetBitsFromISR` |

### `osal/baremetal/` — Cooperative scheduler

Implements a simple round-robin tick loop. Tasks are function pointers called in sequence. `taskDelayMs` busy-loops or yields to the next task depending on build flags.

### `osal/posix/` — POSIX

Uses `pthread_create` for tasks, POSIX semaphores for mutexes, and a condition-variable-based event group implementation. Used exclusively for running the Cgreen unit test suite on a development PC.

---

## Available HAL ports

### `hal/esp32/`

Two variants sharing the same GPIO and SPI pin definitions (`macros_esp32.h`):

| File | Description |
|---|---|
| `hal_esp32.c` | FreeRTOS-aware SPI (DMA, transaction queues) |
| `hal_esp32_baremetal.c` | Polling SPI (no FreeRTOS dependencies) |
| `macros_esp32.h` | Pin definitions: MISO=47, MOSI=38, CLK=48, CS_ICM=21, CS_BMP=18, CS_SDC=8 |

### `hal/stub/`

Every function returns `K_SPP_OK` immediately. Used with `osal/posix/` for host-side unit tests where no hardware is present.

---

## Implementing a new port

### New OSAL port (new OS)

1. Create `ports/osal/<os>/osal_<os>.c`
2. Fill all fields of `SPP_OsalPort_t` with your OS's APIs
3. Declare `const SPP_OsalPort_t g_<os>OsalPort = { ... }`
4. At startup: `SPP_Core_setOsalPort(&g_<os>OsalPort)`
5. Add the source to `CMakeLists.txt`

Minimum checklist:
- [ ] `taskCreate` — create a task with the given stack size and priority
- [ ] `taskDelayMs` — block for at least N milliseconds
- [ ] `getTickMs` — return monotonic ms counter
- [ ] `queueCreate` / `queueSend` / `queueRecv` — FIFO with timeout
- [ ] `mutexCreate` / `mutexLock` / `mutexUnlock`
- [ ] `eventCreate` / `eventWait` / `eventSetFromIsr`

### New HAL port (new MCU)

1. Create `ports/hal/<target>/hal_<target>.c`
2. Fill all mandatory fields of `SPP_HalPort_t`
3. Declare `const SPP_HalPort_t g_<target>HalPort = { ... }`
4. At startup: `SPP_Core_setHalPort(&g_<target>HalPort)`

Minimum checklist:
- [ ] `spiBusInit` / `spiGetHandle` / `spiDeviceInit` / `spiTransmit`
- [ ] `gpioConfigInterrupt` / `gpioRegisterIsr`
- [ ] `getTimeMs`
- [ ] `storageMount` / `storageUnmount` (or leave NULL if no storage)

See `hal/esp32/hal_esp32.c` and `osal/freertos/osal_freertos.c` as complete reference implementations.
