#pragma once

#include "py32f0xx_ll_gpio.h"

// 所有板级引脚只在此处定义。更换 PCB 时不得修改核心、游戏或显示驱动。
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
