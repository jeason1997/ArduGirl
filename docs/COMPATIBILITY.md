# 兼容范围与差异策略

## 兼容等级

| 等级 | 游戏特征 | 目标 |
|---|---|---|
| A | 仅使用 Arduboy2/Sprites 和标准 C++ | 原样或仅改构建文件 |
| B | 使用 PROGMEM、EEPROM、Print、tone 等常见 Arduino API | 由兼容层解决 |
| C | 使用额外 Arduboy 库 | 按库逐个移植 |
| D | 直接访问 AVR 寄存器、ISR、SPI 或汇编 | 游戏补丁或暂不支持 |
| E | 只有 `.hex`，没有可用源码 | 不属于本项目范围 |

## 第一阶段 API

### Arduboy2

- framebuffer、clear、display、getBuffer
- 像素、直线、矩形、圆、三角形和圆角矩形图元
- 页面布局 bitmap、XY bitmap、compressed bitmap
- Sprites/SpritesB 的 overwrite、plus-mask、external-mask、self-mask 和 erase 模式
- frame rate、frameCount、everyXFrames
- pollButtons 和全部按钮查询
- 支持缩放、换行、光标查询、字符串和整数的基本文本输出
- audio enabled 状态、`initAudio`、定时 `playTone` 和停止接口；SDL2 后端输出方波。ArduboyTones 已实现单音、最多三音内联序列、PROGMEM/RAM 频率时长序列、休止、结束、重复、播放状态和音量模式 API；ArduboyPlaytune 已实现音符、停止、毫秒等待、标记、重复和移调，并支持两个方波声道；`playWave` 支持一次性播放 8 位无符号波表
- ATMlib 已实现四声道乐谱、延时、轨道调用与重复、音量/频率滑动、移调、琶音、音符截断、颤音/震音和滑音解释；主循环按单调时间推进节拍，SDL2 后端合成脉冲、三角和噪声波形

### Arduino

- `millis`, `micros`, `delay`
- `random`, `randomSeed`, `map`
- 常用 bit/byte/min/max/constrain 辅助项
- `Print` 的整数和字符串基本输出
- `EEPROM.read/write/update/get/put`

### AVR compatibility

- `PROGMEM`, `PSTR`, `F`
- `pgm_read_byte/word/dword/ptr`
- 常用 `_P` 内存和字符串函数
- `_BV`

不提供 AVR 寄存器、ISR、`cli/sei` 的假实现。静默空操作会制造难以定位的错误。

## AVR ABI 差异

| 项目 | ATmega32U4 常见值 | Linux/32-bit MCU 常见值 | 策略 |
|---|---:|---:|---|
| `char` | 8 | 8 | 不假设 signedness |
| `int` | 16 | 32 | API 固定宽度，游戏审计 |
| `long` | 32 | Linux x64 常为 64 | 不用于协议/布局 |
| pointer | 16 | 32/64 | 用真实指针或 `uintptr_t` |
| `double` | 与 float 同为 32 | 常为 64 | 避免浮点；必要时审计 |
| flash/RAM | 分离地址空间 | 通常统一 | PROGMEM 安全兼容 |

特别危险的模式：

```cpp
int values[] = {...};                 // 数组占用可能翻倍
pgm_read_word(&functionPointers[i]);  // 不能读取 32/64 位指针
int x = 32767; ++x;                   // 依赖 16 位有符号溢出，本身是 UB
EEPROM.put(addr, someNativeStruct);   // padding 和字段宽度变化
```

## 第三方库顺序

建议兼容顺序：

1. Arduboy2 + Sprites
2. Arduboy2Beep 的剩余差异
3. ArdBitmap
4. ArduboyFX 数据读取抽象
5. 由游戏样本驱动的其他库

ArduboyTones、ArduboyPlaytune 和 ATMlib 已作为独立兼容库实现。ArduboyTones 由 Rooftop Rescue 的固定上游样本驱动，以平台单调时间推进序列，不复制 AVR Timer3/ISR；ATMlib 保留乐谱语义但不复制 AVR Timer4/ISR。其他直接依赖 AVR timer/ISR 的库仍待真实游戏驱动后移植。

当前 `micros()` 使用平台单调高精度计数器并按无符号 32 位自然回绕；不再由毫秒值乘 1000 模拟。Linux SDL2 音频支持两个方波声道、一条波表流和四个 ATM 合成声部，不模拟 AVR 定时器寄存器。

`random()`、`randomSeed()` 以及 AVR 游戏可能直接使用的 `rand()`、`srand()` 共享兼容层自有的 32 位确定性状态，不依赖宿主 libc 的随机数实现。这保留 Arduino 所需的半开区间、重播种和零种子不改变状态语义，同时避免 MCU 固件引入 newlib 的可重入与 stdio 全局状态；具体随机序列不承诺与某个 Arduino core 或宿主 libc 相同。

## 测试策略

- 单元测试：绘图边界、遮罩、碰撞、按钮边沿、时间回绕、PROGMEM 加载。
- Golden test：给定绘图序列后比对完整 1024 字节 buffer。
- 编译矩阵：Debug、Release、ASan+UBSan；GCC 与 Clang。
- 游戏冒烟：启动、标题画面、模拟固定输入、EEPROM 往返。
- 差异审计：导入脚本搜索 AVR 寄存器、ISR、汇编、直接 SPI 和危险类型假设。
