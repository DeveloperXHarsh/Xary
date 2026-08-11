#pragma once

#include <cstdint>
#include <span>
#include <filesystem>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Core Secure Archiver (Archiver.hpp)
 * Description : Header for high-throughput zero-allocation directory archiver
 *               supporting full metadata/filename encryption and stealth headers.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

enum class EntryType : uint8_t {
    Directory = 1,
    File = 2
};

class Archiver {
public:
    static constexpr uint8_t STEALTH_MAGIC[4] = {0xFA, 0x3C, 0x91, 0xE2};
    static constexpr uint8_t NORMAL_MAGIC[4]  = {0x58, 0x41, 0x52, 0x59}; // "XARY"
    static constexpr uint32_t DEFAULT_KEY     = 0x5A9C3F11;
    static constexpr size_t CHUNK_SIZE         = 64 * 1024;

    Archiver() = default;

    // Fast reversible stream ciphers
    static void encryptChunk(std::span<uint8_t> chunk, uint32_t key) noexcept;
    static void decryptChunk(std::span<uint8_t> chunk, uint32_t key) noexcept;

    // Packs directory/file tree into single archive
    bool pack(const std::filesystem::path& inputPath, 
              const std::filesystem::path& outputPath, 
              bool secureMode, 
              uint32_t key = DEFAULT_KEY);

    // Unpacks archive and reconstructs complete directory structure
    bool unpack(const std::filesystem::path& archivePath, 
                const std::filesystem::path& outputDir, 
                uint32_t key = DEFAULT_KEY);
};

} // namespace xary::core