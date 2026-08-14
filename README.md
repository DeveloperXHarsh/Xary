<div align="center">

# ⚡ XARY BINARY ENGINE

**An ultra-fast C++20 tool for processing binary files, packing folders securely, and inspecting file signatures.**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-4.4%2B-B22222?style=for-the-badge&logo=cmake&logoColor=white)](https://cmake.org/)
[![Developer](https://img.shields.io/badge/Developer-DeveloperXHarsh-663399?style=for-the-badge&logo=github&logoColor=white)](https://github.com/DeveloperXHarsh)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-black?style=for-the-badge&logo=windows&logoColor=white)](#)
[![License](https://img.shields.io/badge/License-MIT-FFBF00.svg?style=for-the-badge)](#)

---

</div>

<h2 align="center">📌 Engine Overview</h2>

**Xary Engine** is a lightweight C++20 command-line tool and software library built to handle binary files and directories quickly, safely, and securely.

Normally, when software opens a large file (like 10 GB), it tries to load the entire file into your computer's RAM memory. This slows down your system and can cause crashes. **Xary solves this problem.**

Xary processes files in small **64 KB pieces (chunks)**. Whether your file is 10 Megabytes or 500 Gigabytes, Xary uses almost **zero extra RAM memory**.

### What can you do with Xary?
1. **Pack Folders Securely (`--pack`)**: Turn any folder and its contents into a single archive file (`.xary`).
2. **Hide File Information (`--sec`)**: Enable stealth mode to hide and encrypt file names, folder paths, and file sizes so nobody can read them inside the packed file.
3. **Unpack Archives (`--unpack`)**: Instantly restore your packed `.xary` archives back to their original folders and files.
4. **Identify File Types (`--inspect`)**: Find out what kind of file something is (PNG, PDF, MP4, EXE, etc.) by checking its hidden internal signature, even if someone changed the file extension.
5. **Encrypt Files (`--encode` / `--decode`)**: Fast file encryption and decryption using a custom key.

---

<h2 align="center">🌱 Beginner's Corner: Key Concepts Explained</h2>

If you are new to C++ or binary tools, here is a simple breakdown of the main terms used in this project:

* **What is Stream Processing?**  
  Instead of loading an entire book into memory at once, stream processing reads it one page at a time. Xary reads files in small 64 KB pieces at a time.
* **What is Zero-Allocation?**  
  "Dynamic allocation" means asking your computer for new RAM memory while a program runs (which is slow). "Zero-allocation" means Xary reuses the exact same tiny memory space over and over, making it extremely fast.
* **What is Stealth Mode (`--sec`)?**  
  Standard ZIP files allow anyone to see the names of files inside an archive. Xary's Stealth Mode scrambles file names, folder paths, and file data into random noise so no text reader can inspect your content.

---

<h2 align="center">🚀 Main Features</h2>

* **📦 Low Memory Usage**: Uses a fixed 64 KB buffer space so memory usage never spikes.
* **⚡ Vectorized Speed**: Uses SIMD hardware instructions (`#pragma GCC unroll 16`) to speed up byte rotation and encryption.
* **🔐 Full Metadata Obfuscation**: Encrypts file paths, folder structures, entry types, and byte sizes.
* **📂 Directory Reconstruction**: Automatically recreates complex subfolders when extracting archives.
* **🛡️ Smart Format Detector**: Identifies real file types (PNG, JPEG, PDF, MP4, EXE, etc.) using magic byte pattern matching.
* **⚙️ Fast Command Line Tool**: Zero-copy argument parsing directly from terminal inputs (`argv`).
* **🏭 Modern Build System**: Easy build process configured with CMake 4.4+ and C++20 standards.

---

<h2 align="center">📐 Binary Layout (How Archives Look)</h2>

```text
┌────────────────────────────────────────────────────────────────────────┐
│                        XARY BINARY CONTAINER                          │
├───────────────┬─────────────────┬─────────────────┬────────────────────┤
│ Magic (4B)    │ Entry Type (1B) │ Path Len (4B)   │ Path String (Var)  │
├───────────────┼─────────────────┼─────────────────┼────────────────────┤
│ Size Header   │ File Payload    │ Next Entry...   │ End of File (EOF)  │
│ (8 Bytes)     │ (64 KB Chunks)  │ ...             │                    │
└───────────────┴─────────────────┴─────────────────┴────────────────────┘
```

* **Standard Archive**: Starts with the header identifier `XARY` and stores file paths clearly.
* **Stealth Archive (`--sec`)**: Scrambles the header, file names, folder paths, and file contents so the file looks like random noise when opened in a text editor.

---

<h2 align="center">📂 Project Folder Structure</h2>

```text
xary/
├── 📁 .github/              # CI/CD build scripts
├── 📁 include/
│   └── 📁 xary/
│       ├── 📁 cli/
│       │   └── 📄 ArgumentParser.hpp   # Terminal command parser
│       └── 📁 core/
│           ├── 📄 Archiver.hpp         # Folder packing & stealth encryption module
│           ├── 📄 BufferView.hpp       # Memory buffer view helper
│           ├── 📄 FileTypeDetector.hpp # File type detector
│           ├── 📄 Stream.hpp           # 64 KB chunk binary reader
│           └── 📄 StreamWriter.hpp     # Binary file writer
├── 📁 src/
│   ├── 📁 cli/
│   │   └── 📄 ArgumentParser.cpp   # Command validation logic
│   ├── 📁 core/
│   │   ├── 📄 Archiver.cpp         # Folder packing, extraction & cipher routines
│   │   ├── 📄 BufferView.cpp       # Memory view helper implementation
│   │   ├── 📄 FileTypeDetector.cpp # File signature matcher
│   │   ├── 📄 Stream.cpp           # Stream buffer manager
│   │   └── 📄 StreamWriter.cpp     # File writer implementation
│   └── 📄 main.cpp                 # Main program entry point
├── 📁 tests/                  # Automated test files
├── 📄 build.sh                # Automated build script
├── 📄 CMakeLists.txt          # CMake 4.4+ build setup
├── 📄 LICENSE                 # MIT License file
└── 📄 README.md               # Project documentation
```

---

<h2 align="center">💻 How to Use the Command Line</h2>

You can run Xary directly from your terminal or command prompt:

```bash
# Show help menu and options
xary --help

# Check what type of file something is (Detect real format)
xary -i mystery_file.png

# Encrypt a single file with a secret key
xary -e my_file.txt -o encrypted.xary -k 0x5A9C3F11

# Decrypt an encrypted file back to its original state
xary -d encrypted.xary -o restored.txt -k 0x5A9C3F11

# Pack a folder into a standard archive file
xary -p ./my_folder -o archive.xary

# Pack a folder in Stealth Mode (--sec) so file names are hidden
xary -p ./private_folder -o vault.xary --sec -k 0xDEADBEEF

# Unpack an archive file back into a folder
xary -u vault.xary -o ./extracted_folder -k 0xDEADBEEF
```

### 🎛️ Command Flags Reference

| Flag | Long Flag | Parameter | What it does |
| :--- | :--- | :--- | :--- |
| `-h` | `--help` | — | Displays the help menu and usage instructions |
| `-v` | `--version` | — | Shows the current software version |
| `-p` | `--pack` | `<folder>` | Packs a folder or file into an archive |
| `-u` | `--unpack` | `<file>` | Unpacks an archive and restores its folder structure |
| `-e` | `--encode` | `<file>` | Encrypts a single file using a stream cipher |
| `-d` | `--decode` | `<file>` | Decrypts an encrypted file back to original data |
| `-i` | `--inspect` | `<file>` | Reads the file header and detects its real MIME type |
| `-o` | `--output` | `<path>` | Sets the output file or destination directory path |
| `—`  | `--sec` | — | Turns on Stealth Mode (hides file names, paths, and sizes) |
| `-k` | `--key` | `<hex/int>`| Sets a custom encryption key (Default: `0x5A9C3F11`) |

---

<h2 align="center">💡 C++ Developer Code Examples</h2>

### 1. Packing and Unpacking Folders in C++

```cpp
#include "xary/core/Archiver.hpp"
#include <iostream>

int main() {
    xary::core::Archiver archiver;
    uint32_t secretKey = 0x8F3A2B1C;

    // Pack a folder using stealth mode (--sec)
    bool packed = archiver.pack("./my_folder", "vault.xary", /*secureMode=*/true, secretKey);
    if (packed) {
        std::cout << "Folder successfully packed and encrypted!\n";
    }

    // Unpack the archive back into a folder
    bool unpacked = archiver.unpack("vault.xary", "./restored_folder", secretKey);
    if (unpacked) {
        std::cout << "Folder successfully restored!\n";
    }

    return 0;
}
```

### 2. Reading Binary Streams & Detecting File Types in C++

```cpp
#include "xary/core/Stream.hpp"
#include "xary/core/BufferView.hpp"
#include "xary/core/FileTypeDetector.hpp"
#include <iostream>

int main() {
    // Open a file using a 64 KB streaming buffer
    xary::core::Stream reader("unknown_file.bin", 64 * 1024);
    if (!reader.isOpen()) return 1;

    // Read the first chunk of data
    std::vector<uint8_t> buffer;
    reader.readChunk(buffer);

    // Inspect the header to find the real format
    xary::core::BufferView view(buffer);
    xary::core::FileTypeInfo info = xary::core::FileTypeDetector::detect(view);

    std::cout << "MIME Type          : " << info.mimeType << "\n";
    std::cout << "Expected Extension : " << info.expectedExtension << "\n";

    return 0;
}
```

---

<h2 align="center">🛠️ Building the Project</h2>

### 📋 Prerequisites

| Tool | Minimum Version | Recommended Version |
| :--- | :--- | :--- |
| **C++ Compiler** | GCC 10+ / Clang 11+ / MSVC 2019+ | GCC 13+ / Clang 16+ (C++20 compliant) |
| **Build System** | CMake 4.4+ | CMake 4.4+ |
| **Operating System** | Windows, Linux, or macOS | Any 64-bit operating system |

---

### ⚡ Easy Build Method (Using Script)

Run the included `build.sh` script to automatically compile the project using all available CPU cores:

```bash
# Build and run the project
./build.sh

# Rebuild everything from scratch
./build.sh --clean

# Build without automatically running the executable
./build.sh --no-run
```

---

### 🔧 Manual CMake Build Method

```bash
# 1. Create a build folder
mkdir -p build && cd build

# 2. Configure the project with CMake 4.4+
cmake -DCMAKE_BUILD_TYPE=Release ..

# 3. Compile the executable
cmake --build . --parallel

# 4. Run Xary
./xary --version
```

---

<br />

<div align="center">

---

Crafted with 💻 by **[Piyush Rajput (@DeveloperXHarsh)](https://github.com/DeveloperXHarsh)**  
Released under the open-source **[MIT License](https://opensource.org/licenses/MIT)**

</div>
