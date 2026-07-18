#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build/debug}"

find include src tests -name '*.cpp' -o -name '*.hpp' | xargs clang-tidy -p "$BUILD_DIR"
