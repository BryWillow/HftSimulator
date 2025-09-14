/*
#pragma once
#include <atomic>
#include <unordered_map>
#include <string>
#include <iostream>
#include "itch_message.h"

/// @brief A very simple strategy to illustrate the architecture of this system.
///        It simply keeps track of the # of buys/sells and last price for each symbol.
class SimpleStrategy {
public:
    SimpleStrategy() = default;

    // Called for each message popped from the ring buffer
    void onMessage(const ItchMessage& msg) {
        std::string symbol(msg.symbol, 8);

        // Track last price
        _lastPrice[symbol] = msg.price;

        // Count buys and sells
        if (msg.side == Side::Buy) _buyCount++;
        else if (msg.side == Side::Sell) _sellCount++;
    }

    void printStats() const {
        std::cout << "Total Buys: " << _buyCount.load() 
                  << ", Total Sells: " << _sellCount.load() << "\n";
        for (const auto& [symbol, price] : _lastPrice) {
            std::cout << symbol << " last price: " << price << "\n";
        }
    }

private:
    std::unordered_map<std::string, uint32_t> _lastPrice;
    std::atomic<int> _buyCount{0};
    std::atomic<int> _sellCount{0};
};
*/
