# Solaris Packet Protocol (SPP)

SPP is a small, portable C11 library for packaging and routing sensor data as structured packets on embedded systems. It has **no dynamic memory allocation** and **no OS dependency** — all buffers are statically declared at compile time. The same core library runs on an ESP32-S3 microcontroller and on a standard Linux PC (for unit tests), because hardware details are hidden behind a thin **HAL** abstraction.

---

## Architecture

```
  ┌──────────────────────────────────────────────────────────┐
  │                      Application                         │
  │         superloop / bare-metal main / user code          │
  ├──────────────────────────────────────────────────────────┤
  │                  Sensor Services                         │
  │        bmp390 · icm20948 · datalogger                    │
  ├──────────────────────────────────────────────────────────┤
  │                  SPP Services                            │
  │      databank · pubsub · log · service registry          │
  ├──────────────────────────────────────────────────────────┤
  │                       HAL                                │
  │           SPI · GPIO · Storage · Time                    │
  │               (contract only)                            │
  ├──────────────────────────────────────────────────────────┤
  │                  Platform Ports                          │
  │   ports/hal/esp32/        ports/hal/stub/                │
  └──────────────────────────────────────────────────────────┘
```

There is no OSAL layer. Sensor services run in a bare-metal superloop: an ISR sets a `volatile` flag, `callProducers()` calls each module's `produce` callback, which builds a packet and publishes it. SYNC subscribers are called synchronously inside `publish()`; other subscribers are deferred and dispatched one-per-call via `callConsumers()`.

---

## Directory layout

```
spp/
├── core/           Packet format, portable types, return codes, core init
├── hal/            Hardware abstraction contract (SPI, GPIO, storage, time)
├── services/       Packet lifecycle services and sensor drivers
│   ├── databank/   Static packet pool (get / return)
│   ├── pubsub/     Synchronous publish-subscribe router
│   ├── log/        Level-filtered logging with swappable output
│   ├── bmp390/     BMP390 pressure / altitude sensor service
│   ├── icm20948/   ICM20948 IMU service (accel + gyro + mag)
│   └── datalogger/ SD card packet logger service
├── util/           CRC-16, compile-time flags, structof macro
├── ports/          Concrete platform implementations
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
#include "spp/services/pubsub/pubsub.h"
```

Set your include root to the **parent directory of `spp/`**:

```cmake
target_include_directories(my_target PRIVATE path/to/parent_of_spp)
```

Or include everything via the umbrella header:

```c
#include "spp/spp.h"
```

---

## Quick start

### 1. Boot

```c
#include "spp/spp.h"

extern const SPP_HalPort_t g_esp32HalPort;

void app_main(void)
{
    // Registers HAL port, inits databank + pub/sub, wires log output to pub/sub
    SPP_CORE_boot(&g_esp32HalPort);
}
```

### 2. Register services

```c
static ICM20948_t s_icm = { .spiDevIdx=0U, .intPin=10U, .intIntrType=1U, .intPull=0U };
static BMP390_t   s_bmp = { .spiDevIdx=1U, .intPin=17U, .intIntrType=1U, .intPull=0U };

// register() calls init + start immediately; auto-subscribes if the module has an onPacket handler
SPP_SERVICES_register(&g_icm20948Module, &s_icm);
SPP_SERVICES_register(&g_bmp390Module,   &s_bmp);
```

### 3. Superloop

```c
for (;;)
{
    // Calls each module's produce callback; each checks its own DRDY flag and returns immediately if not set
    SPP_SERVICES_callProducers();

    // Dispatches one deferred subscriber per call
    SPP_SERVICES_callConsumers();
}
```

### 4. Sensor service data flow

```
ISR sets drdyFlag
  → SPP_SERVICES_callProducers() → module->produce(ctx)
      → SPP_SERVICES_DATABANK_getPacket()
      → SPP_SERVICES_DATABANK_packetData()   fills headers + computes CRC
      → SPP_SERVICES_PUBSUB_publish()
            → SYNC subscribers called synchronously
            → rest enqueued for SPP_SERVICES_callConsumers()
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
| `crc` | 2 B | CRC-16/CCITT over full packet (0 = not computed) |

---

## Reserved APIDs

APIDs are single-bit bitmasks. Subscribers combine bits to match multiple sources.

| APID | Hex | Owner |
|---|---|---|
| `K_SPP_APID_LOG` | `0x0001` | SPP log message packets |
| `K_ICM20948_SERVICE_APID` | `0x0002` | ICM20948 sensor packets |
| `K_BMP390_SERVICE_APID` | `0x0004` | BMP390 sensor packets |
| `K_SPP_APID_NONE` | `0x0000` | No APID |
| `K_SPP_APID_ALL` | `0xFFFF` | Pub/sub wildcard (matches every packet) |

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
  -DSPP_HAL_ESP32_BM=ON
```

---

## Adding a new service

1. Create `services/myservice/myservice.h` and `myservice.c`
2. Define a single `MyService_t` struct with config fields (set at declaration) + runtime fields (filled by `init`)
3. Implement `init`, `start`, `stop`, `deinit` as static callbacks; `init` reads config from the struct directly
4. Implement `produce(void *ctx)` if the module is a sensor producer
5. Declare `const SPP_Module_t g_myModule = { ... }` with all fields
6. Add the source to `CMakeLists.txt`
7. In `app_main()`: `static MyService_t s_ctx = { /* config */ }; SPP_SERVICES_register(&g_myModule, &s_ctx);`

See [`services/README.md`](services/README.md) for a full walkthrough.

---

## Porting to a new platform

Implement `SPP_HalPort_t` (hardware drivers), place the files under `ports/hal/<target>/`, then pass your port to `SPP_CORE_boot()`.

See [`ports/README.md`](ports/README.md) for a step-by-step guide.

---
