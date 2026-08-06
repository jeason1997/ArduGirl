# Arduventure

Arduventure 是 TEAM a.r.g. 制作的 Arduboy 动作 RPG。玩家探索多个区域、与敌人战斗、收集和装备物品，并由四声道 ATMlib 合成器播放标题、地图和战斗音乐。

## 截图验收

| 主菜单 | 剧情对话 | 房间探索玩法 |
| --- | --- | --- |
| ![Arduventure 主菜单](assets/title.png) | ![Arduventure 剧情对话](assets/menu.png) | ![Arduventure 房间探索玩法](assets/gameplay.png) |

## 状态与运行

当前状态为 `partial`：SDL2 冷构建、180 帧无头启动、四声道标题音乐和上述固定输入回放已经通过；截图证明已进入可移动的房间探索玩法，但战斗、存档、完整剧情和长时间音乐仍未验收。

```bash
make arduventure
```

代码采用 MIT 许可证；上游明确声明故事、角色、精灵、图块、设计和美术不属于 MIT 授权范围，相关权利归 TEAM a.r.g. 所有。
