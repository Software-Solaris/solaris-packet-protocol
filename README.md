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

There is no OSAL layer. Sensor services run in a bare-metal superloop: an ISR sets a `volatile` flag, the superloop detects the flag and calls the service task, which builds a packet and publishes it. Subscribers (SD logger, antenna encoder, …) register once at startup and are called synchronously during publish.

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

### 1. Register HAL port and initialise

```c
#include "spp/spp.h"

extern const SPP_HalPort_t g_esp32BaremetalHalPort;

void app_main(void)
{
    SPP_CORE_setHalPort(&g_esp32BaremetalHalPort);
    SPP_CORE_init();   // calls SPP_SERVICES_DATABANK_init + SPP_SERVICES_PUBSUB_init internally
}
```

### 2. Subscribe consumers before starting services

```c
// SD card logger subscribes to ALL packets (sensor data + log messages)
SPP_SERVICES_PUBSUB_subscribe(K_SPP_APID_ALL, sdLogHandler, &s_logger);
```

### 3. Register and start services

```c
SPP_SERVICES_register(&g_bmp390ServiceDesc,   &s_bmpCtx, &s_bmpCfg);
SPP_SERVICES_register(&g_icm20948ServiceDesc, &s_icmCtx, &s_icmCfg);
SPP_SERVICES_initAll();
SPP_SERVICES_startAll();
```

### 4. Superloop

```c
for (;;)
{
    if (s_bmpCtx.bmpData.drdyFlag)
        SPP_SERVICES_BMP390_serviceTask(&s_bmpCtx);

    if (s_icmCtx.icmData.drdyFlag)
        SPP_SERVICES_ICM20948_serviceTask(&s_icmCtx);

    // SD card is passive — handled through pub/sub callbacks
}
```

### 5. Typical sensor service data flow

```
ISR sets drdyFlag
  → superloop detects flag
    → ServiceTask reads sensor
      → SPP_SERVICES_DATABANK_getPacket()
      → SPP_SERVICES_DATABANK_packetData()   fills headers + computes CRC
      → SPP_SERVICES_PUBSUB_publish()        dispatches to all subscribers, then returns packet
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

| APID | Owner |
|---|---|
| `0x0001` (`K_SPP_APID_LOG`) | SPP log message packets |
| `0x0101` | BMP390 service |
| `0x0201` | ICM20948 service |
| `0xFFFF` (`K_SPP_APID_ALL`) | Pub/sub wildcard (subscribe to every packet) |

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
2. Implement the four lifecycle callbacks: `init`, `start`, `stop`, `deinit`
3. Declare a `const SPP_ServiceDesc_t g_myServiceDesc` with your APID and callbacks
4. In `ServiceTask()`: `getPacket()` → `packetData()` → `publish()`
5. Add the source to `CMakeLists.txt`

See [`services/README.md`](services/README.md) for a full walkthrough.

---

## Porting to a new platform

Implement `SPP_HalPort_t` (hardware drivers), place the files under `ports/hal/<target>/`, then register at boot with `SPP_CORE_setHalPort()`.

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
