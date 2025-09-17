// ---------------------------------------------------------------------------
// File        : micro_mean_reversion_strategy.h
// Project     : HftSimulator
// Description : A simple micro mean-reversion strategy for demonstration
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <deque>
#include <numeric>
#include <cstddef>
#include <mutex>

// ---------------------------------------------------------------------------
// MicroMeanReversionStrategy
//   - Computes a short-term moving average of the last N prices
//   - Buys when price drops below the moving average
//   - Sells when price rises above the moving average
// ---------------------------------------------------------------------------
template <typename PriceType>
class MicroMeanReversionStrategy {
public:
    explicit MicroMeanReversionStrategy(std::size_t window_size)
        : window_size_(window_size)
    {}

    // Feed a new price and get an action: -1 = sell, 0 = hold, 1 = buy
    int on_new_price(PriceType price) {
        std::lock_guard<std::mutex> lock(mutex_);

        prices_.push_back(price);
        if (prices_.size() > window_size_) {
            prices_.pop_front();
        }

        if (prices_.size() < window_size_) {
            return 0; // not enough data yet
        }

        PriceType avg = std::accumulate(prices_.begin(), prices_.end(), PriceType(0)) /
                        static_cast<PriceType>(prices_.size());

        if (price < avg) return 1;   // buy
        if (price > avg) return -1;  // sell
        return 0;                     // hold
    }

private:
    std::size_t window_size_;
    std::deque<PriceType> prices_;
    std::mutex mutex_;  // simple thread safety if multiple threads feed prices
};