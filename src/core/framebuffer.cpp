#include "ardugirl/framebuffer.hpp"

#include <algorithm>
#include <cstdlib>

namespace ardugirl {

namespace {

constexpr bool is_visible(std::int16_t x, std::int16_t y) noexcept {
    return x >= 0 && x < kScreenWidth && y >= 0 && y < kScreenHeight;
}

} // 匿名命名空间

void Framebuffer::clear(bool white) noexcept {
    pixels_.fill(white ? 0xFFu : 0x00u);
}

void Framebuffer::set_pixel(std::int16_t x, std::int16_t y, bool white) noexcept {
    if (!is_visible(x, y)) {
        return;
    }

    // Arduboy 的每个字节保存垂直方向的八个像素。严格保持这个布局后，
    // 未来的 SSD1306 后端可以直接发送显存，不需要额外转换或复制。
    const auto index = static_cast<std::size_t>(x) +
                       static_cast<std::size_t>(y / 8) * kScreenWidth;
    const auto mask = static_cast<std::uint8_t>(1u << (y & 7));
    if (white) {
        pixels_[index] |= mask;
    } else {
        pixels_[index] &= static_cast<std::uint8_t>(~mask);
    }
}

bool Framebuffer::pixel(std::int16_t x, std::int16_t y) const noexcept {
    if (!is_visible(x, y)) {
        return false;
    }

    const auto index = static_cast<std::size_t>(x) +
                       static_cast<std::size_t>(y / 8) * kScreenWidth;
    const auto mask = static_cast<std::uint8_t>(1u << (y & 7));
    return (pixels_[index] & mask) != 0;
}

void Framebuffer::draw_line(std::int16_t x0, std::int16_t y0,
                            std::int16_t x1, std::int16_t y1,
                            bool white) noexcept {
    // Bresenham 算法只使用整数运算，因此在小型 MCU 和桌面平台上的行为一致。
    const auto dx = static_cast<std::int16_t>(std::abs(x1 - x0));
    const auto sx = static_cast<std::int16_t>(x0 < x1 ? 1 : -1);
    const auto dy = static_cast<std::int16_t>(-std::abs(y1 - y0));
    const auto sy = static_cast<std::int16_t>(y0 < y1 ? 1 : -1);
    auto error = static_cast<std::int16_t>(dx + dy);

    while (true) {
        set_pixel(x0, y0, white);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        const auto twice_error = static_cast<std::int16_t>(error * 2);
        if (twice_error >= dy) {
            error = static_cast<std::int16_t>(error + dy);
            x0 = static_cast<std::int16_t>(x0 + sx);
        }
        if (twice_error <= dx) {
            error = static_cast<std::int16_t>(error + dx);
            y0 = static_cast<std::int16_t>(y0 + sy);
        }
    }
}

void Framebuffer::draw_rect(std::int16_t x, std::int16_t y,
                            std::int16_t width, std::int16_t height,
                            bool white) noexcept {
    if (width <= 0 || height <= 0) {
        return;
    }
    const auto right = static_cast<std::int16_t>(x + width - 1);
    const auto bottom = static_cast<std::int16_t>(y + height - 1);
    draw_line(x, y, right, y, white);
    draw_line(right, y, right, bottom, white);
    draw_line(right, bottom, x, bottom, white);
    draw_line(x, bottom, x, y, white);
}

Framebuffer& framebuffer() noexcept {
    static Framebuffer instance;
    return instance;
}

} // 命名空间 ardugirl
