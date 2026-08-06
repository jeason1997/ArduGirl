#pragma once

#include "ardugirl/constants.hpp"

#include <array>
#include <cstdint>

namespace ardugirl {

class Framebuffer {
public:
    using Storage = std::array<std::uint8_t, kFramebufferSize>;

    void clear(bool white = false) noexcept;
    void set_pixel(std::int16_t x, std::int16_t y, bool white = true) noexcept;
    bool pixel(std::int16_t x, std::int16_t y) const noexcept;
    void draw_line(std::int16_t x0, std::int16_t y0,
                   std::int16_t x1, std::int16_t y1,
                   bool white = true) noexcept;
    void draw_rect(std::int16_t x, std::int16_t y,
                   std::int16_t width, std::int16_t height,
                   bool white = true) noexcept;

    const Storage& data() const noexcept { return pixels_; }
    Storage& data() noexcept { return pixels_; }

private:
    Storage pixels_{};
};

Framebuffer& framebuffer() noexcept;

} // 命名空间 ardugirl
