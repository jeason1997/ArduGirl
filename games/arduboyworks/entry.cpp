#ifndef ARDUBOYWORKS_ENTRY
#error 必须通过构建目标指定 ArduboyWorks 游戏入口
#endif

#include ARDUBOYWORKS_ENTRY

#define ARDUGIRL_STRINGIFY_INNER(value) #value
#define ARDUGIRL_STRINGIFY(value) ARDUGIRL_STRINGIFY_INNER(value)

const char* ardugirl_game_id() noexcept { return ARDUGIRL_STRINGIFY(ARDUBOYWORKS_GAME_ID); }
const char* ardugirl_game_title() noexcept { return ARDUGIRL_STRINGIFY(ARDUBOYWORKS_GAME_ID); }
