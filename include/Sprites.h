#pragma once

#include <cstdint>

class Sprites {
public:
    static void drawOverwrite(std::int16_t x, std::int16_t y,
                              const std::uint8_t* bitmap,
                              std::uint8_t frame = 0) noexcept;
    static void drawPlusMask(std::int16_t x, std::int16_t y,
                             const std::uint8_t* bitmap,
                             std::uint8_t frame = 0) noexcept;
};

