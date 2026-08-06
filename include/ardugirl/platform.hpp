#pragma once

#include "ardugirl/framebuffer.hpp"

#include <cstdint>

namespace ardugirl::platform {

struct Config {
    bool plain_output = false;
    bool headless = false;
    bool fullscreen = false;
    bool invert = false;
    std::uint8_t scale = 1;
    const char* title = "ArduGirl";
    const char* game_id = "unknown";
    const char* save_dir = nullptr;
};

bool init(const Config& config) noexcept;
void shutdown() noexcept;
bool pump_events() noexcept;
std::uint8_t buttons() noexcept;
std::uint32_t millis() noexcept;
std::uint32_t micros() noexcept;
void sleep_ms(std::uint32_t duration) noexcept;
void set_tone(std::uint16_t frequency_hz, std::uint8_t channel = 0) noexcept;
void stop_tone(std::uint8_t channel = 0) noexcept;
void play_wave(std::uint16_t sample_rate_hz, const std::uint8_t* samples,
               std::uint16_t sample_count) noexcept;
void stop_wave() noexcept;
struct SynthVoice {
    std::uint16_t frequency_hz = 0;
    std::uint8_t volume = 0;
    std::uint8_t waveform = 0;
};
void set_synth(const SynthVoice* voices, std::uint8_t count) noexcept;
void present(const Framebuffer::Storage& pixels) noexcept;
bool storage_read(std::uint16_t offset, void* destination,
                  std::uint16_t size) noexcept;
bool storage_write(std::uint16_t offset, const void* source,
                   std::uint16_t size) noexcept;

} // 命名空间 ardugirl::platform
