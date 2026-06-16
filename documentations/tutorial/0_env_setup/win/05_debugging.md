# 05 - 调试方法

> **本章目标**：掌握 STM32F103C8T6 项目的调试方法，学会使用 VSCode + Cortex-Debug 进行断点调试、变量监视和故障排查。

---

## 目录

- [调试环境配置](#调试环境配置)
- [已知问题与修复](#已知问题与修复)
- [启动调试会话](#启动调试会话)
- [断点调试](#断点调试)
- [变量监视与寄存器查看](#变量监视与寄存器查看)
- [调试技巧](#调试技巧)
- [故障排查流程](#故障排查流程)
- [常见问题](#常见问题)

---

## 调试环境配置

### 硬件要求

| 组件 | 说明 |
|------|------|
| **调试器** | ST-Link V2 或 J-Link |
| **连接方式** | SWD (SWDIO, SWCLK, GND) |
| **目标板** | STM32F103C8T6 Blue Pill |

### 软件要求

#### 1. 安装 VSCode 扩展

在 VSCode 中安装 **Cortex-Debug** 扩展：

```
扩展 ID: marus25.cortex-debug
```

安装步骤：
1. 打开 VSCode
2. 按 `Ctrl+Shift+X` 打开扩展面板
3. 搜索 "Cortex-Debug"
4. 点击 "Install" 安装

#### 2. 安装 OpenOCD

**Windows 手动安装：**

1. 从 xPack OpenOCD GitHub Releases 页面下载最新版本：
   - 访问 [xPack OpenOCD Releases](https://github.com/xpack-dev-tools/openocd-xpack/releases/)
   - 下载 `xpack-openocd-<version>-win32-x64.zip`

2. 解压到目标目录：
   - 建议解压到 `C:\Tools\openocd`
   - 解压后目录结构应为 `C:\Tools\openocd\bin\openocd.exe`

3. 添加到系统 PATH：
   - 打开 "系统属性" → "高级" → "环境变量"
   - 在 "系统变量" 中找到 `Path`，点击 "编辑"
   - 添加新路径：`C:\Tools\openocd\bin`
   - 点击 "确定" 保存

> **提示**：也可以在 VSCode 集成终端中使用 PowerShell 快速添加用户 PATH：
> ```powershell
> # 仅对当前用户生效
> [Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\Tools\openocd\bin", "User")
> ```
> 添加后需要重启 VSCode 才能生效。

#### 3. 验证安装

```powershell
# 检查 OpenOCD 版本
openocd --version

# 检查 ARM 工具链
arm-none-eabi-gdb --version
```

### 调试器连接

#### ST-Link V2 接线图

```
ST-Link V2          Blue Pill
─────────           ──────────
  SWDIO    <───>      SWDIO
  SWCLK    <───>      SWCLK
  GND      <───>      GND
  3.3V     <───>      3.3V (可选，用于供电)
```

> **注意**：确保 Blue Pill 的 BOOT0 跳线设置为 0（接地），否则无法进入调试模式。

---

## 已知问题与修复

### launch.json 配置错误

**问题描述**：项目模板中的 `.vscode/launch.json` 配置存在可执行文件路径错误。

#### 错误配置

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
            "executable": "build/STM32Demo.elf",  // 错误：文件不存在
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

#### 正确配置

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
            "executable": "build/STM32F1.elf",  // 正确：实际生成的文件名
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

### 修复步骤

1. **打开配置文件**
   ```powershell
   # 在项目根目录下
   code .vscode/launch.json
   ```

2. **修改可执行文件路径**
   - 将 `"executable": "build/STM32Demo.elf"`
   - 改为 `"executable": "build/STM32F1.elf"`

3. **保存文件**
   - 按 `Ctrl+S` 保存

4. **验证修复**
   ```powershell
   # 确认 ELF 文件存在
   dir build\STM32F1.elf
   ```

### 为什么会出现这个问题？

| 原因 | 说明 |
|------|------|
| **模板命名不一致** | CMakeLists.txt 中项目名为 `STM32F1`，但 launch.json 使用了示例名称 |
| **文件名来源** | `STM32F1.elf` 由 `project(STM32F1 ...)` 定义生成 |
| **影响** | 调试器无法找到可执行文件，导致启动失败 |

### Cortex-Debug 路径配置

在 Windows 上，Cortex-Debug 需要找到 `arm-none-eabi-gdb` 和 `openocd` 的可执行文件。如果路径配置不正确，调试会话将无法启动。

#### 首选方案：使用系统 PATH（推荐）

确保 ARM GCC 工具链和 OpenOCD 的 `bin` 目录已加入系统 PATH（参见 [01-环境配置](./01_environment_setup)）。Cortex-Debug 会自动在 PATH 中查找 `arm-none-eabi-gdb` 和 `openocd`。

验证 PATH 配置：
```powershell
# 确认工具链和 OpenOCD 在 PATH 中可用
arm-none-eabi-gdb --version
openocd --version
```

如果两个命令都能正常输出版本信息，Cortex-Debug 即可自动找到它们，无需额外配置。

#### 备用方案：手动指定路径

如果 PATH 方式不生效（例如 VSCode 未重启、PATH 未正确加载），可以在 VSCode 的 `settings.json` 中手动指定路径：

1. 按 `Ctrl+Shift+P`，输入 "Open User Settings (JSON)"
2. 添加以下配置：

```json
{
    "cortex-debug.armToolchainPath": "C:/Tools/gcc-arm/bin",
    "cortex-debug.openOCDPath": "C:/Tools/openocd/bin/openocd.exe"
}
```

> **重要提示**：
> - 路径必须使用正斜杠（`/`）或双反斜杠（`\\`）。单反斜杠（`\`）在 JSON 中是转义字符，会导致路径解析失败。
> - 正确写法：`C:/Tools/gcc-arm/bin` 或 `C:\\Tools\\gcc-arm\\bin`
> - 错误写法：`C:\Tools\gcc-arm\bin`
> - 如果使用工作区级别的 settings.json，路径应使用 `${workspaceFolder}` 变量保持可移植性。

---

## 启动调试会话

### 前置条件

在启动调试之前，确保：

1. 项目已编译（存在 `build/STM32F1.elf`）
2. 调试器已连接（ST-Link/J-Link）
3. 目标板已供电
4. launch.json 配置正确

### 启动步骤

#### 方法 1：快捷键启动

1. 打开 VSCode
2. 按 `F5` 启动调试
3. 选择 "STM32 Debug" 配置（如有多个配置）

#### 方法 2：调试面板启动

1. 点击左侧活动栏的 "Run and Debug" 图标（或按 `Ctrl+Shift+D`）
2. 在顶部下拉菜单选择 "STM32 Debug"
3. 点击绿色播放按钮

### 调试启动流程

```
+------------------+
|  按 F5 启动      |
+--------+---------+
         |
         v
+------------------+
| VSCode 读取      |
| launch.json      |
+--------+---------+
         |
         v
+------------------+
| 启动 OpenOCD     |
| 连接调试器       |
+--------+---------+
         |
         v
+------------------+
| 加载 ELF 文件    |
| 到 GDB           |
+--------+---------+
         |
         v
+------------------+
| 程序停在 main()  |
| 等待用户操作     |
+------------------+
```

### 启动成功标志

当调试会话成功启动时，您会看到：

1. **底部状态栏**：显示 "Cortex-Debug: Running" 或 "Paused"
2. **调试工具栏**：出现在编辑器顶部
3. **调试控制台**：显示 GDB 输出信息
4. **断点高亮**：如果设置了断点，程序会暂停在那里

---

## 断点调试

### 设置断点

#### 方法 1：点击行号

在代码编辑器中，点击行号左侧的空白区域，会出现一个红色圆点，表示断点已设置。

```
  45 |     HAL_Init();
  46 |     SystemClock_Config();
  47 |     MX_GPIO_Init();
  48 |     MX_USART1_UART_Init();  // <-- 点击这里设置断点
  49 |
  50 |     while (1)
  51 |     {
```

#### 方法 2：右键菜单

1. 右键点击代码行
2. 选择 "Add Breakpoint"

#### 方法 3：快捷键

- 将光标移动到目标行
- 按 `F9` 切换断点

### 断点类型

| 类型 | 说明 |
|------|------|
| **普通断点** | 红色圆点，程序执行到此暂停 |
| **条件断点** | 红色圆点带条件，满足条件时暂停 |
| **日志断点** | 菱形图标，输出日志但不暂停 |
| **禁用断点** | 空心圆点，断点已禁用 |

### 设置条件断点

1. 右键点击断点
2. 选择 "Edit Breakpoint..."
3. 输入条件表达式，例如：
   ```c
   i == 100
   ```
4. 程序只有在 `i == 100` 时才会暂停

### 断点管理

在 **断点面板**（左侧调试面板）中可以：

- 查看所有断点
- 启用/禁用断点
- 删除断点
- 编辑断点条件

---

## 调试控制

### 基本操作

| 操作 | 快捷键 | 说明 |
|------|--------|------|
| **继续运行** | `F5` | 运行到下一个断点 |
| **单步跳过** | `F10` | 执行当前行，不进入函数 |
| **单步进入** | `F11` | 进入函数内部 |
| **单步返回** | `Shift+F11` | 执行完当前函数并返回 |
| **重启调试** | `Ctrl+Shift+F5` | 重新启动调试会话 |
| **停止调试** | `Shift+F5` | 终止调试会话 |

### 执行流程示例

```
当前代码：
    48 |     MX_USART1_UART_Init();  // <-- 当前位置
    49 |
    50 |     while (1)
    51 |     {
    52 |         HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

操作：按 F10 (Step Over)
结果：执行 MX_USART1_UART_Init()，然后停在第 50 行

操作：按 F11 (Step Into)
结果：进入 MX_USART1_UART_Init() 函数内部
```

### 调试工具栏

```
+----------------------------------------------------------+
| >   >>   v   ^   @   X                                   |
| |   |    |   |   |   |                                   |
| |   |    |   |   |   +-- 停止调试                        |
| |   |    |   |   +------ 重启调试                        |
| |   |    |   +---------- 单步返回                        |
| |   |    +-------------- 单步进入                        |
| |   +------------------- 单步跳过                        |
| +----------------------- 继续运行                        |
+----------------------------------------------------------+
```

---

## 变量监视与寄存器查看

### 变量面板

在左侧调试面板的 **变量** 区域，可以查看：

#### 局部变量

自动显示当前作用域内的局部变量：

```c
// 在断点处暂停时
void blink_led(void) {
    uint32_t counter = 0;      // <-- Locals 区域显示
    GPIO_PinState state;       // <-- Locals 区域显示

    while (1) {
        counter++;             // <-- 值会实时更新
        ...
    }
}
```

#### 全局变量

在 **Watch** 面板中添加全局变量：

1. 点击 "Watch" 区域的 "+" 按钮
2. 输入变量名，如 `g_uart_buffer`
3. 变量值会实时显示

### 监视表达式

#### 添加监视

1. 在 Watch 面板点击 "+"
2. 输入表达式，例如：
   - `counter` - 变量值
   - `array[0]` - 数组元素
   - `ptr->member` - 结构体成员
   - `GPIOA->ODR` - 外设寄存器

#### 实用监视表达式

| 表达式 | 说明 |
|--------|------|
| `HAL_GetTick()` | 当前系统滴答计数 |
| `GPIOA->ODR` | GPIOA 输出数据寄存器 |
| `USART1->DR` | USART1 数据寄存器 |
| `RCC->CR` | 时钟控制寄存器 |
| `*(uint32_t*)0x20000000` | 查看特定内存地址 |

### 寄存器查看

在 **Call Stack** 面板下方，展开 **Registers** 可以查看：

#### ARM Cortex-M3 核心寄存器

| 寄存器 | 说明 |
|--------|------|
| `R0-R12` | 通用寄存器 |
| `SP` (R13) | 堆栈指针 |
| `LR` (R14) | 链接寄存器 |
| `PC` (R15) | 程序计数器 |
| `xPSR` | 程序状态寄存器 |

#### 特殊寄存器

| 寄存器 | 说明 |
|--------|------|
| `MSP` | 主堆栈指针 |
| `PSP` | 进程堆栈指针 |
| `PRIMASK` | 异常屏蔽寄存器 |
| `CONTROL` | 控制寄存器 |

### 外设寄存器查看

如果配置了 SVD 文件，可以在 **Peripherals** 面板查看外设寄存器：

#### 配置 SVD 文件

在 `launch.json` 中添加：

```json
{
    "svdFile": "${workspaceFolder}/STM32F103.svd"
}
```

#### 查看外设

1. 在调试暂停时，展开 **Peripherals** 面板
2. 选择外设（如 GPIOA）
3. 查看寄存器值

```
Peripherals
+-- GPIOA
    +-- CRL: 0x44444444
    +-- CRH: 0x44444444
    +-- IDR: 0x0000FFFF
    +-- ODR: 0x00000000
    +-- ...
```

---

## 调试技巧

### 1. LED 视觉调试

使用 LED 作为程序状态的视觉指示器：

```c
// 在关键位置切换 LED
void critical_function(void) {
    LED_ON();  // 进入函数

    // ... 执行关键代码 ...

    LED_OFF(); // 函数完成
}
```

#### 状态编码

通过 LED 闪烁模式表示不同状态：

```c
void indicate_error(uint8_t error_code) {
    for (int i = 0; i < error_code; i++) {
        LED_ON();
        HAL_Delay(200);
        LED_OFF();
        HAL_Delay(200);
    }
    HAL_Delay(1000);  // 间隔 1 秒
}

// 使用示例
if (init_failed) {
    indicate_error(3);  // 闪烁 3 次 = 初始化失败
}
```

### 2. printf 风格调试（串口输出）

通过串口输出调试信息：

#### 配置串口重定向

```c
// 在 main.c 或单独文件中添加
#include <stdio.h>

// 重定向 printf 到 USART1
#ifdef __GNUC__
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif
```

#### 使用示例

```c
// 在关键位置输出调试信息
void process_data(uint8_t *data, uint32_t len) {
    printf("Processing %lu bytes\r\n", len);

    for (uint32_t i = 0; i < len; i++) {
        printf("data[%lu] = 0x%02X\r\n", i, data[i]);
    }

    printf("Processing complete\r\n");
}
```

#### 查看串口输出（Windows）

**步骤一：确认 COM 端口号**

1. 将 USB 转串口模块连接到电脑
2. 右键点击 "开始" 菜单，选择 "设备管理器"
3. 展开 "端口 (COM 和 LPT)" 节点
4. 找到对应的 COM 端口号（例如 `COM3`、`COM4` 等）

> **提示**：如果设备管理器中看不到 COM 端口，或者显示为带黄色感叹号的未知设备，说明需要安装 USB 转串口驱动（通常是 CH340 或 CP2102 驱动）。

**步骤二：使用串口终端工具**

以下任选其一：

**PuTTY（推荐）**

1. 下载并安装 [PuTTY](https://www.putty.org/)
2. 打开 PuTTY，选择 "Serial" 连接类型
3. 设置参数：
   - Serial line：`COM3`（替换为实际端口号）
   - Speed：`115200`
4. 点击 "Open" 打开连接

**Tera Term（备选）**

1. 下载并安装 [Tera Term](https://ttssh2.osdn.jp/)
2. 打开 Tera Term，选择 "Serial" 连接
3. 选择对应的 COM 端口
4. 在 "Setup" → "Serial port" 中设置波特率为 `115200`

### 3. 断言检查

使用断言在开发阶段捕获错误：

```c
#include <assert.h>

void set_gpio_pin(GPIO_TypeDef *GPIOx, uint16_t Pin) {
    // 检查参数有效性
    assert(GPIOx != NULL);
    assert(Pin < 16);

    // ... 设置 GPIO ...
}
```

#### 自定义断言处理

```c
// 在 stm32f1xx_hal_conf.h 或单独文件中
#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))

void assert_failed(uint8_t *file, uint32_t line) {
    printf("Assertion failed: %s:%lu\r\n", file, line);
    while (1);  // 停在这里等待调试
}
```

### 4. 看门狗调试

使用独立看门狗（IWDG）检测死循环：

```c
// 初始化看门狗
void init_watchdog(void) {
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
    hiwdg.Init.Reload = 0xFFF;
    HAL_IWDG_Init(&hiwdg);
}

// 在主循环中喂狗
while (1) {
    HAL_IWDG_Refresh(&hiwdg);  // 喂狗

    // ... 主循环代码 ...
}
```

> **提示**：如果程序卡死在看门狗复位，说明主循环中有阻塞代码。

### 5. 内存调试

#### 检查栈溢出

```c
// 在链接脚本中检查栈使用
// 或使用静态分析工具

// 运行时检查栈指针
uint32_t get_stack_usage(void) {
    extern uint32_t _estack;  // 栈顶地址
    extern uint32_t _Min_Stack_Size;

    uint32_t sp;
    __asm volatile ("mov %0, sp" : "=r" (sp));

    return (_estack - sp);
}
```

#### 堆内存检查

```c
// 如果使用动态内存分配
void check_heap(void) {
    extern uint32_t _end;
    extern uint32_t _Heap_Limit;

    printf("Heap start: 0x%08lX\r\n", (uint32_t)&_end);
    printf("Heap limit: 0x%08lX\r\n", (uint32_t)&_Heap_Limit);
}
```

---

## 故障排查流程

### 调试无法启动

#### 问题诊断流程图

```
+----------------------------------------------------------+
| 调试无法启动                                              |
+---------------------------+------------------------------+
                            |
                            v
+----------------------------------------------------------+
| 检查硬件连接                                              |
| - ST-Link 是否正确连接？                                  |
| - 目标板是否供电？                                        |
| - BOOT0 是否为 0？                                        |
+---------------------------+------------------------------+
                            |
                            v
+----------------------------------------------------------+
| 检查 OpenOCD                                              |
| - openocd --version 是否正常？                            |
| - 是否有其他 OpenOCD 进程占用？                           |
+---------------------------+------------------------------+
                            |
                            v
+----------------------------------------------------------+
| 检查 ELF 文件                                             |
| - build\STM32F1.elf 是否存在？                            |
| - launch.json 路径是否正确？                              |
+---------------------------+------------------------------+
                            |
                            v
+----------------------------------------------------------+
| 检查 VSCode 扩展                                          |
| - Cortex-Debug 是否已安装？                               |
| - 是否有版本冲突？                                        |
+----------------------------------------------------------+
```

#### 常见错误及解决

| 错误信息 | 可能原因 | 解决方法 |
|----------|----------|----------|
| `Error: open failed` | 调试器未连接 | 检查 USB 连接 |
| `Error: target not found` | 目标芯片未响应 | 检查 SWD 连接、BOOT0 设置 |
| `Error: executable not found` | ELF 文件不存在 | 先编译项目 |
| `Error: program not halted` | 程序正在运行 | 按复位按钮后重试 |

### 程序异常行为

#### 硬件故障（Hard Fault）

当程序进入 Hard Fault 时：

```c
// 在 stm32f1xx_it.c 中添加
void HardFault_Handler(void) {
    printf("Hard Fault!\r\n");
    printf("SCB->HFSR = 0x%08lX\r\n", SCB->HFSR);
    printf("SCB->CFSR = 0x%08lX\r\n", SCB->CFSR);
    while (1);
}
```

#### 常见故障原因

| 故障类型 | 可能原因 | 排查方法 |
|----------|----------|----------|
| **栈溢出** | 递归太深、大数组 | 减小栈使用、增大栈空间 |
| **空指针** | 未初始化指针 | 检查指针使用前是否初始化 |
| **数组越界** | 索引超出范围 | 检查数组边界条件 |
| **时钟错误** | 外设时钟未使能 | 检查 RCC 时钟配置 |

### 调试器连接问题

#### OpenOCD 连接测试

```powershell
# 手动启动 OpenOCD 测试连接
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg
```

> **注意**：如果 openocd 不在当前目录的 PATH 中，需要使用完整路径：
> ```powershell
> C:\Tools\openocd\bin\openocd.exe -f interface/stlink.cfg -f target/stm32f1x.cfg
> ```

成功输出示例：
```
Open On-Chip Debugger 0.11.0
...
Info : clock speed 1000 kHz
Info : STLINK V2J14S0 (API v2) VID:PID 0483:3748
Info : Target voltage: 3.2
Info : stm32f1x.cpu: hardware has 6 breakpoints, 4 watchpoints
Info : starting gdb server for stm32f1x.cpu on 3333
```

#### Windows 驱动问题排查

在 Windows 上，ST-Link 调试器需要正确的 USB 驱动才能被 OpenOCD 识别。

**步骤一：检查设备管理器中的驱动状态**

1. 右键点击 "开始" 菜单，选择 "设备管理器"
2. 查看 "通用串行总线控制器" 或 "STMicroelectronics" 节点
3. 正常状态应显示 "STMicroelectronics STLink dongle" 或类似名称
4. 如果显示黄色感叹号或 "未知设备"，说明驱动有问题

**步骤二：安装 ST-Link 官方驱动**

1. 访问 [STSW-LINK009 页面](https://www.st.com/en/development-tools/stsw-link009.html)
2. 下载并安装 ST-Link USB 驱动
3. 安装完成后，重新插拔 ST-Link USB
4. 在设备管理器中确认设备正常识别

**步骤三：克隆板使用 Zadig（如需要）**

部分 ST-Link 克隆板使用不同的 USB 芯片，可能需要使用 [Zadig](https://zadig.akeo.ie/) 安装 WinUSB 驱动：

1. 下载并运行 Zadig
2. 在 "Options" 菜单中勾选 "List All Devices"
3. 从下拉列表中选择 ST-Link 设备
4. 将驱动设置为 **WinUSB**
5. 点击 "Replace Driver" 或 "Install Driver"
6. 完成后重新插拔 ST-Link

> **警告**：使用 Zadig 替换驱动后，ST-Link 官方工具（如 ST-Link Utility）可能无法识别该设备。如需恢复，请重新安装 STSW-LINK009 驱动。

---

## 常见问题

### Q1: 调试时程序不暂停在断点？

**可能原因**：
- 断点设置在优化掉的代码
- 编译优化级别过高

**解决方法**：
```cmake
# 在 CMakeLists.txt 中降低优化级别
add_compile_options(-O0 -g3)  # 无优化，最大调试信息
```

### Q2: 变量值显示不正确？

**可能原因**：
- 编译器优化导致变量被优化掉
- 变量作用域已结束

**解决方法**：
```c
// 使用 volatile 防止优化
volatile uint32_t debug_counter;

// 或在编译时禁用优化
// add_compile_options(-O0)
```

### Q3: 无法进入 HAL 库函数？

**解决方法**：
1. 确保 HAL 库编译时包含调试信息
2. 在 `CMakeLists.txt` 中添加：
   ```cmake
   add_compile_options(-g3)  # 最大调试信息
   ```

### Q4: 调试速度很慢？

**可能原因**：
- SWD 时钟频率过低
- 断点数量过多

**解决方法**：
```json
// 在 launch.json 中提高时钟频率
"swdClock": 4000,  // 4 MHz
```

> **Windows 路径注意**：确保 launch.json 中所有路径使用正斜杠（`/`）或 `${workspaceFolder}` 变量，避免使用反斜杠。

### Q5: 如何查看汇编代码？

在调试暂停时：
1. 右键点击代码编辑器
2. 选择 "Open Disassembly View"
3. 查看对应的汇编指令

### Q6: 如何调试中断？

```c
// 在中断处理函数中设置断点
void EXTI0_IRQHandler(void) {
    // 在这里设置断点
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0)) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
        // 中断处理代码
    }
}
```

---

## 进阶主题

### RTOS 调试

如果使用 FreeRTOS，需要配置：

```json
// launch.json 添加
"rtos": "FreeRTOS"
```

### 多核调试

对于多核芯片（如 STM32H7），需要配置多个调试目标：

```json
{
    "configurations": [
        {
            "name": "Cortex-M7",
            "type": "cortex-debug",
            "cpu": "Cortex-M7"
        },
        {
            "name": "Cortex-M4",
            "type": "cortex-debug",
            "cpu": "Cortex-M4"
        }
    ]
}
```

### 半主机模式（Semihosting）

使用半主机模式通过调试器输出 printf：

```c
// 初始化代码
extern void initialise_monitor_handles(void);
initialise_monitor_handles();

// printf 会通过调试器输出
printf("Debug message\r\n");
```

---

## 参考资料

- [Cortex-Debug 扩展文档](https://github.com/Marus/cortex-debug)
- [OpenOCD 官方文档](https://openocd.org/documentation/)
- [ARM Cortex-M3 技术参考手册](https://developer.arm.com/documentation/ddi0337/latest/)
- [STM32F103 数据手册](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)

---

## 总结

本章介绍了 STM32F103C8T6 项目的调试方法，包括：

| 内容 | 要点 |
|------|------|
| **环境配置** | Cortex-Debug 扩展、OpenOCD 手动安装与 PATH 配置 |
| **已知问题** | launch.json 可执行文件路径错误、Cortex-Debug 路径配置 |
| **断点调试** | 设置断点、单步执行、继续运行 |
| **变量监视** | 局部变量、全局变量、寄存器查看 |
| **调试技巧** | LED 调试、printf 串口调试（PuTTY/Tera Term）、断言检查 |
| **故障排查** | 连接问题、Hard Fault、Windows 驱动问题 |

---

**上一章**：[编译与烧录](./04_build_and_flash)

**下一章**：[STM32基础概念](./06_stm32_basics)
