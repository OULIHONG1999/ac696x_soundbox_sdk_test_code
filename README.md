# AC696x 蓝牙音箱 SDK

基于杰理 AC696x 芯片的蓝牙音箱开发项目。

## 目录结构

```
ac696x_soundbox_sdk-测试代码/
├── apps/                        # 应用层代码
│   ├── src/                     # 用户自定义代码（完全隔离于SDK）
│   │   ├── gpio/                # GPIO配置管理
│   │   ├── pa/                  # PA功放驱动
│   │   ├── led_effect/          # LED灯效控制（火焰/呼吸/流光/粒子）
│   │   ├── key_handler/         # 按键事件拦截与自定义处理
│   │   ├── user_main.c          # 用户主入口
│   │   └── Makefile             # 用户源文件列表
│   ├── soundbox/                # SDK应用框架
│   └── common/                  # 通用应用模块
├── cpu/br25/                    # 芯片相关代码（音频、驱动等）
│   └── audio_dec/              # 音频解码模块（含PA控制逻辑）
├── scripts/                     # PC端开发工具脚本
│   └── particles_sim.py        # LED粒子效果可视化模拟器
├── include_lib/                 # SDK库文件和头文件
├── doc/                         # 项目文档
├── Makefile                     # 主编译脚本
└── README.md                    # 本文件
```

## 硬件信息

- **芯片**: AC696x (杰理)
- **PA使能脚**: IO_PORTA_02（高电平有效，可通过宏切换为低电平有效）
- **DAC**: 芯片内部集成，模拟输出连接外部功放
  - 当前输出模式：**双声道差分**（`DAC_OUTPUT_DUAL_LR_DIFF`）
  - 信号：L+/L-、R+/R- 两对差分线
- **串口日志**: IO_PORT_DP（UART0 TX，波特率 1000000）
- **RGB LED**: 8个（通过 SPI 驱动，连接到 JL_SPI2，Z字形排列）

## 编译方法

```bash
# Windows
make

# 显示详细编译过程
make VERBOSE=1

# 清除编译文件
make clean
```

## 主要模块

| 模块 | 说明 |
|------|------|
| `apps/soundbox/` | 蓝牙音箱应用主框架 |
| `apps/src/` | 用户自定义代码（PA驱动、GPIO配置、LED灯效、按键处理等） |
| `apps/src/led_effect/` | 4种RGB LED灯效（火焰/呼吸/流光/粒子） |
| `apps/src/key_handler/` | 按键事件拦截与调试菜单 |
| `scripts/` | PC端开发工具（粒子效果可视化模拟器） |
| `cpu/br25/audio_dec/` | 音频解码模块（含PA控制逻辑） |
| `cpu/br25/audio_effect/` | 音频效果处理 |

---

## PA 功放控制

### 引脚定义

`apps/src/gpio/gpio_cfg.h`:

```c
#define PA_EN_PORT    IO_PORTA_02
```

### 使能电平极性

`apps/src/pa/pa_drv.c` 使用宏 `PA_EN_ACTIVE_HIGH` 控制极性：

```c
#define PA_EN_ACTIVE_HIGH           // 定义：高电平使能，低电平关闭（默认）
// #define PA_EN_ACTIVE_HIGH        // 注释掉：低电平使能，高电平关闭
```

### PA 触发逻辑

PA 使能/关闭由各音频解码器的启动/关闭函数控制，覆盖**所有音频播放路径**：

| 音频路径 | 使能位置 | 关闭位置 |
|---------|---------|---------|
| **A2DP 蓝牙** (audio_dec_bt.c) | `a2dp_dec_start()` → `audio_decoder_start()` 成功后 | `a2dp_audio_res_close()` 末尾 |
| **文件播放** (audio_dec_file.c) | `file_dec_start()` → `audio_decoder_start()` 成功后 | `file_dec_close()` 末尾 |
| **FM 收音** (audio_dec_fm.c) | `fm_dec_start()` → `audio_decoder_start()` 成功后 | `fm_dec_close()` 末尾 |
| **LINE IN** (audio_dec_linein.c) | `linein_dec_start()` → `audio_decoder_start()` 成功后 | `linein_dec_close()` 末尾 |
| **PC USB** (audio_dec_pc.c) | `uac_audio_start()` → `audio_decoder_start()` 成功后 | `uac_audio_close()` 末尾 |
| **SPDIF** (audio_dec_spdif.c) | `spdif_dec_start()` → `audio_decoder_start()` 成功后 | `spdif_dec_close()` 末尾 |
| **MIDI 控制** (audio_dec_midi_ctrl.c) | `midi_ctrl_dec_start()` → `audio_decoder_start()` 成功后 | `__midi_ctrl_dec_close()` 末尾 |
| **提示音/正弦波** (audio_dec_tone.c) | `tone_dec_*_app_evt_cb` → `START_INIT_OK` | `tone_dec_end_ctrl()` 播放链结束 |

> ⚠️ **注意**：`AUDIO_DEC_EVENT_START` 事件在 A2DP 流式解码中**不会触发**，因此 PA 使能放在 `audio_decoder_start()` 成功后，而非 event handler 中。

---

## LED 灯效（8个RGB灯，Z字形排列）

**文件位置**: `apps/src/led_effect/led_effect.c`

### 硬件布局

8 个灯珠，4 个一排，SPI 数据走 Z 字形连线：

```
  上排 (左→右):  LED0 ─→ LED1 ─→ LED2 ─→ LED3
                                           ↓
  下排 (左→右):  LED4 ←─ LED5 ←─ LED6 ←─ LED7
                    ↑                     ↓
                    └──────── 回到0 ──────┘
```

SPI 索引 `0→1→2→3→4→5→6→7` 对应物理位置如上。粒子效果中 `led_pos[]` 将 LED 按 S 形循环路径映射到 0~255 线上（下排反向），实现粒子从左到右、从上到下、再从右到左的循环流动：

| LED | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|:---:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| 位置 | 0 | 36 | 73 | 109 | 255 | 219 | 182 | 146 |

### 驱动接口

| 接口 | 说明 |
|------|------|
| `led_spi_init()` | 初始化 SPI（波特率 8MHz） |
| `led_spi_rgb_to_24byte(r,g,b,buf,idx)` | 将第 idx 个 LED 设为 RGB 值 |
| `led_spi_send_rgbbuf_isr(buf,num)` | 中断方式发送缓冲区到 LED 灯带 |

### 灯效列表

#### ① 🔥 火焰（Fire）

8 个 LED 模拟真实火焰，从根部红色到尖端白黄色，每个 LED 独立随机 flicker。

**Z字形映射**：下排(4~7)=火焰根部(红→橙)，上排(0~3)=火焰尖端(黄→白)，物理上从底向上燃烧。

| LED范围 | 位置 | 颜色 |
|:-------:|:----:|------|
| 0~1 | 火焰根部 | 红 (R=255, G=30~85, B=0) |
| 2~3 | 火焰下部 | 橙 (R=255, G=85~140, B=0~25) |
| 4~5 | 火焰中部 | 黄 (R=255, G=140~195, B=25~75) |
| 6~7 | 火焰尖端 | 白黄 (R=255, G=195~250, B=75~125) |

**音乐联动**：低频能量（鼓点/贝斯）越大 → 火焰越旺，亮度越高。
**静音时**：幽暗红光微微飘动，不熄灭。
**平滑**：`(target - current) >> 3` 指数平滑，变化自然。

#### ② 💓 呼吸心跳（Breath）

8 个灯一起三角波呼吸（亮→暗→亮，周期≈3秒），颜色缓慢漂移。

**颜色循环**：`红 → 橙 → 黄 → 绿 → 青 → 蓝 → 紫`（每步色相+1）
**音乐联动**：低频能量骤增（鼓点）→ 瞬间闪白，然后指数衰减到当前呼吸亮度。
**平滑**：闪白通过 `breath_beat_decay *= 7/8` 指数衰减。

#### ③ 🏓 流光弹跳（Bounce）

一个光点在 8 个 LED 上来回弹跳，经过的 LED 留下渐暗拖尾。

**位置计算**：三角波 `0→1→2→3→4→5→6→7→6→5→...`
**颜色**：每 4 个弹跳周期换一种颜色（彩虹 7 色循环）
**音乐联动**：能量越大 → 弹跳越快。
**拖尾**：距离光点越远亮度越低 (`50 / dist`)。

#### ④ 🌀 能量粒子（Particles）

3 个彩色粒子（R/G/B）在 0~255 线上循环环绕运动（不反弹），每个粒子带有能量场（半径可调）。
8 个 LED 按 S 形路径映射到线上（上排左→右正序，下排右→左逆序），颜色 = 所有粒子场在该位置的叠加值。

**粒子速度**：R=1 G=2 B=3（速度不同产生追赶和分离）
**场半径**：默认 100（可运行时调整，范围 1~120）
**运动方式**：循环环绕（到达 255 后回到 0，始终同一方向）
**平滑**：`(target - current) >> 3`（每帧 1/8 平滑，制造拖尾流动感）
**音乐联动**：
- 平静 → 粒子慢速（30%），微光
- 正常 → 粒子中速，中等亮度
- 节拍 → 粒子弹开 + 闪白衰减
- 高潮 → 粒子快速（180%），高亮

### 切换方式

**自动轮播**（默认）：每 ≈30 秒自动切换到下一个效果，循环：
`①火焰 → ②呼吸 → ③流光弹跳 → ④能量粒子 → ①火焰 → ...`

**手动切换**：按键操作覆盖自动切换，并重置 30 秒计时：

| 按键 | 功能 |
|:----:|------|
| IO1 单击 | 切换到下一个效果（火焰→呼吸→弹跳→粒子→循环） |
| IO0 长按 | 切换场半径 100↔50 |
| IO1 长按 | 粒子加速 |

### 运行时参数 API

```c
void led_effect_set_field_r(int r);      // 设置场半径 1~120
void led_effect_set_speed(int idx, int speed);  // 设置粒子速度 0~20
void led_effect_show_status(void);         // 打印当前状态
```

切换时通过指数平滑自然淡入淡出。

---

## 按键系统

### IO 按键事件表

**文件位置**: `apps/soundbox/board/br25/board_ac696x_demo/key_table/iokey_table.c`

按键事件不是"按下/释放两个通用事件"，而是由**事件表**定义的。每列代表一类操作，配了什么值才发什么事件：

| 列 | 触发条件 | 例子 | KEY_NULL 时 |
|:--:|:---------|:----:|:-----------:|
| **单击** | 按下后短时间松开 | `KEY_MUSIC_PP` | 不产生事件 |
| **长按** | 按住超过 `long_time` | `KEY_CALL_HANG_UP` | 不发 |
| **hold** | 长按中持续触发（循环） | `KEY_VOL_UP` | 不发（不循环） |
| **抬起** | 松开按键 | — | 不发 |
| **双击** | 快速按两次 | `KEY_CALL_LAST_NO` | 不发 |
| **三击** | 快速按三次 | — | 不发 |

当前配置：

```c
[0] = {
    KEY_MUSIC_PP,      // 单击
    KEY_CALL_HANG_UP,  // 长按
    KEY_NULL,           // hold（不循环）
    KEY_NULL,           // 抬起（不触发）
    KEY_CALL_LAST_NO,  // 双击
    KEY_NULL            // 三击
},
[1] = {
    KEY_MUSIC_NEXT,    // 单击
    KEY_VOL_UP,        // 长按
    KEY_VOL_UP,        // hold（长按中持续发）
    KEY_NULL,           // 抬起
    KEY_NULL,           // 双击
    KEY_NULL            // 三击
},
```

所以要实现"长按→开，松开→关"，需要在**抬起列**配一个事件值：
```c
[0] = {
    KEY_MUSIC_PP,       // 单击
    KEY_MUSIC_PP,       // 长按（按下进直通）
    KEY_NULL,
    KEY_MUSIC_PREV,     // 抬起（松开关直通）
    KEY_CALL_LAST_NO,   // 双击
    KEY_NULL,
},
```

### 按键事件拦截

**文件位置**: `apps/src/key_handler/key_handler.c`

SDK 提供 `SYS_EVENT_HANDLER` 宏注册系统事件监听：

```c
#define SYS_EVENT_HANDLER(type, fn, pri) \
    const struct static_event_handler __event_handler_##fn \
    sec(.sys_event.pri.handler) = { \
        .event_type = type, \
        .handler = fn, \
    }
```

- `type`: 事件类型（`SYS_KEY_EVENT` = 0x0001）
- `fn`: 回调函数
- `pri`: 优先级（**数字越小越优先**）

当前 handler 包含完整的 switch/case 处理，可在此处添加灯效切换等自定义操作：

```c
static void my_key_handler(struct sys_event *event)
{
    u16 key = event->u.key.event;

    switch (key) {
    case KEY_MUSIC_PP:
        // 单击/长按 → 播放暂停（或切换灯效）
        led_effect_switch(1);          // 切到下一个灯效
        break;
    case KEY_MUSIC_NEXT:
        // 下一曲
        break;
    case KEY_MUSIC_PREV:
        // 上一曲
        break;
    case KEY_VOL_UP:
        // 音量+（hold持续触发）
        break;
    case KEY_MUSIC_PLAYER_START:
        // 按键抬起/释放
        break;
    case KEY_CHANGE_MODE:
        // 切换工作模式
        break;
    }
}
```

输出示例：
```
[KEY] event=134(0x86)  value=0  init=1    ← IO0 单击
[KEY] event=136(0x88)  value=0  init=1    ← IO1 单击
[KEY] event=163(0xA3)  value=0  init=1    ← IO0 双击
[KEY] event=164(0xA4)  value=0  init=1    ← IO0 长按
```

> ⚠️ 注意：所有按键事件 init 字段固定为 1（驱动在判定完手势后才发事件，不区分按下/释放）。如果需要区分，需在事件表的**抬起列**配一个独立事件值。

### 事件消耗

调用 `sys_key_event_consume()` 可阻止 SDK 其他模块处理该按键：

```c
sys_key_event_consume(&event->u.key);
```

不调用则 SDK（BT 播放/暂停等）照常响应。

---

## 调试注意事项

### DP脚日志打印

**文件位置**: `apps/soundbox/board/br25/board_ac696x_demo/board_ac696x_demo.c`

当使用 DP 脚作为日志打印引脚时，需要在电源唤醒初始化后重新初始化调试串口：

```c
printf("board_power_wakeup_init\n");
power_wakeup_init(&wk_param);
debug_uart_init(NULL);  // 重新初始化，防止DP引脚被重新配置导致无日志输出
printf("board_power_wakeup_init ok\n");
```

**原因**: `power_wakeup_init()` 可能会重新配置引脚，导致之前配置的 DP 脚日志输出功能失效。

---

## 添加用户代码

在 `apps/src/` 下按功能创建文件夹，然后在 `apps/src/Makefile` 中添加源文件路径即可：

```makefile
c_SRC_FILES += \
    apps/src/your_module/your_file.c \

INCLUDES += \
    -Iapps/src/your_module \
```

详见 `apps/src/README.md`。
