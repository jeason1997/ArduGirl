# 参考工程与资料

## 核心上游

### Arduboy2

- Repository: https://github.com/MLXXXp/Arduboy2
- Documentation: https://mlxxxp.github.io/documents/Arduino/libraries/Arduboy2/Doxygen/html/index.html
- 用途：公共 API、framebuffer、绘图、Sprites、按钮和帧率行为的事实基线。
- 注意：`Arduboy2Core`、PROGMEM 和部分优化直接依赖 AVR，不能原样作为多平台层。

采用上游代码前必须记录固定 commit 并保留其许可证。不要仅复制零散代码而丢失版权信息。

## 源码移植参考

### ESP32_Arduboy

- Repository: https://github.com/tonym128/ESP32_Arduboy
- 用途：32 位指针、EEPROM、TFT、按键、声音和真实游戏适配经验。
- 可借鉴：`pgm_read_word` 读取指针的问题、直接 SPI 游戏的分类、EEPROM commit 差异。
- 不直接作为核心：项目与 ESPboy/Arduino/TFT 绑定较深，且游戏集合必须逐个核对许可证。

### ESPboy Arduboy2 library

- Repository: https://github.com/ESPboy-edu/ESPboy_Arduboy2_lib
- 用途：ESP8266 平台的 Arduboy2 源码重编译参考。

## 模拟器参考（仅作行为对照）

### Ardens

- Repository: https://github.com/tiberiusbrown/ardens
- 用途：验证原机行为、像素输出、时序和边界情况。
- 限制：ArduGirl 不采用它的 AVR 模拟执行路线，也不以 `.hex` 为输入。

## Linux 平台

### SDL2

- Website: https://www.libsdl.org/
- Repository: https://github.com/libsdl-org/SDL/tree/SDL2
- Documentation: https://wiki.libsdl.org/SDL2/FrontPage
- Support status: https://wiki.libsdl.org/SDL2/Support
- 用途：Linux 窗口、输入、计时、音频和后续 gamepad 支持。

SDL2 类型只能存在于 `platform/linux_sdl2` 内部。未来可增加 SDL3 backend，但不改变平台公共接口。

### POSIX terminal

- termios: https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/termios.h.html
- ANSI terminal control: https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
- 用途：TTY raw input、光标定位、差分刷新和退出恢复。

## 游戏发现渠道

- Arduboy Community: https://community.arduboy.com/
- GitHub topic: https://github.com/topics/arduboy-game
- Erwin's Arduboy Collection: https://arduboy.ried.cl/
- Arduboy Cloud: https://cloud.arduboy.com/
- Cart Builder: https://www.bloggingadeadhorse.com/cart/CartBuilder.html

这些目录适合寻找作者和 source URL，不代表其中所有游戏都允许把源码、音乐和素材复制进 ArduGirl。

## 语言和 ABI 参考

- Fixed-width integers: https://en.cppreference.com/w/cpp/types/integer
- Integer conversions/promotions: https://en.cppreference.com/w/cpp/language/implicit_conversion
- Object size/alignment: https://en.cppreference.com/w/cpp/language/object

规范实现时优先查阅目标编译器 ABI、芯片参考手册和官方 SDK 文档，不依靠 AVR 上的偶然行为。
