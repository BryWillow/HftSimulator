#pragma once

#include <arpa/inet.h>
#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <iostream>
#include "spsc_ringbuffer.h"
#include "itch_message.h"

class UdpItchListener {
public:
    UdpItchListener(uint16_t udpPort, SpScRingBuffer<ItchMessage>& buffer)
        : _udpListenPort(udpPort), _ringBuffer(buffer) {}

    /// @brief Start listening for market data.
    /// @param Client sets stopFlag to true which gracefully closes the socket.
    void run(std::atomic<bool>& stopFlag) {
        int socketDescriptor = setupSocket();

        while (!stopFlag.load(std::memory_order_relaxed)) {
            ItchMessage msg{};
            ssize_t received = recvfrom(socketDescriptor, &msg, sizeof(msg),
                                        MSG_DONTWAIT, nullptr, nullptr);
            if (received == sizeof(msg)) {
                if (!_ringBuffer.tryPush(msg)) {
                  // We don't want pass along stale market data.
                  // But we do need to provide an alert.
                  // TODO: we don't have a logging or an alert framework.
                  std::cout << "Market data is stale. Dropping packet." << std::endl;
                } 
            } else if (received < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                // We haven't received any market data to process. Let other threads run.
                std::this_thread::yield();
            }
        }

        close(socketDescriptor);
    }

private:
    int setupSocket() {
        int socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketDescriptor < 0)
            throw std::runtime_error("Failed to create socket");

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(_udpListenPort);

        if (bind(socketDescriptor, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            close(socketDescriptor);
            throw std::runtime_error("Failed to bind UDP socket");
        }

        return socketDescriptor;
    }

    uint16_t _udpListenPort;
    SpScRingBuffer<ItchMessage>& _ringBuffer;
};