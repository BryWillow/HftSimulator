// ---------------------------------------------------------------------------
// File        : pinned_thread.h
// Project     : HftSimulator
// Component   : Common
// Description : Minimal HFT-style pinned thread wrapper
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <thread>
#include <iostream>
#include <functional>

#if defined(__linux__)
#include <pthread.h>
#include <sched.h>
#elif defined(__APPLE__)
#include <pthread.h>
#include <mach/mach.h>
#include <mach/thread_policy.h>
#endif

#include "cpu_pause.h" ///< defines _mm_pause() stub for ARM/Apple Silicon

/**
 * @class PinnedThread
 * @brief Minimal HFT-style pinned thread wrapper.
 *
 * Hot-path design:
 * - The user lambda receives a reference to a stop flag (std::atomic<bool>&)
 * - Thread can be pinned to a specific CPU core for low-latency processing
 * - Exceptions inside the lambda are caught and logged
 *
 * Example usage:
 * @code{.cpp}
 * std::atomic<bool> stop{false};
 * PinnedThread t(
 *     [](std::atomic<bool>& stop){
 *         while(!stop){
 *             // do work
 *             _mm_pause(); // reduce busy-wait CPU pressure
 *         }
 *     },
 *     0,    // CPU core 0
 *     stop
 * );
 * @endcode
 */
class PinnedThread
{
public:
    PinnedThread() = default;

    // Deleted copy constructor/assignment to prevent accidental copying of thread object
    PinnedThread(const PinnedThread &) = delete;
    PinnedThread &operator=(const PinnedThread &) = delete;

    // Move constructor/assignment are allowed for transferring ownership of the thread
    PinnedThread(PinnedThread &&) noexcept = default;
    PinnedThread &operator=(PinnedThread &&) noexcept = default;

    /**
     * @brief Construct and start a thread pinned to a CPU core
     * @param fn User callable: receives std::atomic<bool>& stop
     * @param core CPU core index to pin thread
     * @param stopFlag Atomic stop flag to signal shutdown
     *
     * Notes:
     * - Captures the stopFlag by reference to allow external signaling
     * - Thread exceptions are caught and logged to std::cerr
     */
    template <typename Fn>
    PinnedThread(Fn &&fn, size_t core, std::atomic<bool> &stopFlag)
    {
        // Capture 'this' explicitly to call member function pinThread
        _thread = std::thread([this, fn = std::forward<Fn>(fn), core, &stopFlag]() mutable
                              {
            pinThread(core); // OS-specific pinning
            try {
                fn(stopFlag); // execute user lambda
            } catch (const std::exception& e) {
                std::cerr << "[PinnedThread] Exception: " << e.what() << "\n";
            } catch (...) {
                std::cerr << "[PinnedThread] Unknown exception\n";
            } });
    }

    ~PinnedThread()
    {
        if (_thread.joinable())
            _thread.join();
    }

    /// @brief Join thread (blocks until finished)
    void join()
    {
        if (_thread.joinable())
            _thread.join();
    }

    /// @brief Check if thread is joinable
    bool joinable() const { return _thread.joinable(); }

private:
    std::thread _thread; ///< Managed thread object

    /**
     * @brief OS-specific thread pinning
     * @param core CPU core index
     */
    void pinThread(int core)
    {
#if defined(__linux__)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
#elif defined(__APPLE__)
        thread_affinity_policy_data_t policy = {core};
        thread_port_t machThread = pthread_mach_thread_np(pthread_self());
        if (thread_policy_set(machThread, THREAD_AFFINITY_POLICY,
                              reinterpret_cast<thread_policy_t>(&policy),
                              THREAD_AFFINITY_POLICY_COUNT) != KERN_SUCCESS)
        {
            std::cerr << "[PinnedThread] Failed to set thread affinity\n";
        }
#endif
    }
};