#include "system/includes.h"
#include "app_config.h"
#include "led_effect/led_effect.h"
#include "key_handler/key_handler.h"

void user_main()
{
    led_effect_init();
    key_handler_init();
    printf("user main\n");
}