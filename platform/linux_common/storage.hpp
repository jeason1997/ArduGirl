#pragma once

#include <cstdint>

namespace ardugirl::linux_storage {

bool init(const char* game_id, const char* save_dir) noexcept;
void shutdown() noexcept;
bool read(std::uint16_t offset, void* destination, std::uint16_t size) noexcept;
bool write(std::uint16_t offset, const void* source, std::uint16_t size) noexcept;

} // 命名空间 ardugirl::linux_storage
