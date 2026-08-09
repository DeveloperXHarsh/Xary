#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <cstddef>
#include <memory>

namespace xary::core {

class StreamWriter {
public:
    explicit StreamWriter(const std::string& filepath);
    ~StreamWriter();

    bool isOpen() const;
    bool writeChunk(const std::vector<std::uint8_t>& buffer);
    bool writeChunk(const std::uint8_t* data, std::size_t size);

private:
    std::string m_filepath;
    std::unique_ptr<std::ofstream> m_fileStream;
};

}