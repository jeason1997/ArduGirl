# 游戏源码获取与导入

## 来源优先级

1. 游戏作者的官方 GitHub/GitLab 仓库和明确 release tag。
2. Arduboy 社区帖中由作者发布的源码链接。
3. Arduboy 游戏目录、Cart Builder、Erwin's Collection 等只用来发现游戏及其 source URL。
4. 第三方合集仅用于发现或比较，除非每个游戏的来源与许可证都能独立验证。

`.arduboy` 和 FX cart 通常主要包含已编译 `.hex`、元数据和资产。它们不能替代源码，也不能证明再分发权。

## 许可证门槛

导入仓库前必须确认：

- 源码许可证文本或仓库明确声明；
- 游戏素材是否与代码使用相同许可证；
- 是否允许修改和再分发；
- 是否要求署名、相同许可证或公开修改；
- 固定使用的 commit/release。

许可证未知时，可以在文档中记录外部链接和本地构建步骤，但不得把源码复制进 `games/ports`。

## 游戏目录格式

```text
games/ports/<game-id>/
├── game.toml
├── README.md       游戏简介、截图、来源、许可证、移植状态和运行方法
├── LICENSES/
├── upstream/       固定版本的上游源码，尽量不改
├── port/           ArduGirl wrapper/config
└── patches/        必须修改上游时的可审查补丁
```

每个移植游戏的 `README.md` 必须包含至少一张能辨认实际游戏画面的截图，以及一段无需了解上游项目也能看懂的简短介绍。截图必须由当前 ArduGirl 前端实际运行游戏后生成，并保存在对应移植目录中，不能依赖上游仓库附带图片或可能失效的外部热链。若游戏尚未运行到可截图阶段，其移植文档视为未完成，不得用上游宣传图代替运行证据。

计划中的 `game.toml`：

```toml
id = "example-game"
name = "Example Game"
source = "https://github.com/author/example"
revision = "full-commit-sha"
license = "MIT"
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

1. 记录来源、commit、作者和许可证。
2. 构建原始 AVR 版本（若工具链可用），保存构建版本信息作为参考。
3. 运行静态扫描并分配兼容等级。
4. 让 `.ino` 通过 wrapper 参与 C++ 构建；不要一开始重排全部源码。
5. 修复 include/构建差异，再修复类型和平台差异。
6. 禁止把 AVR 寄存器访问伪装成无操作；使用小而明确的补丁。
7. 添加启动和固定输入冒烟测试。
8. 在 ArduGirl 前端实际运行游戏并生成截图。
9. 更新游戏 README（包括简介和截图）、兼容矩阵和项目进度。

## 上游零修改策略

优先级从高到低：

1. 在 ArduGirl 的 Arduino/Arduboy2 兼容层补齐通用 API。
2. 使用外部 `entry.cpp`、include path 和构建选项接入原始 `.ino/.cpp`。
3. 为额外开源库提供独立兼容实现。
4. 确实依赖 AVR 寄存器或未定义行为时，维护独立、最小、可重放补丁。

禁止直接进入子模块修改游戏文件。上游目录必须保持 clean，更新流程应只是移动固定 revision，然后重新运行构建、静态审计和测试。

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

## 首批样本选择标准

不要以“最热门”作为唯一标准。首批三个真实游戏应覆盖：

1. 纯 Arduboy2 + 基本图元；
2. Sprites + PROGMEM 图片；
3. EEPROM + 文本或简单声音。

每个游戏需 MIT/BSD/Apache/CC0 等清晰且兼容的许可。具体名单在实际核对上游许可证后写入 `docs/PROGRESS.md`，规格阶段不提前承诺。

## 上游更新

游戏使用主仓库 gitlink 记录的完整 commit SHA，不自动追踪默认分支。日常初始化只能使用：

```bash
git submodule update --init --recursive
```

禁止日常使用 `git submodule update --remote` 或在子模块中执行 `git pull`。只有用户明确要求获取最新版本或指定版本时才能更新：

1. 记录更新前 SHA。
2. 获取用户指定的 tag/commit；“最新”表示当次任务开始时上游默认分支的 HEAD。
3. 将结果解析并固定为完整 commit SHA，即使用户指定的是 branch。
4. 检查许可证、源码布局和新依赖。
5. 保持上游目录无本地修改，重新应用外部补丁（如果存在）。
6. 运行静态审计、完整构建、单元测试、输入回放和存档兼容测试。
7. 同步更新 `game.toml`、`docs/UPSTREAMS.md` 和兼容状态。

任一步失败时保留原固定版本，除非用户明确接受未通过验证的新 revision。
