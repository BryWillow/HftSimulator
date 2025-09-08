#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <thread>
#include <string>
#include "itch_message.h"

// Encapsulates captured messages with timestamp (nanoseconds)
struct CapturedMsg {
    std::uint64_t ts; // capture timestamp in ns
    ItchMessage msg;
};

class UdpMessageReplayer {
public:
    UdpMessageReplayer(const std::string& filePath,
                const std::string& destIp,
                uint16_t destPort,
                double speedFactor = 1.0)
        : _filePath(filePath),
          _destIp(destIp),
          _destPort(destPort),
          _speedFactor(speedFactor)
    {}

    // Plays the captured messages at the original timing (scaled by speedFactor)
    void replay() {
        if (!setupSocket()) return;

        std::ifstream inputFile(_filePath, std::ios::binary);
        if (!inputFile) {
            std::cerr << "Could not open capture file: " << _filePath << "\n";
            return;
        }

        // Don't simply blast market data - play it back at the specified rate.        
        // Use the current time as the "seed" time.
        CapturedMsg msg;
        std::uint64_t firstTs = 0;
        auto replayStart = std::chrono::steady_clock::now();

        while (inputFile.read(reinterpret_cast<char*>(&msg), sizeof(msg))) {
            if (firstTs == 0) firstTs = msg.ts;

            // Compute the time to send the next market data packet.
            std::uint64_t deltaNs = static_cast<std::uint64_t>(
                (msg.ts - firstTs) / _speedFactor
            );

            // Wait until it's time to send the next packet.
            auto targetTime = replayStart + std::chrono::nanoseconds(deltaNs);
            std::this_thread::sleep_until(targetTime);

            // Send the next packet.
            ssize_t sent = sendto(_socketDescriptor, &msg.msg, sizeof(msg.msg), 0,
                                  reinterpret_cast<sockaddr*>(&_destAddr), sizeof(_destAddr));
            if (sent < 0) {
                perror("sendto");
                break;
            }
        }

        // We've sent all the entries in the file.
        close(_socketDescriptor);
        _socketDescriptor = -1;
    }

private:
    bool setupSocket() {
        _socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
        if (_socketDescriptor < 0) {
            perror("socket");
            return false;
        }

        _destAddr.sin_family = AF_INET;
        _destAddr.sin_port = htons(_destPort);
        if (inet_pton(AF_INET, _destIp.c_str(), &_destAddr.sin_addr) <= 0) {
            std::cerr << "Invalid destination IP: " << _destIp << "\n";
            close(_socketDescriptor);
            _socketDescriptor = -1;
            return false;
        }

        return true;
    }

    std::string _filePath;
    std::string _destIp;
    uint16_t _destPort;
    double _speedFactor; // 1.0 = real-time, 2.0 = 2x faster

    int _socketDescriptor = -1;
    sockaddr_in _destAddr{};
};