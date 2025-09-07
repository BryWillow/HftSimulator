#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <cstring>

/// @brief ItchMessage is parsed/decoded market data message used by our entire app.
///        This is intentionally called 'ItchMessage' in case other non-Itch formats
///        are added in the future that might contain additional information we find useful.
#pragma pack(push, 1)  // strict layout, no padding
struct ItchMessage {
    char msgType;
    char symbol[8];
    std::uint64_t orderId;
    std::uint32_t price;
    std::uint32_t quantity;
    char side;
};
#pragma pack(pop)

constexpr size_t MarketDataMessageSize = sizeof(ItchMessage);

constexpr size_t SymbolOffset   = offsetof(ItchMessage, symbol);
constexpr size_t OrderIdOffset  = offsetof(ItchMessage, orderId);
constexpr size_t PriceOffset    = offsetof(ItchMessage, price);
constexpr size_t QuantityOffset = offsetof(ItchMessage, quantity);
constexpr size_t SideOffset     = offsetof(ItchMessage, side);

// Simple compile time sanity checks.
static_assert(MarketDataMessageSize == 1 + 8 + 8 + 4 + 4 + 1, "Unexpected ItchMessage struct size");
static_assert(SymbolOffset == 1, "Unexpected symbol offset");