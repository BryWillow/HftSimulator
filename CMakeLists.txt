cmake_minimum_required(VERSION 3.16)
project(HftSimulator VERSION 1.0 LANGUAGES CXX)

# --- Sanitizer options ---
set(SANITIZE "" CACHE STRING "Enable a sanitizer: Address, Undefined, Thread, Memory, or None")

if (SANITIZE STREQUAL "Address")
    message(STATUS "Sanitizer enabled: AddressSanitizer")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address -fno-omit-frame-pointer)

elseif (SANITIZE STREQUAL "Undefined")
    message(STATUS "Sanitizer enabled: UndefinedBehaviorSanitizer")
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined -fno-omit-frame-pointer)

elseif (SANITIZE STREQUAL "Thread")
    message(STATUS "Sanitizer enabled: ThreadSanitizer")
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread -fno-omit-frame-pointer)

elseif (SANITIZE STREQUAL "Memory")
    message(STATUS "Sanitizer enabled: MemorySanitizer")
    add_compile_options(-fsanitize=memory -fno-omit-frame-pointer)
    add_link_options(-fsanitize=memory -fno-omit-frame-pointer)

elseif (SANITIZE STREQUAL "None" OR SANITIZE STREQUAL "")
    message(STATUS "No sanitizer enabled")

else()
    message(FATAL_ERROR "Unknown SANITIZE option: ${SANITIZE}")
endif()

# ------------------------------
# 1. Set C++ standard
# ------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Where binaries will be copied.
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Output binaries per configuration
set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR}/Release)

# The path to all header files.
include_directories(${PROJECT_SOURCE_DIR}/include)

# The name / entry point of the main executable.
add_executable(HftSimulator src/main.cpp)

# Use a local clone of GoogleTest.
# I got stuck on an issue with Fetch().
add_subdirectory(external/googletest)
enable_testing()

# The name of the test executable.
add_executable(HftSimulatorTests tests/test_spsc_ringbuffer.cpp)

# Link in GoogleTest to the to the test executable.
target_link_libraries(HftSimulatorTests gtest_main)

# Include project headers in the test executable.
target_include_directories(HftSimulatorTests PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Include project GoogleTest in ctest.
include(GoogleTest)
gtest_discover_tests(HftSimulatorTests)