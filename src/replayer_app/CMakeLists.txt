# ---------------------------------------------------------------------------
# File        : CMakeLists.txt
# Project     : HftSimulator
# App         : Replayer App
# Description : Replay captured ITCH messages over UDP for simulation
# Author      : Bryan Camp
# ---------------------------------------------------------------------------

add_executable(replayer_app main_replayer.cpp)

# Link against the common headers
target_link_libraries(replayer_app
    hft_common
    pthread
)
