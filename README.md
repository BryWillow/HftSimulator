# High Frequency Trading (HFT) Simulator

## Overview

The HFTSimulator is an example of how one might design a lock-free HFT algoritm. 
- Consumes/Decodes historic ITCH market data.
- Market data is passed to the strategy via an SPSC queue
- The strategy makes execution decisions.
- Execution decisions are passed along via an SPSC queue.
- Execution requests are sent via OUCH protocol to an exchange.
- Matching Engine / Order Respones / Fills in next release.

## Build Requirements
- C++ 20 compatible compiler (developed using clang 1700.0.13.5).
- CMake 3.16 or greater (https://cmake.org/download/) (developed using 4.1.1).
- Note: GTest is included locally due to Fetch/CMake quirks on MacOS.
- Run **build.sh** {Debug|Release|Clean} in the project directory.
  * Note that a Debug build also *sanitizes* and *runs unit tests*.
  * Binaries are located in bin/Debug and bin/Release
## Market Data Consumption
- The entire flow is intended to emulate **NSDQ**.
- Market data is formatted using the **TotalView-ITCH 5.0** protocol.
- It is encoded using **SBE** (Simple Binary Encoding).
- It is transmitted via the **UDP** protocol.
- Implementation is in the <u>ItchConnection</u> and <u>ItchProcessor</u> classes.

## Market Data Decoding
- See the <u>ItchProcessor</u> class.
- Producer for the Market Data SPSC queue.

## 🚀 Lock-Free SPSC Queue
- Lock-free SPSC queue
- Cache-line padding to avoid false sharing
- Hard-coded power-of-two capacity for fast modulo (`& mask`)
- Unit tests with **GoogleTest**
- Optional runtime **sanitizers** (Address, UB, Thread, Memory)

---

## Strategy
- Consumer of the Market Data SPSC queue.
- Makes Execution Decisions
- Producer for the Execution SPSC queue.

## Execution Encoding
- Transforms strategy execution requests into OUCH format. 

## Execution
- Sends orders to an exchange.


## Building and Running Tests

## Building and Running Tests

### 1. Configure & Build
From the project root:

```bash
- mkdir -p build
- cd build
- cmake -DCMAKE_BUILD_TYPE=Release ..
- cmake --build . -j
```