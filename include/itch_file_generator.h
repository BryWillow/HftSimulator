#pragma once
#include <fstream>
#include <string>
#include <random>
#include <cstring>
#include <iostream>
#include <filesystem>
#include "itch_message.h"

struct CapturedMsg {
    uint64_t tsNs;
    ItchMessage msg;
};

inline void GenerateItchCaptureFile(const std::string& fileName, size_t numMessages = 10000) {
    // Ensure 'data/' directory exists
    std::filesystem::path dataDir("data");
    if (!std::filesystem::exists(dataDir)) {
        std::filesystem::create_directories(dataDir);
    }

    std::filesystem::path filePath = dataDir / fileName;

    std::ofstream out(filePath, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open " << filePath << " for writing\n";
        return;
    }

    std::mt19937_64 rng(42);
    std::uniform_int_distribution<uint32_t> priceDist(10000, 20000);
    std::uniform_int_distribution<uint32_t> qtyDist(1, 1000);
    std::uniform_int_distribution<int> sideDist(0, 1); // use int instead of char

    const char* symbols[] = {"AAPL", "MSFT", "GOOG", "AMZN", "TSLA"};
    const size_t symbolCount = sizeof(symbols)/sizeof(symbols[0]);

    uint64_t ts = 0;
    const uint64_t dt = 1'000'000; // 1 ms between messages

    for (size_t i = 0; i < numMessages; ++i) {
        CapturedMsg msg{};
        msg.tsNs = ts;
        ts += dt;

        msg.msg.msgType = 'D';
        const char* sym = symbols[i % symbolCount];
        std::memset(msg.msg.symbol, ' ', 8);
        std::memcpy(msg.msg.symbol, sym, std::strlen(sym));
        msg.msg.orderId = i + 1;
        msg.msg.price = priceDist(rng);
        msg.msg.quantity = qtyDist(rng);
        msg.msg.side = sideDist(rng) ? 'B' : 'S'; // cast int to char

        out.write(reinterpret_cast<const char*>(&msg), sizeof(msg));
    }

    out.close();
    std::cout << "Generated " << numMessages << " messages to " << filePath << "\n";
}