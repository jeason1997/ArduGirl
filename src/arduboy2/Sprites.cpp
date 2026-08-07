#include "Sprites.h"

#include "ardugirl/framebuffer.hpp"

#include <cstddef>

namespace {

bool sprite_bit(const std::uint8_t* data, std::uint8_t width,
                std::uint8_t x, std::uint8_t y) noexcept {
    const auto index = static_cast<std::size_t>(x) +
                       static_cast<std::size_t>(y / 8) * width;
    return (data[index] & static_cast<std::uint8_t>(1u << (y & 7))) != 0;
}

} // 匿名命名空间

void Sprites::drawOverwrite(std::int16_t x, std::int16_t y,
                            const std::uint8_t* bitmap,
                            std::uint8_t frame) noexcept {
    if (bitmap == nullptr) return;
    const auto width = bitmap[0];
    const auto height = bitmap[1];
    const auto frame_size = static_cast<std::size_t>(width) * ((height + 7u) / 8u);
    const auto* data = bitmap + 2u + frame_size * frame;
    auto& screen = ardugirl::framebuffer();
    if ((y & 7) == 0 && (height & 7u) == 0) {
        auto& destination = screen.data();
        const auto first_page = y / 8;
        const auto pages = height / 8u;
        for (std::uint8_t page = 0; page < pages; ++page) {
            const auto destination_page = first_page + page;
            if (destination_page < 0 || destination_page >= 8) continue;
            for (std::uint8_t column = 0; column < width; ++column) {
                const auto destination_x = x + column;
                if (destination_x < 0 || destination_x >= 128) continue;
                destination[static_cast<std::size_t>(destination_x) +
                            static_cast<std::size_t>(destination_page) * 128u] =
                    data[static_cast<std::size_t>(page) * width + column];
            }
        }
        return;
    }
    for (std::uint8_t row = 0; row < height; ++row) {
        for (std::uint8_t column = 0; column < width; ++column) {
            screen.set_pixel(x + column, y + row,
                             sprite_bit(data, width, column, row));
        }
    }
}

void Sprites::drawPlusMask(std::int16_t x, std::int16_t y,
                           const std::uint8_t* bitmap,
                           std::uint8_t frame) noexcept {
    if (bitmap == nullptr) return;
    const auto width = bitmap[0];
    const auto height = bitmap[1];
    const auto pages = static_cast<std::size_t>((height + 7u) / 8u);
    const auto frame_size = static_cast<std::size_t>(width) * pages * 2u;
    const auto* data = bitmap + 2u + frame_size * frame;
    auto& screen = ardugirl::framebuffer();
    for (std::uint8_t row = 0; row < height; ++row) {
        for (std::uint8_t column = 0; column < width; ++column) {
            const auto byte_index =
                (static_cast<std::size_t>(row / 8) * width + column) * 2u;
            const auto bit = static_cast<std::uint8_t>(1u << (row & 7));
            const bool image = (data[byte_index] & bit) != 0;
            const bool mask = (data[byte_index + 1u] & bit) != 0;
            if (mask) {
                screen.set_pixel(x + column, y + row, image);
            }
        }
    }
}

void Sprites::drawExternalMask(std::int16_t x, std::int16_t y,
                               const std::uint8_t* bitmap,
                               const std::uint8_t* mask,
                               std::uint8_t frame,
                               std::uint8_t mask_frame) noexcept {
    if (bitmap == nullptr || mask == nullptr) return;
    const auto width = bitmap[0];
    const auto height = bitmap[1];
    const auto frame_size = static_cast<std::size_t>(width) * ((height + 7u) / 8u);
    const auto* image_data = bitmap + 2u + frame_size * frame;
    const auto* mask_data = mask + frame_size * mask_frame;
    auto& screen = ardugirl::framebuffer();
    for (std::uint8_t row = 0; row < height; ++row)
        for (std::uint8_t column = 0; column < width; ++column)
            if (sprite_bit(mask_data, width, column, row))
                screen.set_pixel(x + column, y + row, sprite_bit(image_data, width, column, row));
}

void Sprites::drawSelfMasked(std::int16_t x, std::int16_t y,
                             const std::uint8_t* bitmap,
                             std::uint8_t frame) noexcept {
    if (bitmap == nullptr) return;
    const auto width = bitmap[0]; const auto height = bitmap[1];
    const auto size = static_cast<std::size_t>(width) * ((height + 7u) / 8u);
    const auto* data = bitmap + 2u + size * frame;
    auto& screen = ardugirl::framebuffer();
    if ((y & 7) == 0 && (height & 7u) == 0) {
        auto& destination = screen.data();
        const auto first_page = y / 8;
        const auto pages = height / 8u;
        for (std::uint8_t page = 0; page < pages; ++page) {
            const auto destination_page = first_page + page;
            if (destination_page < 0 || destination_page >= 8) continue;
            for (std::uint8_t column = 0; column < width; ++column) {
                const auto destination_x = x + column;
                if (destination_x < 0 || destination_x >= 128) continue;
                destination[static_cast<std::size_t>(destination_x) +
                            static_cast<std::size_t>(destination_page) * 128u] |=
                    data[static_cast<std::size_t>(page) * width + column];
            }
        }
        return;
    }
    for (std::uint8_t row = 0; row < height; ++row)
        for (std::uint8_t column = 0; column < width; ++column)
            if (sprite_bit(data, width, column, row)) screen.set_pixel(x + column, y + row, true);
}

void Sprites::drawErase(std::int16_t x, std::int16_t y,
                        const std::uint8_t* bitmap,
                        std::uint8_t frame) noexcept {
    if (bitmap == nullptr) return;
    const auto width = bitmap[0]; const auto height = bitmap[1];
    const auto size = static_cast<std::size_t>(width) * ((height + 7u) / 8u);
    const auto* data = bitmap + 2u + size * frame;
    auto& screen = ardugirl::framebuffer();
    if ((y & 7) == 0 && (height & 7u) == 0) {
        auto& destination = screen.data();
        const auto first_page = y / 8;
        const auto pages = height / 8u;
        for (std::uint8_t page = 0; page < pages; ++page) {
            const auto destination_page = first_page + page;
            if (destination_page < 0 || destination_page >= 8) continue;
            for (std::uint8_t column = 0; column < width; ++column) {
                const auto destination_x = x + column;
                if (destination_x < 0 || destination_x >= 128) continue;
                const auto source = data[static_cast<std::size_t>(page) * width + column];
                destination[static_cast<std::size_t>(destination_x) +
                            static_cast<std::size_t>(destination_page) * 128u] &=
                    static_cast<std::uint8_t>(~source);
            }
        }
        return;
    }
    for (std::uint8_t row = 0; row < height; ++row)
        for (std::uint8_t column = 0; column < width; ++column)
            if (sprite_bit(data, width, column, row)) screen.set_pixel(x + column, y + row, false);
}
