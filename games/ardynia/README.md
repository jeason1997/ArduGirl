# Ardynia

Ardynia 是一款受经典俯视角动作冒险游戏启发的 Arduboy RPG，包含探索、战斗、地牢、道具和存档。本移植同时验证大型 Arduboy2 游戏和 ArduboyPlaytune 双声道音效路径。

## 截图验收

| 标题与开始菜单 | 新游戏出生点 | 野外移动玩法 |
| --- | --- | --- |
| ![Ardynia 标题与开始菜单](assets/title.png) | ![Ardynia 新游戏出生点](assets/game-start.png) | ![Ardynia 野外移动玩法](assets/gameplay.png) |

## 状态与运行

清单状态为 `replay-tested`：Linux SDL2 构建、无头启动、ASan+UBSan 固定回放和截图路径已通过；截图证明已进入核心探索玩法，但完整战斗、地牢、道具、存档和音频流程仍未逐项验收，因此尚未完成移植。

```bash
make PLATFORM=linux GAME=ardynia
```

仅构建或执行定向测试：

```bash
make PLATFORM=linux GAME=ardynia build
make PLATFORM=linux GAME=ardynia test-ardynia
```

方向键或 WASD 控制移动，J/Z 对应 A 键，K/X 对应 B 键，Escape 退出。
