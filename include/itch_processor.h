#pragma once 

// #include <thread>
// #include <array>
// #include <cstdint>
// #include "SpScRingBuffer.h"

// // Simplfied, parsed ITCH message returned by the algo's strategy layer.
// //
// // Message Types :
// // 'A' = Add     : Add liquidity to current price.
// // 'E' = Execute : Aggressive, usually crossing the bid/ask spread.
// // 'X' = Cancel  : Cancels the specified order.
// //
// // Symbol        : "AAPL, "MSFT", etc.
// // Order ID      : Returned by exchange on add/execute/modify.
// // Price         : You submit.
// // Quatity       : You submit.
// // Side          : 'B' = Buy, 'S' = Sell.
// struct ITCHMessage {
//     char msgType;
//     char symbol[8];
//     std::uint64_t orderId;
//     std::uint32_t price;
//     std::uint32_t quantity;
//     char side;
// };

// SpScRingBuffer<std::array<uint_8, 2048>, 128) RawPacketBuffer;

// // Simulate reading raw UDP messages from the exchange.
// void UdpExchangeThread()
// {
//     // Spin loop, pops market data off the ring buffer as it becomes available.
//     while(true) {
//         std::array<uint8_t,2048> packet{};
//         size_t offset=0;
//         for(size_t i = 0;i < PACKET_BATCH; i++) {
//             packet[offset]= 'A'; offset+=1;
//             std::memcpy(packet.data()+offset, symbols[rng()%MAX_SYMBOLS],8); offset+=8;
//             *reinterpret_cast<uint64_t*>(packet.data()+offset) = nextOrderId++; offset+=8;
//             *reinterpret_cast<uint32_t*>(packet.data()+offset) = priceDist(rng); offset+=4;
//             *reinterpret_cast<uint32_t*>(packet.data()+offset) = qtyDist(rng); offset+=4;
//             packet[offset]= sideDist(rng)?'B':'S'; offset+=1;
//         }
//         RawPacketBuffer.push(packet);
//         _mm_pause();
//         std::this_thread::sleep_for(std::chrono::microseconds(50));
//     }

//   RawPacketBuffer.tryPush()
// }

// void ItchParserThread()
// {
//     // Simulate parsing raw ITCH packets into ITCHMessage structs.
//     while(true) {
//         std::array<uint8_t,2048> packet;
//         while(!RawPacketBuffer.tryPop(packet)) {
//             _mm_pause(); // Light-weight yield
//         }
//         size_t offset=0;
//         for(size_t i=0;i<PACKET_BATCH;i++) {
//             ITCHMessage msg;
//             msg.msgType = packet[offset]; offset+=1;
//             std::memcpy(msg.symbol, packet.data()+offset, 8); offset+=8;
//             msg.orderId = *reinterpret_cast<uint64_t*>(packet.data()+offset); offset+=8;
//             msg.price = *reinterpret_cast<uint32_t*>(packet.data()+offset); offset+=4;
//             msg.quantity = *reinterpret_cast<uint32_t*>(packet.data()+offset); offset+=4;
//             msg.side = packet[offset]; offset+=1;

//             // Process the parsed ITCHMessage (e.g., update order book).
//             ProcessItchMessage(msg);
//         }
//     }
//   RawPacketBuffer.tryPop();
// }