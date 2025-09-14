#pragma once

#include <atomic>
#include <cstddef>
#include <thread>

#include "spsc_ringbuffer.h"
#include "itch_message.h"
#include "cpu_pause.h"

/**
 * @class RingBufferConsumer
 * @brief Consumes messages from a single-producer, single-consumer ring buffer in a pinned thread.
 *
 * Template Parameters:
 * - MsgType: type stored in the ring buffer (e.g., ItchMessage)
 * - Callback: callable invoked for each popped message
 *
 * Usage:
 * ```
 * RingBufferConsumer<ItchMessage, decltype(lambda)> consumer(buffer, lambda);
 * consumer.start(core);
 * ```
 *
 * Hot-path considerations:
 * - Consumer only modifies _tail; producer only modifies _head
 * - Relaxed atomics ensure low-latency visibility
 * - _mm_pause() reduces CPU pressure while spinning
 * - Thread is optionally pinned to a CPU core for cache locality
 */
template <typename MsgType, typename Callback>
class RingBufferConsumer {
public:
    /// @brief Construct with reference to buffer and a callback
    RingBufferConsumer(SpScRingBuffer<MsgType>& buffer, Callback callback)
        : _buffer(buffer), _callback(std::move(callback)), _stopFlag(false) {}

    /// @brief Start consuming messages on a pinned thread
    void start(size_t core = -1) {
        _thread = std::thread([this, core] { consumeLoop(core); });
        if (core >= 0) pinThreadToCore(_thread, core);
    }

    /// @brief Stop consumer gracefully
    void stop() noexcept {
        _stopFlag.store(true, std::memory_order_relaxed);
        if (_thread.joinable()) _thread.join();
    }

private:
    SpScRingBuffer<MsgType>& _buffer;
    Callback _callback;               ///< Hot-path callback
    std::atomic<bool> _stopFlag;      ///< Relaxed atomic stop flag
    std::thread _thread;

    /// @brief Core consuming loop
    void consumeLoop(size_t /*core*/) {
        MsgType msg{};
        while (!_stopFlag.load(std::memory_order_relaxed)) {
            if (_buffer.pop(msg)) {
                _callback(msg);       // Hot-path processing
            } else {
                _mm_pause();          // reduce CPU pressure while spinning
            }
        }
    }

    /// @brief Pin thread to CPU core
    static void pinThreadToCore(std::thread& t, size_t core) {
#if defined(__linux__)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core, &cpuset);
        pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
#elif defined(__APPLE__)
        (void)t;
        (void)core; // macOS thread pinning omitted
#endif
    }
};
