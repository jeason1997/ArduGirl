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
#define NOTE_C1 33
#define NOTE_D1 37
#define NOTE_C2 65
#define NOTE_D2 73
#define NOTE_E2 82
#define NOTE_D3 147
#define NOTE_G3 196
#define NOTE_AS3 233
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_C7 2093
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_G7 3136
#define NOTE_AS7 3729
#define NOTE_D8 4699

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
