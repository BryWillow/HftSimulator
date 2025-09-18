# ---------------------------------------------------------------------------
# File        : CMakeLists.txt
# Project     : HftSimulator
# App         : Top-Level Build
# Description : Configure and orchestrate all subprojects and dependencies
# Author      : Bryan Camp
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)
project(HftSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Common interface library for headers ----
add_library(hft_common INTERFACE)

target_include_directories(hft_common INTERFACE
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/vendor/json
)

# ---- Default binary output directories ----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/Release)

# ---- Subdirectories ----
add_subdirectory(src/common)
add_subdirectory(src/strategies)
add_subdirectory(src/listener_app)
add_subdirectory(src/replayer_app)
add_subdirectory(src/md_generator_app)
add_subdirectory(tests)