#pragma once

#include "ardugirl/framebuffer.hpp"

#include <array>
#include <cstdint>

namespace ardugirl::platform::detail {

using TexturePixels = std::array<std::uint32_t, kScreenWidth * kScreenHeight>;

// 将 Arduboy 页面布局转换为 SDL 纹理使用的逐行 ARGB8888 像素。
// 反色只影响显示结果，调用者提供的核心 framebuffer 始终保持不变。
void convert_framebuffer(const Framebuffer::Storage& source, bool invert,
                         TexturePixels& destination) noexcept;

} // 命名空间 ardugirl::platform::detail
