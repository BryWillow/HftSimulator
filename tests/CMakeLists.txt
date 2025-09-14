# tests/CMakeLists.txt

# Find GoogleTest (assumes installed, e.g. via Homebrew on macOS)
find_package(GTest REQUIRED)

add_executable(HftSimulatorTests
    test_spsc_ringbuffer.cpp
    test_hft_pinned_thread.cpp
    # add other tests here
)

target_link_libraries(HftSimulatorTests
    hft_common
    pthread
    GTest::GTest
    GTest::Main
)

include(GoogleTest)
gtest_discover_tests(HftSimulatorTests)