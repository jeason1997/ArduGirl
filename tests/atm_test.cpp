#include "ATMlib.h"
#include "Arduboy2.h"
#include "ardugirl/platform.hpp"

#include <array>
#include <cassert>
#include <cstdint>

namespace {

std::uint32_t fake_millis = 0;
std::array<ardugirl::platform::SynthVoice, 4> published{};

// 一个轨道由四个声道共同读取：设置音量、播放 A4，并保持一个节拍。
constexpr byte song[] = {
    1,
    0, 0,
    0, 0, 0, 0,
    0x40, 32, 10, 160,
};

} // 匿名命名空间

std::uint32_t millis() noexcept { return fake_millis; }

bool Arduboy2Audio::enabled() noexcept { return true; }

void ardugirl::platform::set_synth(const SynthVoice* voices, std::uint8_t count) noexcept {
    published = {};
    for (std::size_t index = 0; voices != nullptr && index < count && index < published.size(); ++index)
        published[index] = voices[index];
}

int main() {
    ATMsynth synth;
    synth.play(song);
    ardugirl::atm::service();

    for (std::size_t index = 0; index < published.size(); ++index) {
        assert(published[index].frequency_hz == 440);
        assert(published[index].volume == 32);
        assert(published[index].waveform == index);
    }

    synth.muteChannel(2);
    assert(published[2].frequency_hz == 0);
    assert(published[0].frequency_hz == 440);

    synth.playPause();
    for (const auto& voice : published) assert(voice.frequency_hz == 0);
    synth.playPause();
    assert(published[0].frequency_hz == 440);

    synth.stop();
    for (const auto& voice : published) assert(voice.frequency_hz == 0);
    return 0;
}
