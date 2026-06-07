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
- **PA使能脚**: IO_PORTA_00（高电平有效，可通过宏切换为低电平有效）
- **DAC**: 芯片内部集成，模拟输出连接外部功放
  - 当前输出模式：**双声道差分**（`DAC_OUTPUT_DUAL_LR_DIFF`）
  - 信号：L+/L-、R+/R- 两对差分线
- **串口日志**: IO_PORT_DP（UART0 TX，波特率 1000000）
- **RGB LED**: 4个（通过 SPI 驱动，连接到 JL_SPI2）

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
#define PA_EN_PORT    IO_PORTA_00
```

### 使能电平极性

`apps/src/pa/pa_drv.c` 使用宏 `PA_EN_ACTIVE_HIGH` 控制极性：

```c
#define PA_EN_ACTIVE_HIGH           // 定义：高电平使能，低电平关闭（默认）
// #define PA_EN_ACTIVE_HIGH        // 注释掉：低电平使能，高电平关闭
```

### PA 触发逻辑

PA 使能/关闭由 `cpu/br25/audio_dec/audio_dec_bt.c` 中的 A2DP 解码状态控制：

| 时机 | 调用 | 位置 |
|------|------|------|
| A2DP 解码器启动成功 | `pa_enable()` | `a2dp_dec_start()` → `audio_decoder_start()` 成功后 |
| A2DP 解码器资源关闭 | `pa_disable()` | `a2dp_audio_res_close()` 末尾 |

> ⚠️ **注意**：`AUDIO_DEC_EVENT_START` 事件在 A2DP 流式解码中**不会触发**（此事件仅用于文件播放解码器），因此 `pa_enable()` 不能放在 decoder event handler 中。

---

## LED 灯效（4个RGB灯）

**文件位置**: `apps/src/led_effect/led_effect.c`

### 驱动接口

| 接口 | 说明 |
|------|------|
| `led_spi_init()` | 初始化 SPI（波特率 8MHz） |
| `led_spi_rgb_to_24byte(r,g,b,buf,idx)` | 将第 idx 个 LED 设为 RGB 值 |
| `led_spi_send_rgbbuf_isr(buf,num)` | 中断方式发送缓冲区到 LED 灯带 |

### 灯效列表

#### ① 🔥 火焰（Fire）

4 个 LED 模拟真实火焰，从根部红色到尖端白黄色，每个 LED 独立随机 flicker。

| LED | 位置 | 颜色 |
|:---:|:----:|------|
| 0 | 火焰根部 | 红 (R=255, G=20~60, B=0) |
| 1 | 火焰下部 | 橙 (R=255, G=60~120, B=0) |
| 2 | 火焰中部 | 黄 (R=255, G=130~180, B=5~20) |
| 3 | 火焰尖端 | 白黄 (R=255, G=190~230, B=40~80) |

**音乐联动**：低频能量（鼓点/贝斯）越大 → 火焰越旺，亮度越高。
**静音时**：幽暗红光微微飘动，不熄灭。
**平滑**：`(target - current) >> 2` 指数平滑，变化自然。

#### ② 💓 呼吸心跳（Breath）

4 个灯一起三角波呼吸（亮→暗→亮，周期≈3秒），颜色缓慢漂移。

**颜色循环**：`红 → 橙 → 黄 → 绿 → 青 → 蓝 → 紫`（每步色相+1）
**音乐联动**：低频能量骤增（鼓点）→ 瞬间闪白，然后指数衰减到当前呼吸亮度。
**平滑**：闪白通过 `breath_beat_decay *= 7/8` 指数衰减。

#### ③ 🏓 流光弹跳（Bounce）

一个光点在 4 个 LED 上来回弹跳，经过的 LED 留下渐暗拖尾。

**位置计算**：三角波 `0→1→2→3→2→1→0→...`
**颜色**：每 4 个弹跳周期换一种颜色（彩虹 7 色循环）
**音乐联动**：能量越大 → 弹跳越快。
**拖尾**：距离光点越远亮度越低 (`50 / dist`)。

#### ④ 🌀 能量粒子（Particles）

3 个彩色粒子（R/G/B）在 0~255 线上来回弹跳，每个粒子带有能量场（半径可调）。
4 个 LED 固定在线上（位置 32/96/160/224），颜色 = 所有粒子场在该位置的叠加值。

**粒子速度**：R=1 G=2 B=3（速度不同产生追赶和分离）
**场半径**：默认 30（可运行时调整，范围 1~120）
**音乐联动**：
- 平静 → 粒子慢速（30%），微光
- 正常 → 粒子中速，中等亮度
- 节拍 → 粒子弹开 + 闪白衰减
- 高潮 → 粒子快速（180%），高亮

### 切换方式

通过 **按键** 手动切换效果，不再自动轮播：

| 按键 | 功能 |
|:----:|------|
| IO1 单击 | 切换到下一个效果（火焰→呼吸→弹跳→粒子→循环） |
| IO0 长按 | 切换场半径 30↔60 |
| IO1 长按 | 粒子加速 |

### 运行时参数 API

```c
void led_effect_set_field_r(int r);      // 设置场半径 1~120
void led_effect_set_speed(int idx, int speed);  // 设置粒子速度 0~20
void led_effect_show_status(void);         // 打印当前状态
```

切换时通过指数平滑自然淡入淡出。

---

## 按键事件拦截

**文件位置**: `apps/src/key_handler/key_handler.c`

### 原理

SDK 提供 `SYS_EVENT_HANDLER` 宏，可以在任意 .c 文件中注册系统事件监听：

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

### 拦截流程

```
按键按下
  ↓
IO 驱动 → 原始值 134 → 映射为 KEY_MUSIC_PP
  ↓
sys_event_notify() 广播给所有 handler（按优先级排序）
  ├─ 你的 handler (prio=3) → 先拿到事件 → 调 sys_key_event_consume()
  └─ SDK handler (prio=4) → 发现事件已消耗 → 跳过
```

### 当前调试菜单

| 按键 | 动作 | 功能 |
|:----:|:----:|------|
| **IO0 单击** | KEY_MUSIC_PP | 打印 LED 粒子状态 + 按键帮助 |
| **IO0 长按** | KEY_CALL_HANG_UP | 切换场半径 30↔60 |
| **IO0 双击** | KEY_CALL_LAST_NO | 所有粒子速度设为 5 |
| **IO1 单击** | KEY_MUSIC_NEXT | 切换到下一个 LED 效果 |
| **IO1 长按** | KEY_VOL_UP | 所有粒子速度 +1 |

### 添加自定义按键处理

在 `apps/src/key_handler/key_handler.c` 的 `my_key_handler()` 中加 case：

```c
case KEY_MUSIC_NEXT:
    // 自己的逻辑（例如切换 LED 效果）
    led_effect_switch(1);
    break;
```

### 注意事项

- `sys_key_event_consume(&event->u.key)` 调用后，SDK 模块（BT 播放/暂停等）**不会收到此按键事件**
- 如果希望某个按键同时触发自定义操作和 SDK 默认行为，**不要调用** `sys_key_event_consume`
- 该 API 在 `include_lib/system/event.h` 中声明：

```c
void sys_key_event_consume(struct key_event *e);
```

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
