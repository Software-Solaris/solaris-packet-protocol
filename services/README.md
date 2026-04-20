# services/

SPP's service layer. Provides the packet lifecycle infrastructure (databank, pub/sub, logging) and the sensor/logging drivers (BMP390, ICM20948, datalogger). All services share a common registration interface defined in `service.h`.

---

## Service types

### Infrastructure services (always compiled)

| Directory | Description |
|---|---|
| `databank/` | Static packet pool — allocates and recycles `SPP_Packet_t` objects |
| `pubsub/` | Synchronous publish-subscribe router — dispatches packets to registered subscribers |
| `log/` | Level-filtered logging with a swappable output callback |

### Sensor services (opt-in at build time)

| Directory | CMake flag | Description |
|---|---|---|
| `bmp390/` | `SPP_SERVICE_BMP390` | BMP390 pressure / altitude sensor |
| `icm20948/` | `SPP_SERVICE_ICM20948` | ICM20948 IMU (accelerometer + gyroscope + magnetometer) |
| `datalogger/` | `SPP_SERVICE_DATALOGGER` | SD card packet logger |

---

## Service descriptor and registry

Every service describes itself with a static `SPP_ServiceDesc_t`:

```c
typedef struct {
    const char   *p_name;   // Human-readable name for logging
    uint16_t      apid;     // Unique Application Process ID
    size_t        ctxSize;  // sizeof(service-private context struct)

    SPP_RetVal_t (*init)  (void *ctx, const void *cfg);
    SPP_RetVal_t (*start) (void *ctx);
    SPP_RetVal_t (*stop)  (void *ctx);
    SPP_RetVal_t (*deinit)(void *ctx);
} SPP_ServiceDesc_t;
```

Registration pattern in `app_main()`:

```c
static BMP390_ServiceCtx_t  s_bmpCtx;
static BMP390_ServiceCfg_t  s_bmpCfg = { .spiDevIdx = 1U, .intPin = 5U };

SPP_SERVICES_register(&g_bmp390ServiceDesc, &s_bmpCtx, &s_bmpCfg);
SPP_SERVICES_initAll();
SPP_SERVICES_startAll();
```

The registry calls `init` and `start` on each service in registration order. `stop` and `deinit` are called in reverse order on shutdown. No memory is allocated by the registry — all context buffers are caller-managed.

---

## Packet data flow

```
ISR (sets drdyFlag)
  │
  └─► superloop detects flag
        │
        └─► ServiceTask()
              │
              ├─ SPP_SERVICES_DATABANK_getPacket()
              ├─ SPP_SERVICES_DATABANK_packetData(pkt, apid, seq, data, len)
              │     fills headers, copies payload, computes CRC-16
              └─ SPP_SERVICES_PUBSUB_publish(pkt)
                    │
                    ├─► subscriber A (SD card logger)
                    ├─► subscriber B (antenna encoder)
                    └─► SPP_SERVICES_DATABANK_returnPacket(pkt)  ← automatic
```

Sensor services are **producers only** — they do not know who consumes their packets. Consumers register as subscribers at startup.

---

## Pub/sub API

```c
// Subscribe to a specific APID
SPP_SERVICES_PUBSUB_subscribe(K_BMP_SERVICE_APID, myHandler, &myCtx);

// Subscribe to all packets (sensor + log messages)
SPP_SERVICES_PUBSUB_subscribe(K_SPP_APID_ALL, sdLogHandler, &s_logger);

// Publish — dispatches to subscribers, then auto-returns packet to databank
SPP_SERVICES_PUBSUB_publish(p_pkt);
```

---

## APID allocation

| Range | Owner |
|---|---|
| `0x0001` | `K_SPP_APID_LOG` — SPP log message packets |
| `0x0100` – `0x01FF` | Solaris sensor services |
| `0x0200` – `0x02FF` | Reserved for future Solaris services |
| `0x0300` – `0xFFFE` | User-defined services |
| `0xFFFF` | `K_SPP_APID_ALL` — pub/sub wildcard |

---

## Adding a new service

1. Create `services/myservice/myservice.h` and `myservice.c`
2. Implement `init`, `start`, `stop`, `deinit` callbacks
3. Define a `ServiceTask()` that calls `getPacket()` → `packetData()` → `publish()`
4. Declare `const SPP_ServiceDesc_t g_myServiceDesc = { ... }`
5. Add `services/myservice/myservice.c` to `CMakeLists.txt`
6. In `app_main()`: call `SPP_SERVICES_register()` before `SPP_SERVICES_initAll()`
