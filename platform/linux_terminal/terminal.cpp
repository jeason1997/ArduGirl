#include "ardugirl/constants.hpp"
#include "ardugirl/platform.hpp"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace ardugirl::platform {

namespace {

using Clock = std::chrono::steady_clock;

Clock::time_point start_time;
termios original_terminal{};
int original_flags = 0;
bool terminal_changed = false;
bool input_flags_changed = false;
bool running = true;
bool plain_output = false;
const char* game_title = "ArduGirl";
std::uint8_t current_buttons = 0;
Clock::time_point buttons_expire;

constexpr std::uint8_t kLeft = 0x01;
constexpr std::uint8_t kRight = 0x02;
constexpr std::uint8_t kUp = 0x04;
constexpr std::uint8_t kDown = 0x08;
constexpr std::uint8_t kA = 0x10;
constexpr std::uint8_t kB = 0x20;

bool framebuffer_pixel(const Framebuffer::Storage& pixels, int x, int y) noexcept {
    const auto index = static_cast<std::size_t>(x) +
                       static_cast<std::size_t>(y / 8) * kScreenWidth;
    return (pixels[index] & static_cast<std::uint8_t>(1u << (y & 7))) != 0;
}

void append_utf8(std::string& output, std::uint32_t codepoint) {
    output.push_back(static_cast<char>(0xE0u | ((codepoint >> 12u) & 0x0Fu)));
    output.push_back(static_cast<char>(0x80u | ((codepoint >> 6u) & 0x3Fu)));
    output.push_back(static_cast<char>(0x80u | (codepoint & 0x3Fu)));
}

std::uint8_t braille_mask(const Framebuffer::Storage& pixels, int x, int y) noexcept {
    // Unicode 盲文把左列编号为 1、2、3、7，右列编号为 4、5、6、8，
    // 它不是普通的按行排列位图，因此需要显式映射每个点的掩码。
    constexpr std::uint8_t masks[4][2] = {
        {0x01, 0x08},
        {0x02, 0x10},
        {0x04, 0x20},
        {0x40, 0x80},
    };

    std::uint8_t result = 0;
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 2; ++column) {
            if (framebuffer_pixel(pixels, x + column, y + row)) {
                result |= masks[row][column];
            }
        }
    }
    return result;
}

std::uint8_t map_key(char key) noexcept {
    switch (key) {
    case 'a': case 'A': return kLeft;
    case 'd': case 'D': return kRight;
    case 'w': case 'W': return kUp;
    case 's': case 'S': return kDown;
    case 'j': case 'J': case 'z': case 'Z': return kA;
    case 'k': case 'K': case 'x': case 'X': return kB;
    default: return 0;
    }
}

} // 匿名命名空间

bool init(const Config& config) noexcept {
    start_time = Clock::now();
    buttons_expire = start_time;
    plain_output = config.plain_output || !isatty(STDOUT_FILENO);
    game_title = config.title;

    original_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (original_flags >= 0 &&
        fcntl(STDIN_FILENO, F_SETFL, original_flags | O_NONBLOCK) == 0) {
        input_flags_changed = true;
    }

    if (isatty(STDIN_FILENO)) {
        if (tcgetattr(STDIN_FILENO, &original_terminal) != 0) {
            return false;
        }
        auto raw = original_terminal;
        raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
            return false;
        }
        terminal_changed = true;
    }

    if (!plain_output) {
        std::fputs("\x1b[?25l\x1b[2J\x1b[H", stdout);
    }
    return true;
}

void shutdown() noexcept {
    if (terminal_changed) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal);
    }
    if (input_flags_changed) {
        fcntl(STDIN_FILENO, F_SETFL, original_flags);
    }
    if (!plain_output) {
        std::fputs("\x1b[?25h\x1b[0m\n", stdout);
    }
    std::fflush(stdout);
}

bool pump_events() noexcept {
    char key = 0;
    while (read(STDIN_FILENO, &key, 1) == 1) {
        if (key == 'q' || key == 'Q' || key == 27) {
            running = false;
        }
        const auto mapped = map_key(key);
        if (mapped != 0) {
            current_buttons = mapped;
            // TTY 输入只能报告字节，没有可靠的按键释放事件。短暂保持按键状态
            // 可以让游戏观察到轻触，同时避免旧输入一直处于按下状态。
            buttons_expire = Clock::now() + std::chrono::milliseconds(100);
        }
    }
    if (Clock::now() >= buttons_expire) {
        current_buttons = 0;
    }
    return running;
}

std::uint8_t buttons() noexcept {
    return current_buttons;
}

std::uint32_t millis() noexcept {
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now() - start_time).count();
    return static_cast<std::uint32_t>(elapsed);
}

void sleep_ms(std::uint32_t duration) noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
}

void present(const Framebuffer::Storage& pixels) noexcept {
    std::string output;
    output.reserve(1200);
    if (!plain_output) {
        output += "\x1b[H";
    }
    output += "+----------------------------------------------------------------+\n";

    for (int y = 0; y < kScreenHeight; y += 4) {
        output += '|';
        for (int x = 0; x < kScreenWidth; x += 2) {
            append_utf8(output, 0x2800u + braille_mask(pixels, x, y));
        }
        output += "|\n";
    }

    output += "+----------------------------------------------------------------+\n";
    output += "ArduGirl terminal | ";
    output += game_title;
    output += " | Q: quit\n";
    std::fwrite(output.data(), 1, output.size(), stdout);
    std::fflush(stdout);
}

} // 命名空间 ardugirl::platform
