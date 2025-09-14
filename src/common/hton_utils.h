#pragma once

#include <cstdint>
#include <arpa/inet.h>

/**
 * @file hton_utils.h
 * @brief Cross-platform helpers for 64-bit host/network byte order conversion.
 *
 * This file isolates htonll/ntohll to simplify endian conversions
 * and improve readability. Keeps the main code free from platform
 * macros and messy inline functions.
 *
 * Linux/macOS safe and high performance. No dynamic memory used.
 */

/**
 * @brief Swap bytes of a 64-bit integer.
 * @param x 64-bit value to swap
 * @return byte-swapped 64-bit value
 */
inline uint64_t bswap64(uint64_t x) {
    return ((x & 0xFF00000000000000ULL) >> 56) |
           ((x & 0x00FF000000000000ULL) >> 40) |
           ((x & 0x0000FF0000000000ULL) >> 24) |
           ((x & 0x000000FF00000000ULL) >> 8)  |
           ((x & 0x00000000FF000000ULL) << 8)  |
           ((x & 0x0000000000FF0000ULL) << 24) |
           ((x & 0x000000000000FF00ULL) << 40) |
           ((x & 0x00000000000000FFULL) << 56);
}

/**
 * @brief Convert 64-bit host byte order to network byte order.
 * @param x host-order 64-bit integer
 * @return network-order 64-bit integer
 */
inline uint64_t htonll(uint64_t x) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return bswap64(x);
#else
    return x;
#endif
}

/**
 * @brief Convert 64-bit network byte order to host byte order.
 * @param x network-order 64-bit integer
 * @return host-order 64-bit integer
 */
inline uint64_t ntohll(uint64_t x) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return bswap64(x);
#else
    return x;
#endif
}
