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

# ------------------------------
# 2. Add include directory
# ------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include)

# ------------------------------
# 3. Add main executable
# ------------------------------
add_executable(HftSimulator
    src/main.cpp
)

# ------------------------------------
# 4. GoogleTest setup from local clone
# ------------------------------------
add_subdirectory(external/googletest)
enable_testing()

# ------------------------------
# 5. Add test executable
# ------------------------------
add_executable(HftSimulatorTests
    tests/test_spsc_ringbuffer.cpp
)

# Link GoogleTest
target_link_libraries(HftSimulatorTests
    gtest_main
)

# Include project headers
target_include_directories(HftSimulatorTests PRIVATE ${PROJECT_SOURCE_DIR}/include)

# ------------------------------
# 6. Enable CTest
# ------------------------------
include(GoogleTest)
gtest_discover_tests(HftSimulatorTests)