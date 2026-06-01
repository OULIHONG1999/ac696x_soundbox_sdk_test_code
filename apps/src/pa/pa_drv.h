#ifndef __PA_DRV_H__
#define __PA_DRV_H__

#include "system/includes.h"
#include "app_config.h"

//=================================================================================
//                          PA功放驱动接口
//=================================================================================

/**
 * @brief 使能PA功放（拉高使能脚）
 */
void pa_enable(void);

/**
 * @brief 禁止PA功放（拉低使能脚）
 */
void pa_disable(void);

/**
 * @brief 获取PA当前状态
 * @return 1-已使能, 0-已禁止
 */
u8 pa_get_status(void);

#endif // __PA_DRV_H__
