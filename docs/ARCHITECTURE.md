# 架构与平台契约

## 1. 分层

ArduGirl 使用四层结构：

1. **Game**：原始游戏逻辑、资源、`setup()` 和 `loop()`。
2. **Compatibility**：Arduboy2、Sprites、Arduino 和有限 AVR 兼容 API。
3. **Core**：统一 runtime、1024 字节 framebuffer 和平台无关服务。
4. **Backend**：Linux 使用 SDL2；PY32F002A 已有首版实验后端，STM32 尚未实现。早期 terminal 后端已经废弃并删除。

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
    bool plain_output;
    bool headless;
    bool fullscreen;
    bool invert;
    std::uint8_t scale;
    const char* title;
    const char* game_id;
    const char* save_dir;
};

bool init(const Config&) noexcept;
void shutdown() noexcept;
bool pump_events() noexcept;       // false 表示请求退出
std::uint8_t buttons() noexcept;

std::uint32_t millis() noexcept;
std::uint32_t micros() noexcept;
void idle_until(std::uint32_t deadline_us) noexcept;

void present(const std::uint8_t (&framebuffer)[1024]) noexcept;

void set_tone(std::uint16_t frequency_hz,
              std::uint8_t channel = 0) noexcept;
void stop_tone(std::uint8_t channel = 0) noexcept;
void play_wave(std::uint16_t sample_rate_hz,
               const std::uint8_t* samples,
               std::uint16_t sample_count) noexcept;
void stop_wave() noexcept;

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

当前最小实现使用 GNU Make，生成：

- `build/microtd-sdl`
- `build/framebuffer-test`

根目录 Makefile 只负责选择平台 Makefile 和转发用户目标，不声明编译器参数、平台源码、公共测试对象或游戏构建细节。Linux 构建实现位于 `platform/linux/Makefile`，PY32 构建实现位于 `platform/py32/Makefile`。每款游戏以 `game.toml` 作为唯一必需的构建描述；通用准备器递归生成已规范换行并应用补丁的源码快照和统一入口，各平台只消费这份共同结果。新增普通游戏不得再创建 `port.mk` 或手写入口文件。

Linux 平台先用一次清单扫描生成轻量 Make 元数据，再由单一模板展开编译、链接、冒烟和回放目标。指定 `GAME=<game-id>` 时只扫描和展开选中的游戏，并只加载它的 `.d` 文件；不得先展开全部游戏再过滤，因为 GNU Make 在判断目标是否最新之前就会解析依赖文件，在 WSL 挂载的 Windows 文件系统上会形成显著启动延迟。未指定 `GAME` 的 `all`、`test` 等聚合目标才展开全部清单。

构建入口在读取上游源码前按清单解析最小子模块集合。指定游戏时只自动初始化公共 Arduboy2 与该游戏入口所属子模块；不允许为了单游戏构建初始化无关游戏。未指定游戏的聚合构建才初始化全部清单对应的上游。初始化固定使用 gitlink 记录的 revision，不跟踪远端分支。

同仓库多游戏可以在各自 `game.toml` 的 `[build] profile` 中显式引用集合级 `profile.toml`。profile 只描述该上游集合共有的源码排除、附加适配翻译单元、编译兼容选项和可校验源码转换；不得包含 Linux、SDL、MCU、工具链或最终目标规则，也不得由构建器按父目录隐式猜测。普通游戏不需要 profile。

`make`、具体游戏目标和 `make test` 使用 SDL2。当平台数量和工具链配置增长后，再评估是否加入下列 CMake targets：

- `ardugirl_core`
- `ardugirl_arduboy2`
- `ardugirl_compat`
- `ardugirl_platform_linux_sdl2`
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

## 8. C++ 游戏层与 C 平台 ABI

Arduboy2Beep 属于 `src/arduboy2`，ArduboyTones 与 ArduboyPlaytune 属于 `src/compat` 中的独立官方生态库兼容实现。音符序列或乐谱的解析、等待、重复、移调和声道状态不得进入平台后端；平台只接收最多两个方波声道的频率，或一段带固定采样率和生命周期契约的 8 位无符号波表。游戏专用适配只能处理上游私有优先级等扩展并转发到公共播放器，不得复制序列或乐谱解释器。

ATMlib 同样位于 `src/compat`：兼容层解释四声道 tracker 乐谱并按单调时间推进节拍，平台只接收固定宽度的频率、音量和波形编号快照。SDL 回调负责生成采样但不读取乐谱或游戏对象。这样既保留上游音乐语义，也不把 AVR Timer4、ISR 或寄存器引入公共接口。

Arduboy2 游戏依赖类、方法、重载和 Arduino `Print`，所以游戏与兼容层必须使用 C++ 编译。把整个项目改写为纯 C 会迫使我们修改上游游戏源码，不符合项目目标。

`Arduboy2Base` 在当前实现中与 `Arduboy2` 共享实现，但仍公开上游约定的名称和 `sBuffer` 静态入口。`sBuffer` 直接指向唯一的 1024 字节核心 framebuffer，不复制画面。`Arduboy2Audio` 的静态开关接口与实例 `audio` 对象共享同一状态，确保同时使用两种官方 API 风格的游戏得到一致结果。

游戏私有代码中的 AVR 整数宽度假设或游戏逻辑越界不得通过改变 core 整数模型来掩盖，也不得下沉到平台后端。此类问题保存在对应游戏的可重放最小补丁中，生成后的修复源码由所有平台目标共同编译。Linux 致命信号与调用栈属于后端诊断能力，仅 Linux 运行目标链接 POSIX 实现和 `-rdynamic`；未来 MCU 后端可独立接入 HardFault 或异常向量，不把 POSIX 能力加入公共平台契约。

平台边界应保持可由 C 实现：公共结构只使用固定宽度整数、指针和普通字节缓冲区，不在 ABI 中暴露 C++ 类、模板、异常或标准库容器。PY32/STM32 后端可以使用厂商 C HAL，实现 `extern "C"` 平台函数，再由薄 C++ 包装连接到兼容层。

只提供 C 编译器、完全没有 C++ 前端的平台无法直接编译原生 Arduboy 游戏，因此不列为支持目标。

## 9. MCU 接入条件

PY32/STM32 后端只有在 Linux golden tests 稳定后开始。每个后端仅需提供：初始化、按键、时间、present、音频和存储。显示为 SSD1306 时可以直接发送页面 buffer；RGB565 屏幕在后端转换。PY32 内部另设不进入公共 API 的显示驱动接口，使具体屏幕可以在构建时替换；当前 ST7789 实现仅消费 128×64 页面 buffer，不改变核心布局。

MCU 构建由对应平台目录所有。游戏的公共构建描述以既有 `game.toml` 为数据源，通用准备器负责解析上游入口、补丁和生成物；不得为每个“平台×游戏”组合增加适配文件。游戏描述不得包含芯片型号、厂商 SDK、工具链、启动文件、链接脚本、具体显示驱动或烧录器配置。MCU 厂商底层代码必须作为固定 revision 或可审计快照进入对应平台自己的 `vendor/`，不得占用仅用于游戏与 Arduboy 生态上游的根 `third_party/`，平台构建也不得依赖开发机上的绝对路径。
