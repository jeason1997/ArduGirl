#include "Arduboy2.h"

#include "ardugirl/framebuffer.hpp"
#include "ardugirl/platform.hpp"
#include "ardugirl/runtime.hpp"

#include <array>
#include <cstdlib>

namespace {

using Glyph = std::array<std::uint8_t, 5>;

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

std::uint32_t millis() noexcept { return ardugirl::platform::millis(); }
std::uint32_t micros() noexcept { return ardugirl::platform::millis() * 1000u; }
void randomSeed(unsigned long seed) noexcept { std::srand(static_cast<unsigned int>(seed)); }
long random(long maximum) noexcept { return maximum > 0 ? std::rand() % maximum : 0; }
long random(long minimum, long maximum) noexcept {
    return maximum > minimum ? minimum + random(maximum - minimum) : minimum;
}

void Arduboy2::AudioControl::saveOnOff() noexcept {
    EEPROM.update(EEPROM_STORAGE_SPACE_START, enabled ? 1 : 0);
}

void Arduboy2::begin() noexcept {
    clear();
    display();
}

void Arduboy2::setFrameRate(std::uint8_t rate) noexcept {
    frame_rate_ = rate;
}

bool Arduboy2::nextFrame() noexcept {
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

void Arduboy2::display() noexcept {
    ardugirl::display();
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
    // 当前 Linux 基线尚未接入音频输出；这里保留完整调用契约和状态门控，
    // 后续平台音频后端应在此统一转发，游戏适配层不得各自吞掉声音请求。
    (void) frequency;
    (void) duration;
    (void) priority;
    (void) duty_cycle;
}

void Arduboy2::stopTone() noexcept {}

void Arduboy2::playScore(const byte* score, std::uint8_t priority,
                         std::int8_t pitch) noexcept {
    (void) score;
    (void) priority;
    (void) pitch;
}

void Arduboy2::playWave(std::uint16_t frequency, const byte* wave,
                        std::uint16_t samples, std::uint8_t priority) noexcept {
    (void) frequency;
    (void) wave;
    (void) samples;
    (void) priority;
}

void Arduboy2::stopScore() noexcept {}

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
    const auto glyph = glyph_for(character);
    auto& screen = ardugirl::framebuffer();

    for (std::int16_t column = 0; column < 5; ++column) {
        for (std::int16_t row = 0; row < 8; ++row) {
            const auto mask = static_cast<std::uint8_t>(1u << row);
            const bool foreground =
                (glyph[static_cast<std::size_t>(column)] & mask) != 0;
            const auto color = foreground ? text_color_ : text_background_;
            if (text_color_ != text_background_ || foreground) {
                screen.set_pixel(static_cast<std::int16_t>(cursor_x + column),
                                 static_cast<std::int16_t>(cursor_y + row),
                                 color == WHITE);
            }
        }
    }
    if (text_color_ != text_background_) {
        for (std::int16_t row = 0; row < 8; ++row) {
            screen.set_pixel(static_cast<std::int16_t>(cursor_x + 5),
                             static_cast<std::int16_t>(cursor_y + row),
                             text_background_ == WHITE);
        }
    }
    cursor_x = static_cast<std::int16_t>(cursor_x + 6);
}
