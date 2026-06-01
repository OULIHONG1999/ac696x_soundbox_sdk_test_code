#include "gpio_cfg.h"
#include "debug.h"

#define LOG_TAG_CONST       GPIO_CFG
#define LOG_TAG             "[GPIO_CFG]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

/**
 * @brief GPIO全局初始化
 * @note  在board_init()中调用，统一初始化所有用户GPIO
 */
void gpio_cfg_init(void)
{
    log_info("gpio cfg init\n");

    // PA功放使能脚初始化（初始关闭）
    gpio_set_direction(PA_EN_PORT, GPIO_DIR_OUTPUT);
    gpio_set_pull_up(PA_EN_PORT, 0);
    gpio_set_pull_down(PA_EN_PORT, 1);
    gpio_write(PA_EN_PORT, LEVEL_LOW);

    // 其他GPIO初始化...
}
