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

