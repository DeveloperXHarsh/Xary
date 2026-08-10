#include "../../include/xary/core/Archiver.hpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <system_error>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Core Secure Archiver (Archiver.cpp)
 * Description : Implementation of C++20 stream-based folder packer/extractor
 *               with --sec obfuscation layer and bounded 64 KB chunk buffers.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

void Archiver::transformChunk(std::span<uint8_t> chunk, uint32_t key) noexcept {
    const size_t size = chunk.size();
    const uint8_t keyBytes[4] = {
        static_cast<uint8_t>(key & 0xFF),
        static_cast<uint8_t>((key >> 8) & 0xFF),
        static_cast<uint8_t>((key >> 16) & 0xFF),
        static_cast<uint8_t>((key >> 24) & 0xFF)
    };

    // Vector-unrolled bitwise XOR + bit-rotation cipher
    #pragma GCC unroll 16
    for (size_t i = 0; i < size; ++i) {
        uint8_t b = chunk[i];
        // Bitwise rotation right by 3 + XOR key masking
        b = static_cast<uint8_t>((b >> 3) | (b << 5));
        chunk[i] = b ^ keyBytes[i % 4];
    }
}

bool Archiver::pack(const std::filesystem::path& inputPath, 
                    const std::filesystem::path& outputPath, 
                    bool secureMode, 
                    uint32_t key) {
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) return false;

    // 1. Write File Signature Header
    if (secureMode) {
        uint8_t obfuscatedMagic[4];
        for (int i = 0; i < 4; ++i) {
            obfuscatedMagic[i] = STEALTH_MAGIC[i] ^ static_cast<uint8_t>(key >> (i * 8));
        }
        outFile.write(reinterpret_cast<const char*>(obfuscatedMagic), 4);
    } else {
        outFile.write(reinterpret_cast<const char*>(NORMAL_MAGIC), 4);
    }

    std::vector<uint8_t> buffer(CHUNK_SIZE);
    std::error_code ec;

    // Helper closure to process single file streaming
    auto processFile = [&](const std::filesystem::path& filePath, const std::string& relPath) {
        uint64_t fileSize = std::filesystem::file_size(filePath, ec);
        uint32_t pathLen = static_cast<uint32_t>(relPath.size());

        // Write entry header
        outFile.write(reinterpret_cast<const char*>(&pathLen), sizeof(pathLen));
        outFile.write(relPath.data(), pathLen);
        outFile.write(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));

        std::ifstream inFile(filePath, std::ios::binary);
        uint64_t bytesRemaining = fileSize;

        while (bytesRemaining > 0 && inFile) {
            size_t bytesToRead = static_cast<size_t>(std::min<uint64_t>(bytesRemaining, CHUNK_SIZE));
            inFile.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);
            size_t bytesRead = inFile.gcount();

            if (bytesRead == 0) break;

            if (secureMode) {
                transformChunk(std::span<uint8_t>(buffer.data(), bytesRead), key);
            }

            outFile.write(reinterpret_cast<const char*>(buffer.data()), bytesRead);
            bytesRemaining -= bytesRead;
        }
    };

    if (std::filesystem::is_directory(inputPath, ec)) {
        for (auto it = std::filesystem::recursive_directory_iterator(inputPath, ec);
             it != std::filesystem::recursive_directory_iterator(); ++it) {
            if (it->is_regular_file(ec)) {
                std::string relPath = std::filesystem::relative(it->path(), inputPath, ec).string();
                processFile(it->path(), relPath);
            }
        }
    } else if (std::filesystem::is_regular_file(inputPath, ec)) {
        processFile(inputPath, inputPath.filename().string());
    }

    return true;
}

bool Archiver::unpack(const std::filesystem::path& archivePath, 
                      const std::filesystem::path& outputDir, 
                      uint32_t key) {
    std::ifstream inFile(archivePath, std::ios::binary);
    if (!inFile.is_open()) return false;

    // Read magic bytes
    uint8_t magic[4];
    inFile.read(reinterpret_cast<char*>(magic), 4);

    bool secureMode = false;
    uint8_t expectedStealth[4];
    for (int i = 0; i < 4; ++i) {
        expectedStealth[i] = STEALTH_MAGIC[i] ^ static_cast<uint8_t>(key >> (i * 8));
    }

    if (std::equal(magic, magic + 4, expectedStealth)) {
        secureMode = true;
    } else if (!std::equal(magic, magic + 4, NORMAL_MAGIC)) {
        std::cerr << "[Xary Archiver] Error: Unknown or corrupted archive format.\n";
        return false;
    }

    std::vector<uint8_t> buffer(CHUNK_SIZE);
    std::error_code ec;

    while (inFile.peek() != EOF) {
        uint32_t pathLen = 0;
        if (!inFile.read(reinterpret_cast<char*>(&pathLen), sizeof(pathLen))) break;

        std::string relPath(pathLen, '\0');
        inFile.read(&relPath[0], pathLen);

        uint64_t fileSize = 0;
        inFile.read(reinterpret_cast<char*>(&fileSize), sizeof(fileSize));

        std::filesystem::path targetPath = outputDir / relPath;
        std::filesystem::create_directories(targetPath.parent_path(), ec);

        std::ofstream outFile(targetPath, std::ios::binary);
        uint64_t bytesRemaining = fileSize;

        while (bytesRemaining > 0 && inFile) {
            size_t bytesToRead = static_cast<size_t>(std::min<uint64_t>(bytesRemaining, CHUNK_SIZE));
            inFile.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);
            size_t bytesRead = inFile.gcount();

            if (bytesRead == 0) break;

            if (secureMode) {
                transformChunk(std::span<uint8_t>(buffer.data(), bytesRead), key);
            }

            outFile.write(reinterpret_cast<const char*>(buffer.data()), bytesRead);
            bytesRemaining -= bytesRead;
        }
    }

    return true;
}

} // namespace xary::core