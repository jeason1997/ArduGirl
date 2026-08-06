# 实施路线图

## Phase 0：规格与治理

- [x] 明确源码级原生编译，不采用模拟器。
- [x] 定义目录、分层、平台边界和游戏导入规则。
- [x] 定义整数、内存和性能规则。
- [x] 固定上游版本与零修改导入流程。

## Phase 1：Linux 运行基线

- [x] GNU Make 聚合构建和游戏扩展点。
- [x] Arduino `setup/loop` runtime 和 platform API。
- [x] Linux terminal Braille 显示、raw input 和 headless 固定帧运行。
- [x] SDL2 窗口、键盘事件、最近邻整数缩放和 headless 模式。
- [x] Linux EEPROM 文件后端与游戏隔离。
- [ ] SDL2 gamepad 输入和键位配置。

## Phase 2：Arduboy2 与 Arduino 兼容

- [x] 固定 Arduboy2 上游基线。
- [x] 128×64、1-bit、1024 字节 framebuffer。
- [x] 高频图元、bitmap、compressed bitmap 和 Sprites 模式。
- [x] 帧率、按钮电平与边沿、字体和基础 Print。
- [x] PROGMEM、安全读取、时间、随机数和 EEPROM。
- [x] framebuffer、兼容 API、SDL 和存储回归测试。
- [ ] 对照固定上游版本完成 API 差异清单。
- [ ] 补齐仍有价值的 Arduboy2/Print 辅助接口和官方示例编译测试。

## Phase 3：声音与扩展库

- [x] Arduboy2Beep 和定时 tone。
- [x] ArduboyPlaytune 音符、等待、重复、移调和双声道调度。
- [x] SDL2 双方波混合与 8 位波表播放。
- [ ] 增加确定性 PCM golden test、抢占和长时间播放验证。
- [ ] 新游戏实际依赖时再移植 ArduboyTones、ATMlib、ArdBitmap 或 ArduboyFX。

## Phase 4：真实游戏基线

- [x] 接入 MicroTD 和 ArduboyWorks 18 个游戏。
- [x] 全部 19 个游戏完成冷构建和启动冒烟。
- [x] MicroTD 固定输入回放覆盖选图、建塔和启动波次。
- [ ] 为 ArduboyWorks 游戏逐个增加固定输入回放、截图和玩法验证。
- [ ] 为代表游戏增加存档往返与长时间运行验证。
- [ ] 按 `GAME_PORTS.md` 继续导入不同依赖类型的游戏。

## Phase 5：质量基线

- [ ] 增加 GCC/Clang、Debug/Release 和 ASan/UBSan 构建矩阵。
- [ ] 增加兼容性静态审计脚本。
- [ ] 增加时间回绕、帧调度、音频混合和复杂压缩图片测试。
- [ ] 清理仍依赖 `-fpermissive` 的上游适配问题。

## Phase 6：PY32

- [ ] 选定具体芯片、开发板、屏幕、引脚、工具链和存储方案。
- [ ] 实现 GPIO、SysTick/timer、显示、PWM 和 Flash storage backend。
- [ ] 使用 Linux golden tests 作为像素参考。
- [ ] 输出 Flash/RAM/栈预算和帧时间。

具体硬件未确定前不写厂商 HAL 代码，避免错误绑定外设和容量假设。

## Phase 7：STM32 和其他平台

- [ ] 以相同 platform API 添加 STM32 backend。
- [ ] 区分 SSD1306 直传和 RGB565 scanline 转换。
- [ ] 建立每个平台的板级配置，不复制核心兼容实现。
