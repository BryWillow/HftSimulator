#include "pinned_thread.h"
#include <gtest/gtest.h>
#include <string>

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