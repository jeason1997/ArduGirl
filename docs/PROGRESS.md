# 当前进度

最后更新：2026-08-07

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
| 社区游戏移植清单 | done | `GAME_PORTS.md`；21 款已移植游戏按实际接入顺序以绿色勾选状态置顶，41 款待移植候选保留原综合优先级 |
| Agent 约束 | done | 根目录 `AGENTS.md` |
| 构建系统 | done | GNU Make；`make` 聚合全部游戏构建，`make test` 聚合公共与游戏测试 |
| framebuffer core | done | 1024 字节页面布局、像素、直线、矩形、圆、三角形、bitmap、compressed bitmap 及回归测试 |
| Linux terminal backend | partial | Braille 显示、raw input、固定帧模式已运行；按当前决策暂停效果优化 |
| MicroTD | partial | 子模块源码未修改；固定输入回放已验证进入地图、建塔和启动波次；待持久化和完整游玩验证 |
| ArduboyWorks 游戏集 | partial | 整仓固定到 `d4b1f041`；18 个游戏均已成功构建、通过 180 帧 SDL2 无头冒烟，并分别保存 Logo、菜单和核心玩法截图；待逐游戏完整流程、音频和存档验证 |
| Linux SDL2 backend | partial | 已实现窗口、最近邻整数缩放、键盘输入、单调毫秒/微秒计时、headless、事件注入、framebuffer golden test、EEPROM 文件后端、双声道方波、波表和四声道 ATM 合成；gamepad 与长时间音频仍待验证 |
| Arduboy2 兼容实现 | partial | 已覆盖当前 21 个游戏，并补齐高频图元、bitmap、Sprites 模式、文本缩放/换行、按钮、帧率、PROGMEM、EEPROM、静态音频控制和定时 tone；尚非完整上游 API |
| 游戏源码 | partial | 已固定并接入 MicroTD、ArduboyWorks 18 个游戏、Ardynia 与 Arduventure；后续游戏按 `GAME_PORTS.md` 继续导入 |
| 音频 | partial | SDL2 已实现双声道方波、8 位波表和四声道 ATM 合成；Arduboy2Beep、ArduboyPlaytune、ArduboyWorks 音频扩展与 ATMlib 已接入，Arduventure 标题音乐回放通过。ArduboyTones 与长时间音乐仍待真实样本验证 |
| PY32 | partial | PY32F002A + ST7789 已改用额定 24 MHz HSI；2026-08-07 最近一次完整冷构建通过 21/24，失败项为 Bananonsense、Chri-Bocchi Cat、Stairs Sweep；其后 Stairs Sweep 已独立冷构建通过，尚未重跑全量，剩余两款仍待修复；另待实机长时间静置、60 FPS、按钮和蜂鸣器验收，Flash EEPROM 尚未实现 |
| STM32 | planned | Linux/PY32 之后 |

## 已完成

- 移除集合 profile 中与正式补丁重复的 `replacements` 源码改写机制；原有 ArduboyWorks 兼容修改已迁移为每个游戏显式引用、带上下文校验的编号补丁。公共准备器拒绝构建配置重新声明源码替换，源码适配只保留 `patches` 一条路径。
- 修复 Stairs Sweep 的 PY32 裸机链接失败：ArduboyWorks 集合 profile 原先只会把 `MyArduboy.cpp` 中动态创建的 `ArduboyPlaytune` 替换为公共固定容量播放器工厂，遗漏了该游戏采用的 `MyArduboyV.cpp`，导致固件引用未提供的 `operator new`。现由同一平台无关 profile 覆盖该文件名，继续保持上游目录不变且不向 PY32 构建加入游戏特判。生成快照已确认改用 `ardugirl_create_playtune()`；PY32 独立冷构建通过，固件 text 17272、data 340、bss 3204 字节；Linux 单游戏构建、180 帧冒烟及公共 Playtune 回归测试通过。最近一次完整 PY32 冷构建尚未重跑，因此不把独立结果误报为新的全量通过数。

- 修复 PY32 并行冷构建的生成源码竞态：平台 Makefile 现在把准备器同批产生的全部 C/C++ 快照显式连接到主生成目标，避免 `make -j` 在准备器完成前报告“没有规则可生成”；PY32 游戏测试会删除对应生成快照并使用 `-j4` 构建，持续覆盖该依赖契约。
- 修复 ArduboyWorks Hollow 的 PY32 裸机链接失败：集合适配原先把上游动态创建的 `ArduboyPlaytune` 改成函数内静态对象，其非平凡析构会注册 `atexit`，继而把 newlib 退出链和缺失的 `_fini` 拉入固件。公共 Playtune 兼容层新增固定容量、无堆分配且不注册退出析构的进程期播放器工厂，集合 profile 统一使用该入口，并增加最小回归测试。Hollow 从空目标目录交叉构建通过，固件 text 22536、data 340、bss 3356 字节；CMSIS-DAP/OpenOCD 写入、校验和复位成功，Linux 全量测试同时通过。
- 统一游戏构建描述：Linux 不再加载每个游戏或集合的 `port.mk`，而是一次扫描 `game.toml` 生成轻量目录，并由平台内单一模板展开 SDL、终端、冒烟和回放目标；PY32 与 Linux 现共同消费 `tools/prepare_game.py` 生成的原子源码快照和运行入口。删除 7 份重复 `port.mk` 与 7 份手写 `entry.cpp`。普通游戏只需清单和可选补丁；ArduboyWorks 的真实集合差异由每款清单显式引用集合级 `profile.toml`，不再依赖构建器向父目录猜测 `build.toml`。从空 `build/` 执行 Linux 全部 24 个 SDL 游戏的 `make -j4` 冷构建通过，随后 `make test -j4` 的公共测试、24 款冒烟和 3 款固定回放全部通过，`make test-terminal` 通过；PY32 MicroTD 交叉构建通过，固件 text 19280、data 364、bss 3484 字节。
- 修复 Linux 平台 `GAME=<game-id>` 选择发生得过晚的问题：平台 Makefile 原先先加载全部游戏 `port.mk` 和依赖，再从最终目标中过滤指定游戏，导致 `/mnt/e` 上的单游戏构建仍承担全仓解析开销。现根据直属游戏目录或集合内 `game.toml` 自动定位并只加载所属 `port.mk`；ArduboyWorks 集合同时只展开指定游戏，根 Makefile不包含任何游戏名称或路径特判。
- 修复统一入口的 Linux 单游戏选择回归：根 Makefile 转发默认 `all` 后，Linux 平台曾忽略 `GAME` 并依赖全部 `PORT_BUILD_TARGETS`，导致 `make PLATFORM=linux GAME=microtd` 编译所有游戏。Linux 平台现从各移植模块登记的构建产物中精确选择与 `GAME` 对应的唯一目标；未指定 `GAME` 时才保持全量构建，未知或重复匹配立即报错。MicroTD 和 ArduboyWorks 的依赖文件裁剪同时识别 `GAME` 参数，避免单游戏调用重新读取无关游戏依赖。默认单游戏调用保持原有的“编译后运行”行为；`build` 目标只生成 SDL 程序，`run` 可显式启动。
- 整理平台所有权与统一构建入口：根 Makefile 不再声明 `py32`、`flash-py32` 或 `PY32_GAME`，只按 `PLATFORM` 选择平台 Makefile，并原样转发目标和 `GAME` 等变量；PY32 统一使用 `make PLATFORM=py32 GAME=<game-id>` 构建、追加 `flash` 烧录。Linux 与 PY32 自有文件均按职责分类：实现进入 `src/`，私有头文件进入 `include/`，平台测试或工具分别进入 `tests/`、`tools/`，厂商依赖保留在 `vendor/`；平台实现和专用测试不再平铺在平台根目录或散放到公共测试目录。WSL 冷构建并执行迁移后的 SDL 后端测试和存储测试均通过；终端 smoke 的全部新路径对象可编译，但既有目标最终链接仍缺少 `Arduboy2Audio::enabled()` 与 `millis()`，未把该历史问题误记为本次整理通过。
- 新增 PY32 全游戏编译测试：`make PLATFORM=py32 test` 由 `platform/py32/tools/test_games.py` 自动遍历全部 `game.toml`，对每款游戏定向清理 `build/py32/<game-id>/` 并独立冷构建，失败后继续检查剩余游戏，最终以汇总状态决定测试退出码。2026-08-06 首轮覆盖 24 款游戏，通过 5 款：Arduventure、Fantasy Rampage、Helmets & Hordes、MicroTD、Sunfire；18 款 ArduboyWorks 游戏因集合入口所需编译定义尚未接入通用 PY32 构建而失败，Ardynia 因现有补丁上下文不匹配而在源码准备阶段失败。测试实现与构建规则均位于 `platform/py32/`，未向游戏目录或公共兼容层加入平台逻辑。
- 2026-08-07 修正全量测试把 `clean all` 作为并列目标导致旧依赖图误判的问题，并让公共准备器支持递归源码、集合构建描述和原子快照。最新完整冷构建退出码仍为 2，通过 11/24；失败项为 Bananonsense、Chie Magari、Chri Bocchi、Hollow、Hopper、Knight Move、Lasers、Pi 24k、Psicolo、Quarto、Reversi、SameGame、Stairs Sweep。当前提交是阶段性修复，不代表全部游戏已经通过。
- 重构 Make 分层并消除 WSL 下已完成单游戏目标的长时间等待：根 Makefile 现只选择平台构建文件并转发目标，Linux 的编译器、SDL、公共测试和移植挂载规则全部迁入 `platform/linux/Makefile`，游戏细节继续由各自 `port.mk` 管理。MicroTD 与 ArduboyWorks 集合在单独构建某个游戏时只加载该游戏自身依赖，避免在 `/mnt/e` 的 DrvFS 上读取全部移植的 124 个 `.d` 文件；完整构建、聚合测试和多目标调用仍加载全部依赖。ArduboyWorks 的源码能力探测也由逐游戏 54 次子进程合并为 3 次批量扫描。包含 PowerShell 启动 WSL 的外部计时中，`make -n microtd` 由约 7.6 秒降至约 0.85 秒。
- 修复 MicroTD 的通用 MCU 构建入口：该游戏仍引用早期 Linux 专用生成物 `build/generated/microtd_patched.ino`，导致 PY32 通用准备器虽然成功应用补丁，却无法满足入口包含路径。MicroTD 自有入口与 `port.mk` 现统一消费 `build/generated/microtd/microtd.ino`，旧的 sed/patch 特殊生成规则已删除；PY32 的统一 Arduino 源码编译环境同时补齐 `ARDUINO=10819`，避免上游启用桌面专用 `main()`。公共兼容层新增无动态分配的整数 `sprintf` 子集，避免一次 `%d` 格式化拉入完整 newlib，固件最终为 text 21360、data 372、bss 3504 字节，并已通过 OpenOCD 写入和校验。上游子模块保持不变，PY32 Makefile 未增加游戏特判。
- 完成首次 PY32 移植错误复盘，记录平台代码污染游戏目录、厂商库误放根 `third_party/`、直接修改上游游戏、建立“平台 × 游戏”组合构建文件、遗留无关构建目录、GPIO 多引脚误用导致黑屏、全仓清理导致 WSL 构建缓慢，以及遗漏 AVR/ARM `char` 符号差异八类问题；对应纠正方式和后续 MCU 平台检查表已保存至 `docs/PY32_PORTING_RETROSPECTIVE.md`。
- 建立 PY32F002A 首版后端与独立显示驱动边界：ST7789 仅在具体驱动中实现，128×64 framebuffer 不缩放显示于 240×240 屏幕中央；板级按钮和蜂鸣器引脚集中配置。Arduventure 交叉链接结果为 text 28436、data 52、bss 2588 字节，已通过 CMSIS-DAP/OpenOCD 写入和校验。复位运行两秒后帧计数为 42，PC 位于 `Sprites::drawSelfMasked`，MSP 为 `0x20000f88`。蜂鸣器已实现四声道到单声部的软件 DDS 输出；按钮、声音听感和画面方向仍待实机交互验收，EEPROM 暂不持久化，因此平台继续标为 `partial`。
- 纠正首版 PY32 构建边界：`games/arduventure/` 保持本次任务前的内容，不承载任何 PY32 改动；删除“平台×游戏”专用目标文件，改由 `tools/prepare_game.py` 通用读取已有 `game.toml` 并生成源码快照。CMSIS、PY32F002Ax5 设备头、LL、系统文件、启动汇编、链接脚本和 OpenOCD 配置已作为固定快照收口到 `platform/py32/vendor/`，不占用游戏与 Arduboy 上游专用的根 `third_party/`，构建与烧录也不再依赖开发机参考工程的绝对路径。重构后从空 `build/` 完成通用准备和 PY32 固件构建，尺寸保持不变。
- 修复 PY32 ST7789 无输出：PY32 LL 的 GPIO 单元接口一次只允许一个引脚，原实现错误地把多个引脚组合传入，导致 PA5/PA7 SPI 复用配置无效；现已逐引脚配置显示和六个按钮，并补齐参考驱动的上下拉、Gamma 表及 SPI 使能顺序。修复后固件 text 为 28652 字节，已重新烧录校验；运行三秒帧计数为 58，GPIOA MODER=`0xEBFFBB5F`、SPI1 CR1=`0xC347`、MSP=`0x20000f80`，确认应用持续运行且显示总线寄存器配置正确。
- 修复 Sunfire 的 PY32 构建失败：Arm GCC 默认无符号 `char`，与该 AVR 游戏依赖的有符号 `char` 语义不一致，既导致负数位图表发生窄化错误，也会改变俯仰和滚转判断。PY32 的统一 C++ 编译契约现使用 `-fsigned-char`，未修改游戏源码，也未增加游戏专用构建文件。Sunfire 冷构建结果为 text 17376、data 368、bss 2832 字节，并已通过 CMSIS-DAP/OpenOCD 写入和校验。

- 固定并接入 Helmets & Hordes 87d1b7e1、Fantasy Rampage 29d30d35 和 Sunfire e99fff73；三款游戏均复用公共 ATMlib 四声道解释器与 SDL 合成后端。已补齐旧版 Arduino/Arduboy API 并增加回归断言，通过各自 C++17 构建、固定输入无头运行和三阶段 framebuffer 截图。上游子模块保持 clean；完整流程和长期音乐仍标为 partial。
- 重排 `GAME_PORTS.md`：补齐全部 21 款现有移植游戏，按 MicroTD、ArduboyWorks 批次、Ardynia、Arduventure 的实际接入顺序置顶并标记 `✅ 已移植`；其余 41 款源码可访问候选单独保留原优先级，移植状态更易识别。
- 全量复核当前 21 个移植游戏的截图状态：MicroTD 保留 4 张验收图，Ardynia、Arduventure 与 ArduboyWorks 18 个游戏均补齐各自 `assets/` 下至少 3 张不同阶段 PNG，并在每个游戏自己的 README 中展示 Logo/菜单、剧情或出生点以及实际核心玩法；仓库现有 64 张游戏截图。自动审计确认全部图片为 128×64、游戏内哈希互不重复且 README 引用完整；从空 `build/` 执行 `make test -j4` 重建并测试全部 21 个游戏，135.5 秒退出码为 0。所有尚未完成完整流程验收的游戏继续标为 `partial`。
- 通用 SDL2 运行入口新增可重复的 `--replay-button 帧:掩码:持续帧数` 与 `--capture-frame 帧:路径`，可按确定性输入路径导出 128×64 PGM framebuffer，为后续移植提供可复现截图证据。
- 固定输入首次进入 Psi Colo 骰子棋盘时发现上游 `drawFloorOrBlank` 声明返回 `bool` 却不返回值；已在游戏自身保存编号最小补丁，并将 ArduboyWorks 生成阶段扩展为按编号安全应用 `game.cpp` 补丁。修复后 260 帧玩法回放和三阶段截图通过，上游子模块保持 clean。
- 统一游戏截图验收规范：每个完成移植的游戏必须在自身 `assets/` 中保存至少 3 张由当前 ArduGirl 前端生成的不同阶段截图，至少包含 1 张实际玩法画面，并在 README 中全部展示和说明；单张截图或无头冒烟只能标为 `partial`。
- 明确本仓库发布约束：用户要求“提交”时只使用 Git 检查、提交并推送当前分支，不依赖 GitHub CLI，也不默认创建 Pull Request。
- 固定并接入 Arduventure `938fae77` 与 ATMlib `952d079f`：上游子模块保持 clean，公共兼容层解释四声道乐谱，SDL2 合成脉冲、三角和噪声波形；从空 `build/` 完成全仓构建，ATM 单元测试、180 帧无头启动和“标题→新游戏→剧情入口”固定回放通过，并保存 ArduGirl framebuffer 实际截图。当前仍标为 `partial`，待战斗、存档、完整剧情和长时间音乐验证。
- 固定并接入 Ardynia `860312d2`：上游子模块保持 clean，通过外部入口和独立 `port.mk` 构建；补齐公共 `Arduboy2Base::sBuffer`、静态 `Arduboy2Audio` 与 EEPROM `get/put` 契约。修复玩家贴近屏幕顶部时依赖 AVR 16 位 `int` 回绕的 framebuffer 越界，以及蛇敌人离屏后先读瓦片造成的数组越界；“进入游戏后持续向上移动”固定回放已在 ASan+UBSan 与普通优化构建下通过。
- Linux SDL2 与终端运行时新增致命信号调用栈输出，可执行文件使用 `-rdynamic` 导出符号；崩溃后仍恢复默认信号行为，以保留非零退出状态和 core dump 能力。

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
2. 将本次 ArduboyWorks 截图输入时序固化为逐游戏自动回放断言，并继续验证完整流程、音频和存档；当前 18 个游戏均已有实际核心玩法截图。
3. 添加 PCM、复杂压缩图片、时间回绕、GCC/Clang 和 sanitizers 测试。
4. 增加 SDL2 gamepad 输入和键位配置。
5. 按 `GAME_PORTS.md` 优先级引入 Twotris，并完成源码固定、构建、冒烟和截图闭环。
6. 以新游戏样本继续验证 ArduboyTones 或其他扩展库，并为 ATMlib 增加长时间音乐与更多效果指令回归；随后再评估 MCU 后端。

## 未决策项

- 第一个 PY32 的确切芯片、开发板、屏幕接口和编译工具链。
- Arduboy2 上游使用原始文件加平台补丁，还是维护窄范围 fork；应在 Phase 2 通过 diff 规模决定。
