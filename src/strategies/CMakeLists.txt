# ---------------------------------------------------------------------------
# File        : CMakeLists.txt
# Project     : HftSimulator
# App         : Strategies Library
# Description : Header-only algorithm implementations
# Author      : Bryan Camp
# ---------------------------------------------------------------------------

# Header-only interface library for all strategies
add_library(hft_strategies INTERFACE)

# Make all headers in this directory available to any app that links this library
target_include_directories(hft_strategies INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)