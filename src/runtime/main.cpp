#include "ardugirl/platform.hpp"
#include "ATMlib.h"
#include "ardugirl/runtime.hpp"
#include "ArduboyPlaytune.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

void setup();
void loop();
const char* ardugirl_game_title() noexcept;
const char* ardugirl_game_id() noexcept;

namespace {

std::uint32_t frame_limit = 0;

struct ButtonReplay {
    std::uint32_t first_frame;
    std::uint32_t last_frame;
    std::uint8_t mask;
};

struct FrameCapture {
    std::uint32_t frame;
    const char* path;
};

constexpr std::size_t replay_capacity = 128;
constexpr std::size_t capture_capacity = 16;
ButtonReplay button_replays[replay_capacity]{};
FrameCapture frame_captures[capture_capacity]{};
std::size_t button_replay_count = 0;
std::size_t frame_capture_count = 0;
std::uint32_t current_frame = 0;

bool parse_u32(const char*& cursor, char separator, std::uint32_t& value) {
    char* end = nullptr;
    const auto parsed = std::strtoul(cursor, &end, 0);
    if (end == cursor || parsed > UINT32_MAX || *end != separator) {
        return false;
    }
    value = static_cast<std::uint32_t>(parsed);
    cursor = end + 1;
    return true;
}

bool parse_button_replay(const char* argument) {
    if (button_replay_count >= replay_capacity) {
        return false;
    }

    const char* cursor = argument;
    std::uint32_t first_frame = 0;
    std::uint32_t mask = 0;
    if (!parse_u32(cursor, ':', first_frame) || !parse_u32(cursor, ':', mask)) {
        return false;
    }

    char* end = nullptr;
    const auto duration = std::strtoul(cursor, &end, 0);
    if (end == cursor || *end != '\0' || duration == 0 || duration > UINT32_MAX || mask > 0x3Fu ||
        first_frame > UINT32_MAX - static_cast<std::uint32_t>(duration - 1)) {
        return false;
    }

    button_replays[button_replay_count++] = {
        first_frame,
        first_frame + static_cast<std::uint32_t>(duration - 1),
        static_cast<std::uint8_t>(mask),
    };
    return true;
}

bool parse_frame_capture(const char* argument) {
    if (frame_capture_count >= capture_capacity) {
        return false;
    }

    const char* cursor = argument;
    std::uint32_t frame = 0;
    if (!parse_u32(cursor, ':', frame) || *cursor == '\0') {
        return false;
    }
    frame_captures[frame_capture_count++] = {frame, cursor};
    return true;
}

bool capture_framebuffer(const char* path) {
    auto* output = std::fopen(path, "wb");
    if (output == nullptr) {
        return false;
    }

    // PGM 是验收流水线的无损中间格式。逐像素展开页面布局，避免截图工具依赖 SDL 窗口状态。
    if (std::fprintf(output, "P5\n128 64\n255\n") < 0) {
        std::fclose(output);
        return false;
    }
    const auto& pixels = ardugirl::framebuffer().data();
    for (std::uint32_t y = 0; y < 64; ++y) {
        for (std::uint32_t x = 0; x < 128; ++x) {
            const auto byte = pixels[x + (y / 8u) * 128u];
            const unsigned char value = (byte & (1u << (y & 7u))) != 0 ? 255u : 0u;
            if (std::fwrite(&value, 1, 1, output) != 1) {
                std::fclose(output);
                return false;
            }
        }
    }
    return std::fclose(output) == 0;
}

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
        } else if (std::strcmp(argv[index], "--save-dir") == 0 && index + 1 < argc) {
            config.save_dir = argv[++index];
        } else if (std::strcmp(argv[index], "--replay-button") == 0 && index + 1 < argc) {
            if (!parse_button_replay(argv[++index])) {
                return false;
            }
        } else if (std::strcmp(argv[index], "--capture-frame") == 0 && index + 1 < argc) {
            if (!parse_frame_capture(argv[++index])) {
                return false;
            }
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
    auto result = platform::buttons();
    for (std::size_t index = 0; index < button_replay_count; ++index) {
        const auto& replay = button_replays[index];
        if (current_frame >= replay.first_frame && current_frame <= replay.last_frame) {
            result = static_cast<std::uint8_t>(result | replay.mask);
        }
    }
    return result;
}

} // 命名空间 ardugirl

int main(int argc, char** argv) {
    ardugirl::platform::Config config;
    config.title = ardugirl_game_title();
    config.game_id = ardugirl_game_id();
    if (!parse_arguments(argc, argv, config) || !ardugirl::platform::init(config)) {
        return 1;
    }

    setup();
    std::uint32_t rendered_frames = 0;
    while (ardugirl::platform::pump_events()) {
        current_frame = rendered_frames;
        ardugirl_update_playtunes();
        ardugirl::atm::service();
        loop();
        ++rendered_frames;
        for (std::size_t index = 0; index < frame_capture_count; ++index) {
            if (frame_captures[index].frame == rendered_frames &&
                !capture_framebuffer(frame_captures[index].path)) {
                ardugirl::platform::shutdown();
                return 1;
            }
        }
        if (frame_limit != 0 && rendered_frames >= frame_limit) {
            break;
        }
    }

    ardugirl::platform::shutdown();
    return 0;
}
