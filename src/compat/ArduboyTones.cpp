#include "ArduboyTones.h"

#include "ardugirl/platform.hpp"

#include <cstddef>

namespace {

bool default_enabled() noexcept { return true; }

bool (*output_enabled)() = default_enabled;
bool* output_enabled_state = nullptr;
const std::uint16_t* sequence_start = nullptr;
const std::uint16_t* sequence_index = nullptr;
std::uint16_t inline_sequence[7]{};
std::uint32_t note_end_ms = 0;
bool tones_playing = false;
std::uint8_t volume_mode = VOLUME_IN_TONE;

bool deadline_reached(std::uint32_t now, std::uint32_t deadline) noexcept {
    return static_cast<std::int32_t>(now - deadline) >= 0;
}

} // 匿名命名空间

ArduboyTones::ArduboyTones(bool (*enabled)()) noexcept {
    output_enabled = enabled == nullptr ? default_enabled : enabled;
    output_enabled_state = nullptr;
}

ArduboyTones::ArduboyTones(bool& enabled) noexcept {
    output_enabled = default_enabled;
    output_enabled_state = &enabled;
}

void ArduboyTones::tone(std::uint16_t frequency, std::uint16_t duration) noexcept {
    inline_sequence[0] = frequency;
    inline_sequence[1] = duration;
    inline_sequence[2] = TONES_END;
    startSequence(inline_sequence);
}

void ArduboyTones::tone(std::uint16_t frequency1, std::uint16_t duration1,
                        std::uint16_t frequency2, std::uint16_t duration2) noexcept {
    inline_sequence[0] = frequency1;
    inline_sequence[1] = duration1;
    inline_sequence[2] = frequency2;
    inline_sequence[3] = duration2;
    inline_sequence[4] = TONES_END;
    startSequence(inline_sequence);
}

void ArduboyTones::tone(std::uint16_t frequency1, std::uint16_t duration1,
                        std::uint16_t frequency2, std::uint16_t duration2,
                        std::uint16_t frequency3, std::uint16_t duration3) noexcept {
    inline_sequence[0] = frequency1;
    inline_sequence[1] = duration1;
    inline_sequence[2] = frequency2;
    inline_sequence[3] = duration2;
    inline_sequence[4] = frequency3;
    inline_sequence[5] = duration3;
    inline_sequence[6] = TONES_END;
    startSequence(inline_sequence);
}

void ArduboyTones::tones(const std::uint16_t* sequence) noexcept {
    startSequence(sequence);
}

void ArduboyTones::tonesInRAM(std::uint16_t* sequence) noexcept {
    startSequence(sequence);
}

void ArduboyTones::startSequence(const std::uint16_t* sequence) noexcept {
    noTone();
    sequence_start = sequence;
    sequence_index = sequence;
    nextTone();
}

void ArduboyTones::nextTone() noexcept {
    if (sequence_index == nullptr) return;

    std::uint16_t frequency = pgm_read_word(sequence_index++);
    if (frequency == TONES_END) {
        noTone();
        return;
    }
    if (frequency == TONES_REPEAT) {
        sequence_index = sequence_start;
        frequency = pgm_read_word(sequence_index++);
    }

    const std::uint16_t duration = pgm_read_word(sequence_index++);
    frequency &= static_cast<std::uint16_t>(~TONE_HIGH_VOLUME);
    tones_playing = true;
    note_end_ms = duration == 0 ? 0 : millis() + duration;
    const bool enabled = output_enabled_state == nullptr ? output_enabled() : *output_enabled_state;
    if (frequency != NOTE_REST && enabled) {
        ardugirl::platform::set_tone(frequency);
    } else {
        ardugirl::platform::stop_tone();
    }
}

void ArduboyTones::noTone() noexcept {
    ardugirl::platform::stop_tone();
    tones_playing = false;
    note_end_ms = 0;
    sequence_start = nullptr;
    sequence_index = nullptr;
}

void ArduboyTones::volumeMode(std::uint8_t mode) noexcept {
    // 当前平台输出没有 Arduboy 双引脚高音量模式，但保留选择值以维持 API 行为。
    volume_mode = mode;
    (void)volume_mode;
}

bool ArduboyTones::playing() noexcept {
    update();
    return tones_playing;
}

void ArduboyTones::update() noexcept {
    while (tones_playing && note_end_ms != 0 && deadline_reached(millis(), note_end_ms)) {
        nextTone();
    }
}

void ardugirl_update_tones() noexcept { ArduboyTones::update(); }
