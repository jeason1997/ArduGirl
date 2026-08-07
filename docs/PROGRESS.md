# 当前进度

最后更新：2026-08-07

本文只记录当前快照与最近一次可复核证据。详细历史由 Git 提交保存；PY32 首次移植的问题总结见 `PY32_PORTING_RETROSPECTIVE.md`。

## 状态总览

| 范围 | 状态 | 当前事实与证据 |
|---|---|---|
| 架构与构建 | partial | 根 Makefile 只转发平台目标；Linux 与 PY32 各自拥有构建规则；24 款游戏由 `game.toml` 自动发现。公共契约稳定，仍需完善兼容 API 差异审计。 |
| Linux SDL2 | partial | 唯一 Linux 前端；窗口、键盘、单调时间、headless、输入回放、截图、文件 EEPROM、崩溃诊断和音频已实现。待 gamepad、键位配置和长时间音频验证。 |
| Linux terminal | removed | 2026-08-07 确认废弃；实现、构建目标、测试、清单字段和规格文档均已删除。 |
| Arduboy2/Arduino 兼容 | partial | 覆盖当前 24 款游戏所需的绘图、Sprites、Print、按钮、帧率、PROGMEM、EEPROM 与多种音频接口；尚不等于完整上游 API。 |
| 游戏 | partial | 24 款已接入：MicroTD、ArduboyWorks 18 款、Ardynia、Arduventure、Fantasy Rampage、Helmets & Hordes、Sunfire。完成度以各游戏 README 和 `game.toml` 为准。 |
| PY32F002A | partial | 已有 24 MHz、ST7789、按钮、蜂鸣器、统一游戏构建和 OpenOCD 烧录路径；Flash EEPROM 未实现，硬件交互仍未完整验收。最近一次完整冷构建为 21/24；之后 Chri-Bocchi 与 Stairs Sweep 已单独通过，Bananonsense 仍待修复，尚未重跑全量。 |
| STM32 | not-started | 尚无实现；未来复用同一 platform API。 |

## 最近验证

- 2026-08-07：删除 terminal 后，MicroTD 的 SDL2 定向构建、固定回放和无头冒烟通过。随后执行聚合 `make test -j4` 时，Helmets & Hordes 在既有补丁 `0001-host-use-native-function-pointers.patch` 的源码准备阶段因上下文不匹配失败，因此本次不能给出新的 24 款全量通过结论。删除前最近一次全量 Linux 证据仍为全部 24 款构建、公共测试、无头冒烟与 3 款固定回放通过。
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
