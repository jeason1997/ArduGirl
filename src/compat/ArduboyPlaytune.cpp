#include "ArduboyPlaytune.h"

#include "ardugirl/platform.hpp"

#include <cstdint>

namespace {

ArduboyPlaytune* first_player = nullptr;

std::uint16_t midi_frequency(std::uint8_t note) noexcept {
    // MIDI 0 的频率以 Q16 表示，逐半音乘以 2^(1/12)。全程使用整数，
    // 避免把浮点运算变成资源受限 MCU 后端的必要能力。
    std::uint64_t frequency_q16 = 535810u;
    for (std::uint8_t index = 0; index < note; ++index) {
        frequency_q16 = (frequency_q16 * 69433u + 32768u) >> 16u;
    }
    return static_cast<std::uint16_t>((frequency_q16 + 32768u) >> 16u);
}

} // 匿名命名空间

ArduboyPlaytune::ArduboyPlaytune(bool& output_enabled) noexcept
    : output_enabled_(&output_enabled), next_(first_player) {
    first_player = this;
}

ArduboyPlaytune::~ArduboyPlaytune() noexcept {
    stopScore();
    auto** link = &first_player;
    while (*link != nullptr && *link != this) link = &((*link)->next_);
    if (*link == this) *link = next_;
}

void ArduboyPlaytune::initChannel(std::uint8_t channel) noexcept {
    // 参数是物理扬声器引脚而非声道编号；每次调用按顺序追加一个逻辑声道。
    // DevKit 会用相同引脚调用两次，因此不能依据引脚值去重。
    if (channel_count_ < 2) ++channel_count_;
    (void) channel;
}

void ArduboyPlaytune::playScore(const byte* score, std::int8_t pitch) noexcept {
    stopScore();
    if (score == nullptr) return;
    score_start_ = score;
    score_cursor_ = score;
    repeat_count_ = 0;
    default_pitch_ = pitch;
    pitch_ = pitch;
    playing_ = true;
    stepScore();
}

void ArduboyPlaytune::stopScore() noexcept {
    for (std::uint8_t channel = 0; channel < 2; ++channel) stopNote(channel);
    playing_ = false;
    wait_deadline_ms_ = 0;
}

void ArduboyPlaytune::update() noexcept {
    if (!playing_ || wait_deadline_ms_ == 0) return;
    if (static_cast<std::int32_t>(ardugirl::platform::millis() - wait_deadline_ms_) >= 0) {
        wait_deadline_ms_ = 0;
        stepScore();
    }
}

void ArduboyPlaytune::stepScore() noexcept {
    // 单次调用一直解释到等待或结束指令，保持原库由定时器中断推进的语义。
    while (playing_) {
        const auto command = pgm_read_byte(score_cursor_++);
        const auto opcode = static_cast<std::uint8_t>(command & 0xF0u);
        const auto operand = static_cast<std::uint8_t>(command & 0x0Fu);
        const auto channel = static_cast<std::uint8_t>(operand & 0x03u);
        if (opcode == 0x80u) {
            stopNote(channel);
        } else if (opcode == 0x90u) {
            playNote(channel, pgm_read_byte(score_cursor_++));
        } else if (opcode < 0x80u) {
            const auto duration = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(command) << 8u |
                pgm_read_byte(score_cursor_++));
            wait_deadline_ms_ = ardugirl::platform::millis() + (duration == 0 ? 1u : duration);
            return;
        } else if (opcode == 0xC0u) {
            score_start_ = score_cursor_ - 1;
            if (repeat_count_ > 0) {
                constexpr std::int8_t pitch_steps[] = {
                    0, 1, 2, 3, 4, 6, 8, 12, 0, -12, -8, -6, -4, -3, -2, -1
                };
                pitch_ = static_cast<std::int8_t>(pitch_ + pitch_steps[operand]);
            }
        } else if (opcode == 0xD0u) {
            if (operand == 0 || ++repeat_count_ < (1u << operand)) {
                score_cursor_ = score_start_;
            } else {
                score_start_ = score_cursor_;
                repeat_count_ = 0;
                pitch_ = default_pitch_;
            }
        } else if (opcode == 0xF0u) {
            stopScore();
        }
    }
}

void ArduboyPlaytune::playNote(std::uint8_t channel, std::uint8_t note) noexcept {
    if (channel >= channel_count_ || channel >= 2) return;
    if (output_enabled_ != nullptr && *output_enabled_) {
        const auto shifted = static_cast<std::int16_t>(note) + pitch_;
        if (shifted >= 0 && shifted <= 127) {
            ardugirl::platform::set_tone(midi_frequency(static_cast<std::uint8_t>(shifted)), channel);
        }
    }
}

void ArduboyPlaytune::stopNote(std::uint8_t channel) noexcept {
    if (channel < 2) ardugirl::platform::stop_tone(channel);
}

void ardugirl_update_playtunes() noexcept {
    for (auto* player = first_player; player != nullptr; player = player->next_) {
        player->update();
    }
}
