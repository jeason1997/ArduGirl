// 官方示例保持在 Git 子模块中不做修改，本文件只让 .ino 参与标准 C++ 构建。
const char* ardugirl_game_title() noexcept {
    return "Arduboy2 official HelloWorld";
}

const char* ardugirl_game_id() noexcept {
    return "arduboy2-hello";
}

#include "../../../third_party/Arduboy2/examples/HelloWorld/HelloWorld.ino"
