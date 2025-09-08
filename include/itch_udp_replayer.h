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
#include <filesystem>
#include "itch_message.h"
#include "itch_file_generator.h"


//struct CapturedMsg {
//    std::uint64_t tsNs;
//    ItchMessage msg;
//};

class UdpMessageReplayer {
public:
    UdpMessageReplayer(const std::string& fileName,
                       const std::string& destIp,
                       uint16_t destPort,
                       double speedFactor = 1.0)
        : _fileName(fileName),
          _destIp(destIp),
          _destPort(destPort),
          _speedFactor(speedFactor)
    {
        std::filesystem::path dataDir("data");
        if (!std::filesystem::exists(dataDir)) {
            std::filesystem::create_directories(dataDir);
        }

        _filePath = dataDir / _fileName;

        // Generate capture file if missing
        if (!std::filesystem::exists(_filePath)) {
            std::cout << _filePath << " not found. Generating capture file..." << std::endl;
            GenerateItchCaptureFile(_fileName, 5000); // default 5000 messages
        }
    }

    void replay() {
        if (!setupSocket()) return;

        std::ifstream inputFile(_filePath, std::ios::binary);
        if (!inputFile) {
            std::cerr << "Could not open capture file: " << _filePath << "\n";
            return;
        }

        CapturedMsg msg;
        std::uint64_t firstTs = 0;
        auto replayStart = std::chrono::steady_clock::now();

        while (inputFile.read(reinterpret_cast<char*>(&msg), sizeof(msg))) {
            if (firstTs == 0) firstTs = msg.tsNs;

            std::uint64_t deltaNs = static_cast<std::uint64_t>((msg.tsNs - firstTs) / _speedFactor);
            auto targetTime = replayStart + std::chrono::nanoseconds(deltaNs);

            std::this_thread::sleep_until(targetTime);

            ssize_t sent = sendto(_socketDescriptor, &msg.msg, sizeof(msg.msg), 0,
                                  reinterpret_cast<sockaddr*>(&_destAddr), sizeof(_destAddr));
            if (sent < 0) {
                perror("sendto");
                break;
            }
        }

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

    std::filesystem::path _filePath;
    std::string _fileName;
    std::string _destIp;
    uint16_t _destPort;
    double _speedFactor;

    int _socketDescriptor = -1;
    sockaddr_in _destAddr{};
};