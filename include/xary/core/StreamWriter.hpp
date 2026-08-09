#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <vector>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Core Stream Writer (StreamWriter.hpp)
 * Description : High-throughput binary output stream writer featuring direct stack
 *               buffer streaming, zero heap allocation, std::span integration,
 *               and RAII file handle lifecycle management.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

class StreamWriter {
public:
    explicit StreamWriter(const std::filesystem::path& filepath);
    ~StreamWriter() = default;

    // StreamWriter objects manage OS file handles and are move-only
    StreamWriter(const StreamWriter&) = delete;
    StreamWriter& operator=(const StreamWriter&) = delete;
    StreamWriter(StreamWriter&&) noexcept = default;
    StreamWriter& operator=(StreamWriter&&) noexcept = default;

    [[nodiscard]] bool isOpen() const noexcept;

    [[nodiscard]] bool writeChunk(const std::vector<std::uint8_t>& buffer);
    [[nodiscard]] bool writeChunk(std::span<const std::uint8_t> data);
    [[nodiscard]] bool writeChunk(const std::uint8_t* data, std::size_t size);

    bool flush() noexcept;

private:
    std::filesystem::path m_filepath;
    std::ofstream m_fileStream;
};

} // namespace xary::core
