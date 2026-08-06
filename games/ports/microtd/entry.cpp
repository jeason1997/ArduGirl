// MicroTD 上游源码保持不变，本文件只负责把 Arduino sketch 纳入 C++ 构建。
const char* ardugirl_game_title() noexcept {
    return "MicroTD";
}

const char* ardugirl_game_id() noexcept {
    return "microtd";
}

#include "microtd_patched.ino"
