# AC696x 蓝牙音箱 SDK

基于杰理 AC696x 芯片的蓝牙音箱开发项目。

## 目录结构

```
ac696x_soundbox_sdk-测试代码/
├── apps/                        # 应用层代码
│   ├── src/                     # 用户自定义代码（自己写的驱动和功能）
│   │   ├── gpio/                # GPIO配置管理
│   │   ├── pa/                  # PA功放驱动
│   │   ├── user_main.c          # 用户主入口
│   │   └── Makefile             # 用户源文件列表
│   ├── soundbox/                # SDK应用框架
│   └── common/                  # 通用应用模块
├── cpu/br25/                    # 芯片相关代码（音频、驱动等）
├── include_lib/                 # SDK库文件和头文件
├── doc/                         # 项目文档
├── Makefile                     # 主编译脚本
└── README.md                    # 本文件
```

## 硬件信息

- **芯片**: AC696x (杰理)
- **PA使能脚**: IO_PORTB_00（高电平有效）
- **DAC**: 芯片内部集成，模拟输出连接外部功放

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
| `apps/src/` | 用户自定义代码（PA驱动、GPIO配置等） |
| `cpu/br25/audio_dec/` | 音频解码模块 |
| `cpu/br25/audio_effect/` | 音频效果处理 |

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

## 添加用户代码

在 `apps/src/` 下按功能创建文件夹，然后在 `apps/src/Makefile` 中添加源文件路径即可。

详见 `apps/src/README.md`。
