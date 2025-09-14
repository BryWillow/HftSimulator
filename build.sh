#!/bin/bash
# build.sh
# Usage: ./build.sh [Debug|Release|All]

set -euo pipefail

# Usage function
usage() {
    echo "Usage: $0 [Debug|Release|All]"
    echo "  Debug   : build Debug pipeline with sanitizers and run tests"
    echo "  Release : build Release pipeline (optimized)"
    echo "  All     : build both Debug and Release sequentially"
    exit 1
}

# Check for argument
if [[ $# -lt 1 ]]; then
    usage
fi

TARGET=$1

# Validate argument
if [[ "$TARGET" != "Debug" && "$TARGET" != "Release" && "$TARGET" != "All" ]]; then
    echo "Error: Invalid build target '$TARGET'."
    usage
fi

CXX=clang++
CXXFLAGS="-std=c++20 -Wall -Wextra -pthread -O3 -march=native -I./src/common"

build_config() {
    local BUILD_TYPE=$1
    local BUILD_DIR="build/$BUILD_TYPE"
    local BIN_DIR="bin/$BUILD_TYPE"

    echo "[Build] Cleaning $BUILD_DIR and $BIN_DIR..."
    rm -rf "$BUILD_DIR" "$BIN_DIR"
    mkdir -p "$BUILD_DIR" "$BIN_DIR"

    # Enable sanitizers for Debug
    local EXTRA_FLAGS=""
    if [[ "$BUILD_TYPE" == "Debug" ]]; then
        EXTRA_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
        echo "[Build] Debug mode: enabling sanitizers"
    fi

    # Compile a target executable
    compile_target() {
        local name=$1
        local src_files
        src_files=$(find "src/$name" -name "*.cpp")
        local output="$BIN_DIR/$name"
        echo "[Build] Compiling $name -> $output"
        $CXX $CXXFLAGS $EXTRA_FLAGS $src_files -o "$output"
    }

    compile_target "listener_app"
    compile_target "replayer_app"

    echo "[Build] Done. Executables are in $BIN_DIR"

    # Build and run tests if Debug
    if [[ "$BUILD_TYPE" == "Debug" ]]; then
        if [[ -d "src/tests" ]]; then
            echo "[Build] Compiling and running tests..."
            TEST_OUTPUT="$BIN_DIR/tests"
            TEST_SRC=$(find "src/tests" -name "*.cpp")
            $CXX $CXXFLAGS $EXTRA_FLAGS $TEST_SRC -o "$TEST_OUTPUT"
            echo "[Build] Running tests..."
            "$TEST_OUTPUT"
        else
            echo "[Build] No tests directory found."
        fi
    fi
}

case "$TARGET" in
    Debug)
        build_config "Debug"
        ;;
    Release)
        build_config "Release"
        ;;
    All|all)
        build_config "Debug"
        build_config "Release"
        ;;
esac