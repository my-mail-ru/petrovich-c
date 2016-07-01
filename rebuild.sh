#!/bin/bash -e

SELF_DIR=$(readlink -e "$(dirname "$0")")
BUILD_DIR="$SELF_DIR/build"

rm -rf "$BUILD_DIR"

mkdir -p "$BUILD_DIR/debug"
cd "$BUILD_DIR/debug"
cmake ../.. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

mkdir -p "$BUILD_DIR/release"
cd "$BUILD_DIR/release"
cmake ../.. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
