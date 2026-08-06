# 架构与平台契约

## 1. 分层

ArduGirl 使用四层结构：

1. **Game**：原始游戏逻辑、资源、`setup()` 和 `loop()`。
2. **Compatibility**：Arduboy2、Sprites、Arduino 和有限 AVR 兼容 API。
3. **Core**：统一 runtime、1024 字节 framebuffer 和平台无关服务。
4. **Backend**：首先实现 Linux terminal，随后实现 Linux SDL2，未来再实现 PY32/STM32。

只有下层可以被上层依赖。后端通过注册或链接时选择实现平台契约。

## 2. 屏幕契约

```cpp
inline constexpr std::uint16_t kScreenWidth = 128;
inline constexpr std::uint16_t kScreenHeight = 64;
inline constexpr std::size_t kFramebufferBytes = 1024;
```

像素 `(x, y)` 对应：

```cpp
index = x + (y >> 3) * 128;
mask  = 1u << (y & 7);
```

核心只保存 1-bit buffer。Linux 后端上传纹理时转换为显示格式；彩色 MCU 后端也在 `present` 阶段转换，不改变核心布局。

## 3. 按键契约

平台每次采样返回以下六位：

```cpp
enum class Button : std::uint8_t {
    Left  = 0x01,
    Right = 0x02,
    Up    = 0x04,
    Down  = 0x08,
    A     = 0x10,
    B     = 0x20,
};
```

兼容层负责保存 previous/current 状态并实现 `pressed`、`notPressed`、`justPressed`、`justReleased`。平台只报告电平，不实现游戏语义。

## 4. 建议的平台接口

实际代码落地时允许按测试需要拆成更小接口，但不能泄漏后端类型：

```cpp
namespace ardugirl::platform {

struct Config {
    const char* app_id;
    const char* title;
    std::uint8_t scale;
};

bool init(const Config&) noexcept;
void shutdown() noexcept;
bool pump_events() noexcept;       // false 表示请求退出
std::uint8_t buttons() noexcept;

std::uint32_t millis() noexcept;
std::uint32_t micros() noexcept;
void idle_until(std::uint32_t deadline_us) noexcept;

void present(const std::uint8_t (&framebuffer)[1024]) noexcept;

void set_tone(std::uint16_t frequency_hz) noexcept;
void stop_tone() noexcept;

bool storage_read(std::uint16_t offset, void* dst,
                  std::uint16_t size) noexcept;
bool storage_write(std::uint16_t offset, const void* src,
                   std::uint16_t size) noexcept;

} // namespace ardugirl::platform
```

错误策略：初始化阶段返回错误并输出诊断；游戏主循环中的服务必须 `noexcept`，资源受限平台不依赖异常。

## 5. Runtime

桌面入口由 ArduGirl 提供，游戏仍只声明：

```cpp
void setup();
void loop();
```

Runtime 顺序：初始化平台 → `setup()` 一次 → 事件泵 → `loop()` → 退出清理。帧率节流主要由 `Arduboy2::nextFrame()` 保持上游语义，而不是 runtime 强制固定 60 FPS。

## 6. 构建目标

预期 CMake target：

- `ardugirl_core`
- `ardugirl_arduboy2`
- `ardugirl_compat`
- `ardugirl_platform_linux_sdl2`
- `ardugirl_platform_linux_terminal`
- `game_<id>`
- `ardugirl_<id>` 最终可执行文件

平台和游戏通过 cache 变量选择：

```bash
cmake -S . -B build \
  -DARDUGIRL_PLATFORM=linux_sdl2 \
  -DARDUGIRL_GAME=hello
cmake --build build
```

## 7. 可移植性约束

- 不假设小端序，尽管首批目标通常为小端。
- 不假设指针宽度；函数指针和数据指针绝不存进 `uint16_t`。
- 不假设未对齐读取合法。
- 不假设 `char` 的 signedness。
- 只要求 8-bit byte，并以 `static_assert(CHAR_BIT == 8)` 验证。
- 时间差使用无符号减法处理 32 位回绕。
- EEPROM 和游戏存档是字节协议，不直接序列化带 padding 的宿主结构体。

## 8. 后续 MCU 接入条件

PY32/STM32 后端只有在 Linux golden tests 稳定后开始。每个后端仅需提供：初始化、按键、时间、present、音频和存储。显示为 SSD1306 时可以直接发送页面 buffer；RGB565 屏幕在后端转换。
