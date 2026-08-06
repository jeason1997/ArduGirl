#pragma once

#include "Arduino.h"

#include <cstdint>

class ATMsynth {
public:
    ATMsynth() noexcept = default;

    void play(const byte* song) noexcept;
    void playPause() noexcept;
    void stop() noexcept;
    void muteChannel(byte channel) noexcept;
    void unMuteChannel(byte channel) noexcept;
};

namespace ardugirl::atm {

// ATMlib 在 AVR 上由定时器中断推进；宿主平台在主循环中调用此入口，按单调
// 毫秒时间补齐所有到期节拍。乐谱解析仍属于兼容层，不进入音频后端。
void service() noexcept;

} // 命名空间 ardugirl::atm
