#pragma once
#include <cstddef>
#include <cstdint>
#include <utility>

/// @brief  A highly optimized, simple, Single-Producer Single-Consumer (SPSC) ring buffer.
/// @note   The capacity must be a power of 2. This is the simple trick so we can use the bitwise and
///         operator for the modulo operation instead of '%'. As we know, using the bitwise trick,
///         the compiler emits a single ADD instruction, which takes 1 CPU cycle. Using the '%' operator
///         causes the compiler to emit an IDIV instruction, which can take 20-40+ CPU cycles.
///         Downsides: lack of readability for some, and power of 2 restriction for Capacity.
/// @note   The producer and consumer threads must run on separate cores.
/// @note   We avoid using std::atomic by forcing producer and consumer to run on separate cores.
/// @note   The default Capacity is intentionally 4096 to fit into L2 cache. Adjust accordindgly.
/// @tparam T The data type stored in the ring buffer.
/// @tparam The size, in bytes, of the ring buffer.
template<typename T, size_t Capacity = 4096>
class SpScRingBuffer { 
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

public:
    // Constructor initializes head and tail to 0
    SpScRingBuffer() noexcept : _head(0), _tail(0) {}

    // Being explicit: copying and assignment are not allowed.
    SpScRingBuffer(const SpScRingBuffer&) = delete;
    SpScRingBuffer& operator=(const SpScRingBuffer&) = delete;

    // Being explicit: (in)equality checks are not allowed.
    bool operator==(const SpScRingBuffer&) = delete;
    bool operator!=(const SpScRingBuffer&) = delete;

    /// @brief  Attempts to push an item onto the ring buffer.
    /// @param  item An rvalue reference as moving is cheaper than copying.
    /// @note   Returns immediately. Does not wait for space to free up.
    /// @note   We will always always be using POD (Plain Old Data), but add nothrow just in case.
    /// @return True if the buffer has capacity for the item. False otherwise.
    bool tryPush(T&& item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        const size_t nextHead = (_head + 1) & (Capacity - 1); // Wrap-around index

        if (nextHead == _tail) [[unlikely]] // Compiler optimization: the queue likely won't be full.
            return false;                   // The queue is full. Fail fast. Behavior is on the client.

        buffer_[_head] = std::move(item);   // Move rvalue reference into the buffer.
        _head = nextHead;                   // Advance head.
        return true;
    }

    /// @brief  Attempts to push an item onto the ring buffer.
    /// @param  item a const lvalue reference that will be pushed onto the queue.
    /// @note   Returns immediately. Does not wait for space to free up.
    /// @note   We will always always be using POD (Plain Old Data), but add nothrow just in case.
    /// @return True if the buffer has capacity for the item. False otherwise.
    bool tryPush(const T& item) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        const size_t nextHead = (_head + 1) & (Capacity - 1); // Wrap-around

        if (nextHead == _tail) [[unlikely]] // Compiler optimization: the queue likely won't be full.
            return false;                   // The queue is full. Fail fast. Behavior is on the client.

        buffer_[_head] = item; // Copy lvalue reference into the buffer.
        _head = nextHead;      // Advance head.
        return true;
    }

    /// @brief  Attempts to pop an item onto the ring buffer without waiting for space to free up.
    /// @note   Returns immediately. Does not wait for items to be added to the queue.
    /// @return True if the buffer contains and returns data. False otherwise.
    bool tryPop(T& item) noexcept(std::is_nothrow_move_assignable_v<T>){
        if (_tail == _head) [[unlikely]]
            return false; // Buffer empty

        item = std::move(buffer_[_tail]);         // Move item out
        _tail = (_tail + 1) & (Capacity - 1);    // Advance tail
        return true;
    }

    /**
     * @brief  Checks if the buffer is empty.
     * @return True if the queue is empty. False otherwise.
     */
    bool empty() const noexcept {
        return _head == _tail;
    }

    /**
     * @brief  Checks if the buffer is full.
     * @return True if the buffer is full. False otherwise.
     */
    bool full() const noexcept {
        return ((_head + 1) & (Capacity - 1)) == _tail;
    }

private:
    // Note: by aligning each each item in the buffer on a cache line, we are, indeed, avoid false sharing.
    // However, be mindful of the memory footprint. Be mindful of sizeof(T) and Capacity.
    alignas(64) T buffer_[Capacity]; // Align buffer to cache line to avoid false sharing
    alignas(64) size_t _head;        // Producer writes data to _head index slot in the buffer, and reads from _tail.
    alignas(64) size_t _tail;        // Consumer advances the _tail index after reading buffer.
};