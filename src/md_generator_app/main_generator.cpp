// ---------------------------------------------------------------------------
// File        : main_generator.cpp
// Project     : HftSimulator
// App         : Market Data Generator
// Description : Generates ITCH-format messages for simulation
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <getopt.h>

#include "itch_message.h"

/// Default settings
constexpr size_t DEFAULT_NUM_MESSAGES = 10000;
constexpr bool   DEFAULT_STRESS_TEST  = false;
constexpr double DEFAULT_SPEED_FACTOR = 1.0;
const std::string SYMBOL = "MSFT";

/**
 * @brief Print usage information
 */
void printUsage(const std::string& progName) {
    std::cout << "Usage: " << progName << " [--count=N] [--stress_test=T/F] [--speed_factor=X]\n";
    std::cout << "\n";
    std::cout << "Generates ITCH-format messages and writes to top-level project/data directory.\n\n";
    std::cout << "Optional arguments:\n";
    std::cout << "  --count=N         Number of messages to generate (default: " << DEFAULT_NUM_MESSAGES << ")\n";
    std::cout << "  --stress_test=T/F Whether to enable stress test mode (default: F)\n";
    std::cout << "  --speed_factor=X  Replay speed factor (default: " << DEFAULT_SPEED_FACTOR << ")\n";
    std::cout << "  --help            Show this help message\n";
}

/**
 * @brief Parse command line arguments
 */
bool parseArguments(int argc, char* argv[],
                    size_t& numMessages,
                    bool& stressTest,
                    double& speedFactor) {

    static struct option long_options[] = {
        {"count",        required_argument, 0, 'c'},
        {"stress_test",  required_argument, 0, 's'},
        {"speed_factor", required_argument, 0, 'f'},
        {"help",         no_argument,       0, 'h'},
        {0,0,0,0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "c:s:f:h", long_options, &option_index)) != -1) {
        switch(opt) {
            case 'c':
                try { numMessages = std::stoul(optarg); }
                catch (...) { std::cerr << "[Error] Invalid count value: " << optarg << "\n"; return false; }
                break;
            case 's': {
                std::string val = optarg;
                std::transform(val.begin(), val.end(), val.begin(), ::tolower);
                if (val == "t" || val == "true") stressTest = true;
                else if (val == "f" || val == "false") stressTest = false;
                else { std::cerr << "[Error] Invalid stress_test value: " << optarg << "\n"; return false; }
                break;
            }
            case 'f':
                try { speedFactor = std::stod(optarg); }
                catch (...) { std::cerr << "[Error] Invalid speed_factor value: " << optarg << "\n"; return false; }
                break;
            case 'h':
                printUsage(argv[0]);
                return false;
            default:
                printUsage(argv[0]);
                return false;
        }
    }
    return true;
}

/**
 * @brief Generate ITCH messages
 */
std::vector<ItchMessage> generateMessages(size_t numMessages) {
    std::vector<ItchMessage> messages;
    messages.reserve(numMessages);

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint32_t> priceDist(10000, 20000);
    std::uniform_int_distribution<uint32_t> sizeDist(1, 100);

    for (size_t i = 0; i < numMessages; ++i) {
        ItchMessage msg{};
        std::memset(msg.symbol, 0, sizeof(msg.symbol));
        std::memcpy(msg.symbol, SYMBOL.c_str(), SYMBOL.size());

        msg.price = static_cast<double>(priceDist(rng));
        msg.size  = sizeDist(rng);
        msg.side  = (i % 2 == 0) ? Side::Buy : Side::Sell;

        auto now = std::chrono::steady_clock::now();
        msg.tsNanoSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                now.time_since_epoch()).count();

        messages.push_back(msg);
    }

    return messages;
}

/**
 * @brief Generate filename based on parameters
 */
std::string generateFilename(size_t numMessages, bool stressTest, double speedFactor) {
    std::string fname = SYMBOL;
    fname += "-c" + std::to_string(numMessages);
    fname += "-s" + std::string(stressTest ? "T" : "F");
    fname += "-p" + std::to_string(speedFactor);
    fname += ".itch";

    // Determine top-level project/data directory relative to source file
    std::filesystem::path outputDir = std::filesystem::path(__FILE__).parent_path() // src/md_generator_app
                                     .parent_path() // src
                                     .parent_path() // project root
                                     / "data";
    std::filesystem::create_directories(outputDir); // ensure it exists

    return (outputDir / fname).string();
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    size_t numMessages = DEFAULT_NUM_MESSAGES;
    bool stressTest = DEFAULT_STRESS_TEST;
    double speedFactor = DEFAULT_SPEED_FACTOR;

    if (!parseArguments(argc, argv, numMessages, stressTest, speedFactor)) {
        return 1;
    }

    std::vector<ItchMessage> messages = generateMessages(numMessages);

    std::string filename = generateFilename(numMessages, stressTest, speedFactor);

    try {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs.is_open()) {
            std::cerr << "[Error] Failed to open output file: " << filename << "\n";
            return 1;
        }

        for (const auto& msg : messages) {
            msg.serialize(ofs);
        }

        std::cout << "[Generator] Successfully wrote " << messages.size()
                  << " messages to " << filename << "\n";

    } catch (const std::exception& e) {
        std::cerr << "[Error] Exception while writing file: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
