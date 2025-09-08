#pragma once
#include <atomic>
#include <thread>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "spsc_ringbuffer.h"
#include "itch_message.h"

class UdpItchListener {
public:
    UdpItchListener(uint16_t port, SpScRingBuffer<ItchMessage>& buffer)
        : _port(port), _buffer(buffer) {}

    /// @brief Listen for UDP market data and decode into ItchMessage
    void run(std::atomic<bool>& stopFlag) {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            perror("socket");
            return;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind");
            close(sock);
            return;
        }

        ItchMessage msg;
        while (!stopFlag.load(std::memory_order_relaxed)) {
            ssize_t received = recvfrom(sock, &msg, sizeof(msg), MSG_DONTWAIT, nullptr, nullptr);
            if (received == sizeof(msg)) {
                // Decoded directly into ItchMessage struct
                if (!_buffer.tryPush(std::move(msg))) {
                    std::cout << "Ring buffer full, dropping message\n";
                }
            } else if (received < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                std::this_thread::yield(); // no data yet, let other threads run
            }
        }

        close(sock);
    }

private:
    uint16_t _port;
    SpScRingBuffer<ItchMessage>& _buffer;
};