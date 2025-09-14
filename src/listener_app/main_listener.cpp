#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>
#include <csignal>

#include "itch_message.h"
#include "itch_listener.h"
#include "spsc_ringbuffer.h"
#include "ring_buffer_consumer.h"
#include "ring_buffer_consumer_internal.h"

/**
 * @file main.cpp
 * @brief Listener app: full HFT-style market data pipeline using TotalView ITCH messages.
 *
 * Pipeline overview:
 * - Producer: ItchUdpListener receives ITCH messages via UDP.
 * - Consumer: RingBufferConsumer processes messages from the SPSC queue.
 * - Strategy: Tracks message counts per symbol.
 *
 * Notes:
 * - Uses pinned threads and relaxed atomics for HFT-style low-latency behavior.
 * - ITCH messages are converted from raw bytes to host order inside the listener.
 * - Ring buffer is single-producer, single-consumer for maximum speed.
 * - `_mm_pause()` is used in spin loops to reduce aggressive CPU pressure.
 */

/// @brief Example structure to track message counts per symbol
struct SimpleStrategy {
    std::unordered_map<std::string, uint64_t> symbolCounts;

    /// @brief Process a single ITCH message
    void processMessage(const ItchMessage& msg) {
        if (msg.isAddOrder()) { // Only count Add Order messages
            symbolCounts[msg.symbolStr()]++;
        }
    }

    /// @brief Print current counts
    void printCounts() const {
        std::cout << "--- Message Counts ---\n";
        for (const auto& [sym, count] : symbolCounts) {
            std::cout << sym << ": " << count << "\n";
        }
        std::cout << "--------------------\n";
    }
};

// Global stop flag for clean shutdown via signal
std::atomic<bool> g_stopFlag{false};

void signalHandler(int) {
    g_stopFlag.store(true);
}

int main() {
    // --- Pipeline configuration ---
    constexpr uint16_t LOCAL_UDP_PORT = 5555;        // UDP port for incoming ITCH messages
    constexpr size_t   RING_BUFFER_SIZE = 2048;     // Compile-time SPSC buffer size
    constexpr size_t   CPU_CORE_MARKET_DATA = 0;    // Listener pinned to this core
    constexpr size_t   CPU_CORE_CONSUMER = 1;       // Consumer pinned to this core

    // Handle Ctrl+C
    std::signal(SIGINT, signalHandler);

    // --- Setup SPSC ring buffer for market data ---
    SpScRingBuffer<ItchMessage, RING_BUFFER_SIZE> marketDataRingBuffer;

    // --- Start UDP listener (pinned to core 0) ---
    auto listenerCallback = [&marketDataRingBuffer](const ItchMessage& msg) {
        // Push message into the ring buffer in a tight loop until successful
        while (!marketDataRingBuffer.tryPush(msg)) {
            _mm_pause(); // reduce aggressive spinning
        }
    };
    ItchUdpListener listener(LOCAL_UDP_PORT, listenerCallback, CPU_CORE_MARKET_DATA);
    listener.start();

    // --- Simple strategy to count Add Order messages ---
    SimpleStrategy strategy;

    // --- Consumer lambda ---
    auto consumerLambda = [&strategy](const ItchMessage& msg) {
        strategy.processMessage(msg); // Hot-path processing
    };

    // --- RingBufferConsumer pinned to core 1 using factory ---
    auto consumer = make_ring_buffer_consumer<ItchMessage, RING_BUFFER_SIZE>(
        marketDataRingBuffer,
        consumerLambda
    );
    consumer.start(CPU_CORE_CONSUMER);

    // --- Run pipeline until signal ---
    while (!g_stopFlag.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // --- Stop pipeline gracefully ---
    listener.stop();
    consumer.stop();

    // --- Print final strategy results ---
    strategy.printCounts();
    std::cout << "[Listener App] Pipeline complete.\n";

    return 0;
}