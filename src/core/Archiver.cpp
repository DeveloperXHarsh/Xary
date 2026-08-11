#include "../../include/xary/core/Archiver.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <system_error>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Core Secure Archiver (Archiver.cpp)
 * Description : Stream-based folder packer & extractor with complete directory
 *               reconstruction, path obfuscation, and full metadata encryption.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

inline uint8_t rotl8(uint8_t value, unsigned int count) noexcept {
    return static_cast<uint8_t>((value << count) | (value >> (8 - count)));
}

inline uint8_t rotr8(uint8_t value, unsigned int count) noexcept {
    return static_cast<uint8_t>((value >> count) | (value << (8 - count)));
}

void Archiver::encryptChunk(std::span<uint8_t> chunk, uint32_t key) noexcept {
    const size_t size = chunk.size();
    const uint8_t keyBytes[4] = {
        static_cast<uint8_t>(key & 0xFF),
        static_cast<uint8_t>((key >> 8) & 0xFF),
        static_cast<uint8_t>((key >> 16) & 0xFF),
        static_cast<uint8_t>((key >> 24) & 0xFF)
    };

    #pragma GCC unroll 16
    for (size_t i = 0; i < size; ++i) {
        uint8_t b = chunk[i] ^ keyBytes[i % 4];
        chunk[i] = rotl8(b, 3);
    }
}

void Archiver::decryptChunk(std::span<uint8_t> chunk, uint32_t key) noexcept {
    const size_t size = chunk.size();
    const uint8_t keyBytes[4] = {
        static_cast<uint8_t>(key & 0xFF),
        static_cast<uint8_t>((key >> 8) & 0xFF),
        static_cast<uint8_t>((key >> 16) & 0xFF),
        static_cast<uint8_t>((key >> 24) & 0xFF)
    };

    #pragma GCC unroll 16
    for (size_t i = 0; i < size; ++i) {
        uint8_t b = rotr8(chunk[i], 3);
        chunk[i] = b ^ keyBytes[i % 4];
    }
}

bool Archiver::pack(const std::filesystem::path& inputPath, 
                    const std::filesystem::path& outputPath, 
                    bool secureMode, 
                    uint32_t key) {
    std::error_code ec;
    if (!std::filesystem::exists(inputPath, ec)) {
        std::cerr << "❌ Error: Target path '" << inputPath.string() << "' does not exist.\n";
        return false;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) return false;

    // 1. Write Header Magic
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

    auto writeEntry = [&](const std::filesystem::path& path, const std::string& genericRelPath, EntryType type) {
        uint8_t typeByte = static_cast<uint8_t>(type);
        uint32_t pathLen = static_cast<uint32_t>(genericRelPath.size());
        uint64_t fileSize = (type == EntryType::File) ? std::filesystem::file_size(path, ec) : 0;

        if (secureMode) {
            // Mask metadata headers
            uint8_t encType = typeByte ^ static_cast<uint8_t>(key & 0xFF);
            uint32_t encPathLen = pathLen ^ key;
            uint64_t mask64 = (static_cast<uint64_t>(key) << 32) | static_cast<uint64_t>(key);
            uint64_t encFileSize = fileSize ^ mask64;

            outFile.write(reinterpret_cast<const char*>(&encType), sizeof(encType));
            outFile.write(reinterpret_cast<const char*>(&encPathLen), sizeof(encPathLen));

            // Encrypt path string in-place
            std::vector<uint8_t> pathBuffer(genericRelPath.begin(), genericRelPath.end());
            encryptChunk(pathBuffer, key);
            outFile.write(reinterpret_cast<const char*>(pathBuffer.data()), pathBuffer.size());

            if (type == EntryType::File) {
                outFile.write(reinterpret_cast<const char*>(&encFileSize), sizeof(encFileSize));

                std::ifstream inFile(path, std::ios::binary);
                uint64_t bytesRemaining = fileSize;

                while (bytesRemaining > 0 && inFile) {
                    size_t toRead = static_cast<size_t>(std::min<uint64_t>(bytesRemaining, CHUNK_SIZE));
                    inFile.read(reinterpret_cast<char*>(buffer.data()), toRead);
                    size_t readCount = inFile.gcount();
                    if (readCount == 0) break;

                    encryptChunk(std::span<uint8_t>(buffer.data(), readCount), key);
                    outFile.write(reinterpret_cast<const char*>(buffer.data()), readCount);
                    bytesRemaining -= readCount;
                }
            }
        } else {
            // Standard plain mode
            outFile.write(reinterpret_cast<const char*>(&typeByte), sizeof(typeByte));
            outFile.write(reinterpret_cast<const char*>(&pathLen), sizeof(pathLen));
            outFile.write(genericRelPath.data(), pathLen);

            if (type == EntryType::File) {
                outFile.write(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));

                std::ifstream inFile(path, std::ios::binary);
                uint64_t bytesRemaining = fileSize;

                while (bytesRemaining > 0 && inFile) {
                    size_t toRead = static_cast<size_t>(std::min<uint64_t>(bytesRemaining, CHUNK_SIZE));
                    inFile.read(reinterpret_cast<char*>(buffer.data()), toRead);
                    size_t readCount = inFile.gcount();
                    if (readCount == 0) break;

                    outFile.write(reinterpret_cast<const char*>(buffer.data()), readCount);
                    bytesRemaining -= readCount;
                }
            }
        }
    };

    if (std::filesystem::is_directory(inputPath, ec)) {
        for (auto it = std::filesystem::recursive_directory_iterator(inputPath, ec);
             it != std::filesystem::recursive_directory_iterator(); ++it) {
            
            std::string genericRel = std::filesystem::relative(it->path(), inputPath, ec).generic_string();
            
            if (it->is_directory(ec)) {
                writeEntry(it->path(), genericRel, EntryType::Directory);
            } else if (it->is_regular_file(ec)) {
                writeEntry(it->path(), genericRel, EntryType::File);
            }
        }
    } else if (std::filesystem::is_regular_file(inputPath, ec)) {
        writeEntry(inputPath, inputPath.filename().generic_string(), EntryType::File);
    }

    return true;
}

bool Archiver::unpack(const std::filesystem::path& archivePath, 
                      const std::filesystem::path& outputDir, 
                      uint32_t key) {
    std::ifstream inFile(archivePath, std::ios::binary);
    if (!inFile.is_open()) return false;

    uint8_t magic[4];
    if (!inFile.read(reinterpret_cast<char*>(magic), 4)) return false;

    bool secureMode = false;
    uint8_t expectedStealth[4];
    for (int i = 0; i < 4; ++i) {
        expectedStealth[i] = STEALTH_MAGIC[i] ^ static_cast<uint8_t>(key >> (i * 8));
    }

    if (std::equal(magic, magic + 4, expectedStealth)) {
        secureMode = true;
    } else if (!std::equal(magic, magic + 4, NORMAL_MAGIC)) {
        return false;
    }

    std::error_code ec;
    std::filesystem::create_directories(outputDir, ec);

    std::vector<uint8_t> buffer(CHUNK_SIZE);

    while (inFile.peek() != EOF) {
        uint8_t typeByte = 0;
        uint32_t pathLen = 0;
        std::string relPath;
        uint64_t fileSize = 0;

        if (secureMode) {
            uint8_t encType = 0;
            if (!inFile.read(reinterpret_cast<char*>(&encType), sizeof(encType))) break;
            typeByte = encType ^ static_cast<uint8_t>(key & 0xFF);

            uint32_t encPathLen = 0;
            if (!inFile.read(reinterpret_cast<char*>(&encPathLen), sizeof(encPathLen))) break;
            pathLen = encPathLen ^ key;

            if (pathLen == 0 || pathLen > 8192) {
                return false;
            }

            std::vector<uint8_t> pathBuffer(pathLen);
            if (!inFile.read(reinterpret_cast<char*>(pathBuffer.data()), pathLen)) break;
            decryptChunk(pathBuffer, key);
            relPath.assign(pathBuffer.begin(), pathBuffer.end());

            // Direct C++20 path concatenation (replaces deprecated u8path)
            std::filesystem::path targetPath = outputDir / relPath;

            if (static_cast<EntryType>(typeByte) == EntryType::Directory) {
                std::filesystem::create_directories(targetPath, ec);
                continue;
            }

            std::filesystem::create_directories(targetPath.parent_path(), ec);

            uint64_t encFileSize = 0;
            if (!inFile.read(reinterpret_cast<char*>(&encFileSize), sizeof(encFileSize))) break;
            uint64_t mask64 = (static_cast<uint64_t>(key) << 32) | static_cast<uint64_t>(key);
            fileSize = encFileSize ^ mask64;

            std::ofstream outFile(targetPath, std::ios::binary);
            uint64_t bytesRemaining = fileSize;

            while (bytesRemaining > 0 && inFile) {
                size_t toRead = static_cast<size_t>(std::min<uint64_t>(bytesRemaining, CHUNK_SIZE));
                inFile.read(reinterpret_cast<char*>(buffer.data()), toRead);
                size_t readCount = inFile.gcount();
                if (readCount == 0) break;

                decryptChunk(std::span<uint8_t>(buffer.data(), readCount), key);
                outFile.write(reinterpret_cast<const char*>(buffer.data()), readCount);
                bytesRemaining -= readCount;
            }
        } else {
            if (!inFile.read(reinterpret_cast<char*>(&typeByte), sizeof(typeByte))) break;
            if (!inFile.read(reinterpret_cast<char*>(&pathLen), sizeof(pathLen))) break;

            relPath.resize(pathLen);
            if (!inFile.read(&relPath[0], pathLen)) break;

            // Direct C++20 path concatenation (replaces deprecated u8path)
            std::filesystem::path targetPath = outputDir / relPath;

            if (static_cast<EntryType>(typeByte) == EntryType::Directory) {
                std::filesystem::create_directories(targetPath, ec);
                continue;
            }

            std::filesystem::create_directories(targetPath.parent_path(), ec);

            if (!inFile.read(reinterpret_cast<char*>(&fileSize), sizeof(fileSize))) break;

            std::ofstream outFile(targetPath, std::ios::binary);
            uint64_t bytesRemaining = fileSize;

            while (bytesRemaining > 0 && inFile) {
                size_t toRead = static_cast<size_t>(std::min<uint64_t>(bytesRemaining, CHUNK_SIZE));
                inFile.read(reinterpret_cast<char*>(buffer.data()), toRead);
                size_t readCount = inFile.gcount();
                if (readCount == 0) break;

                outFile.write(reinterpret_cast<const char*>(buffer.data()), readCount);
                bytesRemaining -= readCount;
            }
        }
    }

    return true;
}

} // namespace xary::core