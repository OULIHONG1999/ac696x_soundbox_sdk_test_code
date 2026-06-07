#include "system/includes.h"
#include "system/event.h"
#include "key_event_deal.h"

static void my_key_handler(struct sys_event *event)
{
    u16 key = event->u.key.event;
    u16 init = event->u.key.init;
    u32 value = event->u.key.value;

    // init: 0=按下 1=松开
    printf("[KEY] event=%d(0x%x)  value=%d  init=%d\n", key, key, value, init);

    // 先不消耗，看看所有事件能不能到
    // sys_key_event_consume(&event->u.key);
}

SYS_EVENT_HANDLER(SYS_KEY_EVENT, my_key_handler, 3);

void key_handler_init(void)
{
    printf("[USER_KEY] 按键拦截已启动 (优先级3)\n");
}
