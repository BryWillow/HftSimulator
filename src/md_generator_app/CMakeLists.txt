# ---------------------------------------------------------------------------
# File        : CMakeLists.txt
# Project     : HftSimulator
# App         : Market Data Generator App
# Description : Generates ITCH messages for simulation purposes
# Author      : Bryan Camp
# ---------------------------------------------------------------------------

add_executable(md_generator_app main_generator.cpp)

# Link against the common headers
target_link_libraries(md_generator_app
    hft_common
    pthread
)
