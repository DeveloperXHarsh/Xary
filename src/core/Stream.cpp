#include "../../include/xary/core/Stream.hpp"
#include <iostream>

namespace xary::core {

Stream::Stream(const std::string& filepath, std::size_t chunkSize)
    : m_filepath(filepath), m_chunkSize(chunkSize), m_fileSize(0) {
    
    m_fileStream = std::make_unique<std::ifstream>(m_filepath, std::ios::binary);
    if (m_fileStream && m_fileStream->is_open()) {
        m_fileStream->seekg(0, std::ios::end);
        auto pos = m_fileStream->tellg();
        if (pos > 0) {
            m_fileSize = static_cast<std::uint64_t>(pos);
        }
        m_fileStream->seekg(0, std::ios::beg);
    }
}

Stream::~Stream() {
    if (m_fileStream && m_fileStream->is_open()) {
        m_fileStream->close();
    }
}

bool Stream::isOpen() const {
    return m_fileStream && m_fileStream->is_open() && m_fileStream->good();
}

std::uint64_t Stream::getFileSize() const {
    return m_fileSize;
}

std::size_t Stream::readChunk(std::vector<std::uint8_t>& buffer) {
    if (!isOpen() || m_fileStream->eof()) {
        return 0;
    }

    buffer.resize(m_chunkSize);
    m_fileStream->read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(m_chunkSize));
    
    std::streamsize bytesRead = m_fileStream->gcount();
    if (bytesRead <= 0) {
        buffer.clear();
        return 0;
    }

    buffer.resize(static_cast<std::size_t>(bytesRead));
    return static_cast<std::size_t>(bytesRead);
}

void Stream::reset() {
    if (isOpen()) {
        m_fileStream->clear();
        m_fileStream->seekg(0, std::ios::beg);
    }
}

} // namespace xary::core
