cmake_minimum_required(VERSION 3.20)

# ============================================================================
# Project     : Xary Engine
# Module      : CMake Build System Definition (CMakeLists.txt)
# Description : Modern C++20 CMake build configuration for Xary Engine with
#               target-centric flags, LTO optimizations, and static linking.
# Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
# Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
# ============================================================================

project(xary
    VERSION 1.0.0
    DESCRIPTION "High-performance binary stream processing engine"
    LANGUAGES CXX
)

# Enforce strict C++20 standard requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Interprocedural Optimization (Link-Time Optimization / LTO) for Release builds
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    message(STATUS "✔ Link-Time Optimization (LTO/IPO) enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

# Core Source Files Collection
set(XARY_SOURCES
    src/main.cpp
    src/core/Stream.cpp
    src/core/StreamWriter.cpp
    src/core/BufferView.cpp
    src/core/FileTypeDetector.cpp
    src/core/Archiver.cpp
    src/cli/ArgumentParser.cpp
)

# Define Main Executable Target
add_executable(xary ${XARY_SOURCES})

# Target Feature Requirements & Encapsulated Include Directories
target_compile_features(xary PRIVATE cxx_std_20)
target_include_directories(xary
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# ==============================================================================
# Cross-Platform Static & Runtime Linking Configuration
# ==============================================================================
if(MSVC)
    # Windows (MSVC): Statically link the C/C++ Runtime (/MT for Release, /MTd for Debug)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

elseif(APPLE)
    # macOS (Apple Clang): Apple strictly requires dynamic linking for system libraries and libc++.
    # Do NOT pass -static or -static-libstdc++ on macOS.
    message(STATUS "✔ macOS target detected: Using dynamic system runtime")

else()
    # Linux & Windows MinGW (GCC/Clang): Statically link C/C++ standard libraries
    target_link_options(xary PRIVATE 
        -static 
        -static-libgcc 
        -static-libstdc++
    )
endif()

# ==============================================================================
# Target-Scoped Compiler Warning & Optimization Flags
# ==============================================================================
target_compile_options(xary PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        /O2
        /permissive-
        /utf-8
    >
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -O3
    >
)

# Set Output Executable Directory
set_target_properties(xary PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
