#pragma once

#include "Arduino.h"

#include <cstddef>
#include <cstdint>

#define WIDTH 128
#define HEIGHT 64
#define BLACK 0
#define WHITE 1

#define LEFT_BUTTON  0x01
#define RIGHT_BUTTON 0x02
#define UP_BUTTON    0x04
#define DOWN_BUTTON  0x08
#define A_BUTTON     0x10
#define B_BUTTON     0x20

class Arduboy2 {
public:
    void begin() noexcept;
    void setFrameRate(std::uint8_t rate) noexcept;
    bool nextFrame() noexcept;
    void clear() noexcept;
    void display() noexcept;
    void setCursor(std::int16_t x, std::int16_t y) noexcept;
    std::size_t print(const char* text) noexcept;
    bool pressed(std::uint8_t buttons) const noexcept;

private:
    void drawCharacter(char character) noexcept;

    std::uint8_t frame_rate_ = 60;
    std::int16_t cursor_x_ = 0;
    std::int16_t cursor_y_ = 0;
};

