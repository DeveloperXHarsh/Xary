#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <cstddef>
#include <memory>

namespace xary::core {

class Stream {
public:
    explicit Stream(const std::string& filepath, std::size_t chunkSize = 64 * 1024);
    ~Stream();

    bool isOpen() const;
    std::uint64_t getFileSize() const;
    
    std::size_t readChunk(std::vector<std::uint8_t>& buffer);
    void reset();

private:
    std::string m_filepath;
    std::size_t m_chunkSize;
    std::uint64_t m_fileSize{0};
    std::unique_ptr<std::ifstream> m_fileStream;
};

} // namespace xary::core