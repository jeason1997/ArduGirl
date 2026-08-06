# Linux 终端前端规格

终端前端是 ArduGirl 的第一实现目标，让同一份游戏在纯 TTY、SSH 和没有图形环境的设备上运行。它不是另一个游戏实现，只消费统一的 1024 字节 framebuffer 和六键状态。第一阶段构建不得依赖 SDL2。

## 显示模式

按终端能力依次选择：

1. **Braille**：Unicode Braille 每字符表达 2x4 像素，128x64 映射为 64x16 字符，细节最好。
2. **Half block**：`▀`/`▄`/`█` 每字符表达 1x2 像素，128x64 映射为 128x32 字符，要求较宽终端。
3. **ASCII**：` ` 和 `#`，用于 Unicode 不可用的环境。

默认使用 Braille。终端太小时给出诊断，可用 `--terminal-mode` 强制模式。刷新使用光标定位和行级差分，不在每帧清屏或滚屏。

## 输入

- 启动时保存 termios，进入 non-canonical、no-echo 模式。
- 退出、信号或初始化失败时恢复终端状态。
- WASD 映射方向，J/Z 映射 A，K/X 映射 B，Q/Escape 退出。
- 终端字节流没有可靠 key-up 事件，因此使用“最后一次按键事件 + 短保持窗口”合成电平。
- 第一版以单键和常见组合可玩为目标；终端输入限制必须通过测试和文档明确记录，后续再由 SDL2/gamepad 提供完整同时按键能力。

默认保持窗口建议约 80–120 ms，并允许命令行配置。测试必须使用可注入时钟，避免真实等待。

## 平台复用

终端 backend 只负责 termios/ANSI 输入输出、framebuffer 字符编码、差分刷新和退出事件。Linux 平台的终端、SDL2 和共享存储实现统一放在 `platform/linux/src/`，平台私有头文件与测试分别位于 `include/` 和 `tests/`，共享代码由两个前端复用。

终端前端第一阶段不提供音频；可以使用 terminal bell 作为显式调试选项，但不能作为 ArduboyTones 的兼容实现。

## 命令行

```text
--terminal-mode braille|half|ascii
--terminal-hold-ms N
--invert
--mute
--save-dir PATH
--frames N
```

## 测试

- 对固定 framebuffer 比较 Braille/half/ASCII 编码结果。
- 使用 pseudo-terminal 测试 raw mode、输入映射和退出后的终端恢复。
- 验证重复刷新只输出变化的行。
- 验证 SIGINT/SIGTERM 和正常退出都会恢复 termios。
- 在无 SDL2 依赖的构建配置中独立编译运行。

## 完成定义

同一个 `hello` 游戏和至少一个真实游戏可通过 `ARDUGIRL_PLATFORM=linux_terminal` 构建；在常见 UTF-8 终端中画面稳定、无滚屏，退出后终端输入回显和行模式恢复正常。
