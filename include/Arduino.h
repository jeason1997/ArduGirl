#pragma once

#include <cstddef>
#include <cstdint>

using byte = std::uint8_t;

// 非 AVR 平台使用统一地址空间，字符串常量不需要放入独立的程序存储区。
#define F(text) (text)
