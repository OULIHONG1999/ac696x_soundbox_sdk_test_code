#include "pa_drv.h"
#include "../gpio/gpio_cfg.h"
#include "debug.h"

#define LOG_TAG_CONST       PA_DRV
#define LOG_TAG             "[PA_DRV]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

static u8 pa_status = 0;

void pa_enable(void)
{
    gpio_write(PA_EN_PORT, LEVEL_HIGH);
    pa_status = 1;
    log_info("PA enable\n");
}

void pa_disable(void)
{
    gpio_write(PA_EN_PORT, LEVEL_LOW);
    pa_status = 0;
    log_info("PA disable\n");
}

u8 pa_get_status(void)
{
    return pa_status;
}
