# 02 - 项目结构

> **本章目标**：深入了解 STM32F103C8T6 模板项目的目录结构，理解每个文件的作用，掌握 CMake 构建系统的工作原理。

---

## 📋 目录

- [项目目录概览](#项目目录概览)
- [核心文件详解](#核心文件详解)
  - [main.c - 主程序入口](#mainc---主程序入口)
  - [system.c - 系统时钟配置](#systemc---系统时钟配置)
  - [CMakeLists.txt - 构建配置](#cmakeliststxt---构建配置)
  - [STM32F103C8TX_FLASH.ld - 链接脚本](#stm32f103c8tx_flashld---链接脚本)
  - [stm32f1xx_hal_conf.h - HAL 配置](#stm32f1xx_hal_confh---hal-配置)
  - [.vscode/launch.json - 调试配置](#vscodelaunchjson---调试配置)
- [构建输出目录](#构建输出目录)
- [第三方依赖](#第三方依赖)
- [CMake 构建系统详解](#cmake-构建系统详解)
- [如何基于模板创建新项目](#如何基于模板创建新项目)
- [常见问题](#常见问题)

---

## 项目目录概览

### 目录树结构

模板项目位于 `project/0_template/` 目录，结构如下：

```
project/0_template/
├── main.c                      # 主程序入口（LED 闪烁示例）
├── system.c                    # 系统时钟配置（64MHz）
├── CMakeLists.txt              # CMake 构建配置文件
├── STM32F103C8TX_FLASH.ld      # 链接脚本（内存布局）
├── stm32f1xx_hal_conf.h        # HAL 库配置头文件
├── README.md                   # 项目说明文档
├── .vscode/
│   └── launch.json             # VSCode 调试配置
└── build/                      # 构建输出目录（自动生成）
    ├── STM32F1.elf             # ELF 格式可执行文件
    ├── STM32F1.bin             # 二进制固件文件
    ├── STM32F1.map             # 链接映射文件
    └── ...                     # 其他构建产物
```

### 文件分类

| 类别 | 文件 | 说明 |
|------|------|------|
| **源代码** | main.c, system.c | 用户编写的应用程序代码 |
| **构建配置** | CMakeLists.txt | 定义编译规则和目标 |
| **链接配置** | STM32F103C8TX_FLASH.ld | 定义内存布局 |
| **HAL 配置** | stm32f1xx_hal_conf.h | 启用/禁用 HAL 模块 |
| **调试配置** | .vscode/launch.json | VSCode 调试器配置 |

---

## 核心文件详解

### main.c - 主程序入口

`main.c` 是整个程序的入口点，包含 `main()` 函数和主要的应用逻辑。

#### 文件结构

```c
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"

void SystemClock_Config(void);

int main() {
  HAL_Init();
  SystemClock_Config();

  GPIO_InitTypeDef def;
  def.Mode = GPIO_MODE_OUTPUT_PP;
  def.Pin = GPIO_PIN_13;
  def.Speed = GPIO_SPEED_FREQ_LOW;
  def.Pull = GPIO_PULLDOWN;
  __HAL_RCC_GPIOC_CLK_ENABLE();
  HAL_GPIO_Init(GPIOC, &def);

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  while (1) {
    HAL_Delay(1000);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }
}
```

#### 代码解析

| 代码段 | 说明 |
|--------|------|
| `#include "stm32f1xx_hal.h"` | 包含 HAL 库主头文件 |
| `HAL_Init()` | 初始化 HAL 库，配置 SysTick |
| `SystemClock_Config()` | 配置系统时钟（定义在 system.c） |
| `GPIO_InitTypeDef def` | 定义 GPIO 初始化结构体 |
| `def.Mode = GPIO_MODE_OUTPUT_PP` | 设置为推挽输出模式 |
| `def.Pin = GPIO_PIN_13` | 选择 PC13 引脚（板载 LED） |
| `__HAL_RCC_GPIOC_CLK_ENABLE()` | 使能 GPIOC 时钟 |
| `HAL_GPIO_Init(GPIOC, &def)` | 初始化 GPIO |
| `HAL_GPIO_TogglePin()` | 翻转 LED 状态 |

#### 为什么是 PC13？

Blue Pill 开发板上有一个板载 LED 连接在 PC13 引脚。这个 LED 采用**共阳极接法**，即：
- LED 正极连接 VCC（3.3V）
- LED 负极通过限流电阻连接 PC13
- **低电平时 LED 点亮**，高电平时 LED 熄灭

---

### system.c - 系统时钟配置

`system.c` 负责配置 STM32 的系统时钟，将内部振荡器（HSI）通过 PLL 倍频到 64MHz。

#### 文件内容

```c
#include "stm32f1xx_hal.h"

void SystemClock_Config(void) {
  RCC_OscInitTypeDef osc = {0};
  osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  osc.HSIState = RCC_HSI_ON;
  osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2; /* 4MHz  */
  osc.PLL.PLLMUL = RCC_PLL_MUL16;             /* 64MHz */
  HAL_RCC_OscConfig(&osc);

  RCC_ClkInitTypeDef clk = {0};
  clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk.AHBCLKDivider = RCC_SYSCLK_DIV1; /* HCLK  = 64MHz */
  clk.APB1CLKDivider = RCC_HCLK_DIV2;  /* APB1  = 32MHz */
  clk.APB2CLKDivider = RCC_HCLK_DIV1;  /* APB2  = 64MHz */
  HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);
}

void _init() {}

void SysTick_Handler(void) { HAL_IncTick(); }
```

#### 时钟配置详解

**时钟计算过程**：

```
HSI (内部高速振荡器) = 8 MHz
        ↓
PLL 输入 = HSI / 2 = 4 MHz
        ↓
PLL 倍频 = 4 MHz × 16 = 64 MHz
        ↓
SYSCLK (系统时钟) = 64 MHz
```

**总线时钟分配**：

| 总线 | 分频系数 | 最终频率 | 连接的外设 |
|------|----------|----------|------------|
| AHB (HCLK) | /1 | 64 MHz | GPIO, DMA, Flash |
| APB1 (PCLK1) | /2 | 32 MHz | USART2/3, TIM2-7, I2C |
| APB2 (PCLK2) | /1 | 64 MHz | USART1, SPI1, ADC, TIM1 |

> **⚠️ 注意**：APB1 总线最高只能运行在 36 MHz，所以必须 2 分频。

#### 辅助函数

| 函数 | 说明 |
|------|------|
| `void _init()` | 空函数，满足链接器要求（C++ 构造函数调用） |
| `void SysTick_Handler()` | SysTick 中断服务程序，更新 HAL 滴答计数 |

---

### CMakeLists.txt - 构建配置

`CMakeLists.txt` 是 CMake 构建系统的配置文件，定义了如何编译、链接和生成最终固件。

#### 文件结构解析

```cmake
cmake_minimum_required(VERSION 3.16)

# 1. 系统配置
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# 2. 工具链设置
set(CMAKE_C_COMPILER       arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER     arm-none-eabi-gcc)
set(CMAKE_OBJCOPY          arm-none-eabi-objcopy)
set(CMAKE_SIZE             arm-none-eabi-size)

# 3. 项目定义
project(STM32F1 
        VERSION 0.0.1
        LANGUAGES C ASM)

# 4. 编译选项
add_compile_options(
    -mcpu=cortex-m3
    -mthumb
    -O2
    -Wall
    -Wextra
    ...
)

# 5. 源文件和目标
add_executable(${PROJECT_NAME}.elf 
    main.c
    system.c
    ${CMSIS_TMPL}/system_stm32f1xx.c
    ${CMSIS_TMPL}/gcc/startup_stm32f103xb.s
    ${HAL_SRC})

# 6. 链接选项
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -T${CMAKE_SOURCE_DIR}/STM32F103C8TX_FLASH.ld
    ...
)

# 7. 后处理命令
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ...)

# 8. 自定义目标
add_custom_target(flash ...)
add_custom_target(erase ...)
```

#### 关键配置说明

| 配置项 | 说明 |
|--------|------|
| `CMAKE_SYSTEM_NAME Generic` | 告诉 CMake 这是嵌入式项目，无操作系统 |
| `arm-none-eabi-gcc` | ARM 交叉编译工具链 |
| `-mcpu=cortex-m3` | 指定目标处理器为 Cortex-M3 |
| `-mthumb` | 使用 Thumb 指令集（代码密度更高） |
| `-O2` | 优化级别 2（平衡速度和大小） |
| `-DUSE_HAL_DRIVER` | 启用 HAL 库 |
| `-DSTM32F103xB` | 指定芯片型号 |

#### 构建目标

| 目标 | 命令 | 说明 |
|------|------|------|
| 默认目标 | `make` | 编译生成 .elf 和 .bin 文件 |
| flash | `make flash` | 通过 OpenOCD 烧录固件 |
| erase | `make erase` | 擦除 Flash 存储器 |

---

### STM32F103C8TX_FLASH.ld - 链接脚本

链接脚本（Linker Script）定义了程序的内存布局，告诉链接器代码和数据应该放在哪里。

#### 文件内容

```ld
/* STM32F103C8T6 链接脚本
 *   Flash : 64KB @ 0x08000000
 *   SRAM  : 20KB @ 0x20000000
 */

ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);   /* 栈顶 = SRAM 末尾 */

MEMORY
{
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
}

SECTIONS
{
  /* 中断向量表，必须放最前面 */
  .isr_vector :
  {
    KEEP(*(.isr_vector))
  } >FLASH

  /* 代码 + 只读数据 */
  .text :
  {
    *(.text*)
    *(.rodata*)
    _etext = .;
  } >FLASH

  /* 初始化数据：Flash 存储，运行时复制到 SRAM */
  _sidata = LOADADDR(.data);
  .data :
  {
    _sdata = .;
    *(.data*)
    _edata = .;
  } >RAM AT >FLASH

  /* 未初始化数据：SRAM，启动文件负责清零 */
  .bss :
  {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } >RAM
}
```

#### 内存布局说明

| 区域 | 起始地址 | 大小 | 用途 |
|------|----------|------|------|
| FLASH | 0x08000000 | 64 KB | 存储程序代码和常量 |
| RAM | 0x20000000 | 20 KB | 存储运行时数据 |

#### 关键符号

| 符号 | 说明 |
|------|------|
| `ENTRY(Reset_Handler)` | 程序入口点 |
| `_estack` | 栈顶地址（SRAM 末尾） |
| `_sdata`, `_edata` | 初始化数据段的起止地址 |
| `_sbss`, `_ebss` | 未初始化数据段的起止地址 |

---

### stm32f1xx_hal_conf.h - HAL 配置

这个头文件用于配置 HAL 库，可以启用或禁用不同的外设模块，减少编译后的代码体积。

#### 模块启用配置

```c
/* ########################## Module Selection ############################## */
#define HAL_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_CAN_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_CRC_MODULE_ENABLED
#define HAL_DAC_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
// ... 更多模块
```

#### 如何禁用未使用的模块？

如果项目不需要某个外设，可以注释掉对应的宏定义：

```c
// #define HAL_CAN_MODULE_ENABLED    // 禁用 CAN 模块
// #define HAL_DAC_MODULE_ENABLED    // 禁用 DAC 模块
```

**好处**：
- 减少编译后的代码体积
- 减少 RAM 占用
- 加快编译速度

#### 振荡器配置

```c
#define HSE_VALUE 8000000U    // 外部晶振频率（8 MHz）
#define HSI_VALUE 8000000U    // 内部振荡器频率（8 MHz）
#define LSI_VALUE 40000U     // 内部低速振荡器（40 kHz）
#define LSE_VALUE 32768U     // 外部低速晶振（32.768 kHz）
```

---

### .vscode/launch.json - 调试配置

VSCode 的调试配置文件，用于配置 Cortex-Debug 扩展与 OpenOCD 的集成。

#### 文件内容

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "STM32 Debug",
            "type": "cortex-debug",
            "request": "launch",
            "servertype": "openocd",
            "cwd": "${workspaceRoot}",
            "executable": "build/STM32F1.elf",
            "configFiles": [
                "interface/stlink.cfg",
                "target/stm32f1x.cfg"
            ],
            "runToEntryPoint": "main",
            "svdFile": ""
        }
    ]
}
```

#### 配置项说明

| 配置项 | 说明 |
|--------|------|
| `type: "cortex-debug"` | 使用 Cortex-Debug 扩展 |
| `servertype: "openocd"` | 使用 OpenOCD 作为调试服务器 |
| `executable` | 指向编译生成的 .elf 文件 |
| `configFiles` | OpenOCD 配置文件 |
| `runToEntryPoint: "main"` | 启动调试时自动运行到 main 函数 |

#### 使用方法

1. 连接 ST-Link 调试器
2. 编译项目：`make`
3. 在 VSCode 中按 `F5` 启动调试

> **Windows 注意**：确保 ARM GCC 和 OpenOCD 的 `bin` 目录已加入系统 PATH，Cortex-Debug 才能找到 `arm-none-eabi-gdb` 和 `openocd`。如果 PATH 方式不生效，可在 VSCode 的 `settings.json` 中手动配置 `cortex-debug.armToolchainPath` 和 `cortex-debug.openOCDPath`。详见 [01_environment_setup](./01_environment_setup)。

---

## 构建输出目录

### build/ 目录结构

执行 `cmake -G "MinGW Makefiles" .. && make` 后，`build/` 目录会生成以下文件：

```
build/
├── STM32F1.elf              # ELF 格式可执行文件（包含调试信息）
├── STM32F1.bin              # 二进制固件文件（用于烧录）
├── STM32F1.map              # 链接映射文件（内存使用情况）
├── CMakeCache.txt           # CMake 缓存文件
├── CMakeFiles/              # CMake 内部文件
├── Makefile                 # 生成的 Makefile
└── ...                      # 其他构建产物
```

### 文件说明

| 文件 | 格式 | 用途 |
|------|------|------|
| `STM32F1.elf` | ELF | 包含调试信息，用于 GDB 调试 |
| `STM32F1.bin` | Binary | 纯二进制固件，用于烧录 |
| `STM32F1.map` | Text | 链接映射，查看函数/变量地址 |

### 如何查看内存使用情况？

```cmd
:: 方法1：使用 arm-none-eabi-size
arm-none-eabi-size build/STM32F1.elf

:: 输出示例：
::    text    data     bss     dec     hex filename
::    1234     128      64    1426     592 STM32F1.elf

:: 方法2：查看 .map 文件
type build\STM32F1.map | more
```

**内存区域说明**：

| 区域 | 说明 |
|------|------|
| text | 代码段（存储在 Flash） |
| data | 初始化数据段（Flash 存储，运行时复制到 RAM） |
| bss | 未初始化数据段（RAM） |

---

## 第三方依赖

### STM32CubeF1 HAL 库

项目使用 ST 官方的 STM32CubeF1 HAL 库，位于 `third_party/STM32CubeF1/` 目录。

#### 目录结构

```
third_party/STM32CubeF1/
├── Drivers/
│   ├── CMSIS/                    # CMSIS 标准接口
│   │   ├── Include/              # CMSIS 核心头文件
│   │   └── Device/
│   │       └── ST/STM32F1xx/
│   │           ├── Include/      # 设备头文件
│   │           └── Source/
│   │               └── Templates/
│   │                   ├── system_stm32f1xx.c    # 系统初始化
│   │                   └── gcc/
│   │                       └── startup_stm32f103xb.s  # 启动文件
│   └── STM32F1xx_HAL_Driver/     # HAL 驱动库
│       ├── Inc/                  # HAL 头文件
│       └── Src/                  # HAL 源文件
├── Documentation/                # 文档
└── LICENSE.md                    # 许可证
```

#### CMake 如何包含 HAL 库？

在 `CMakeLists.txt` 中：

```cmake
# 设置 HAL 库路径
set(STM32F1_ROOT ${CMAKE_SOURCE_DIR}/../../third_party/STM32CubeF1/Drivers)

# 收集所有 HAL 源文件
file(GLOB HAL_SRC
    ${STM32F1_ROOT}/STM32F1xx_HAL_Driver/Src/*.c
)

# 排除模板文件
list(FILTER HAL_SRC EXCLUDE REGEX ".*_template\\.c$")

# 添加头文件路径
target_include_directories(${PROJECT_NAME}.elf PRIVATE
    ${CMAKE_SOURCE_DIR}                                          # stm32f1xx_hal_conf.h
    ${STM32F1_ROOT}/CMSIS/Include
    ${STM32F1_ROOT}/CMSIS/Device/ST/STM32F1xx/Include
    ${STM32F1_ROOT}/STM32F1xx_HAL_Driver/Inc
)
```

#### HAL 库的优势

| 优势 | 说明 |
|------|------|
| 可移植性 | 同一 API 可用于不同 STM32 系列 |
| 易用性 | 无需了解寄存器细节 |
| 安全性 | 内置参数检查 |
| 文档完善 | ST 官方提供详细文档 |

---

## CMake 构建系统详解

### 构建流程

```
┌─────────────────────────────────────────────────────────────┐
│                    CMake 构建流程                            │
├─────────────────────────────────────────────────────────────┤
│  1. 配置阶段 (cmake -G "MinGW Makefiles" ..)                 │
│     ├── 检测编译器 (arm-none-eabi-gcc)                      │
│     ├── 解析 CMakeLists.txt                                 │
│     ├── 生成 Makefile                                       │
│     └── 生成 compile_commands.json                          │
│                                                             │
│  2. 编译阶段 (make)                                          │
│     ├── 预处理 (.c → .i)                                    │
│     ├── 编译 (.i → .s → .o)                                 │
│     └── 链接 (.o → .elf)                                    │
│                                                             │
│  3. 后处理阶段                                              │
│     ├── objcopy (.elf → .bin)                               │
│     └── size (显示内存使用)                                 │
└─────────────────────────────────────────────────────────────┘
```

### 完整构建命令

```cmd
:: 进入项目目录
cd project\0_template

:: 创建构建目录
mkdir build
cd build

:: 配置项目（Windows 必须指定生成器）
cmake -G "MinGW Makefiles" ..

:: 编译
make

:: 烧录（需要连接 ST-Link）
make flash
```

> **重要**：Windows 上必须指定 `-G "MinGW Makefiles"` 生成器。省略此参数会导致 CMake 使用 Visual Studio 生成器，编译将失败。

### 增量编译

CMake 支持增量编译，只重新编译修改过的文件：

```cmd
:: 修改 main.c 后
make        :: 只重新编译 main.c 和链接步骤

:: 清理所有构建产物
make clean

:: 完全重新构建
rmdir /s /q build
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
make
```

---

## 如何基于模板创建新项目

### 方法一：复制模板目录

**使用命令行（Git Bash / PowerShell）：**

```bash
# 1. 复制模板目录
cp -r project/0_template project/my_project

# 2. 进入新项目
cd project/my_project

# 3. 修改项目名称（可选）
# 编辑 CMakeLists.txt，修改 project(STM32F1 ...) 为 project(MyProject ...)

# 4. 清理构建产物
rm -rf build

# 5. 构建新项目
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
make
```

**使用 Windows 原生命令（CMD）：**

```cmd
:: 1. 复制模板目录
xcopy /E /I project\0_template project\my_project

:: 2. 进入新项目
cd project\my_project

:: 3. 清理构建产物
rmdir /s /q build

:: 4. 构建新项目
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
make
```

### 方法二：创建新项目清单

创建新项目时需要复制的文件：

| 文件 | 必须 | 说明 |
|------|------|------|
| `main.c` | ✅ | 主程序（可修改） |
| `system.c` | ✅ | 系统时钟配置（通常不需要修改） |
| `CMakeLists.txt` | ✅ | 构建配置 |
| `STM32F103C8TX_FLASH.ld` | ✅ | 链接脚本 |
| `stm32f1xx_hal_conf.h` | ✅ | HAL 配置 |
| `.vscode/launch.json` | ⚪ | 调试配置（可选） |

### 添加新源文件

如果需要添加新的源文件（如 `uart.c`）：

```cmake
# 在 CMakeLists.txt 中修改 add_executable
add_executable(${PROJECT_NAME}.elf 
    main.c
    system.c
    uart.c              # 添加新文件
    ${CMSIS_TMPL}/system_stm32f1xx.c
    ${CMSIS_TMPL}/gcc/startup_stm32f103xb.s
    ${HAL_SRC})
```

### 修改芯片型号

如果使用不同的 STM32F1 芯片：

1. **修改链接脚本**：更新 Flash 和 RAM 大小
2. **修改 CMakeLists.txt**：更新 `-DSTM32F103xB` 宏定义
3. **修改启动文件**：使用对应的 `startup_stm32f1xx.s`

---

## 常见问题

### Q1: 编译时找不到头文件？

**错误信息**：
```
fatal error: stm32f1xx_hal.h: No such file or directory
```

**解决方案**：
```cmd
:: 确保已克隆 third_party 子模块
git submodule update --init --recursive

:: 检查 HAL 库是否存在
dir third_party\STM32CubeF1\Drivers\STM32F1xx_HAL_Driver\Inc\
```

### Q2: 链接时提示空间不足？

**错误信息**：
```
region `FLASH' overflowed by XXXX bytes
```

**解决方案**：
1. 检查是否启用了不需要的 HAL 模块
2. 在 `stm32f1xx_hal_conf.h` 中注释掉未使用的模块
3. 使用 `-Os` 优化大小

### Q3: 烧录失败？

**错误信息**：
```
Error: libusb_open() failed with LIBUSB_ERROR_NOT_FOUND
```

**解决方案**：
1. 检查设备管理器中 ST-Link 是否被正确识别
2. 确认已安装 ST-Link 驱动（STSW-LINK009）
3. 如果使用克隆版 ST-Link，可能需要使用 Zadig 替换驱动为 WinUSB
4. 详见 [01_environment_setup](./01_environment_setup) 中的 ST-Link 驱动安装章节

### Q4: 调试时无法暂停程序？

**可能原因**：
1. ST-Link 连接不稳定
2. 调试配置文件路径错误
3. .elf 文件路径不正确

**解决方案**：
1. 检查 ST-Link 连接
2. 确认 `launch.json` 中的 `executable` 路径正确
3. 重新编译项目

---

## 📖 参考资料

- [CMake 官方文档](https://cmake.org/documentation/)
- [STM32CubeF1 HAL 库文档](https://www.st.com/en/embedded-software/stm32cube-f1.html)
- [ARM GCC 工具链文档](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
- [链接脚本详解](https://sourceware.org/binutils/docs/ld/)

---

**上一章**：[环境配置](./01_environment_setup) | **下一章**：[代码详解](./03_code_walkthrough)