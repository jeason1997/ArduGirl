# 当前进度

最后更新：2026-08-06

## 总览

| 领域 | 状态 | 证据/说明 |
|---|---|---|
| 项目方向 | done | 已确定源码直接编译，不采用模拟器 |
| Git 仓库 | done | 本地 `main` 已初始化，`origin` 已绑定空远程仓库 |
| 架构规格 | done | `ARCHITECTURE.md` |
| 兼容范围 | done | `COMPATIBILITY.md` |
| 性能/整数规范 | done | `PERFORMANCE.md` |
| Linux SDL 规格 | done | `LINUX_SDL.md`；默认 SDL2 |
| Linux terminal 规格 | done | `LINUX_TERMINAL.md`；Braille/half/ASCII |
| 游戏导入规范 | done | `GAME_PORTING.md` |
| Agent 约束 | done | 根目录 `AGENTS.md` |
| 构建系统 | done | GNU Make；`make`、`run`、`demo`、`test` |
| framebuffer core | done | 1024 字节页面布局、像素、直线、矩形及单元测试 |
| Linux terminal backend | partial | Braille 显示、raw input、固定帧模式已运行；待差分刷新和 PTY 测试 |
| 官方 Arduboy2 示例 | partial | 未修改的官方 HelloWorld 已构建运行；当前仅覆盖所需最小 API/字形 |
| Linux SDL2 backend | deferred | 终端兼容基线稳定后开始 |
| Arduboy2 兼容实现 | not started | 等最小 backend/runtime |
| 游戏源码 | not started | 尚未选择，需先逐个核对许可证 |
| 音频 | not started | Linux baseline 后实现 |
| PY32 | planned | 需先指定芯片/板卡/屏幕 |
| STM32 | planned | Linux/PY32 之后 |

## 已完成

- 建立项目目标、非目标和依赖边界。
- 定义 framebuffer、按键、时间、存储和音频的平台契约。
- 说明 AVR 16 位 `int` 与 32/64 位宿主 ABI 的兼容策略。
- 规定第三方游戏的来源、许可证和版本固定方式。
- 列出 Linux → Arduboy2 → 游戏 → 音频 → PY32 的实施顺序。
- 在 WSL2 使用 GCC 成功构建并运行 `hello` 终端示例。
- 实现 128x64 framebuffer 到 64x16 Unicode Braille 的转换。
- 实现 WASD、A/B、退出键输入和非交互固定帧模式。
- 添加 framebuffer 位布局、裁剪、矩形和清屏单元测试。
- 以 Git 子模块固定 Arduboy2 `bc460a2`，未修改的官方 `HelloWorld.ino` 已在终端显示。
- 自写图形示例已移出 `games/`，仅保留为底层 smoke test。
- 确定游戏与 Arduboy2 兼容层使用 C++，未来 MCU 平台边界保持可由 C HAL 实现。

## 下一步

1. 确定 ArduGirl 自身开源许可证。
2. 为终端渲染器加入行级差分刷新和 pseudo-terminal 测试。
3. 从官方 Arduboy2 字体与绘图行为继续补齐通用兼容实现。
4. 添加更完整的 golden test 和 sanitizers。
5. 终端跑通真实游戏基线后再实现 SDL2。

## 未决策项

- ArduGirl 自身许可证（MIT、BSD-3-Clause 等）。
- 第一个 PY32 的确切芯片、开发板、屏幕接口和编译工具链。
- 第三方测试游戏名单；必须先完成许可证核查。
- Arduboy2 上游使用原始文件加平台补丁，还是维护窄范围 fork；应在 Phase 2 通过 diff 规模决定。
