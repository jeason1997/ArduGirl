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
};

bool init(const Config& config) noexcept;
void shutdown() noexcept;
bool pump_events() noexcept;
std::uint8_t buttons() noexcept;
std::uint32_t millis() noexcept;
void sleep_ms(std::uint32_t duration) noexcept;
void present(const Framebuffer::Storage& pixels) noexcept;

} // 命名空间 ardugirl::platform
