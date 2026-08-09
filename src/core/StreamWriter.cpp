#include "../../include/xary/core/StreamWriter.hpp"

namespace xary::core {

StreamWriter::StreamWriter(const std::string& filepath)
    : m_filepath(filepath) {
    m_fileStream = std::make_unique<std::ofstream>(m_filepath, std::ios::binary | std::ios::trunc);
}

StreamWriter::~StreamWriter() {
    if (m_fileStream && m_fileStream->is_open()) {
        m_fileStream->close();
    }
}

bool StreamWriter::isOpen() const {
    return m_fileStream && m_fileStream->is_open() && m_fileStream->good();
}

bool StreamWriter::writeChunk(const std::vector<std::uint8_t>& buffer) {
    return writeChunk(buffer.data(), buffer.size());
}

bool StreamWriter::writeChunk(const std::uint8_t* data, std::size_t size) {
    if (!isOpen() || !data || size == 0) {
        return false;
    }
    m_fileStream->write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    return m_fileStream->good();
}

}