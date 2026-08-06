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
- 像素和基本图元
- bitmap、compressed bitmap
- Sprites/SpritesB
- frame rate、frameCount、everyXFrames
- pollButtons 和全部按钮查询
- 基本文本输出
- audio enabled 状态，以及 `initAudio`、`playTone`、`playScore`、`playWave` 和对应停止接口；Linux 音频后端接入前由 Arduboy2 兼容核心统一提供无声、非阻塞实现，游戏移植目录不得各自实现缺失的通用音频语义

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
2. ArduboyTones / Arduboy2Beep
3. ArdBitmap
4. ArduboyFX 数据读取抽象
5. 由游戏样本驱动的其他库

ATMlib、ArduboyPlaytune 和直接依赖 AVR timer/ISR 的库不进入第一阶段。

## 测试策略

- 单元测试：绘图边界、遮罩、碰撞、按钮边沿、时间回绕、PROGMEM 加载。
- Golden test：给定绘图序列后比对完整 1024 字节 buffer。
- 编译矩阵：Debug、Release、ASan+UBSan；GCC 与 Clang。
- 游戏冒烟：启动、标题画面、模拟固定输入、EEPROM 往返。
- 差异审计：导入脚本搜索 AVR 寄存器、ISR、汇编、直接 SPI 和危险类型假设。
