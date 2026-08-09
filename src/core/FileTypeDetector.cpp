#include "../../include/xary/core/FileTypeDetector.hpp"
#include <array>

namespace xary::core {

// Xary Proprietary Header
constexpr std::array<uint8_t, 8> XARY_MAGIC = {0x8F, 0x1E, 0xAA, 0x4D, 0x9C, 0x05, 0xF3, 0x72};

FileTypeInfo FileTypeDetector::detect(BufferView view) {
    if (view.size() < 4) {
        return {".bin", "application/octet-stream", "Unknown / File too small", false};
    }

    // 1. Check Xary Encrypted Binary
    if (view.size() >= 8) {
        bool isXary = true;
        for (std::size_t i = 0; i < 8; ++i) {
            if (view[i] != XARY_MAGIC[i]) {
                isXary = false;
                break;
            }
        }
        if (isXary) {
            return {".xary", "application/x-xary-encrypted", "Xary Encrypted Binary Stream", true};
        }
    }

    // 2. Standard Binary Magic Signatures
    
    // PNG: 89 50 4E 47 0D 0A 1A 0A
    if (view.size() >= 8 && view[0] == 0x89 && view[1] == 0x50 && view[2] == 0x4E && view[3] == 0x47) {
        return {".png", "image/png", "PNG Image", true};
    }

    // JPEG: FF D8 FF
    if (view[0] == 0xFF && view[1] == 0xD8 && view[2] == 0xFF) {
        return {".jpg", "image/jpeg", "JPEG Image", true};
    }

    // PDF: %PDF (25 50 44 46)
    if (view[0] == 0x25 && view[1] == 0x50 && view[2] == 0x44 && view[3] == 0x46) {
        return {".pdf", "application/pdf", "PDF Document", true};
    }

    // ZIP / Office Docs (DOCX, XLSX, PPTX): PK.. (50 4B 03 04)
    if (view[0] == 0x50 && view[1] == 0x4B && view[2] == 0x03 && view[3] == 0x04) {
        return {".zip", "application/zip", "ZIP Archive / MS Office Document", true};
    }

    // Windows Executable / DLL: MZ (4D 5A)
    if (view[0] == 0x4D && view[1] == 0x5A) {
        return {".exe", "application/x-msdownload", "Windows Executable / DLL", true};
    }

    // Linux Executable: ELF (7F 45 4C 46)
    if (view[0] == 0x7F && view[1] == 0x45 && view[2] == 0x4C && view[3] == 0x46) {
        return {".elf", "application/x-executable", "Linux ELF Executable", true};
    }

    // GIF Image: GIF8 (47 49 46 38)
    if (view[0] == 0x47 && view[1] == 0x49 && view[2] == 0x46 && view[3] == 0x38) {
        return {".gif", "image/gif", "GIF Animated Image", true};
    }

    // GZIP Archive: 1F 8B
    if (view[0] == 0x1F && view[1] == 0x8B) {
        return {".gz", "application/gzip", "GZIP Compressed Archive", true};
    }

    // MP4 Video: ftyp box at offset 4
    if (view.size() >= 12 && view[4] == 'f' && view[5] == 't' && view[6] == 'y' && view[7] == 'p') {
        return {".mp4", "video/mp4", "MP4 Video Container", true};
    }

    return {".bin", "application/octet-stream", "Generic Binary Data", false};
}

}