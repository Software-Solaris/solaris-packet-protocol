# services/log/

Level-filtered logging service. Provides `SPP_LOG*` macros that route output through a registered callback function. The default callback prints to stdout. On embedded targets, replace it with a custom output function — no recompilation needed.

---

## Files

| File | Description |
|---|---|
| `log.h` | Public API, log macros, `SPP_LogLevel_t` |
| `log.c` | Implementation |

---

## Log levels

```c
K_SPP_LOG_NONE    = 0   // Silence all output
K_SPP_LOG_ERROR   = 1   // Critical errors only
K_SPP_LOG_WARN    = 2   // Warnings and errors
K_SPP_LOG_INFO    = 3   // Normal operational messages
K_SPP_LOG_DEBUG   = 4   // Detailed debug output
K_SPP_LOG_VERBOSE = 5   // Full trace (default after init)
```

Messages with a level **higher** than the current filter are suppressed.

---

## API

```c
SPP_RetVal_t    SPP_SERVICES_LOG_init(void);
void            SPP_SERVICES_LOG_setLevel(SPP_LogLevel_t level);
SPP_LogLevel_t  SPP_SERVICES_LOG_getLevel(void);
void            SPP_SERVICES_LOG_registerOutput(SPP_LogOutputFn_t fn);
```

---

## Macros

```c
SPP_LOGE("TAG", "SPI error %d", ret);    // error
SPP_LOGW("TAG", "pool at %d%%", pct);    // warning
SPP_LOGI("TAG", "service started");      // info
SPP_LOGD("TAG", "seq=%u ts=%u", s, t);  // debug
SPP_LOGV("TAG", "raw bytes: %02X", b);  // verbose
```

The tag is a short string identifying the module — use a module-level `const char *`:

```c
static const char *const k_tag = "BMP390";
SPP_LOGI(k_tag, "altitude = %.2f m", altitude);
```

---

## Log → pub/sub bridge

On the bare-metal target, every `SPP_LOG*` call can be forwarded into the pub/sub system as a `K_SPP_APID_LOG` packet. This lets the SD card logger capture log messages alongside sensor data — one line per message.

```c
static spp_bool_t s_logBusy = false;   // reentrancy guard

static void logPubSubOutput(const char *p_tag, SPP_LogLevel_t level,
                             const char *p_message)
{
    static const char k_lvl[] = "?EWID V";
    char lvlChar = k_lvl[(unsigned)level < sizeof(k_lvl) ? (unsigned)level : 0U];

    printf("[%c] %s: %s\n", lvlChar, p_tag, p_message);

    if (s_logBusy) { return; }     // prevent infinite loop
    s_logBusy = true;

    SPP_Packet_t *p_pkt = SPP_SERVICES_DATABANK_getPacket();
    if (p_pkt != NULL)
    {
        char buf[K_SPP_PKT_PAYLOAD_MAX];
        int n = snprintf(buf, sizeof(buf), "[%c] %s: %s", lvlChar, p_tag, p_message);
        spp_uint16_t len = (n > 0 && n < (int)sizeof(buf))
                           ? (spp_uint16_t)(n + 1U)
                           : (spp_uint16_t)sizeof(buf);
        (void)SPP_SERVICES_DATABANK_packetData(p_pkt, K_SPP_APID_LOG, s_logSeq++, buf, len);
        (void)SPP_SERVICES_PUBSUB_publish(p_pkt);
    }

    s_logBusy = false;
}

// Register before any SPP_LOG* calls:
SPP_SERVICES_LOG_registerOutput(logPubSubOutput);
```

The `s_logBusy` guard prevents recursion: if a subscriber calls `SPP_LOGE()`, the bridge skips publishing that nested message to avoid an infinite loop.

---

## Custom output callback

```c
// Redirect to UART on embedded target
static void uartLogOutput(const char *p_tag, SPP_LogLevel_t level,
                           const char *p_msg)
{
    uart_write_bytes(UART_NUM_0, p_msg, strlen(p_msg));
    uart_write_bytes(UART_NUM_0, "\n", 1);
}

SPP_SERVICES_LOG_registerOutput(uartLogOutput);
```

Pass NULL to restore the default stdout output.
