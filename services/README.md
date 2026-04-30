# services/

SPP's service layer. Provides the packet lifecycle infrastructure (databank, pub/sub, logging) and the sensor/logging drivers (BMP390, ICM20948, datalogger). All modules share a common registration interface defined in `service.h`.

---

## Module types

### Infrastructure modules (always compiled)

| Directory | Description |
|---|---|
| `databank/` | Static packet pool — allocates and recycles `SPP_Packet_t` objects |
| `pubsub/` | Priority-aware publish-subscribe router with deferred dispatch via `tick()` |
| `log/` | Level-filtered logging with a swappable output callback |

### Sensor/logger modules (opt-in at build time)

| Directory | CMake flag | Description |
|---|---|---|
| `bmp390/` | `SPP_SERVICE_BMP390` | BMP390 pressure / altitude sensor |
| `icm20948/` | `SPP_SERVICE_ICM20948` | ICM20948 IMU (accelerometer + gyroscope + magnetometer) |
| `datalogger/` | `SPP_SERVICE_DATALOGGER` | SD card packet logger |

---

## Module descriptor and registry

Every module describes itself with a static `SPP_Module_t`:

```c
typedef struct {
    const char   *p_name;        // Human-readable name for logging
    uint16_t      apid;          // APID bitmask produced by this module (single bit, or K_SPP_APID_NONE)
    size_t        ctxSize;       // sizeof(module-private context struct)

    SPP_RetVal_t (*init)       (void *ctx, const void *cfg);
    SPP_RetVal_t (*start)      (void *ctx);
    SPP_RetVal_t (*stop)       (void *ctx);
    SPP_RetVal_t (*deinit)     (void *ctx);
    void         (*serviceTask)(void *ctx);  // called by pollAll() each superloop iteration

    uint16_t             consumesApid;    // APID bitmask this module subscribes to
    SPP_PubSub_Handler_t onPacket;        // auto-registered on SPP_SERVICES_register()
    uint8_t              onPacketPrio;    // K_SPP_PUBSUB_PRIO_CRITICAL … K_SPP_PUBSUB_PRIO_LOW
} SPP_Module_t;
```

Registration pattern in `app_main()`:

```c
static BMP390_ServiceCtx_t  s_bmpCtx;
static BMP390_ServiceCfg_t  s_bmpCfg = { .spiDevIdx = 1U, .intPin = 17U };

// Registration auto-subscribes the module if onPacket != NULL
SPP_SERVICES_register(&g_bmp390Module, &s_bmpCtx, &s_bmpCfg);
SPP_SERVICES_initAll();
SPP_SERVICES_startAll();
```

The registry calls `init` and `start` on each module in registration order. `stop` and `deinit` are called in reverse order on shutdown. No memory is allocated by the registry — all context buffers are caller-managed.

---

## Packet data flow

```
ISR (sets drdyFlag)
  │
  └─► SPP_SERVICES_pollAll()
        │
        └─► module->serviceTask(ctx)   [checks DRDY, returns if not set]
              │
              ├─ SPP_SERVICES_DATABANK_getPacket()
              ├─ SPP_SERVICES_DATABANK_packetData(pkt, apid, seq, data, len)
              └─ SPP_SERVICES_PUBSUB_publish(pkt)
                    │
                    ├─► CRITICAL subscribers — called synchronously
                    └─► enqueued for deferred dispatch via tick()

SPP_SERVICES_PUBSUB_tick()   [call once per superloop iteration]
  └─► dispatches one deferred subscriber (e.g. SD card logger)
```

Sensor modules are **producers only** — they do not know who consumes their packets. Consumers declare `onPacket` in their `SPP_Module_t` and are wired up automatically at registration time.

---

## Pub/sub API

```c
// Subscribe to a specific APID bitmask (K_SPP_APID_ALL for everything)
SPP_SERVICES_PUBSUB_subscribe(K_BMP390_SERVICE_APID, K_SPP_PUBSUB_PRIO_NORMAL, myHandler, &myCtx);

// Publish — dispatches CRITICAL subscribers synchronously, enqueues the rest
SPP_SERVICES_PUBSUB_publish(p_pkt);

// Drain one deferred subscriber per call (call from superloop)
SPP_SERVICES_PUBSUB_tick();

// Read per-APID overflow counter (incremented on queue-full drops)
SPP_SERVICES_PUBSUB_overflowCount(K_ICM20948_SERVICE_APID);
```

### Subscriber priorities

| Constant | Value | Dispatch |
|---|---|---|
| `K_SPP_PUBSUB_PRIO_CRITICAL` | 0 | Synchronous inside `publish()` |
| `K_SPP_PUBSUB_PRIO_HIGH` | 1 | Deferred via `tick()`, first |
| `K_SPP_PUBSUB_PRIO_NORMAL` | 2 | Deferred via `tick()` |
| `K_SPP_PUBSUB_PRIO_LOW` | 3 | Deferred via `tick()`, last |

---

## APID allocation

APIDs are single-bit bitmasks. Subscribers combine bits to match multiple sources.

| APID | Hex | Owner |
|---|---|---|
| `K_SPP_APID_LOG` | `0x0001` | SPP log message packets |
| `K_ICM20948_SERVICE_APID` | `0x0002` | ICM20948 sensor packets |
| `K_BMP390_SERVICE_APID` | `0x0004` | BMP390 sensor packets |
| `K_SPP_APID_NONE` | `0x0000` | No APID (producer-only or consumer-only) |
| `K_SPP_APID_ALL` | `0xFFFF` | Wildcard — matches every packet |

---

## Adding a new module

1. Create `services/mymodule/mymodule.h` and `mymodule.c`
2. Implement `init`, `start`, `stop`, `deinit` callbacks
3. Implement `serviceTask(void *ctx)` if the module is a sensor producer (check DRDY at the top, return immediately if not set)
4. Set `onPacket` and `consumesApid` if the module consumes packets
5. Declare `const SPP_Module_t g_myModule = { ... }` with all fields
6. Add `services/mymodule/mymodule.c` to `CMakeLists.txt`
7. In `app_main()`: call `SPP_SERVICES_register(&g_myModule, &ctx, &cfg)` before `SPP_SERVICES_initAll()`
