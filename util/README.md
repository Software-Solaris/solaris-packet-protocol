# util/

Standalone utilities used across SPP. No dependencies on HAL or services.

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
    K_SPP_DATABANK_SIZE=10           # Default: 5 — packet pool size
    K_SPP_PUBSUB_MAX_SUBSCRIBERS=16  # Default: 8 — max registered subscribers
    K_SPP_MAX_SERVICES=8             # Default: 16 — service registry slots
    SPP_NO_MALLOC=1                  # Disable dynamic allocation
    SPP_NO_STORAGE=1                 # Disable SD card / filesystem
)
```

---

## crc.h — CRC-16/CCITT

`SPP_Databank_packetData()` calls this automatically — you do not need to call it directly unless computing a CRC on a raw buffer.

```c
// Manual CRC example (not needed for normal SPP usage):
spp_uint16_t crc = SPP_Util_crc16(
    (const spp_uint8_t *)p_pkt,
    (spp_uint32_t)offsetof(SPP_Packet_t, crc)  // exclude the crc field itself
);
p_pkt->crc = crc;
```

Polynomial: **0x1021**, initial value: **0xFFFF**. Compatible with standard CRC-16/CCITT implementations.

The full packet struct is `memset` to zero before filling, so compiler padding bytes are always 0 and the CRC is deterministic across compilers and architectures.

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
