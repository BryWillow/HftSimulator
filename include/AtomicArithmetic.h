#include <cstdint>

class AtomicArithmetic {
  public:

    /// @brief 
    AtomicArithmetic() : _numberOfThreads(10) {};

    /// @brief 
    /// @param numberOfThreads that will be performing the arithmetic operations. 
    AtomicArithmetic(int numberOfThreads) : _numberOfThreads(numberOfThreads) {};

    /// @brief multiple threads add/subtract the same value to/from an atomic variable.
    /// @param the value the atomic variable will be modified by.
    /// @return the expected value is 0.
    std::uint32_t DoAtomicAsyncAddSubtract(unsigned int modifyValue = 10);

    /// @brief multiple threads add/subtract the same value to/from a non-atomic variable.
    /// @param the value the non-atomic variable will be modified by.
    /// @return the expected value is undefined.
    std::uint32_t DoNonAtomicAsyncAddSubtract(unsigned int modifyValue = 10);

  private:
    std::uint32_t _numberOfThreads;
};