#pragma once

#include <cstddef>
#include <atomic>
#include <type_traits>
#include <thread>
#include <iostream>

#if defined(__x86_64__)
#include <immintrin.h> // x86-only cache-line optimizations
#endif

/**
 * @brief Single-Producer Single-Consumer Ring Buffer (SPSC).
 *
 * Highly optimized for low-latency HFT pipelines:
 * - Single producer thread writes into the buffer.
 * - Single consumer thread reads from the buffer.
 * - Cache-line aligned head/tail and storage to prevent false sharing.
 * - Uses atomic counters for statistics, memory_order_relaxed is sufficient
 *   because only one producer/consumer updates each value.
 * 
 * This class works well with ItchUdpListener: listener pushes messages into
 * the buffer on a pinned thread, while a consumer thread reads from it.
 *
 * @tparam T         Type of element stored (preferably POD, cache-line aligned).
 * @tparam Capacity  Must be a power of 2 for fast bitwise wrapping.
 */
template <typename T, size_t Capacity = 4096>
class SpScRingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power-of-2");

public:
    SpScRingBuffer() noexcept
        : _head{0}, _tail{0}, _droppedMessageCount{0}, _pushedMessageCount{0},
          _poppedMessageCount{0}, _highWaterMark{0} {}

    SpScRingBuffer(const SpScRingBuffer&) = delete;
    SpScRingBuffer& operator=(const SpScRingBuffer&) = delete;

    /**
     * @brief Try to push an item into the ring buffer (move version).
     * @return true if pushed, false if buffer is full
     * @note This is the hot path: highly inlined and uses relaxed atomics
     */
    bool tryPush(T&& item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        const size_t nextHead = (_head + 1) & (Capacity - 1);
        if (nextHead == _tail) { // buffer full
            _droppedMessageCount.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        _buffer[_head] = std::move(item);
        _head = nextHead;

        _pushedMessageCount.fetch_add(1, std::memory_order_relaxed);
        updateHighWaterMark();
        return true;
    }

    /**
     * @brief Try to push an item into the ring buffer (copy version).
     */
    bool tryPush(const T& item) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        const size_t nextHead = (_head + 1) & (Capacity - 1);
        if (nextHead == _tail) { 
            _droppedMessageCount.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        _buffer[_head] = item;
        _head = nextHead;

        _pushedMessageCount.fetch_add(1, std::memory_order_relaxed);
        updateHighWaterMark();
        return true;
    }

    /**
     * @brief Try to pop an item from the ring buffer
     * @return true if successful, false if buffer is empty
     */
    bool tryPop(T& item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        if (_tail == _head) return false; // empty
        item = std::move(_buffer[_tail]);
        _tail = (_tail + 1) & (Capacity - 1);
        _poppedMessageCount.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    bool empty() const noexcept { return _head == _tail; }
    bool full() const noexcept { return ((_head + 1) & (Capacity - 1)) == _tail; }

    uint64_t droppedMessageCount() const noexcept { return _droppedMessageCount.load(std::memory_order_relaxed); }
    uint64_t pushedMessageCount() const noexcept { return _pushedMessageCount.load(std::memory_order_relaxed); }
    uint64_t poppedMessageCount() const noexcept { return _poppedMessageCount.load(std::memory_order_relaxed); }

    size_t size() const noexcept { return (_head - _tail) & (Capacity - 1); }
    size_t highWaterMark() const noexcept { return _highWaterMark.load(std::memory_order_relaxed); }

private:
    alignas(64) T _buffer[Capacity]; // storage aligned to cache line
    alignas(64) size_t _head;
    alignas(64) size_t _tail;

    std::atomic<uint64_t> _droppedMessageCount;
    std::atomic<uint64_t> _pushedMessageCount;
    std::atomic<uint64_t> _poppedMessageCount;
    std::atomic<size_t> _highWaterMark;

    /**
     * @brief Track high-water mark for buffer usage
     * @note Only called by producer; relaxed atomics sufficient
     */
    void updateHighWaterMark() noexcept {
        size_t currentSize = (_head - _tail) & (Capacity - 1);
        size_t prevHigh = _highWaterMark.load(std::memory_order_relaxed);
        while (currentSize > prevHigh &&
               !_highWaterMark.compare_exchange_weak(prevHigh, currentSize, std::memory_order_relaxed)) {}
    }
};