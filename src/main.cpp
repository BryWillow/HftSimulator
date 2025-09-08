#include <iostream>
#include <atomic>
#include <thread>
#include <cassert>
#include <vector>
#include "hft_pinned_thread.h"
#include "spsc_ringbuffer.h"
#include "hft_pinned_thread.h"
#include "itch_message.h"
#include "itch_udp_replayer.h"
#include "itch_udp_listener.h"

int main()
{
    // 1. Create an SPSC ring buffer quere where:
    //    Producer: UDP Market Data Receiver
    //    Consumer: Strategy Hot Path
    SpScRingBuffer<ItchMessage> buffer;

    // 2. Create the UDP market-data listener.
    //.   The whole point is to simulate receiving market data from an exchange.
    //    The market data "played" back are FIX messages encoded with SBE (Simple Binary Encoding).
    //    The market data is played back using timestamps, that make it intentionally bursty.
    //    This happens on a single thread that's pinned to the CPU core of our choice.
    //    Cam be stopped by setting the stopFlag below.
    UdpItchListener listener(12345, buffer);
    HftPinnedThread listenerThread(
        [&](std::atomic<bool>& stopFlag) { listener.run(stopFlag); }, 0);

    // Setup for sending/playing messages from the the text file over UDP.
    // (sends to localhost:12345).
    // This essentially actys an exchange.
    UdpMessageReplayer player("127.0.0.1", 12345);
    std::thread sender([&]() {
        for (int i = 0; i < 10; ++i) {
            ItchMessage msg{};
            msg.length = sizeof(ItchMessage);
            msg.type   = 'A'; // Add order
            std::snprintf(msg.payload, sizeof(msg.payload), "TestMsg%d", i);
            player.send(msg);

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    // 6. Main loop: consume "replayed" UDP market data messagess.
    //    This is the "hot path", this constantly polls the ring buff
    int receivedCount = 0;
    while (receivedCount < 10) {
        ItchMessage msg;
        if (buffer.pop(msg)) {
            std::cout << "Received ITCH message: type=" << msg.type
                      << " payload=" << msg.payload << "\n";
            ++receivedCount;
        }
    }

    // 7. Cleanup
    sender.join();
    listenerThread.stop();  // signals stopFlag, joins thread

    std::cout << "All done!\n";
    return 0;

}