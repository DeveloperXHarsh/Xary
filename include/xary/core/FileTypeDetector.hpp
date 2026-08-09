#pragma once

#include "../core/BufferView.hpp"
#include <string>

namespace xary::core {

struct FileTypeInfo {
    std::string expectedExtension;
    std::string mimeType;
    std::string description;
    bool isKnown{false};
};

class FileTypeDetector {
public:
    static FileTypeInfo detect(BufferView headerView);
};

}