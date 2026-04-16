# services/db_flow/

Circular FIFO that routes filled `SPP_Packet_t` pointers from producer tasks to consumer tasks. Producers call `pushReady()` after filling a packet; consumers call `popReady()` to retrieve the next one. No data is copied — only the pointer moves through the FIFO.

FIFO depth is controlled by `K_SPP_DBFLOW_READY_SIZE` (default **16**, must be a power of two, configurable via CMake).

---

## Files

| File | Description |
|---|---|
| `db_flow.h` | Public API |
| `db_flow.c` | Implementation |

---

## API

```c
SPP_RetVal_t SPP_DbFlow_init(void);
SPP_RetVal_t SPP_DbFlow_pushReady(SPP_Packet_t *p_pkt);
SPP_RetVal_t SPP_DbFlow_popReady(SPP_Packet_t **pp_pkt);
uint32_t     SPP_DbFlow_readyCount(void);
```

---

## Usage

```c
// Producer task
SPP_DbFlow_pushReady(p_pkt);

// Consumer task
SPP_Packet_t *p_pkt = NULL;
SPP_RetVal_t ret = SPP_DbFlow_popReady(&p_pkt);
if (ret == K_SPP_OK) {
    // process p_pkt ...
    SPP_Databank_returnPacket(p_pkt);
} else {
    // K_SPP_NOT_ENOUGH_PACKETS — FIFO was empty
}
```

---

## Rules

- `pushReady` returns `K_SPP_ERROR` if the FIFO is full. The calling producer is responsible for handling this (log a warning, increment the drop counter, return the packet to the databank).
- `popReady` returns `K_SPP_NOT_ENOUGH_PACKETS` when empty — this is a normal condition when the consumer runs faster than the producer.
- `db_flow` does **not** own the packets. The consumer must always call `SPP_Databank_returnPacket()` after processing.
- `init` is idempotent.

---

## Sizing guidance

If you see producers dropping packets (`pushReady` returns `K_SPP_ERROR`), increase `K_SPP_DBFLOW_READY_SIZE`. Keep it a power of two. The memory cost is `K_SPP_DBFLOW_READY_SIZE × sizeof(void *)` — negligible for typical values (16 × 4 = 64 bytes).
