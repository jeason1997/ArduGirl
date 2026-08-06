# ArduboyWorks 游戏集

本目录接入 obono 的 ArduboyWorks 多游戏仓库。上游源码固定在 `third_party/ArduboyWorks`，保持未修改；各子目录的 `game.toml` 分别记录 18 个成品游戏的入口和兼容状态。

当前已通过 SDL2 无头启动冒烟：Hollow Seeker、Hopper、Chri-Bocchi Cat。其余游戏已经拥有构建目标，但仍在处理旧版 Arduboy API、AVR 函数指针布局、AVR 汇编或 USB HID 等兼容差异，因此整体状态为 `partial`。

构建与运行示例：

```bash
make hollow
make hopper
make chribocchi
```

每个游戏完成固定输入回放后，还需从 ArduGirl 实际运行画面生成截图并补入对应说明，才可标记为完成。
