# osal/

OS Abstraction Layer. Defines the **contract** that any OS port must fulfill — a struct of function pointers (`SPP_OsalPort_t`) covering task management, queues, mutexes, and event groups. SPP core and services call only these abstractions; they never call FreeRTOS, POSIX, or any OS API directly.

`osal/` contains only headers and the thin dispatch layer (`dispatch.c`). The actual implementations live in [`ports/osal/`](../ports/osal/).

---

## Files

| File | Description |
|---|---|
| `port.h` | `SPP_OsalPort_t` — the full contract struct with all function pointer signatures |
| `task.h` | `SPP_Osal_taskCreate()`, `SPP_Osal_taskDelete()`, `SPP_Osal_taskDelayMs()`, `SPP_Osal_getTickMs()` |
| `queue.h` | `SPP_Osal_queueCreate()`, `SPP_Osal_queueSend()`, `SPP_Osal_queueRecv()`, `SPP_Osal_queueCount()` |
| `mutex.h` | `SPP_Osal_mutexCreate()`, `SPP_Osal_mutexLock()`, `SPP_Osal_mutexUnlock()` |
| `event.h` | `SPP_Osal_eventCreate()`, `SPP_Osal_eventWait()`, `SPP_Osal_eventSetFromIsr()` |
| `dispatch.c` | Routes every `SPP_Osal_*()` call through the port registered via `SPP_Core_setOsalPort()` |

---

## The port contract

```c
typedef struct {
    // Tasks
    void        *(*taskCreate)(void (*fn)(void *), const char *name,
                               uint32_t stackWords, void *arg, uint32_t prio);
    void         (*taskDelete)(void *handle);
    void         (*taskDelayMs)(uint32_t ms);
    uint32_t     (*getTickMs)(void);

    // Queues
    void        *(*queueCreate)(uint32_t len, uint32_t itemSize);
    SPP_RetVal_t (*queueSend)(void *q, const void *item, uint32_t timeoutMs);
    SPP_RetVal_t (*queueRecv)(void *q, void *item, uint32_t timeoutMs);
    uint32_t     (*queueCount)(void *q);

    // Mutexes
    void        *(*mutexCreate)(void);
    SPP_RetVal_t (*mutexLock)(void *m, uint32_t timeoutMs);
    SPP_RetVal_t (*mutexUnlock)(void *m);

    // Event groups
    void        *(*eventCreate)(void);
    SPP_RetVal_t (*eventWait)(void *e, uint32_t bits, bool clearOnExit,
                              bool waitAll, uint32_t timeoutMs, uint32_t *actualBits);
    SPP_RetVal_t (*eventSetFromIsr)(void *e, uint32_t bits,
                                   uint32_t *prev, bool *yield);
} SPP_OsalPort_t;
```

---

## Priority levels

Use the `K_SPP_OSAL_PRIO_*` constants when creating tasks — they map to the OS-specific values inside each port:

| Constant | Value | Intended use |
|---|---|---|
| `K_SPP_OSAL_PRIO_IDLE` | 0 | Background / idle work |
| `K_SPP_OSAL_PRIO_LOW` | 1 | Non-critical logging |
| `K_SPP_OSAL_PRIO_NORMAL` | 2 | Typical sensor tasks |
| `K_SPP_OSAL_PRIO_HIGH` | 3 | Time-sensitive processing |
| `K_SPP_OSAL_PRIO_CRITICAL` | 4 | Interrupt-driven tasks |

---

## Available ports

| Port | Location | Notes |
|---|---|---|
| FreeRTOS | `ports/osal/freertos/` | Production port for ESP32 and similar MCUs |
| Baremetal | `ports/osal/baremetal/` | Cooperative scheduler, no OS required |
| POSIX | `ports/osal/posix/` | Host testing on Linux/macOS via pthreads |

---

## Implementing a new OSAL port

1. Create `ports/osal/<os>/osal_<os>.c`
2. Implement every function pointer in `SPP_OsalPort_t` (all are mandatory)
3. Declare a `const SPP_OsalPort_t g_<os>OsalPort = { ... }`
4. Register it at startup: `SPP_Core_setOsalPort(&g_<os>OsalPort)`

See `ports/osal/freertos/osal_freertos.c` as the reference implementation.
