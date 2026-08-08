#pragma once

#include <cstdint>

#include "ardugirl/platform.hpp"

class BeepPin1 {
public:
    void begin() noexcept { remaining_ = 0; }
    void timer() noexcept {
        if (remaining_ != 0 && --remaining_ == 0) ardugirl::platform::stop_tone();
    }

    std::uint16_t freq(std::uint16_t frequency) const noexcept {
        return frequency;
    }

    void tone(std::uint16_t frequency, std::uint8_t duration) noexcept {
        // Beep 的持续时间由游戏按帧调用 timer() 管理，这里只负责平台方波输出。
        if (frequency == 0 || duration == 0) {
            ardugirl::platform::stop_tone();
        } else {
            ardugirl::platform::set_tone(frequency);
        }
        remaining_ = duration;
    }

private:
    std::uint8_t remaining_ = 0;
};

using BeepPin2 = BeepPin1;
