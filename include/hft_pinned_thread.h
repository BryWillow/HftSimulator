#include <atomic>
#include <thread>
//#include <immintrin.h>

#if defined(__linux__)
  #include <pthread.h>
  #include <sched.h>
#elif defined(__APPLE__)
  #include <pthread.h>
#endif

/// @brief  Creates a new thread and pins it to the specified core.
/// @tparam Callable A lambda, function, functor that can be called.
/// @param  task A Callable, see ablove.
/// @param  core This is intentionally not defaulted. You must specify the CPU core index, e.g., '0'. 
template <typename Callable>
class HftPinnedThread {
public:
    // Constructor
    // - Accepts any callable object (lambda, function, or functor) as 'task'
    //   or boost its Quality of Service (QoS) on macOS for latency-sensitive work
    HftPinnedThread(Callable&& task, size_t core)
        : _shouldStop(false)
    {
        // Start a new thread. Can safely stop manually at any time. The destructor makes sure it's stopped.
        _thread = std::thread([this, task = std::move(task), core] {
            // Set CPU affinity or QoS (on MacOS) for this thread.
            setAffinity(core);

            // Call the user-provided function, passing the atomic stop flag
            // The function should periodically check stopFlag to exit cleanly
            task(_shouldStop);
        });
    }

    // Delete the copy constructor and copy assignment operator.
    HftPinnedThread(const HftPinnedThread&) = delete;
    HftPinnedThread& operator=(const HftPinnedThread&) = delete;

    // Delete the move constructor and move assignment operator.
    HftPinnedThread(HftPinnedThread&&) = delete;
    HftPinnedThread& operator=(HftPinnedThread&&) = delete;    

    ~HftPinnedThread() { stop(); }
    
    /// @brief Signals the thread to exit. 
    /// @note. This method blocks until the user-provided Callable honors _shouldStop.
    void stop() {
        _shouldStop.store(true, std::memory_order_relaxed);
        if (_thread.joinable()) _thread.join();
    }

private:
    void setAffinity(size_t core) {
#if defined(__linux__)
        // Linux: pin thread to a specific CPU core
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(static_cast<int>(core % std::thread::hardware_concurrency()), &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
#elif defined(__APPLE__)
        // macOS: cannot pin thread to a specific CPU, but we can boost its
        // Quality of Service (QoS) for latency-sensitive tasks
        // QOS_CLASS_USER_INTERACTIVE gives the highest priority for user-facing, low-latency work
        pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
#endif
    }

    std::atomic<bool> _shouldStop;
    std::thread _thread;
};