# util/

Standalone utilities used across SPP. No dependencies on OSAL, HAL, or services.

---

## Files

| File | Description |
|---|---|
| `macros.h` | Compile-time feature flags and capacity constants |
| `crc.h` + `crc.c` | CRC-16/CCITT checksum |
| `structof.h` | Container-of macro for intrusive data structures |

---

## macros.h — Feature flags and constants

Override any of these in your `CMakeLists.txt` to tune SPP for your target:

```cmake
target_compile_definitions(spp PUBLIC
    K_SPP_DATABANK_SIZE=10          # Default: 5
    K_SPP_DBFLOW_READY_SIZE=32      # Default: 16 (must be power of two)
    K_SPP_STACK_SIZE=8192           # Default: 4096 bytes
    K_SPP_MAX_TASKS=12              # Default: 8
    K_SPP_MAX_SERVICES=8            # Default: 16
    SPP_NO_RTOS=1                   # Disable RTOS, use baremetal port
    SPP_NO_MALLOC=1                 # Disable dynamic allocation
    SPP_NO_STORAGE=1                # Disable SD card / filesystem
)
```

---

## crc.h — CRC-16/CCITT

```c
// Compute CRC over a packet's full content before transmitting
spp_uint16_t crc = SPP_Util_crc16(
    (const uint8_t *)p_pkt,
    sizeof(SPP_Packet_t) - sizeof(uint16_t)  // exclude the crc field itself
);
p_pkt->crc = crc;
```

Polynomial: **0x1021**, initial value: **0xFFFF**. Compatible with standard CRC-16/CCITT implementations.

---

## structof.h — Container-of macro

Recovers a pointer to the enclosing struct from a pointer to one of its members. Useful for intrusive linked lists and trees — no separate allocation for list nodes.

```c
typedef struct {
    uint32_t    value;
    ListNode_t  node;   // intrusive list node embedded in the struct
} MyItem_t;

// Recover the MyItem_t from a pointer to its node member:
ListNode_t *p_node = list_next(head);
MyItem_t   *p_item = SPP_STRUCTOF(p_node, MyItem_t, node);
```

Inspired by the `container_of` macro in the Linux kernel and the `structof` pattern in lely-core.
