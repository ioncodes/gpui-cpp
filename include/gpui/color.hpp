#pragma once

#include <cstdint>

namespace gpui {

constexpr std::uint32_t rgb(std::uint32_t rgb) noexcept {
    return (rgb << 8) | 0xFFu;
}

constexpr std::uint32_t rgba(std::uint32_t rgba) noexcept { return rgba; }

constexpr std::uint32_t rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b,
                             std::uint8_t a = 0xFF) noexcept {
    return (std::uint32_t(r) << 24) | (std::uint32_t(g) << 16) |
           (std::uint32_t(b) << 8) | std::uint32_t(a);
}

constexpr std::uint32_t white = 0xFFFFFFFFu;
constexpr std::uint32_t black = 0x000000FFu;
constexpr std::uint32_t transparent = 0x00000000u;

}
