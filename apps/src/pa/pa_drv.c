#include "pa_drv.h"
#include "../gpio/gpio_cfg.h"
#include "debug.h"

#define LOG_TAG_CONST       PA_DRV
#define LOG_TAG             "[PA_DRV]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

//=================================================================================
//                          PA使能电平极性控制
//=================================================================================
// 定义 PA_EN_ACTIVE_HIGH  → 高电平使能，低电平关闭（默认）
// 注释掉 PA_EN_ACTIVE_HIGH → 低电平使能，高电平关闭
//=================================================================================
#define PA_EN_ACTIVE_HIGH

#if defined(PA_EN_ACTIVE_HIGH)
    #define PA_ENABLE_LEVEL     LEVEL_HIGH
    #define PA_DISABLE_LEVEL    LEVEL_LOW
#else
    #define PA_ENABLE_LEVEL     LEVEL_LOW
    #define PA_DISABLE_LEVEL    LEVEL_HIGH
#endif

static u8 pa_status = 0;

void pa_enable(void)
{
    gpio_write(PA_EN_PORT, PA_ENABLE_LEVEL);
    pa_status = 1;
    log_info("PA enable\n");
}

void pa_disable(void)
{
    gpio_write(PA_EN_PORT, PA_DISABLE_LEVEL);
    pa_status = 0;
    log_info("PA disable\n");
}

u8 pa_get_status(void)
{
    return pa_status;
}
