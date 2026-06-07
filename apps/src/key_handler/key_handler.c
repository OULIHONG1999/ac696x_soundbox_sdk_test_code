#include "system/includes.h"
#include "system/event.h"
#include "key_event_deal.h"
#include "led_effect/led_effect.h"

// ============ 按键事件映射表 ============
// IO0: 单击=PP → 显示状态, 长按=KEY_CALL_HANG_UP → 切换场半径, 双击=KEY_CALL_LAST_NO → 重置粒子
// IO1: 单击=NEXT → 下个效果, 长按=VOL_UP → 粒子加速+1, hold=VOL_UP → 继续加速

static int g_fast_mode = 0;   // 用于场半径切换

static void my_key_handler(struct sys_event *event)
{
    u16 key = event->u.key.event;
    u16 value = (u16)event->u.key.value;   // 原始按键值

    switch (key) {

    // ======== IO0: 单击 → 显示状态 ========
    case KEY_MUSIC_PP:
        printf("─────────── LED 调试菜单 ───────────\n");
        led_effect_show_status();
        printf("  [长按] 切换场半径 30↔60\n");
        printf("  [双击] 重置粒子位置\n");
        printf("  [单击IO1] 下个效果\n");
        printf("  [长按IO1] 粒子加速\n");
        printf("────────────────────────────────\n");
        break;

    // ======== IO0: 长按 → 切换场半径 ========
    case KEY_CALL_HANG_UP:
        g_fast_mode = !g_fast_mode;
        if (g_fast_mode) {
            led_effect_set_field_r(60);
        } else {
            led_effect_set_field_r(30);
        }
        break;

    // ======== IO0: 双击 → 重置粒子位置 ========
    case KEY_CALL_LAST_NO:
        printf("[USER_KEY] 长按 → 粒子速度+1\n");
        for (int i = 0; i < 3; i++) {
            led_effect_set_speed(i, 5);
        }
        break;

    // ======== IO1: 单击 → 下个效果 ========
    case KEY_MUSIC_NEXT:
        led_effect_switch(1);
        break;

    // ======== IO1: 长按/Hold → 粒子加速 ========
    case KEY_VOL_UP:
        printf("[USER_KEY] 长按 → 所有粒子速度+1\n");
        for (int i = 0; i < 3; i++) {
            led_effect_set_speed(i, 5);
        }
        break;

    default:
        printf("[USER_KEY] 未处理按键: %d (0x%x)\n", key, key);
        break;
    }

    sys_key_event_consume(&event->u.key);
}

SYS_EVENT_HANDLER(SYS_KEY_EVENT, my_key_handler, 3);

void key_handler_init(void)
{
    printf("[USER_KEY] 按键拦截已启动 (优先级3)\n");
    printf("[USER_KEY]   IO0: 单击=状态  长按=场半径  双击=重置\n");
    printf("[USER_KEY]   IO1: 单击=效果  长按=加速\n");
}
