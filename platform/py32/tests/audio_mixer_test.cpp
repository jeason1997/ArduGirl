#include "audio_mixer.hpp"

#include <array>
#include <cassert>

using ardugirl::py32::AudioMixer;

int main() {
    AudioMixer mixer;
    assert(mixer.next_sample() == 0);

    mixer.set_tone(0, 1000);
    bool positive = false;
    bool negative = false;
    for (int index = 0; index < 100; ++index) {
        const auto output = mixer.next_sample();
        positive |= output > 0;
        negative |= output < 0;
    }
    assert(positive && negative);

    // 两个 Playtune 声道必须能独立停止，不能像旧后端一样停止任一声道就全局静音。
    mixer.set_tone(1, 1500);
    mixer.set_tone(0, 0);
    for (int index = 0; index < 20; ++index)
        assert(mixer.next_sample() != 0);
    mixer.set_tone(1, 0);
    assert(mixer.next_sample() == 0);

    std::array<ardugirl::platform::SynthVoice, 4> voices{};
    voices[0] = {440, 32, 0};
    voices[1] = {660, 24, 1};
    voices[2] = {880, 16, 2};
    voices[3] = {1200, 8, 3};
    mixer.set_synth(voices.data(), static_cast<std::uint8_t>(voices.size()));
    positive = false;
    negative = false;
    for (int index = 0; index < 500; ++index) {
        const auto output = mixer.next_sample();
        positive |= output > 0;
        negative |= output < 0;
    }
    assert(positive && negative);

    mixer.set_synth(nullptr, 0);
    assert(mixer.next_sample() == 0);
    return 0;
}
