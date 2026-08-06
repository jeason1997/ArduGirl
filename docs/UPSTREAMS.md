# 固定的上游版本

ArduGirl 的主仓库通过 Git 子模块 gitlink 固定每个上游的完整 commit SHA。下表是便于审查的镜像记录；实际检出版本以主仓库树中的 gitlink 为准。

| 上游 | 路径 | 固定 commit | 用途 | 更新策略 |
|---|---|---|---|---|
| Arduboy2 | `third_party/Arduboy2` | `bc460a2cff1a3e116880991aa2f88bae4b2e3160` | 官方 API、字体、官方示例 | 仅按用户明确要求更新 |
| MicroTD | `third_party/MicroTD` | `0c8958fdcf57060c1380b3ca72082ca45b7a2bb5` | 第一个社区游戏 | 仅按用户明确要求更新 |
| ArduboyWorks | `third_party/ArduboyWorks` | `d4b1f041789dcd1d71907654e4025d613b4ab420` | obono 的 18 个成品游戏 | 仅按用户明确要求更新；逐游戏验收 |
| Ardynia | `third_party/Ardynia` | `860312d2c22524b2b6e96951691e4d426eb3b701` | 双声道 ArduboyPlaytune 动作 RPG 样本 | 仅按用户明确要求更新 |

MicroTD 当前带有一份可独立重放的最小补丁：为 `buildSelectedTower()` 的成功路径补充缺失的 `true` 返回值，并显式标记预期的状态分支贯穿。该补丁只消除上游未定义行为和编译歧义，构建时应用到生成目录，子模块工作树保持 clean。

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
