#include "ardugirl/framebuffer.hpp"
#include "ardugirl/runtime.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>

namespace {

std::uint8_t replay_buttons = 0;
std::uint32_t presented_frames = 0;
std::uint8_t replay_storage[1024]{};

} // 匿名命名空间

namespace ardugirl {

bool next_frame(std::uint8_t) noexcept {
    return true;
}

void display() noexcept {
    ++presented_frames;
}

std::uint8_t buttons() noexcept {
    return replay_buttons;
}

} // 命名空间 ardugirl

namespace ardugirl::platform {

std::uint32_t millis() noexcept {
    return presented_frames * 16u;
}

std::uint32_t micros() noexcept { return millis() * 1000u; }
void sleep_ms(std::uint32_t) noexcept {}
void set_tone(std::uint16_t, std::uint8_t) noexcept {}
void stop_tone(std::uint8_t) noexcept {}
void play_wave(std::uint16_t, const std::uint8_t*, std::uint16_t) noexcept {}
void stop_wave() noexcept {}

bool storage_read(std::uint16_t offset, void* destination,
                  std::uint16_t size) noexcept {
    if (static_cast<std::size_t>(offset) + size > sizeof(replay_storage)) {
        return false;
    }
    std::memcpy(destination, replay_storage + offset, size);
    return true;
}

bool storage_write(std::uint16_t offset, const void* source,
                   std::uint16_t size) noexcept {
    if (static_cast<std::size_t>(offset) + size > sizeof(replay_storage)) {
        return false;
    }
    std::memcpy(replay_storage + offset, source, size);
    return true;
}

} // 命名空间 ardugirl::platform

// 测试直接编译未修改的移植入口，以便检查真实游戏状态而非复制状态机。
#include "../games/microtd/entry.cpp"

namespace {

void frame(std::uint8_t buttons = 0) {
    replay_buttons = buttons;
    loop();
}

void tap(std::uint8_t button) {
    frame(button);
    frame();
}

} // 匿名命名空间

int main() {
    setup();
    assert(game.mState == STATE_MENU);

    // 选择第一张地图，确认输入边沿已进入建造阶段。
    tap(A_BUTTON);
    assert(game.mState == STATE_PLAYING_BUILDING);
    assert(game.mCursor.mX == 8 && game.mCursor.mY == 4);

    // 初始光标位于可建造格；打开菜单并建造默认的第一种塔。
    const auto tower_position = game.mCursor;
    tap(A_BUTTON);
    assert(game.mState == STATE_PLAYING_MENU);
    assert(game.getGameMenuItem(0).mState == ITEM_STATE_AVAILABLE);
    tap(A_BUTTON);
    assert(game.mState == STATE_PLAYING_BUILDING);
    assert(game.mPlayground.getTile(tower_position)->mType == TILE_TOWER);

    // 再次打开菜单，逐项移动到“下一波”并确认，验证波次真正生成敌人。
    tap(A_BUTTON);
    assert(game.mState == STATE_PLAYING_MENU);
    for (std::uint8_t index = 0; index < GAME_MENU_NEXT_WAVE; ++index) {
        tap(RIGHT_BUTTON);
    }
    assert(game.mSelectedMenuItem == GAME_MENU_NEXT_WAVE);
    tap(A_BUTTON);
    assert(game.mState == STATE_PLAYING_WAVE);
    assert(game.mCreepCount > 0);
    assert(presented_frames > 0);
    return 0;
}
