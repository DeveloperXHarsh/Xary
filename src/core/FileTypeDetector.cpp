#include "../../include/xary/core/FileTypeDetector.hpp"
#include <array>
#include <span>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : File Type Inspection (FileTypeDetector.cpp)
 * Description : Optimized pattern matching engine implementing magic signature
 *               inspection tables for instant zero-allocation header identification.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

namespace {

struct MagicSignature {
    std::size_t offset;
    std::span<const std::uint8_t> pattern;
    FileTypeInfo info;
};

// Binary magic byte pattern dictionary
constexpr std::array<std::uint8_t, 8> MAGIC_XARY = {0x8F, 0x1E, 0xAA, 0x4D, 0x9C, 0x05, 0xF3, 0x72};
constexpr std::array<std::uint8_t, 8> MAGIC_PNG  = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
constexpr std::array<std::uint8_t, 3> MAGIC_JPEG = {0xFF, 0xD8, 0xFF};
constexpr std::array<std::uint8_t, 4> MAGIC_PDF  = {0x25, 0x50, 0x44, 0x46};
constexpr std::array<std::uint8_t, 4> MAGIC_ZIP  = {0x50, 0x4B, 0x03, 0x04};
constexpr std::array<std::uint8_t, 2> MAGIC_EXE  = {0x4D, 0x5A};
constexpr std::array<std::uint8_t, 4> MAGIC_ELF  = {0x7F, 0x45, 0x4C, 0x46};
constexpr std::array<std::uint8_t, 4> MAGIC_GIF  = {0x47, 0x49, 0x46, 0x38};
constexpr std::array<std::uint8_t, 2> MAGIC_GZIP = {0x1F, 0x8B};
constexpr std::array<std::uint8_t, 4> MAGIC_MP4  = {'f', 't', 'y', 'p'};

const MagicSignature KNOWN_SIGNATURES[] = {
    {0, MAGIC_XARY, {".xary", "application/x-xary-encrypted", "Xary Encrypted Binary Stream", true}},
    {0, MAGIC_PNG,  {".png",  "image/png",                   "PNG Image",                   true}},
    {0, MAGIC_JPEG, {".jpg",  "image/jpeg",                  "JPEG Image",                  true}},
    {0, MAGIC_PDF,  {".pdf",  "application/pdf",             "PDF Document",                true}},
    {0, MAGIC_ZIP,  {".zip",  "application/zip",             "ZIP Archive / MS Office Document", true}},
    {0, MAGIC_EXE,  {".exe",  "application/x-msdownload",    "Windows Executable / DLL",    true}},
    {0, MAGIC_ELF,  {".elf",  "application/x-executable",    "Linux ELF Executable",        true}},
    {0, MAGIC_GIF,  {".gif",  "image/gif",                   "GIF Animated Image",          true}},
    {0, MAGIC_GZIP, {".gz",   "application/gzip",            "GZIP Compressed Archive",     true}},
    {4, MAGIC_MP4,  {".mp4",  "video/mp4",                   "MP4 Video Container",          true}}
};

} // namespace

FileTypeInfo FileTypeDetector::detect(BufferView headerView) noexcept {
    if (headerView.size() < 2) {
        return {".bin", "application/octet-stream", "Unknown / File too small", false};
    }

    // Direct table inspection without heap allocations
    for (const auto& sig : KNOWN_SIGNATURES) {
        if (headerView.size() >= sig.offset + sig.pattern.size()) {
            bool matches = true;
            for (std::size_t i = 0; i < sig.pattern.size(); ++i) {
                if (headerView[sig.offset + i] != sig.pattern[i]) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                return sig.info;
            }
        }
    }

    return {".bin", "application/octet-stream", "Generic Binary Data", false};
}

} // namespace xary::core