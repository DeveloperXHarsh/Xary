#include "../../include/xary/core/BufferView.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Core Buffer View Engine (BufferView.cpp)
 * Description : Implementation of zero-copy span inspector, fast memory-search
 *               primitives using std::memchr, and bounded multi-byte reads.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

BufferView BufferView::subview(std::size_t offset, std::size_t count) const {
    if (offset > m_view.size()) {
        throw std::out_of_range("BufferView::subview offset out of bounds");
    }
    const std::size_t remaining = m_view.size() - offset;
    const std::size_t sliceSize = (count == npos) ? remaining : std::min(count, remaining);
    return BufferView(m_view.subspan(offset, sliceSize));
}

std::uint8_t BufferView::operator[](std::size_t index) const noexcept {
    return m_view[index];
}

std::optional<std::uint8_t> BufferView::at(std::size_t index) const noexcept {
    if (index >= m_view.size()) {
        return std::nullopt;
    }
    return m_view[index];
}

std::size_t BufferView::find(std::uint8_t byte, std::size_t startOffset) const noexcept {
    if (startOffset >= m_view.size()) {
        return npos;
    }

    const void* ptr = std::memchr(m_view.data() + startOffset, byte, m_view.size() - startOffset);
    if (ptr != nullptr) {
        return static_cast<std::size_t>(static_cast<const std::uint8_t*>(ptr) - m_view.data());
    }
    return npos;
}

std::size_t BufferView::findSequence(std::span<const std::uint8_t> pattern, std::size_t startOffset) const noexcept {
    if (pattern.empty() || startOffset + pattern.size() > m_view.size()) {
        return npos;
    }

    auto it = std::search(m_view.begin() + startOffset, m_view.end(), pattern.begin(), pattern.end());
    if (it != m_view.end()) {
        return static_cast<std::size_t>(std::distance(m_view.begin(), it));
    }
    return npos;
}

std::uint16_t BufferView::readU16BE(std::size_t offset) const {
    if (offset + 2 > m_view.size()) throw std::out_of_range("BufferView::readU16BE out of bounds");
    return static_cast<std::uint16_t>((m_view[offset] << 8) | m_view[offset + 1]);
}

std::uint32_t BufferView::readU32BE(std::size_t offset) const {
    if (offset + 4 > m_view.size()) throw std::out_of_range("BufferView::readU32BE out of bounds");
    return (static_cast<std::uint32_t>(m_view[offset]) << 24) |
           (static_cast<std::uint32_t>(m_view[offset + 1]) << 16) |
           (static_cast<std::uint32_t>(m_view[offset + 2]) << 8) |
           (static_cast<std::uint32_t>(m_view[offset + 3]));
}

std::uint64_t BufferView::readU64BE(std::size_t offset) const {
    if (offset + 8 > m_view.size()) throw std::out_of_range("BufferView::readU64BE out of bounds");
    return (static_cast<std::uint64_t>(m_view[offset]) << 56) |
           (static_cast<std::uint64_t>(m_view[offset + 1]) << 48) |
           (static_cast<std::uint64_t>(m_view[offset + 2]) << 40) |
           (static_cast<std::uint64_t>(m_view[offset + 3]) << 32) |
           (static_cast<std::uint64_t>(m_view[offset + 4]) << 24) |
           (static_cast<std::uint64_t>(m_view[offset + 5]) << 16) |
           (static_cast<std::uint64_t>(m_view[offset + 6]) << 8) |
           (static_cast<std::uint64_t>(m_view[offset + 7]));
}

std::uint16_t BufferView::readU16LE(std::size_t offset) const {
    if (offset + 2 > m_view.size()) throw std::out_of_range("BufferView::readU16LE out of bounds");
    return static_cast<std::uint16_t>(m_view[offset] | (m_view[offset + 1] << 8));
}

std::uint32_t BufferView::readU32LE(std::size_t offset) const {
    if (offset + 4 > m_view.size()) throw std::out_of_range("BufferView::readU32LE out of bounds");
    return (static_cast<std::uint32_t>(m_view[offset])) |
           (static_cast<std::uint32_t>(m_view[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(m_view[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(m_view[offset + 3]) << 24);
}

std::uint64_t BufferView::readU64LE(std::size_t offset) const {
    if (offset + 8 > m_view.size()) throw std::out_of_range("BufferView::readU64LE out of bounds");
    return (static_cast<std::uint64_t>(m_view[offset])) |
           (static_cast<std::uint64_t>(m_view[offset + 1]) << 8) |
           (static_cast<std::uint64_t>(m_view[offset + 2]) << 16) |
           (static_cast<std::uint64_t>(m_view[offset + 3]) << 24) |
           (static_cast<std::uint64_t>(m_view[offset + 4]) << 32) |
           (static_cast<std::uint64_t>(m_view[offset + 5]) << 40) |
           (static_cast<std::uint64_t>(m_view[offset + 6]) << 48) |
           (static_cast<std::uint64_t>(m_view[offset + 7]) << 56);
}

} // namespace xary::core