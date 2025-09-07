#include <iostream>
#include <atomic>
#include <thread>
#include <cassert>
#include <vector>
#include "hft_pinned_thread.h"
#include "spsc_ringbuffer.h"
#include "itch_connection.h"
#include "itch_message.h"
#include "itch_sender.h"

int main()
{
    // Market Data:
    // CME / BrokerTec: CME FIX/SBE (MDP 3.0) -- much faster to parse
    // eSpeed: NSDQ ITCH-style Binary
    // Both are multicast feeds with TCP recovery.

    // Ring buffer - let's go with 2048 bytes instead of the default.
    SpScRingBuffer<ItchMessage, 2048> marketDataQueue;

    // Producer: Wrte market data to the SPSC ring buffer.
    HftPinnedThread producer([&](std::atomic<bool>& stopFlag) {
        int i = 0;
        while (!stopFlag.load(std::memory_order_relaxed)) {
            ItchMessage msg;
            msg.msgType = 'A';
            msg.orderId = 1;
            msg.price = 1;
            msg.quantity = 5;
            msg.side = 5;
            strcpy(msg.symbol, "AAPL");
            marketDataQueue.tryPush(std::move(msg));
            ++i;
        }
    }, 0); // pinned to core 0

    // Consumer: Readd market data from the SPSC ring buffer.
    HftPinnedThread consumer([&](std::atomic<bool>& stopFlag) {
        ItchMessage msg{};
        while (!stopFlag.load(std::memory_order_relaxed)) {
            while (marketDataQueue.tryPop(msg)) {
                std::cout << "Price: " << msg.price << ", Symbol: " << msg.symbol << "\n";
                //Execute strategy code.
            }
        }
    }, 1); // pinned to core 1

    // Let threads run briefly
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Stop both threads safely
    producer.stop();
    consumer.stop();

    return 0;
}