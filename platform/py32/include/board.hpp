#pragma once

#include "py32f0xx_ll_gpio.h"

// 所有板级引脚只在此处定义。更换 PCB 时不得修改核心、游戏或显示驱动。
// 当前样机未连接实体按键，因此运行时固定报告全部松开；接入按键后改为 1。
#define ARDUGIRL_BUTTONS_CONNECTED 0
#define ARDUGIRL_BUTTON_PORT GPIOB
#define ARDUGIRL_BUTTON_LEFT LL_GPIO_PIN_0
#define ARDUGIRL_BUTTON_RIGHT LL_GPIO_PIN_1
#define ARDUGIRL_BUTTON_UP LL_GPIO_PIN_2
#define ARDUGIRL_BUTTON_DOWN LL_GPIO_PIN_3
#define ARDUGIRL_BUTTON_A LL_GPIO_PIN_4
#define ARDUGIRL_BUTTON_B LL_GPIO_PIN_6

// 蜂鸣器采用软件相位累加输出，因而只要求普通 GPIO，不依赖固定定时器通道。
#define ARDUGIRL_BUZZER_PORT GPIOB
#define ARDUGIRL_BUZZER_PIN LL_GPIO_PIN_5

// ST7789 调色板使用 RGB565。0 bit 映射到背景色，1 bit 映射到前景色。
#define ARDUGIRL_COLOR_BLACK_RGB565 0x0000u
#define ARDUGIRL_COLOR_WHITE_RGB565 0xF900u
