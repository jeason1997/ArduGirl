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
| 构建系统 | not started | 仓库尚无 CMake/code |
| Linux terminal backend | not started | 下一阶段唯一实现目标 |
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

## 下一步

1. 确定 ArduGirl 自身开源许可证。
2. 创建 CMake、platform headers、fake backend 和 `games/examples/hello`。
3. 用 hello 游戏接入 terminal framebuffer、差分输出和 raw input。
4. 添加 headless/golden test 后再开始 Arduboy2 上游代码整理。
5. 终端跑通真实游戏基线后再实现 SDL2。

## 未决策项

- ArduGirl 自身许可证（MIT、BSD-3-Clause 等）。
- 第一个 PY32 的确切芯片、开发板、屏幕接口和编译工具链。
- 第三方测试游戏名单；必须先完成许可证核查。
- Arduboy2 上游使用原始文件加平台补丁，还是维护窄范围 fork；应在 Phase 2 通过 diff 规模决定。
