# services/log/

Level-filtered logging service. Provides `SPP_LOG*` macros that route output through a registered callback function. The default callback prints to stdout. On embedded targets, replace it with a UART writer or a circular buffer — no recompilation needed.

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
SPP_RetVal_t    SPP_Log_init(void);
void            SPP_Log_setLevel(SPP_LogLevel_t level);
SPP_LogLevel_t  SPP_Log_getLevel(void);
void            SPP_Log_registerOutput(SPP_LogOutputFn_t fn);
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

The tag is a short string identifying the module — use a module-level `#define`:

```c
#define TAG "BMP390"
SPP_LOGI(TAG, "altitude = %.2f m", altitude);
```

---

## Custom output callback

```c
// Redirect to UART on embedded target
static void uartLogOutput(const char *tag, SPP_LogLevel_t level,
                          const char *msg)
{
    uart_write_bytes(UART_NUM_0, msg, strlen(msg));
    uart_write_bytes(UART_NUM_0, "\n", 1);
}

SPP_Log_registerOutput(uartLogOutput);
```

Pass NULL to silence all output.
