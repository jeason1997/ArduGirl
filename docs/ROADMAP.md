# 实施路线图

本文件只保留未完成方向。当前完成度与最近证据见 `PROGRESS.md`，已完成工作的演进历史见 Git。

## 1. Linux SDL2 质量基线

- [ ] 增加 gamepad 输入与可配置键位。
- [ ] 增加确定性 PCM golden test、声音抢占和长时间播放验证。
- [ ] 建立 GCC/Clang、Debug/Release 与 ASan/UBSan 构建矩阵。
- [ ] 增加时间回绕、帧调度和复杂压缩图片测试。

Linux terminal 已废弃，不再属于路线图。

## 2. 兼容层完整性

- [ ] 对照固定 Arduboy2 revision 建立 API 差异清单。
- [ ] 补齐真实游戏需要的 Arduboy2、Print 和 Arduino 辅助接口。
- [ ] 清理仍依赖 `-fpermissive` 的上游适配。
- [ ] 仅在新游戏出现真实依赖时评估 ArduboyTones、ArdBitmap 或 ArduboyFX。

## 3. 游戏验收

- [ ] 为 24 款已接入游戏逐一固化回放、玩法、音频与存档证据。
- [ ] 继续按 `GAME_PORTS.md` 导入不同依赖类型的游戏。
- [ ] 保持每款游戏独立的清单、冷构建、冒烟、回放和多阶段截图验收。

## 4. PY32F002A

- [ ] 修复剩余游戏构建并完成全量 24 款冷构建。
- [ ] 完成实机显示、按钮、蜂鸣器、帧率和长时间稳定性验收。
- [ ] 实现掉电持久化的 Flash EEPROM 后端。
- [ ] 固化 Flash、RAM、栈和帧时间预算。

## 5. 后续平台

- [ ] Linux 与 PY32 基线稳定后，以相同 platform API 添加 STM32 backend。
- [ ] 为每个平台建立板级配置，并保持 SSD1306 直传与 RGB565 后端转换的边界。
- [ ] 不复制核心兼容实现，不让平台类型泄漏到公共接口。
