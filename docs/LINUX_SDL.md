# Linux SDL2 平台规格

## 版本选择

SDL2 是当前默认 Linux 后端，以覆盖更多现有 Linux 发行版、开发环境和嵌入式 Linux 系统。平台抽象不暴露 SDL 类型，未来增加 SDL3 backend 时不修改游戏或兼容层。

## 功能范围

### M0：窗口与 framebuffer

- 创建标题为游戏名的窗口。
- 默认窗口为 128x64，按 1:1 显示逻辑像素；可通过 `--scale N` 指定整数缩放倍率。
- 使用最近邻采样，禁止模糊。
- 默认黑底白像素；允许命令行反色但不改变核心 buffer。
- 窗口缩放保持宽高比和整数优先的 letterbox。

### M1：输入与 runtime

默认键位：

| Arduboy | 键盘 |
|---|---|
| D-pad | Arrow / WASD |
| A | Z / J |
| B | X / K |
| Quit | Escape / window close |

事件泵在每次 `loop()` 前执行。按键接口提供当前电平；边沿由 Arduboy2 层计算。后续可添加 SDL Gamepad，但不是首个验收条件。

### M2：时间

- 来源为 SDL 单调高精度计时接口。
- 暴露 Arduino 风格的低 32 位 micro/millisecond 值。
- `delay` 和 `nextFrame` 不依赖 wall clock。
- 测试必须覆盖 `uint32_t` 回绕。

### M3：EEPROM

- 默认逻辑容量 1024 bytes，初始值为 `0xFF`。
- 每个游戏使用独立文件：XDG data 目录下 `ardugirl/<game-id>/eeprom.bin`。
- 测试中允许通过环境变量/参数指定临时目录。
- 写入采用 RAM shadow + dirty 标记；退出或显式提交时原子替换。
- 文件头可由 ArduGirl 管理，但游戏可见的 1024 bytes 必须保持稳定。

### M4：音频

SDL2 audio callback 以 48 kHz、16 位单声道混合最多两个方波声道和一条 8 位无符号波表流。波表使用定点相位按调用方给出的采样率重采样，播放到样本末尾后自动停止。游戏线程只通过加锁后的状态启动或停止声音，音频回调不得访问或修改游戏对象。无音频设备时初始化继续成功并静音降级。当前覆盖定时 tone、Arduboy2Beep、ArduboyPlaytune 和 ArduboyWorks `playWave`；ArduboyTones 与 ATMlib 待出现实际依赖后单独移植。

## 命令行接口

当前支持：

```text
--scale N
--fullscreen
--invert
--mute
--save-dir PATH
--headless
--frames N
```

`--save-dir PATH` 已实现，用于覆盖 XDG data 根目录；最终文件仍按 `<game-id>/eeprom.bin` 隔离。`--mute` 将随音频后端实现。

`--headless --frames N` 用于 CI smoke test，不初始化可见窗口并执行固定帧。SDL 后端测试使用内部纯转换函数验证 framebuffer 的正常与反色 ARGB8888 golden hash，不向公共平台接口暴露 SDL 类型。

## 构建依赖

- CMake 3.24+
- C++17 编译器（GCC/Clang）
- SDL2 development package 和 CMake/pkg-config metadata
- 测试框架优先使用轻量、可 vendoring 的方案；最终选择在实现阶段记录

预期链接方式：

```cmake
find_package(SDL2 REQUIRED CONFIG)
target_link_libraries(ardugirl_platform_linux_sdl2 PRIVATE SDL2::SDL2)
```

## 测试

- framebuffer 全白、棋盘、边界像素和 sprite golden images。
- 人工事件注入验证同时按键和边沿。
- 当前已通过 SDL 事件队列覆盖组合键、释放、自动重复过滤和退出事件。
- 当前已通过 8x8 棋盘图案的正常与反色 ARGB8888 golden hash 验证页面布局转换。
- fake clock 验证 30/60 FPS 以及时间回绕。
- EEPROM 首次启动、重启保持、损坏/短文件恢复。
- 当前已覆盖首次擦除态、同游戏重启保持、不同游戏隔离、越界拒绝和短文件恢复。
- headless 模式在无显示服务器的 CI 中运行。
- ASan/UBSan 下运行至少一个官方示例和三个真实游戏。

## 完成定义

SDL2 backend 只有在窗口、输入、单调时间、headless 测试和按游戏隔离 EEPROM 全部验证后才标记完成。音频作为独立里程碑，可在最初 Linux baseline 后完成。
