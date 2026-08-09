#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include <string_view>
#include <optional>
#include <vector>

namespace xary::core {

class BufferView {
public:
    // Constructors
    BufferView() = default;
    BufferView(const std::uint8_t* data, std::size_t size);
    BufferView(std::span<const std::uint8_t> view);
    BufferView(const std::vector<std::uint8_t>& buffer);

    // Memory View Accessors
    [[nodiscard]] const std::uint8_t* data() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::span<const std::uint8_t> span() const noexcept;

    // Zero-Copy Slicing
    [[nodiscard]] BufferView subview(std::size_t offset, std::size_t count = std::string_view::npos) const;

    // Byte Inspection
    [[nodiscard]] std::uint8_t operator[](std::size_t index) const;
    [[nodiscard]] std::optional<std::uint8_t> at(std::size_t index) const noexcept;

    // Fast Pattern Search
    [[nodiscard]] std::size_t find(std::uint8_t byte, std::size_t startOffset = 0) const noexcept;
    [[nodiscard]] std::size_t findSequence(std::span<const std::uint8_t> pattern, std::size_t startOffset = 0) const noexcept;

    // Endian-Safe Multi-Byte Reading
    [[nodiscard]] std::uint16_t readU16BE(std::size_t offset) const;
    [[nodiscard]] std::uint32_t readU32BE(std::size_t offset) const;
    [[nodiscard]] std::uint64_t readU64BE(std::size_t offset) const;

private:
    std::span<const std::uint8_t> m_view{};
};

}