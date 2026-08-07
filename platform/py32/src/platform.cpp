#include "ardugirl/platform.hpp"
#include "board.hpp"
#include "display.hpp"

#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_rcc.h"
#include "py32f0xx_ll_utils.h"

#include <cstddef>

namespace {

constexpr std::uint32_t kSystemClockHz = 48000000u;
constexpr std::uint32_t kFlashRatedClockHz = 48000000u;
constexpr std::uint32_t kAudioTickHz = 20000u;
auto& pll_config = *reinterpret_cast<volatile std::uint32_t*>(RCC_BASE + 0x0Cu);
constexpr std::uint32_t kPllEnable = 1u << 24;
constexpr std::uint32_t kPllReady = 1u << 25;
constexpr std::uint32_t kPllClockSource = 2u << RCC_CFGR_SW_Pos;
constexpr std::uint32_t kPllClockStatus = 2u << RCC_CFGR_SWS_Pos;
volatile std::uint32_t audio_ticks = 0;
volatile std::uint32_t tone_phase = 0;
volatile std::uint32_t tone_step = 0;

void configure_clock() noexcept {
    LL_RCC_HSI_SetCalibFreq(LL_RCC_HSICALIBRATION_24MHz);
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1) {}
    if (LL_SetFlashLatency(kFlashRatedClockHz) != SUCCESS) while (true) {}
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    pll_config = 0;
    RCC->CR |= kPllEnable;
    while ((RCC->CR & kPllReady) == 0) {}
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW_Msk, kPllClockSource);
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != kPllClockStatus) {}
    LL_SetSystemCoreClock(kSystemClockHz);
    SysTick_Config(kSystemClockHz / kAudioTickHz);
}

void configure_io() noexcept {
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
    constexpr std::uint32_t buttons[] = {
        ARDUGIRL_BUTTON_LEFT, ARDUGIRL_BUTTON_RIGHT, ARDUGIRL_BUTTON_UP,
        ARDUGIRL_BUTTON_DOWN, ARDUGIRL_BUTTON_A, ARDUGIRL_BUTTON_B};
    for (const auto pin : buttons) {
        // PY32 LL 的单元 GPIO 接口一次只接受一个引脚，组合掩码会破坏相邻配置位。
        LL_GPIO_SetPinMode(ARDUGIRL_BUTTON_PORT, pin, LL_GPIO_MODE_INPUT);
        LL_GPIO_SetPinPull(ARDUGIRL_BUTTON_PORT, pin, LL_GPIO_PULL_UP);
    }
    LL_GPIO_SetPinMode(ARDUGIRL_BUZZER_PORT, ARDUGIRL_BUZZER_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_ResetOutputPin(ARDUGIRL_BUZZER_PORT, ARDUGIRL_BUZZER_PIN);
}

} // 匿名命名空间

extern "C" void ardugirl_py32_hardware_init() noexcept {
    configure_clock();
    configure_io();
}

extern "C" void SysTick_Handler() {
    ++audio_ticks;
    const auto step = tone_step;
    if (step == 0) return;
    const auto old_phase = tone_phase;
    tone_phase += step;
    if (((old_phase ^ tone_phase) & 0x80000000u) != 0) {
        ARDUGIRL_BUZZER_PORT->ODR ^= ARDUGIRL_BUZZER_PIN;
    }
}

namespace ardugirl::platform {

bool init(const Config&) noexcept { return py32::display::init(); }
void shutdown() noexcept { stop_tone(); }
bool pump_events() noexcept { return true; }

std::uint8_t buttons() noexcept {
#if ARDUGIRL_BUTTONS_CONNECTED == 0
    // 未接按键时不能采信悬空输入；固定返回 Arduboy 六键全部松开。
    return 0;
#else
    const auto pins = ARDUGIRL_BUTTON_PORT->IDR;
    std::uint8_t value = 0;
    if ((pins & ARDUGIRL_BUTTON_LEFT) == 0) value |= 0x01u;
    if ((pins & ARDUGIRL_BUTTON_RIGHT) == 0) value |= 0x02u;
    if ((pins & ARDUGIRL_BUTTON_UP) == 0) value |= 0x04u;
    if ((pins & ARDUGIRL_BUTTON_DOWN) == 0) value |= 0x08u;
    if ((pins & ARDUGIRL_BUTTON_A) == 0) value |= 0x10u;
    if ((pins & ARDUGIRL_BUTTON_B) == 0) value |= 0x20u;
    return value;
#endif
}

std::uint32_t micros() noexcept { return audio_ticks * (1000000u / kAudioTickHz); }
std::uint32_t millis() noexcept { return audio_ticks / (kAudioTickHz / 1000u); }
void sleep_ms(std::uint32_t duration) noexcept {
    const auto start = millis();
    while (static_cast<std::uint32_t>(millis() - start) < duration) __WFI();
}

void set_tone(std::uint16_t frequency_hz, std::uint8_t) noexcept {
    // 2^32/20000 向下取整后乘频率，避免 Cortex-M0+ 引入昂贵的 64 位除法辅助函数。
    tone_step = frequency_hz == 0 ? 0 : static_cast<std::uint32_t>(frequency_hz) * 214748u;
    if (tone_step == 0) LL_GPIO_ResetOutputPin(ARDUGIRL_BUZZER_PORT, ARDUGIRL_BUZZER_PIN);
}
void stop_tone(std::uint8_t) noexcept { set_tone(0); }
void play_wave(std::uint16_t, const std::uint8_t*, std::uint16_t) noexcept {}
void stop_wave() noexcept {}

void set_synth(const SynthVoice* voices, std::uint8_t count) noexcept {
    std::uint8_t best_volume = 0;
    std::uint16_t frequency = 0;
    for (std::uint8_t index = 0; index < count; ++index) {
        if (voices[index].volume > best_volume) {
            best_volume = voices[index].volume;
            frequency = voices[index].frequency_hz;
        }
    }
    set_tone(frequency);
}

void present(const Framebuffer::Storage& pixels) noexcept { py32::display::present(pixels.data()); }

// 首版不冒充持久化能力：读出擦除态，拒绝写入。后续将用独立 Flash 页实现日志式 EEPROM。
bool storage_read(std::uint16_t, void* destination, std::uint16_t size) noexcept {
    auto* bytes = static_cast<std::uint8_t*>(destination);
    while (size-- != 0) *bytes++ = 0xFFu;
    return true;
}
bool storage_write(std::uint16_t, const void*, std::uint16_t) noexcept { return false; }

} // 命名空间 ardugirl::platform
