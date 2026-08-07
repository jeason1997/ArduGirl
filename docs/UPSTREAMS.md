# 固定的上游版本

ArduGirl 的主仓库通过 Git 子模块 gitlink 固定每个上游的完整 commit SHA。下表是便于审查的镜像记录；实际检出版本以主仓库树中的 gitlink 为准。

| 上游 | 路径 | 固定 commit | 用途 | 更新策略 |
|---|---|---|---|---|
| Arduboy2 | `third_party/Arduboy2` | `bc460a2cff1a3e116880991aa2f88bae4b2e3160` | 官方 API、字体、官方示例 | 仅按用户明确要求更新 |
| MicroTD | `third_party/MicroTD` | `0c8958fdcf57060c1380b3ca72082ca45b7a2bb5` | 第一个社区游戏 | 仅按用户明确要求更新 |
| ArduboyWorks | `third_party/ArduboyWorks` | `d4b1f041789dcd1d71907654e4025d613b4ab420` | obono 的 18 个成品游戏 | 仅按用户明确要求更新；逐游戏验收 |
| Ardynia | `third_party/Ardynia` | `860312d2c22524b2b6e96951691e4d426eb3b701` | 双声道 ArduboyPlaytune 动作 RPG 样本 | 仅按用户明确要求更新 |
| Arduventure | `third_party/Arduventure` | `938fae770f04369ae4e4dd29c1039f7b43e7d96c` | 带 ATMlib 背景音乐的动作 RPG 样本 | 仅按用户明确要求更新 |
| ATMlib | `third_party/ATMlib` | `952d079f573c0a9361d9f1a101134b7df884ab12` | Arduventure 使用的四声道乐谱格式与 AVR 参考实现 | 仅按用户明确要求更新 |
| Helmets & Hordes | `third_party/Helmets-Hordes` | `87d1b7e1cc7419be84e078404cbceba58f76a64e` | ATMlib 横向地牢动作游戏 | 仅按用户明确要求更新 |
| Fantasy Rampage | `third_party/Fantasy-Rampage` | `29d30d3574e31074aa8bb5d070c073f652b85f1a` | ATMlib 元素卡牌游戏 | 仅按用户明确要求更新 |
| Sunfire | `third_party/Sunfire` | `e99fff73e2d7ba974b8fbef90f7adf90c764397b` | ATMlib 太空飞行射击游戏 | 仅按用户明确要求更新 |
| Twotris | `third_party/Twotris` | `73391606ba4b8e4e18e9838eb130aff410310599` | 双人下落方块与组合输入样本 | 仅按用户明确要求更新 |
| Rooftop Rescue | `third_party/Rooftop` | `cb8e9203f62f5ce49423742aa3fb7bc6e1ca3847` | 待移植的直升机救援游戏 | 仅按用户明确要求更新 |
| Ravine Despoiler | `third_party/Ravine-Despoiler` | `c48915c69c92c1f2e070775558866d84cd70b91b` | 待移植的街机投弹游戏 | 仅按用户明确要求更新 |
| Evade 2 | `third_party/Evade2` | `bc6fa60203afc35266b5d4c5c5140ce6a714334d` | 待移植的街机躲避游戏 | 仅按用户明确要求更新 |
| Blackjack | `third_party/Blackjack` | `72a6b1b7c583971568d92eca39c81b8cb99def29` | 待移植的卡牌游戏 | 仅按用户明确要求更新 |
| Waternet | `third_party/Waternet` | `2f5e8ce47f9bb292a8c30fdf04f91988f0c2a300` | 待移植的管线谜题 | 仅按用户明确要求更新 |
| LayingPipe | `third_party/LayingPipe` | `d3c0279079ef0a5c7161cbf39d2375e4d5a7aa25` | 待移植的连线谜题 | 仅按用户明确要求更新 |
| LATE | `third_party/LATE` | `643440bf49257cf070cd54ab6e24e5e8122c7442` | 待移植的三合一休闲谜题 | 仅按用户明确要求更新 |
| Glove | `third_party/Glove` | `81e9988fea3fc68edc984260b17d6f52beb5d497` | 待移植的迷宫动作冒险 | 仅按用户明确要求更新 |
| Kong | `third_party/Kong` | `ec95141029a2a401edcf97b4b8d0f121139a2d15` | 待移植的 Game & Watch 式街机游戏 | 仅按用户明确要求更新 |
| Kong II | `third_party/Kong-II` | `8ba0b35b998d2c52161964ee06f126704bea1b13` | 待移植的 Game & Watch 式街机游戏 | 仅按用户明确要求更新 |
| Oh Mummy | `third_party/OhMummy` | `a8f155afd5682236da394bfca07f0f65153b3146` | 待移植的迷宫街机游戏 | 仅按用户明确要求更新 |
| Arduboy Game Collection | `third_party/Arduboy-Game-Collection` | `b1ffab7e4f2591c93b18380d4ff6eb3011a71bac` | Pipe Boy 等候选游戏的合集上游 | 仅按用户明确要求更新 |
| Games'n Goblins | `third_party/Games-n-Goblins` | `56560944bf0b2fc17973d78e436e5a22efc3847f` | 待移植的限时拼图游戏 | 仅按用户明确要求更新 |
| Omega Chase | `third_party/OmegaChase` | `64bc7d38d21fe6684a147554b4b46b043fe6be4f` | 待移植的双摇杆式射击游戏 | 仅按用户明确要求更新 |
| Quadrastic | `third_party/Quadrastic` | `9d36cc53865f33c89467501c18cea2257e9db29e` | 待移植的区域争夺动作游戏 | 仅按用户明确要求更新 |
| Pyoro! | `third_party/Pyoro` | `edc9d63244e92946596c67f7af476eb10145055d` | 待移植的接物动作游戏 | 仅按用户明确要求更新 |
| The Bounce | `third_party/TheBounce` | `2fcc65fffc3c3a213d8882aaa8ee7811fd738cdd` | 待移植的物理平台游戏 | 仅按用户明确要求更新 |
| TiMiNoo | `third_party/TiMiNoo` | `73ccd7fee1e675ca36f016cde2b317dc0bdb2a84` | 待移植的虚拟宠物游戏 | 仅按用户明确要求更新 |

MicroTD 当前带有一份可独立重放的最小补丁：为 `buildSelectedTower()` 的成功路径补充缺失的 `true` 返回值，并显式标记预期的状态分支贯穿。该补丁只消除上游未定义行为和编译歧义，构建时应用到生成目录，子模块工作树保持 clean。

Arduventure 使用两份独立补丁：移除上游自带的 AVR `main()` 并以宿主宽度安全的 `pgm_read_ptr` 读取函数指针；把 GCC 接受的 C++20 风格指定初始化改写为等价 C++17 构造。补丁仅应用于生成目录，上游游戏和 ATMlib 子模块均保持 clean。上游许可证明确区分 MIT 代码与保留版权的剧情、角色及美术资产，导入不改变该授权边界。

## 日常检出

```bash
git submodule update --init --recursive
```

该命令只检出主仓库记录的 commit，不会自动升级到上游最新代码。

## 更新约束

- 禁止自动更新、定时更新或隐式跟随上游 branch。
- “更新到最新”必须由用户明确提出，并在一次受控任务中解析为完整 SHA。
- 指定 tag、branch 或缩写 SHA 时，最终也必须记录完整 SHA。
- 新版本只有在构建、测试和兼容审计通过后才能替换当前版本。
- 更新失败时继续使用上一个已验证 revision。
