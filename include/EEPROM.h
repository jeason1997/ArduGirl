#pragma once

#include <cstddef>
#include <cstdint>

class EEPROMClass {
public:
    std::uint8_t read(int address) const noexcept;
    void write(int address, std::uint8_t value) noexcept;
    void update(int address, std::uint8_t value) noexcept;
    int length() const noexcept;
    std::uint8_t operator[](int address) const noexcept { return read(address); }
};

extern EEPROMClass EEPROM;
