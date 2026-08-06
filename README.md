# ArduGirl

ArduGirl 让 Arduboy 游戏源码直接编译到 Linux、PY32、STM32 等平台。它不模拟 AVR，也不运行 `.hex` 文件；游戏继续包含官方 `Arduboy2.h`，平台差异由兼容层处理。

项目当前默认使用 Linux SDL2 前端。默认示例是 Arduboy2 官方 `HelloWorld.ino`，上游文件通过 Git 子模块引入并保持不修改。终端后端继续保留为可选目标，MCU 后端将在 Linux 基线稳定后实现。

## 快速开始

环境要求：Linux 或 WSL、GNU Make、支持 C++17 的 GCC/Clang、`pkg-config` 和 SDL2 开发包。

```bash
git clone --recursive https://github.com/jeason1997/ArduGirl.git
cd ArduGirl
make
make run
```

已有工作副本需要初始化上游子模块：

```bash
git submodule update --init --recursive
```

运行默认 SDL 测试或限帧窗口演示：

```bash
make test
make demo
```

SDL 窗口默认按 1:1 显示 128x64 单色画面，可通过 `--scale N` 指定最近邻整数缩放倍率。方向键或 `WASD` 控制方向，`J/Z` 为 A 键，`K/X` 为 B 键，按 Escape 退出。无显示服务器时可用 `--headless --frames N` 执行固定帧测试。

EEPROM 默认保存在 XDG data 目录的 `ardugirl/<game-id>/eeprom.bin`。测试或便携运行可用 `--save-dir PATH` 指定存档根目录，各游戏仍会使用独立子目录。

运行首个社区游戏 MicroTD：

```bash
make microtd
```

终端后端暂不继续优化，可通过 `make terminal`、`make run-terminal`、`make microtd-terminal` 和 `make test-terminal` 显式构建或测试。

## 上游源码原则

游戏和官方示例尽量保持原文件不变。ArduGirl 使用固定上游 revision、外部构建入口和兼容 API 完成移植；无法通过兼容层解决时，才允许建立可独立审查的最小补丁。

## 文档

- [架构与平台契约](docs/ARCHITECTURE.md)
- [兼容范围](docs/COMPATIBILITY.md)
- [游戏源码获取与导入](docs/GAME_PORTING.md)
- [固定的上游版本](docs/UPSTREAMS.md)
- [性能与整数模型](docs/PERFORMANCE.md)
- [SDL2 前端](docs/LINUX_SDL.md)
- [终端前端](docs/LINUX_TERMINAL.md)
- [路线图](docs/ROADMAP.md)
- [当前进度](docs/PROGRESS.md)
- [参考资料](docs/REFERENCES.md)
