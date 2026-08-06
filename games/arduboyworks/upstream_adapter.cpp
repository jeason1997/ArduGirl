#include "MyArduboy2.h"
#include "ArduboyPlaytune.h"

namespace {

bool score_output_enabled = true;
ArduboyPlaytune score_player(score_output_enabled);
std::uint8_t score_priority = 0xFF;

struct ScoreChannels {
    ScoreChannels() noexcept {
        score_player.initChannel(0);
        score_player.initChannel(0);
    }
} score_channels;

} // 匿名命名空间

// MyArduboy2 是 ArduboyWorks 上游自定义的派生类，并非 ArduGirl API。
// 这层只满足上游重复声明产生的链接符号，所有语义统一转发给 Arduboy2 核心。
void MyArduboy2::initAudio(std::uint8_t channels) { Arduboy2::initAudio(channels); }
void MyArduboy2::closeAudio() { Arduboy2::closeAudio(); }
bool MyArduboy2::isAudioEnabled() { return Arduboy2::isAudioEnabled(); }
void MyArduboy2::setAudioEnabled(bool enabled) { Arduboy2::setAudioEnabled(enabled); }
void MyArduboy2::toggleAudioEnabled() { Arduboy2::toggleAudioEnabled(); }
void MyArduboy2::saveAudioOnOff() { Arduboy2::saveAudioOnOff(); }
void MyArduboy2::playTone(std::uint16_t frequency, std::uint16_t duration,
                          std::uint8_t priority, std::uint8_t duty_cycle) {
    Arduboy2::playTone(frequency, duration, priority, duty_cycle);
}
void MyArduboy2::playScore(const byte* score, std::uint8_t priority, std::int8_t pitch) {
    // ArduboyWorks 以较小数值表示更高优先级；移调是其私有扩展，
    // 当前公共 Playtune 先保持原谱音高。
    if (score_player.playing() && priority > score_priority) return;
    score_output_enabled = Arduboy2::isAudioEnabled();
    score_priority = priority;
    score_player.playScore(score, pitch);
}
void MyArduboy2::stopScore() { score_player.stopScore(); }

#ifdef ARDUBOYWORKS_HAS_STOP_TONE
void MyArduboy2::stopTone() { Arduboy2::stopTone(); }
#endif

#ifdef ARDUBOYWORKS_HAS_PLAY_WAVE
void MyArduboy2::playWave(std::uint16_t frequency, const byte* wave,
                          std::uint16_t samples, std::uint8_t priority) {
    Arduboy2::playWave(frequency, wave, samples, priority);
}
#endif
