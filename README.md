<div align="center">

# ⚡ XARY BINARY ENGINE

**High-Throughput C++20 Chunked Stream Processing & Memory-Efficient Binary Core**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-4.4%2B-B22222?style=for-the-badge&logo=cmake&logoColor=white)](https://cmake.org/)
[![Build](https://img.shields.io/badge/Build-Passing-2ea44f?style=for-the-badge&logo=github-actions&logoColor=white)](#)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-black?style=for-the-badge&logo=windows&logoColor=white)](#)
[![License](https://img.shields.io/badge/License-MIT-FFBF00.svg?style=for-the-badge)](#)

---

</div>

## 📌 Overview

**Xary** is a lightweight, ultra-fast C++20 binary engine designed for streaming large data files through high-performance **64 KB chunk buffers**. Built with zero-overhead standard abstractions, Xary ensures predictable memory usage and cross-platform reliability without external runtime dependencies.

---

## 🔥 Key Features

* **🚀 High-Throughput Streaming**: Process multi-gigabyte binary files with a predictable 64 KB memory allocation footprint.
* **🛡️ Static Runtime Linking**: Fully self-contained executable binaries compiled without external runtime DLL dependencies.
* **⚡ Modern C++20 Core**: Built using strict modern compiler standards (`-Wall -Wextra -Wpedantic -Werror`).
* **🛠️ One-Click Automation**: Complete shell automation (`build.sh`) for clean builds, configuration, and testing.

---

## 📂 Project Architecture

```text
xary/
├── 📁 include/
│   └── 📁 xary/
│       └── 📁 core/
│           └── 📄 Stream.hpp      # Core chunked stream reader interface
├── 📁 src/
│   ├── 📁 core/
│   │   └── 📄 Stream.cpp          # Stream implementation & buffer logic
│   └── 📄 main.cpp                # Engine entry point & driver test
├── 📄 .gitignore                  # Source control exclusion rules
├── 📄 build.sh                    # One-command build automation script
├── 📄 CMakeLists.txt              # Cross-platform CMake configuration
├── 📄 compile.txt                 # Build reference guide
└── 📄 README.md                   # Project documentation
```

---

## 💡 Quick API Example

```cpp
#include "xary/core/Stream.hpp"
#include <iostream>

int main() {
    // Open a binary file with 64 KB chunk allocation
    xary::core::Stream stream("data.bin", 64 * 1024);

    if (!stream.isOpen()) {
        std::cerr << "Failed to open stream!" << std::endl;
        return 1;
    }

    std::vector<uint8_t> buffer;
    while (std::size_t bytesRead = stream.readChunk(buffer)) {
        // Process 64 KB chunk buffer in memory...
    }

    return 0;
}
```

---

## 🛠️ Building & Installation

### 📋 Prerequisites

| Requirement | Supported Versions |
| :--- | :--- |
| **Compiler** | GCC 10+ / Clang 11+ / MSVC 2019+ |
| **Build System** | CMake 3.20+ |
| **Environment** | Git Bash / Linux Shell / macOS Terminal |

---

### ⚡ Option 1: Automated One-Command Build (Recommended)

Run the included build script to automatically configure, compile, and execute **Xary**:

```bash
# Make script executable (first time only)
chmod +x build.sh

# Run automated build pipeline
./build.sh
```

---

### 🔧 Option 2: Manual CMake Build

```bash
# Create and step into build directory
mkdir -p build && cd build

# Configure CMake with MinGW generator
cmake -G "MinGW Makefiles" ..

# Compile binary target
cmake --build .

# Run Xary executable
./xary.exe
```

---

## 📄 License

This project is licensed under the **MIT License**.