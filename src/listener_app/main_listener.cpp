// ---------------------------------------------------------------------------
// File        : main_listener.cpp
// Project     : HftSimulator
// App         : Listener App
// Description : HFT-style UDP listener for ITCH messages with SPSC ring buffer
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#include <atomic>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>

#include "json.hpp"                   // JSON parsing using nlohmann::json library
#include "itch_message.h"             // ITCH message definitions
#include "itch_udp_listener.h"        // UDP listener
#include "spsc_ringbuffer_consumer_internal.h"
#include "spsc_ringbuffer.h"          // SPSC ring buffer
#include "spsc_ringbuffer_consumer.h" // Ring buffer consumer

// Use nlohmann::json for easy, header-only, C++17/20-friendly JSON parsing
using json = nlohmann::json;

/// @brief Global stop flag for clean shutdown via signal
std::atomic<bool> g_stopFlag{false};

/// @brief Signal handler for Ctrl+C / termination
void signalHandler(int)
{
    g_stopFlag.store(true);
}

/// @brief Simple strategy to track Add Order messages per symbol
struct SimpleStrategy
{
    std::unordered_map<std::string, uint64_t> symbolCounts;

    /// @brief Process a single ITCH message
    void processMessage(const ItchMessage &msg)
    {
        if (msg.isAddOrder())
        {
            symbolCounts[msg.symbolStr()]++;
        }
    }

    /// @brief Print current counts
    void printCounts() const
    {
        std::cout << "--- Message Counts ---\n";
        for (const auto &[sym, count] : symbolCounts)
        {
            std::cout << sym << ": " << count << "\n";
        }
        std::cout << "--------------------\n";
    }
};

/// @brief Load listener config from JSON file
struct ListenerConfig
{
    uint16_t udpPort{5555};
    size_t ringBufferSize{2048};
    int cpuCoreListener{0};
    int cpuCoreConsumer{1};
    std::chrono::seconds marketDataIdleTimeout{1800}; // default 30 min
};

ListenerConfig loadListenerConfig(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file: " + path);

    json configJson;
    file >> configJson;

    const auto &listenerJson = configJson["listener"];
    const auto &sharedJson = configJson["shared"];

    ListenerConfig cfg;
    cfg.udpPort = listenerJson.value("udp_port", 5555);
    cfg.ringBufferSize = listenerJson.value("mkt_data_buffer_size", 2048);
    cfg.cpuCoreListener = listenerJson.value("cpu_core_listener", 0);
    cfg.cpuCoreConsumer = listenerJson.value("cpu_core_consumer", 1);

    // Parse hh:mm:ss timeout into seconds
    std::string timeoutStr = listenerJson.value("market_data_idle_timeout", "00:30:00");
    int hh, mm, ss;
    if (std::sscanf(timeoutStr.c_str(), "%d:%d:%d", &hh, &mm, &ss) == 3)
    {
        cfg.marketDataIdleTimeout = std::chrono::hours(hh) + std::chrono::minutes(mm) + std::chrono::seconds(ss);
    }

    return cfg;
}

int main()
{
    try
    {
        // --- Load configuration ---
        ListenerConfig cfg = loadListenerConfig("config.json");

        // Handle Ctrl+C gracefully
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        // --- Setup SPSC ring buffer for market data ---
        SpScRingBuffer<ItchMessage, hft::DEFAULT_RING_BUFFER_CAPACITY> marketDataRingBuffer;


        // --- Start UDP listener ---
        auto listenerCallback = [&marketDataRingBuffer](const ItchMessage &msg)
        {
            while (!marketDataRingBuffer.tryPush(msg))
            {                // non-blocking push
                _mm_pause(); // reduce CPU spinning
            }
        };
        ItchUdpListener listener(cfg.udpPort, listenerCallback, cfg.cpuCoreListener);
        listener.start();

        // --- Setup simple strategy ---
        SimpleStrategy strategy;

        // --- Consumer lambda ---
        auto consumerLambda = [&strategy](const ItchMessage &msg)
        {
            strategy.processMessage(msg);
        };

        // --- Start ring buffer consumer using factory ---
        auto consumer = make_ring_buffer_consumer(marketDataRingBuffer, consumerLambda);
        consumer.start(cfg.cpuCoreConsumer);

        // --- Track idle timeout ---
        auto lastMessageTime = std::chrono::steady_clock::now();

        while (!g_stopFlag.load(std::memory_order_relaxed))
        {
            if (!marketDataRingBuffer.empty())
            {
                lastMessageTime = std::chrono::steady_clock::now();
            }

            // Check for idle timeout
            auto now = std::chrono::steady_clock::now();
            if (now - lastMessageTime > cfg.marketDataIdleTimeout)
            {
                std::cout << "[Listener App] Market data idle timeout reached. Shutting down.\n";
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // --- Stop pipeline gracefully ---
        listener.stop(); // stops pinned listener thread
        consumer.stop(); // stops consumer thread

        // --- Print final strategy results ---
        strategy.printCounts();
        std::cout << "[Listener App] Pipeline complete.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Listener App] Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}