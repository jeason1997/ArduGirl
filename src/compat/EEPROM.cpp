#include "EEPROM.h"
#include "ardugirl/platform.hpp"

bool valid_address(int address) noexcept {
    return address >= 0 && address < 1024;
}

EEPROMClass EEPROM;

std::uint8_t EEPROMClass::read(int address) const noexcept {
    std::uint8_t value = 0xFFu;
    if (valid_address(address)) {
        ardugirl::platform::storage_read(static_cast<std::uint16_t>(address), &value, 1);
    }
    return value;
}

void EEPROMClass::write(int address, std::uint8_t value) noexcept {
    if (valid_address(address)) {
        ardugirl::platform::storage_write(static_cast<std::uint16_t>(address), &value, 1);
    }
}

void EEPROMClass::update(int address, std::uint8_t value) noexcept {
    if (read(address) != value) {
        write(address, value);
    }
}

int EEPROMClass::length() const noexcept {
    return 1024;
}
