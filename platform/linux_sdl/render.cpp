#include "render.hpp"

#include <cstddef>

namespace ardugirl::platform::detail {

void convert_framebuffer(const Framebuffer::Storage& source, bool invert,
                         TexturePixels& destination) noexcept {
    for (std::size_t y = 0; y < kScreenHeight; ++y) {
        for (std::size_t x = 0; x < kScreenWidth; ++x) {
            const auto source_index = x + (y / 8u) * kScreenWidth;
            const auto source_mask = static_cast<std::uint8_t>(1u << (y & 7u));
            const bool lit = ((source[source_index] & source_mask) != 0) != invert;
            destination[x + y * kScreenWidth] = lit ? 0xFFFFFFFFu : 0xFF000000u;
        }
    }
}

} // 命名空间 ardugirl::platform::detail
