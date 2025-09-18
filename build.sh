#!/bin/bash
# ---------------------------------------------------------------------------
# File        : build.sh
# Project     : HftSimulator
# App         : Build Script
# Description : Builds Debug and Release executables using CMake. Debug builds
#               enable sanitizers, compile and run tests. Release builds are
#               optimized. Builds both sequentially if requested.
# Author      : Bryan Camp
# ---------------------------------------------------------------------------

set -euo pipefail

# Usage help
usage() {
    echo "Usage: $0 [Debug|Release|All]"
    exit 1
}

# Check arguments
if [[ $# -lt 1 ]]; then
    usage
fi

TARGET=$1
if [[ "$TARGET" != "Debug" && "$TARGET" != "Release" && "$TARGET" != "All" ]]; then
    echo "Error: Invalid build target '$TARGET'"
    usage
fi

# ---------------------------------------------------------------------------
# Archive existing binaries
# ---------------------------------------------------------------------------
archive_existing_bins() {
    local ARCHIVE_DIR="build_archive"
    if [[ -d bin ]]; then
        mkdir -p "$ARCHIVE_DIR"
        echo "[Build] Archiving existing binaries and config files to $ARCHIVE_DIR..."
        cp -r bin/Debug "$ARCHIVE_DIR/" 2>/dev/null || true
        cp -r bin/Release "$ARCHIVE_DIR/" 2>/dev/null || true
        cp -f *.json "$ARCHIVE_DIR/" 2>/dev/null || true
    fi
}

# ---------------------------------------------------------------------------
# Clean build and bin directories
# ---------------------------------------------------------------------------
cleanup() {
    echo "[Build] Removing build/ and bin/ directories..."
    rm -rf build
    rm -rf bin
}

# ---------------------------------------------------------------------------
# Build one configuration
# ---------------------------------------------------------------------------
build_config() {
    local BUILD_TYPE=$1
    local BUILD_DIR="build/$BUILD_TYPE"

    echo "[Build] Starting $BUILD_TYPE build..."

    # Configure with CMake
    cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

    # Build
    cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j

    # Run tests only in Debug
    if [[ "$BUILD_TYPE" == "Debug" ]]; then
        echo "[Build] Running tests..."
        (cd "$BUILD_DIR" && ctest --output-on-failure || true)
    fi

    # Copy only the executables to bin/
    mkdir -p "bin/$BUILD_TYPE"
    find "$BUILD_DIR" -maxdepth 1 -type f -perm +111 -exec cp {} "bin/$BUILD_TYPE/" \; || true

    echo "[Build] $BUILD_TYPE build complete."
}

# ---------------------------------------------------------------------------
# Main build sequence
# ---------------------------------------------------------------------------
archive_existing_bins
cleanup

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

# Final cleanup of top-level build directory
if [[ -d build ]]; then
    echo "[Build] Removing top-level build/ directory..."
    rm -rf build
fi

echo "[Build] All done."
