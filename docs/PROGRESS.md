# 当前进度

最后更新：2026-08-06

## 总览

| 领域 | 状态 | 证据/说明 |
|---|---|---|
| 项目方向 | done | 已确定源码直接编译，不采用模拟器 |
| Git 仓库 | done | `main` 已初始化并持续推送到固定 `origin` |
| 架构规格 | done | `ARCHITECTURE.md` |
| 兼容范围 | done | `COMPATIBILITY.md` |
| 性能/整数规范 | done | `PERFORMANCE.md` |
| Linux SDL 规格 | done | `LINUX_SDL.md`；默认 SDL2 |
| Linux terminal 规格 | done | `LINUX_TERMINAL.md`；Braille/half/ASCII |
| 游戏导入规范 | done | `GAME_PORTING.md` |
| 社区游戏移植清单 | done | `GAME_PORTS.md`；只收录源码可访问的游戏，47 款合并为单表并按综合移植优先级排序 |
| Agent 约束 | done | 根目录 `AGENTS.md` |
| 构建系统 | done | GNU Make；`make` 聚合全部游戏构建，`make test` 聚合公共与游戏测试 |
| framebuffer core | done | 1024 字节页面布局、像素、直线、矩形、圆、三角形、bitmap、compressed bitmap 及回归测试 |
| Linux terminal backend | partial | Braille 显示、raw input、固定帧模式已运行；按当前决策暂停效果优化 |
| MicroTD | partial | 子模块源码未修改；固定输入回放已验证进入地图、建塔和启动波次；待持久化和完整游玩验证 |
| ArduboyWorks 游戏集 | partial | 整仓固定到 `d4b1f041`；18 个游戏均已成功构建并分别通过 180 帧 SDL2 无头冒烟；待逐游戏固定输入回放、实际画面截图和完整玩法验证 |
| Linux SDL2 backend | partial | 已实现窗口、最近邻整数缩放、键盘输入、单调毫秒/微秒计时、headless、事件注入、framebuffer golden test、EEPROM 文件后端、双声道方波和波表播放；gamepad 与长时间音频仍待验证 |
| Arduboy2 兼容实现 | partial | 已覆盖当前 19 个游戏，并补齐高频图元、bitmap、Sprites 模式、文本缩放/换行、按钮、帧率、PROGMEM、EEPROM 和定时 tone；尚非完整上游 API |
| 游戏源码 | partial | 已固定并接入 MicroTD 与 ArduboyWorks 18 个游戏；后续游戏按 `GAME_PORTS.md` 继续导入 |
| 音频 | partial | SDL2 已实现双声道方波、8 位波表混合和定时停止；Arduboy2Beep、ArduboyPlaytune 与 ArduboyWorks 音频扩展已接入。当前游戏不依赖 ArduboyTones/ATMlib，待真实样本驱动后续移植 |
| PY32 | planned | 需先指定芯片/板卡/屏幕 |
| STM32 | planned | Linux/PY32 之后 |

## 已完成

- 将 `platform/linux_common`、`platform/linux_sdl` 和 `platform/linux_terminal` 收拢为单一 `platform/linux/` 目录；SDL2、终端和共享存储通过文件名区分，构建目标与平台行为保持不变。
- 补齐 Arduboy2 高频兼容接口：三角形、圆角矩形、XY/compressed bitmap、Sprites 外部遮罩/自遮罩/擦除、文本缩放与换行、`notPressed()`、`delay()`、`map()` 和真实微秒计时；新增兼容层回归测试。
- SDL2 后端新增 48 kHz 方波输出、线程安全的频率切换和停止控制；无音频设备时安全降级，终端后端保持静音。
- 新增独立 ArduboyPlaytune 兼容实现，以主循环非阻塞推进音符、停止、等待、标记和重复指令；SDL2 后端扩展为双声道方波混合，ArduboyWorks 私有优先级接口通过集合适配层转发。
- ArduboyPlaytune 回归测试已验证 MIDI 音高、毫秒等待、停止和声音开关；SameGame 已完成定向重编译与 180 帧 SDL2 无头冒烟，未按默认流程扩大运行其余游戏。
- 修正 `initChannel()` 参数语义：参数表示扬声器引脚，每次调用追加一个逻辑声道；双轨回归使用同一引脚连续初始化两次，并验证两个 MIDI 音符同时输出和停止。当前已导入的 ArduboyWorks 游戏均只初始化一个声道，因此游戏内仍表现为单声道音效。
- 补齐 ArduboyWorks 剩余音频路径：Playtune 支持调用移调和重复段移调，`playWave()` 将固定采样率的 8 位波表交给 SDL 回调播放；Bananonsense 的香蕉波表不再是静音入口。
- 删除 `games/examples/`，不再维护官方示例构建目标；原 `games/ports/` 下的游戏已提升到 `games/` 直接子目录，根 Makefile 统一自动加载 `games/*/port.mk`。已从空 `build/` 完成全部 19 个 SDL 游戏冷构建，并通过 `make test` 与 `make test-terminal`。
- 根目录 Makefile 已移除全部具体游戏、游戏集合、上游私有类型和游戏专用补丁规则，改为自动加载各游戏或集合自己的 `port.mk`；公共测试通过扩展目标聚合。ArduboyWorks 的自动发现和私有适配全部位于其移植目录，MicroTD 的构建、补丁及回放测试全部位于自身移植目录。Lasers 的灰屏适配已从集合级 sed 脚本迁移到游戏目录内的编号统一 diff 补丁。

- 完成 ArduboyWorks 全部 18 个成品游戏的 SDL2 构建和逐游戏 180 帧无头启动冒烟；兼容层覆盖旧版 Arduboy API、PROGMEM、AVR EEPROM、音频和无副作用 HID，生成阶段安全改写 AVR 函数指针表，并为 Lasers 的灰屏汇编保存独立转换脚本。

- 建立项目目标、非目标和依赖边界。
- 定义 framebuffer、按键、时间、存储和音频的平台契约。
- 说明 AVR 16 位 `int` 与 32/64 位宿主 ABI 的兼容策略。
- 规定第三方游戏的源码位置和版本固定方式。
- 列出 Linux → Arduboy2 → 游戏 → 音频 → PY32 的实施顺序。
- 在 WSL2 使用 GCC 成功构建并运行 `hello` 终端示例。
- 实现 128x64 framebuffer 到 64x16 Unicode Braille 的转换。
- 实现 WASD、A/B、退出键输入和非交互固定帧模式。
- 添加 framebuffer 位布局、裁剪、矩形和清屏单元测试。
- 自写图形示例已移出 `games/`，仅保留为底层 smoke test。
- 确定游戏与 Arduboy2 兼容层使用 C++，未来 MCU 平台边界保持可由 C HAL 实现。
- 引入未修改的 MicroTD `0c8958f`，补齐字体、图元、Sprites、按钮边沿、EEPROM 内存接口和 Beep 接口；终端主菜单已显示，注入 A 键后 framebuffer 发生预期变化。
- 明确上游版本锁定策略：默认禁止跟踪最新分支，只有用户明确要求时才移动并重新验证固定 SHA。
- 修复终端方向键：解析 `ESC [ A/B/C/D` 和 `ESC O A/B/C/D`，单独 Escape 延迟判定为退出，并加入回归测试。
- 新增 SDL2 后端并设为默认构建和测试平台；MicroTD 可在无显示服务器的 headless 模式执行固定帧测试。
- SDL2 窗口默认使用 1:1 像素倍率，并保留 `--scale N` 整数倍率覆盖。
- 保留终端构建、运行和回归测试目标，但按当前决策暂停终端显示效果优化。
- 实际操作 MicroTD 终端前端进入地图、建塔菜单和敌人波次，从 128x64 framebuffer 生成多张未缩放截图，并为 README 补充游戏简介；后续每个移植游戏都必须提供 ArduGirl 实际运行截图。
- 为 SDL2 后端增加真实事件队列注入回归，覆盖组合按键、释放、自动重复过滤和退出；拆分无 SDL 类型的内部 framebuffer 转换函数，并以棋盘图案正常/反色 golden hash 验证 ARGB8888 输出。
- 为 MicroTD 增加固定输入回放，验证选择地图、打开建塔菜单、建造防御塔和启动敌人波次；回放发现上游成功建塔路径缺失布尔返回值，现通过生成目录中的独立最小补丁消除未定义行为，子模块保持 clean。
- 实现 SDL/终端共享的 Linux EEPROM 文件后端：默认使用 XDG data 目录，支持 `--save-dir` 覆盖，按稳定 game-id 隔离，并在退出时通过临时文件替换；回归测试覆盖首次启动、重启保持、游戏隔离、越界和短文件恢复。
- 调研 Arduboy 官网精选、社区热门讨论与社区游戏目录，新增 `GAME_PORTS.md`；已收集热门候选，后续按源码完整性和兼容风险重新分层。
- 调整根目录 `AGENTS.md`：游戏候选以完整源码可获取为准，要求固定上游版本并完成构建与冒烟验证。
- 按新准入规则重整 `GAME_PORTS.md`：删除无源码候选和梯队划分，将 47 款源码可访问游戏合并为一张表，并按热度、验证价值、源码稳定性和预计工作量排序。

## 下一步

1. 对照固定 Arduboy2 上游版本生成公共 API 差异清单，并补齐代表性官方示例编译测试。
2. 为 ArduboyWorks 游戏逐个补充固定输入回放、实际运行截图和完整玩法验证；当前全部游戏已达到可构建、可启动状态。
3. 添加 PCM、复杂压缩图片、时间回绕、GCC/Clang 和 sanitizers 测试。
4. 增加 SDL2 gamepad 输入和键位配置。
5. 按 `GAME_PORTS.md` 优先级引入 Twotris，并完成源码固定、构建、冒烟和截图闭环。
6. 根据新游戏的真实依赖选择 ArduboyTones、ATMlib 或其他扩展库；随后再评估 MCU 后端。

## 未决策项

- 第一个 PY32 的确切芯片、开发板、屏幕接口和编译工具链。
- Arduboy2 上游使用原始文件加平台补丁，还是维护窄范围 fork；应在 Phase 2 通过 diff 规模决定。
