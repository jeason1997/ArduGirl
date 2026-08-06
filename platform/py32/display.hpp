#pragma once

#include <cstdint>

namespace ardugirl::py32::display {

bool init() noexcept;
void present(const std::uint8_t* framebuffer) noexcept;

} // 命名空间 ardugirl::py32::display
