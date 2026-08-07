#include <cmath>
#include <cstdint>

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kHalfPi = kPi * 0.5f;
constexpr float kTwoPi = kPi * 2.0f;

float reduce_angle(float value) noexcept {
    while (value > kPi) value -= kTwoPi;
    while (value < -kPi) value += kTwoPi;
    return value;
}

float sine(float value) noexcept {
    value = reduce_angle(value);
    const float squared = value * value;
    // 七阶多项式在游戏动画所需范围内保持亚像素级误差，避免 Cortex-M0+ 链入完整双精度 libm。
    return value * (1.0f + squared * (-0.16666667f + squared * (0.0083333310f - squared * 0.0001984090f)));
}

} // 匿名命名空间

extern "C" double sin(double value) noexcept { return static_cast<double>(sine(static_cast<float>(value))); }
extern "C" double cos(double value) noexcept { return static_cast<double>(sine(static_cast<float>(value) + kHalfPi)); }

extern "C" double sqrt(double value) noexcept {
    if (value <= 0.0) return 0.0;
    float estimate = static_cast<float>(value);
    for (std::uint8_t iteration = 0; iteration < 6; ++iteration) {
        estimate = 0.5f * (estimate + static_cast<float>(value) / estimate);
    }
    return static_cast<double>(estimate);
}

extern "C" double atan2(double y_value, double x_value) noexcept {
    const float y = static_cast<float>(y_value);
    const float x = static_cast<float>(x_value);
    if (x == 0.0f) return y > 0.0f ? kHalfPi : (y < 0.0f ? -kHalfPi : 0.0f);
    const float absolute_y = y < 0.0f ? -y : y;
    float angle;
    if (x >= 0.0f) {
        const float ratio = (x - absolute_y) / (x + absolute_y);
        angle = kPi * 0.25f - kPi * 0.25f * ratio;
    } else {
        const float ratio = (x + absolute_y) / (absolute_y - x);
        angle = kPi * 0.75f - kPi * 0.25f * ratio;
    }
    return static_cast<double>(y < 0.0f ? -angle : angle);
}

extern "C" double pow(double base_value, double exponent_value) noexcept {
    // 当前游戏只用 2 的实数次幂做按键加速；拆分整数和小数部分可避免通用对数/指数库。
    float exponent = static_cast<float>(exponent_value);
    if (base_value != 2.0) return 0.0;
    int whole = static_cast<int>(exponent);
    if (exponent < 0.0f && static_cast<float>(whole) != exponent) --whole;
    const float fraction = exponent - static_cast<float>(whole);
    float result = 1.0f + fraction * (0.69314718f + fraction * (0.24022651f + fraction * 0.05550411f));
    while (whole > 0) { result *= 2.0f; --whole; }
    while (whole < 0) { result *= 0.5f; ++whole; }
    return static_cast<double>(result);
}
