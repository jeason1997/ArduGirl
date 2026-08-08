#include "ardugirl/framebuffer.hpp"
#include "ardugirl/platform.hpp"
#include "ardugirl/runtime.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {

std::uint8_t replay_buttons = 0;
std::uint32_t presented_frames = 0;
std::array<std::uint8_t, 1024> replay_storage{};

} // 匿名命名空间

namespace ardugirl {

bool next_frame(std::uint8_t) noexcept { return true; }
void display() noexcept { ++presented_frames; }
std::uint8_t buttons() noexcept { return replay_buttons; }

} // 命名空间 ardugirl

namespace ardugirl::platform {

std::uint32_t millis() noexcept { return presented_frames * 16u; }
std::uint32_t micros() noexcept { return millis() * 1000u; }
void sleep_ms(std::uint32_t) noexcept {}
void set_tone(std::uint16_t, std::uint8_t) noexcept {}
void stop_tone(std::uint8_t) noexcept {}
void play_wave(std::uint16_t, const std::uint8_t*, std::uint16_t) noexcept {}
void stop_wave() noexcept {}
void set_synth(const SynthVoice*, std::uint8_t) noexcept {}
bool storage_read(std::uint16_t offset, void* destination, std::uint16_t size) noexcept {
    if (static_cast<std::size_t>(offset) + size > replay_storage.size()) return false;
    std::memcpy(destination, replay_storage.data() + offset, size);
    return true;
}
bool storage_write(std::uint16_t offset, const void* source, std::uint16_t size) noexcept {
    if (static_cast<std::size_t>(offset) + size > replay_storage.size()) return false;
    std::memcpy(replay_storage.data() + offset, source, size);
    return true;
}

} // 命名空间 ardugirl::platform

#include "../build/generated/rooftop-rescue/ardugirl_entry.cpp"

namespace {

void frame(std::uint8_t buttons = 0) {
    replay_buttons = buttons;
    loop();
}

void tap(std::uint8_t button) {
    frame(button);
    frame();
}

} // 匿名命名空间

int main() {
    replay_storage.fill(0xFF);
    setup();
    assert(game.state == Game::SPLASH);

    for (int index = 0; index < 210; ++index) frame();
    assert(game.state == Game::TITLE);
    tap(A_BUTTON);
    assert(game.state == Game::FOREPLAY);

    for (int index = 0; index < 64; ++index) frame();
    assert(game.state == Game::PLAYING);
    assert(game.chopper.active);

    for (int index = 0; index < 300; ++index) frame();
    assert(game.chopper.x == game.stops[4]);
    tap(LEFT_BUTTON);
    for (int index = 0; index < 180; ++index) frame();
    assert(game.chopper.current_stop == 3);
    assert(game.chopper.x == game.stops[3]);
    for (int index = 0; index < 30; ++index) frame(DOWN_BUTTON);
    assert(game.chopper.rope.length > 0.0f);
    assert(presented_frames > 400);
    return 0;
}
