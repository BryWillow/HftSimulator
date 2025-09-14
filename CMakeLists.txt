cmake_minimum_required(VERSION 3.20)
project(HftSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Common interface library for headers ----
add_library(hft_common INTERFACE)
target_include_directories(hft_common INTERFACE
    ${CMAKE_SOURCE_DIR}/src/common
)

# ---- Subdirectories ----
add_subdirectory(src/common)
add_subdirectory(src/listener_app)
add_subdirectory(src/replayer_app)
add_subdirectory(tests)
