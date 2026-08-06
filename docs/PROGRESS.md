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
| 社区游戏移植清单 | done | `GAME_PORTS.md`；只收录源码可访问的游戏，47 款合并为单表并按综合移植优先级排序 |
| Agent 约束 | done | 根目录 `AGENTS.md` |
| 构建系统 | done | GNU Make；`make`、`run`、`demo`、`test` |
| framebuffer core | done | 1024 字节页面布局、像素、直线、矩形及单元测试 |
| Linux terminal backend | partial | Braille 显示、raw input、固定帧模式已运行；按当前决策暂停效果优化 |
| 官方 Arduboy2 示例 | partial | 未修改的官方 HelloWorld 已构建运行；当前仅覆盖所需最小 API/字形 |
| MicroTD | partial | CC0 子模块源码未修改；固定输入回放已验证进入地图、建塔和启动波次；待持久化和完整游玩验证 |
| ArduboyWorks 游戏集 | partial | 整仓已固定到 `d4b1f041`；18 个游戏均已建立独立元数据和共享构建目标；Hollow Seeker、Hopper、Chri-Bocchi Cat 已构建并通过 180 帧 SDL2 无头冒烟，其余游戏正在补齐兼容层 |
| Linux SDL2 backend | partial | 已实现窗口、最近邻整数缩放、键盘输入、单调计时、headless、事件注入、framebuffer golden test 和 EEPROM 文件后端；音频待实现 |
| Arduboy2 兼容实现 | not started | 等最小 backend/runtime |
| 游戏源码 | not started | 尚未选择，需先逐个核对许可证 |
| 音频 | not started | Linux baseline 后实现 |
| PY32 | planned | 需先指定芯片/板卡/屏幕 |
| STM32 | planned | Linux/PY32 之后 |

## 已完成

- 按多游戏同仓库的新规则整体引入 obono/ArduboyWorks，并为 README 列出的 18 个成品游戏建立独立 `game.toml` 与共享构建目标；首批 Hollow Seeker、Hopper、Chri-Bocchi Cat 已通过构建和 180 帧无头冒烟。

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
- 引入未修改的 MicroTD `0c8958f`，补齐字体、图元、Sprites、按钮边沿、EEPROM 内存接口和静音 Beep 接口；终端主菜单已显示，注入 A 键后 framebuffer 发生预期变化。
- 明确上游版本锁定策略：默认禁止跟踪最新分支，只有用户明确要求时才移动并重新验证固定 SHA。
- 修复终端方向键：解析 `ESC [ A/B/C/D` 和 `ESC O A/B/C/D`，单独 Escape 延迟判定为退出，并加入回归测试。
- 新增 SDL2 后端并设为默认构建和测试平台；HelloWorld 与 MicroTD 均可在无显示服务器的 headless 模式执行固定帧测试。
- SDL2 窗口默认使用 1:1 像素倍率，并保留 `--scale N` 整数倍率覆盖。
- 保留终端构建、运行和回归测试目标，但按当前决策暂停终端显示效果优化。
- 实际操作 MicroTD 终端前端进入地图、建塔菜单和敌人波次，从 128x64 framebuffer 生成多张未缩放截图，并为 README 补充游戏简介；后续每个移植游戏都必须提供 ArduGirl 实际运行截图。
- 为 SDL2 后端增加真实事件队列注入回归，覆盖组合按键、释放、自动重复过滤和退出；拆分无 SDL 类型的内部 framebuffer 转换函数，并以棋盘图案正常/反色 golden hash 验证 ARGB8888 输出。
- 为 MicroTD 增加固定输入回放，验证选择地图、打开建塔菜单、建造防御塔和启动敌人波次；回放发现上游成功建塔路径缺失布尔返回值，现通过生成目录中的独立最小补丁消除未定义行为，子模块保持 clean。
- 实现 SDL/终端共享的 Linux EEPROM 文件后端：默认使用 XDG data 目录，支持 `--save-dir` 覆盖，按稳定 game-id 隔离，并在退出时通过临时文件替换；回归测试覆盖首次启动、重启保持、游戏隔离、越界和短文件恢复。
- 调研 Arduboy 官网精选、社区热门讨论与社区游戏目录，新增 `GAME_PORTS.md`；已收集热门候选，后续按源码完整性和兼容风险重新分层。
- 按个人研究和自用场景调整根目录 `AGENTS.md`：游戏候选改为以完整源码可获取为准，不再以许可证或再分发条件作为导入门槛；仍要求固定上游版本、记录作者和来源，并完成构建与冒烟验证。
- 按新准入规则重整 `GAME_PORTS.md`：删除无源码候选和梯队划分，将 47 款源码可访问游戏合并为一张表，并按热度、验证价值、源码稳定性和预计工作量排序。

## 下一步

1. 完成 ArduboyWorks 其余 15 个游戏的构建、固定输入回放和实际运行截图；重点处理 AVR 函数指针读取、Lasers 的 AVR 汇编、Morse 的 USB HID 与静音音频适配。

1. 确定 ArduGirl 自身开源许可证。
2. 添加更完整的 golden test 和 sanitizers。
3. 按 `GAME_PORTS.md` 优先级引入 Twotris，并完成源码固定、构建、冒烟和截图闭环。
4. 在 SDL Linux 基线稳定后实现音频，再评估 MCU 后端。

## 未决策项

- ArduGirl 自身许可证（MIT、BSD-3-Clause 等）。
- 第一个 PY32 的确切芯片、开发板、屏幕接口和编译工具链。
- 第三方测试游戏名单；必须先完成许可证核查。
- Arduboy2 上游使用原始文件加平台补丁，还是维护窄范围 fork；应在 Phase 2 通过 diff 规模决定。
