# ArduGirl Agent Guide

本文件只定义全仓通用规则。进入子目录工作时，还必须阅读距离目标文件最近的 `AGENTS.md`；平台实现、游戏导入等局部规则不在根文件重复维护。开始修改前先阅读根 `README.md`、`docs/README.md`，再按文档索引选择任务相关规格。

## 项目与架构边界

ArduGirl 是源码级 Arduboy 兼容运行层：游戏源码在目标平台原生编译，不模拟 ATmega32U4，也不执行 `.hex` ROM。

依赖方向只能是：

```text
game -> compat/arduboy2 -> core/platform API -> platform backend
```

- `games/` 不得调用 Linux、SDL、PY32、STM32 或其他平台 API。
- `src/arduboy2/` 和 `src/compat/` 不得包含 SDL 或 MCU HAL；`src/compat/` 只提供 Arduino/AVR 源码兼容。
- `platform/*` 不得包含游戏逻辑，也不得改变公共 framebuffer 格式。
- 平台差异优先在后端解决；只有真实的公共 API 差异才能进入兼容层。
- 根 `Makefile` 只选择平台并转发目标；平台构建细节属于 `platform/<platform>/`。

详细设计以 `docs/ARCHITECTURE.md` 为准；本文件只保留评审时必须执行的边界。

## 不可改变的兼容契约

- 逻辑屏幕固定为 128×64、1 bit，framebuffer 固定为 1024 字节。
- framebuffer 使用 SSD1306/Arduboy 页面布局：索引为 `x + (y / 8) * 128`，bit 为 `y & 7`。
- 按键为 LEFT、RIGHT、UP、DOWN、A、B 六位掩码。
- 时间戳使用无符号 32 位并支持自然回绕。
- 公共 API 和持久化格式使用 `<cstdint>` 固定宽度类型。
- 不得把 C++ 关键字 `int` 全局替换为 16 位，也不得把核心 framebuffer 改为 RGB/RGBA。
- core/compat 不得把异常、RTTI、线程或动态分配作为必要能力。

## 编码规则

- 核心语言基线为 C++17，并保持可由无操作系统 MCU 工具链编译。
- ArduGirl 自有源码、测试和构建脚本中的新增或修改注释使用中文；第三方上游注释保持原样。
- 注释解释契约、边界、平台差异、所有权和非直观实现，不逐句复述代码。
- 头文件使用 `#pragma once`；内部命名空间使用 `ardugirl`，兼容 API 保持上游名称。
- 优先使用固定宽度整数；需要回绕时显式使用无符号类型。
- `pgm_read_word/dword` 等读取不得依赖未对齐指针强转。
- 新平台实现既有 platform API，不复制 Arduboy2 或兼容层。

## 工作区与 Git

- 临时日志、转储、随机测试输出和中间文件不得写入工程目录。诊断输出放在操作系统临时目录，验证后清理；正式回归源码和固定基准按仓库结构保存。
- 不覆盖用户已有改动，不用破坏性 Git 命令清理工作树。
- 未经用户明确要求“提交”，不得创建 commit 或推送。
- 用户要求“提交”时，默认检查范围、执行适当验证、提交并用 Git 推送当前分支到 `origin`；不默认创建 Pull Request，不强制推送。
- 上游子模块日常只使用 `git submodule update --init --recursive`。除非用户明确要求更新 revision，不得使用 `--remote`、在子模块中 pull 或切换到浮动分支。

## 变更与验证

- 代码、测试、文档和 `docs/PROGRESS.md` 的当前状态必须一致；只更新受本次变更影响的状态和证据。
- 改变公共契约时同步更新 `docs/ARCHITECTURE.md` 和相关规格。
- 新兼容 API 必须有最小测试；修复游戏问题优先增加回归测试。
- 在 Linux 上至少运行与改动相关的格式检查、构建和测试；无法运行时明确记录原因。
- 性能改动必须提供基准或生成物尺寸证据。
- 用户要求“全部”时，先枚举完整目标集合，并逐项记录结果。聚合命令未结束、退出码非零或任一目标未运行时，不得宣称全部完成。

## 完成定义

功能只有在实现、测试、规格和进度一致时才算完成。游戏的额外完成条件由 `games/AGENTS.md` 和 `docs/GAME_PORTING.md` 定义；平台的硬件或运行验收由对应平台目录的 `AGENTS.md` 定义。
