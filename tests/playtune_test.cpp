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

    bool stereo_enabled = true;
    ArduboyPlaytune stereo_player(stereo_enabled);
    // 官方 DevKit 用同一个扬声器引脚初始化两个逻辑声道。
    stereo_player.initChannel(0);
    stereo_player.initChannel(0);
    constexpr byte stereo_score[] = {
        0x90, 69, 0x91, 57, 0x00, 0x64, 0x80, 0x81, 0xF0
    };
    now_ms = 200;
    stereo_player.playScore(stereo_score);
    assert(frequencies[0] >= 439 && frequencies[0] <= 441);
    assert(frequencies[1] >= 219 && frequencies[1] <= 221);
    now_ms = 300;
    ardugirl_update_playtunes();
    assert(!stereo_player.playing());
    assert(frequencies[0] == 0 && frequencies[1] == 0);

    constexpr byte shifted_score[] = {0x90, 69, 0x00, 0x01, 0x80, 0xF0};
    now_ms = 400;
    stereo_player.playScore(shifted_score, 12);
    assert(frequencies[0] >= 879 && frequencies[0] <= 881);
    stereo_player.stopScore();

    bool persistent_enabled = true;
    auto* persistent_player = ardugirl_create_playtune(persistent_enabled);
    assert(persistent_player != nullptr);
    persistent_player->initChannel(0);
    persistent_player->playScore(shifted_score);
    assert(persistent_player->playing());
    persistent_player->stopScore();
    return 0;
}
