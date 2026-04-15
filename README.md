# Solaris Packet Protocol (SPP)

SPP is a small, portable C library for packaging and routing sensor data as structured packets. It has no dynamic memory allocation (`malloc` is never called) — all buffers are statically declared at compile time. The same core library runs on an ESP32 microcontroller and on a standard PC (for unit tests), because hardware and OS details are hidden behind two thin abstraction layers: HAL and OSAL.

---

## Directory layout

```
spp/
├── include/spp/          # Public headers — everything you #include
│   ├── core/             # Packet format, portable types, error codes
│   ├── services/         # Databank, DB Flow, logging, sensor service headers
│   ├── hal/              # Hardware interface contracts (SPI, GPIO, storage)
│   ├── osal/             # OS interface contracts (tasks, queues, mutexes, events)
│   └── util/             # CRC16, compile-time macros, structof helper
├── src/                  # Implementations of core, services, and dispatch glue
├── ports/                # Concrete implementations per platform
│   ├── osal/freertos/    # FreeRTOS OSAL port (tasks, queues, event groups)
│   ├── osal/baremetal/   # Cooperative scheduler port (no RTOS required)
│   └── hal/esp32/        # ESP32 SPI bus, GPIO interrupts, SD card storage
├── services/             # Optional sensor and logging services
│   ├── bmp390/           # BMP390 pressure sensor service
│   ├── icm20948/         # ICM20948 IMU service
│   └── datalogger/       # SD card datalogger service
└── tests/                # Unit tests — run on a PC, no hardware needed
```

---

## Layer diagram

```
  ┌──────────────────────────────────────────────────────┐
  │                    Application                       │
  │                  main/main.c                         │
  ├──────────────────────────────────────────────────────┤
  │                     Services                         │
  │   BMP390 · ICM20948 · Datalogger  (sensor services)  │
  │   Databank · DB Flow · Logging    (SPP services)     │
  ├──────────────────────────────────────────────────────┤
  │                     SPP Core                         │
  │           packet · types · returntypes               │
  ├────────────────────────┬─────────────────────────────┤
  │          HAL           │           OSAL              │
  │  SPI · GPIO · Storage  │  Tasks · Queues · Events    │
  │  (interface only)      │  (interface only)           │
  ├────────────────────────┴─────────────────────────────┤
  │              Platform ports                          │
  │  ports/hal/esp32/         ports/osal/freertos/       │
  │  ports/hal/esp32_bm/      ports/osal/baremetal/      │
  └──────────────────────────────────────────────────────┘
           Hardware: ESP32-S3 · SPI bus · FreeRTOS
```

---

## Layer explanations

### Core

Core defines `SPP_Packet_t`, the single data container used everywhere in the stack. A packet has a primary header (APID identifying the source, sequence number, payload length), a secondary header (timestamp, drop counter), up to 48 bytes of payload, and a CRC16 for integrity checking. Portable integer types (`spp_uint8_t`, `spp_int32_t`, etc.) and the `SPP_RetVal_t` error code enum are also defined here.

### Services

Three built-in SPP services handle packet lifecycle:

- **Databank**: a fixed pool of 5 packets. Call `SPP_Databank_getPacket()` to lease one, fill its payload, and call `SPP_Databank_returnPacket()` when done. No `malloc` is ever called.
- **DB Flow**: a 16-slot circular FIFO of packet pointers. Sensor tasks push with `SPP_DbFlow_pushReady()`; the logger task pops with `SPP_DbFlow_popReady()`.
- **Logging**: level-filtered log macros `SPP_LOGI`, `SPP_LOGW`, `SPP_LOGE` that route output through a registered callback — making it easy to redirect logs to UART, a file, or a test buffer.

Sensor services (BMP390, ICM20948, Datalogger) live in `services/` and use the Databank and DB Flow to produce and consume packets.

### HAL

HAL defines *what* hardware operations exist (SPI bus init, GPIO interrupt config, SD card mount) but contains no implementation. The headers in `include/spp/hal/` describe the `SPP_HalPort_t` struct — a table of function pointers. The concrete implementations live in `ports/hal/`.

### OSAL

OSAL does the same for OS primitives. `include/spp/osal/` defines `SPP_OsalPort_t` — function pointers for task creation, queue send/receive, mutex lock/unlock, and event group operations. The core and services call only these wrappers, so you can swap FreeRTOS for a cooperative scheduler without touching any other code.

### Ports

A port fills in `SPP_HalPort_t` and `SPP_OsalPort_t` with real function implementations for a specific target. For ESP32 + FreeRTOS, the port is split across:

- `ports/hal/esp32/hal_esp32.c` — SPI2_HOST driver, GPIO ISR service, FATFS SD card
- `ports/osal/freertos/osal_freertos.c` — `xTaskCreateStatic`, `xQueueCreate`, `xEventGroupCreate`, etc.

See `ports/freertos/` as a working reference when porting to a new platform.

---

## Typical data flow

### 1. Boot — register ports and initialise core

```c
// Register the platform implementations
SPP_Core_setOsalPort(&g_freertosOsalPort);
SPP_Core_setHalPort(&g_esp32HalPort);

// Initialise the stack (logging + databank)
SPP_Core_init();
SPP_Databank_init();
SPP_DbFlow_init();
SPP_Hal_spiBusInit();
```

### 2. Sensor task — acquire packet, fill payload, push to FIFO

```c
SPP_Packet_t *p_pkt = SPP_Databank_getPacket();
if (p_pkt == NULL) { /* pool exhausted, drop reading */ return; }

p_pkt->primaryHeader.apid       = 0x0101;
p_pkt->primaryHeader.seq        = s_seq++;
p_pkt->secondaryHeader.timestampMs = SPP_Hal_getTimeMs();
p_pkt->primaryHeader.payloadLen = 12U;

// Write 3 floats (altitude, pressure, temperature) into the payload
memcpy(p_pkt->payload, &altitude, 4U);
memcpy(p_pkt->payload + 4U, &pressure, 4U);
memcpy(p_pkt->payload + 8U, &temperature, 4U);

SPP_DbFlow_pushReady(p_pkt);
```

### 3. Logger task — pop packet, log to SD, return to pool

```c
SPP_Packet_t *p_pkt = NULL;
SPP_DbFlow_popReady(&p_pkt);

DATALOGGER_LogPacket(&s_logger, p_pkt);

SPP_Databank_returnPacket(p_pkt);
```

---

## Adding a new sensor service

### Step 1 — Create the service files

```
services/
└── mysensor/
    ├── include/spp/services/mysensor.h    # public API
    └── src/myensor_service.c             # implementation
```

### Step 2 — Add a CMake option in `compiler/spp/CMakeLists.txt`

```cmake
option(SPP_SERVICE_MYsensor "Enable MySensor service" ON)

if(SPP_SERVICE_MYENSOR)
    list(APPEND SPP_SOURCES
        "${SPP_ROOT}/services/myensor/src/myensor_service.c"
    )
    list(APPEND SPP_INCLUDE_DIRS
        "${SPP_ROOT}/services/myensor/include"
    )
endif()
```

### Step 3 — Implement the service descriptor and register it

```c
// myensor_service.c
static retval_t myensor_init(void *p_ctx, const void *p_cfg) { /* ... */ return K_SPP_OK; }
static retval_t myensor_start(void *p_ctx)                   { /* ... */ return K_SPP_OK; }
static retval_t myensor_stop(void *p_ctx)                    { /* ... */ return K_SPP_OK; }

const SPP_ServiceDesc_t g_mysensorServiceDesc = {
    .p_name  = "myensor",
    .apid    = 0x0102,
    .ctxSize = sizeof(MySensor_Ctx_t),
    .init    = myensor_init,
    .start   = myensor_start,
    .stop    = myensor_stop,
    .deinit  = NULL,
};
```

Then in `main.c`:

```c
static MySensor_Ctx_t  s_mysensorCtx;
static MySensor_Cfg_t  s_mysensorCfg = { .csPin = 10U };
SPP_Service_register(&g_mysensorServiceDesc, &s_mysensorCtx, &s_mysensorCfg);
SPP_Service_startAll();
```

---

## Porting to a new platform

You only need to provide two structs full of function pointers: `SPP_OsalPort_t` (OS primitives) and `SPP_HalPort_t` (hardware drivers). Fill each field with a function from your target's SDK, then register them at boot:

```c
static const SPP_OsalPort_t g_myOsalPort = {
    .taskCreate   = myos_task_create,
    .taskDelayMs  = myos_delay_ms,
    .queueCreate  = myos_queue_create,
    .queueSend    = myos_queue_send,
    .queueRecv    = myos_queue_recv,
    // ... fill in the remaining fields
};

static const SPP_HalPort_t g_myHalPort = {
    .spiBusInit        = mymcu_spi_init,
    .spiTransmit       = mymcu_spi_transmit,
    .gpioConfigInterrupt = mymcu_gpio_config,
    // ... fill in the remaining fields
};

SPP_Core_setOsalPort(&g_myOsalPort);
SPP_Core_setHalPort(&g_myHalPort);
```

See `ports/osal/freertos/osal_freertos.c` and `ports/hal/esp32/hal_esp32.c` for a complete working example. The unit tests in `tests/` use the POSIX port (`ports/osal/posix/`) and run on any PC without hardware — a good starting point to verify your OSAL port before flashing anything.
