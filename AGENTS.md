# AGENTS.md - AC696x 蓝牙音箱 SDK

SDK 版本: `ac696n_soundbox_sdk_v1.7.0`，杰理 AC696x 芯片，pi32v2 专用编译工具链。

## 编译

```bash
make                    # 编译 → pre_build → 编译 → 链接 → 自动运行 download.bat/sh
make VERBOSE=1          # 显示详细编译过程
make clean              # 清除 objs/ 和 sdk.elf
make LINK_AT=0          # 旧版 make 不支持 $(file ...) 函数时使用
```

- **工具链**：`C:/JL/pi32/bin`（Windows），`/opt/jieli/pi32v2/bin`（Linux）
- **输出**：`cpu/br25/tools/sdk.elf`，`make` 后自动运行 `download.bat`/`download.sh` 下载
- **pre_build**：从 `.c` 模板（`-D__LD__ -E -P`）生成 `sdk.ld`、`download.bat/sh`、`sdk_used_list.used`、`section.txt`

## 用户代码

`apps/src/` 下添加新文件后在 `apps/src/Makefile` 注册：
```makefile
c_SRC_FILES += apps/src/新模块/新文件.c
INCLUDES += -Iapps/src/新模块
```

**不要修改** `apps/soundbox/`、`cpu/br25/`、`include_lib/` 下的 SDK 原生文件。

## 板级选择

`apps/soundbox/board/br25/board_config.h` 中取消注释对应 `CONFIG_BOARD_*` 宏。当前使用 `CONFIG_BOARD_AC696X_DEMO`，引脚和外设配置在 `board_ac696x_demo/board_ac696x_demo_cfg.h`（所有 `TCFG_*` 宏）。

## 关键文件

| 文件 | 作用 |
|------|------|
| `apps/src/user_main.c` | 用户主入口 |
| `apps/src/gpio/gpio_cfg.h` | 引脚定义（PA_EN_PORT = IO_PORTA_00） |
| `apps/src/pa/pa_drv.c` | PA 功放驱动（`PA_EN_ACTIVE_HIGH` 切换极性） |
| `apps/src/led_effect/led_effect.c` | RGB LED 灯效 |
| `apps/src/key_handler/key_handler.c` | 按键事件拦截（`SYS_EVENT_HANDLER` 优先级 3） |
| `cpu/br25/audio_dec/audio_dec_*.c` | PA 使能/关闭触发点（覆盖 BT/文件播放/FM/LINEIN/PC/SPDIF） |
| `board_ac696x_demo/board_ac696x_demo_cfg.h` | 当前板型 TCFG_* 配置 |
| `board_ac696x_demo/key_table/iokey_table.c` | 各模式 IO 按键事件表 |

## 硬件

- **PA 使能脚**：IO_PORTA_02（高电平有效，`PA_EN_ACTIVE_HIGH` 宏切换）
- **DAC 输出**：`DAC_OUTPUT_MONO_LR_DIFF`（单声道差分）
- **串口日志**：IO_PORT_DP，波特率 1000000
- **RGB LED**：8 个，SPI2 驱动（2MHz），Z字形排列（上排 0~3左→右，下排 4~7左→右）
- **IO 按键**：IO0=PA01(key_value=0)，IO1=PA03(key_value=2)

## 注意事项

- `power_wakeup_init()` 后需调用 `debug_uart_init(NULL)` 重初始化串口
- 所有解码器（BT/文件/FM/LINEIN/PC/SPDIF）中 `AUDIO_DEC_EVENT_START` **不触发**（流式解码），PA 使能放在各 `*_dec_start()` 函数的 `audio_decoder_start()` 成功后
- 按键事件优先级：数字越小越优先，调用 `sys_key_event_consume()` 阻止 SDK 处理
- 按键事件表 6 列：单击/长按/hold/抬起/双击/三击，各模式独立
- 所有按键事件 `init=1`（驱动判定完手势才发事件），如需区分按下/释放用抬起列

## 详细文档

`README.md` 包含 PA 控制、LED 灯效、按键系统的完整说明。
