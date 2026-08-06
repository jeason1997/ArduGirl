#include "ATMlib.h"

#include "Arduboy2.h"
#include "ardugirl/platform.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {

constexpr std::array<std::uint16_t, 64> kNoteTable{
    0, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
    523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
    1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760,
    1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136,
    3322, 3520, 3729, 3951, 4186, 4435, 4699, 4978, 5274, 5588,
    5920, 6272, 6645, 7040, 7459, 7902, 8372, 8870, 9397,
};

struct Channel {
    const byte* pointer = nullptr;
    byte note = 0;
    std::array<std::uint16_t, 7> stack_pointer{};
    std::array<byte, 7> stack_counter{};
    std::array<byte, 7> stack_track{};
    byte stack_index = 0;
    byte repeat_point = 0;
    std::uint16_t delay = 0;
    byte counter = 0;
    byte track = 0;
    std::uint16_t frequency = 0;
    byte volume = 0;
    std::int8_t slide = 0;
    byte slide_config = 0;
    byte slide_count = 0;
    byte arpeggio_notes = 0;
    byte arpeggio_timing = 0;
    byte arpeggio_count = 0;
    byte retrigger_config = 0;
    byte retrigger_count = 0;
    std::int8_t transpose = 0;
    byte modulation_depth = 0;
    byte modulation_config = 0;
    byte modulation_count = 0;
    std::int8_t glissando_config = 0;
    byte glissando_count = 0;
};

std::array<Channel, 4> channels{};
const byte* track_offsets = nullptr;
const byte* track_base = nullptr;
byte track_count = 0;
byte tick_rate = 25;
byte active_mask = 0;
byte mute_mask = 0;
bool playing = false;
bool paused = false;
std::uint32_t next_tick_ms = 0;

std::uint16_t read_vle(const byte*& pointer) noexcept {
    std::uint16_t value = 0;
    byte current = 0;
    do {
        value = static_cast<std::uint16_t>((value << 7u) | ((current = *pointer++) & 0x7Fu));
    } while ((current & 0x80u) != 0);
    return value;
}

const byte* track_pointer(byte track) noexcept {
    if (track_offsets == nullptr || track >= track_count) return track_base;
    const auto offset_index = static_cast<std::size_t>(track) * 2u;
    const auto offset = static_cast<std::uint16_t>(track_offsets[offset_index]) |
                        (static_cast<std::uint16_t>(track_offsets[offset_index + 1u]) << 8u);
    return track_base + offset;
}

std::uint16_t note_frequency(int note) noexcept {
    if (note < 0) note = 0;
    if (note > 63) note = 63;
    return kNoteTable[static_cast<std::size_t>(note)];
}

void publish_audio() noexcept {
    std::array<ardugirl::platform::SynthVoice, 4> voices{};
    if (playing && !paused && Arduboy2Audio::enabled()) {
        for (std::size_t index = 0; index < channels.size(); ++index) {
            if ((active_mask & (1u << index)) != 0 && (mute_mask & (1u << index)) == 0) {
                voices[index].frequency_hz = channels[index].frequency;
                voices[index].volume = channels[index].volume;
                voices[index].waveform = static_cast<std::uint8_t>(index);
            }
        }
    }
    ardugirl::platform::set_synth(voices.data(), static_cast<std::uint8_t>(voices.size()));
}

void stop_channel(std::size_t index, Channel& channel) noexcept {
    active_mask &= static_cast<byte>(~(1u << index));
    channel.volume = 0;
    channel.delay = 0xFFFFu;
}

void apply_effect(std::size_t index, Channel& channel, byte effect) noexcept {
    switch (effect) {
    case 0: channel.volume = *channel.pointer++; channel.retrigger_count = channel.volume; break;
    case 1: case 4: channel.slide = static_cast<std::int8_t>(*channel.pointer++);
        channel.slide_config = effect == 1 ? 0 : 0x40; break;
    case 2: case 5: channel.slide = static_cast<std::int8_t>(*channel.pointer++);
        channel.slide_config = *channel.pointer++; break;
    case 3: case 6: channel.slide = 0; break;
    case 7: channel.arpeggio_notes = *channel.pointer++;
        channel.arpeggio_timing = *channel.pointer++; break;
    case 8: channel.arpeggio_notes = 0; break;
    case 9: channel.retrigger_config = *channel.pointer++; break;
    case 10: channel.retrigger_config = 0; break;
    case 11: channel.transpose = static_cast<std::int8_t>(channel.transpose +
        static_cast<std::int8_t>(*channel.pointer++)); break;
    case 12: channel.transpose = static_cast<std::int8_t>(*channel.pointer++); break;
    case 13: channel.transpose = 0; break;
    case 14: case 16: channel.modulation_depth = *channel.pointer++;
        channel.modulation_config = static_cast<byte>(*channel.pointer++ + (effect == 16 ? 0x40 : 0)); break;
    case 15: case 17: channel.modulation_depth = 0; break;
    case 18: channel.glissando_config = static_cast<std::int8_t>(*channel.pointer++); break;
    case 19: channel.glissando_config = 0; break;
    case 20: channel.arpeggio_notes = 0xFF; channel.arpeggio_timing = *channel.pointer++; break;
    case 21: channel.arpeggio_notes = 0; break;
    case 92: tick_rate = static_cast<byte>(tick_rate + *channel.pointer++); break;
    case 93: tick_rate = *channel.pointer++; break;
    case 94:
        for (auto& item : channels) item.repeat_point = *channel.pointer++;
        break;
    case 95: stop_channel(index, channel); break;
    default: break;
    }
}

void update_modulation(Channel& channel) noexcept {
    if (channel.glissando_config != 0 &&
        channel.glissando_count++ >= (static_cast<byte>(channel.glissando_config) & 0x7Fu)) {
        channel.note = static_cast<byte>(channel.note + (channel.glissando_config < 0 ? -1 : 1));
        channel.note = constrain(channel.note, 1, 63);
        channel.frequency = note_frequency(channel.note);
        channel.glissando_count = 0;
    }
    if (channel.slide != 0 && channel.slide_count++ >= (channel.slide_config & 0x3Fu)) {
        channel.slide_count = 0;
        if ((channel.slide_config & 0x40u) != 0) {
            auto value = static_cast<std::int32_t>(channel.frequency) + channel.slide;
            if ((channel.slide_config & 0x80u) == 0) value = constrain(value, 0, 9397);
            channel.frequency = static_cast<std::uint16_t>(value);
        } else {
            auto value = static_cast<int>(channel.volume) + channel.slide;
            if ((channel.slide_config & 0x80u) == 0) value = constrain(value, 0, 63);
            channel.volume = static_cast<byte>(value);
        }
    }
    if (channel.arpeggio_notes != 0 && channel.note != 0 &&
        (channel.arpeggio_count & 0x1Fu) >= (channel.arpeggio_timing & 0x1Fu)) {
        const auto phase = static_cast<byte>(channel.arpeggio_count & 0xE0u);
        channel.arpeggio_count = phase == 0 ? 0x20 :
            (phase == 0x20 && (channel.arpeggio_timing & 0x40u) == 0 &&
             channel.arpeggio_notes != 0xFF ? 0x40 : 0);
        int note = channel.note;
        if ((channel.arpeggio_count & 0xE0u) != 0)
            note = channel.arpeggio_notes == 0xFF ? 0 : note + (channel.arpeggio_notes >> 4u);
        if ((channel.arpeggio_count & 0xE0u) == 0x40u) note += channel.arpeggio_notes & 0x0Fu;
        channel.frequency = note_frequency(note + channel.transpose);
    } else if (channel.arpeggio_notes != 0 && channel.note != 0) {
        ++channel.arpeggio_count;
    }
}

void interpret_channel(std::size_t index, Channel& channel) noexcept {
    if (channel.delay != 0) {
        if (channel.delay != 0xFFFFu) --channel.delay;
        return;
    }
    do {
        const byte command = *channel.pointer++;
        if (command < 64) {
            channel.note = command;
            channel.frequency = note_frequency(static_cast<int>(command) + channel.transpose);
            if (channel.slide_config == 0) channel.volume = channel.retrigger_count;
            if ((channel.arpeggio_timing & 0x20u) != 0) channel.arpeggio_count = 0;
        } else if (command < 160) {
            apply_effect(index, channel, static_cast<byte>(command - 64));
        } else if (command < 224) {
            channel.delay = static_cast<std::uint16_t>(command - 159);
        } else if (command == 224) {
            channel.delay = static_cast<std::uint16_t>(read_vle(channel.pointer) + 65);
        } else if (command == 252 || command == 253) {
            const byte counter = command == 252 ? 0 : *channel.pointer++;
            const byte track = *channel.pointer++;
            if (track != channel.track && channel.stack_index < channel.stack_pointer.size()) {
                const auto stack = channel.stack_index++;
                channel.stack_counter[stack] = channel.counter;
                channel.stack_track[stack] = channel.track;
                channel.stack_pointer[stack] = static_cast<std::uint16_t>(channel.pointer - track_base);
                channel.track = track;
            }
            channel.counter = counter;
            channel.pointer = track_pointer(channel.track);
        } else if (command == 254) {
            if (channel.counter != 0 || channel.stack_index == 0) {
                if (channel.counter != 0) --channel.counter;
                channel.pointer = track_pointer(channel.track);
            } else {
                const auto stack = --channel.stack_index;
                channel.pointer = track_base + channel.stack_pointer[stack];
                channel.counter = channel.stack_counter[stack];
                channel.track = channel.stack_track[stack];
            }
        } else if (command == 255) {
            channel.pointer += read_vle(channel.pointer);
        }
    } while (channel.delay == 0 && (active_mask & (1u << index)) != 0);
    if (channel.delay != 0 && channel.delay != 0xFFFFu) --channel.delay;
}

void tick() noexcept {
    for (std::size_t index = 0; index < channels.size(); ++index) {
        auto& channel = channels[index];
        if ((active_mask & (1u << index)) == 0) continue;
        update_modulation(channel);
        interpret_channel(index, channel);
    }
    if (active_mask == 0) {
        byte repeats = 0;
        for (const auto& channel : channels) repeats |= channel.repeat_point;
        if (repeats != 0) {
            for (auto& channel : channels) {
                channel.pointer = track_pointer(channel.repeat_point);
                channel.delay = 0;
            }
            active_mask = 0x0F;
        } else {
            playing = false;
        }
    }
    publish_audio();
}

} // 匿名命名空间

void ATMsynth::play(const byte* song) noexcept {
    if (song == nullptr) return;
    channels = {};
    mute_mask = 0;
    active_mask = 0x0F;
    track_count = *song++;
    track_offsets = song;
    song += static_cast<std::size_t>(track_count) * 2u;
    track_base = song + 4;
    for (std::size_t index = 0; index < channels.size(); ++index)
        channels[index].pointer = track_pointer(*song++);
    tick_rate = 25;
    playing = true;
    paused = false;
    next_tick_ms = millis();
    publish_audio();
}

void ATMsynth::playPause() noexcept { paused = !paused; publish_audio(); }

void ATMsynth::stop() noexcept {
    playing = false;
    paused = false;
    active_mask = 0;
    channels = {};
    publish_audio();
}

void ATMsynth::muteChannel(byte channel) noexcept {
    if (channel < 4) mute_mask |= static_cast<byte>(1u << channel);
    publish_audio();
}

void ATMsynth::unMuteChannel(byte channel) noexcept {
    if (channel < 4) mute_mask &= static_cast<byte>(~(1u << channel));
    publish_audio();
}

void ardugirl::atm::service() noexcept {
    if (!playing || paused || tick_rate == 0) return;
    const auto now = millis();
    const auto interval = static_cast<std::uint32_t>(1000u / tick_rate);
    std::uint8_t catch_up = 0;
    while (static_cast<std::int32_t>(now - next_tick_ms) >= 0 && catch_up++ < 8) {
        tick();
        next_tick_ms += interval == 0 ? 1 : interval;
    }
}
