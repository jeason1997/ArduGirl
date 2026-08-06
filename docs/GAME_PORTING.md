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
├── README.md
├── LICENSES/
├── upstream/       固定版本的上游源码，尽量不改
├── port/           ArduGirl wrapper/config
└── patches/        必须修改上游时的可审查补丁
```

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
8. 更新游戏 README、兼容矩阵和项目进度。

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

游戏使用固定 commit，不自动追踪默认分支。更新时重新检查许可证、补丁能否应用、golden tests 和存档兼容性。

