# High Frequency Trading (HFT) Simulator
*Author: Bryan Camp*

## Overview

The **HftSimulator** is a high-performance, lock-free HFT pipeline designed to demonstrate professional-grade market data consumption, strategy execution, and low-latency design patterns.

Key highlights:
- Realistic **NASDAQ TotalView-ITCH 5.0** market data replay.
- **Lock-free SPSC queues** with cache-line padding for zero-contention communication.
- Pluggable **header-only strategies** for flexible algo development.
- Full control over thread pinning and CPU core assignment.
- Minimal heap allocations to maintain microsecond-level latency.

---

## Build Requirements

- **Supported Platforms:** Linux and MacOS (fully tested on MacOS)
- **C++20** compatible compiler  
  - Tested with **Clang 17.0.0.13.5** on MacOS
- **CMake 3.16+**  
  - Tested with **CMake 4.1.1**
- GoogleTest included locally for unit tests.

**Build Script:** `build.sh`  

    ./build.sh [Debug|Release|All]

- **Debug:** runs unit tests and enables sanitizers (Address, Thread, Memory, UB)
- **Release:** fully optimized, latency-focused
- Binaries are placed in `bin/Debug` and `bin/Release`

---

## Components

### Replayer App (`replayer_app`)
- Reads historic ITCH messages from files.
- Replays market data over UDP.
- Supports configurable speed and stress testing.

### Listener App (`listener_app`)
- Receives UDP market data from replayer or exchange.
- Decodes ITCH messages into structured types.
- Pushes decoded messages into **lock-free SPSC queues**.
- Utilizes **pinned threads** for consistent microsecond-level processing.

### Strategies (`src/strategies/`)
- Header-only implementations (e.g., `micro_mean_reversion_strategy.h`).
- Consume market data from SPSC queues.
- Produce execution decisions to execution queue.
- Fully pluggable for rapid experimentation and deployment.

### Common (`src/common/`)
- Header-only utilities shared across executables.
- Includes:
  - Lock-free **SPSC ring buffers**  
  - **Pinned threads** for low-latency cores  
  - ITCH decoding helpers  
  - CPU pause loops and cache-aware structures

---

## Market Data

- Simulates **NASDAQ TotalView-ITCH 5.0**.
- Encoded using **SBE** (Simple Binary Encoding).
- Transmitted via **UDP**.
- Classes: `ItchConnection` and `ItchProcessor` handle reception and decoding.

---

## Lock-Free SPSC Queues

- **Single-producer, single-consumer** design.
- Cache-line aligned to prevent false sharing.
- Fixed-size power-of-two capacity allows fast modulo operations using bitmask.
- Optional runtime **sanitizers** (Address, Thread, Memory, UB).
- Fully unit-tested with **GoogleTest**.

---

## Adding New Strategies

1. Create a header in `src/strategies/`.
2. Include it in your pipeline consumer (listener or main.cpp).
3. Build and test using `build.sh`.
4. Your strategy is immediately pluggable without changing other code.

Example: Plugging in `micro_mean_reversion_strategy.h`

    #include "strategies/micro_mean_reversion_strategy.h"

    // inside listener or main.cpp
    MicroMeanReversionStrategy strategy;
    strategy.consume(marketDataRingBuffer);

- No heap allocations required.
- Fully compatible with existing SPSC queues.
- Strategy can make execution decisions asynchronously.

---

## Notes

- Pinned threads reduce OS scheduling jitter.
- Ring buffers and structures are **cache-aligned** for microsecond performance.
- Fully designed for **rapid strategy experimentation**.
- Written entirely by **Bryan Camp**.

---

## Pipeline Structure

See below for a simplified view of how data flows through the HftSimulator pipeline:

    +----------------+     UDP     +----------------+     Ring Buffer     +----------------------+
    | Replayer App   | ----------> | Listener App   | ----------------->  | Header-Only Strategy |
    | (replay ITCH)  |             | (decode ITCH) |                     | (Micro Mean-Reversion|
    +----------------+             +----------------+                     |  or your custom algo)|
                                                                         +----------+-----------+
                                                                                    |
                                                                                    v
                                                                        +------------------+
                                                                        | Execution Queue  |
                                                                        | (for OUCH orders |
                                                                        |  in future)      |
                                                                        +------------------+
