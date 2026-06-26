# Makefile `include` 变量注入技巧

## 概述

一种零侵入的 Makefile 扩展模式：主 Makefile 定义构建变量（源文件列表、头文件路径、宏定义等），子 Makefile 用 `+=` 追加内容，主 Makefile 通过 `include` 指令拉入子 Makefile。用户只需维护自己的 Makefile 片段，无需修改主构建文件。

## 架构图

```
主 Makefile                         子 Makefile（用户/模块）
┌─────────────────────────┐         ┌──────────────────────┐
│ c_SRC_FILES := foo.c    │  include│ c_SRC_FILES += my.c  │
│ INCLUDES := -Icore      │◄────────│ INCLUDES += -Imy     │
│ include user/Makefile   │         └──────────────────────┘
│ $(OBJS): $(c_SRC_FILES) │
└─────────────────────────┘
         │
         ▼
    编译一体化（共用编译规则、优化参数、依赖跟踪）
```

## 核心机制

| 要素 | 说明 |
|------|------|
| **变量定义** | 主 Makefile 用 `=` 或 `:=` 定义初始值 |
| **变量追加** | 子 Makefile 用 `+=` 追加内容 |
| **include 拉入** | 主 Makefile 用 `include path/to/sub.mk` 引入子文件 |
| **统一消费** | 主 Makefile 继续使用合并后的变量进行编译/链接 |

## 标准模板

### 主 Makefile

```makefile
# 工具链设置
CC := gcc
CFLAGS := -Wall -O2

# 源文件列表（初始值）
c_SRC_FILES := main.c core.c
INCLUDES := -Icore
DEFINES :=

# 引入用户扩展（目录名可自定义，如 user/、apps/src/ 等）
include apps/src/Makefile

# 编译规则（消费合并后的变量）
OBJS := $(c_SRC_FILES:%.c=%.o)
$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

all: $(OBJS)
	$(CC) -o output.elf $(OBJS)
```

### 用户 Makefile（`apps/src/Makefile`）

```makefile
# 只追加，不覆盖
c_SRC_FILES += apps/src/my_module.c \
               apps/src/another.c

INCLUDES += -Iapps/src \
            -Iapps/src/my_module

DEFINES += -DMY_MODULE_ENABLE
```

## 多种变量类型支持

```makefile
# 源文件
c_SRC_FILES += path/to/file.c
S_SRC_FILES += path/to/asm.S
cpp_SRC_FILES += path/to/file.cpp

# 头文件路径
INCLUDES += -Ipath/to/headers

# 宏定义
DEFINES += -DMY_FEATURE_ENABLE=1

# 链接库
LIBS += -lmylib
LIBPATHS += -Lpath/to/lib

# 链接参数
LFLAGS += --extra-link-option
```

## 优点

| 特性 | 说明 |
|------|------|
| **零侵入** | 不改主 Makefile，只追加文件 |
| **增量扩展** | 每个模块独立注册，互不干扰 |
| **编译一体化** | 用户代码与核心代码共用同一套编译规则和优化参数 |
| **依赖自动跟踪** | 主 Makefile 的 `.d` 依赖生成规则自动覆盖用户文件 |
| **合并友好** | 多人各自维护自己的 Makefile 片段，不碰主文件，无冲突 |

## 适用场景

- **SDK 项目**：分离用户代码与 SDK 原生代码（如杰理 AC696x SDK 的 `apps/src/Makefile`）
- **插件式架构**：每个插件一个 Makefile 片段，主工程 `include` 所有插件
- **多人协作**：每人维护自己的模块 Makefile，主 Makefile 只 `include` 所有人
- **条件编译**：子 Makefile 中可用 `ifdef` 条件追加，实现可选模块

## 注意事项

1. **include 位置**：必须在变量使用**之前** `include`，否则追加不生效
2. **变量赋值方式**：子 Makefile 必须用 `+=`（追加），不能用 `=` 或 `:=`（会覆盖）
3. **路径问题**：子 Makefile 中的路径建议用**相对于主 Makefile** 的路径，或使用 `$(dir $(lastword $(MAKEFILE_LIST)))` 获取自身目录
4. **多次 include**：同一个文件多次 `include` 会导致变量重复追加，可用 `-include` 或守卫变量防止

## 实战示例：杰理 AC696x SDK

### 目录结构

```
project/
├── Makefile              # 主 Makefile（定义编译规则、SDK 源文件）
├── apps/
│   └── src/
│       ├── Makefile      # 用户 Makefile（只追加用户文件）
│       ├── user_main.c
│       ├── gpio/
│       │   ├── gpio_cfg.c
│       │   └── gpio_cfg.h
│       ├── pa/
│       │   ├── pa_drv.c
│       │   └── pa_drv.h
│       └── key_handler/
│           ├── key_handler.c
│           └── key_handler.h
```

### 主 Makefile（末尾）

```makefile
include apps/src/Makefile
```

### `apps/src/Makefile`

```makefile
c_SRC_FILES += \
    apps/src/user_main.c \
    apps/src/gpio/gpio_cfg.c \
    apps/src/pa/pa_drv.c \
    apps/src/key_handler/key_handler.c \

INCLUDES += \
    -Iapps/src \
    -Iapps/src/gpio \
    -Iapps/src/pa \
    -Iapps/src/key_handler \
```

## 复刻步骤

要在新项目中复刻此框架：

1. 在主 Makefile 中定义好 `c_SRC_FILES`、`INCLUDES` 等变量
2. 创建用户代码目录（如 `apps/src/`）
3. 在该目录下创建 `Makefile`，用 `+=` 追加用户文件
4. 在主 Makefile 末尾添加 `include apps/src/Makefile`
5. 用户新增模块时，只需在 `apps/src/Makefile` 中追加一行
