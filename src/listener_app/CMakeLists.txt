# ---------------------------------------------------------------------------
# File        : CMakeLists.txt
# Project     : HftSimulator
# App         : Listener App
# Description : HFT-style UDP listener for ITCH messages
# Author      : Bryan Camp
# ---------------------------------------------------------------------------

add_executable(listener_app main_listener.cpp)

# Link against the common headers
target_link_libraries(listener_app
    hft_common
    pthread
)