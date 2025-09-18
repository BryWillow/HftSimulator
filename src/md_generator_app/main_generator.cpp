// ---------------------------------------------------------------------------
// File        : main_generator.cpp
// Project     : HftSimulator
// App         : Market Data Generator
// Description : Generates ITCH messages for simulation
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#include <random>
#include <chrono>
#include <fstream>
#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
#include "itch_message.h"

namespace fs = std::filesystem;

/**
 * @brief Generate ITCH messages and write to file
 * @param filePath Full path to output file
 * @param numMessages Number of messages to generate
 */
void generateMessages(const fs::path &filePath, size_t numMessages)
{
  constexpr std::string_view SYMBOL = "MSFT";

  std::vector<ItchMessage> messages;
  messages.reserve(numMessages);

  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<uint32_t> priceDist(10000, 20000);
  std::uniform_int_distribution<uint32_t> sizeDist(1, 100);

  for (size_t i = 0; i < numMessages; ++i)
  {
    ItchMessage msg{};
    std::memset(msg.symbol, 0, sizeof(msg.symbol));
    std::memcpy(msg.symbol, SYMBOL.data(), SYMBOL.size());

    msg.price = static_cast<double>(priceDist(rng));
    msg.size = sizeDist(rng);
    msg.side = (i % 2 == 0) ? Side::Buy : Side::Sell;

    auto now = std::chrono::steady_clock::now();
    msg.tsNanoSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            now.time_since_epoch())
                            .count();

    messages.push_back(msg);
  }

  // Write to file
  std::ofstream ofs(filePath, std::ios::binary);
  if (!ofs)
  {
    std::cerr << "[Generator] Error: Failed to open file: " << filePath << "\n";
    return;
  }

  for (const auto &msg : messages)
    msg.serialize(ofs);

  std::cout << "[Generator] Generated " << numMessages << " messages to " << filePath << "\n";
}

/**
 * @brief Main generator entry point
 */
int main()
{
  try
  {
    // --- Determine project root and /data directory ---
    fs::path projectRoot = fs::path(__FILE__).parent_path().parent_path().parent_path();
    fs::path dataDir = projectRoot / "data";

    if (!fs::exists(dataDir))
      fs::create_directories(dataDir);

    fs::path defaultFile = dataDir / "default.itch";

    // --- Generate default.itch if it doesn't exist ---
    if (!fs::exists(defaultFile))
    {
      std::cout << "[Generator] default.itch not found, generating...\n";
      generateMessages(defaultFile, 10000);
    }
    else
    {
      std::cout << "[Generator] default.itch already exists.\n";
    }

    // --- Here you could handle command-line args for additional files ---
    // Example: --count=50000 --stress_test=true --speed_factor=1.5
    // (not shown here, default behavior ensures default.itch exists)
  }
  catch (const std::exception &e)
  {
    std::cerr << "[Generator] Exception: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
