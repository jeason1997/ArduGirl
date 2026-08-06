#include "ATMlib.h"
#include "ArduboyPlaytune.h"
#include "ardugirl/platform.hpp"
#include "ardugirl/runtime.hpp"

void setup();
void loop();

extern "C" void ardugirl_py32_hardware_init() noexcept;
extern "C" void (*__init_array_start[])();
extern "C" void (*__init_array_end[])();
extern "C" {
volatile std::uint32_t g_ardugirl_frames = 0;
}

namespace {
void run_global_constructors() noexcept {
    // 厂商最小启动文件不链接 libc，因此由运行层显式执行 C++ 静态构造函数。
    for (auto constructor = __init_array_start; constructor != __init_array_end; ++constructor) {
        (*constructor)();
    }
}
} // 匿名命名空间

namespace ardugirl {

bool next_frame(std::uint8_t frames_per_second) noexcept {
    static std::uint32_t deadline = 0;
    if (frames_per_second == 0) return true;
    const auto duration = static_cast<std::uint32_t>(1000u / frames_per_second);
    const auto now = platform::millis();
    if (deadline == 0) deadline = now;
    if (static_cast<std::int32_t>(now - deadline) < 0) platform::sleep_ms(deadline - now);
    deadline += duration;
    return true;
}

void display() noexcept { platform::present(framebuffer().data()); }
std::uint8_t buttons() noexcept { return platform::buttons(); }

} // 命名空间 ardugirl

extern "C" int main() {
    run_global_constructors();
    ardugirl_py32_hardware_init();
    ardugirl::platform::Config config;
    if (!ardugirl::platform::init(config)) while (true) {}
    setup();
    while (true) {
        ardugirl_update_playtunes();
        ardugirl::atm::service();
        loop();
        ++g_ardugirl_frames;
    }
}
