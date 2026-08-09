#pragma once

#include "../core/BufferView.hpp"
#include <string_view>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : File Type Inspection (FileTypeDetector.hpp)
 * Description : Static magic byte analyzer identifying binary stream format,
 *               MIME type, and suggested file extension without heap allocations.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

struct FileTypeInfo {
    std::string_view expectedExtension{".bin"};
    std::string_view mimeType{"application/octet-stream"};
    std::string_view description{"Generic Binary Data"};
    bool isKnown{false};
};

class FileTypeDetector {
public:
    [[nodiscard]] static FileTypeInfo detect(BufferView headerView) noexcept;
};

} // namespace xary::core