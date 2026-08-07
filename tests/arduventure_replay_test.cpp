#include "ardugirl/framebuffer.hpp"
#include "ardugirl/runtime.hpp"
#include "ardugirl/platform.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>

namespace {

std::uint8_t replay_buttons = 0;
std::uint32_t presented_frames = 0;
std::array<std::uint8_t, 1024> replay_storage{};
std::array<ardugirl::platform::SynthVoice, 4> replay_voices{};

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
void set_synth(const SynthVoice* voices, std::uint8_t count) noexcept {
    replay_voices = {};
    for (std::size_t index = 0; voices != nullptr && index < count && index < replay_voices.size(); ++index)
        replay_voices[index] = voices[index];
}
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

#include "../build/generated/arduventure/ardugirl_entry.cpp"

namespace {

void frame(std::uint8_t buttons = 0) {
    replay_buttons = buttons;
    ardugirl::atm::service();
    loop();
}

void tap(std::uint8_t button) {
    frame(button);
    frame();
}

} // 匿名命名空间

int main(int argc, char** argv) {
    replay_storage.fill(0xFF);
    setup();
    frame();
    assert(gameState == STATE_MENU_INTRO);
    bool music_started = false;
    for (const auto& voice : replay_voices) music_started |= voice.frequency_hz != 0;
    assert(music_started);

    tap(A_BUTTON);
    assert(gameState == STATE_MENU_MAIN);
    assert(cursorY == STATE_MENU_NEW);

    tap(B_BUTTON);
    frame();
    assert(gameState == STATE_GAME_NEW);
    assert(currentLetter == 0);

    for (std::uint8_t index = 0; index < 4; ++index) tap(DOWN_BUTTON);
    assert(cursorY == 4);
    tap(B_BUTTON);
    assert(gameState == STATE_GAME_INTRO);
    frame();
    assert(presented_frames > 0);

    if (argc == 2) {
        gameState = STATE_MENU_INTRO;
        globalCounter = 255;
        frame();
        std::ofstream image(argv[1], std::ios::binary);
        image << "P5\n128 64\n255\n";
        const auto& pixels = ardugirl::framebuffer().data();
        for (std::uint8_t y = 0; y < 64; ++y) {
            for (std::uint8_t x = 0; x < 128; ++x) {
                const auto value = (pixels[x + (y / 8u) * 128u] & (1u << (y & 7u))) != 0 ? 255 : 0;
                image.put(static_cast<char>(value));
            }
        }
        assert(image.good());
    }
    return 0;
}
