#include <atomic>
#include <iostream>
#include "spsc_ringbuffer.h"
#include "itch_message.h"
#include "itch_udp_listener.h"
#include "itch_udp_replayer.h"
#include "hft_pinned_thread.h"

int main() {
    
    constexpr const char* UDP_ADDRESS = "127.0.0.1";
    constexpr int UDP_PORT = 5555;

    // File Description: *NSDQ ITCH* format specific.
    // 1. "capture.sbe" contains *SBE-encoded FIX packets*, just as received from NSDQ.
    // 2. The file is located in the project/data directory.
    // 3. If this file doesn't exist, the app auto-generates it.
    // 4. This file contains 10K (configurable) randomly generated messages.
    // Goal: To be *identical* to those we receive from an exchange. 
    const std::string captureFile = "capture.sbe";

    // SPSC Description: HFT tuned, lock free, ring buffer.
    // 1. Transports decoded market data to the strategy.
    // 2. If the buffer is full, stale packets are dropped.
    // 3. Buffer size is configurable, default to 2048.
    // Goal: To deliver market data as fast as possible to the strategy.
    SpScRingBuffer<ItchMessage> ringBuffer;

    // Listens for Market Data on the specified port.
    // 1. This listener is pinned to a specific CPU core.
    // 2. As soon as a messsage is received, it is decoded into a structure our app understands.
    // 3. This decoded structure is pushed onto the ring buffer, which will be read by the strategy.
    UdpItchListener listener(UDP_PORT, ringBuffer);

    // The UdpMessageReplayer reades from the capture file outlined above.
    // 1. It plays back the contentss of the file over UDP using configurable address/port.
    // 2. If the capture file doesns't exist, it is auto-generated.
    // 3. The number of messages in the capture file is configurable, defaults to 10K.
    // 4. The replayer is "smart", meaning that it plays messages in bursts, based on timestamps.
    // 5. The burst rate is adjustable, right now it is set to 1.
    UdpMessageReplayer replayer(captureFile, UDP_ADDRESS, UDP_PORT, 1.0);

    std::atomic<bool> stopFlag{false};

    // Start listening for the market data being replayed over UDP.
    // 1. The listener runs on a dedicated thread that is pinned to a specific core.
    // 2. This core is configurable. See below, it is configured to Core 0.
    // 3. The atomic bool variable tells the listener to stop: set stopFlag = true.
    HftPinnedThread listenerThread([&](std::atomic<bool>& stop) {
        listener.run(stop);
    }, 0);

    // Start replaying market data over UDP.
    // 1. The replayer runs on a pinned core, number, 1 bu default.
    // 2. If you believe this is overkill than you can simply use std::thread here.
    // 3. Again, if the file to be replayed does not exist, it will be auto-generated.
    HftPinnedThread replayerThread([&](std::atomic<bool>& stop) {
        replayer.replay();
        stop.store(true);  // signal listener to stop when replay ends
    }, 1);

    // We are finished replaying data over UDP.
    // Wait for the thread to stop gracefully.
    replayerThread.stop();

    // We are inished processing the data from the exchange.
    // Wait for this thread to shut down gracefully.
    listenerThread.stop();

    std::cout << "Replay complete." << std::endl;

    return 0;
}