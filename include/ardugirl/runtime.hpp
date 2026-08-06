#pragma once

#include <cstdint>

namespace ardugirl {

bool next_frame(std::uint8_t frames_per_second = 30) noexcept;
void display() noexcept;
std::uint8_t buttons() noexcept;

} // 命名空间 ardugirl
