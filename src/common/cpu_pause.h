#pragma once

/**
 * @file cpu_pause.h
 * @brief Provides a cross-platform stub for _mm_pause / CPU pause instruction.
 *
 * - On x86_64, includes <immintrin.h> for real _mm_pause.
 * - On other platforms (ARM / Apple Silicon), defines an empty inline function.
 *
 * Purpose:
 * - Reduces busy-wait CPU pressure in HFT hot-paths.
 */
#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h> // SSE2 intrinsics
#else
inline void _mm_pause() {} // stub for ARM / Apple Silicon
#endif