# Solaris Packet Protocol (SPP)

SPP is a small, portable C11 library for packaging and routing sensor data as structured packets on embedded systems. It has **no dynamic memory allocation** — all buffers are statically declared at compile time. The same core library runs on an ESP32-S3 microcontroller and on a standard Linux PC (for unit tests), because hardware and OS details are hidden behind two thin abstraction layers: **HAL** and **OSAL**.

---

## Architecture

```
  ┌──────────────────────────────────────────────────────────┐
  │                      Application                         │
  │                    main / user code                      │
  ├──────────────────────────────────────────────────────────┤
  │                  Sensor Services                         │
  │        bmp390 · icm20948 · datalogger                    │
  ├──────────────────────────────────────────────────────────┤
  │                  SPP Services                            │
  │      databank · db_flow · log · service registry         │
  ├────────────────────────┬─────────────────────────────────┤
  │          HAL           │             OSAL                │
  │  SPI · GPIO · Storage  │  Tasks · Queues · Events        │
  │  (contracts only)      │  (contracts only)               │
  ├────────────────────────┴─────────────────────────────────┤
  │                  Platform Ports                          │
  │   ports/hal/esp32/        ports/osal/freertos/           │
  │   ports/hal/stub/         ports/osal/baremetal/          │
  │                           ports/osal/posix/              │
  └──────────────────────────────────────────────────────────┘
```

HAL and OSAL are **orthogonal axes** — they are independent and mix freely. The ESP32 HAL works with the FreeRTOS OSAL, the baremetal OSAL, or any other OSAL. Porting to a new MCU means implementing the HAL for it; porting to a new OS means implementing the OSAL for it. Neither affects the other.

---

## Directory layout

```
spp/
├── core/           Packet format, portable types, return codes, core init
├── osal/           OS abstraction contract (tasks, queues, mutexes, events)
├── hal/            Hardware abstraction contract (SPI, GPIO, storage)
├── services/       Packet lifecycle services and sensor drivers
│   ├── databank/   Static packet pool (producer side)
│   ├── db_flow/    Circular FIFO routing packets to consumers
│   ├── log/        Level-filtered logging with swappable output
│   ├── bmp390/     BMP390 pressure / altitude sensor service
│   ├── icm20948/   ICM20948 IMU service (accel + gyro + DMP)
│   └── datalogger/ SD card packet logger service
├── util/           CRC-16, compile-time flags, structof macro
├── ports/          Concrete platform implementations
│   ├── osal/
│   │   ├── freertos/   FreeRTOS OSAL port
│   │   ├── baremetal/  Cooperative scheduler OSAL (no OS required)
│   │   └── posix/      POSIX/Linux OSAL (for host unit tests)
│   └── hal/
│       ├── esp32/      ESP32-S3 SPI, GPIO, SD card HAL
│       └── stub/       No-op HAL stub (for host unit tests)
└── tests/          Cgreen unit tests — run on a PC, no hardware needed
    ├── core/
    ├── services/
    └── util/
```

Each module directory contains its header(s) and source(s) together. There is no separate `include/` or `src/` tree.

---

## Include path

SPP uses `spp/` as a namespace prefix in all `#include` directives:

```c
#include "spp/core/packet.h"
#include "spp/services/databank/databank.h"
#include "spp/osal/port.h"
```

Set your include root to the **parent directory of `spp/`**:

```cmake
# In CMakeLists.txt (standalone):
target_include_directories(my_target PRIVATE path/to/parent_of_spp)

# The ESP-IDF wrappers in compiler/ handle this automatically.
```

Or include everything via the umbrella header:

```c
#include "spp/spp.h"
```

---

## Quick start

### 1. Register ports and initialise

```c
#include "spp/spp.h"

// Defined in your chosen port files
extern const SPP_OsalPort_t g_freertosOsalPort;
extern const SPP_HalPort_t  g_esp32HalPort;

void app_main(void)
{
    SPP_Core_setOsalPort(&g_freertosOsalPort);
    SPP_Core_setHalPort(&g_esp32HalPort);
    SPP_Core_init();

    SPP_Databank_init();
    SPP_DbFlow_init();
    SPP_Hal_spiBusInit();
}
```

### 2. Register and start services

```c
extern const SPP_ServiceDesc_t g_bmp390ServiceDesc;

static BMP390_ServiceCtx_t s_bmpCtx;
static BMP390_ServiceCfg_t s_bmpCfg = { .csPin = 18U, .intPin = 5U };

SPP_Service_register(&g_bmp390ServiceDesc, &s_bmpCtx, &s_bmpCfg);
SPP_Service_startAll();
```

### 3. Typical sensor task data flow

```
getPacket() → fill payload → pushReady()    [producer task]
                                  ↓
                             db_flow FIFO
                                  ↓
popReady() → process/log → returnPacket()   [consumer task]
```

---

## Packet format

Every piece of data in SPP is carried in an `SPP_Packet_t`:

| Field | Size | Description |
|---|---|---|
| `primaryHeader.version` | 1 B | Protocol version (= 1) |
| `primaryHeader.apid` | 2 B | Source identifier |
| `primaryHeader.seq` | 2 B | Sequence counter |
| `primaryHeader.payloadLen` | 2 B | Payload length in bytes |
| `secondaryHeader.timestampMs` | 4 B | Creation time (ms) |
| `secondaryHeader.dropCounter` | 1 B | Packets dropped since reset |
| `payload` | 0–48 B | Raw data |
| `crc` | 2 B | CRC-16/CCITT (0 = not computed) |

---

## Building

### Standalone (host / unit tests)

```bash
cmake -S . -B build -DSPP_BUILD_TESTS=ON -DSPP_PORT=posix
cmake --build build
ctest --test-dir build --output-on-failure
```

### ESP-IDF (via component wrappers)

The `compiler/spp` and `compiler/spp_ports` ESP-IDF components handle the build automatically. Control services and ports at build time:

```bash
idf.py build \
  -DSPP_SERVICE_BMP390=ON \
  -DSPP_SERVICE_ICM20948=ON \
  -DSPP_SERVICE_DATALOGGER=ON \
  -DSPP_OSAL_FREERTOS=ON \
  -DSPP_HAL_ESP32=ON
```

---

## Adding a new service

1. Create `services/myservice/myservice.h` and `myservice.c`
2. Implement the four lifecycle callbacks: `init`, `start`, `stop`, `deinit`
3. Declare a `const SPP_ServiceDesc_t g_myServiceDesc` with your APID and callbacks
4. Add the source to `CMakeLists.txt` (and to `compiler/spp/CMakeLists.txt` for ESP-IDF)

See [`services/README.md`](services/README.md) for a full walkthrough.

---

## Porting to a new platform

Implement `SPP_OsalPort_t` (OS primitives) and/or `SPP_HalPort_t` (hardware drivers), place the files under `ports/osal/<os>/` or `ports/hal/<target>/`, then register them at boot.

See [`ports/README.md`](ports/README.md) for a step-by-step guide.

---

## Running unit tests

```bash
# From the spp/ directory
cmake -S . -B build -DSPP_BUILD_TESTS=ON -DSPP_PORT=posix
cmake --build build
ctest --test-dir build --output-on-failure

# Or from the devcontainer terminal:
run_tests solaris-v1/spp
```

See [`tests/README.md`](tests/README.md) for details.
