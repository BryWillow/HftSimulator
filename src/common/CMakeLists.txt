# This is a header-only library; just declare sources for IDEs
set(COMMON_HEADERS
    itch_listener.h
    pinned_thread.h
    ring_buffer_consumer.h
    spsc_ringbuffer.h
    itch_message.h
    itch_message_udp_replayer.h
    captured_message.h
    hton_utils.h
    itch_file_generator.h
    strategy_buysellcount.h
    ring_buffer_consumer_internal.h
)

# Add sources to the INTERFACE library
target_sources(hft_common
    INTERFACE
        ${COMMON_HEADERS}
)
