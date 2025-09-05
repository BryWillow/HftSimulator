#include <cstdint>  // for uint_32, uint_64
#include <unistd.h> // fpr size_t 

struct ItchMessage
{
    char msgType;
    char symbol[8];
    std::uint64_t orderId;
    std::uint32_t price;
    std::uint32_t quantity;
    char side;
};

constexpr size_t MarketDataMessageSize = sizeof(ItchMessage);
constexpr size_t MessateTypeSizeBytes = sizeof(ItchMessage::msgType);
constexpr size_t SymbolSizeBytes = sizeof(ItchMessage::symbol);
constexpr size_t OrderIdSizeBytes = sizeof(ItchMessage::orderId);
constexpr size_t PriceSizeBytes = sizeof(ItchMessage::price);
constexpr size_t QuantitySizeBytes = sizeof(ItchMessage::quantity);
constexpr size_t SideSizeBytes = sizeof(ItchMessage::side);

constexpr size_t MessageTypeOffset = 0;
constexpr size_t MessageSizeOffset = 1;
constexpr size_t SymbolOffset = 3;
constexpr size_t OrderIdOffset = 11;
constexpr size_t PriceOffset = 19;
constexpr size_t QuantityOffset = 23;
constexpr size_t SideOffset = 27;