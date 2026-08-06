#include "ardugirl/framebuffer.hpp"
#include "ardugirl/runtime.hpp"

#include <cstdint>

namespace {

std::int16_t box_x = 8;
std::int16_t velocity = 1;

} // 匿名命名空间

const char* ardugirl_game_title() noexcept { return "终端冒烟测试"; }
const char* ardugirl_game_id() noexcept { return "terminal-smoke"; }

void setup() {
    ardugirl::framebuffer().clear();
}

void loop() {
    ardugirl::next_frame(20);
    auto& screen = ardugirl::framebuffer();
    screen.clear();
    screen.draw_rect(0, 0, ardugirl::kScreenWidth, ardugirl::kScreenHeight);
    screen.draw_rect(box_x, 24, 8, 8);
    box_x = static_cast<std::int16_t>(box_x + velocity);
    if (box_x <= 2 || box_x >= ardugirl::kScreenWidth - 10) {
        velocity = static_cast<std::int16_t>(-velocity);
    }
    ardugirl::display();
}
