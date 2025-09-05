#include "spsc_ringbuffer.h"
#include <gtest/gtest.h>
#include <string>


// Example MarketData struct
struct MarketData {
    std::string symbol;
    double price;
    int size;

    MarketData() = default;
    MarketData(std::string s, double p, int sz)
        : symbol(std::move(s)), price(p), size(sz) {}
};

// Test fixture (reuses buffer in each test)
class RingBufferTest : public ::testing::Test {
protected:
    SpScRingBuffer<MarketData, 4> buffer; // capacity 4
};

// --- Basic push/pop test ---
TEST_F(RingBufferTest, PushPopSingle) {
    MarketData md{"AAPL", 150.0, 100};
    EXPECT_TRUE(buffer.tryPush(md));

    MarketData out;
    EXPECT_TRUE(buffer.tryPop(out));
    EXPECT_EQ(out.symbol, "AAPL");
    EXPECT_DOUBLE_EQ(out.price, 150.0);
    EXPECT_EQ(out.size, 100);
}

// --- Buffer empty/full states ---
TEST_F(RingBufferTest, EmptyAndFull) {
    EXPECT_TRUE(buffer.empty());
    EXPECT_FALSE(buffer.full());

    EXPECT_TRUE(buffer.tryPush({"GOOG", 2800.0, 50}));
    EXPECT_FALSE(buffer.empty());
    EXPECT_FALSE(buffer.full());

    // Fill the rest
    EXPECT_TRUE(buffer.tryPush({"MSFT", 300.0, 25}));
    EXPECT_TRUE(buffer.tryPush({"TSLA", 700.0, 10}));
    EXPECT_TRUE(buffer.full()); // capacity 4, last push filled it
}

// --- Overflow protection ---
TEST_F(RingBufferTest, Overflow) {
    EXPECT_TRUE(buffer.tryPush({"AAPL", 150.0, 1}));
    EXPECT_TRUE(buffer.tryPush({"GOOG", 2800.0, 2}));
    EXPECT_TRUE(buffer.tryPush({"MSFT", 300.0, 3}));
    EXPECT_TRUE(buffer.tryPush({"TSLA", 700.0, 4}));
    EXPECT_FALSE(buffer.tryPush({"NFLX", 500.0, 5})); // should fail
}

// --- Underflow protection ---
TEST_F(RingBufferTest, Underflow) {
    MarketData out;
    EXPECT_FALSE(buffer.tryPop(out)); // nothing to pop
}

// --- Push/Pop order (FIFO) ---
TEST_F(RingBufferTest, MaintainsOrder) {
    buffer.tryPush({"AAPL", 150.0, 1});
    buffer.tryPush({"GOOG", 2800.0, 2});
    buffer.tryPush({"MSFT", 300.0, 3});

    MarketData out;
    buffer.tryPop(out);
    EXPECT_EQ(out.symbol, "AAPL");
    buffer.tryPop(out);
    EXPECT_EQ(out.symbol, "GOOG");
    buffer.tryPop(out);
    EXPECT_EQ(out.symbol, "MSFT");
}