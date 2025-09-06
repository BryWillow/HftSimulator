#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
BIN_DIR="bin"

# Detect macOS
IS_MACOS=0
if [[ "$OSTYPE" == "darwin"* ]]; then
    IS_MACOS=1
fi

build_mode() {
    local mode=$1
    local build_subdir="$BUILD_DIR/$mode"
    local outdir="$BIN_DIR/$mode"

    echo ">> Building in $mode mode..."
    mkdir -p "$build_subdir" "$outdir"

    # Configure
    if [ "$mode" = "Debug" ]; then
        cmake -S . -B "$build_subdir" \
            -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${PWD}/$outdir" \
            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG="${PWD}/$outdir"
    else
        cmake -S . -B "$build_subdir" \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${PWD}/$outdir" \
            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE="${PWD}/$outdir"
    fi

    # Build
    cmake --build "$build_subdir" -j
    echo ">> Executables are in $outdir/"

    # Run tests automatically in Debug
    if [ "$mode" = "Debug" ]; then
        local test_bin="$outdir/HftSimulatorTests"
        if [ -x "$test_bin" ]; then
            echo ">> Running unit tests with sanitizer enabled..."

            # On macOS, disable nano allocator to avoid warnings
            if [ $IS_MACOS -eq 1 ]; then
                MallocNanoZone=0 "$test_bin"
            else
                "$test_bin"
            fi
        else
            echo ">> No test binary found at $test_bin (skipping tests)"
        fi
    fi

    # Clean up build directory
    echo ">> Removing intermediate build directory $build_subdir"
    rm -rf "$build_subdir"
}

# Entry point
case "${1:-}" in
    Debug|debug)
        build_mode Debug
        ;;
    Release|release)
        build_mode Release
        ;;
    clean)
        echo ">> Cleaning bin and build directories..."
        rm -rf "$BUILD_DIR" "$BIN_DIR"
        ;;
    *)
        echo "Usage: $0 {Debug|Release|clean}"
        exit 1
        ;;
esac