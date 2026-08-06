# 游戏源码获取与导入

## 来源优先级

1. 游戏项目的上游代码仓库和明确 release tag。
2. Arduboy 社区帖中发布的完整源码链接。
3. Arduboy 游戏目录、Cart Builder、Erwin's Collection 等发现入口。
4. 第三方合集只用于发现、比较和追踪源码位置。

`.arduboy` 和 FX cart 通常主要包含已编译 `.hex`、元数据和资产，不能替代完整、可编译的源码。只有 `.hex` 而无法取得源码的游戏不进入本项目。

## 游戏目录格式

```text
games/<game-id>/
├── game.toml        来源、固定版本、依赖、补丁和兼容状态
├── README.md        游戏简介、截图、移植状态和运行方法
├── upstream/        固定版本的上游源码，尽量不改
├── port/            ArduGirl wrapper/config
└── patches/         必须修改上游时的可审查补丁
```

同一上游仓库包含多个游戏时，集合构建应从各游戏的 `game.toml` 自动发现目标，不得在主构建文件中维护游戏名白名单。游戏特有补丁放在对应游戏的 `patches/` 目录，使用 `0001-<作用域>-<说明>.patch` 形式的编号统一 diff，并在 `game.toml` 中记录；不得用依赖行范围匹配的临时文本转换脚本代替可审查补丁。

每个移植游戏的 `README.md` 必须包含至少一张能辨认实际游戏画面的截图，以及一段无需了解上游项目也能看懂的简短介绍。截图必须由当前 ArduGirl 前端实际运行游戏后生成，并保存在对应移植目录中。尚未运行到可截图阶段的移植不得标为完成。

`game.toml` 示例：

```toml
id = "example-game"
name = "Example Game"
source = "https://example.com/example-game"
revision = "full-commit-sha"
entry = "upstream/ExampleGame.ino"

[compat]
level = "A"
status = "builds"
libraries = ["Arduboy2"]

[port]
patches = []
notes = []
```

## 导入流程

默认一次处理一个游戏。同一固定 revision 的上游仓库包含多个独立游戏时，可以整体固定仓库并批量接入，但每个游戏仍需独立的 `game.toml`、构建目标、静态扫描、冒烟、回放和截图证据。

1. 记录源码位置和完整 commit。
2. 构建原始 AVR 版本（若工具链可用），保存工具链和构建结果。
3. 运行静态扫描并分配兼容等级。
4. 让 `.ino` 通过 wrapper 参与 C++ 构建，不先重排上游源码。
5. 依次处理 include、构建、类型和平台差异。
6. 禁止把 AVR 寄存器访问伪装成无操作；使用小而明确的补丁。
7. 添加启动和固定输入冒烟测试。
8. 在 ArduGirl 前端实际运行游戏并生成截图。
9. 更新游戏 README、兼容矩阵和项目进度。

## 上游零修改策略

优先级从高到低：

1. 在 ArduGirl 的 Arduino/Arduboy2 兼容层补齐通用 API。
2. 使用外部 `entry.cpp`、include path 和构建选项接入原始 `.ino/.cpp`。
3. 为额外库提供独立兼容实现。
4. 确实依赖 AVR 寄存器或未定义行为时，维护独立、最小、可重放补丁。

禁止直接修改子模块中的游戏文件。上游目录必须保持 clean，更新流程只移动固定 revision，再重新运行构建、静态审计和测试。

## 静态扫描项

```text
PORT[B-F], DDR[B-F], PIN[B-F]
TCCR, TIMSK, OCR, TCNT
ISR(, cli(), sei()
SPDR, SPSR, TWBR
asm, __asm__
pgm_read_word with pointers
EEPROM.put/get native structs
ArduboyPlaytune, ATMlib, ArduboyFX
```

## 样本选择标准

选择游戏时优先覆盖不同技术路径：

1. Arduboy2 基本图元；
2. Sprites 和 PROGMEM 图片；
3. EEPROM、文本和声音；
4. 额外库或平台相关实现。

## 上游更新

游戏使用主仓库 gitlink 记录的完整 commit SHA，不自动追踪默认分支。日常初始化只能使用：

```bash
git submodule update --init --recursive
```

禁止日常使用 `git submodule update --remote` 或在子模块中执行 `git pull`。只有用户明确要求获取最新版本或指定版本时才能更新：

1. 记录更新前 SHA。
2. 获取用户指定的 tag/commit；“最新”表示任务开始时上游默认分支的 HEAD。
3. 将结果解析并固定为完整 commit SHA，即使用户指定的是 branch。
4. 检查源码布局和新依赖。
5. 保持上游目录无本地修改，重新应用外部补丁。
6. 运行静态审计、构建、单元测试、输入回放和存档兼容测试。
7. 同步更新 `game.toml`、`docs/UPSTREAMS.md` 和兼容状态。

任一步失败时保留原固定版本，除非用户明确接受未通过验证的新 revision。
