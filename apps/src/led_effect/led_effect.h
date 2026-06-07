#ifndef __LED_EFFECT_H__
#define __LED_EFFECT_H__

#include "system/includes.h"

void led_effect_init(void);
void led_effect_switch(u8 direct);   // 1=下一个效果, 0=上一个效果

// 粒子效果参数运行时调节
void led_effect_set_field_r(int r);    // 场半径 1~120
void led_effect_set_speed(int idx, int speed);  // 粒子idx(0~2)速度 0~20
void led_effect_show_status(void);     // 打印当前状态

#endif
