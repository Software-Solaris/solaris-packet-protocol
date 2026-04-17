# core/

Foundation layer of SPP. Defines the packet format, portable types, error codes, and the startup sequence that wires the HAL port into the rest of the stack. Everything else in SPP depends on `core/`; `core/` depends on nothing inside SPP.

---

## Files

| File | Description |
|---|---|
| `types.h` | Portable integer aliases (`spp_uint8_t` … `spp_uint64_t`, `spp_bool_t`) and hardware config structs (`SPP_SpiInitCfg_t`, `SPP_StorageInitCfg_t`) |
| `returntypes.h` | `SPP_RetVal_t` — the single return type used by every public SPP function |
| `packet.h` | `SPP_Packet_t` — the on-wire packet layout (primary header + secondary header + 48 B payload + CRC) |
| `version.h` | `K_SPP_VERSION_MAJOR / MINOR / PATCH` compile-time constants |
| `error.h` | Extended error context (error code + optional string message) |
| `core.h` | `SPP_Core_setHalPort()`, `SPP_Core_init()` |
| `core.c` | Implementation of port registration and startup |
| `error.c` | Implementation of error context helpers |

---

## Packet format

```
 ┌──────────┬──────┬───────┬─────────────┐
 │ version  │ apid │  seq  │ payloadLen  │  ← primaryHeader (7 B)
 ├──────────┴──────┴───────┴─────────────┤
 │    timestampMs    │  dropCounter      │  ← secondaryHeader (5 B)
 ├───────────────────────────────────────┤
 │           payload (0–48 B)            │
 ├───────────────────────────────────────┤
 │               crc (2 B)              │
 └───────────────────────────────────────┘
```

- **apid** — Application Process ID. Each service has a unique APID (e.g. BMP390 = `0x0101`, ICM20948 = `0x0201`). `K_SPP_APID_LOG` (`0x0001`) is reserved for log message packets. Subscribers use APID to filter packets.
- **seq** — Monotonically increasing counter per service. Gaps indicate dropped packets.
- **payloadLen** — Number of valid bytes in `payload`. Must be ≤ `K_SPP_PKT_PAYLOAD_MAX` (48).
- **crc** — CRC-16/CCITT computed over the full packet minus the CRC field. Computed automatically by `SPP_Databank_packetData()`. Set to 0 if not used.

---

## Return codes

All SPP functions return `SPP_RetVal_t`. Always check the return value.

| Code | Meaning |
|---|---|
| `K_SPP_OK` | Success |
| `K_SPP_ERROR` | Generic error |
| `K_SPP_NOT_ENOUGH_PACKETS` | Databank pool exhausted |
| `K_SPP_NULL_PACKET` | Packet pointer is NULL |
| `K_SPP_ERROR_ALREADY_INITIALIZED` | Module already initialised |
| `K_SPP_ERROR_NULL_POINTER` | Required pointer argument is NULL |
| `K_SPP_ERROR_NOT_INITIALIZED` | Module not yet initialised |
| `K_SPP_ERROR_INVALID_PARAMETER` | Argument out of range |
| `K_SPP_ERROR_ON_SPI_TRANSACTION` | SPI transaction failed |
| `K_SPP_ERROR_TIMEOUT` | Operation timed out |
| `K_SPP_ERROR_NO_PORT` | HAL port not registered |
| `K_SPP_ERROR_REGISTRY_FULL` | Service registry is full |

---

## Startup sequence

```c
// 1. Register HAL port (mandatory before SPP_Core_init)
SPP_Core_setHalPort(&g_myHalPort);

// 2. Initialise core (calls SPP_Databank_init + SPP_PubSub_init internally)
SPP_Core_init();

// 3. Subscribe consumers
SPP_PubSub_subscribe(K_SPP_APID_ALL, myHandler, &myCtx);

// 4. Register and start application services
SPP_Service_register(&g_bmp390ServiceDesc, &s_ctx, &s_cfg);
SPP_Service_initAll();
SPP_Service_startAll();
```

---

## Portable types

`types.h` provides width-exact aliases so SPP code is independent of compiler and architecture:

```c
spp_uint8_t   spp_int8_t
spp_uint16_t  spp_int16_t
spp_uint32_t  spp_int32_t
spp_uint64_t  spp_int64_t
spp_bool_t                  // bool
```

Use these inside SPP modules. For application code outside SPP, your platform's native types are fine.
