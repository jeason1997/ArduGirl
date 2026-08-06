# ArduGirl Agent Guide

本文件约束所有在本仓库工作的开发者和自动化 Agent。开始修改前必须先阅读根目录 `README.md`，再阅读与任务对应的 `docs/` 文档。

## 项目目标

ArduGirl 是源码级 Arduboy 兼容运行层。游戏源码在目标平台上原生编译，不模拟 ATmega32U4，也不执行 `.hex` ROM。

当前唯一实现目标是 Linux 纯终端前端。SDL2 调整到终端基线稳定之后，PY32、STM32 等后端只保留接口和规划；在终端基线通过前，不得提前引入 SDL 或具体 MCU HAL。

## 架构边界

依赖方向只能是：

```text
game -> compat/arduboy2 -> core/platform API -> platform backend
```

- `games/` 不得包含 SDL、Linux、PY32 或 STM32 API。
- `src/arduboy2/` 不得包含 SDL 或 MCU HAL。
- `src/compat/` 只实现 Arduino/AVR 源码兼容，不驱动硬件。
- `platform/*` 不得包含游戏逻辑，也不得修改 Arduboy framebuffer 的格式。
- SDL 类型不能出现在公共平台接口中。
- 平台差异优先在后端解决；只有真实 API 差异才能进入兼容层。

## 不可改变的兼容契约

- 逻辑屏幕固定为 128x64、1 bit，framebuffer 固定为 1024 字节。
- framebuffer 使用 SSD1306/Arduboy 页面布局：索引为 `x + (y / 8) * 128`，bit 为 `y & 7`。
- 按键为 LEFT、RIGHT、UP、DOWN、A、B 六位掩码。
- 时间戳使用无符号 32 位并支持自然回绕。
- 公共 API 和持久化格式使用 `<cstdint>` 固定宽度类型。
- 不得用宏或 typedef 企图把 C++ 关键字 `int` 全局改成 16 位。
- 不得为了桌面显示把核心 framebuffer 改成 RGB/RGBA。
- 禁止在 core/compat 中使用异常、RTTI、线程、动态分配作为必要能力。

## 编码规则

- 核心语言基线为 C++17；保持可在无操作系统 MCU 工具链上编译。
- 所有 ArduGirl 自有源代码注释必须使用中文，包括普通注释、文档注释、TODO、FIXME、测试说明和构建脚本注释；不得新增英文注释。
- 代码需要详细注释接口契约、位布局、边界条件、平台差异、时间回绕、所有权和非直观优化，不要只逐句复述代码。
- 第三方上游源码自带的英文注释可以原样保留，以维护许可证和可追溯性；ArduGirl 新增或修改的注释只能使用中文。
- 头文件使用 `#pragma once`。
- 内部命名空间使用 `ardugirl`；兼容 API 必须保持上游名称。
- 优先 `std::uint8_t`、`std::int16_t`、`std::uint32_t`；只在重载兼容等必要位置使用普通 `int`。
- 资源、数组元素、结构体字段按取值范围选宽度；临时计算值按目标 CPU 自然字宽选择。
- 有符号溢出不可作为游戏逻辑；需要回绕时显式使用无符号固定宽度类型。
- 禁止未对齐指针强转实现 `pgm_read_word/dword`，使用安全加载或 `memcpy`。
- 新平台必须实现同一组 platform API，不得复制一套 Arduboy2 实现。

## 游戏导入规则

- 只导入有明确源码许可证且允许再分发的版本。
- 每个游戏必须有 `game.toml`，记录上游 URL、固定 commit、许可证、依赖、补丁和兼容状态。
- 原始源码通过固定 revision 的 Git 子模块或可验证快照引入，不得直接修改；ArduGirl 适配优先放在兼容层和外部构建入口中。
- 默认禁止修改游戏和官方示例的上游文件。只有兼容层无法表达真实平台差异时，才能在 `patches/` 中保存可独立应用的最小补丁，并记录原因和上游 issue。
- 更新上游 revision 时必须先在无补丁模式下构建；已有补丁需要单独验证能否继续应用，不能把补丁内容揉进上游目录。
- 默认只能执行 `git submodule update --init --recursive`，它会检出主仓库记录的固定 commit。禁止自行执行 `git submodule update --remote`、在子模块内 `git pull`，或把子模块切到浮动分支。
- 只有用户明确要求“更新到最新版本”或指定 tag/commit/branch 时，才能移动上游 revision。若用户指定 branch，最终仍需把当时解析到的完整 commit SHA 记录进主仓库，不能持续跟踪该 branch。
- 每次移动上游 revision 都是独立任务：记录旧/新 SHA，重新检查许可证和依赖，确认子模块 clean，运行该游戏构建、测试和兼容审计，并更新 `game.toml`、`docs/UPSTREAMS.md` 与进度文档。
- 不得把只有 `.hex`/`.arduboy`、没有可再分发源码许可的游戏提交到仓库。
- 不得静默修改第三方作者或许可证信息。
- 一次只引入一个游戏，并先通过 license、build、smoke test。

## 实现顺序

1. 构建系统和最小平台接口。
2. Linux terminal：ANSI/Unicode 显示、raw input、单调时钟、主循环。
3. framebuffer 基础绘图和官方 HelloWorld 等示例。
4. `PROGMEM`、Arduino 基础函数、按钮边沿、帧率。
5. Sprites、Print、EEPROM 文件后端。
6. 三个许可证清晰的真实游戏形成终端兼容基线。
7. Linux SDL2：窗口、最近邻缩放、输入和音频。
8. 更多音频和第三方库。
9. Linux 基线稳定后才开始 PY32。

## Git 工作流

- 远程仓库固定为 `origin = https://github.com/jeason1997/ArduGirl.git`，默认开发分支为 `main`。
- 未经用户明确说“提交”，不得创建 commit，也不得推送。
- 当用户说“提交”时，默认同时执行：检查变更范围、运行适当验证、创建有意义的提交、推送当前分支到 `origin`。
- 如果推送因为认证、网络、分支保护或远程冲突失败，保留本地 commit，并明确报告原因和下一步。
- 不得使用强制推送，除非用户明确指定并确认其影响。

## 每次变更必须完成

- 更新 `docs/PROGRESS.md` 的状态、证据和下一步。
- 若改变公共契约，先更新 `docs/ARCHITECTURE.md` 和相关规格。
- 新兼容 API 必须有最小测试；修复游戏问题时优先增加回归测试。
- 在 Linux 上至少运行格式检查、构建和测试；无法运行时明确记录原因。
- 性能改动必须附基准或生成物尺寸证据，不凭直觉压缩类型。

## 完成定义

一个功能只有在代码、测试、文档和进度记录一致时才算完成。能编译但没有运行验证的功能标为 `partial`，不得标为 `done`。
