# ArduGirl

ArduGirl 是一个让 Arduboy 游戏源码直接在 Linux、PY32、STM32 等平台原生编译运行的兼容层。它不是模拟器，不加载 AVR `.hex`，也不模拟 ATmega32U4 指令。

当前阶段：**规格完成，Linux 终端实现尚未开始；SDL2 排在终端基线之后。** 详细状态见 [docs/PROGRESS.md](docs/PROGRESS.md)。

## 目标

- 尽量不修改使用 `Arduboy2` 的游戏主体源码。
- 保持 Arduboy 的 128x64、1-bit、1024-byte framebuffer 和六键语义。
- 将 Arduboy2/Arduino 兼容行为与 Linux、PY32、STM32 硬件后端分离。
- 先用纯终端前端建立最小、可测试的 Linux 参考实现，再添加 SDL2，最后移植到资源受限 MCU。
- 明确处理 AVR 与 32 位平台在 `int`、指针、PROGMEM、EEPROM 和定时器方面的差异。

## 非目标

- 不执行 `.hex`、`.arduboy` 或 AVR 二进制。
- 不保证依赖 AVR 汇编、寄存器或中断实现的游戏无需修改。
- 第一阶段不实现 Arduboy FX 外置 Flash、ATMlib 或完整 Arduino SDK。
- 不收集许可证不明确的游戏源码。

## 总体结构

```text
                     +---------------------+
                     |  Arduboy game code  |
                     |  setup() / loop()   |
                     +----------+----------+
                                |
                +---------------v----------------+
                | Arduboy2 + Arduino/AVR compat  |
                +---------------+----------------+
                                |
                     +----------v----------+
                     | ArduGirl platform API|
                     +-----+----------+-----+
                           |          |
                     +-----v---+  +---v----------+
                     | Linux   |  | future MCU   |
                     | SDL2/TTY|  | PY32 / STM32 |
                     +---------+  +--------------+
```

平台 API 只传递固定宽度整数、1024 字节 framebuffer、按键状态、时间、音频命令和持久化字节；它不暴露 SDL 或任何 MCU HAL 类型。

## 计划中的目录

```text
ArduGirl/
├── AGENTS.md                 开发与 Agent 约束
├── CMakeLists.txt            顶层构建入口（下一阶段创建）
├── cmake/                    工具链、选项和游戏注册函数
├── include/
│   ├── ardugirl/             平台无关公共接口
│   ├── Arduino.h             Arduino 源码兼容入口
│   ├── EEPROM.h
│   ├── Arduboy2.h
│   └── avr/pgmspace.h
├── src/
│   ├── core/                 runtime、framebuffer、平台调度
│   ├── arduboy2/             Arduboy2 平台无关实现
│   └── compat/               Arduino、AVR、Print、EEPROM 兼容
├── platform/
│   ├── linux_terminal/       第一实现目标，ANSI/Unicode 前端
│   ├── linux_sdl2/           后续图形前端
│   ├── py32/                 后续目标
│   └── stm32/                后续目标
├── games/
│   ├── examples/             自有测试游戏
│   └── ports/<game>/         获准导入的第三方游戏
├── tests/
│   ├── unit/
│   ├── golden/               framebuffer 像素基准
│   └── games/                游戏构建/冒烟测试
├── tools/                    导入、审计、截图和资产工具
└── docs/                     规格与进度
```

暂时不创建大量空目录；实现对应阶段时按该结构落地。

## 文档导航

- [架构与平台契约](docs/ARCHITECTURE.md)
- [兼容范围和差异策略](docs/COMPATIBILITY.md)
- [性能与整数模型](docs/PERFORMANCE.md)
- [Linux SDL2 实施规格](docs/LINUX_SDL.md)
- [Linux 终端前端规格](docs/LINUX_TERMINAL.md)
- [游戏源码获取与导入](docs/GAME_PORTING.md)
- [实施路线图](docs/ROADMAP.md)
- [当前进度](docs/PROGRESS.md)
- [参考工程和资料](docs/REFERENCES.md)

## 第一阶段验收目标

第一阶段终端基线完成时，应满足：

1. CMake 能构建不依赖 SDL2 的 terminal 游戏目标。
2. 游戏继续使用标准 `setup()`、`loop()` 和 `Arduboy2` API。
3. 终端使用 Braille、半块字符或 ASCII 显示同一 framebuffer。
4. 六键、帧率、`millis()`、按钮边沿行为正确。
5. EEPROM 按游戏隔离并保存到文件。
6. 官方示例及至少三个许可证清晰的真实游戏通过构建和冒烟测试。
7. AddressSanitizer/UndefinedBehaviorSanitizer 和 framebuffer golden tests 通过。

## 关于 8 位和 32 位整数

Arduboy 的 AVR ABI 中 `int` 为 16 位，而 Linux、PY32 和绝大多数 STM32 环境中 `int` 为 32 位。ArduGirl 不会全局伪造一个 16 位 `int`，因为这既不符合 C++ ABI，也无法可靠实现。

原则是：

- framebuffer、图片、地图、存档和结构体字段使用最小且明确的固定宽度类型。
- 坐标等 Arduboy2 API 保持上游规定的 `int16_t`/`uint8_t`。
- 32 位 CPU 的局部计算可以使用自然字宽；8 位值参与表达式时本来也通常会整数提升。
- 对依赖 AVR 16 位溢出或对象大小的游戏进行静态审计并做最小补丁。

因此，32 位 `int` 确实可能让 `int[]` 和含 `int` 字段的对象变大，但不会让每个临时变量都固定占用四字节 RAM；优化后很多临时值存在寄存器中。完整规则见 [docs/PERFORMANCE.md](docs/PERFORMANCE.md)。

## 许可证原则

ArduGirl 自身许可证将在引入实现代码前确定。第三方库和游戏保持各自许可证与归属；仅有可下载二进制不代表允许再分发源码或资产。
