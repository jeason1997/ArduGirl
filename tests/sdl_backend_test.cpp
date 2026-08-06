#include "ardugirl/platform.hpp"
#include "../platform/linux/render.hpp"

#include <SDL.h>

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace {

constexpr std::uint8_t kLeft = 0x01;
constexpr std::uint8_t kUp = 0x04;
constexpr std::uint8_t kA = 0x10;

void push_key(Uint32 type, SDL_Keycode key, Uint8 repeat = 0) {
    SDL_Event event{};
    event.type = type;
    event.key.type = type;
    event.key.state = type == SDL_KEYDOWN ? SDL_PRESSED : SDL_RELEASED;
    event.key.repeat = repeat;
    event.key.keysym.sym = key;
    assert(SDL_PushEvent(&event) == 1);
}

std::uint64_t hash_pixels(const ardugirl::platform::detail::TexturePixels& pixels) {
    // 固定按 32 位像素值计算 FNV-1a，避免主机字节序影响 golden 值。
    std::uint64_t hash = 14695981039346656037ull;
    for (const auto pixel : pixels) {
        hash ^= pixel;
        hash *= 1099511628211ull;
    }
    return hash;
}

} // 匿名命名空间

int main() {
    ardugirl::platform::Config config;
    config.headless = true;
    assert(ardugirl::platform::init(config));
    const auto before_us = ardugirl::platform::micros();
    ardugirl::platform::sleep_ms(2);
    assert(ardugirl::platform::micros() > before_us);
    // 无音频设备的 CI 环境也必须安全接受音频控制调用。
    ardugirl::platform::set_tone(440);
    ardugirl::platform::stop_tone();
    constexpr std::uint8_t wave[] = {0, 128, 255};
    ardugirl::platform::play_wave(8000, wave, 3);
    ardugirl::platform::sleep_ms(2);
    ardugirl::platform::stop_wave();

    push_key(SDL_KEYDOWN, SDLK_LEFT);
    push_key(SDL_KEYDOWN, SDLK_w);
    push_key(SDL_KEYDOWN, SDLK_j);
    assert(ardugirl::platform::pump_events());
    assert(ardugirl::platform::buttons() == (kLeft | kUp | kA));

    // SDL 的自动重复事件不能产生新的按键边沿，也不能改变当前电平。
    push_key(SDL_KEYDOWN, SDLK_x, 1);
    push_key(SDL_KEYUP, SDLK_LEFT);
    assert(ardugirl::platform::pump_events());
    assert(ardugirl::platform::buttons() == (kUp | kA));

    SDL_Event quit{};
    quit.type = SDL_QUIT;
    assert(SDL_PushEvent(&quit) == 1);
    assert(!ardugirl::platform::pump_events());
    ardugirl::platform::shutdown();

    ardugirl::Framebuffer framebuffer;
    ardugirl::platform::detail::TexturePixels pixels{};
    for (std::size_t y = 0; y < ardugirl::kScreenHeight; ++y) {
        for (std::size_t x = 0; x < ardugirl::kScreenWidth; ++x) {
            if (((x / 8u) + (y / 8u)) % 2u == 0) {
                framebuffer.set_pixel(static_cast<int>(x), static_cast<int>(y));
            }
        }
    }

    ardugirl::platform::detail::convert_framebuffer(framebuffer.data(), false, pixels);
    assert(hash_pixels(pixels) == 0xf4221735b77a9325ull);
    assert(pixels[0] == 0xFFFFFFFFu);
    assert(pixels[8] == 0xFF000000u);

    ardugirl::platform::detail::convert_framebuffer(framebuffer.data(), true, pixels);
    assert(hash_pixels(pixels) == 0x3e9273dc5f7a9325ull);
    assert(pixels[0] == 0xFF000000u);
    assert(pixels[8] == 0xFFFFFFFFu);
    return 0;
}
