#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="${1:-debug}"

cmake --preset "$BUILD_TYPE"
cmake --build --preset "$BUILD_TYPE"
