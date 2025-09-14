#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>

#include "itch_message_udp_replayer.h"

/**
 * @file main.cpp
 * @brief Replay captured ITCH messages over UDP to simulate market data.
 *
 * Notes:
 * - Uses pinned threads for realistic HFT simulation.
 * - Preserves timestamps and burst patterns from captured files.
 * - Supports configurable replay speed.
 */

int main() {
    std::string fileName = "data.itch";
    std::string destIp = "127.0.0.1";
    uint16_t destPort = 5555;
    double speedFactor = 1.0;
    int cpuCore = 0;

    ItchMessageUdpReplayer replayer(fileName, destIp, destPort, speedFactor, cpuCore);
    replayer.start();

    // sleep or wait
    std::this_thread::sleep_for(std::chrono::seconds(10));

    replayer.stop();
    return 0;
}