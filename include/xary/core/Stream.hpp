#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Core Stream Engine (Stream.hpp)
 * Description : High-performance input stream reader designed for chunked
 *               binary file ingestion, RAII stream lifecycle management, and
 *               zero-heap pointer indirection.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

class Stream {
public:
    explicit Stream(const std::filesystem::path& filepath, std::size_t chunkSize = 64 * 1024);
    ~Stream() = default;

    // Stream objects manage OS file handles and are move-only
    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;
    Stream(Stream&&) noexcept = default;
    Stream& operator=(Stream&&) noexcept = default;

    [[nodiscard]] bool isOpen() const noexcept;
    [[nodiscard]] std::uint64_t getFileSize() const noexcept;
    [[nodiscard]] std::size_t getChunkSize() const noexcept;

    std::size_t readChunk(std::vector<std::uint8_t>& buffer);
    void reset() noexcept;

private:
    std::filesystem::path m_filepath;
    std::size_t m_chunkSize;
    std::uint64_t m_fileSize{0};
    std::ifstream m_fileStream;
};

} // namespace xary::core
