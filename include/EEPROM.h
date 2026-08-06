#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

class EEPROMClass {
public:
    std::uint8_t read(int address) const noexcept;
    void write(int address, std::uint8_t value) noexcept;
    void update(int address, std::uint8_t value) noexcept;
    int length() const noexcept;
    std::uint8_t operator[](int address) const noexcept { return read(address); }

    template<typename Value>
    Value& get(int address, Value& value) const noexcept {
        static_assert(std::is_trivially_copyable_v<Value>,
                      "EEPROM.get 只支持可按字节复制的类型");
        auto* bytes = reinterpret_cast<std::uint8_t*>(&value);
        for (std::size_t index = 0; index < sizeof(Value); ++index) {
            bytes[index] = read(address + static_cast<int>(index));
        }
        return value;
    }

    template<typename Value>
    const Value& put(int address, const Value& value) noexcept {
        static_assert(std::is_trivially_copyable_v<Value>,
                      "EEPROM.put 只支持可按字节复制的类型");
        const auto* bytes = reinterpret_cast<const std::uint8_t*>(&value);
        for (std::size_t index = 0; index < sizeof(Value); ++index) {
            update(address + static_cast<int>(index), bytes[index]);
        }
        return value;
    }
};

extern EEPROMClass EEPROM;
