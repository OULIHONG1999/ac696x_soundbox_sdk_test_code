# 用户自定义代码

本目录存放用户自己编写的驱动和功能代码，与 SDK 原生代码分离，方便管理和移植。

## 文件结构

```
apps/src/
├── gpio/                # GPIO配置管理
│   ├── gpio_cfg.h       # 引脚定义、宏定义
│   └── gpio_cfg.c       # GPIO初始化
├── pa/                  # PA功放驱动
│   ├── pa_drv.h         # 驱动接口声明
│   └── pa_drv.c         # 驱动实现
├── user_main.c          # 用户主入口函数
└── Makefile             # 源文件列表（在此添加新文件）
```

## 模块说明

### gpio 模块
统一管理所有用户GPIO引脚配置，包括引脚定义、方向设置、初始化。

### pa 模块
PA功放芯片使能控制，在音频播放开始/结束时自动开关功放。

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

### 3. 在SDK中调用

在需要调用的地方使用 extern 声明：

```c
extern void module_func(void);
module_func();
```

## 注意事项

- 头文件引用使用相对路径，如 `#include "../gpio/gpio_cfg.h"`
- SDK代码尽量只添加 extern 声明和函数调用，保持最小侵入
