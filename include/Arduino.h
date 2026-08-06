#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <type_traits>

using byte = std::uint8_t;

// 非 AVR 平台使用统一地址空间，字符串常量不需要放入独立的程序存储区。
#define F(text) (text)
#define PROGMEM

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
