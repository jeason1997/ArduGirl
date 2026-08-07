# 性能、内存与整数模型

## 结论

“8 位机器的代码搬到 32 位机器，所有整数都保持 8 位”不是正确优化方向。必须区分：

- **存储宽度**：数组、结构体、全局状态、资源和存档确实直接影响 RAM/Flash。
- **运算宽度**：C++ 会把多数 `uint8_t` 算术提升为 `int`；32 位 CPU 通常以 32 位寄存器运算最自然。
- **语义宽度**：计时回绕、坐标范围、随机数和存档协议必须明确位宽。

## `int` 变成 32 位会浪费吗

会，但只在某些位置明显：

- `int map[100]`：AVR 常为 200 bytes，32 位平台为 400 bytes。
- 含多个 `int` 字段且实例很多的对象：字段和 padding 可能增大。
- 全局或静态 `int` 状态：每个通常多 2 bytes。

不应简单累计每个局部变量的四字节。优化构建中，短生命周期标量通常存在寄存器或被消除。把局部循环计数强制成 `uint8_t` 还可能增加扩展、截断指令。

## 类型选择规则

| 用途 | 推荐类型 | 原因 |
|---|---|---|
| framebuffer/图片/地图 byte | `uint8_t` | 存储格式明确 |
| 屏幕坐标和裁剪计算 | `int16_t` | 覆盖屏外负坐标并匹配 Arduboy2 |
| 小计数/枚举存储 | `uint8_t`/窄 enum | 节省对象和数组空间 |
| 时间戳 | `uint32_t` | Arduino 回绕语义 |
| buffer size/index | 边界层 `size_t`，已知小范围可用 `uint16_t` | 避免指针算术错误 |
| 指针整数表示 | `uintptr_t` | 不截断 32/64 位指针 |
| 通用局部算术 | `int`/`unsigned` | 使用 CPU 自然宽度 |
| 文件/EEPROM 协议 | 显式字节编码 | 不受 ABI/padding 影响 |

## 内存预算

平台无关核心的持续 RAM 目标：

| 项目 | 目标 |
|---|---:|
| framebuffer | 1024 B 固定 |
| Arduboy2 状态 | <= 64 B |
| 按键/帧率/runtime 状态 | <= 64 B |
| 音频命令状态 | <= 64 B |
| 核心动态分配 | 0 B |

Linux 后端可拥有 RGBA staging texture 和 SDL 对象，但这些不能进入 MCU core。MCU 彩屏应按行或小块转换，避免额外完整 RGB framebuffer。

## Framebuffer 性能

- 保持页面布局，避免每帧重排核心 buffer。
- Linux 第一版可以每帧把 1024 bytes 展开为 128x64 RGBA；规模很小，优先保证正确。
- 后续如有证据再使用查表展开 8 个像素、dirty pages 或纹理格式优化。
- SSD1306 后端直接 DMA/SPI 发送 1024 bytes。
- 彩屏后端使用固定大小 scanline buffer 转 RGB565。

## 从 1-bit 屏幕迁移到 RGB 彩屏

不能只用 CPU 主频判断 Arduboy 游戏在新 MCU 上是否更快。原版 Arduboy 虽然只有 16 MHz，但 SSD1306 每帧直接接收 1024 字节页面 framebuffer；128×64 RGB565 后端每帧需要发送 `128 × 64 × 2 = 16384` 字节，传输量扩大 16 倍。新平台必须分别预算游戏绘制、像素格式转换和总线传输，不能把三者合并成一个模糊的“帧率问题”。

2026-08-07 的 PY32/ST7789 优化形成以下可复用规则：

1. **保留页面布局并优化页面操作。** 对 y 坐标和高度均按 8 像素对齐的 `drawOverwrite`、`drawSelfMasked` 和 `drawErase`，直接复制、OR 或 AND framebuffer 字节，一次处理纵向 8 个像素；裁剪或非对齐情况继续使用通用路径。该优化属于 Arduboy2/核心绘制层，可供所有平台复用。
2. **彩屏只保留小型转换缓冲区。** PY32 使用一条 128 像素 RGB565 扫描线，即 256 字节，不复制完整 16 KB 彩色 framebuffer。其他 RAM 紧张的 MCU 也应优先采用扫描线或小块缓冲。
3. **数据宽度必须与 DMA 配套。** 单独把 SPI 从 8 位数据帧改成 16 位，若 CPU 仍逐像素等待 TXE，线上数据量不变，实测不会自然提速，甚至可能因等待粒度和模式切换变慢。有效路径是 16 位 RGB565 缓冲区、SPI 16 位数据帧和 TX DMA 共同使用。
4. **DMA 请求映射和使能顺序必须来自已验证资料。** PY32 使用 DMA1 Channel 1 映射 SPI1_TX；先配置并使能 DMA 通道，再开启 SPI TX DMA 请求，完成后关闭请求和通道并等待 SPI BUSY 清零。不能只凭其他 STM32/PY32 型号的相似寄存器猜测。
5. **静态画面避免重复传输。** 对 framebuffer 计算固定宽度哈希；内容未变化时跳过 RGB565 展开和 SPI 传输，同时周期性强制刷新，避免哈希碰撞长期隐藏更新。若平台 RAM 允许，逐字节比较影子 framebuffer 的确定性更强。
6. **时钟、SysTick 和外设时钟必须一致。** 提高 SYSCLK 后同步更新 Flash 等待周期、`SystemCoreClock`、SysTick reload、APB 分频和 SPI 实际频率。用真实 tick 计数证明时间基准，而不是只检查配置常量。

本次 PY32 测量过程也说明应逐层定位：24 MHz、12 MHz SPI、逐像素 Sprites 和轮询发送时，Arduventure 约为 19 FPS；仅使用 16 位轮询 SPI 没有改善；加入 DMA 和静态帧跳过后，开场仍受软件绘制限制；最终使用 48 MHz、24 MHz SPI、16 位 DMA、静态帧跳过和页面字节 Sprites 快速路径，复位后 5 秒运行 303 个逻辑帧，并在 `globalCounter=254` 时进入标题画面。

移植其他平台时建议按以下顺序测量，避免同时修改后无法归因：

1. 空循环与 SysTick 的真实时间基准；
2. 只运行游戏绘制、不提交显示的逻辑帧耗时；
3. 固定纯色或固定 framebuffer 的完整传输耗时；
4. 像素格式转换加传输的总耗时；
5. 代表性游戏的静态画面、动态玩法和最坏帧耗时；
6. 固件 text/data/bss、剩余栈空间和长时间稳定性。

## 帧率与等待

- 使用单调时钟。
- `nextFrame()` 保持 32 位无符号时间差语义。
- Linux 等待采用 deadline + 短时精细等待，不在忙循环中持续占满 CPU。
- 不以显示器刷新率替代游戏帧率；关闭或配置 VSync，避免双重限速。

## 编译和测量

Release 建议开启优化与 section GC；MCU 可使用 LTO，但需单独验证：

```text
-O2 or -Os
-ffunction-sections -fdata-sections
--gc-sections
optional: -flto
```

每个性能修改至少报告一种证据：

- Linux benchmark 的 frame/update 时间；
- `size`/map 文件中的 text/data/bss；
- 对象大小 `static_assert` 或报告；
- MCU 周期计数或逻辑分析仪测量。

## 兼容审计规则

导入游戏时重点搜索：

```text
int [name][
sizeof(int)
sizeof(long)
pgm_read_word.*ptr
reinterpret_cast.*uint16_t
EEPROM.put
union
bitfield
```

不要机械替换所有 `int`。只修复确实承担存储协议、范围或溢出语义的变量。
