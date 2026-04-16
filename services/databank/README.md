# services/databank/

Static packet pool. Maintains a fixed array of `SPP_Packet_t` objects and a free-list stack. Producers call `SPP_Databank_getPacket()` to lease a packet, fill it, and hand it to `db_flow`. After the consumer is done with it, `SPP_Databank_returnPacket()` puts it back in the pool. `malloc` is never called.

Pool size is controlled by `K_SPP_DATABANK_SIZE` (default **5**, configurable via CMake).

---

## Files

| File | Description |
|---|---|
| `databank.h` | Public API |
| `databank.c` | Implementation |

---

## API

```c
SPP_RetVal_t   SPP_Databank_init(void);
SPP_Packet_t  *SPP_Databank_getPacket(void);
SPP_RetVal_t   SPP_Databank_returnPacket(SPP_Packet_t *p_packet);
uint32_t       SPP_Databank_freeCount(void);
```

---

## Usage

```c
// Producer
SPP_Packet_t *p_pkt = SPP_Databank_getPacket();
if (p_pkt == NULL) {
    SPP_LOGW("MY_SVC", "pool empty, dropping reading");
    return;
}

p_pkt->primaryHeader.apid      = 0x0101U;
p_pkt->primaryHeader.seq       = s_seq++;
p_pkt->primaryHeader.payloadLen = 12U;
p_pkt->secondaryHeader.timestampMs = SPP_Hal_getTimeMs();
memcpy(p_pkt->payload, &myData, 12U);

SPP_DbFlow_pushReady(p_pkt);

// Consumer (after popping from db_flow)
SPP_Databank_returnPacket(p_pkt);
```

---

## Rules

- Never call `returnPacket` twice on the same pointer — the double-return guard will return `K_SPP_ERROR_ALREADY_INITIALIZED`.
- Never use a packet after calling `returnPacket` — the memory may be reused immediately.
- If `getPacket` returns NULL, the pool is exhausted. Either increase `K_SPP_DATABANK_SIZE` or ensure consumers keep up with producers.
- `init` is idempotent — calling it twice returns `K_SPP_ERROR_ALREADY_INITIALIZED` and is harmless.
