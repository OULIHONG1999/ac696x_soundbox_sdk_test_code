#include "key_event_deal.h"
#include "key_driver.h"
#include "app_config.h"
#include "board_config.h"
#include "app_task.h"

#ifdef CONFIG_BOARD_AC696X_DEMO
/***********************************************************
 *				bt 模式的 iokey table
 *
 * KEY_IO_NUM_MAX = 物理 IO 按键数量
 * KEY_EVENT_MAX  = 6 (每列代表一类操作)
 *
 * 每列的触发条件和行为:
 *
 *  列0（单击）  : 按下后短时间松开 → 发一次事件
 *  列1（长按）  : 按住超过 long_time(ms) → 发一次事件
 *  列2（hold）  : 长按判定后继续按住 → 循环发事件(用于音量连加)
 *  列3（抬起）  : 松开按键 → 发一次事件(用于"按下开/松开关")
 *  列4（双击）  : 快速按两次 → 发一次事件
 *  列5（三击）  : 快速按三次 → 发一次事件
 *
 *  配 KEY_NULL = 不处理该操作
 *
 *  注意: 驱动在判定完手势后才发事件, init 固定为1,
 *        不区分按下/释放。如需区分需用抬起列。
 *
 *  按键值定义见: apps/soundbox/include/key_event_deal.h
 ***********************************************************/
#if TCFG_APP_BT_EN
const u16 bt_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //  列0:单击          列1:长按          列2:hold          列3:抬起       列4:双击           列5:三击
    [0] = {
        KEY_MUSIC_PP,	  KEY_MUSIC_PP,	  KEY_VOL_UP,	      KEY_MUSIC_PLAYER_START,	    KEY_MUSIC_NEXT,	KEY_MUSIC_PREV
    },
    // IO0: 单击=播放暂停, 长按=挂断, 双击=重拨, hold/抬起/三击 无
    [1] = {
        KEY_MUSIC_NEXT,	  KEY_VOL_UP,		  KEY_VOL_UP,	  KEY_NULL,	    KEY_NULL,			KEY_NULL
    },
    // IO1: 单击=下一曲, 长按=音量+(hold持续发)
    [2] = {
        KEY_MUSIC_NEXT,	  KEY_VOL_UP,		  KEY_VOL_UP,	  KEY_NULL,	    KEY_NULL,			KEY_NULL
    },
    // IO2: 同 IO1
    [3] = {
        KEY_CHANGE_MODE,  KEY_NULL,			  KEY_NULL,	  KEY_NULL,	    KEY_NULL,			KEY_NULL
    },
    // IO3: 单击=切换模式, 其他无
    [4] = {
        KEY_NULL,		  KEY_NULL,			  KEY_NULL,	  KEY_NULL,	    KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,		  KEY_NULL,			  KEY_NULL,	  KEY_NULL,	    KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				fm 模式的 iokey table
 ***********************************************************/
#if TCFG_APP_FM_EN
const u16 fm_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_MUSIC_PP,			KEY_POWEROFF,			KEY_POWEROFF_HOLD,	KEY_NULL,	KEY_CALL_LAST_NO,	KEY_NULL
    },
    [1] = {
        KEY_FM_NEXT_STATION,	KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [2] = {
        KEY_FM_PREV_STATION,	KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [3] = {
        KEY_CHANGE_MODE,		KEY_FM_SCAN_ALL,		KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_FM_NEXT_FREQ,		KEY_FM_SCAN_ALL_DOWN,	KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_FM_PREV_FREQ,		KEY_FM_SCAN_ALL_UP,		KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				linein 模式的 iokey table
 ***********************************************************/
#if TCFG_APP_LINEIN_EN
const u16 linein_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_MUSIC_PP,			KEY_POWEROFF,			KEY_POWEROFF_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [2] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [3] = {
        KEY_CHANGE_MODE,		KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				music 模式的 iokey table
 ***********************************************************/
#if TCFG_APP_MUSIC_EN
const u16 music_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_MUSIC_PP,			KEY_POWEROFF,			KEY_POWEROFF_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_MUSIC_NEXT,			KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [2] = {
        KEY_MUSIC_PREV,			KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [3] = {
        KEY_CHANGE_MODE,		KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				pc 模式的 iokey table
 ***********************************************************/
#if TCFG_APP_PC_EN
const u16 pc_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_MUSIC_PP,			KEY_POWEROFF,			KEY_POWEROFF_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_VOL_UP,				KEY_VOL_UP,				KEY_VOL_UP,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [2] = {
        KEY_VOL_DOWN,			KEY_VOL_DOWN,			KEY_VOL_DOWN,		KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [3] = {
        KEY_CHANGE_MODE,		KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				record 模式的 iokey table
 ***********************************************************/
#if TCFG_APP_RECORD_EN
const u16 record_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_MUSIC_PP,			KEY_POWEROFF,			KEY_POWEROFF_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [2] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [3] = {
        KEY_CHANGE_MODE,		KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				rtc 模式的 iokey table
 ***********************************************************/
#if TCFG_APP_RTC_EN
const u16 rtc_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_MUSIC_PP,			KEY_POWEROFF,			KEY_POWEROFF_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [2] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [3] = {
        KEY_CHANGE_MODE,		KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_RTC_SW_POS,			KEY_RTC_SW,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				spdif 模式的 iokey table
 ***********************************************************/
#if TCFG_APP_SPDIF_EN
const u16 spdif_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_MUSIC_PP,			KEY_POWEROFF,			KEY_POWEROFF_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_SPDIF_SW_SOURCE, KEY_NULL
    },
    [2] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_SPDIF_SW_SOURCE, KEY_NULL
    },
    [3] = {
        KEY_CHANGE_MODE,		KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				idle 模式的 iokey table
 ***********************************************************/
const u16 idle_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_NULL,			    KEY_POWER_ON,			KEY_POWER_ON_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,           KEY_NULL
    },
    [2] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,           KEY_NULL
    },
    [3] = {
        KEY_NULL,		        KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif
