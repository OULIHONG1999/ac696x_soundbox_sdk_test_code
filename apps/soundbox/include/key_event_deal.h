#ifndef __KEY_EVENT_DEAL_H__
#define __KEY_EVENT_DEAL_H__

#include "typedef.h"
#include "system/event.h"

enum {
    KEY_POWER_ON = 0x80,            //从0x80开始,避免与系统默认事件冲突
    KEY_POWER_ON_HOLD,              //开机/长按开机
    KEY_POWEROFF,                   //关机事件
    KEY_POWEROFF_HOLD,              //长按关机

    KEY_BT_DIRECT_INIT,             //蓝牙直连初始化
    KEY_BT_DIRECT_CLOSE,            //蓝牙直连关闭

    KEY_MUSIC_PP,                   //音乐播放/暂停
    KEY_MUSIC_PREV,                 //上一曲
    KEY_MUSIC_NEXT,                 //下一曲
    KEY_MUSIC_FF,                   //快进
    KEY_MUSIC_FR,                   //快退

    KEY_MUSIC_PLAYER_START,         //解码器启动
    KEY_MUSIC_PLAYER_END,           //解码器结束
    KEY_MUSIC_PLAYER_DEC_ERR,       //解码出错
    KEY_MUSIC_DEVICE_TONE_END,      //设备提示音播放结束
    KEY_MUSIC_PLAYER_QUIT,          //退出播放
    KEY_MUSIC_PLAYER_AUTO_NEXT,     //自动切下一曲
    KEY_MUSIC_PLAYER_PLAY_FIRST,    //播放列表第一首
    KEY_MUSIC_PLAYER_PLAY_LAST,     //播放列表最后一首
    KEY_MUSIC_CHANGE_REPEAT,        //切换循环模式（单曲/全部/随机）
    KEY_MUSIC_CHANGE_DEV,           //切换播放设备（SD/USB/FLASH）
    KEY_MUSIC_AUTO_NEXT_DEV,        //自动切到下一个设备播放
    KEY_MUSIC_CHANGE_DEV_REPEAT,    //切换设备循环
    KEY_MUSIC_SET_PITCH,            //设置变调
    KEY_MUSIC_SET_SPEED,            //设置变速
    KEY_MUSIC_PLAYE_BY_DEV_FILENUM, //按文件号播放
    KEY_MUSIC_PLAYE_BY_DEV_SCLUST,  //按簇号播放
    KEY_MUSIC_PLAYE_BY_DEV_PATH,    //按路径播放
    KEY_MUSIC_DELETE_FILE,          //删除当前文件
    KEY_MUSIC_PLAYE_NEXT_FOLDER,    //下一文件夹
    KEY_MUSIC_PLAYE_PREV_FOLDER,    //上一文件夹
    KEY_MUSIC_PLAYE_REC_FOLDER_SWITCH,  //切换录音文件夹
    KEY_MUSIC_PLAYER_AB_REPEAT_SWITCH,  //AB点循环开关

    KEY_VOL_UP,                     //音量+
    KEY_VOL_DOWN,                   //音量-

    KEY_CALL_LAST_NO,               //重拨/通话记录
    KEY_CALL_HANG_UP,               //挂断/拒接
    KEY_CALL_ANSWER,                //接听

    KEY_OPEN_SIRI,                  //唤醒语音助手（Siri等）
    KEY_HID_CONTROL,                //HID控制（遥控器/键盘）
    KEY_LOW_LANTECY,                //低延迟模式开关

    KEY_CHANGE_MODE,                //切换工作模式（BT→FM→LINEIN→...）

    KEY_EQ_MODE,                    //切换EQ模式
    KEY_THIRD_CLICK,                //三击事件

    KEY_FM_SCAN_ALL,                //FM全自动搜台
    KEY_FM_SCAN_ALL_UP,             //FM向上全自动搜台
    KEY_FM_SCAN_ALL_DOWN,           //FM向下全自动搜台
    KEY_FM_PREV_STATION,            //FM上一电台
    KEY_FM_NEXT_STATION,            //FM下一电台
    KEY_FM_PREV_FREQ,               //FM上一频率（微调）
    KEY_FM_NEXT_FREQ,               //FM下一频率（微调）
    KEY_FM_SCAN_UP,                 //FM半自动搜台（向上）
    KEY_FM_SCAN_DOWN,               //FM半自动搜台（向下）
    KEY_FM_DEL_STATION,             //FM删除当前频道

    KEY_FM_EMITTER_MENU,            //FM发射器菜单
    KEY_FM_EMITTER_NEXT_FREQ,       //FM发射器下一频率
    KEY_FM_EMITTER_PERV_FREQ,       //FM发射器上一频率

    KEY_RTC_UP,                     //RTC时钟+
    KEY_RTC_DOWN,                   //RTC时钟-
    KEY_RTC_SW,                     //RTC时钟设置切换
    KEY_RTC_SW_POS,                 //RTC光标位置切换

    KEY_SPDIF_SW_SOURCE,            //SPDIF切换音源

    KEY_BT_EMITTER_SW,              //蓝牙发射器开关
    KEY_BT_EMITTER_PLAY,            //蓝牙发射器播放
    KEY_BT_EMITTER_PAUSE,           //蓝牙发射器暂停
    KEY_BT_EMITTER_RECEIVER_SW,     //蓝牙发射/接收模式切换

    KEY_SWITCH_PITCH_MODE,          //切换变调模式
    KEY_ENC_START,                  //编码/录音开始
    KEY_REVERB_OPEN,                //混响开关
    KEY_REVERB_DEEPVAL_UP,          //混响深度+
    KEY_REVERB_DEEPVAL_DOWN,        //混响深度-
    KEY_REVERB_GAIN0_UP,            //混响增益0+
    KEY_REVERB_GAIN1_UP,            //混响增益1+
    KEY_REVERB_GAIN2_UP,            //混响增益2+
    // KEY_REVERB_GAIN_DOWN,         //（废弃）

    KEY_TM_GMA_SEND,                //天猫精灵语音发送
    KEY_APP_SEND_SPEECH_START,      //APP发起语音开始
    KEY_SEND_SPEECH_START,          //发起语音开始
    KEY_SEND_SPEECH_STOP,           //发起语音结束
    KEY_AI_DEC_SUSPEND,             //AI解码暂停
    KEY_AI_DEC_RESUME,              //AI解码继续
    KEY_DUEROS_CONNECTED,           //小度连接成功
    KEY_DUEROS_DISCONNECTED,        //小度断开连接
    KEY_DUEROS_VER,                 //小度版本信息
    KEY_DUEROS_SEND,                //小度语音发送
    KEY_TWS_DUEROS_RAND_SET,        //TWS小度随机数设置
    KEY_TWS_BLE_SLAVE_SPEECH_START, //TWS从机BLE语音开始
    KEY_SPEECH_START_FROM_TWS,      //TWS发起语音
    KEY_SPEECH_STOP_FROM_TWS,       //TWS结束语音
    KEY_TWS_BLE_DUEROS_CONNECT,     //TWS BLE小度连接
    KEY_TWS_BLE_DUEROS_DISCONNECT,  //TWS BLE小度断开

    KEY_TWS_SEARCH_PAIR,            //TWS搜索配对
    KEY_TWS_REMOVE_PAIR,            //TWS删除配对
    KEY_TWS_SEARCH_REMOVE_PAIR,     //TWS搜索并删除配对
    KEY_TWS_DISCONN,                //TWS断开连接
    KEY_TWS_CONN,                   //TWS连接

    KEY_BOX_POWER_CLICK,            //充电仓按键单击
    KEY_BOX_POWER_LONG,             //充电仓按键长按
    KEY_BOX_POWER_HOLD,             //充电仓按键hold（持续触发）
    KEY_BOX_POWER_UP,               //充电仓按键抬起
    KEY_BOX_POWER_DOUBLE,           //充电仓按键双击
    KEY_BOX_POWER_THREE,            //充电仓按键三击
    KEY_BOX_POWER_FOUR,             //充电仓按键四击
    KEY_BOX_POWER_FIVE,             //充电仓按键五击

    ///soundcard相关按键消息
    KEY_SOUNDCARD_MODE_ELECTRIC,    //声卡电音模式
    KEY_SOUNDCARD_MODE_PITCH,       //声卡变调模式
    KEY_SOUNDCARD_MODE_PITCH_BY_VALUE,  //声卡按值变调
    KEY_SOUNDCARD_MODE_MAGIC,       //声卡魔音模式
    KEY_SOUNDCARD_MODE_BOOM,        //声卡爆音模式
    KEY_SOUNDCARD_MODE_SHOUTING_WHEAT,  //声卡喊麦模式
    KEY_SOUNDCARD_MODE_DODGE,       //声卡闪避模式
    KEY_SOUNDCARD_MODE_ELECTRIC_CANCEL, //声卡电音取消

    KEY_SOUNDCARD_MAKE_NOISE0,      //声卡造乐0
    KEY_SOUNDCARD_MAKE_NOISE1,      //声卡造乐1
    KEY_SOUNDCARD_MAKE_NOISE2,      //声卡造乐2
    KEY_SOUNDCARD_MAKE_NOISE3,      //声卡造乐3
    KEY_SOUNDCARD_MAKE_NOISE4,      //声卡造乐4
    KEY_SOUNDCARD_MAKE_NOISE5,      //声卡造乐5
    KEY_SOUNDCARD_MAKE_NOISE6,      //声卡造乐6
    KEY_SOUNDCARD_MAKE_NOISE7,      //声卡造乐7
    KEY_SOUNDCARD_MAKE_NOISE8,      //声卡造乐8
    KEY_SOUNDCARD_MAKE_NOISE9,      //声卡造乐9
    KEY_SOUNDCARD_MAKE_NOISE10,     //声卡造乐10
    KEY_SOUNDCARD_MAKE_NOISE11,     //声卡造乐11

    ///旋钮按键
    KEY_SOUNDCARD_SLIDE_MIC,        //声卡麦克风音量调节
    KEY_SOUNDCARD_SLIDE_WET_GAIN,   //声卡湿声增益调节
    KEY_SOUNDCARD_SLIDE_HIGH_SOUND, //声卡高音调节
    KEY_SOUNDCARD_SLIDE_LOW_SOUND,  //声卡低音调节
    KEY_SOUNDCARD_SLIDE_RECORD_VOL, //声卡录音音量调节
    KEY_SOUNDCARD_SLIDE_MUSIC_VOL,  //声卡音乐音量调节
    KEY_SOUNDCARD_SLIDE_EARPHONE_VOL, //声卡耳机音量调节

    KEY_SOUNDCARD_USB_MIC_MUTE_SWICH,       //声卡USB麦克风静音开关
    KEY_SOUNDCARD_NORMAL_MIC_STATUS_UPDATE, //声卡普通麦克风状态更新
    KEY_SOUNDCARD_EAR_MIC_STATUS_UPDATE,    //声卡耳麦状态更新
    KEY_SOUNDCARD_AUX_STATUS_UPDATE,        //声卡AUX状态更新

    KEY_TEST_DEMO_0,                //测试demo按键0
    KEY_TEST_DEMO_1,                //测试demo按键1

    KEY_IR_NUM_0,                   //红外数字0（中间不允许插入）
    KEY_IR_NUM_1,                   //红外数字1
    KEY_IR_NUM_2,                   //红外数字2
    KEY_IR_NUM_3,                   //红外数字3
    KEY_IR_NUM_4,                   //红外数字4
    KEY_IR_NUM_5,                   //红外数字5
    KEY_IR_NUM_6,                   //红外数字6
    KEY_IR_NUM_7,                   //红外数字7
    KEY_IR_NUM_8,                   //红外数字8
    KEY_IR_NUM_9,                   //红外数字9（中间不允许插入）
    //在这里增加元素
    //
    KEY_HID_MODE_SWITCH,            //HID模式切换
    KEY_HID_TAKE_PICTURE,           //HID拍照
    KEY_LINEIN_START,               //LINEIN开始
    MSG_HALF_SECOND,                //半秒定时事件
    //不会出现在按键主流程，用于不重要得其他操作
    KEY_MINOR_OPT,                  //次要操作（低优先级）
    KEY_TONE_PLAY,                  //提示音播放

    KEY_NULL = 0xFFFF,              //空事件（无操作）

    KEY_MSG_MAX = 0xFFFF,           //按键消息最大值
    //音箱sdk 按键消息已经加大为0xffff
};


enum {
    ONE_KEY_CTL_NEXT_PREV = 1,      //单键控制：上/下一曲
    ONE_KEY_CTL_VOL_UP_DOWN,        //单键控制：音量+-
};


#endif
