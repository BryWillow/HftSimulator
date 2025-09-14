add_executable(listener_app main_listener.cpp)

# Link against the common headers
target_link_libraries(listener_app
    hft_common
    pthread
)