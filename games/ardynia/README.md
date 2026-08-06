# Ardynia

Ardynia 是一款受经典俯视角动作冒险游戏启发的 Arduboy RPG，包含探索、战斗、地牢、道具和存档。本移植主要用于验证大型 Arduboy2 游戏以及 ArduboyPlaytune 双声道声音效果。

## 运行

```bash
make ardynia
```

方向键或 WASD 控制移动，J/Z 对应 A 键，K/X 对应 B 键，Escape 退出。

## 当前状态

上游源码固定为 `860312d2c22524b2b6e96951691e4d426eb3b701`，并保持未修改。当前完成 Linux SDL2 冷构建、无头启动冒烟，以及“进入游戏后持续向上移动”的 ASan+UBSan 固定回放；实际运行截图和完整玩法验证尚未完成，因此状态仍为 `partial`。
