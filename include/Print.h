#pragma once

#include <Arduino.h>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <type_traits>

class Print {
public:
    virtual ~Print() = default;
    virtual std::size_t write(std::uint8_t value) = 0;

    std::size_t print(const char* text) {
        std::size_t count = 0;
        while (text && *text) count += write(static_cast<std::uint8_t>(*text++));
        return count;
    }
    std::size_t print(const __FlashStringHelper* text) {
        return print(reinterpret_cast<const char*>(text));
    }

    template <typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
    std::size_t print(Integer value) {
        char buffer[24]{};
        const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
        return result.ec == std::errc{} ? print(buffer) : 0;
    }

    std::size_t println() { return write('\n'); }
    template <typename Value>
    std::size_t println(Value value) { return print(value) + write('\n'); }
};
