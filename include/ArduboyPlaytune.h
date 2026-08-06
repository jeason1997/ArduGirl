#pragma once

#include "Arduino.h"

class ArduboyPlaytune {
public:
    explicit ArduboyPlaytune(bool) noexcept {}
    void initChannel(std::uint8_t) noexcept {}
    bool playing() const noexcept { return false; }
    void playScore(const byte*) noexcept {}
    void stopScore() noexcept {}
};
