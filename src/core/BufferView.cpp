#include "../../include/xary/core/BufferView.hpp"
#include <stdexcept>
#include <algorithm>

namespace xary::core {

BufferView::BufferView(const std::uint8_t* data, std::size_t size)
    : m_view(data, size) {}

BufferView::BufferView(std::span<const std::uint8_t> view)
    : m_view(view) {}

BufferView::BufferView(const std::vector<std::uint8_t>& buffer)
    : m_view(buffer.data(), buffer.size()) {}

const std::uint8_t* BufferView::data() const noexcept {
    return m_view.data();
}

std::size_t BufferView::size() const noexcept {
    return m_view.size();
}

bool BufferView::empty() const noexcept {
    return m_view.empty();
}

std::span<const std::uint8_t> BufferView::span() const noexcept {
    return m_view;
}

BufferView BufferView::subview(std::size_t offset, std::size_t count) const {
    if (offset > m_view.size()) {
        throw std::out_of_range("BufferView::subview offset out of bounds");
    }
    auto sub = m_view.subspan(offset, (count == std::string_view::npos) ? (m_view.size() - offset) : std::min(count, m_view.size() - offset));
    return BufferView(sub);
}

std::uint8_t BufferView::operator[](std::size_t index) const {
    return m_view[index];
}

std::optional<std::uint8_t> BufferView::at(std::size_t index) const noexcept {
    if (index >= m_view.size()) {
        return std::nullopt;
    }
    return m_view[index];
}

std::size_t BufferView::find(std::uint8_t byte, std::size_t startOffset) const noexcept {
    if (startOffset >= m_view.size()) return std::string_view::npos;
    
    auto it = std::find(m_view.begin() + startOffset, m_view.end(), byte);
    if (it != m_view.end()) {
        return static_cast<std::size_t>(std::distance(m_view.begin(), it));
    }
    return std::string_view::npos;
}

std::size_t BufferView::findSequence(std::span<const std::uint8_t> pattern, std::size_t startOffset) const noexcept {
    if (pattern.empty() || startOffset + pattern.size() > m_view.size()) {
        return std::string_view::npos;
    }

    auto it = std::search(m_view.begin() + startOffset, m_view.end(), pattern.begin(), pattern.end());
    if (it != m_view.end()) {
        return static_cast<std::size_t>(std::distance(m_view.begin(), it));
    }
    return std::string_view::npos;
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

}