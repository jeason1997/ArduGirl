#pragma once

#include <cstddef>
#include <cstdint>

class HIDSubDescriptor {
public:
    HIDSubDescriptor(const void*, std::size_t) noexcept {}
};

class HID_ {
public:
    void AppendDescriptor(HIDSubDescriptor*) noexcept {}
    void SendReport(std::uint8_t, const void*, std::size_t) noexcept {}
};

inline HID_& HID() noexcept {
    static HID_ instance;
    return instance;
}
