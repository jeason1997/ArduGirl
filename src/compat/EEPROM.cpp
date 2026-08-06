#include "EEPROM.h"
#include "avr/eeprom.h"
#include "ardugirl/platform.hpp"

bool valid_address(int address) noexcept {
    return address >= 0 && address < 1024;
}

EEPROMClass EEPROM;

namespace {
int eeprom_address(const void* address) noexcept {
    return static_cast<int>(reinterpret_cast<std::uintptr_t>(address));
}
}

std::uint8_t eeprom_read_byte(const std::uint8_t* address) noexcept {
    return EEPROM.read(eeprom_address(address));
}

std::uint16_t eeprom_read_word(const std::uint16_t* address) noexcept {
    const int base = eeprom_address(address);
    return static_cast<std::uint16_t>(EEPROM.read(base) |
           static_cast<std::uint16_t>(EEPROM.read(base + 1)) << 8);
}

std::uint32_t eeprom_read_dword(const std::uint32_t* address) noexcept {
    const int base = eeprom_address(address);
    std::uint32_t value = 0;
    for (int offset = 0; offset < 4; ++offset) {
        value |= static_cast<std::uint32_t>(EEPROM.read(base + offset)) << (offset * 8);
    }
    return value;
}

void eeprom_read_block(void* destination, const void* address, std::size_t size) noexcept {
    auto* bytes = static_cast<std::uint8_t*>(destination);
    const int base = eeprom_address(address);
    for (std::size_t offset = 0; offset < size; ++offset) bytes[offset] = EEPROM.read(base + static_cast<int>(offset));
}

void eeprom_write_byte(std::uint8_t* address, std::uint8_t value) noexcept {
    EEPROM.update(eeprom_address(address), value);
}

void eeprom_write_word(std::uint16_t* address, std::uint16_t value) noexcept {
    const int base = eeprom_address(address);
    EEPROM.update(base, static_cast<std::uint8_t>(value));
    EEPROM.update(base + 1, static_cast<std::uint8_t>(value >> 8));
}

void eeprom_write_dword(std::uint32_t* address, std::uint32_t value) noexcept {
    const int base = eeprom_address(address);
    for (int offset = 0; offset < 4; ++offset) EEPROM.update(base + offset, static_cast<std::uint8_t>(value >> (offset * 8)));
}

void eeprom_write_block(const void* source, void* address, std::size_t size) noexcept {
    const auto* bytes = static_cast<const std::uint8_t*>(source);
    const int base = eeprom_address(address);
    for (std::size_t offset = 0; offset < size; ++offset) EEPROM.update(base + static_cast<int>(offset), bytes[offset]);
}

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
