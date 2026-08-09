#include "../../include/xary/core/StreamWriter.hpp"

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Core Stream Writer (StreamWriter.cpp)
 * Description : Implementation of binary block writer optimized for unbuffered
 *               raw byte sequence writes, cross-platform paths, and stream safety.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

StreamWriter::StreamWriter(const std::filesystem::path& filepath)
    : m_filepath(filepath), m_fileStream(filepath, std::ios::binary | std::ios::trunc) {}

bool StreamWriter::isOpen() const noexcept {
    return m_fileStream.is_open() && m_fileStream.good();
}

bool StreamWriter::writeChunk(const std::vector<std::uint8_t>& buffer) {
    return writeChunk(buffer.data(), buffer.size());
}

bool StreamWriter::writeChunk(std::span<const std::uint8_t> data) {
    return writeChunk(data.data(), data.size());
}

bool StreamWriter::writeChunk(const std::uint8_t* data, std::size_t size) {
    if (!isOpen() || !data || size == 0) {
        return false;
    }

    m_fileStream.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    return m_fileStream.good();
}

bool StreamWriter::flush() noexcept {
    if (m_fileStream.is_open()) {
        m_fileStream.flush();
        return m_fileStream.good();
    }
    return false;
}

} // namespace xary::core