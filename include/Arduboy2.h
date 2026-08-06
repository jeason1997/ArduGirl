#pragma once

#include "Arduino.h"
#include "Arduboy2Beep.h"
#include "EEPROM.h"
#include "Sprites.h"
#include "avr/eeprom.h"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#define WIDTH 128
#define HEIGHT 64
#define BLACK 0
#define WHITE 1
#define INVERT 2
#define CLEAR_BUFFER true

#define LEFT_BUTTON  0x01
#define RIGHT_BUTTON 0x02
#define UP_BUTTON    0x04
#define DOWN_BUTTON  0x08
#define A_BUTTON     0x10
#define B_BUTTON     0x20
#define ARDUBOY_LIB_VER 60000
#define EEPROM_STORAGE_SPACE_START 16
#define PIN_SPEAKER_1 0
#define PIN_SPEAKER_2 0
#define OLED_PIXELS_NORMAL 0xA6
#define OLED_PIXELS_INVERTED 0xA7

struct Point {
    constexpr Point(std::int16_t x_value = 0, std::int16_t y_value = 0) noexcept
        : x(x_value), y(y_value) {}
    std::int16_t x;
    std::int16_t y;
};
struct Rect {
    constexpr Rect(std::int16_t x_value = 0, std::int16_t y_value = 0,
                   std::uint8_t width_value = 0, std::uint8_t height_value = 0) noexcept
        : x(x_value), y(y_value), width(width_value), height(height_value) {}
    std::int16_t x;
    std::int16_t y;
    std::uint8_t width;
    std::uint8_t height;
};

class Arduboy2Audio {
public:
    static bool enabled() noexcept;
    static void on() noexcept;
    static void off() noexcept;
    static void toggle() noexcept;
    static void saveOnOff() noexcept;
    static bool& enabledState() noexcept;
};

class Arduboy2 {
public:
    // 上游 Arduboy2Base 允许低开销绘图代码直接访问页面布局缓冲区；指针始终指向
    // ArduGirl 唯一的 1024 字节核心 framebuffer，不建立第二份画面存储。
    static std::uint8_t* sBuffer;

    class TuneControl {
    public:
        bool playing() const noexcept { return false; }
        void playScore(const byte*) noexcept {}
        void stopScore() noexcept {}
    };

    class AudioControl {
    public:
        class EnabledState {
        public:
            operator bool&() noexcept { return Arduboy2Audio::enabledState(); }
            operator const bool&() const noexcept { return Arduboy2Audio::enabledState(); }
            bool operator()() const noexcept { return Arduboy2Audio::enabled(); }
        } enabled;

        void on() noexcept { Arduboy2Audio::on(); }
        void off() noexcept { Arduboy2Audio::off(); }
        void begin() noexcept {}
        void toggle() noexcept { Arduboy2Audio::toggle(); }
        void saveOnOff() noexcept;
    };

    AudioControl audio;
    TuneControl tunes;
    std::uint16_t frameCount = 0;

    void begin() noexcept;
    void beginNoLogo() noexcept { begin(); }
    void boot() noexcept {}
    void blank() noexcept { clear(); }
    void flashlight() noexcept {}
    void systemButtons() noexcept {}
    void exitToBootloader() noexcept {}
    void setFrameRate(std::uint8_t rate) noexcept;
    bool nextFrame() noexcept;
    bool everyXFrames(std::uint8_t frames) const noexcept {
        return frames != 0 && frameCount % frames == 0;
    }
    void clear() noexcept;
    void fillScreen(std::uint8_t color = WHITE) noexcept;
    void display(bool clear_buffer = false) noexcept;
    void setCursor(std::int16_t x, std::int16_t y) noexcept;
    std::size_t print(const char* text) noexcept;
    std::size_t print(const __FlashStringHelper* text) noexcept {
        return print(reinterpret_cast<const char*>(text));
    }
    std::size_t println() noexcept { return write('\n'); }
    template<typename Value>
    std::size_t println(Value value) noexcept { return print(value) + write('\n'); }
    template<typename Integer,
             typename = std::enable_if_t<std::is_integral_v<Integer>>>
    std::size_t print(Integer value) noexcept {
        char buffer[24]{};
        const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
        if (result.ec != std::errc{}) {
            return 0;
        }
        *result.ptr = '\0';
        return print(buffer);
    }
    std::size_t write(std::uint8_t character) noexcept;
    bool pressed(std::uint8_t buttons) const noexcept;
    bool notPressed(std::uint8_t buttons) const noexcept;
    void pollButtons() noexcept;
    bool justPressed(std::uint8_t buttons) const noexcept;
    bool justReleased(std::uint8_t buttons) const noexcept;
    std::uint8_t buttonsState() const noexcept;
    std::uint8_t* getBuffer() noexcept;
    std::int16_t getCursorX() const noexcept { return cursor_x; }
    std::int16_t getCursorY() const noexcept { return cursor_y; }
    void setRGBled(std::uint8_t, std::uint8_t, std::uint8_t) noexcept {}
    void sendLCDCommand(std::uint8_t) noexcept {}
    void setTextSize(std::uint8_t size) noexcept { textSize = size == 0 ? 1 : size; }
    std::uint8_t getTextSize() const noexcept { return textSize; }
    void setTextWrap(bool wrap) noexcept { textWrap = wrap; }
    bool getTextWrap() const noexcept { return textWrap; }
    std::uint8_t getTextColor() const noexcept { return text_color_; }
    std::uint8_t getTextBackground() const noexcept { return text_background_; }
    void initAudio(std::uint8_t channels) noexcept;
    void closeAudio() noexcept;
    bool isAudioEnabled() const noexcept;
    void setAudioEnabled(bool enabled) noexcept;
    void toggleAudioEnabled() noexcept;
    void saveAudioOnOff() noexcept;
    void playTone(std::uint16_t frequency, std::uint16_t duration,
                  std::uint8_t priority = 0xFF, std::uint8_t duty_cycle = 2) noexcept;
    void stopTone() noexcept;
    void playScore(const byte* score, std::uint8_t priority = 0,
                   std::int8_t pitch = 0) noexcept;
    void playWave(std::uint16_t frequency, const byte* wave,
                  std::uint16_t samples, std::uint8_t priority = 0) noexcept;
    void stopScore() noexcept;
    static bool collide(std::int16_t x1, std::int16_t y1, std::uint8_t w1, std::uint8_t h1,
                        std::int16_t x2, std::int16_t y2, std::uint8_t w2, std::uint8_t h2) noexcept;
    static bool collide(Point point, Rect rect) noexcept {
        return point.x >= rect.x && point.x < rect.x + rect.width &&
               point.y >= rect.y && point.y < rect.y + rect.height;
    }
    static bool collide(Rect first, Rect second) noexcept {
        return collide(first.x, first.y, first.width, first.height,
                       second.x, second.y, second.width, second.height);
    }

    void drawPixel(std::int16_t x, std::int16_t y,
                   std::uint8_t color = WHITE) noexcept;
    void drawLine(std::int16_t x0, std::int16_t y0,
                  std::int16_t x1, std::int16_t y1,
                  std::uint8_t color = WHITE) noexcept;
    void drawFastHLine(std::int16_t x, std::int16_t y,
                       std::uint8_t width,
                       std::uint8_t color = WHITE) noexcept;
    void drawFastVLine(std::int16_t x, std::int16_t y,
                       std::uint8_t height,
                       std::uint8_t color = WHITE) noexcept;
    void drawRect(std::int16_t x, std::int16_t y,
                  std::uint8_t width, std::uint8_t height,
                  std::uint8_t color = WHITE) noexcept;
    void fillRect(std::int16_t x, std::int16_t y,
                  std::uint8_t width, std::uint8_t height,
                  std::uint8_t color = WHITE) noexcept;
    void drawCircle(std::int16_t x, std::int16_t y,
                    std::uint8_t radius,
                    std::uint8_t color = WHITE) noexcept;
    void fillCircle(std::int16_t x, std::int16_t y,
                    std::uint8_t radius,
                    std::uint8_t color = WHITE) noexcept;
    void drawTriangle(std::int16_t x0, std::int16_t y0,
                      std::int16_t x1, std::int16_t y1,
                      std::int16_t x2, std::int16_t y2,
                      std::uint8_t color = WHITE) noexcept;
    void fillTriangle(std::int16_t x0, std::int16_t y0,
                      std::int16_t x1, std::int16_t y1,
                      std::int16_t x2, std::int16_t y2,
                      std::uint8_t color = WHITE) noexcept;
    void drawRoundRect(std::int16_t x, std::int16_t y,
                       std::uint8_t width, std::uint8_t height,
                       std::uint8_t radius,
                       std::uint8_t color = WHITE) noexcept;
    void fillRoundRect(std::int16_t x, std::int16_t y,
                       std::uint8_t width, std::uint8_t height,
                       std::uint8_t radius,
                       std::uint8_t color = WHITE) noexcept;
    void drawBitmap(std::int16_t x, std::int16_t y,
                    const std::uint8_t* bitmap,
                    std::uint8_t width, std::uint8_t height,
                    std::uint8_t color = WHITE) noexcept;
    void drawSlowXYBitmap(std::int16_t x, std::int16_t y,
                          const std::uint8_t* bitmap,
                          std::uint8_t width, std::uint8_t height,
                          std::uint8_t color = WHITE) noexcept;
    void drawCompressed(std::int16_t x, std::int16_t y,
                        const std::uint8_t* bitmap,
                        std::uint8_t color = WHITE) noexcept;
    void setTextColor(std::uint8_t color) noexcept;
    void setTextBackground(std::uint8_t color) noexcept;

protected:
    std::int16_t cursor_x = 0;
    std::int16_t cursor_y = 0;
    std::uint8_t textSize = 1;
    bool textWrap = true;
    std::uint8_t textColor = WHITE;
    std::uint8_t textBackground = BLACK;

private:
    void drawCharacter(char character) noexcept;

    std::uint8_t frame_rate_ = 60;
    std::uint8_t text_color_ = WHITE;
    std::uint8_t text_background_ = BLACK;
    std::uint8_t current_buttons_ = 0;
    std::uint8_t previous_buttons_ = 0;
    std::uint8_t audio_channels_ = 0;
    std::uint32_t tone_end_ms_ = 0;
};

// 当前兼容实现没有 Print 代码体积裁剪需求，因此 Base 与完整类共享同一实现，
// 同时保持上游源码使用 Arduboy2Base 名称和静态缓冲区入口的契约。
using Arduboy2Base = Arduboy2;
