#pragma once

#include "Arduino.h"

#include <cstdint>

#define TONES_END 0x8000
#define TONES_REPEAT 0x8001
#define TONE_HIGH_VOLUME 0x8000
#define VOLUME_IN_TONE 0
#define VOLUME_ALWAYS_NORMAL 1
#define VOLUME_ALWAYS_HIGH 2

#define NOTE_REST 0
#define NOTE_D3 147
#define NOTE_G3 196
#define NOTE_AS3 233
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_F4 349
#define NOTE_G4 392

class ArduboyTones {
public:
    explicit ArduboyTones(bool (*output_enabled)()) noexcept;
    explicit ArduboyTones(bool& output_enabled) noexcept;
    template<typename EnabledState>
    explicit ArduboyTones(EnabledState& output_enabled) noexcept
        : ArduboyTones(static_cast<bool&>(output_enabled)) {}

    static void tone(std::uint16_t frequency, std::uint16_t duration = 0) noexcept;
    static void tone(std::uint16_t frequency1, std::uint16_t duration1,
                     std::uint16_t frequency2, std::uint16_t duration2) noexcept;
    static void tone(std::uint16_t frequency1, std::uint16_t duration1,
                     std::uint16_t frequency2, std::uint16_t duration2,
                     std::uint16_t frequency3, std::uint16_t duration3) noexcept;
    static void tones(const std::uint16_t* sequence) noexcept;
    static void tonesInRAM(std::uint16_t* sequence) noexcept;
    static void noTone() noexcept;
    static void volumeMode(std::uint8_t mode) noexcept;
    static bool playing() noexcept;
    static void update() noexcept;

private:
    static void startSequence(const std::uint16_t* sequence) noexcept;
    static void nextTone() noexcept;
};

void ardugirl_update_tones() noexcept;
