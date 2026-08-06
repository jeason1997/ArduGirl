# ArduboyWorks 游戏集

本目录接入 obono 的 ArduboyWorks 多游戏仓库。上游源码固定在 `third_party/ArduboyWorks`，保持未修改；各子目录的 `game.toml` 分别记录 18 个成品游戏的入口和兼容状态。

18 个成品游戏均已成功生成 SDL2 可执行文件，并分别通过 180 帧无头启动冒烟。构建过程在生成目录中把 AVR 16 位函数指针表改写为宿主平台的原生函数指针访问；Lasers 的灰屏 AVR 汇编通过独立转换脚本替换为等价 C++ 循环；Morse 的 USB HID 在 Linux 前端使用无副作用兼容接口。`MyArduboy2` 是上游自定义类，移植入口只为其重复声明提供到 Arduboy2 核心的符号转发，不承载兼容语义。上游子模块保持未修改。

构建与运行示例：

```bash
make hollow
make hopper
make chribocchi
make arduboyworks-build
make test-arduboyworks
```

当前结果证明全部游戏能够构建和启动。每个游戏完成固定输入回放后，还需从 ArduGirl 实际运行画面生成截图并补入对应说明，才可把完整移植标记为完成。
