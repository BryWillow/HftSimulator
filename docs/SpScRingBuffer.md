# SpScRingBuffer

<u>**Implementation:**</u> [SpScRingBuffer.h](../include/SpScRingBuffer.h)

A **high-performance, single-producer single-consumer (SPSC) ring buffer** designed for **ultra-low-latency systems**.

This ring buffer assumes:

- Producer and consumer run on **separate CPU cores**.
- Non-blocking operations using `tryPush` and `tryPop`.
- Power-of-two capacity for *fast wrap-around* using **bitwise AND**.

It is optimized for **cache locality**, **minimal latency**, and **predictable performance**.

---

## Features

- **Lock-free** – No mutexes or locks.
- **Non-blocking** – `tryPush` and `tryPop` return immediately if the buffer is full or empty.
- **Cache-line optimized** – `head` and `tail` are aligned to separate cache lines to prevent false sharing.
- **Power-of-two capacity** – Fast modulo using bitwise AND.
- **Move semantics** – Supports moving large objects in and out to reduce copying overhead.
- **C++20 compatible** – Uses `[[unlikely]]` for compiler branch prediction hints.

---

## Notes

```cpp
// Capacity must be a power of 2. Default: 4096.
template<typename T, size_t Capacity = 4096>
class SpscRingBuffer;

// Supports copy and move semantics.
bool tryPush(const T& item);
bool tryPush(T&& item);
```

## Example Usage: Consuming Market Data
```cpp
#include "SpscRingBuffer.h"
#include <thread>
#include <chrono>
#include <cstring>

// Unfortunately the Windows and Linux APIs are different.
#ifdef _WIN32
  #include "windows.h"
#else
  #include <pthread.h>
  #include <sched.h>
#endif

// Simple structure to illustrate move semantics.
struct MarketData {
    char symbol[16];
    double price;
    uint32_t size;
};

// Override the default capacity of 4096 to 1024.
SpscRingBuffer<MarketData, 1024> queue;

// Per above, run producer and consumer on separate cores.
// Unfortunately the Windows and Linux APIs are different.
void pinThreadToCore(std::thread& t, int core_id) {
#ifdef _WIN32
    DWORD_PTR mask = 1ULL << core_id;
    HANDLE handle = (HANDLE)t.native_handle();
    SetThreadAffinityMask(handle, mask);
#else
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
#endif
}

// Simulate incoming market data from an exchange.
void producer() {
    for (int i = 0; i < 1000; ++i) {
        MarketData md;
        snprintf(md.symbol, sizeof(md.symbol), "symbol_%d", i % 10);
        md.price = 100.0 + i * 0.01; 
        md.size = 100 + i % 50;      

        // Push inbound market data onto the ring buffer.
        while (!queue.tryPush(std::move(md))) {
            // Spin until the ring buffer isn't full.
        }
    }
}

// Simulate the strategy reading the market data from the producer thread.
void consumer() {
    MarketData md;
    for (int i = 0; i < 1000; ++i) {
        // Read market data from the ring buffer.
        while (!queue.tryPop(md)) {
            // Spin until there's some data available.
        }
        // Process the market data...
    }
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);

    // Pin threads to separate cores (example: core 0 and core 1)
    pinThreadToCore(t1, 0);
    pinThreadToCore(t2, 1);

    t1.join();
    t2.join();
}