#pragma once

#include <atomic>       // std::atomic for stop flag
#include <memory>       // std::unique_ptr for owning the thread
#include <string>       // std::string for filenames, IP addresses
#include "itch_message.h"     // ItchMessage definition
#include "pinned_thread.h"    // PinnedThread class for HFT-style pinned thread
#include "cpu_pause.h"        // cpu_pause() for reducing CPU busy-wait pressure

/**
 * @brief Plays back ITCH messages over UDP at configurable speed.
 * 
 * Designed for low-latency HFT testing. Uses a pinned thread to replay
 * messages and an atomic stop flag to safely terminate.
 */
class ItchMessageUdpReplayer {
public:
    /**
     * @brief Construct a replayer
     * @param fileName Path to the ITCH message file
     * @param destIp Destination IP for UDP
     * @param destPort Destination UDP port
     * @param speedFactor Replay speed multiplier (1.0 = real-time)
     * @param cpuCore Optional CPU core to pin the thread (-1 = no pin)
     */
    ItchMessageUdpReplayer(const std::string& fileName,
                           const std::string& destIp,
                           uint16_t destPort,
                           double speedFactor,
                           int cpuCore = -1)
        : _fileName(fileName)
        , _destIp(destIp)
        , _destPort(destPort)
        , _speedFactor(speedFactor)
        , _cpuCore(cpuCore)
        , _stopFlag(false)
    {}

    /// Disable copy and move
    ItchMessageUdpReplayer(const ItchMessageUdpReplayer&) = delete;
    ItchMessageUdpReplayer(ItchMessageUdpReplayer&&) = delete;
    ItchMessageUdpReplayer& operator=(const ItchMessageUdpReplayer&) = delete;
    ItchMessageUdpReplayer& operator=(ItchMessageUdpReplayer&&) = delete;

    /// @brief Start the replay thread
    void start() {
        _thread = std::make_unique<PinnedThread>(
            [this](std::atomic<bool>& stop) {
                replayLoop(stop);
            },
            _cpuCore,
            _stopFlag
        );
    }

    /// @brief Stop the replay thread
    void stop() {
        _stopFlag.store(true, std::memory_order_relaxed);
        if (_thread) _thread->join();
    }

private:
    /// @brief Actual replay loop executed on the pinned thread
    void replayLoop(std::atomic<bool>& stop) {
        while (!stop.load(std::memory_order_relaxed)) {
            ItchMessage msg{}; // populate message from file/memory
            // send message via UDP
            _mm_pause();       // reduce CPU pressure while spinning
        }
    }

private:
    std::string _fileName;
    std::string _destIp;
    uint16_t _destPort;
    double _speedFactor;
    int _cpuCore;
    std::atomic<bool> _stopFlag;
    std::unique_ptr<PinnedThread> _thread;
};