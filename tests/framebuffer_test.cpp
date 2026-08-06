#include "ardugirl/framebuffer.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>

int main() {
    ardugirl::Framebuffer screen;

    // 这两个像素分别验证第一页和最后一页的 SSD1306 垂直位布局。
    screen.set_pixel(0, 0);
    screen.set_pixel(127, 63);
    assert(screen.data()[0] == 0x01u);
    assert(screen.data()[1023] == 0x80u);
    assert(screen.pixel(0, 0));
    assert(screen.pixel(127, 63));

    // 屏外写入必须被裁剪，不能破坏显存首尾字节。
    screen.set_pixel(-1, 0);
    screen.set_pixel(128, 63);
    assert(screen.data()[0] == 0x01u);
    assert(screen.data()[1023] == 0x80u);

    screen.clear();
    screen.draw_rect(2, 2, 4, 4);
    assert(screen.pixel(2, 2));
    assert(screen.pixel(5, 5));
    assert(!screen.pixel(3, 3));

    screen.clear(true);
    for (std::size_t index = 0; index < screen.data().size(); ++index) {
        assert(screen.data()[index] == 0xFFu);
    }
    return 0;
}

