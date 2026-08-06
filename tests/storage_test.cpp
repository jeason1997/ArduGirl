#include "../platform/linux/storage.hpp"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

#include <unistd.h>

int main() {
    const auto root = std::filesystem::path("/tmp") /
        ("ardugirl-storage-test-" + std::to_string(static_cast<long long>(getpid())));
    std::filesystem::remove_all(root);

    assert(ardugirl::linux_storage::init("game-a", root.c_str()));
    std::uint8_t value = 0;
    assert(ardugirl::linux_storage::read(0, &value, 1));
    assert(value == 0xFFu);
    value = 0x42u;
    assert(ardugirl::linux_storage::write(17, &value, 1));
    assert(!ardugirl::linux_storage::write(1024, &value, 1));
    ardugirl::linux_storage::shutdown();

    // 重启同一游戏必须恢复数据，另一个游戏必须使用独立目录。
    assert(ardugirl::linux_storage::init("game-a", root.c_str()));
    value = 0;
    assert(ardugirl::linux_storage::read(17, &value, 1));
    assert(value == 0x42u);
    ardugirl::linux_storage::shutdown();

    assert(ardugirl::linux_storage::init("game-b", root.c_str()));
    value = 0;
    assert(ardugirl::linux_storage::read(17, &value, 1));
    assert(value == 0xFFu);
    ardugirl::linux_storage::shutdown();

    // 短文件视为损坏存档，读取结果恢复为擦除态。
    const auto corrupt_path = root / "game-c" / "eeprom.bin";
    std::filesystem::create_directories(corrupt_path.parent_path());
    {
        std::ofstream output(corrupt_path, std::ios::binary);
        output.put(static_cast<char>(0x12));
    }
    assert(ardugirl::linux_storage::init("game-c", root.c_str()));
    value = 0;
    assert(ardugirl::linux_storage::read(0, &value, 1));
    assert(value == 0xFFu);
    ardugirl::linux_storage::shutdown();

    std::filesystem::remove_all(root);
    return 0;
}
