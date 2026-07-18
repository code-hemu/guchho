#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="${1:-debug}"

case "$BUILD_TYPE" in
  debug|release|ci)
    cmake --preset "$BUILD_TYPE"
    cmake --build --preset "$BUILD_TYPE"
    ;;
  *)
    echo "Usage: $0 {debug|release|ci}"
    exit 1
    ;;
esac
