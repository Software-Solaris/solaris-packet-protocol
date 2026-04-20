# services/datalogger/

SD card packet logger. Mounts a FAT filesystem over SPI, opens a log file, and writes packets as structured text — one line per packet. Used as a pub/sub subscriber: registered at startup and called synchronously for every published packet.

---

## Files

| File | Description |
|---|---|
| `datalogger.h` | Public API, config and context structs |
| `datalogger.c` | Implementation — mount, open, write, flush, close |

---

## Key types

```c
typedef struct {
    const char   *p_basePath;         // VFS mount point, e.g. "/sdcard"
    int           spiHostId;          // SPI host for the SD card
    int           pinCs;              // Chip-select GPIO
    uint32_t      maxFiles;           // Max simultaneously open files
    uint32_t      allocationUnitSize; // FAT cluster size in bytes
    bool          formatIfMountFailed;
} SPP_StorageInitCfg_t;               // From spp/core/types.h

typedef struct {
    void     *p_storage_cfg;   // Pointer to SPP_StorageInitCfg_t
    FILE     *p_file;
    uint8_t   is_initialized;
    uint32_t  logged_packets;
} Datalogger_t;
```

---

## API

```c
SPP_RetVal_t SPP_SERVICES_DATALOGGER_init(Datalogger_t *p_logger, void *p_storage_cfg,
                              const char *p_file_path);
SPP_RetVal_t SPP_SERVICES_DATALOGGER_logPacket(Datalogger_t *p_logger,
                                   const SPP_Packet_t *p_packet);
SPP_RetVal_t SPP_SERVICES_DATALOGGER_flush(Datalogger_t *p_logger);
SPP_RetVal_t SPP_SERVICES_DATALOGGER_deinit(Datalogger_t *p_logger);
```

---

## Log line format

**Log message packets** (`apid == K_SPP_APID_LOG`):
```
[I] BMP390: altitude = 1234.56 m
```

**Sensor/telemetry packets** (all other APIDs):
```
ts=12345 apid=0x0101 seq=7 len=12 payload_hex=44 9A 4B 45 00 00 B8 43 00 80 FF 42
```

Each line is terminated with `\n`. Call `SPP_SERVICES_DATALOGGER_flush()` periodically to limit data loss on power cut.

---

## Usage as a pub/sub subscriber

```c
#define K_SD_FLUSH_EVERY (20U)

static void sdLogHandler(const SPP_Packet_t *p_packet, void *p_ctx)
{
    Datalogger_t *p_log = (Datalogger_t *)p_ctx;
    (void)SPP_SERVICES_DATALOGGER_logPacket(p_log, p_packet);

    if ((p_log->logged_packets % K_SD_FLUSH_EVERY) == 0U)
    {
        (void)SPP_SERVICES_DATALOGGER_flush(p_log);
    }
}

// Register before starting services:
SPP_SERVICES_DATALOGGER_init(&s_logger, &s_storageCfg, "/sdcard/log.txt");
SPP_SERVICES_PUBSUB_subscribe(K_SPP_APID_ALL, sdLogHandler, &s_logger);
```

After this, every published packet — sensor data and log messages alike — is written to the SD card automatically.

---

## Hardware configuration (ESP32-S3)

- SPI bus: SPI2_HOST (host ID 1)
- CS GPIO: 8
- Mount point: `/sdcard`
- Default allocation unit: 16 KB
