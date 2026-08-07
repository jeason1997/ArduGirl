# ArduboyWorks 游戏集

本目录接入 obono 的 ArduboyWorks 多游戏仓库。上游源码固定在 `third_party/ArduboyWorks`，保持未修改；各子目录的 `game.toml` 分别记录 18 个成品游戏的入口和兼容状态。

18 个成品游戏均已成功生成 SDL2 可执行文件，并分别通过 180 帧无头启动冒烟。主构建文件根据本目录下一层的 `game.toml` 自动发现游戏，不维护游戏名白名单；上游存在 `MyArduboy2.h` 时自动接入符号转发，并按其声明探测可选音频接口。构建过程在生成目录中把 AVR 16 位函数指针表改写为宿主平台的原生函数指针访问；Lasers 的灰屏 AVR 汇编通过游戏目录内的编号统一 diff 补丁替换为等价 C++ 循环；Morse 的 USB HID 在 Linux 前端使用无副作用兼容接口。`MyArduboy2` 是上游自定义类，移植入口只为其重复声明提供到 Arduboy2 核心的符号转发，不承载兼容语义。上游子模块保持未修改。

新增同仓库游戏时，只需创建 `games/arduboyworks/<game-id>/game.toml`，并保证上游入口为 `<game-id>/<game-id>.ino`。若必须修正上游源码，补丁应放在该游戏的 `patches/` 下，采用 `0001-<作用域>-<说明>.patch` 形式的统一 diff，并由 `game.toml` 按编号顺序显式引用。集合 profile 不得通过文本或正则替换修改源码。每个游戏仍需独立完成构建、冒烟和文档验证。

构建与运行示例：

```bash
make hollow
make hopper
make chribocchi
make arduboyworks-build
make test-arduboyworks
```

当前结果证明全部游戏能够构建和启动。每个游戏完成固定输入回放后，还需从 ArduGirl 实际运行画面生成截图并补入对应说明，才可把完整移植标记为完成。
