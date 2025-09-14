#pragma once
#include <cstdint>
#include <string>

#if defined(__x86_64__)
#include <immintrin.h> // SSE/AVX instructions if needed
#endif

// ITCH message definitions
// Clean layout, compact, HFT-optimized


/**
 * @file itch_message.h
 * @brief Represents a TotalView ITCH message in a cache-line friendly format.
 *
 * Notes:
 * - Single struct for all message types
 * - Fixed-size fields; no dynamic memory
 * - Aligned to avoid false sharing
 */

/// @brief Buy/Sell side
enum class Side : uint8_t {
    Buy  = 0,
    Sell = 1,
    Unknown = 255 // distinct invalid value for testing.
};

/// @brief ITCH message type codes
enum class ItchMsgType : uint8_t {
    AddOrder      = 'A',
    AddOrderMP    = 'F',  /// MPID (optional ID of who submitted the order)
    OrderExecuted = 'E',
    OrderCancel   = 'X',
    Trade         = 'P',
    Unknown       = 0
};

/**
 * @brief Single ITCH Message.
 * @note  Fits in a cache line (typically 64 bytes on mordern hardware).
 *        (48 bytes, 16 bytes forced padding due to alignas(64) => 1 cache line).
 */
struct alignas(64) ItchMessage {
    ItchMsgType type{ItchMsgType::Unknown}; ///< Order Message type
    uint32_t orderId{0};                    ///< Order ID
    char symbol[8]{0};                      ///< Order Symbol (w/ padding)
    uint32_t size{0};                       ///< Order Quantity
    double price{0.0};                      ///< Order Price
    Side side{Side::Unknown};               ///< Order Side
    uint64_t tsNanoSeconds{0};              ///< Order Timestamp in ns
    uint64_t sequenceNumber{0};             ///< Order Sequence Number

    // --- Helpers ---
    std::string symbolStr() const { return std::string(symbol, strnlen(symbol, 8)); }

    bool isAddOrder() const { return type == ItchMsgType::AddOrder || type == ItchMsgType::AddOrderMP; }
    bool isExecuted() const { return type == ItchMsgType::OrderExecuted; }
    bool isCanceled() const { return type == ItchMsgType::OrderCancel; }
    bool isTrade()    const { return type == ItchMsgType::Trade; }

    /// @brief Converts Network Byte Order (Big-Endian) => Host Byte Order (Little-Endian)
    void toHostOrder() {
        orderId = ntohl(orderId);
        size    = ntohl(size);
        sequenceNumber = ntohll(sequenceNumber);
        tsNanoSeconds    = ntohll(tsNanoSeconds);
        // price and char arrays don't need conversion
    }

    /// @brief Converts Host Byte Order (Little-Endian) => Network Byte Order
    void toNetworkOrder() {
        orderId = htonl(orderId);
        size    = htonl(size);
        sequenceNumber = htonll(sequenceNumber);
        tsNanoSeconds    = htonll(tsNanoSeconds);
        // price and char arrays don't need conversion        
    }
};