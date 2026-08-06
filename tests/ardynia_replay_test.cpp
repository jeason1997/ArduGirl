#include "ArduboyPlaytune.h"
#include "ardugirl/framebuffer.hpp"
#include "ardugirl/runtime.hpp"
#include "ardugirl/platform.hpp"

#include <cstdint>
#include <cstring>

namespace {

std::uint8_t replay_buttons = 0;
std::uint32_t replay_frames = 0;
std::uint8_t replay_storage[1024]{};

} // 匿名命名空间

namespace ardugirl {

bool next_frame(std::uint8_t) noexcept { return true; }
void display() noexcept { ++replay_frames; }
std::uint8_t buttons() noexcept { return replay_buttons; }

} // 命名空间 ardugirl

namespace ardugirl::platform {

std::uint32_t millis() noexcept { return replay_frames * 16u; }
std::uint32_t micros() noexcept { return millis() * 1000u; }
void sleep_ms(std::uint32_t) noexcept {}
void set_tone(std::uint16_t, std::uint8_t) noexcept {}
void stop_tone(std::uint8_t) noexcept {}
void play_wave(std::uint16_t, const std::uint8_t*, std::uint16_t) noexcept {}
void stop_wave() noexcept {}
void set_synth(const SynthVoice*, std::uint8_t) noexcept {}
bool storage_read(std::uint16_t offset, void* destination, std::uint16_t size) noexcept {
    if (static_cast<std::size_t>(offset) + size > sizeof(replay_storage)) return false;
    std::memcpy(destination, replay_storage + offset, size);
    return true;
}
bool storage_write(std::uint16_t offset, const void* source, std::uint16_t size) noexcept {
    if (static_cast<std::size_t>(offset) + size > sizeof(replay_storage)) return false;
    std::memcpy(replay_storage + offset, source, size);
    return true;
}

} // 命名空间 ardugirl::platform

#include "../games/ardynia/entry.cpp"

namespace {

void frame(std::uint8_t buttons = 0) {
    replay_buttons = buttons;
    ardugirl_update_playtunes();
    loop();
}

} // 匿名命名空间

int main() {
    setup();
    for (int index = 0; index < 180; ++index) frame();
    frame(A_BUTTON);
    frame();
    for (int index = 0; index < 180; ++index) frame(UP_BUTTON);
    return 0;
}
