#include "ArduboyPlaytune.h"

#include <cassert>
#include <cstdint>

namespace {

std::uint32_t now_ms = 0;
std::uint16_t frequencies[2]{};

} // 匿名命名空间

namespace ardugirl::platform {

std::uint32_t millis() noexcept { return now_ms; }

void set_tone(std::uint16_t frequency, std::uint8_t channel) noexcept {
    assert(channel < 2);
    frequencies[channel] = frequency;
}

void stop_tone(std::uint8_t channel) noexcept {
    assert(channel < 2);
    frequencies[channel] = 0;
}

} // 命名空间 ardugirl::platform

int main() {
    bool enabled = true;
    ArduboyPlaytune player(enabled);
    player.initChannel(0);

    // 播放 A4 100 毫秒、停止声道、再结束乐谱。
    constexpr byte score[] = {0x90, 69, 0x00, 0x64, 0x80, 0xF0};
    player.playScore(score);
    assert(player.playing());
    assert(frequencies[0] >= 439 && frequencies[0] <= 441);

    now_ms = 99;
    ardugirl_update_playtunes();
    assert(player.playing());
    assert(frequencies[0] != 0);

    now_ms = 100;
    ardugirl_update_playtunes();
    assert(!player.playing());
    assert(frequencies[0] == 0);

    enabled = false;
    player.playScore(score);
    assert(frequencies[0] == 0);
    player.stopScore();
    return 0;
}
