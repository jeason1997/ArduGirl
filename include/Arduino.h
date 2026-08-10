#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cctype>
#include <cstdarg>
#include <type_traits>

using byte = std::uint8_t;
using boolean = bool;
class __FlashStringHelper {};
using PGM_P = const char*;

#ifndef BLACK
#define BLACK 0
#define WHITE 1
#define INVERT 2
#endif

#define ARDUBOY_NO_USB

// 非 AVR 平台使用统一地址空间，字符串常量不需要放入独立的程序存储区。
#define F(text) (reinterpret_cast<const __FlashStringHelper*>(text))
#define PROGMEM
#define PSTR(text) (text)

inline std::uint8_t pgm_read_byte(const void* address) noexcept {
    return *static_cast<const std::uint8_t*>(address);
}

// 统一地址空间没有 AVR 的 near/far 区别，保留旧名称以兼容早期游戏源码。
inline std::uint8_t pgm_read_byte_near(const void* address) noexcept {
    return pgm_read_byte(address);
}

template<typename Type>
inline Type pgm_read_word(const Type* address) noexcept {
    Type value{};
    std::memcpy(&value, address, sizeof(value));
    return value;
}

inline std::uint32_t pgm_read_dword(const void* address) noexcept {
    std::uint32_t value{};
    std::memcpy(&value, address, sizeof(value));
    return value;
}

template<typename Type>
inline Type pgm_read_ptr(const Type* address) noexcept {
    Type value{};
    std::memcpy(&value, address, sizeof(value));
    return value;
}

#define strcpy_P std::strcpy
#define memcpy_P std::memcpy
inline std::size_t ardugirl_strnlen(const char* text, std::size_t maximum) noexcept {
    std::size_t length = 0;
    while (length < maximum && text[length] != '\0') ++length;
    return length;
}
#define strnlen_P ardugirl_strnlen
#define strnlen ardugirl_strnlen
#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.017453292519943295769236907684886
#endif
#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif
#define bitRead(value, bit_index) (((value) >> (bit_index)) & 0x01u)
#define bitSet(value, bit_index) ((value) |= (1UL << (bit_index)))
#define bitClear(value, bit_index) ((value) &= ~(1UL << (bit_index)))
#define bitToggle(value, bit_index) ((value) ^= (1UL << (bit_index)))
#define bit(bit_index) (1UL << (bit_index))
#define strlen_P std::strlen

template<typename Value, typename Lower, typename Upper>
constexpr auto constrain(Value value, Lower lower, Upper upper) noexcept {
    using Result = std::common_type_t<Value, Lower, Upper>;
    const Result converted = static_cast<Result>(value);
    return converted < static_cast<Result>(lower) ? static_cast<Result>(lower) :
           converted > static_cast<Result>(upper) ? static_cast<Result>(upper) : converted;
}

using uint_farptr_t = std::uintptr_t;
inline std::size_t strlen_PF(uint_farptr_t address) noexcept {
    return std::strlen(reinterpret_cast<const char*>(address));
}

#define _BV(bit_index) (1u << (bit_index))
inline std::uint8_t USBSTA = 0;
inline constexpr std::uint8_t VBUS = 0;
class USBDeviceClass {
public:
    bool configured() const noexcept { return false; }
};
inline USBDeviceClass USBDevice;

std::uint32_t millis() noexcept;
std::uint32_t micros() noexcept;
void delay(unsigned long duration) noexcept;
void randomSeed(unsigned long seed) noexcept;
long random() noexcept;
long random(long maximum) noexcept;
long random(long minimum, long maximum) noexcept;
long map(long value, long from_low, long from_high,
         long to_low, long to_high) noexcept;
inline char* itoa(int value, char* buffer, int radix) noexcept {
    if (buffer == nullptr || radix < 2 || radix > 36) return buffer;
    char temporary[35]{};
    unsigned int magnitude = value < 0 ? 0u - static_cast<unsigned int>(value) :
                                        static_cast<unsigned int>(value);
    std::size_t count = 0;
    do {
        const auto digit = magnitude % static_cast<unsigned int>(radix);
        temporary[count++] = static_cast<char>(digit < 10 ? '0' + digit : 'a' + digit - 10);
        magnitude /= static_cast<unsigned int>(radix);
    } while (magnitude != 0);
    std::size_t output = 0;
    if (value < 0 && radix == 10) buffer[output++] = '-';
    while (count != 0) buffer[output++] = temporary[--count];
    buffer[output] = '\0';
    return buffer;
}

inline char* ltoa(long value, char* buffer, int radix) noexcept {
    if (buffer == nullptr || radix < 2 || radix > 36) return buffer;
    char temporary[sizeof(unsigned long) * 8u + 1u]{};
    const auto unsigned_value = static_cast<unsigned long>(value);
    unsigned long magnitude = value < 0 ? 0ul - unsigned_value : unsigned_value;
    std::size_t count = 0;
    do {
        const auto digit = magnitude % static_cast<unsigned long>(radix);
        temporary[count++] = static_cast<char>(digit < 10 ? '0' + digit : 'a' + digit - 10);
        magnitude /= static_cast<unsigned long>(radix);
    } while (magnitude != 0);
    std::size_t output = 0;
    if (value < 0 && radix == 10) buffer[output++] = '-';
    while (count != 0) buffer[output++] = temporary[--count];
    buffer[output] = '\0';
    return buffer;
}

// 使用函数而不是 Arduino 常见的宏，避免宏展开破坏 C++ 标准库中的 std::min/max。
template<typename Left, typename Right>
constexpr std::common_type_t<Left, Right> min(Left left, Right right) noexcept {
    using Result = std::common_type_t<Left, Right>;
    return left < right ? static_cast<Result>(left) : static_cast<Result>(right);
}

template<typename Left, typename Right>
constexpr std::common_type_t<Left, Right> max(Left left, Right right) noexcept {
    using Result = std::common_type_t<Left, Right>;
    return left > right ? static_cast<Result>(left) : static_cast<Result>(right);
}
