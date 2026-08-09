#!/usr/bin/env bash

# ============================================================================
# Project     : Xary Engine
# Module      : Build Automation Pipeline (build.sh)
# Description : High-performance shell automation script with ANSI reporting,
#               hardware core auto-detection, process handling, and flags.
# Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
# Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
# ============================================================================

set -euo pipefail

# ANSI Color & Formatting Constants
COLOR_RESET="\033[0m"
COLOR_BOLD="\033[1m"
COLOR_CYAN="\033[36m"
COLOR_GREEN="\033[32m"
COLOR_YELLOW="\033[33m"
COLOR_RED="\033[31m"

# Console Output Helpers
info()    { echo -e "${COLOR_CYAN}[i] $1${COLOR_RESET}"; }
success() { echo -e "${COLOR_GREEN}[✔] $1${COLOR_RESET}"; }
warn()    { echo -e "${COLOR_YELLOW}[!] $1${COLOR_RESET}"; }
error()   { echo -e "${COLOR_RED}[✘] $1${COLOR_RESET}" >&2; }

BUILD_TYPE="Release"
CLEAN_BUILD=false
RUN_AFTER_BUILD=true

# Parse Command Line Options
for arg in "$@"; do
    case "$arg" in
        --clean)   CLEAN_BUILD=true ;;
        --debug)   BUILD_TYPE="Debug" ;;
        --no-run)  RUN_AFTER_BUILD=false ;;
        --help|-h)
            echo "Usage: ./build.sh [OPTIONS]"
            echo "  --clean    Purge build directory before compilation"
            echo "  --debug    Compile engine target with Debug symbols"
            echo "  --no-run   Skip executable launch after successful build"
            echo "  -h, --help Display build system options"
            exit 0
            ;;
        *)
            error "Unknown option flag: $arg"
            exit 1
            ;;
    esac
done

START_TIME=$SECONDS

echo -e "${COLOR_BOLD}${COLOR_CYAN}"
echo "======================================================================"
echo "             Building Xary Engine Target [${BUILD_TYPE}]              "
echo "======================================================================"
echo -e "${COLOR_RESET}"

# Terminate active runtime process instances to release file locks
if command -v taskkill &>/dev/null; then
    taskkill //F //IM xary.exe 2>/dev/null || true
elif command -v pkill &>/dev/null; then
    pkill -f xary 2>/dev/null || true
fi

# Workspace preparation
if [ "$CLEAN_BUILD" = true ] || [ ! -d "build" ]; then
    info "Preparing clean build workspace..."
    rm -rf build
    mkdir -p build
fi

cd build

# Auto-detect physical CPU thread count for parallel multi-core compiling
CPU_CORES=4
if command -v nproc &>/dev/null; then
    CPU_CORES=$(nproc)
elif command -v sysctl &>/dev/null; then
    CPU_CORES=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
fi

info "Configuring CMake build system (${BUILD_TYPE})..."
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" ..

info "Compiling binary targets using ${CPU_CORES} parallel threads..."
cmake --build . --parallel "${CPU_CORES}"

ELAPSED=$(( SECONDS - START_TIME ))

echo -e "\n${COLOR_BOLD}${COLOR_GREEN}"
echo "======================================================================"
success "Build completed successfully in ${ELAPSED}s!"
echo "======================================================================"
echo -e "${COLOR_RESET}"

# Launch executable binary if requested
if [ "$RUN_AFTER_BUILD" = true ]; then
    if [ -f "./xary.exe" ]; then
        info "Executing output binary: ./xary.exe\n"
        ./xary.exe
    elif [ -f "./xary" ]; then
        info "Executing output binary: ./xary\n"
        ./xary
    else
        warn "Executable binary not found in build directory."
    fi
fi