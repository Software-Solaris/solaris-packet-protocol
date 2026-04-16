# services/

SPP's service layer. Provides the packet lifecycle infrastructure (databank, db_flow, logging) and the sensor/logging drivers (BMP390, ICM20948, datalogger). All services share a common registration interface defined in `service.h`.

---

## Service types

### Infrastructure services (always compiled)

| Directory | Description |
|---|---|
| `databank/` | Static packet pool — allocates and recycles `SPP_Packet_t` objects |
| `db_flow/` | Circular FIFO — routes filled packets from producers to consumers |
| `log/` | Level-filtered logging with a swappable output callback |

### Sensor services (opt-in at build time)

| Directory | CMake flag | Description |
|---|---|---|
| `bmp390/` | `SPP_SERVICE_BMP390` | BMP390 pressure / altitude sensor |
| `icm20948/` | `SPP_SERVICE_ICM20948` | ICM20948 IMU (accelerometer + gyroscope + DMP) |
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

Registration pattern in `main.c`:

```c
static BMP390_ServiceCtx_t  s_bmpCtx;
static BMP390_ServiceCfg_t  s_bmpCfg = { .csPin = 18U, .intPin = 5U };

SPP_Service_register(&g_bmp390ServiceDesc, &s_bmpCtx, &s_bmpCfg);
SPP_Service_startAll();
```

The registry calls `init` and `start` on each service in registration order. `stop` and `deinit` are called in reverse order on shutdown. No memory is allocated by the registry — all context buffers are caller-managed.

---

## Packet data flow

```
   Producer task                          Consumer task
  ─────────────                          ─────────────
  SPP_Databank_getPacket()  ──→ packet
  fill packet fields
  SPP_DbFlow_pushReady(pkt) ──→ FIFO ──→ SPP_DbFlow_popReady(&pkt)
                                          process / log packet
                                          SPP_Databank_returnPacket(pkt)
```

---

## Adding a new service

1. Create `services/myservice/myservice.h` and `myservice.c`
2. Implement `init`, `start`, `stop`, `deinit` callbacks
3. Declare `const SPP_ServiceDesc_t g_myServiceDesc = { ... }`
4. Add `services/myservice/myservice.c` to `CMakeLists.txt`
5. In `main.c`: call `SPP_Service_register()` before `SPP_Service_startAll()`

### APID allocation

| Range | Owner |
|---|---|
| `0x0100` – `0x01FF` | Solaris sensor services |
| `0x0200` – `0x02FF` | Reserved for future Solaris services |
| `0x0300` – `0xFFFE` | User-defined services |
| `0xFFFF` | Broadcast / unassigned |
