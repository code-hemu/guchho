#!/usr/bin/env bash

set -euo pipefail

PRESET="${1:-release-win32-x64}"
TARGET="${2:-}"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/${PRESET}"

echo "========================================"
echo " G U C H H O"
echo " Build System"
echo "========================================"
echo "Preset     : ${PRESET}"
echo "Build Dir  : ${BUILD_DIR}"
echo

echo "[1/3] Configuring..."
cmake --preset "${PRESET}"

echo
echo "[2/3] Building..."

BUILD_CMD=(
    cmake
    --build
    --preset "${PRESET}"
)

if [[ -n "${TARGET}" ]]; then
    BUILD_CMD+=(--target "${TARGET}")
fi

"${BUILD_CMD[@]}"

echo
echo "[3/3] Running tests..."
ctest --preset "${PRESET}"

echo

if [[ "${PRESET}" == *"win32"* ]]; then
    EXE="$(find "${BUILD_DIR}" -type f -name "*.exe" | head -n 1 || true)"
else
    EXE="$(find "${BUILD_DIR}" -maxdepth 2 -type f -perm -111 | head -n 1 || true)"
fi

echo "========================================"
echo "[+] Build completed successfully."
echo "[+] Preset          : ${PRESET}"
echo "[+] Build directory : ${BUILD_DIR}"

if [[ -n "${EXE}" ]]; then
    echo "[+] Executable      : ${EXE}"
fi

echo "========================================"