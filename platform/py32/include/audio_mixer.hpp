#pragma once

#include "ardugirl/platform.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace ardugirl::py32 {

class AudioMixer {
public:
    static constexpr std::uint32_t kSampleRateHz = 50000u;

    void reset() noexcept {
        tones_ = {};
        voices_ = {};
        noise_state_ = 1;
    }

    void set_tone(std::uint8_t channel, std::uint16_t frequency_hz) noexcept {
        if (channel >= tones_.size()) return;
        auto& tone = tones_[channel];
        if (tone.step == 0 && frequency_hz != 0) tone.phase = 0;
        tone.step = phase_step(frequency_hz);
    }

    void set_synth(const platform::SynthVoice* voices, std::uint8_t count) noexcept {
        for (std::size_t index = 0; index < voices_.size(); ++index) {
            const auto enabled = voices != nullptr && index < count &&
                                 voices[index].frequency_hz != 0 && voices[index].volume != 0;
            if (!enabled) {
                voices_[index].step = 0;
                voices_[index].volume = 0;
                continue;
            }
            auto& destination = voices_[index];
            if (destination.step == 0) destination.phase = 0;
            destination.step = phase_step(voices[index].frequency_hz);
            destination.volume = voices[index].volume;
            destination.waveform = voices[index].waveform;
        }
    }

    bool active() const noexcept {
        for (const auto& tone : tones_)
            if (tone.step != 0) return true;
        for (const auto& voice : voices_)
            if (voice.step != 0 && voice.volume != 0) return true;
        return false;
    }

    std::int8_t next_sample() noexcept {
        std::int32_t tones = 0;
        std::int32_t synth = 0;
        for (auto& tone : tones_) {
            if (tone.step == 0) continue;
            tone.phase += tone.step;
            // Playtune 的独立方波保持满幅；同时发声时由最终限幅处理。
            tones += tone.phase < 0x80000000u ? 127 : -128;
        }
        for (auto& voice : voices_) {
            if (voice.step == 0 || voice.volume == 0) continue;
            voice.phase += voice.step;
            if (voice.waveform == 0) {
                // ATMlib 声道 0 的 AVR ISR 仅在相位最高两位均为 1 时翻转极性。
                synth += (voice.phase & 0xC0000000u) == 0xC0000000u ?
                    -voice.volume : voice.volume;
            } else if (voice.waveform == 2) {
                const auto ramp = static_cast<std::int32_t>(voice.phase >> 23u) & 0x1FF;
                const auto triangle = ramp < 256 ? ramp - 128 : 383 - ramp;
                synth += triangle * voice.volume / 128;
            } else if (voice.waveform == 3) {
                noise_state_ = (noise_state_ >> 1u) ^
                    (static_cast<std::uint32_t>(-(noise_state_ & 1u)) & 0xB400u);
                synth += (noise_state_ & 1u) != 0 ? voice.volume : -voice.volume;
            } else {
                synth += voice.phase < 0x80000000u ? voice.volume : -voice.volume;
            }
        }
        // ATMlib 四声部理论峰值超过 8 位范围；固定留出一位余量，避免硬削波产生毛刺。
        auto mixed = tones + synth / 2;
        if (mixed > 127) mixed = 127;
        if (mixed < -128) mixed = -128;
        return static_cast<std::int8_t>(mixed);
    }

private:
    struct Tone {
        std::uint32_t phase = 0;
        std::uint32_t step = 0;
    };

    struct Voice {
        std::uint32_t phase = 0;
        std::uint32_t step = 0;
        std::uint8_t volume = 0;
        std::uint8_t waveform = 0;
    };

    static std::uint32_t phase_step(std::uint16_t frequency_hz) noexcept {
        // floor(2^32 / 50000)，避免 Cortex-M0+ 在运行时执行 64 位除法。
        return static_cast<std::uint32_t>(frequency_hz) * 85899u;
    }

    std::array<Tone, 2> tones_{};
    std::array<Voice, 4> voices_{};
    std::uint32_t noise_state_ = 1;
};

} // 命名空间 ardugirl::py32
