#pragma once

#include <fstream>
#include <random>
#include <string>
#include <filesystem>
#include "itch_message.h"
#include "captured_message.h"

/**
 * @brief Generates a capture file containing random ITCH messages.
 */
class ItchFileGenerator {
public:
    static void Generate(const std::string& fileName, size_t numMessages) {
        std::filesystem::path dataDir("data");
        if (!std::filesystem::exists(dataDir)) std::filesystem::create_directories(dataDir);

        std::filesystem::path filePath = dataDir / fileName;
        std::ofstream outFile(filePath, std::ios::binary | std::ios::trunc);
        if (!outFile) throw std::runtime_error("Failed to open capture file");

        std::mt19937_64 rng(0xDEADBEEF);
        std::uniform_int_distribution<uint64_t> orderIdDist(1, 1000000);
        std::uniform_int_distribution<uint32_t> priceDist(100, 10000);
        std::uniform_int_distribution<uint32_t> qtyDist(1, 1000);
        std::uniform_int_distribution<uint8_t> sideDist(0, 1);

        uint64_t tsNs = 0;
        uint64_t seqNum = 0;

        for (size_t i = 0; i < numMessages; ++i) {
            ItchMessage msg{};
            msg.orderId = orderIdDist(rng);
            msg.price   = priceDist(rng);
            msg.size    = qtyDist(rng);
            msg.sequenceNumber = ++seqNum;
            msg.tsNanoSeconds   = tsNs;
            msg.side   = (sideDist(rng) == 0 ? Side::Buy : Side::Sell);
            msg.type   = ItchMsgType::AddOrder;

            tsNs += 1 + (rng() % 100);

            CapturedMessage capMsg{};
            capMsg.msg  = msg;
            capMsg.tsNs = tsNs; 

            outFile.write(reinterpret_cast<const char*>(&capMsg), sizeof(capMsg));
        }
    }
};