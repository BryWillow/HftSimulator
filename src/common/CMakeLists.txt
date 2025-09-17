# ---------------------------------------------------------------------------
# File        : CMakeLists.txt
# Project     : HftSimulator
# App         : Common Library
# Description : Header-only interface library for shared components
# Author      : Bryan Camp
# ---------------------------------------------------------------------------

# This is a header-only library; just declare sources for IDEs
set(COMMON_HEADERS
    itch_udp_listener.h
    pinned_thread.h
    spsc_ringbuffer_consumer_internal.h
    spsc_ringbuffer_consumer.h
    spsc_ringbuffer.h
    itch_message.h
    itch_message_udp_replayer.h
    captured_message.h
    hton_utils.h
    cpu_pause.h
    itch_file_generator.h
    constants.h
)

# Add sources to the INTERFACE library
target_sources(hft_common
    INTERFACE
        ${COMMON_HEADERS}
)