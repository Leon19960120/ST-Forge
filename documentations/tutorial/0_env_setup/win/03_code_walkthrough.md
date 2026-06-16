# 03 - 代码详解

本文档将逐行解析 STM32F103C8T6 模板项目的核心代码，帮助初学者理解每一行代码的作用。

> **前置知识**: 阅读本文档前，建议先了解 [06_stm32_basics](06_stm32_basics) 中的 GPIO 和时钟基础知识。

---

## 目录

1. [main.c 主程序解析](#1-mainc-主程序解析)
2. [system.c 时钟配置解析](#2-systemc-时钟配置解析)
3. [stm32f1xx_hal_conf.h 配置说明](#3-stm32f1xx_hal_confh-配置说明)
4. [HAL 模块启用/禁用方法](#4-hal-模块启用禁用方法)

---

## 1. main.c 主程序解析

### 1.1 头文件包含

```c
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"
```

**逐行解释:**

| 行号 | 代码 | 作用 |
|------|------|------|
| 1 | `#include "stm32f1xx_hal.h"` | HAL 库主头文件，包含所有 HAL 模块的声明 |
| 2 | `#include "stm32f1xx_hal_gpio.h"` | GPIO 操作相关函数声明 |
| 3 | `#include "stm32f1xx_hal_rcc.h"` | 时钟控制相关函数声明 |

**说明:**
- `stm32f1xx_hal.h` 是 HAL 库的统一入口头文件
- 实际上，只包含 `stm32f1xx_hal.h` 即可，它会根据配置自动包含其他模块头文件
- 这里显式包含 GPIO 和 RCC 头文件是为了代码清晰度

---

### 1.2 函数声明

```c
void SystemClock_Config(void);
```

**解释:**
- 声明系统时钟配置函数
- 该函数在 `system.c` 中实现
- 必须在使用前声明，否则编译器会报错

---

### 1.3 主函数结构

```c
int main() {
  HAL_Init();
  SystemClock_Config();

  // GPIO 配置代码...

  while (1) {
    // 主循环代码...
  }
}
```

**主函数执行流程:**

```
┌─────────────────────────────────────────────────────────┐
│                      main() 入口                         │
└─────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────┐
│  HAL_Init() - HAL 库初始化                               │
│  • 初始化 Flash 延迟                                     │
│  • 设置 SysTick 定时器                                   │
│  • 初始化 HAL 库内部状态                                 │
└─────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────┐
│  SystemClock_Config() - 系统时钟配置                     │
│  • 配置 HSI 振荡器                                       │
│  • 配置 PLL 倍频                                         │
│  • 设置系统时钟为 64MHz                                  │
└─────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────┐
│  GPIO 配置                                               │
│  • 使能 GPIOC 时钟                                       │
│  • 配置 PC13 为输出模式                                  │
└─────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────┐
│  while(1) 无限循环                                       │
│  • LED 闪烁逻辑                                          │
│  • 永不退出                                             │
└─────────────────────────────────────────────────────────┘
```

---

### 1.4 HAL_Init() 函数详解

```c
HAL_Init();
```

**函数作用:**
- 初始化 HAL 库的核心功能
- 必须在任何其他 HAL 函数之前调用

**内部执行的操作:**

| 操作 | 说明 |
|------|------|
| Flash 延迟配置 | 根据系统时钟频率设置 Flash 等待周期 |
| SysTick 配置 | 配置系统滴答定时器，用于 HAL_Delay() 函数 |
| HAL 库状态初始化 | 初始化 HAL 库的内部状态变量 |

**返回值:**
- `HAL_OK`: 初始化成功
- `HAL_ERROR`: 初始化失败

---

### 1.5 GPIO 配置详解

```c
GPIO_InitTypeDef def;
def.Mode = GPIO_MODE_OUTPUT_PP;
def.Pin = GPIO_PIN_13;
def.Speed = GPIO_SPEED_FREQ_LOW;
def.Pull = GPIO_PULLDOWN;
__HAL_RCC_GPIOC_CLK_ENABLE();
HAL_GPIO_Init(GPIOC, &def);
```

**逐行解析:**

#### 第11行: 定义配置结构体

```c
GPIO_InitTypeDef def;
```

- `GPIO_InitTypeDef` 是 HAL 库定义的结构体类型
- 用于存储 GPIO 引脚的配置参数
- 结构体成员包括: Pin, Mode, Speed, Pull

#### 第12行: 设置引脚模式

```c
def.Mode = GPIO_MODE_OUTPUT_PP;
```

**GPIO 模式选项:**

| 模式常量 | 说明 | 典型用途 |
|----------|------|----------|
| `GPIO_MODE_INPUT` | 浮空输入 | 读取外部信号 |
| `GPIO_MODE_OUTPUT_PP` | 推挽输出 | LED、继电器控制 |
| `GPIO_MODE_OUTPUT_OD` | 开漏输出 | I2C 通信 |
| `GPIO_MODE_AF_PP` | 复用推挽输出 | UART、SPI |
| `GPIO_MODE_AF_OD` | 复用开漏输出 | I2C |
| `GPIO_MODE_ANALOG` | 模拟模式 | ADC 输入 |
| `GPIO_MODE_IT_RISING` | 中断上升沿触发 | 按键检测 |
| `GPIO_MODE_IT_FALLING` | 中断下降沿触发 | 按键检测 |

**推挽输出 vs 开漏输出:**

```
推挽输出 (Push-Pull):
    VCC
     │
    ┌┴┐
    │ │ P-MOS
    └┬┘
     ├─── 输出
    ┌┴┐
    │ │ N-MOS
    └┬┘
     │
    GND

开漏输出 (Open-Drain):
    VCC
     │
     │ (无上拉)
     │
    ┌┴┐
    │ │ N-MOS
    └┬┘
     ├─── 输出
     │
    GND
```

#### 第13行: 设置引脚号

```c
def.Pin = GPIO_PIN_13;
```

- `GPIO_PIN_13` 表示第 13 号引脚
- Blue Pill 开发板上，PC13 连接板载 LED
- 可以使用位或运算同时配置多个引脚:
  ```c
  def.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  ```

#### 第14行: 设置输出速度

```c
def.Speed = GPIO_SPEED_FREQ_LOW;
```

**GPIO 速度选项:**

| 速度常量 | 频率范围 | 功耗 | 典型用途 |
|----------|----------|------|----------|
| `GPIO_SPEED_FREQ_LOW` | 2MHz 以下 | 最低 | LED 控制、低速信号 |
| `GPIO_SPEED_FREQ_MEDIUM` | 2-10MHz | 中等 | 一般用途 |
| `GPIO_SPEED_FREQ_HIGH` | 10-50MHz | 较高 | 高速通信 |

**说明:**
- 速度越高，信号边沿越陡峭
- 速度越高，功耗和电磁干扰越大
- LED 控制使用低速即可

#### 第15行: 设置上下拉电阻

```c
def.Pull = GPIO_PULLDOWN;
```

**上下拉选项:**

| 选项 | 说明 |
|------|------|
| `GPIO_NOPULL` | 无上下拉 |
| `GPIO_PULLUP` | 内部上拉电阻 |
| `GPIO_PULLDOWN` | 内部下拉电阻 |

**说明:**
- 输出模式下，上下拉配置通常不影响功能
- 但建议设置为确定值，避免未定义行为

#### 第16行: 使能 GPIO 时钟

```c
__HAL_RCC_GPIOC_CLK_ENABLE();
```

**为什么需要使能时钟?**

```
┌─────────────────────────────────────────────────────────┐
│                    RCC (复位和时钟控制)                  │
├─────────────────────────────────────────────────────────┤
│                                                         │
│   ┌─────────┐   ┌─────────┐   ┌─────────┐              │
│   │ GPIOA   │   │ GPIOB   │   │ GPIOC   │   ...        │
│   │ 时钟使能│   │ 时钟使能│   │ 时钟使能│              │
│   └────┬────┘   └────┬────┘   └────┬────┘              │
│        │             │             │                    │
│        ▼             ▼             ▼                    │
│   ┌─────────┐   ┌─────────┐   ┌─────────┐              │
│   │ GPIOA   │   │ GPIOB   │   │ GPIOC   │   ...        │
│   │ 外设    │   │ 外设    │   │ 外设    │              │
│   └─────────┘   └─────────┘   └─────────┘              │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

- STM32 为了省电，默认关闭所有外设时钟
- 使用 GPIO 前必须手动使能对应时钟
- `__HAL_RCC_GPIOC_CLK_ENABLE()` 是宏定义，操作 RCC 寄存器

**时钟使能宏命名规则:**

```c
__HAL_RCC_GPIOA_CLK_ENABLE();  // 使能 GPIOA 时钟
__HAL_RCC_GPIOB_CLK_ENABLE();  // 使能 GPIOB 时钟
__HAL_RCC_GPIOC_CLK_ENABLE();  // 使能 GPIOC 时钟
__HAL_RCC_TIM1_CLK_ENABLE();   // 使能 TIM1 时钟
__HAL_RCC_USART1_CLK_ENABLE(); // 使能 USART1 时钟
```

#### 第17行: 初始化 GPIO

```c
HAL_GPIO_Init(GPIOC, &def);
```

**函数原型:**
```c
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);
```

**参数说明:**

| 参数 | 类型 | 说明 |
|------|------|------|
| `GPIOx` | `GPIO_TypeDef*` | GPIO 端口 (GPIOA, GPIOB, GPIOC...) |
| `GPIO_Init` | `GPIO_InitTypeDef*` | 配置结构体指针 |

**内部执行的操作:**
1. 配置 GPIO 模式寄存器 (MODER)
2. 配置输出类型寄存器 (OTYPER)
3. 配置输出速度寄存器 (OSPEEDR)
4. 配置上下拉寄存器 (PUPDR)

---

### 1.6 LED 初始状态设置

```c
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
```

**函数原型:**
```c
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
```

**参数说明:**

| 参数 | 说明 |
|------|------|
| `GPIOx` | GPIO 端口 |
| `GPIO_Pin` | 引脚号 |
| `PinState` | `GPIO_PIN_SET` (高电平) 或 `GPIO_PIN_RESET` (低电平) |

**Blue Pill LED 特点:**
- PC13 连接的 LED 是**低电平点亮**
- `GPIO_PIN_SET` (高电平) = LED 熄灭
- `GPIO_PIN_RESET` (低电平) = LED 点亮

---

### 1.7 LED 闪烁逻辑

```c
while (1) {
  HAL_Delay(1000);
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}
```

**逐行解析:**

#### HAL_Delay() 函数

```c
HAL_Delay(1000);
```

- 延时 1000 毫秒 (1秒)
- 基于 SysTick 定时器实现
- 精度取决于系统时钟配置

**注意事项:**
- `HAL_Delay()` 是阻塞延时，期间 CPU 无法执行其他任务
- 如需非阻塞延时，应使用定时器中断

#### HAL_GPIO_TogglePin() 函数

```c
HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
```

**函数原型:**
```c
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
```

**功能:**
- 翻转指定引脚的电平状态
- 高电平 → 低电平
- 低电平 → 高电平

**等效代码:**
```c
// Toggle 的等效实现
if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) {
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
} else {
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}
```

---

## 2. system.c 时钟配置解析

### 2.1 时钟系统概述

STM32F103C8T6 的时钟系统较为复杂，主要时钟源包括:

```
                    ┌─────────────────────────────────────────────────────────┐
                    │                    STM32F103 时钟树                      │
                    └─────────────────────────────────────────────────────────┘

┌──────────┐        ┌──────────┐        ┌──────────┐        ┌──────────┐
│   HSI    │        │   HSE    │        │   LSI    │        │   LSE    │
│  8MHz    │        │  8MHz    │        │  40kHz   │        │ 32.768kHz│
│ (内部)   │        │ (外部)   │        │ (内部)   │        │ (外部)   │
└────┬─────┘        └────┬─────┘        └────┬─────┘        └────┬─────┘
     │                   │                   │                   │
     │                   │                   │                   │
     ▼                   ▼                   ▼                   ▼
┌─────────────────────────────┐      ┌──────────┐       ┌──────────┐
│          PLL                │      │   RTC    │       │   IWDG   │
│  (锁相环倍频)               │      │  实时时钟 │       │ 独立看门狗│
│                             │      └──────────┘       └──────────┘
│  输入: HSI/2 或 HSE         │
│  倍频: x2 ~ x16             │
│  输出: 最高 72MHz           │
└────────────┬────────────────┘
             │
             ▼
      ┌──────────────┐
      │   SYSCLK      │  系统时钟
      │   (64MHz)     │
      └──────┬───────┘
             │
             ▼
      ┌──────────────┐
      │   AHB        │  高速总线
      │   HCLK       │  (64MHz)
      └──────┬───────┘
             │
     ┌───────┴───────┐
     │               │
     ▼               ▼
┌─────────┐    ┌─────────┐
│  APB1   │    │  APB2   │
│  PCLK1  │    │  PCLK2  │
│ (32MHz) │    │ (64MHz) │
└─────────┘    └─────────┘
     │               │
     ▼               ▼
 TIM2-7         TIM1, GPIOA-C
 USART2-5       USART1, SPI1
 I2C1-2         ADC1-2
```

---

### 2.2 SystemClock_Config() 函数详解

```c
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
```

---

### 2.3 振荡器配置 (RCC_OscInitTypeDef)

```c
RCC_OscInitTypeDef osc = {0};
```

**结构体成员:**

| 成员 | 类型 | 说明 |
|------|------|------|
| `OscillatorType` | uint32_t | 要配置的振荡器类型 |
| `HSIState` | uint32_t | HSI 状态 (开/关) |
| `HSICalibrationValue` | uint32_t | HSI 校准值 |
| `HSEState` | uint32_t | HSE 状态 |
| `PLL` | RCC_PLLInitTypeDef | PLL 配置 |

---

#### 振荡器类型配置

```c
osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
```

**可选值:**

| 常量 | 说明 |
|------|------|
| `RCC_OSCILLATORTYPE_NONE` | 不配置任何振荡器 |
| `RCC_OSCILLATORTYPE_HSI` | 内部高速振荡器 (8MHz) |
| `RCC_OSCILLATORTYPE_HSE` | 外部高速振荡器 (通常 8MHz) |
| `RCC_OSCILLATORTYPE_LSI` | 内部低速振荡器 (40kHz) |
| `RCC_OSCILLATORTYPE_LSE` | 外部低速振荡器 (32.768kHz) |

---

#### HSI 配置

```c
osc.HSIState = RCC_HSI_ON;
osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
```

**说明:**
- `RCC_HSI_ON`: 开启 HSI 振荡器
- `RCC_HSICALIBRATION_DEFAULT`: 使用默认校准值 (约 16)

**HSI 特点:**
- 频率: 8MHz (出厂校准)
- 精度: ±1% (25°C)
- 启动时间: 快 (约 1μs)
- 功耗: 较低

---

#### PLL 配置

```c
osc.PLL.PLLState = RCC_PLL_ON;
osc.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2; /* 4MHz  */
osc.PLL.PLLMUL = RCC_PLL_MUL16;             /* 64MHz */
```

**PLL (锁相环) 工作原理:**

```
         ┌─────────────────────────────────────────────────────┐
         │                      PLL                            │
         │                                                     │
输入 ────┤  ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────┤──── 输出
(HSI/2)  │  │  相位   │    │  低通   │    │  VCO    │    │     │
 4MHz    │  │  检测器 │───▶│  滤波器 │───▶│压控振荡│───▶│ ÷N  │    64MHz
         │  │         │    │         │    │  器     │    │     │
         │  └────┬────┘    └─────────┘    └─────────┘    └──┬──┘
         │       │                                          │   │
         │       └──────────────────────────────────────────┘   │
         │              反馈比较                                │
         └─────────────────────────────────────────────────────┘
```

**时钟计算:**

```
HSI (内部振荡器) = 8MHz
         │
         ▼
    HSI ÷ 2 = 4MHz (PLL 输入)
         │
         ▼
    PLL × 16 = 64MHz (系统时钟)
```

**PLL 倍频系数选项:**

| 常量 | 倍频 | 输出频率 (HSI/2 输入) |
|------|------|----------------------|
| `RCC_PLL_MUL2` | ×2 | 8MHz |
| `RCC_PLL_MUL3` | ×3 | 12MHz |
| `RCC_PLL_MUL4` | ×4 | 16MHz |
| ... | ... | ... |
| `RCC_PLL_MUL16` | ×16 | 64MHz |

**注意:** STM32F103C8T6 最高支持 72MHz，本例使用 64MHz 是安全选择。

---

### 2.4 时钟分频配置 (RCC_ClkInitTypeDef)

```c
RCC_ClkInitTypeDef clk = {0};
clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
clk.AHBCLKDivider = RCC_SYSCLK_DIV1; /* HCLK  = 64MHz */
clk.APB1CLKDivider = RCC_HCLK_DIV2;  /* APB1  = 32MHz */
clk.APB2CLKDivider = RCC_HCLK_DIV1;  /* APB2  = 64MHz */
HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);
```

---

#### 时钟类型配置

```c
clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
```

**说明:**
- 使用位或运算同时配置多个时钟类型
- `SYSCLK`: 系统时钟
- `HCLK`: AHB 总线时钟
- `PCLK1`: APB1 总线时钟
- `PCLK2`: APB2 总线时钟

---

#### 系统时钟源选择

```c
clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
```

**可选值:**

| 常量 | 说明 |
|------|------|
| `RCC_SYSCLKSOURCE_HSI` | 使用 HSI 作为系统时钟 (8MHz) |
| `RCC_SYSCLKSOURCE_HSE` | 使用 HSE 作为系统时钟 |
| `RCC_SYSCLKSOURCE_PLLCLK` | 使用 PLL 输出作为系统时钟 |

---

#### AHB 分频配置

```c
clk.AHBCLKDivider = RCC_SYSCLK_DIV1; /* HCLK = 64MHz */
```

**说明:**
- AHB (Advanced High-performance Bus) 是高速总线
- 连接 CPU、内存、DMA 等高速外设
- `RCC_SYSCLK_DIV1` 表示不分频，HCLK = SYSCLK = 64MHz

**分频选项:**

| 常量 | 分频比 |
|------|--------|
| `RCC_SYSCLK_DIV1` | 1 (不分频) |
| `RCC_SYSCLK_DIV2` | 2 |
| `RCC_SYSCLK_DIV4` | 4 |
| `RCC_SYSCLK_DIV8` | 8 |
| ... | ... |

---

#### APB1 分频配置

```c
clk.APB1CLKDivider = RCC_HCLK_DIV2; /* APB1 = 32MHz */
```

**说明:**
- APB1 (Advanced Peripheral Bus 1) 是低速外设总线
- 最高频率限制为 36MHz
- 连接 TIM2-7, USART2-5, I2C1-2, CAN, USB 等外设
- `RCC_HCLK_DIV2` 表示 2 分频，APB1 = HCLK/2 = 32MHz

**重要:** APB1 最高只能运行在 36MHz，必须分频！

---

#### APB2 分频配置

```c
clk.APB2CLKDivider = RCC_HCLK_DIV1; /* APB2 = 64MHz */
```

**说明:**
- APB2 (Advanced Peripheral Bus 2) 是高速外设总线
- 最高频率可达 72MHz
- 连接 TIM1, GPIOA-C, USART1, SPI1, ADC1-2 等外设
- `RCC_HCLK_DIV1` 表示不分频，APB2 = HCLK = 64MHz

---

#### Flash 延迟配置

```c
HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);
```

**Flash 延迟说明:**

| 系统时钟 | Flash 延迟 | 常量 |
|----------|------------|------|
| 0-24MHz | 0 等待周期 | `FLASH_LATENCY_0` |
| 24-48MHz | 1 等待周期 | `FLASH_LATENCY_1` |
| 48-72MHz | 2 等待周期 | `FLASH_LATENCY_2` |

**为什么需要 Flash 延迟?**
- Flash 存储器读取速度有限
- 系统时钟越高，CPU 读取 Flash 越快
- 需要插入等待周期确保正确读取

---

### 2.5 时钟配置总结

```
┌─────────────────────────────────────────────────────────────────┐
│                     时钟配置总结                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  HSI (内部振荡器) = 8MHz                                        │
│         │                                                       │
│         ▼                                                       │
│  PLL 输入 = HSI ÷ 2 = 4MHz                                     │
│         │                                                       │
│         ▼                                                       │
│  PLL 输出 = 4MHz × 16 = 64MHz                                   │
│         │                                                       │
│         ▼                                                       │
│  SYSCLK (系统时钟) = 64MHz                                      │
│         │                                                       │
│         ▼                                                       │
│  HCLK (AHB 总线) = 64MHz (DIV1)                                 │
│         │                                                       │
│    ┌────┴────┐                                                  │
│    │         │                                                  │
│    ▼         ▼                                                  │
│  APB1      APB2                                                 │
│  32MHz     64MHz                                                │
│  (DIV2)    (DIV1)                                               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 3. stm32f1xx_hal_conf.h 配置说明

### 3.1 文件作用

`stm32f1xx_hal_conf.h` 是 HAL 库的配置文件，用于:

1. **启用/禁用 HAL 模块** - 减少代码体积
2. **配置时钟参数** - 定义晶振频率
3. **配置系统参数** - 电压、优先级等

---

### 3.2 模块选择配置

```c
#define HAL_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_CAN_MODULE_ENABLED
#define HAL_CEC_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_CRC_MODULE_ENABLED
#define HAL_DAC_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_ETH_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_HCD_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_I2S_MODULE_ENABLED
#define HAL_IRDA_MODULE_ENABLED
#define HAL_IWDG_MODULE_ENABLED
#define HAL_NAND_MODULE_ENABLED
#define HAL_NOR_MODULE_ENABLED
#define HAL_PCCARD_MODULE_ENABLED
#define HAL_PCD_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_RTC_MODULE_ENABLED
#define HAL_SD_MODULE_ENABLED
#define HAL_SMARTCARD_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_SRAM_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_USART_MODULE_ENABLED
#define HAL_WWDG_MODULE_ENABLED
#define HAL_MMC_MODULE_ENABLED
```

**说明:**
- 每个宏定义启用一个 HAL 模块
- 注释掉宏定义可禁用对应模块
- 禁用未使用的模块可减少编译后的代码体积

---

### 3.3 时钟值配置

```c
#define HSE_VALUE 8000000U    // 外部晶振 8MHz
#define HSI_VALUE 8000000U    // 内部振荡器 8MHz
#define LSI_VALUE 40000U      // 内部低速振荡器 40kHz
#define LSE_VALUE 32768U      // 外部低速晶振 32.768kHz
```

**说明:**
- 这些值必须与实际硬件匹配
- 如果使用不同频率的晶振，需要修改 `HSE_VALUE`
- HAL 库使用这些值计算时钟频率

---

### 3.4 系统配置

```c
#define VDD_VALUE 3300U        // 供电电压 3.3V
#define TICK_INT_PRIORITY 0x0F // SysTick 中断优先级
#define USE_RTOS 0U           // 不使用 RTOS
#define PREFETCH_ENABLE 1U    // 启用 Flash 预取
```

---

## 4. HAL 模块启用/禁用方法

### 4.1 启用模块

要启用某个 HAL 模块，只需在 `stm32f1xx_hal_conf.h` 中取消注释对应的宏定义:

```c
// 启用 UART 模块
#define HAL_UART_MODULE_ENABLED
```

---

### 4.2 禁用模块

要禁用某个 HAL 模块，只需注释掉对应的宏定义:

```c
// 禁用 CAN 模块 (如果项目不需要 CAN)
// #define HAL_CAN_MODULE_ENABLED
```

---

### 4.3 最小化配置示例

对于只需要 GPIO 和延时的最小项目:

```c
// 最小配置 - 只启用必要模块
#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED

// 禁用其他所有模块
// #define HAL_ADC_MODULE_ENABLED
// #define HAL_CAN_MODULE_ENABLED
// ... 其他模块注释掉
```

---

### 4.4 常用模块说明

| 模块 | 说明 | 典型用途 |
|------|------|----------|
| `HAL_MODULE_ENABLED` | HAL 核心模块 | 必须启用 |
| `HAL_CORTEX_MODULE_ENABLED` | Cortex-M 内核支持 | 必须启用 |
| `HAL_RCC_MODULE_ENABLED` | 时钟控制 | 必须启用 |
| `HAL_GPIO_MODULE_ENABLED` | GPIO 操作 | LED、按键 |
| `HAL_UART_MODULE_ENABLED` | 串口通信 | 调试输出 |
| `HAL_TIM_MODULE_ENABLED` | 定时器 | PWM、延时 |
| `HAL_I2C_MODULE_ENABLED` | I2C 通信 | 传感器 |
| `HAL_SPI_MODULE_ENABLED` | SPI 通信 | Flash、显示屏 |
| `HAL_ADC_MODULE_ENABLED` | ADC 转换 | 模拟信号采集 |

---

## 5. 错误处理

### 5.1 HAL 函数返回值

大多数 HAL 函数返回 `HAL_StatusTypeDef`:

```c
typedef enum {
  HAL_OK       = 0x00U,  // 操作成功
  HAL_ERROR    = 0x01U,  // 操作失败
  HAL_BUSY     = 0x02U,  // 设备忙
  HAL_TIMEOUT  = 0x03U   // 操作超时
} HAL_StatusTypeDef;
```

---

### 5.2 错误处理示例

```c
// 推荐的错误处理方式
HAL_StatusTypeDef status;

status = HAL_RCC_OscConfig(&osc);
if (status != HAL_OK) {
  // 时钟配置失败处理
  while(1); // 停止运行
}

status = HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);
if (status != HAL_OK) {
  // 时钟配置失败处理
  while(1); // 停止运行
}
```

---

### 5.3 完整的错误处理模板

```c
void Error_Handler(void) {
  // 禁用中断
  __disable_irq();
  
  // 可以在这里添加错误指示 (如闪烁 LED)
  while (1) {
    // 死循环
  }
}

int main(void) {
  if (HAL_Init() != HAL_OK) {
    Error_Handler();
  }
  
  SystemClock_Config();
  
  // ... 其他初始化
  
  while (1) {
    // 主循环
  }
}
```

---

## 6. 代码执行流程总结

```
┌─────────────────────────────────────────────────────────────────┐
│                     完整执行流程                                 │
└─────────────────────────────────────────────────────────────────┘

系统上电
    │
    ▼
┌─────────────────────────────────────────────────────────────────┐
│ 1. 启动文件执行 (startup_stm32f1xx.s)                           │
│    • 初始化数据段                                               │
│    • 设置初始栈指针                                             │
│    • 调用 SystemInit()                                          │
│    • 跳转到 main()                                              │
└─────────────────────────────────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────────────────────────────────┐
│ 2. main() 函数开始                                              │
│    • HAL_Init() - 初始化 HAL 库                                 │
│    • SystemClock_Config() - 配置系统时钟                        │
└─────────────────────────────────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────────────────────────────────┐
│ 3. GPIO 初始化                                                  │
│    • 使能 GPIOC 时钟                                            │
│    • 配置 PC13 为推挽输出                                       │
│    • 设置初始状态                                               │
└─────────────────────────────────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────────────────────────────────┐
│ 4. 主循环 (while(1))                                            │
│    • HAL_Delay(1000) - 延时 1 秒                                │
│    • HAL_GPIO_TogglePin() - 翻转 LED                            │
│    • 循环执行                                                   │
└─────────────────────────────────────────────────────────────────┘
```

---

## 7. 常见问题

### Q1: 为什么 LED 不亮?

**可能原因:**
1. 时钟未正确配置
2. GPIO 时钟未使能
3. 引脚配置错误
4. LED 极性接反 (Blue Pill 的 PC13 LED 是低电平点亮)

---

### Q2: 如何修改 LED 闪烁频率?

修改 `HAL_Delay()` 的参数:
```c
HAL_Delay(500);  // 500ms = 0.5秒，闪烁更快
HAL_Delay(2000); // 2000ms = 2秒，闪烁更慢
```

---

### Q3: 如何使用其他引脚控制 LED?

```c
// 使用 PA0 控制 LED
__HAL_RCC_GPIOA_CLK_ENABLE();  // 使能 GPIOA 时钟

GPIO_InitTypeDef def = {0};
def.Pin = GPIO_PIN_0;          // 使用 PA0
def.Mode = GPIO_MODE_OUTPUT_PP;
def.Speed = GPIO_SPEED_FREQ_LOW;
def.Pull = GPIO_NOPULL;
HAL_GPIO_Init(GPIOA, &def);   // 初始化 GPIOA

// 控制 LED
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
```

---

### Q4: 如何使用外部晶振 (HSE)?

```c
void SystemClock_Config(void) {
  RCC_OscInitTypeDef osc = {0};
  
  // 使用 HSE (外部晶振)
  osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc.HSEState = RCC_HSE_ON;
  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;  // HSE 作为 PLL 输入
  osc.PLL.PLLMUL = RCC_PLL_MUL9;           // 8MHz × 9 = 72MHz
  
  HAL_RCC_OscConfig(&osc);
  
  // ... 时钟分频配置
}
```

---

## 8. 下一步学习

完成本教程后，建议继续学习:

1. **GPIO 输入** - 读取按键状态
2. **中断处理** - 外部中断、定时器中断
3. **UART 通信** - 串口调试输出
4. **定时器** - PWM 输出、输入捕获
5. **ADC** - 模拟信号采集

---

## 参考资料

- [STM32F103xx 参考手册 (RM0008)](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F103C8 数据手册](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
- [STM32CubeF1 HAL 文档](https://www.st.com/resource/en/user_manual/dm00159893-description-of-stm32f1xx-hal-drivers-stmicroelectronics.pdf)