<div align="center">

# ⚡ XARY BINARY ENGINE

**Zero-Allocation C++20 Stream Processing, SIMD Memory Inspection & Binary Transformation Pipeline**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-B22222?style=for-the-badge&logo=cmake&logoColor=white)](https://cmake.org/)
[![Developer](https://img.shields.io/badge/Developer-DeveloperXHarsh-663399?style=for-the-badge&logo=github&logoColor=white)](https://github.com/DeveloperXHarsh)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-black?style=for-the-badge&logo=windows&logoColor=white)](#)
[![License](https://img.shields.io/badge/License-MIT-FFBF00.svg?style=for-the-badge)](#)

---

</div>

## 📌 Overview

**Xary Engine** is a high-performance C++20 binary execution framework engineered for zero-allocation stream processing, SIMD byte manipulation, and instant file format detection. 

Operating on a strict **64 KB chunk footprint**, Xary processes multi-gigabyte binary files with deterministic memory consumption. It incorporates Link-Time Optimization (LTO/IPO), custom vector-unrolled stream ciphers, non-owning memory views (`std::span`), and a data-driven magic signature scanner.

---

## 🔥 Modern Architecture Features

* **🚀 Zero-Allocation Memory Slicing**: Instant $O(1)$ byte subview inspection powered by `BufferView` and `std::span<const uint8_t>` without dynamic memory allocations.
* **⚡ Hardware SIMD Scanning**: Uses `std::memchr` assembly routines for high-throughput single-byte searches and `#pragma GCC unroll 16` cipher loop vectorization.
* **🛡️ Magic Byte Inspection**: Table-driven pattern detector (`FileTypeDetector`) supporting non-owning `std::string_view` format signatures and offset-aware header matching (PNG, JPEG, PDF, MP4, EXE, ELF, etc.).
* **📦 Stack-Allocated Move-Only I/O**: High-efficiency stream writer (`StreamWriter`) with stack-managed handle lifecycles, explicit flushing, and `[[nodiscard]]` return safety guarantees.
* **⚙️ Zero-Copy CLI Parsing**: Subcommand parser (`ArgumentParser`) using `std::span` token iteration directly over `argv` without `std::vector<std::string>` dynamic conversions.
* **🏭 Industrial Build System**: Modern CMake build with `CMAKE_INTERPROCEDURAL_OPTIMIZATION` (LTO), target-scoped generator flags, and `-march=native` hardware tuning.

---

## 📂 Project Architecture

```text
xary/
├── 📁 .github/             # GitHub Actions CI/CD workflows
├── 📁 include/
│   └── 📁 xary/
│       ├── 📁 cli/
│       │   └── 📄 ArgumentParser.hpp
│       └── 📁 core/
│           ├── 📄 BufferView.hpp
│           ├── 📄 FileTypeDetector.hpp
│           ├── 📄 Stream.hpp
│           └── 📄 StreamWriter.hpp
├── 📁 src/
│   ├── 📁 cli/
│   │   └── 📄 ArgumentParser.cpp
│   ├── 📁 core/
│   │   ├── 📄 BufferView.cpp
│   │   ├── 📄 FileTypeDetector.cpp
│   │   ├── 📄 Stream.cpp
│   │   └── 📄 StreamWriter.cpp
│   └── 📄 main.cpp
├── 📁 tests/               # Test suites and test fixtures
├── 📄 .gitignore           # Source control exclusion rules
├── 📄 build.sh             # Multi-threaded build automation script
├── 📄 CMakeLists.txt       # Modern CMake configuration
├── 📄 compile_flags.txt    # Language server & IntelliSense flags
├── 📄 compiler.txt         # Compiler environment notes
├── 📄 LICENSE              # MIT License file
└── 📄 README.md            # Engine technical documentation
```

---

## 💻 Command Line Usage

Xary features an integrated CLI toolset for encoding, decoding, and inspecting binary streams:

```bash
# Display general help menu
xary --help

# Inspect file magic bytes & MIME information
xary -i corrupted_file.png

# Encode binary file into chunked Xary stream
xary -e input.bin -o output.xary

# Decode Xary binary stream back to original format
xary -d output.xary -o restored.bin
```

### 🎛️ Command Options Reference

| Flag | Long Flag | Parameter | Description |
| :--- | :--- | :--- | :--- |
| `-h` | `--help` | — | Display help menu and command documentation |
| `-v` | `--version` | — | Output engine version and build parameters |
| `-e` | `--encode` | `<file>` | Encode binary stream using vector-unrolled stream cipher |
| `-d` | `--decode` | `<file>` | Process/Decrypt Xary binary stream into output file |
| `-i` | `--inspect` | `<file>` | Inspect binary header magic bytes & detect MIME format |
| `-o` | `--output` | `<file>` | Specify target destination file path |

---

## 💡 C++20 Core API Example

```cpp
#include "xary/core/Stream.hpp"
#include "xary/core/StreamWriter.hpp"
#include "xary/core/BufferView.hpp"
#include "xary/core/FileTypeDetector.hpp"
#include <iostream>

int main() {
    // Open a 64 KB chunked binary reader
    xary::core::Stream reader("data.bin", 64 * 1024);
    if (!reader.isOpen()) return 1;

    // Read first chunk and inspect magic signature
    std::vector<uint8_t> buffer;
    reader.readChunk(buffer);

    xary::core::BufferView view(buffer);
    xary::core::FileTypeInfo info = xary::core::FileTypeDetector::detect(view);

    std::cout << "MIME Type    : " << info.mimeType << "\n";
    std::cout << "Detected Ext : " << info.expectedExtension << "\n";

    // Write chunk back using move-only stack writer
    xary::core::StreamWriter writer("output.bin");
    if (writer.isOpen()) {
        writer.writeChunk(buffer);
        writer.flush();
    }

    return 0;
}
```

---

## 🛠️ Building & Automation

### 📋 Prerequisites

| Tool | Minimum Version | Recommended |
| :--- | :--- | :--- |
| **Compiler** | GCC 10+ / Clang 11+ / MSVC 2019+ | GCC 13+ / Clang 16+ (C++20 compliant) |
| **Build System** | CMake 3.20+ | CMake 3.28+ |
| **Environment** | Bash Shell / Git Bash / Terminal | Multi-core CPU for parallel builds |

---

### ⚡ Option 1: Multi-Threaded Build Script (Recommended)

The included `build.sh` script automatically detects system CPU threads (`nproc`/`sysctl`) and compiles with parallel threads:

```bash
# Basic parallel build & immediate run
./build.sh

# Force clean workspace rebuild
./build.sh --clean

# Compile with Debug symbols
./build.sh --debug

# Compile target without running executable
./build.sh --no-run
```

---

### 🔧 Option 2: Manual CMake Pipeline

```bash
# Create build directory
mkdir -p build && cd build

# Configure CMake build system
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

# Compile target with full CPU thread parallelization
cmake --build . --parallel

# Execute binary engine
./xary.exe --version
```

---

---

<div align="center">
  <p><b>Developed with 💻 by <a href="https://github.com/DeveloperXHarsh">Piyush Rajput (@DeveloperXHarsh)</a></b></p>
  <p>Released under the <a href="https://opensource.org/licenses/MIT">MIT License</a></p>
</div>