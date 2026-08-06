# 实施路线图

## Phase 0：规格与治理

- [x] 明确源码级原生编译，不采用模拟器。
- [x] 定义目录、分层、平台边界和游戏导入政策。
- [x] 定义整数、内存和性能规则。
- [x] 确定 Linux 纯终端为第一平台，SDL2 在终端基线后实现。
- [ ] 确定 ArduGirl 自身许可证。

## Phase 1：最小 Linux 可执行程序

- [x] 顶层 Makefile 和 `hello` 测试游戏。
- [ ] 最小 Arduino `setup/loop` runtime。
- [ ] platform API 和 fake test backend。
- [ ] Linux terminal 的 Braille/half-block 显示和 raw input。
- [ ] headless 固定帧运行。

验收：自有 hello 游戏能在不安装 SDL2 的环境中以 terminal 目标运行、响应六键，并在 CI/headless 中产生稳定 framebuffer hash。

## Phase 2：Arduboy2 核心兼容

- [ ] 选择并记录 Arduboy2 上游基线 commit。
- [ ] 分离/替换 AVR `Arduboy2Core`。
- [ ] 基本图元、bitmap、frame timing。
- [ ] 按钮状态和边沿。
- [ ] 字体和 Print 基础。
- [ ] framebuffer golden tests。

验收：Arduboy2 官方 HelloWorld、Buttons、Sprites 等代表性示例通过。

## Phase 3：Arduino/AVR 兼容

- [ ] `PROGMEM` 和安全读取。
- [ ] Arduino 时间、随机数和辅助宏/函数。
- [ ] EEPROM 文件后端。
- [ ] 兼容性审计工具。
- [ ] GCC/Clang + sanitizers 构建矩阵。

## Phase 4：真实游戏基线

- [ ] 选择三个许可证清晰、复杂度递增的游戏。
- [ ] 为每个游戏加入 `game.toml`、许可证和固定 revision。
- [ ] 建立 build/smoke/golden tests。
- [ ] 记录每个源码补丁及其原因。

验收：三个游戏可在 Linux 原生运行，游戏逻辑无 SDL/Linux 条件代码。

## Phase 5：SDL2 图形前端

- [ ] SDL2 窗口、事件和 framebuffer 最近邻显示。
- [ ] SDL2 键盘与 gamepad 输入。
- [ ] 同一游戏可在 terminal 与 SDL2 目标之间切换且无需修改源码。
- [ ] SDL2 单声道方波。

## Phase 6：声音与扩展库

- [ ] Arduboy2Beep。
- [ ] ArduboyTones。
- [ ] 根据游戏需求评估 ArdBitmap/ArduboyFX。

## Phase 7：PY32

- [ ] 选定具体 PY32 型号、板卡、屏幕、引脚、工具链和存储方案。
- [ ] 实现 GPIO、SysTick/timer、显示、PWM 和 Flash storage backend。
- [ ] 使用 Linux golden tests 作为像素参考。
- [ ] 输出 Flash/RAM/栈预算和帧时间。

PY32 型号未确定前不写厂商 HAL 代码，避免错误绑定外设和容量假设。

## Phase 8：STM32 和其他平台

- [ ] 以相同 platform API 添加 STM32 backend。
- [ ] 区分 SSD1306 直传和 RGB565 scanline 转换。
- [ ] 建立每个平台的板级配置，而不是 fork 核心。
