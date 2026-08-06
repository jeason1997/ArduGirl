#include "EEPROM.h"

#include <array>

namespace {

std::array<std::uint8_t, 1024> storage = [] {
    std::array<std::uint8_t, 1024> data{};
    data.fill(0xFFu);
    return data;
}();

bool valid_address(int address) noexcept {
    return address >= 0 && address < static_cast<int>(storage.size());
}

} // 匿名命名空间

EEPROMClass EEPROM;

std::uint8_t EEPROMClass::read(int address) const noexcept {
    return valid_address(address) ? storage[static_cast<std::size_t>(address)] : 0xFFu;
}

void EEPROMClass::write(int address, std::uint8_t value) noexcept {
    if (valid_address(address)) {
        storage[static_cast<std::size_t>(address)] = value;
    }
}

void EEPROMClass::update(int address, std::uint8_t value) noexcept {
    if (read(address) != value) {
        write(address, value);
    }
}

int EEPROMClass::length() const noexcept {
    return static_cast<int>(storage.size());
}

