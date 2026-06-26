#include "system/includes.h"
#include "system/event.h"
#include "key_event_deal.h"
#include "../led_effect/led_effect.h"

/* 按键事件处理器
 * 在SDK之前拦截按键事件（优先级3），可调用 sys_key_event_consume() 阻止SDK处理
 *
 * 事件来源（BT模式下）:
 *   IO0 单击 → KEY_MUSIC_PP       IO0 双击 → KEY_MUSIC_NEXT     IO0 三击 → KEY_MUSIC_PREV
 *   IO0 长按 → KEY_MUSIC_PP       IO0 hold → KEY_VOL_UP         IO0 抬起 → KEY_MUSIC_PLAYER_START
 *   IO1 单击 → KEY_MUSIC_NEXT     IO1 长按 → KEY_VOL_UP         IO1 hold → KEY_VOL_UP
 *   IO3 单击 → KEY_CHANGE_MODE
 *
 * 注意: init=1 表示驱动已判定完手势才发事件（不是按下/释放）。
 *       如需区分按下/释放，用抬起列（KEY_MUSIC_PLAYER_START）。
 */

static void my_key_handler(struct sys_event *event)
{
    u16 key = event->u.key.event;
    // u16 init = event->u.key.init;  // 固定=1，手势已判定
    // u32 value = event->u.key.value;

    switch (key) {

    // ============ 单击 / 长按（播放暂停）============
    case KEY_MUSIC_PP:
        printf("[KEY] 播放/暂停\n");
        led_effect_switch(1);          // 切到下一个效果
        // TODO: 切换LED灯效等自定义操作
        // sys_key_event_consume(&event->u.key);  // 需要时取消注释以阻止SDK处理
        break;

    // ============ 下一曲 / 双击 ============
    case KEY_MUSIC_NEXT:
        printf("[KEY] 下一曲\n");
        // TODO: 切到下一个灯效等
        break;

    // ============ 上一曲 / 三击 ============
    case KEY_MUSIC_PREV:
        printf("[KEY] 上一曲\n");
        // TODO: 切到上一个灯效等
        break;

    // ============ hold 循环触发（音量+/长按持续）============
    case KEY_VOL_UP:
        printf("[KEY] 音量+\n");
        // hold 会持续触发，注意不要在每次触发都做重复操作
        break;

    // ============ 抬起释放 ============
    case KEY_MUSIC_PLAYER_START:
        printf("[KEY] 按键抬起/释放\n");
        break;

    // ============ 切换模式 ============
    case KEY_CHANGE_MODE:
        printf("[KEY] 切换工作模式\n");
        break;

    // ============ 其他按键 ============
    default:
        printf("[KEY] 未处理事件: %d(0x%x)\n", key, key);
        break;
    }

    // 如果需要阻止SDK处理该按键事件，取消下面的注释即可
        sys_key_event_consume(&event->u.key);
}

// static void my_key_handler(struct sys_event *event)
// {
//     u16 key = event->u.key.event;
//     u16 init = event->u.key.init;
//     u32 value = event->u.key.value;

//     // init: 0=按下 1=松开
//     printf("[KEY] event=%d(0x%x)  value=%d  init=%d\n", key, key, value, init);

//     // 先不消耗，看看所有事件能不能到

// }

SYS_EVENT_HANDLER(SYS_KEY_EVENT, my_key_handler, 3);

void key_handler_init(void)
{
    printf("[USER_KEY] 按键拦截已启动 (优先级3)\n");
}
