#include "Arduboy2.h"

#include "ardugirl/framebuffer.hpp"
#include "ardugirl/runtime.hpp"

#include <cassert>
#include <cstdint>

namespace {

std::uint32_t clock_us = 1234567;

} // 匿名命名空间

namespace ardugirl {

bool next_frame(std::uint8_t) noexcept { return true; }
void display() noexcept {}
std::uint8_t buttons() noexcept { return 0; }

} // 命名空间 ardugirl

namespace ardugirl::platform {

std::uint32_t millis() noexcept { return clock_us / 1000u; }
std::uint32_t micros() noexcept { return clock_us; }
void sleep_ms(std::uint32_t duration) noexcept { clock_us += duration * 1000u; }
void set_tone(std::uint16_t) noexcept {}
void stop_tone() noexcept {}
bool storage_read(std::uint16_t, void*, std::uint16_t) noexcept { return false; }
bool storage_write(std::uint16_t, const void*, std::uint16_t) noexcept { return true; }

} // 命名空间 ardugirl::platform

int main() {
    Arduboy2 arduboy;
    assert(micros() == 1234567u);
    delay(2);
    assert(micros() == 1236567u);
    assert(map(5, 0, 10, 0, 100) == 50);

    arduboy.clear();
    arduboy.drawTriangle(1, 1, 8, 1, 4, 6);
    assert(ardugirl::framebuffer().pixel(1, 1));
    arduboy.fillTriangle(20, 1, 28, 1, 24, 8);
    assert(ardugirl::framebuffer().pixel(24, 4));
    arduboy.drawRoundRect(32, 1, 12, 10, 3);
    arduboy.fillRoundRect(48, 1, 12, 10, 3);
    assert(ardugirl::framebuffer().pixel(53, 5));

    constexpr std::uint8_t xy_bitmap[] = {0x80};
    arduboy.drawSlowXYBitmap(64, 1, xy_bitmap, 1, 1);
    assert(ardugirl::framebuffer().pixel(64, 1));
    // 压缩格式按低位优先存放：1×1、首段为白色、长度为 1。
    constexpr std::uint8_t compressed_bitmap[] = {0x00, 0x00, 0x03};
    arduboy.drawCompressed(66, 1, compressed_bitmap);
    assert(ardugirl::framebuffer().pixel(66, 1));

    constexpr std::uint8_t sprite[] = {2, 8, 0x01, 0x02};
    constexpr std::uint8_t mask[] = {0x01, 0x03};
    Sprites::drawExternalMask(0, 16, sprite, mask);
    assert(ardugirl::framebuffer().pixel(0, 16));
    Sprites::drawSelfMasked(4, 16, sprite);
    assert(ardugirl::framebuffer().pixel(5, 17));
    Sprites::drawErase(4, 16, sprite);
    assert(!ardugirl::framebuffer().pixel(5, 17));

    arduboy.setTextSize(2);
    arduboy.setCursor(0, 32);
    arduboy.print("A\nB");
    assert(arduboy.getCursorX() == 12);
    assert(arduboy.getCursorY() == 48);
    arduboy.display(true);
    assert(!ardugirl::framebuffer().pixel(66, 1));
    return 0;
}
