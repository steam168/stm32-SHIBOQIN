# STM32 简易数字示波器

> 基于 **STM32F103C8T6**（Cortex-M3 @ 72MHz，20KB SRAM / 64KB Flash）的便携式数字存储示波器。
> Keil MDK 5 + STM32F10x 标准外设库（SPL）手写实现，**无 HAL、无 CubeMX、无第三方图形库**。
> 大一暑期独立完成：约 13 天 × 6h。

*An entry-level digital storage oscilloscope built from scratch on STM32F103C8T6.
Written entirely with the STM32F10x Standard Peripheral Library — every driver
(TFT, ADC+DMA, input capture, USART DMA) hand-crafted, no HAL, no CubeMX, no vendor graphics library.*

---

## 1. 功能特性

- **波形采集**：ADC1 + DMA 循环缓冲 + TIM3 TRGO 硬件定时触发，5 档采样率运行时切换
- **三种触发**：自动电平触发 / 上升沿单次 / 下降沿单次（软件搜索式）
- **参数测量**：Vpp（峰峰值）、频率（TIM4 输入捕获测周法，1MHz 计数）
- **显示**：1.8" TFT（ST7735，128×160）波形 + OLED（SSD1306）参数面板
- **人机交互**：3 按键 + 旋钮（ADC2 采样）调节触发电平 / 光标 / 模式 / 采样率档位
- **上位机**：USART1 @ 921600 + DMA，256 点/帧文本流，VOFA+ 实时绘制与回放
- **板载自检**：TIM2 PWM 输出 1kHz 方波，不接信号也能验证整条链路

## 2. 性能指标

| 项目 | 参数 | 备注 |
|---|---|---|
| 采样率 | 13.9 Sa/s ~ 10 kSa/s，5 档 | `f = 72MHz / [(ARR+1)(PSC+1)]`，ARR=720 |
| ADC 分辨率 | 12 bit（4096 级） | 屏幕量化到 0~120 行 |
| 输入电压范围 | 0 ~ 3.3 V | ⚠ 无模拟前端，超压即烧片，见 §7 改进路线 |
| 水平点数 | 128 点/屏 | 采样缓冲 256 点，半屏交替 |
| 屏幕刷新 | SPI1 @ 36MHz + DMA | 8 行块生成，双行缓冲 |
| 频率测量 | 测周法，1MHz 计数时钟 | `1000000/(捕获值+1)` |
| 上位机 | 921600 baud，256 点/帧 | VOFA+ 兼容文本格式 |
| SRAM 占用 | ≈ 10 KB / 20 KB | 见 §4.2 无帧缓冲架构 |

## 3. 硬件资源分配

| 外设 | 引脚 / 通道 | 用途 |
|---|---|---|
| ADC1 + DMA1_CH1 | PB0 (CH8) | 波形采集，TIM3 TRGO 触发，循环模式 |
| ADC2 | PA1 (CH1) | 旋钮电压（触发电平 / 光标），软件触发 |
| TIM3 | 内部 | 采样时基，PSC 运行时改写 |
| TIM2 CH1 | PA0 | PWM 自测信号（1kHz 校验方波） |
| TIM4 CH1 | PB6 | 输入捕获测频，PSC=72 → 1MHz 计数 |
| SPI1 + DMA1_CH3 | PA5/6/7 | TFT 驱动，2 分频 = 36MHz |
| GPIO | PA2(DC) PA4(CS) PB1(RES) PB10(BLK) | TFT 控制线 |
| USART1 TX + DMA1_CH4 | PA9 | 上位机数据流 |
| 按键 ×3 | PB13 / PB14 / PB15 | 冻结、光标切换、模式切换 |
| OLED | SSD1306 | 参数显示（Vpp / f / PSC / mode） |

中断优先级（抢占:子）：`DMA1_CH1(ADC)=0:0 > DMA1_CH3(SPI)=1:1 > DMA1_CH4(USART)=2:2 > TIM2=3:3`

## 4. 核心实现

### 4.1 DMA 乒乓缓冲（消除波形撕裂）

DMA 循环搬运 256 点，半传输中断（HT）置 `conut=1`、全传输中断（TC）置 `conut=2`。
主循环只处理已就绪的那一半，DMA 同时在写另一半 → 采集与绘制流水并行。

### 4.2 无帧缓冲分块刷屏（20KB SRAM 的破局）

整屏 RGB565 帧缓冲需 128×160×2 = **40KB > 20KB SRAM**，装不下。
方案：每次只生成 8 行的行缓冲（8×128×2 = 2KB），双缓冲交替，SPI DMA 推送，全屏 16 块。
绘制第 N 块时 DMA 正在发第 N-1 块，CPU 生成与 SPI 传输并行。

### 4.3 跨行区间填充画线（消除波形断点）

逐点精确匹配 `Arrdy[x]==y` 在陡峭边沿会画成散点。改为检测**跨越关系**：
某行 y 落在相邻两点之间即点亮，保证边沿连续。

### 4.4 其他

- **5 点去极值平均滤波**（`LCD_Bo`）：前后 5 点去掉最大最小，中间 3 点平均，O(n) 计算量
- **采样率运行时切换**：直接写 `TIM3->PSC`，利用预分频影子寄存器，下一个更新事件生效

## 5. 调试实录（本项目最值钱的部分）

开发全程记录 **23 条问题 + 9 次实践迭代**，每条均为「现象 → 根因 → 方案」格式，
原始素材为手写笔记，已整理为 [`docs/debug-log.md`](docs/debug-log.md)。摘录三条：

| 现象 | 根因 | 方案 |
|---|---|---|
| DMA 传输偶发失败 | 函数内定义大数组压栈，函数返回即释放 | 大数组加 `static` 提升为全局 |
| 中断里改了值主循环看不见 | 共享变量未加 `volatile`，被编译器优化 | 中断与主循环共享变量一律 `volatile` |
| DMA 配置莫名异常 | 复用 `InitTypeDef` 反复 `Init`，未配置成员被重置 | 结构体声明一次，只改成员，Init 只做一次 |

另附 19 条代码级缺陷清单（按 P0/P1/P2 分级，含越界写、整数除法截断、ADC 超频等），
见 [`docs/datasheet.md`](docs/datasheet.md) §7。

## 6. 目录结构

```
oscilloscope/
├── Project.uvprojx          # Keil 5 工程（双击打开）
├── Hardware/                # 外设驱动层（全部手写）
│   ├── ADC.c/.h             #   ADC1/ADC2 + DMA 采集、乒乓中断、采样率档位
│   ├── LCD.c/.h             #   ST7735 驱动、滤波、波形绘制、Vpp
│   ├── SPI.c/.h             #   SPI1 + DMA1_CH3 封装
│   ├── USART.c/.h           #   串口 + DMA1_CH4 + 波形文本化
│   ├── TIM.c/.h             #   PWM 输出 + 输入捕获测频
│   ├── Key.c/.h             #   按键扫描
│   └── OLED.c/.h            #   SSD1306 驱动
├── User/                    # 应用层
│   └── main.c               #   5 模式状态机主循环
├── System/                  # Delay 延时
├── Library/                 # STM32F10x 标准外设库（ST 官方固件）
├── Start/                   # CMSIS + 启动文件（ST 官方固件）
└── docs/
    ├── datasheet.md         # 项目数据手册（指标/实现/问题清单/改进路线）
    └── debug-log.md         # 调试实录：23 条问题 + 9 次实践迭代
```

## 7. 已知技术债与改进路线

诚实清单，按优先级：

- **P0 模拟前端**（最高优先级）：运放电压跟随 + 电阻衰减网络 → 量程 ±12V；
  钳位二极管 + 限流电阻 → 过压保护。当前直接接 GPIO，接错一根线就烧片
- **P0 修 ADC 时钟**：未调用 `RCC_ADCCLKConfig`，ADC 运行在 36MHz（F103 上限 14MHz），
  精度受损；应配 PCLK2/6 = 12MHz
- **P0 修数组越界 / Vpp 整数除法截断**：见数据手册 §7
- **P1 采样率提升 10~20 倍**：ARR 改小 + 采样时间 1.5 周期 → 目标 100~200 kSa/s
- **P1 按键改非阻塞**、**时基刻度联动**、**FFT 频谱**（CMSIS-DSP）
- **P2 FreeRTOS 重构**、**自画 PCB**、**FPGA + 高速 ADC 采集前端**

## 8. 编译与烧录

1. Keil MDK 5 打开 `Project.uvprojx`
2. `Project → Build Target`（F7）
3. `Flash → Download`（F8）
4. 首次使用在 `Options for Target → Debug` 选择你的 ST-Link / CMSIS-DAP，
   `Utilities` 勾选 *Use Debug Driver*，Flash 算法选 `STM32F10x_128`

**安全警告**：输入信号务必限制在 **0 ~ 3.3V**。本版本无过压保护，超压会永久损坏芯片。

## 9. 说明

- `Library/` 与 `Start/` 为 STMicroelectronics 官方固件，非本项目原创，因其许可证允许再分发而随仓库提供，保证开箱可编译
- 代码保留了大一时期的原始状态（含已知缺陷与技术债），作为学习过程的真实记录；问题清单见文档，不在本仓库内"翻新"
