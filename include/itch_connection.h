// #include <string>
// #include <sys/socket.h> // for creating sockets
// #include <netinet/in.h> // for receiving messages
// #include <unistd.h>     // for closint sockets 
// #include <arpa/inet.h>
// #include <netinet/in.h>
// #include <iostream>
// #include "ItchMessage.h"

// // Maximum UDP packet size (arbitrary large enough)
// // TODO: Make this configurable?
// constexpr size_t MAX_PACKET_SIZE = 4096;



// class ItchConnection 
// {
//   private:
//     unsigned short _udpPort;
//     std::string _udpAddress;

//   public:
//     ItchConnection(unsigned short udpPort, const std::string& udpAddress) 
//     : _udpPort(udpPort), _udpAddress(udpAddress) {}

//     int Start()
//     {
//       // Create a UDP socket for processing ITCH messages.
//       int itchSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
//       if (itchSocketFd < 0) 
//       {
//           std::cerr << std::format("Failed to allocate new ITCH socket: {}\n", itchSocketFd);
//           return -1;
//       }

//       // Binding properties to receive UDP data.
//       sockaddr_in serverAddress{};
//       serverAddress.sin_family = AF_INET;         // Address Family Internet - IPv4 (not familiar with IPv6).
//       serverAddress.sin_port = htons(_udpPort);   // The port to listen for UDP. 5000 is simply an example. CHANGE THIS.
//       serverAddress.sin_addr.s_addr = INADDR_ANY; // Listen for UDP on all NICs. We can bind to a specific NIC if need be.

//       if (bind(itchSocketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) 
//       {
//           std::cerr << "Bind failed\n";
//           close(itchSocketFd);
//           return 1;
//       }

//       // We can join a specific multicast group
//       // ip_mreq mreq{};
//       // mreq.imr_multiaddr.s_addr = inet_addr("239.1.2.3");  // Multicast group
//       // mreq.imr_interface.s_addr = htonl(INADDR_ANY);       // Default interface

//       uint8_t receiveBuffer[MAX_PACKET_SIZE];
//       sockaddr_in clientOrigin{};
//       socklen_t clientOriginLength = sizeof(clientOrigin);

//       // We need to spin here. Will there be copying / invalidating
//       while (true) 
//       {
//         int bytesReceived = recvfrom(itchSocketFd, receiveBuffer, MAX_PACKET_SIZE, 
//           0, // Out of Band data or MSG_PEEK => Need to investigate.                           
//           (struct sockaddr*)&clientOrigin, &clientOriginLength); 
//         if (bytesReceived > 0) 
//         {
//             ItchMessage itchMessage;
//             parseMessage(receiveBuffer, bytesReceived, itchMessage);

//             // push this onto a ring buffer for processing by another thread.
//         } 
//       }

//       close(itchSocketFd);
//       return 0;
//     }

//     // Real-world ITCH parsers are much more complex.
//     void parseMessage(const uint8_t* data, size_t len, ItchMessage& itchMessage) 
//     {
//         size_t offset = 0;

//         // Messages can be batched together.
//         while (offset < len) 
//         {
//           // Every ITCH message starts with 1-byte type.
//   char msgType = data[                                                                                                                                                         ];
//           offset += 1;

//           // The next 2 bytes are message length (excluding type and length)
//           if (offset + 2 > len) break;

//           uint16_t msgLength;
//           std::memcpy(&msgLength, data + offset, 2);
//           msgLength = ntohs(msgLength); // network to host
//           offset += 2;

//           // Check if full message is available
//           if (offset + msgLength > len) break;
//           offset += msgLength;

//           std::memcpy(&itchMessage.msgType, data + MessageTypeOffset, MessateTypeSizeBytes);
//           std::memcpy(&itchMessage.symbol, data + SymbolOffset, SymbolSizeBytes);
//           std::memcpy(&itchMessage.orderId, data + OrderIdOffset, OrderIdSizeBytes);
//           std::memcpy(&itchMessage.price, data + PriceOffset, PriceSizeBytes);
//           std::memcpy(&itchMessage.quantity, data + QuantityOffset, QuantitySizeBytes);
//           std::memcpy(&itchMessage.side, data + SideOffset, SideSizeBytes);

//           // TODO: push this onto a ring buffer
//         }
//       }
//   };