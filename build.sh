#!/usr/bin/env bash

set -e

echo "========================================"
echo "      Building Xary Binary Engine       "
echo "========================================"

# Kill any running instances of xary.exe
taskkill //F //IM xary.exe 2>/dev/null || true

# Force clean build directory to prevent stale ABI object mismatches
rm -rf build
mkdir -p build
cd build

echo "[+] Configuring CMake..."
cmake -G "MinGW Makefiles" ..

echo "[+] Compiling source files..."
cmake --build .

echo "========================================"
echo "✔ Build complete! Running Xary:"
echo "========================================"

./xary.exe