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

## 添加用户代码

在 `apps/src/` 下按功能创建文件夹，然后在 `apps/src/Makefile` 中添加源文件路径即可。

详见 `apps/src/README.md`。
