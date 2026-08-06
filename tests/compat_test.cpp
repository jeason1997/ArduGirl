#include "Arduboy2.h"

#include "ardugirl/framebuffer.hpp"
#include "ardugirl/runtime.hpp"

#include <cassert>
#include <cstring>
#include <cstdint>

namespace {

std::uint32_t clock_us = 1234567;
std::uint16_t wave_rate = 0;
std::uint16_t wave_count = 0;
std::uint8_t storage[1024]{};

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
void set_tone(std::uint16_t, std::uint8_t) noexcept {}
void stop_tone(std::uint8_t) noexcept {}
void play_wave(std::uint16_t rate, const std::uint8_t*, std::uint16_t count) noexcept {
    wave_rate = rate;
    wave_count = count;
}
void stop_wave() noexcept { wave_count = 0; }
void set_synth(const SynthVoice*, std::uint8_t) noexcept {}
bool storage_read(std::uint16_t offset, void* destination, std::uint16_t size) noexcept {
    std::memcpy(destination, storage + offset, size);
    return true;
}
bool storage_write(std::uint16_t offset, const void* source, std::uint16_t size) noexcept {
    std::memcpy(storage + offset, source, size);
    return true;
}

} // 命名空间 ardugirl::platform

int main() {
    static_assert(std::is_same_v<Arduboy2Base, Arduboy2>);
    assert(Arduboy2Base::sBuffer == ardugirl::framebuffer().data().data());
    struct SaveData {
        std::uint8_t health;
        std::uint16_t score;
    };
    static_assert(std::is_trivially_copyable_v<SaveData>);
    const SaveData saved{3, 1200};
    EEPROM.put(64, saved);
    SaveData loaded{};
    EEPROM.get(64, loaded);
    assert(loaded.health == saved.health && loaded.score == saved.score);
    Arduboy2 arduboy;
    Arduboy2Audio::off();
    assert(!arduboy.audio.enabled());
    arduboy.audio.toggle();
    assert(Arduboy2Audio::enabled());
    arduboy.initAudio(1);
    constexpr byte wave[] = {0, 128, 255};
    arduboy.playWave(8000, wave, 3);
    assert(wave_rate == 8000 && wave_count == 3);
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
