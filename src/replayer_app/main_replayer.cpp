// ---------------------------------------------------------------------------
// File        : main_replayer.cpp
// Project     : HftSimulator
// App         : Replayer App
// Description : Replay captured ITCH messages over UDP for simulation
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "json.hpp"                      // JSON parsing using nlohmann::json library
#include "itch_message_udp_replayer.h"   // ITCH UDP replayer
#include "constants.h"                   // Global compile-time constants (e.g., BUFFER_SIZE)

// Use nlohmann::json for easy, header-only, C++17/20-friendly JSON parsing
using json = nlohmann::json;

/* Replayer configuration struct */
struct ReplayerConfig {
    std::string filePath;
    std::string destIp;
    uint16_t destPort;
    double replaySpeed;
    int cpuCore;
    bool stressTest;
    size_t numMessages;
};

/**
 * @brief Load Replayer configuration from JSON file
 * @param path Path to JSON config
 * @throws std::runtime_error if file cannot be opened
 */
ReplayerConfig loadReplayerConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    json configJson;
    file >> configJson;

    const auto& sharedJson   = configJson["shared"];
    const auto& replayerJson = configJson["replayer"];

    ReplayerConfig cfg{};
    cfg.filePath    = replayerJson.value("file_path", "totalview_capture.itch");
    cfg.destIp      = replayerJson.value("dest_ip", "127.0.0.1");
    cfg.destPort    = sharedJson.value("udp_port", 5555);                // shared with listener
    cfg.replaySpeed = replayerJson.value("replay_speed", 1.0);
    cfg.cpuCore     = replayerJson.value("cpu_core", 0);
    cfg.stressTest  = replayerJson.value("stress_test", false);
    cfg.numMessages = sharedJson.value("num_messages_to_send", 10000);   // default messages

    return cfg;
}

int main() {
    try {
        // --- Load configuration ---
        ReplayerConfig cfg = loadReplayerConfig("config.json");

        // --- Create and start the replayer ---
        ItchMessageUdpReplayer replayer(
            cfg.filePath,
            cfg.destIp,
            cfg.destPort,
            cfg.replaySpeed,
            cfg.cpuCore
        );

        replayer.start();

        // --- Wait until all messages are sent ---
        while (!replayer.finished()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        replayer.stop();
        std::cout << "[Replayer] Replay complete.\n";

    } catch (const std::exception& e) {
        std::cerr << "[Replayer] Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}