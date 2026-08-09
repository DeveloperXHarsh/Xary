#include "../../include/xary/core/Stream.hpp"

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Core Stream Engine (Stream.cpp)
 * Description : Implementation of chunked binary file stream reader using
 *               std::filesystem OS metadata queries and stack stream buffers.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

Stream::Stream(const std::filesystem::path& filepath, std::size_t chunkSize)
    : m_filepath(filepath), m_chunkSize(chunkSize), m_fileStream(filepath, std::ios::binary) {

    if (m_fileStream.is_open()) {
        std::error_code ec;
        const auto sz = std::filesystem::file_size(filepath, ec);
        if (!ec) {
            m_fileSize = sz;
        } else {
            // Fallback size calculation if OS filesystem metadata query fails
            m_fileStream.seekg(0, std::ios::end);
            m_fileSize = static_cast<std::uint64_t>(m_fileStream.tellg());
            m_fileStream.seekg(0, std::ios::beg);
        }
    }
}

bool Stream::isOpen() const noexcept {
    return m_fileStream.is_open() && m_fileStream.good();
}

std::uint64_t Stream::getFileSize() const noexcept {
    return m_fileSize;
}

std::size_t Stream::getChunkSize() const noexcept {
    return m_chunkSize;
}

std::size_t Stream::readChunk(std::vector<std::uint8_t>& buffer) {
    if (!isOpen() || m_fileStream.eof()) {
        buffer.clear();
        return 0;
    }

    if (buffer.size() < m_chunkSize) {
        buffer.resize(m_chunkSize);
    }

    m_fileStream.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(m_chunkSize));
    const std::streamsize bytesRead = m_fileStream.gcount();

    if (bytesRead <= 0) {
        buffer.clear();
        return 0;
    }

    buffer.resize(static_cast<std::size_t>(bytesRead));
    return static_cast<std::size_t>(bytesRead);
}

void Stream::reset() noexcept {
    if (m_fileStream.is_open()) {
        m_fileStream.clear();
        m_fileStream.seekg(0, std::ios::beg);
    }
}

} // namespace xary::core