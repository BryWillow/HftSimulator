#include <atomic>
#include <vector>
#include <thread>
#include "AtomicArithmetic.h"

const int IterationCount = 100000;

std::uint32_t AtomicArithmetic::DoAtomicAsyncAddSubtract(unsigned int modifyValue)
{
    std::atomic<int> atomicVar(0);

    // The lambda that does the actual work.
    auto DoAtomicArithmetic = [&atomicVar, modifyValue]() {
        for(int i = 0; i < IterationCount; i++) {
            int valuePreAdd = atomicVar.fetch_add(modifyValue);
            int valuePreSub = atomicVar.fetch_sub(modifyValue);
        }
    };

    std::vector<std::thread> threads;
    for(std::uint32_t i = 0; i < _numberOfThreads; i++) {
        threads.emplace_back(DoAtomicArithmetic);
    }

    for(auto& thread : threads) {
        thread.join();
    }

    return atomicVar.load();
}

std::uint32_t AtomicArithmetic::DoNonAtomicAsyncAddSubtract(unsigned int modifyValue)
{
    std::uint32_t nonAtomicVar = 0;

    // The lambda that does the actual work.
    auto DoNonAtomicArithmetic = [&nonAtomicVar, modifyValue]() {
        for(int i = 0; i < IterationCount; i++) {
            nonAtomicVar += modifyValue;
            nonAtomicVar -= modifyValue;
        }
    };

    // Create a dynamic number of threads.
    std::vector<std::thread> threads;
    for(std::uint32_t i = 0; i < _numberOfThreads; i++) {
        threads.emplace_back(DoNonAtomicArithmetic);
    }

    // Join all of the dynamic threads we created.
    for(auto& thread : threads) {
        thread.join();
    }

    return nonAtomicVar;
}