#include "display.hpp"
#include "ardugirl/platform.hpp"

#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_gpio.h"

namespace {

constexpr std::uint16_t kPanelWidth = 240;
constexpr std::uint16_t kPanelHeight = 240;
constexpr std::uint16_t kContentWidth = 128;
constexpr std::uint16_t kContentHeight = 64;
constexpr std::uint16_t kContentX = (kPanelWidth - kContentWidth) / 2;
constexpr std::uint16_t kContentY = (kPanelHeight - kContentHeight) / 2;
auto* const kPort = GPIOA;
constexpr std::uint32_t kClockPin = LL_GPIO_PIN_5;
constexpr std::uint32_t kDataPin = LL_GPIO_PIN_7;
constexpr std::uint32_t kResetPin = LL_GPIO_PIN_2;
constexpr std::uint32_t kCommandPin = LL_GPIO_PIN_3;

void delay_ms(std::uint32_t milliseconds) noexcept {
    const auto start = ardugirl::platform::millis();
    while (static_cast<std::uint32_t>(ardugirl::platform::millis() - start) < milliseconds) __WFI();
}

void wait_idle() noexcept {
    while ((SPI1->SR & SPI_SR_TXE) == 0) {}
    while ((SPI1->SR & SPI_SR_BSY) != 0) {}
}

void write_byte(std::uint8_t value) noexcept {
    while ((SPI1->SR & SPI_SR_TXE) == 0) {}
    *reinterpret_cast<volatile std::uint8_t*>(&SPI1->DR) = value;
}

void command(std::uint8_t value) noexcept {
    wait_idle();
    LL_GPIO_ResetOutputPin(kPort, kCommandPin);
    write_byte(value);
}

void data(std::uint8_t value) noexcept {
    wait_idle();
    LL_GPIO_SetOutputPin(kPort, kCommandPin);
    write_byte(value);
}

void window(std::uint16_t x0, std::uint16_t y0, std::uint16_t x1, std::uint16_t y1) noexcept {
    command(0x2A);
    data(static_cast<std::uint8_t>(x0 >> 8)); data(static_cast<std::uint8_t>(x0));
    data(static_cast<std::uint8_t>(x1 >> 8)); data(static_cast<std::uint8_t>(x1));
    command(0x2B);
    data(static_cast<std::uint8_t>(y0 >> 8)); data(static_cast<std::uint8_t>(y0));
    data(static_cast<std::uint8_t>(y1 >> 8)); data(static_cast<std::uint8_t>(y1));
    command(0x2C);
    wait_idle();
    LL_GPIO_SetOutputPin(kPort, kCommandPin);
}

void write_color(std::uint16_t color, std::uint32_t count) noexcept {
    const auto high = static_cast<std::uint8_t>(color >> 8);
    const auto low = static_cast<std::uint8_t>(color);
    while (count-- != 0) { write_byte(high); write_byte(low); }
    wait_idle();
}

void configure_bus() noexcept {
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SPI1);
    LL_GPIO_SetPinMode(kPort, kClockPin, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinMode(kPort, kDataPin, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_0_7(kPort, kClockPin, LL_GPIO_AF_0);
    LL_GPIO_SetAFPin_0_7(kPort, kDataPin, LL_GPIO_AF_0);
    LL_GPIO_SetPinOutputType(kPort, kClockPin, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinOutputType(kPort, kDataPin, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(kPort, kClockPin, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinSpeed(kPort, kDataPin, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinPull(kPort, kClockPin, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(kPort, kDataPin, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinMode(kPort, kResetPin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(kPort, kCommandPin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(kPort, kResetPin, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinOutputType(kPort, kCommandPin, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(kPort, kResetPin, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinSpeed(kPort, kCommandPin, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinPull(kPort, kResetPin, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(kPort, kCommandPin, LL_GPIO_PULL_NO);
    LL_GPIO_SetOutputPin(kPort, kResetPin | kCommandPin);
    // 参考屏使用 SPI 模式 3；24 MHz 系统时钟二分频后为 12 MHz，可满足 128×64 单色画面的 60 FPS 带宽。
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPOL |
                SPI_CR1_CPHA | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
    SPI1->CR2 = 0;
    SPI1->CR1 |= SPI_CR1_SPE;
}

} // 匿名命名空间

namespace ardugirl::py32::display {

bool init() noexcept {
    configure_bus();
    LL_GPIO_ResetOutputPin(kPort, kResetPin);
    delay_ms(10);
    LL_GPIO_SetOutputPin(kPort, kResetPin);
    delay_ms(150);
    command(0x11); delay_ms(150);
    command(0x36); data(0x00);
    command(0x3A); data(0x55);
    command(0x21);
    // 单色前端只依赖像素格式和反色命令，沿用面板上电默认的电源、帧率与 Gamma 参数可节省固件空间。
    command(0x29); delay_ms(20);
    window(0, 0, kPanelWidth - 1, kPanelHeight - 1);
    write_color(0x0000, static_cast<std::uint32_t>(kPanelWidth) * kPanelHeight);
    return true;
}

void present(const std::uint8_t* framebuffer) noexcept {
    window(kContentX, kContentY, kContentX + kContentWidth - 1,
           kContentY + kContentHeight - 1);
    for (std::uint16_t y = 0; y < kContentHeight; ++y) {
        for (std::uint16_t x = 0; x < kContentWidth; ++x) {
            const auto value = framebuffer[x + (y / 8u) * kContentWidth];
            const auto color = (value & (1u << (y & 7u))) != 0 ? 0xFFFFu : 0x0000u;
            write_byte(static_cast<std::uint8_t>(color >> 8));
            write_byte(static_cast<std::uint8_t>(color));
        }
    }
    wait_idle();
}

} // 命名空间 ardugirl::py32::display
