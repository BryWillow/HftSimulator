// ---------------------------------------------------------------------------
// File        : main_replayer.cpp
// Project     : HftSimulator
// App         : Replayer App
// Description : Replay captured ITCH messages over UDP for simulation
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "json.hpp"                      // JSON parsing using nlohmann::json
#include "itch_message_udp_replayer.h"   // ITCH UDP replayer
#include "constants.h"                   // Global compile-time constants

using json = nlohmann::json;

/// Global stop flag for SIGINT handling
std::atomic<bool> g_stopFlag{false};

/**
 * @brief Signal handler to safely stop the replayer
 */
void signalHandler(int signal) {
    if (signal == SIGINT) {
        g_stopFlag.store(true, std::memory_order_relaxed);
        std::cerr << "\n[Replayer] SIGINT received, stopping replay...\n";
    }
}

/**
 * @brief Replayer configuration struct
 */
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
    cfg.filePath    = replayerJson.value("file_path", "default.itch");
    cfg.destIp      = replayerJson.value("dest_ip", "127.0.0.1");
    cfg.destPort    = sharedJson.value("udp_port", 5555);
    cfg.replaySpeed = replayerJson.value("replay_speed", 1.0);
    cfg.cpuCore     = replayerJson.value("cpu_core", 0);
    cfg.stressTest  = replayerJson.value("stress_test", false);
    cfg.numMessages = sharedJson.value("num_messages_to_send", 10000);

    return cfg;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    try {
        // --- Register SIGINT handler ---
        std::signal(SIGINT, signalHandler);

        // --- Determine top-level project root directory ---
        std::filesystem::path projectRoot = std::filesystem::path(__FILE__).parent_path() // src/replayer_app
                                            .parent_path() // src
                                            .parent_path(); // project root

        // --- Resolve config.json in project root ---
        std::filesystem::path configPath = projectRoot / "config.json";
        if (!std::filesystem::exists(configPath)) {
            std::cerr << "[Replayer] Error: config.json not found at: " << configPath << "\n";
            return 1;
        }

        // --- Load configuration from JSON ---
        ReplayerConfig cfg = loadReplayerConfig(configPath.string());

        // --- Determine top-level project/data directory ---
        std::filesystem::path dataDir = projectRoot / "data";

        // --- Determine input file ---
        std::string inputFile = "default.itch"; // default file
        if (argc > 1) {
            inputFile = argv[1];
        }

        std::filesystem::path filePath = dataDir / inputFile;
        if (!std::filesystem::exists(filePath)) {
            std::cerr << "[Replayer] Error: File does not exist: " << filePath << "\n";
            return 1;
        }

        // Override filePath in config
        cfg.filePath = filePath.string();

        // --- Create replayer ---
        ItchMessageUdpReplayer replayer(
            cfg.filePath,
            cfg.destIp,
            cfg.destPort,
            cfg.replaySpeed,
            cfg.cpuCore
        );

        // --- Load and validate all messages before starting replay ---
        std::cout << "[Replayer] Loading messages from " << cfg.filePath << " ...\n";
        replayer.loadAllMessages();
        std::cout << "[Replayer] Loaded " << cfg.numMessages << " messages.\n";

        // --- Start replay ---
        replayer.start();

        // --- Wait until all messages are sent or SIGINT received ---
        while (!replayer.finished() && !g_stopFlag.load(std::memory_order_relaxed)) {
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
