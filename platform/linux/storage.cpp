#include "storage.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace ardugirl::linux_storage {

namespace {

constexpr std::size_t kStorageBytes = 1024;
std::array<std::uint8_t, kStorageBytes> bytes{};
std::filesystem::path storage_path;
bool dirty = false;
bool initialized = false;

std::filesystem::path default_root() {
    if (const char* xdg = std::getenv("XDG_DATA_HOME"); xdg != nullptr && xdg[0] != '\0') {
        return xdg;
    }
    if (const char* home = std::getenv("HOME"); home != nullptr && home[0] != '\0') {
        return std::filesystem::path(home) / ".local" / "share";
    }
    return std::filesystem::current_path();
}

bool valid_range(std::uint16_t offset, std::uint16_t size) noexcept {
    return static_cast<std::size_t>(offset) + size <= bytes.size();
}

} // 匿名命名空间

bool init(const char* game_id, const char* save_dir) noexcept {
    try {
        bytes.fill(0xFFu);
        dirty = false;
        const auto root = save_dir != nullptr && save_dir[0] != '\0'
            ? std::filesystem::path(save_dir) : default_root() / "ardugirl";
        storage_path = root / (game_id != nullptr ? game_id : "unknown") / "eeprom.bin";

        std::ifstream input(storage_path, std::ios::binary);
        if (input) {
            input.read(reinterpret_cast<char*>(bytes.data()), bytes.size());
            // 短文件和附带额外数据的文件都视为损坏，恢复为全 0xFF，
            // 避免把部分存档静默暴露给游戏。
            if (input.gcount() != static_cast<std::streamsize>(bytes.size()) ||
                input.peek() != std::ifstream::traits_type::eof()) {
                bytes.fill(0xFFu);
            }
        }
        initialized = true;
        return true;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "EEPROM 初始化失败：%s\n", error.what());
        initialized = false;
        return false;
    }
}

void shutdown() noexcept {
    if (!initialized || !dirty) {
        initialized = false;
        return;
    }
    try {
        std::filesystem::create_directories(storage_path.parent_path());
        auto temporary = storage_path;
        temporary += ".tmp";
        {
            std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
            output.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
            output.flush();
            if (!output) {
                throw std::runtime_error("临时文件写入失败");
            }
        }
        std::error_code error;
        std::filesystem::rename(temporary, storage_path, error);
        if (error) {
            // Windows 不允许 rename 覆盖已有文件；Linux 通常直接走上面的原子替换。
            std::filesystem::remove(storage_path, error);
            error.clear();
            std::filesystem::rename(temporary, storage_path, error);
        }
        if (error) {
            throw std::filesystem::filesystem_error("存档替换失败", error);
        }
    } catch (const std::exception& error) {
        std::fprintf(stderr, "EEPROM 保存失败：%s\n", error.what());
    }
    dirty = false;
    initialized = false;
}

bool read(std::uint16_t offset, void* destination, std::uint16_t size) noexcept {
    if (!initialized || destination == nullptr || !valid_range(offset, size)) {
        return false;
    }
    std::memcpy(destination, bytes.data() + offset, size);
    return true;
}

bool write(std::uint16_t offset, const void* source, std::uint16_t size) noexcept {
    if (!initialized || source == nullptr || !valid_range(offset, size)) {
        return false;
    }
    if (std::memcmp(bytes.data() + offset, source, size) != 0) {
        std::memcpy(bytes.data() + offset, source, size);
        dirty = true;
    }
    return true;
}

} // 命名空间 ardugirl::linux_storage
