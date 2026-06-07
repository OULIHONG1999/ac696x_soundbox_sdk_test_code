# 用户自定义代码

本目录存放用户自己编写的驱动和功能代码，与 SDK 原生代码分离，方便管理和迁移。

## 文件结构

```
apps/src/
├── gpio/                # GPIO配置管理
│   ├── gpio_cfg.h       # 引脚定义、宏定义
│   └── gpio_cfg.c       # GPIO初始化
├── pa/                  # PA功放驱动
│   ├── pa_drv.h         # 驱动接口声明
│   └── pa_drv.c         # 驱动实现
├── led_effect/          # LED RGB 灯效控制
│   ├── led_effect.h     # 灯效接口声明
│   └── led_effect.c     # 4种灯效实现 + 按键切换
├── key_handler/         # 按键事件拦截
│   ├── key_handler.h    # 初始化声明
│   └── key_handler.c    # 按键拦截 + 调试菜单
├── user_main.c          # 用户主入口函数
└── Makefile             # 源文件列表（在此添加新文件）
```

## 模块说明

### gpio 模块
统一管理所有用户 GPIO 引脚配置，包括引脚定义、方向设置、初始化。

### pa 模块
PA 功放芯片使能控制，在音频播放开始/结束时自动开关功放。
支持通过 `PA_EN_ACTIVE_HIGH` 宏切换使能电平极性。

### led_effect 模块
4 个 RGB LED 灯效控制（通过 SPI 驱动）：

| 效果 | 说明 |
|:----:|------|
| 火焰 (Fire) | 4 个 LED 模拟真实火焰 flicker，低频驱动亮度 |
| 呼吸心跳 (Breath) | 三角波呼吸 + 颜色漂移，节拍触发闪白 |
| 流光弹跳 (Bounce) | 光点 4 灯来回弹跳 + 渐暗拖尾 |
| 能量粒子 (Particles) | 3 个 RGB 粒子在线 0~255 运动，场叠加混合颜色 |

可通过按键切换效果，运行时调整场半径和粒子速度。

### key_handler 模块
通过 `SYS_EVENT_HANDLER` 宏拦截按键事件，优先级 3（高于 SDK 的 4），
调用 `sys_key_event_consume()` 阻止 SDK 处理。

调试菜单：

| 按键 | 功能 |
|:----:|------|
| IO0 单击 | 打印 LED 状态 + 调试菜单 |
| IO0 长按 | 切换场半径 30↔60 |
| IO0 双击 | 所有粒子速度设为 5 |
| IO1 单击 | 切换到下一个 LED 效果 |
| IO1 长按 | 所有粒子速度 +1 |

## 添加新模块

### 1. 创建文件夹和文件

```
apps/src/新模块/
├── module.h             # 头文件
└── module.c             # 实现
```

### 2. 修改 Makefile

在 `apps/src/Makefile` 中添加：

```makefile
c_SRC_FILES += \
    apps/src/新模块/module.c \

INCLUDES += \
    -Iapps/src/新模块 \
```

### 3. 在 SDK 中调用

在需要调用的地方使用 extern 声明：

```c
extern void module_func(void);
module_func();
```

## 注意事项

- 头文件引用使用相对路径，如 `#include "../gpio/gpio_cfg.h"`
- SDK 代码尽量只添加 extern 声明和函数调用，保持最小侵入
- 所有自定义代码放在 `apps/src/` 下，不要修改 SDK 原生文件
