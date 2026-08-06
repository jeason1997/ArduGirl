#include "Arduboy2.h"
#include "ArduboyPlaytune.h"

#include "ardugirl/framebuffer.hpp"
#include "ardugirl/platform.hpp"
#include "ardugirl/runtime.hpp"

#include <array>
#include <cstdlib>

namespace {

using Glyph = std::array<std::uint8_t, 5>;

bool legacy_score_enabled = true;
ArduboyPlaytune legacy_score_player(legacy_score_enabled);
std::uint8_t legacy_score_priority = 0xFF;

struct LegacyScoreChannels {
    LegacyScoreChannels() noexcept {
        legacy_score_player.initChannel(PIN_SPEAKER_1);
        legacy_score_player.initChannel(PIN_SPEAKER_2);
    }
} legacy_score_channels;

Glyph glyph_for(char character) noexcept {
    // 字体数据在构建时从固定版本的 Arduboy2 上游文件提取，既保证字符行为
    // 与官方库一致，也避免在 ArduGirl 中维护另一份容易漂移的字体副本。
    static constexpr std::uint8_t font[] = {
#include "font5x7.inc"
    };
    const auto index = static_cast<std::size_t>(static_cast<std::uint8_t>(character)) * 5u;
    if (index + 5u > sizeof(font)) {
        return {0x7F, 0x41, 0x41, 0x41, 0x7F};
    }
    return {font[index], font[index + 1], font[index + 2],
            font[index + 3], font[index + 4]};
}

bool color_value(std::uint8_t color, bool current) noexcept {
    if (color == INVERT) {
        return !current;
    }
    return color == WHITE;
}

} // 匿名命名空间

std::uint8_t* Arduboy2::sBuffer = ardugirl::framebuffer().data().data();

std::uint32_t millis() noexcept { return ardugirl::platform::millis(); }
std::uint32_t micros() noexcept { return ardugirl::platform::micros(); }
void delay(unsigned long duration) noexcept {
    ardugirl::platform::sleep_ms(static_cast<std::uint32_t>(duration));
}
void randomSeed(unsigned long seed) noexcept { std::srand(static_cast<unsigned int>(seed)); }
long random(long maximum) noexcept { return maximum > 0 ? std::rand() % maximum : 0; }
long random(long minimum, long maximum) noexcept {
    return maximum > minimum ? minimum + random(maximum - minimum) : minimum;
}
long map(long value, long from_low, long from_high,
         long to_low, long to_high) noexcept {
    if (from_high == from_low) return to_low;
    return (value - from_low) * (to_high - to_low) /
           (from_high - from_low) + to_low;
}

void Arduboy2::AudioControl::saveOnOff() noexcept {
    EEPROM.update(EEPROM_STORAGE_SPACE_START, enabled ? 1 : 0);
}

bool& Arduboy2Audio::enabledState() noexcept {
    static bool state = true;
    return state;
}

bool Arduboy2Audio::enabled() noexcept { return enabledState(); }
void Arduboy2Audio::on() noexcept { enabledState() = true; }
void Arduboy2Audio::off() noexcept { enabledState() = false; }
void Arduboy2Audio::toggle() noexcept { enabledState() = !enabledState(); }
void Arduboy2Audio::saveOnOff() noexcept {
    EEPROM.update(EEPROM_STORAGE_SPACE_START, enabled() ? 1 : 0);
}

void Arduboy2::begin() noexcept {
    clear();
    display();
}

void Arduboy2::setFrameRate(std::uint8_t rate) noexcept {
    frame_rate_ = rate;
}

bool Arduboy2::nextFrame() noexcept {
    if (tone_end_ms_ != 0 &&
        static_cast<std::int32_t>(millis() - tone_end_ms_) >= 0) {
        stopTone();
    }
    const bool ready = ardugirl::next_frame(frame_rate_);
    if (ready) {
        ++frameCount;
    }
    return ready;
}

void Arduboy2::clear() noexcept {
    ardugirl::framebuffer().clear();
    cursor_x = 0;
    cursor_y = 0;
}

void Arduboy2::fillScreen(std::uint8_t color) noexcept {
    ardugirl::framebuffer().data().fill(color == BLACK ? 0x00 : 0xFF);
}

void Arduboy2::display(bool clear_buffer) noexcept {
    ardugirl::display();
    if (clear_buffer) clear();
}

void Arduboy2::setCursor(std::int16_t x, std::int16_t y) noexcept {
    cursor_x = x;
    cursor_y = y;
}

std::size_t Arduboy2::print(const char* text) noexcept {
    if (text == nullptr) {
        return 0;
    }

    std::size_t written = 0;
    while (*text != '\0') {
        drawCharacter(*text++);
        ++written;
    }
    return written;
}

bool Arduboy2::pressed(std::uint8_t requested) const noexcept {
    return (ardugirl::buttons() & requested) == requested;
}

bool Arduboy2::notPressed(std::uint8_t requested) const noexcept {
    return (ardugirl::buttons() & requested) == 0;
}

void Arduboy2::pollButtons() noexcept {
    previous_buttons_ = current_buttons_;
    current_buttons_ = ardugirl::buttons();
}

bool Arduboy2::justPressed(std::uint8_t requested) const noexcept {
    return (current_buttons_ & requested) != 0 &&
           (previous_buttons_ & requested) == 0;
}

bool Arduboy2::justReleased(std::uint8_t requested) const noexcept {
    return (current_buttons_ & requested) == 0 &&
           (previous_buttons_ & requested) != 0;
}

std::uint8_t Arduboy2::buttonsState() const noexcept { return ardugirl::buttons(); }

std::uint8_t* Arduboy2::getBuffer() noexcept {
    return ardugirl::framebuffer().data().data();
}

bool Arduboy2::collide(std::int16_t x1, std::int16_t y1, std::uint8_t w1, std::uint8_t h1,
                       std::int16_t x2, std::int16_t y2, std::uint8_t w2, std::uint8_t h2) noexcept {
    return x1 < x2 + w2 && x2 < x1 + w1 && y1 < y2 + h2 && y2 < y1 + h1;
}

void Arduboy2::initAudio(std::uint8_t channels) noexcept {
    audio_channels_ = channels;
}

void Arduboy2::closeAudio() noexcept {
    audio_channels_ = 0;
    stopTone();
    ardugirl::platform::stop_wave();
    legacy_score_player.stopScore();
}

bool Arduboy2::isAudioEnabled() const noexcept {
    return audio.enabled();
}

void Arduboy2::setAudioEnabled(bool enabled) noexcept {
    enabled ? audio.on() : audio.off();
}

void Arduboy2::toggleAudioEnabled() noexcept {
    audio.toggle();
}

void Arduboy2::saveAudioOnOff() noexcept {
    audio.saveOnOff();
}

void Arduboy2::playTone(std::uint16_t frequency, std::uint16_t duration,
                        std::uint8_t priority, std::uint8_t duty_cycle) noexcept {
    // 音频请求统一转发给平台后端，游戏适配层不得各自复制输出实现。
    if (audio.enabled() && frequency != 0 && duration != 0) {
        ardugirl::platform::set_tone(frequency);
        tone_end_ms_ = millis() + duration;
    }
    (void) duration;
    (void) priority;
    (void) duty_cycle;
}

void Arduboy2::stopTone() noexcept {
    tone_end_ms_ = 0;
    ardugirl::platform::stop_tone();
}

void Arduboy2::playScore(const byte* score, std::uint8_t priority,
                         std::int8_t pitch) noexcept {
    if (legacy_score_player.playing() && priority > legacy_score_priority) return;
    legacy_score_enabled = audio.enabled();
    legacy_score_priority = priority;
    legacy_score_player.playScore(score, pitch);
}

void Arduboy2::playWave(std::uint16_t frequency, const byte* wave,
                        std::uint16_t samples, std::uint8_t priority) noexcept {
    if (audio.enabled() && audio_channels_ != 0) {
        ardugirl::platform::play_wave(frequency, wave, samples);
    }
    (void) priority;
}

void Arduboy2::stopScore() noexcept { legacy_score_player.stopScore(); }

void Arduboy2::drawPixel(std::int16_t x, std::int16_t y,
                         std::uint8_t color) noexcept {
    auto& screen = ardugirl::framebuffer();
    screen.set_pixel(x, y, color_value(color, screen.pixel(x, y)));
}

void Arduboy2::drawLine(std::int16_t x0, std::int16_t y0,
                        std::int16_t x1, std::int16_t y1,
                        std::uint8_t color) noexcept {
    if (color == INVERT) {
        // 反色直线需要逐像素读取旧值，不能直接调用只接受布尔颜色的核心函数。
        const auto dx = static_cast<std::int16_t>(std::abs(x1 - x0));
        const auto sx = static_cast<std::int16_t>(x0 < x1 ? 1 : -1);
        const auto dy = static_cast<std::int16_t>(-std::abs(y1 - y0));
        const auto sy = static_cast<std::int16_t>(y0 < y1 ? 1 : -1);
        auto error = static_cast<std::int16_t>(dx + dy);
        while (true) {
            drawPixel(x0, y0, INVERT);
            if (x0 == x1 && y0 == y1) break;
            const auto twice = static_cast<std::int16_t>(error * 2);
            if (twice >= dy) { error = static_cast<std::int16_t>(error + dy); x0 += sx; }
            if (twice <= dx) { error = static_cast<std::int16_t>(error + dx); y0 += sy; }
        }
        return;
    }
    ardugirl::framebuffer().draw_line(x0, y0, x1, y1, color == WHITE);
}

void Arduboy2::drawFastHLine(std::int16_t x, std::int16_t y,
                             std::uint8_t width, std::uint8_t color) noexcept {
    for (std::uint8_t offset = 0; offset < width; ++offset) {
        drawPixel(static_cast<std::int16_t>(x + offset), y, color);
    }
}

void Arduboy2::drawFastVLine(std::int16_t x, std::int16_t y,
                             std::uint8_t height, std::uint8_t color) noexcept {
    for (std::uint8_t offset = 0; offset < height; ++offset) {
        drawPixel(x, static_cast<std::int16_t>(y + offset), color);
    }
}

void Arduboy2::drawRect(std::int16_t x, std::int16_t y,
                        std::uint8_t width, std::uint8_t height,
                        std::uint8_t color) noexcept {
    if (width == 0 || height == 0) return;
    drawFastHLine(x, y, width, color);
    drawFastHLine(x, static_cast<std::int16_t>(y + height - 1), width, color);
    drawFastVLine(x, y, height, color);
    drawFastVLine(static_cast<std::int16_t>(x + width - 1), y, height, color);
}

void Arduboy2::fillRect(std::int16_t x, std::int16_t y,
                        std::uint8_t width, std::uint8_t height,
                        std::uint8_t color) noexcept {
    for (std::uint8_t row = 0; row < height; ++row) {
        drawFastHLine(x, static_cast<std::int16_t>(y + row), width, color);
    }
}

void Arduboy2::drawCircle(std::int16_t x0, std::int16_t y0,
                          std::uint8_t radius, std::uint8_t color) noexcept {
    std::int16_t x = radius;
    std::int16_t y = 0;
    std::int16_t error = 1 - x;
    while (x >= y) {
        drawPixel(x0 + x, y0 + y, color); drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color); drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 - x, y0 - y, color); drawPixel(x0 - y, y0 - x, color);
        drawPixel(x0 + y, y0 - x, color); drawPixel(x0 + x, y0 - y, color);
        ++y;
        if (error < 0) {
            error = static_cast<std::int16_t>(error + 2 * y + 1);
        } else {
            --x;
            error = static_cast<std::int16_t>(error + 2 * (y - x) + 1);
        }
    }
}

void Arduboy2::fillCircle(std::int16_t x0, std::int16_t y0,
                          std::uint8_t radius, std::uint8_t color) noexcept {
    for (std::int16_t y = -radius; y <= radius; ++y) {
        for (std::int16_t x = -radius; x <= radius; ++x) {
            if (x * x + y * y <= radius * radius) {
                drawPixel(x0 + x, y0 + y, color);
            }
        }
    }
}

void Arduboy2::drawTriangle(std::int16_t x0, std::int16_t y0,
                            std::int16_t x1, std::int16_t y1,
                            std::int16_t x2, std::int16_t y2,
                            std::uint8_t color) noexcept {
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

void Arduboy2::fillTriangle(std::int16_t x0, std::int16_t y0,
                            std::int16_t x1, std::int16_t y1,
                            std::int16_t x2, std::int16_t y2,
                            std::uint8_t color) noexcept {
    const auto min_x = min(x0, min(x1, x2));
    const auto max_x = max(x0, max(x1, x2));
    const auto min_y = min(y0, min(y1, y2));
    const auto max_y = max(y0, max(y1, y2));
    const auto edge = [](std::int16_t ax, std::int16_t ay, std::int16_t bx,
                         std::int16_t by, std::int16_t px, std::int16_t py) {
        return static_cast<std::int32_t>(px - ax) * (by - ay) -
               static_cast<std::int32_t>(py - ay) * (bx - ax);
    };
    const auto orientation = edge(x0, y0, x1, y1, x2, y2);
    for (auto y = min_y; y <= max_y; ++y) {
        for (auto x = min_x; x <= max_x; ++x) {
            const auto a = edge(x0, y0, x1, y1, x, y);
            const auto b = edge(x1, y1, x2, y2, x, y);
            const auto c = edge(x2, y2, x0, y0, x, y);
            if ((orientation >= 0 && a >= 0 && b >= 0 && c >= 0) ||
                (orientation < 0 && a <= 0 && b <= 0 && c <= 0)) drawPixel(x, y, color);
        }
    }
}

void Arduboy2::drawRoundRect(std::int16_t x, std::int16_t y,
                             std::uint8_t width, std::uint8_t height,
                             std::uint8_t radius, std::uint8_t color) noexcept {
    if (width == 0 || height == 0) return;
    radius = min(radius, static_cast<std::uint8_t>(min(width, height) / 2));
    drawFastHLine(x + radius, y, width - radius * 2, color);
    drawFastHLine(x + radius, y + height - 1, width - radius * 2, color);
    drawFastVLine(x, y + radius, height - radius * 2, color);
    drawFastVLine(x + width - 1, y + radius, height - radius * 2, color);
    for (std::int16_t dy = 0; dy <= radius; ++dy) {
        for (std::int16_t dx = 0; dx <= radius; ++dx) {
            const auto distance = (radius - dx) * (radius - dx) +
                                  (radius - dy) * (radius - dy);
            if (distance <= radius * radius && distance > (radius - 1) * (radius - 1)) {
                drawPixel(x + dx, y + dy, color); drawPixel(x + width - 1 - dx, y + dy, color);
                drawPixel(x + dx, y + height - 1 - dy, color);
                drawPixel(x + width - 1 - dx, y + height - 1 - dy, color);
            }
        }
    }
}

void Arduboy2::fillRoundRect(std::int16_t x, std::int16_t y,
                             std::uint8_t width, std::uint8_t height,
                             std::uint8_t radius, std::uint8_t color) noexcept {
    radius = min(radius, static_cast<std::uint8_t>(min(width, height) / 2));
    fillRect(x + radius, y, width - radius * 2, height, color);
    for (std::int16_t row = 0; row < height; ++row) {
        const auto dy = row < radius ? radius - row :
                        row >= height - radius ? row - (height - radius - 1) : 0;
        std::int16_t inset = 0;
        while (inset < radius && inset * inset + dy * dy > radius * radius) ++inset;
        drawFastHLine(x + inset, y + row, width - inset * 2, color);
    }
}

void Arduboy2::drawBitmap(std::int16_t x, std::int16_t y,
                          const std::uint8_t* bitmap,
                          std::uint8_t width, std::uint8_t height,
                          std::uint8_t color) noexcept {
    if (bitmap == nullptr) return;
    for (std::uint8_t row = 0; row < height; ++row) {
        for (std::uint8_t column = 0; column < width; ++column) {
            const auto index = static_cast<std::size_t>(column) +
                               static_cast<std::size_t>(row / 8) * width;
            if ((bitmap[index] & static_cast<std::uint8_t>(1u << (row & 7))) != 0) {
                drawPixel(x + column, y + row, color);
            }
        }
    }
}

void Arduboy2::drawSlowXYBitmap(std::int16_t x, std::int16_t y,
                                const std::uint8_t* bitmap,
                                std::uint8_t width, std::uint8_t height,
                                std::uint8_t color) noexcept {
    if (bitmap == nullptr) return;
    const auto byte_width = static_cast<std::uint8_t>((width + 7u) / 8u);
    for (std::uint8_t row = 0; row < height; ++row)
        for (std::uint8_t column = 0; column < width; ++column)
            if ((pgm_read_byte(bitmap + row * byte_width + column / 8u) &
                 (0x80u >> (column & 7u))) != 0) drawPixel(x + column, y + row, color);
}

void Arduboy2::drawCompressed(std::int16_t x, std::int16_t y,
                              const std::uint8_t* bitmap,
                              std::uint8_t color) noexcept {
    if (bitmap == nullptr) return;
    std::size_t byte_index = 0;
    std::uint8_t bit_index = 0;
    auto read_bits = [&](std::uint8_t count) {
        std::uint16_t result = 0;
        for (std::uint8_t index = 0; index < count; ++index) {
            if ((pgm_read_byte(bitmap + byte_index) & (1u << bit_index)) != 0) result |= 1u << index;
            if (++bit_index == 8) { bit_index = 0; ++byte_index; }
        }
        return result;
    };
    const auto width = static_cast<std::uint16_t>(read_bits(8) + 1u);
    const auto height = static_cast<std::uint16_t>(read_bits(8) + 1u);
    bool span_color = read_bits(1) != 0;
    std::uint32_t position = 0;
    const auto total = static_cast<std::uint32_t>(width) * height;
    while (position < total) {
        std::uint8_t length_bits = 1;
        while (read_bits(1) == 0) length_bits = static_cast<std::uint8_t>(length_bits + 2);
        const auto length = static_cast<std::uint32_t>(read_bits(length_bits) + 1u);
        for (std::uint32_t index = 0; index < length && position < total; ++index, ++position) {
            if (span_color) {
                const auto column = static_cast<std::int16_t>((position / 8u) % width);
                const auto row = static_cast<std::int16_t>((position / (width * 8u)) * 8u + position % 8u);
                if (row < height) drawPixel(x + column, y + row, color);
            }
        }
        span_color = !span_color;
    }
}

void Arduboy2::setTextColor(std::uint8_t color) noexcept {
    text_color_ = color;
}

void Arduboy2::setTextBackground(std::uint8_t color) noexcept {
    text_background_ = color;
}

std::size_t Arduboy2::write(std::uint8_t character) noexcept {
    drawCharacter(static_cast<char>(character));
    return 1;
}

void Arduboy2::drawCharacter(char character) noexcept {
    if (character == '\r') return;
    if (character == '\n') {
        cursor_x = 0;
        cursor_y = static_cast<std::int16_t>(cursor_y + 8 * textSize);
        return;
    }
    if (textWrap && cursor_x + 6 * textSize > WIDTH) {
        cursor_x = 0;
        cursor_y = static_cast<std::int16_t>(cursor_y + 8 * textSize);
    }
    const auto glyph = glyph_for(character);
    auto& screen = ardugirl::framebuffer();

    for (std::int16_t column = 0; column < 5; ++column) {
        for (std::int16_t row = 0; row < 8; ++row) {
            const auto mask = static_cast<std::uint8_t>(1u << row);
            const bool foreground =
                (glyph[static_cast<std::size_t>(column)] & mask) != 0;
            const auto color = foreground ? text_color_ : text_background_;
            if (text_color_ != text_background_ || foreground)
                for (std::uint8_t sy = 0; sy < textSize; ++sy)
                    for (std::uint8_t sx = 0; sx < textSize; ++sx)
                        screen.set_pixel(cursor_x + column * textSize + sx,
                                         cursor_y + row * textSize + sy, color == WHITE);
        }
    }
    if (text_color_ != text_background_) {
        for (std::int16_t row = 0; row < 8; ++row) {
            screen.set_pixel(static_cast<std::int16_t>(cursor_x + 5),
                             static_cast<std::int16_t>(cursor_y + row),
                             text_background_ == WHITE);
        }
    }
    cursor_x = static_cast<std::int16_t>(cursor_x + 6 * textSize);
}
