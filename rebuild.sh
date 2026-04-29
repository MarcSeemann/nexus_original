#!/bin/bash
# Quick rebuild and sync script for NEXUS
# Usage: ./rebuild.sh
# This script rebuilds NEXUS and automatically syncs the library

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$SCRIPT_DIR/build"
LIB_DIR="$SCRIPT_DIR/lib"
BIN_DIR="$SCRIPT_DIR/bin"

echo "=========================================="
echo "NEXUS Quick Rebuild"
echo "=========================================="
echo ""

# Ensure build directory exists
mkdir -p "$BUILD_DIR"
mkdir -p "$LIB_DIR"
mkdir -p "$BIN_DIR"

# Build
echo "Step 1: Compiling..."
cd "$BUILD_DIR"
make -j4 || { echo "Build failed!"; exit 1; }

echo ""
echo "Step 2: Syncing libraries and executables..."

# Sync library
if [ -f "libnexus.so" ]; then
    cp libnexus.so "$LIB_DIR/libnexus.so"
    echo "  ✓ Synced libnexus.so"
else
    echo "  ✗ libnexus.so not found!"
    exit 1
fi

# Sync executable
if [ -f "nexus" ]; then
    cp nexus "$BIN_DIR/nexus"
    chmod +x "$BIN_DIR/nexus"
    echo "  ✓ Synced nexus executable"
else
    echo "  ✗ nexus executable not found!"
    exit 1
fi

# Sync test executable
if [ -f "nexus-test" ]; then
    cp nexus-test "$BIN_DIR/nexus-test"
    chmod +x "$BIN_DIR/nexus-test"
    echo "  ✓ Synced nexus-test executable"
fi

cd "$SCRIPT_DIR"

echo ""
echo "=========================================="
echo "✓ Rebuild and sync complete!"
echo "=========================================="
echo ""
echo "Library versions:"
ls -lh "$LIB_DIR/libnexus.so"
echo ""
echo "To run NEXUS:"
echo "  $BIN_DIR/nexus -b -n <events> <macro.mac>"
echo ""
