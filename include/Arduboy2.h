#pragma once

#include "Arduino.h"
#include "Arduboy2Beep.h"
#include "EEPROM.h"
#include "Sprites.h"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#define WIDTH 128
#define HEIGHT 64
#define BLACK 0
#define WHITE 1
#define INVERT 2

#define LEFT_BUTTON  0x01
#define RIGHT_BUTTON 0x02
#define UP_BUTTON    0x04
#define DOWN_BUTTON  0x08
#define A_BUTTON     0x10
#define B_BUTTON     0x20

class Arduboy2 {
public:
    class AudioControl {
    public:
        bool enabled() const noexcept { return enabled_; }
        void on() noexcept { enabled_ = true; }
        void off() noexcept { enabled_ = false; }

    private:
        bool enabled_ = true;
    };

    AudioControl audio;
    std::uint16_t frameCount = 0;

    void begin() noexcept;
    void setFrameRate(std::uint8_t rate) noexcept;
    bool nextFrame() noexcept;
    void clear() noexcept;
    void display() noexcept;
    void setCursor(std::int16_t x, std::int16_t y) noexcept;
    std::size_t print(const char* text) noexcept;
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
    void pollButtons() noexcept;
    bool justPressed(std::uint8_t buttons) const noexcept;
    bool justReleased(std::uint8_t buttons) const noexcept;

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
    void drawBitmap(std::int16_t x, std::int16_t y,
                    const std::uint8_t* bitmap,
                    std::uint8_t width, std::uint8_t height,
                    std::uint8_t color = WHITE) noexcept;
    void setTextColor(std::uint8_t color) noexcept;
    void setTextBackground(std::uint8_t color) noexcept;

private:
    void drawCharacter(char character) noexcept;

    std::uint8_t frame_rate_ = 60;
    std::int16_t cursor_x_ = 0;
    std::int16_t cursor_y_ = 0;
    std::uint8_t text_color_ = WHITE;
    std::uint8_t text_background_ = BLACK;
    std::uint8_t current_buttons_ = 0;
    std::uint8_t previous_buttons_ = 0;
};
