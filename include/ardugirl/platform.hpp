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
void sleep_ms(std::uint32_t duration) noexcept;
void present(const Framebuffer::Storage& pixels) noexcept;
bool storage_read(std::uint16_t offset, void* destination,
                  std::uint16_t size) noexcept;
bool storage_write(std::uint16_t offset, const void* source,
                   std::uint16_t size) noexcept;

} // 命名空间 ardugirl::platform
