#include "Arduboy2.h"

#include "ardugirl/framebuffer.hpp"
#include "ardugirl/runtime.hpp"

#include <array>

namespace {

using Glyph = std::array<std::uint8_t, 5>;

Glyph glyph_for(char character) noexcept {
    // 第一阶段只实现官方 HelloWorld 示例实际使用的字符。未知字符显示为空心方框，
    // 这样缺失字形会直接出现在画面中，而不是被静默吞掉。
    switch (character) {
    case ' ': return {0x00, 0x00, 0x00, 0x00, 0x00};
    case '!': return {0x00, 0x00, 0x5F, 0x00, 0x00};
    case ',': return {0x00, 0x50, 0x30, 0x00, 0x00};
    case 'H': return {0x7F, 0x08, 0x08, 0x08, 0x7F};
    case 'd': return {0x38, 0x44, 0x44, 0x48, 0x7F};
    case 'e': return {0x38, 0x54, 0x54, 0x54, 0x18};
    case 'l': return {0x00, 0x41, 0x7F, 0x40, 0x00};
    case 'o': return {0x38, 0x44, 0x44, 0x44, 0x38};
    case 'r': return {0x7C, 0x08, 0x04, 0x04, 0x08};
    case 'w': return {0x3C, 0x40, 0x30, 0x40, 0x3C};
    default:  return {0x7F, 0x41, 0x41, 0x41, 0x7F};
    }
}

} // 匿名命名空间

void Arduboy2::begin() noexcept {
    clear();
    display();
}

void Arduboy2::setFrameRate(std::uint8_t rate) noexcept {
    frame_rate_ = rate;
}

bool Arduboy2::nextFrame() noexcept {
    return ardugirl::next_frame(frame_rate_);
}

void Arduboy2::clear() noexcept {
    ardugirl::framebuffer().clear();
    cursor_x_ = 0;
    cursor_y_ = 0;
}

void Arduboy2::display() noexcept {
    ardugirl::display();
}

void Arduboy2::setCursor(std::int16_t x, std::int16_t y) noexcept {
    cursor_x_ = x;
    cursor_y_ = y;
}

std::size_t Arduboy2::print(const char* text) noexcept {
    if (text == nullptr) {
        return 0;
    }

    std::size_t written = 0;
    while (*text != '\0') {
        drawCharacter(*text++);
        ++written;
    }
    return written;
}

bool Arduboy2::pressed(std::uint8_t requested) const noexcept {
    return (ardugirl::buttons() & requested) == requested;
}

void Arduboy2::drawCharacter(char character) noexcept {
    const auto glyph = glyph_for(character);
    auto& screen = ardugirl::framebuffer();

    for (std::int16_t column = 0; column < 5; ++column) {
        for (std::int16_t row = 0; row < 8; ++row) {
            const auto mask = static_cast<std::uint8_t>(1u << row);
            if ((glyph[static_cast<std::size_t>(column)] & mask) != 0) {
                screen.set_pixel(static_cast<std::int16_t>(cursor_x_ + column),
                                 static_cast<std::int16_t>(cursor_y_ + row));
            }
        }
    }
    cursor_x_ = static_cast<std::int16_t>(cursor_x_ + 6);
}

