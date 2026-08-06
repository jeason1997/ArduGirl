#pragma once

#include "EEPROM.h"

#include <cstddef>
#include <cstdint>

inline void eeprom_busy_wait() noexcept {}
std::uint8_t eeprom_read_byte(const std::uint8_t* address) noexcept;
std::uint16_t eeprom_read_word(const std::uint16_t* address) noexcept;
std::uint32_t eeprom_read_dword(const std::uint32_t* address) noexcept;
void eeprom_read_block(void* destination, const void* address, std::size_t size) noexcept;
void eeprom_write_byte(std::uint8_t* address, std::uint8_t value) noexcept;
void eeprom_write_word(std::uint16_t* address, std::uint16_t value) noexcept;
void eeprom_write_dword(std::uint32_t* address, std::uint32_t value) noexcept;
void eeprom_write_block(const void* source, void* address, std::size_t size) noexcept;
