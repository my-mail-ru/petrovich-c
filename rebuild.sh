#!/bin/bash -e

SELF_DIR=$(readlink -e "$(dirname "$0")")
BUILD_DIR="$SELF_DIR/build"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. -DCMAKE_C_COMPILER=gcc-5 -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
