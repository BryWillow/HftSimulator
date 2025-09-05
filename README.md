# High Frequency Trading (HFT) Simulator

## Overview

The HFT Simulator is a demonstration of how one might design the skeleton of an HFT algoritm.

## Market Data Transmission
- The entire flow is intended to emulate **NSDQ**.
- Market data is formatted using the **TotalView-ITCH 5.0** protocol.
- It is encoded using **SBE** (Simple Binary Encoding).
- It is transmitted via the **UDP** protocol.
- Implementation is in the <u>ItchConnection</u> and <u>ItchProcessor</u> classes.


---

## 🚀 Features
- Lock-free SPSC queue
- Cache-line padding to avoid false sharing
- Hard-coded power-of-two capacity for fast modulo (`& mask`)
- Unit tests with **GoogleTest**
- Optional runtime **sanitizers** (Address, UB, Thread, Memory)

---