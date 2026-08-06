#pragma once

#include <cstdint>

class BeepPin1 {
public:
    void begin() noexcept {}
    void timer() noexcept {}

    std::uint16_t freq(std::uint16_t frequency) const noexcept {
        return frequency;
    }

    void tone(std::uint16_t frequency, std::uint8_t duration) noexcept {
        // 终端阶段暂时静音，但保留非阻塞接口，避免声音调用改变游戏时序。
        (void) frequency;
        (void) duration;
    }
};
