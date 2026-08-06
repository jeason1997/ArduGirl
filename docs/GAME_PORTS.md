# 社区游戏移植清单

最后更新：2026-08-06

## 收录规则

本清单只收录能够直接访问完整源码的 Arduboy 游戏。只有 `.hex`、`.arduboy`、宣传页或截图而找不到源码的作品不列入。

所有游戏放在同一张表中，顺序就是当前建议的移植优先级。排序综合考虑：

1. 社区知名度和代表性。
2. 对 ArduGirl 当前兼容能力的验证价值。
3. 源码入口是否稳定、是否容易固定 revision。
4. 额外库、音频、AVR 专用代码、存档和程序规模带来的工作量。

兼容等级只是源码导入前的预估；只有固定 revision、完成静态扫描并实际构建后，才能写入 `game.toml` 的正式等级。

## 按优先级排序的游戏

| 优先级 | 游戏 | 类型 | 源码 | 预估等级 | 当前移植程度与排序理由 |
|---:|---|---|---|---|---|
| 1 | MicroTD | 塔防 | [上游源码](https://gitlab.com/drummyfish/Arduboy_TD) | B | **回放验证**：SDL2/终端均可构建，已进入地图、建塔并启动波次，EEPROM 持久化已验证；待完整游玩、音频和 sanitizer 验证 |
| 2 | Twotris | 双人下落方块 | [上游源码](https://github.com/skaterced/Twotris) | A/B | 未导入、未构建；玩法明确、代码预计较小，并能覆盖双人组合输入，作为下一款移植的首选 |
| 3 | Rooftop Rescue | 直升机救援 | [上游源码](https://github.com/BertVeer/Rooftop) | A/B | 未导入、未构建；轻量动作游戏，适合验证基础绘图、碰撞和按键 |
| 4 | Ravine Despoiler | 街机投弹 | [上游源码](https://github.com/unwiredben/arduboy-ravine-despoiler) | A/B | 未导入、未构建；源码入口独立，适合作为低风险 Arduboy2 样本 |
| 5 | Evade 2 | 街机躲避 | [上游源码](https://github.com/ArduboyCollection/evade2) | A/B | 未导入、未构建；官方精选作品，循环简单，适合验证帧率和碰撞 |
| 6 | Quarto! | 抽象棋类 | [上游源码](https://github.com/obono/ArduboyWorks/tree/master/quarto) | B | **玩法截图验证**：随 ArduboyWorks 固定到 `d4b1f041`，构建、180 帧冒烟和 CPU 对局棋盘截图通过；待完整对局与存档验证 |
| 7 | Blackjack | 卡牌 | [上游源码](https://github.com/Press-Play-On-Tape/Blackjack) | B | 未导入、未构建；适合验证文本、随机数、按钮边沿和存档 |
| 8 | Waternet | 管线谜题 | [上游源码](https://github.com/joyrider3774/waternet_arduboy) | B | 未导入、未构建；玩家高频推荐，可覆盖关卡数据和 EEPROM 进度 |
| 9 | Pipes | 连线谜题 | [上游源码](https://github.com/ArduboyCollection/LayingPipe) | B | 未导入、未构建；适合验证多尺寸棋盘、文本和 EEPROM |
| 10 | LATE | 三合一休闲谜题 | [上游源码](https://github.com/core1024/LATE) | B | 未导入、未构建；含俄罗斯方块、1010 和 Stick Hero 三种模式，需要分别建立冒烟路径 |
| 11 | Knight Move | 棋盘动作谜题 | [上游源码](https://github.com/obono/ArduboyWorks) | B | **玩法截图验证**：随 ArduboyWorks 固定到 `d4b1f041`，构建、180 帧冒烟和网格玩法截图通过；待完整关卡验证 |
| 12 | Glove | 迷宫动作冒险 | [上游源码](https://github.com/ArduboyCollection/glove) | B | 未导入、未构建；官方精选，适合综合验证 Sprites、碰撞和 EEPROM |
| 13 | Hopper | 立体跳跃动作 | [上游源码](https://github.com/ArduboyCollection/ArduboyWorks/tree/master/hopper) | B | **玩法截图验证**：随 ArduboyWorks 固定到 `d4b1f041`，构建、180 帧冒烟和跳跃玩法截图通过；待完整关卡与音效验证 |
| 14 | Kong | Game & Watch 式街机 | [上游源码](https://github.com/Press-Play-On-Tape/Kong) | B | 未导入、未构建；状态机和动画有代表性，程序规模可控 |
| 15 | Kong II | Game & Watch 式街机 | [上游源码](https://github.com/Press-Play-On-Tape/Kong-II) | B | 未导入、未构建；与 Kong 能复用兼容工作，排在 Kong 之后 |
| 16 | Oh Mummy | 迷宫街机 | [上游源码](https://github.com/Mattvic79/OhMummy) | B | 未导入、未构建；可覆盖迷宫逻辑、EEPROM 和简单声音 |
| 17 | Pipe Boy | 管道谜题/双人 | [上游源码](https://github.com/Glitsch3n/arduboy-game-collection/tree/main/PipeBoy) | B | 未导入、未构建；双人模式适合组合按键回归，需从合集固定子目录 |
| 18 | Games'n Goblins | 限时拼图 | [上游源码](https://github.com/ImMrShrike/gng-arduboy-game) | B | 未导入、未构建；适合验证 Sprites、计时和分数状态 |
| 19 | Omega Chase | 双摇杆式射击 | [上游源码](https://github.com/Karl-Williams/OmegaChase) | B/C | 未导入、未构建；输入、射击物和音频比前序样本复杂 |
| 20 | Quadrastic | 区域争夺动作 | [上游源码](https://github.com/ArduboyCollection/Quadrastic) | B | 未导入、未构建；适合压力测试大量碰撞和帧率 |
| 21 | Pyoro! | 接物动作 | [上游源码](https://github.com/Thesola10/Pyoro) | B | 未导入、未构建；动作机制清晰，资源和声音需要审计 |
| 22 | The Bounce | 物理平台 | [上游源码](https://github.com/ArduboyCollection/TheBounceArduboy) | B | 未导入、未构建；需重点审计定点/浮点运算和 AVR 16 位 `int` 假设 |
| 23 | TiMiNoo | 虚拟宠物 | [上游源码](https://github.com/TME520/TiMiNoo-Arduboy) | B | 未导入、未构建；能验证长期 EEPROM 和时间行为，完整冒烟耗时较长 |
| 24 | Randocity | 开放地图摩托 | [上游源码](https://github.com/pmwasson/Randocity) | B/C | 未导入、未构建；程序化地图很有代表性，但整数宽度和程序规模风险较高 |
| 25 | Space Cab | 飞行平台 | [上游源码](https://github.com/vampirics/SpaceCab) | B/C | 未导入、未构建；玩家高频推荐，关卡规模与音频依赖需要专项处理 |
| 26 | Dark & Under | 地牢 RPG | [上游源码](https://github.com/ArduboyCollection/Dark-And-Under) | B/C | 未导入、未构建；官方精选，适合基础兼容成熟后验证大型 RPG、存档和资源布局 |
| 27 | The Quest for Truth | RPG 平台 | [上游源码](https://github.com/GuillaumeElias/TheQuestForTruth) | B/C | 未导入、未构建；体量较大，需先核查额外库、关卡和存档 |
| 28 | Cursed Mansion 1300 | 探索街机 | [上游源码](https://github.com/ImMrShrike/Cursed-Mansion-1300) | B | 未导入、未构建；内容规模高于轻量样本，适合中后期导入 |
| 29 | Ardynia | Zelda 式 RPG | [上游源码](https://github.com/city41/ardynia) | C | **玩法截图验证**：源码固定为 `860312d2`，SDL2 冷构建、180 帧冒烟、ASan+UBSan 移动回放及标题/出生点/野外玩法截图通过；待战斗、地牢和存档验证 |
| 30 | Catacombs of the Damned | 伪 3D 地牢 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/CatacombsOfTheDamned) | C/D | 未导入、未构建；官方精选和玩家热门，当前源码入口是已有平台移植副本，射线渲染与平台修改需仔细拆分 |
| 31 | MicroCity | 城市建设 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/MicroCity-1.2) | B/C | 未导入、未构建；官方精选，当前入口为已有平台移植副本，模拟状态与存档规模较大 |
| 32 | Lode Runner | 平台解谜 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/LodeRunner) | B/C | 未导入、未构建；包含完整 154 关，需处理大量关卡数据和声音 |
| 33 | The Curse of Astarok | 地牢推运气 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/The-Curse-Of-AstaroK-master) | B/C | 未导入、未构建；当前入口为已有平台移植副本，需还原 Arduboy 构建边界 |
| 34 | Virus LQP-79 | 动作射击 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/ID-40-VIRUS-LQP-79-1.6) | B/C | 未导入、未构建；官方精选，当前入口为已有平台移植副本，需核查额外库 |
| 35 | Mystic Balloon | 平台谜题 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/ID-34-Mystic-Balloon-master) | B/C | 未导入、未构建；TEAM a.r.g. 代表作，当前入口为已有平台移植副本 |
| 36 | Sirène | 动作冒险 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/ID-42-Sirene-master) | B/C | 未导入、未构建；当前入口为已有平台移植副本，需核对 Sprites 与声音依赖 |
| 37 | Shadow Runner | 跑酷 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/ID-15-Shadow-Runner-1.6.1) | B/C | 未导入、未构建；当前入口为已有平台移植副本，适合中后期验证动作性能 |
| 38 | Blob Attack | 谜题 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/ID-28-Blob-Attack-master) | B/C | 未导入、未构建；当前入口为已有平台移植副本，需要拆分平台改动 |
| 39 | Space Battle: Trench Run | 射击 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/TrenchRun-Arduboy-1.0) | B/C | 未导入、未构建；射击物、场景滚动和声音使工作量较高 |
| 40 | Rayne the Rogue | 动作冒险 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/RAYNE_THE_ROGUE-1.0) | B/C | 未导入、未构建；当前入口为已有平台移植副本 |
| 41 | CastleBoy | 平台冒险 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/CastleBoy-master) | C | 未导入、未构建；官方精选，当前入口为已有平台移植副本，程序规模和资源较大 |
| 42 | Arduventure | 动作 RPG | [上游源码](https://github.com/Team-ARG-Museum/ID-46-Arduventure) | C | **玩法截图验证**：源码固定为 `938fae77`，四声道 ATMlib 标题音乐、SDL2 冷构建、180 帧冒烟及“标题→新游戏→剧情→房间移动”回放和三阶段截图通过；待战斗、存档和长时间音乐验收 |
| 43 | Circuit Dude | 线路谜题 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/CircuitDude) | C | 未导入、未构建；当前入口为已有平台移植副本，需评估平台相关改动 |
| 44 | Omega Horizon | 平台射击 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/OMEGA_HORIZON_AB) | C | 未导入、未构建；官方精选，当前入口为已有平台移植副本，资源与音频复杂 |
| 45 | Squario | 平台动作 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/Squario) | C | 未导入、未构建；当前入口为已有平台移植副本，需审计移植层耦合 |
| 46 | JetPac | 喷气平台射击 | [可访问源码](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES/JetPac) | C | 未导入、未构建；当前入口为已有平台移植副本，需审计移植层耦合 |
| 47 | Tetris | VMU 程序适配 | [源码快照](https://gist.github.com/felipemanga/9d5232e219d5adcddae5f18bdb0cb3f3) | D | 未导入、未构建；源码可访问，但通过 SoftVMS 运行 VMU 程序，不是普通 Arduboy2 游戏，技术路线与 ArduGirl 源码级兼容目标差异最大 |

## 执行规则

- 默认一次只引入一个游戏；同一上游仓库包含多个游戏时，可以整体固定并批量接入，但必须逐游戏完成元数据、构建、静态审计、冒烟、回放和截图验收。
- 开始移植前固定完整 commit SHA；只有单文件快照时记录内容哈希和原始 URL。
- 优先使用原始上游仓库；只有找不到更直接入口时才使用已有平台移植副本，并先分离其中的平台改动。
- 每款游戏必须建立 `game.toml`、独立构建入口、静态兼容审计、启动冒烟、固定输入回放和 ArduGirl 实际运行截图。
- 表中的“未导入、未构建”不能理解为预计可直接运行；兼容等级和优先级应随实际证据更新。

## 调研来源

- [Arduboy 官网精选游戏](https://www.arduboy.com/)
- [Arduboy 社区 Games 分类（按点赞）](https://community.arduboy.com/c/games/35?order=op_likes)
- [Erwin's Arduboy Collection](https://arduboy.ried.cl/)
- [ESP32 Arduboy 的可访问游戏源码集合](https://github.com/tonym128/ESP32_Arduboy/tree/master/GAMES)
- [近期玩家 Top 12 讨论](https://www.reddit.com/r/Arduboy/comments/1smod77/my_top_12_arduboy_games_so_far_i_havent_played/)
