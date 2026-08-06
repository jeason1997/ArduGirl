#include "ardugirl/platform.hpp"
#include "ardugirl/runtime.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>

void setup();
void loop();
const char* ardugirl_game_title() noexcept;

namespace {

std::uint32_t frame_limit = 0;

bool parse_arguments(int argc, char** argv, ardugirl::platform::Config& config) {
    for (int index = 1; index < argc; ++index) {
        if (std::strcmp(argv[index], "--plain") == 0) {
            config.plain_output = true;
        } else if (std::strcmp(argv[index], "--headless") == 0) {
            config.headless = true;
        } else if (std::strcmp(argv[index], "--fullscreen") == 0) {
            config.fullscreen = true;
        } else if (std::strcmp(argv[index], "--invert") == 0) {
            config.invert = true;
        } else if (std::strcmp(argv[index], "--scale") == 0 && index + 1 < argc) {
            const auto scale = std::strtoul(argv[++index], nullptr, 10);
            if (scale == 0 || scale > 32) {
                return false;
            }
            config.scale = static_cast<std::uint8_t>(scale);
        } else if (std::strcmp(argv[index], "--frames") == 0 && index + 1 < argc) {
            frame_limit = static_cast<std::uint32_t>(std::strtoul(argv[++index], nullptr, 10));
        } else {
            return false;
        }
    }
    return true;
}

} // 匿名命名空间

namespace ardugirl {

bool next_frame(std::uint8_t frames_per_second) noexcept {
    static std::uint32_t deadline = 0;
    if (frames_per_second == 0) {
        return true;
    }

    const auto frame_duration = static_cast<std::uint32_t>(1000u / frames_per_second);
    const auto now = platform::millis();
    if (deadline == 0) {
        deadline = now;
    }

    // 有符号结果只用于判断期限是否已到。先做无符号减法再转换，可以保留
    // Arduino 常用的 32 位计时器回绕语义。
    if (static_cast<std::int32_t>(now - deadline) < 0) {
        platform::sleep_ms(deadline - now);
    }
    deadline += frame_duration;
    return true;
}

void display() noexcept {
    platform::present(framebuffer().data());
}

std::uint8_t buttons() noexcept {
    return platform::buttons();
}

} // 命名空间 ardugirl

int main(int argc, char** argv) {
    ardugirl::platform::Config config;
    config.title = ardugirl_game_title();
    if (!parse_arguments(argc, argv, config) || !ardugirl::platform::init(config)) {
        return 1;
    }

    setup();
    std::uint32_t rendered_frames = 0;
    while (ardugirl::platform::pump_events()) {
        loop();
        ++rendered_frames;
        if (frame_limit != 0 && rendered_frames >= frame_limit) {
            break;
        }
    }

    ardugirl::platform::shutdown();
    return 0;
}
