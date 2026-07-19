#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

if [ ! -d "$ROOT_DIR/build/release" ]; then
  echo "Building release..."
  "$ROOT_DIR/scripts/build.sh" release
fi

PLATFORM="$(node -e "console.log(process.platform)")"
ARCH="$(node -e "console.log(process.arch)")"
PKG_DIR="$ROOT_DIR/packages/$PLATFORM-$ARCH"

if [ ! -d "$PKG_DIR" ]; then
  echo "No package directory for $PLATFORM-$ARCH at $PKG_DIR"
  exit 1
fi

if [ "$PLATFORM" = "win32" ]; then
  BINARY_NAME="guchho.exe"
else
  BINARY_NAME="guchho"
fi

# Visual Studio multi-config generator puts binary under a config subdirectory
BUILD_BIN="$ROOT_DIR/build/release/src/Release/$BINARY_NAME"
if [ ! -f "$BUILD_BIN" ]; then
  BUILD_BIN="$ROOT_DIR/build/release/src/$BINARY_NAME"
fi

if [ ! -f "$BUILD_BIN" ]; then
  echo "Binary not found at build/release/src/Release/$BINARY_NAME or build/release/src/$BINARY_NAME"
  exit 1
fi

cp "$BUILD_BIN" "$PKG_DIR/$BINARY_NAME"
echo "Copied $BINARY_NAME to $PKG_DIR/"

if [ "$PLATFORM" != "win32" ]; then
  chmod +x "$PKG_DIR/$BINARY_NAME"
fi

echo "Ready to publish: npm publish $PKG_DIR"