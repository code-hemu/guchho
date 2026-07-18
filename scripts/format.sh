#!/usr/bin/env bash
set -euo pipefail

find include src tests -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
