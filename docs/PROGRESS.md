# 当前进度

最后更新：2026-08-08

本文只记录当前快照与最近一次可复核证据。详细历史由 Git 提交保存；PY32 首次移植的问题总结见 `PY32_PORTING_RETROSPECTIVE.md`。

## 状态总览

| 范围 | 状态 | 当前事实与证据 |
|---|---|---|
| 架构与构建 | partial | 根 Makefile 只转发平台目标；Linux 与 PY32 各自拥有构建规则；31 款游戏由 `game.toml` 自动发现，单游戏构建只按需初始化其上游与公共 Arduboy2 子模块。公共契约稳定，仍需完善兼容 API 差异审计。 |
| Linux SDL2 | partial | 唯一 Linux 前端；窗口、键盘、单调时间、headless、输入回放、截图、文件 EEPROM、崩溃诊断和音频已实现。待 gamepad、键位配置和长时间音频验证。 |
| Linux terminal | removed | 2026-08-07 确认废弃；实现、构建目标、测试、清单字段和规格文档均已删除。 |
| Arduboy2/Arduino 兼容 | partial | 覆盖当前 26 款游戏所需的绘图、Sprites、Print、按钮、帧率、PROGMEM、EEPROM、ArduboyTones 与多种音频接口；尚不等于完整上游 API。 |
| 游戏 | partial | 31 款已接入：原有 26 款及 Ravine Despoiler、Evade 2、Blackjack、Waternet、Pipes。新五款均已通过 SDL2 冷构建、冒烟和固定输入截图路径，未完成流程以各游戏 README 和 `game.toml` 为准。 |
| PY32F002A | partial | 已有 48 MHz、ST7789、按钮、PA0/PA1 差分蜂鸣器、六声部软件合成、统一游戏构建和 OpenOCD 烧录路径；Flash EEPROM 未实现，音频听感与硬件交互仍未完整验收。最近一次完整冷构建为 21/24；之后 Chri-Bocchi 与 Stairs Sweep 已单独通过，Bananonsense 仍待修复，尚未重跑全量。 |
| STM32 | not-started | 尚无实现；未来复用同一 platform API。 |

## 最近验证

- 2026-08-10：修正 WSL 首次安装 Puya OpenOCD 时的 Autotools 生成文件、libusb/hidapi 依赖和新版 GCC `-Werror` 兼容问题；usbipd 转发 `c251:f001 DAPLink CMSIS-DAP` 后，Rooftop Rescue 已通过 PY32F002A 写入、校验和复位，OpenOCD 报告 `Verified OK`。
- 2026-08-08：待移植清单前五款 Ravine Despoiler、Evade 2、Blackjack、Waternet、Pipes 完成源码级接入与 SDL2 冷构建；补齐 FixedPoints、Print、Arduboy2Core、Arduboy2Audio、Arduboy2Beep 和 ArduboyTones 音高兼容，固定输入生成并目视验收每款三张阶段截图。Evade 2 的 ATMLib2 音频、Blackjack 完整牌局、Waternet/Pipes 完整解谜仍标为 partial。
- 2026-08-08：Rooftop Rescue SDL2 与 PY32F002A 冷构建通过；PY32 尺寸为 `text=29164`、`data=112`、`bss=3416`。新增公共 ArduboyTones 兼容层及序列推进测试；固定输入回放验证标题、进入玩法、直升机换楼和放绳，标题、玩法、游戏内菜单三张截图已目视验收并嵌入游戏 README。
- 2026-08-07：PY32 蜂鸣器跨接 PA0/PA1，使用 AF13 的 TIM1_CH3/CH4 输出 187.5 kHz 差分硬件 PWM，承载 50 kHz 混合采样；此前 AF12 配置错误导致定时器内部运行但引脚无声，普通 GPIO PDM 路径则经实机确认有明显刺耳噪声。Playtune 两个方波声道与 ATMlib 四个声部均独立合成，ATMlib 使用上游对应的 25% 脉冲、方波、三角波和 LFSR 噪声，并为四声部保留一位余量以避免削波。仍需烧录后验证听感和长期帧率。
- 2026-08-07：PY32 未连接实体按键时固定报告六键全部松开，避免悬空 GPIO 在 Twotris、Arduventure 等游戏启动首帧产生虚假 A/B 边沿；ST7789 后端新增板级 RGB565 黑白调色板，使用 256 字节扫描线缓冲区和 16 位 SPI DMA，并跳过未变化 framebuffer 的重复传输。参考同一样机工程启用 48 MHz 隐藏 PLL 配置；Sprites 的页面对齐路径改为直接按 framebuffer 字节复制或合成。Arduventure 烧录校验后复位运行 5 秒，帧计数为 303、`globalCounter` 为 254，确认开场文字按约 60 FPS 完成并进入标题；长时间稳定性仍待验收。
- 2026-08-07：Twotris SDL2 与 PY32F002A 冷构建通过；PY32 尺寸为 `text=14468`、`data=68`、`bss=2772`。SDL 固定输入从标题进入菜单和双人核心玩法，三张不同状态截图已目视验收并嵌入游戏 README。为其补齐了 Arduboy2 公共 `drawChar` 兼容 API。
- 2026-08-07：修正 Sunfire 既有补丁的空白上下文标记；3 个补丁通过 `git apply --check --whitespace=error-all`，PY32F002A 并行冷构建通过，尺寸为 `text=15408`、`data=44`、`bss=2780`。
- 2026-08-07：新增按游戏清单解析的子模块准备步骤；单游戏选择不会展开其他游戏上游，并由独立 Python 回归测试覆盖最小集合与聚合集合。
- 2026-08-07：删除 terminal 后，MicroTD 的 SDL2 定向构建、固定回放和无头冒烟通过。随后聚合 `make test -j4` 曾因 Helmets & Hordes 的补丁格式错误中止；该游戏的 5 个补丁现已通过 `git apply --check --whitespace=error-all`，PY32F002A 并行冷构建通过，尺寸为 `text=17908`、`data=40`、`bss=3008`。尚未重跑 Linux 聚合测试，因此删除前最近一次全量 Linux 证据仍为全部 24 款构建、公共测试、无头冒烟与 3 款固定回放通过。
- 2026-08-07：PY32 最近一次完整冷构建退出码非零，通过 21/24；失败为 Bananonsense、Chri-Bocchi、Stairs Sweep。随后 Chri-Bocchi 和 Stairs Sweep 分别独立冷构建通过，但不据此改写全量结果。
- 2026-08-06：Arduventure 与 Sunfire 已通过 CMSIS-DAP/OpenOCD 写入和校验；寄存器与帧计数证明 CPU、GPIO 和 SPI 在运行。按钮、声音听感、完整画面与长期稳定性仍需分别验收。

## 下一步

1. 修复 Bananonsense 的 PY32 构建，并重跑完整 24 款冷构建；只有最终退出码为 0 才更新为全量通过。
2. 对 PY32 做实机画面、按钮、蜂鸣器、60 FPS 和长时间运行验收，并实现 Flash EEPROM。
3. 将已有游戏截图输入固化为逐游戏回放断言，继续补足完整玩法、音频和存档验证。
4. 对照固定 Arduboy2 上游版本建立 API 差异清单，按真实游戏需求补齐接口。
5. 增加 GCC/Clang、Debug/Release 和 sanitizer 质量矩阵。

## 本次文档治理

- 根 `AGENTS.md` 已缩减为全仓不变量；游戏、Linux、PY32 规则分别下沉到最近的目录级 `AGENTS.md`。
- 新增 `docs/README.md`，明确架构、规格、状态、路线与历史文档的唯一职责。
- 删除已废弃的 Linux terminal 后端及其规格，修正 README、架构、兼容性、路线图和游戏清单中的现行描述。
- 统一 24 款游戏 README 的 Linux 命令为 `make PLATFORM=linux GAME=<game-id>`，区分构建并运行、仅构建和定向测试；删除失效的 ArduboyWorks 集合目标，并纠正把截图路径误写成自动回放、README 与 `game.toml` 状态不一致等描述。
