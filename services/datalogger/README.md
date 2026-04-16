# services/datalogger/

SD card packet logger. Mounts a FAT filesystem over SPI, opens a log file, and writes `SPP_Packet_t` payloads as structured text. Called by sensor services after popping a packet from `db_flow`.

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
    void     *p_storageCfg;   // Pointer to SPP_StorageInitCfg_t
    FILE     *p_file;
    uint8_t   isInitialized;
    uint32_t  loggedPackets;
} Datalogger_t;
```

---

## API

```c
SPP_RetVal_t DATALOGGER_Init(Datalogger_t *dl, void *storageCfg, const char *path);
SPP_RetVal_t DATALOGGER_LogPacket(Datalogger_t *dl, const SPP_Packet_t *pkt);
SPP_RetVal_t DATALOGGER_Flush(Datalogger_t *dl);
SPP_RetVal_t DATALOGGER_Deinit(Datalogger_t *dl);
```

---

## Usage

```c
static Datalogger_t s_logger;
static SPP_StorageInitCfg_t s_sdCfg = {
    .p_basePath          = "/sdcard",
    .spiHostId           = SPI2_HOST,
    .pinCs               = 8U,
    .maxFiles            = 5U,
    .allocationUnitSize  = 16384U,
    .formatIfMountFailed = false,
};

DATALOGGER_Init(&s_logger, &s_sdCfg, "/sdcard/log.txt");

// In the consumer loop:
SPP_Packet_t *p_pkt = NULL;
SPP_DbFlow_popReady(&p_pkt);
DATALOGGER_LogPacket(&s_logger, p_pkt);
SPP_Databank_returnPacket(p_pkt);
```

---

## Hardware configuration (ESP32-S3)

- SPI bus: SPI2_HOST
- CS GPIO: 8
- Mount point: `/sdcard`
- Default allocation unit: 16 KB
