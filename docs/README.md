# 文档索引与维护规则

本文是 ArduGirl 文档入口。根 `README.md` 只提供项目简介和快速开始；实现事实、计划和历史必须进入下面对应的唯一来源，避免同一状态在多处复制后互相冲突。

## 文档分工

| 文档 | 负责内容 | 不应包含 |
|---|---|---|
| `ARCHITECTURE.md` | 稳定的分层、公共契约、依赖方向和构建边界 | 阶段进度、某次测试流水账 |
| `COMPATIBILITY.md` | 已支持 API、已知 ABI 差异和兼容策略 | 游戏候选清单、平台引脚 |
| `LINUX_SDL.md` | Linux SDL2 前端的行为、命令和验收标准 | MCU 规则、全项目路线 |
| `PY32F002A.md` | 当前 PY32 板级配置、能力、限制和最近硬件证据 | 通用游戏导入规则 |
| `GAME_PORTING.md` | 游戏清单格式、导入、补丁和验收流程 | 各游戏的实时状态 |
| `GAME_PORTS.md` | 已接入游戏与候选队列 | 详细构建规范 |
| `UPSTREAMS.md` | 当前固定上游 SHA 和更新策略 | 开发流水账 |
| `ROADMAP.md` | 尚未完成的阶段目标与方向 | 已完成事项的逐次历史 |
| `PROGRESS.md` | 当前状态快照、最近验证、紧接着要做的事 | 按日期无限追加的变更日志 |
| `PY32_PORTING_RETROSPECTIVE.md` | 已发生问题的历史复盘 | 当前平台状态 |
| `PERFORMANCE.md` / `REFERENCES.md` | 专题原则与外部资料 | 项目进度 |

各游戏的验收证据属于其 `games/**/README.md`；代码层强制规则属于根或最近的 `AGENTS.md`。历史提交细节由 Git 保存，不再复制到 `PROGRESS.md`。

## 阅读路径

- 修改核心或公共 API：`ARCHITECTURE.md` → `COMPATIBILITY.md` → `PERFORMANCE.md`。
- 修改 Linux：对应的 Linux 平台规格。
- 修改 PY32：`PY32F002A.md`；需要理解历史约束时再读复盘。
- 导入或修复游戏：`GAME_PORTING.md` → 该游戏 README 与 `game.toml` → `UPSTREAMS.md`。
- 评估项目现状：`PROGRESS.md`；规划后续：`ROADMAP.md`。

## 维护原则

- 描述“现在是什么”时写可验证事实，并注明证据日期；描述“以后做什么”时使用未完成项，不把旧计划写成永久禁令。
- 数量由 `game.toml` 等仓库事实统计。新增游戏或平台后，同步更新引用该数量的状态文档。
- 历史复盘保留当时语境并明确标为历史，不用它覆盖当前规格。
- 文档变更也要更新 `PROGRESS.md`，但只记录新的当前状态和最近证据。
