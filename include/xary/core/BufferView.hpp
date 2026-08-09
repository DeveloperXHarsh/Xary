#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

/*
 * ============================================================================
 * Project     : Xary Engine
 * Module      : Core Buffer View Engine (BufferView.hpp)
 * Description : Zero-copy memory view container wrapping contiguous byte spans,
 *               offering safe subview slicing, pattern searching, range iteration,
 *               and endian-safe multi-byte integer parsing.
 * Author      : Piyush Rajput aka Harsh (DeveloperXHarsh)
 * Copyright   : (c) 2026 Piyush Rajput. All rights reserved.
 * ============================================================================
 */

namespace xary::core {

class BufferView {
public:
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    constexpr BufferView() noexcept = default;
    constexpr BufferView(const std::uint8_t* data, std::size_t size) noexcept : m_view(data, size) {}
    constexpr BufferView(std::span<const std::uint8_t> view) noexcept : m_view(view) {}
    BufferView(const std::vector<std::uint8_t>& buffer) noexcept : m_view(buffer.data(), buffer.size()) {}

    [[nodiscard]] constexpr const std::uint8_t* data() const noexcept { return m_view.data(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return m_view.size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return m_view.empty(); }
    [[nodiscard]] constexpr std::span<const std::uint8_t> span() const noexcept { return m_view; }

    [[nodiscard]] constexpr auto begin() const noexcept { return m_view.begin(); }
    [[nodiscard]] constexpr auto end() const noexcept { return m_view.end(); }
    [[nodiscard]] constexpr auto cbegin() const noexcept { return m_view.begin(); }
    [[nodiscard]] constexpr auto cend() const noexcept { return m_view.end(); }

    [[nodiscard]] BufferView subview(std::size_t offset, std::size_t count = npos) const;

    [[nodiscard]] std::uint8_t operator[](std::size_t index) const noexcept;
    [[nodiscard]] std::optional<std::uint8_t> at(std::size_t index) const noexcept;

    [[nodiscard]] std::size_t find(std::uint8_t byte, std::size_t startOffset = 0) const noexcept;
    [[nodiscard]] std::size_t findSequence(std::span<const std::uint8_t> pattern, std::size_t startOffset = 0) const noexcept;

    [[nodiscard]] std::uint16_t readU16BE(std::size_t offset) const;
    [[nodiscard]] std::uint32_t readU32BE(std::size_t offset) const;
    [[nodiscard]] std::uint64_t readU64BE(std::size_t offset) const;

    [[nodiscard]] std::uint16_t readU16LE(std::size_t offset) const;
    [[nodiscard]] std::uint32_t readU32LE(std::size_t offset) const;
    [[nodiscard]] std::uint64_t readU64LE(std::size_t offset) const;

private:
    std::span<const std::uint8_t> m_view{};
};

} // namespace xary::core