# Solaris Packet Protocol (SPP)

Solaris Packet Protocol (SPP) is a protocol for telemetry/command packet handling between avionic subsystems. This folder hosts the portable implementation of the protocol core, built around **HAL** (Hardware Abstraction Layer) and **OSAL** (Operating System Abstraction Layer). These layers decouple the protocol logic from any concrete microcontroller or RTOS so the same stack can be reused across platforms.


## Directory layout
- `core/`: core primitives (`core.c`/`core.h`), shared `types.h`, `macros.h`, and error codes (`returntypes.h`). `Core_Init()` lives here and bootstraps every service used by the stack.
- `services/`: reusable services that extend the core. Currently `databank/` provides a pool of `spp_packet_t`, tracks free packets, and exposes APIs to lease/return them safely.
- `hal/`: hardware abstraction APIs. The `spi/` folder defines how a port must initialize the bus, registers peripherals, and performs transactions. Concrete ports (e.g., ESP32) are located under `external/spp-ports` and simply implement this interface.
- `osal/`: operating-system abstraction (tasks, queues, semaphores, mutexes). Protocol code calls only these wrappers, allowing you to swap RTOS backends via `external/spp-ports/osal` without touching the core.
- `docs/`: generated Doxygen output (HTML/LaTeX) plus graphic assets. Rebuild it with `doxygen Doxyfile`.
- `Doxyfile`: documentation configuration describing sources, include paths, and output options for SPP.

## Typical flow
1. `Core_Init()` initializes services like `DataBank`, reserving the available packet pool.
2. The protocol requests a packet from `databank`, fills the `spp_packet_t` fields (see `types.h`), and hands it over to HAL for transmission.
3. Any scheduling/synchronization (tasks, queues, semaphores) uses OSAL only, keeping the protocol logic invariant.

## Abstraction layers
- **HAL** defines hardware-agnostic bus operations (e.g., SPI). Each target implements these hooks to map them to its driver set.
- **OSAL** offers a minimal RTOS contract (task creation, queues, semaphores, mutexes) so the core remains deterministic and portable.

Thanks to HAL/OSAL, SPP can be shared across different Solaris projects while providing the same packet semantics and only swapping the port located in `external/spp-ports`.
