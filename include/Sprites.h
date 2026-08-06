#pragma once

#include <cstdint>

class Sprites {
public:
    static void drawExternalMask(std::int16_t x, std::int16_t y,
                                 const std::uint8_t* bitmap,
                                 const std::uint8_t* mask,
                                 std::uint8_t frame = 0,
                                 std::uint8_t mask_frame = 0) noexcept;
    static void drawOverwrite(std::int16_t x, std::int16_t y,
                              const std::uint8_t* bitmap,
                              std::uint8_t frame = 0) noexcept;
    static void drawPlusMask(std::int16_t x, std::int16_t y,
                             const std::uint8_t* bitmap,
                             std::uint8_t frame = 0) noexcept;
    static void drawSelfMasked(std::int16_t x, std::int16_t y,
                               const std::uint8_t* bitmap,
                               std::uint8_t frame = 0) noexcept;
    static void drawErase(std::int16_t x, std::int16_t y,
                          const std::uint8_t* bitmap,
                          std::uint8_t frame = 0) noexcept;
};

using SpritesB = Sprites;
