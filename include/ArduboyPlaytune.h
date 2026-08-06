#pragma once

#include "Arduino.h"

class ArduboyPlaytune {
public:
    explicit ArduboyPlaytune(bool& output_enabled) noexcept;
    ~ArduboyPlaytune() noexcept;
    ArduboyPlaytune(const ArduboyPlaytune&) = delete;
    ArduboyPlaytune& operator=(const ArduboyPlaytune&) = delete;

    void initChannel(std::uint8_t channel) noexcept;
    bool playing() const noexcept { return playing_; }
    void playScore(const byte* score) noexcept;
    void stopScore() noexcept;
    void update() noexcept;

private:
    void stepScore() noexcept;
    void playNote(std::uint8_t channel, std::uint8_t note) noexcept;
    void stopNote(std::uint8_t channel) noexcept;

    bool* output_enabled_ = nullptr;
    const byte* score_start_ = nullptr;
    const byte* score_cursor_ = nullptr;
    std::uint32_t wait_deadline_ms_ = 0;
    std::uint8_t channel_count_ = 0;
    std::uint8_t repeat_count_ = 0;
    bool playing_ = false;
    ArduboyPlaytune* next_ = nullptr;

    friend void ardugirl_update_playtunes() noexcept;
};

// 运行时调用这一入口推进所有播放器；游戏无需手动轮询乐谱状态。
void ardugirl_update_playtunes() noexcept;
