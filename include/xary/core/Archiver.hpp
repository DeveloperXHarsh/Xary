#ifndef XARY_CORE_ARCHIVER_HPP
#define XARY_CORE_ARCHIVER_HPP

#include <cstdint>
#include <string>
#include <span>
#include <filesystem>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Core Secure Archiver (Archiver.hpp)
 * Description : Header for high-throughput zero-allocation directory archiver
 *               supporting standard and stealth obfuscated (--sec) formats.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

struct FileHeader {
    uint32_t pathLength{0};
    std::string relativePath;
    uint64_t fileSize{0};
};

class Archiver {
public:
    // High-entropy non-ASCII magic bytes for --sec mode (unreadable by hex/strings)
    static constexpr uint8_t STEALTH_MAGIC[4] = {0xFA, 0x3C, 0x91, 0xE2};
    static constexpr uint8_t NORMAL_MAGIC[4]  = {0x58, 0x41, 0x52, 0x59}; // "XARY"
    static constexpr uint32_t DEFAULT_KEY    = 0x5A9C3F11;
    static constexpr size_t CHUNK_SIZE        = 64 * 1024; // Strict 64 KB memory footprint

    Archiver() = default;

    // Fast in-place SIMD-friendly bitwise transform for 64 KB buffer chunks
    static void transformChunk(std::span<uint8_t> chunk, uint32_t key) noexcept;

    // Packs a file or directory tree into a single archive
    bool pack(const std::filesystem::path& inputPath, 
              const std::filesystem::path& outputPath, 
              bool secureMode, 
              uint32_t key = DEFAULT_KEY);

    // Unpacks an archive back to disk
    bool unpack(const std::filesystem::path& archivePath, 
                const std::filesystem::path& outputDir, 
                uint32_t key = DEFAULT_KEY);
};

} // namespace xary::core

#endif // XARY_CORE_ARCHIVER_HPP
