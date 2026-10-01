# Seeed XIAO RP2350（感知节点主板）

[English](README.md) | [中文文档](README.zh.md)

[![Build Firmware](https://github.com/Mi-Bee-Studio/seeed-xiao-rp2350/actions/workflows/build.yml/badge.svg)](https://github.com/Mi-Bee-Studio/seeed-xiao-rp2350/actions/workflows/build.yml)

主板目录规范的一块板。**本仓库按"主板为根"规范组织**：

```
seeed-xiao-rp2350/
├── README.md          # 本文件：这块板的一切硬件信息
└── <project>/         # 每个用这块板做的项目一个目录（按能力命名）
    ├── CMakeLists.txt / main/ / sdkconfig.defaults / main/idf_component.yml
    └── README.md      # 项目说明 + 编译/烧录命令
```

规范要点：

- **主板目录名** = 板子名（kebab-case），根 README 只写硬件、不写业务；
- **项目目录独立可编译**：自带完整构建三件套，`cd <project> && cmake -B build && cmake --build build` 即出固件；
- 项目间不共享代码；需要共性时先拷贝，稳定后再考虑抽组件。

### 固件基线规范（全家桶强制）

1. **看门狗：必须启用**。RP2350 启用片上硬件看门狗（`watchdog_enable`）并在主循环喂狗；
2. **Web/API 固件升级（OTA）：硬件允许则必须提供**。本板**无任何无线电**，Web/API OTA 属硬件不允许，规范豁免——升级走 UF2（BOOTSEL / picotool / 1200bps 软重启）。

| 项目 | 看门狗 | Web/API OTA |
|------|--------|-------------|
| blink | ✅ 硬件看门狗（`watchdog_enable(8000,1)`） | N/A（无网络；UF2/picotool 为升级路径） |

---

## 板子概要

| 项目 | 值 |
|------|-----|
| 芯片 | Raspberry Pi **RP2350A** —— 双核 Arm Cortex-M33 @150MHz（带 FPU，TrustZone/secure boot） |
| Flash | **2MB 外挂 QSPI**（W25Q080 级，专用总线不占 GPIO） |
| SRAM | 520KB（片内，无 PSRAM） |
| 无线 | **无**（纯 USB 设备板；联网需外接伴侣方案） |
| USB | 原生 USB Type-C（固件启用 CDC 才有串口；**BOOTSEL 模式下是 U 盘不是串口**） |
| 板载 LED | **USER LED = GPIO25**（黄色，**低电平点亮**）；**WS2812 RGB = GPIO22**（供电使能 GPIO23） |
| 按键 | BOOT（进 UF2 下载模式，**不是 GPIO**）、RESET（丝印 B） |
| 调试 | 背面 USB-C 上方焊盘组：**CLK(SWCLK) · DIO(SWDIO) · GND · RST · BOOT · 3V3**（openocd/Picoprobe） |
| 尺寸 | 21 × 17.8 mm（标准 XIAO 形制） |
| SDK | 官方 **pico-sdk**（C/C++）；本仓构建 `-DPICO_BOARD=seeed_xiao_rp2350`（官方板级头） |

## 引脚位置图（USB-C 朝上，正面/元件面视角；D 编号 = XIAO 丝印）

```
                 ┌─ USB-C ─┐
       5V ◎┬───┘          ├───┬◎ D0/A0 = GPIO26 (ADC)
      GND ◎│              │   ◎ D1/A1 = GPIO27 (ADC)
      3V3 ◎│ [USER LED]   │   ◎ D2/A2 = GPIO28 (ADC)
     D3 ◎  │  = GPIO25    │   ◎ D4 = GPIO6  (I2C1 SDA)
     D5 ◎  │  [WS2812=22] │   ◎ D5 = GPIO7  (I2C1 SCL)
    D10 ◎  │  [BOOT][RST] │   ◎ D6 = GPIO0  (UART0 TX)
           │   RP2350A    │   ◎ D7 = GPIO1  (UART0 RX)
           └──────────────┘   ◎ D3 = GPIO5  (SPI0 CSn)
            左列（正面）    右列（正面）

  第二排（背面/边缘，D11–D18）：D11=GPIO21(RX1) · D12=GPIO20(TX1) ·
  D13=GPIO17(SCL0) · D14=GPIO16(SDA0) · D15=GPIO11(SPI1_MOSI) ·
  D16=GPIO12(SPI1_MISO) · D17=GPIO10(SPI1_SCK) · D18=GPIO9(SPI1_CSn)
  电池检测：ADC_BAT=GPIO29（1:1 分压，读数×2）· 使能 ADC_BAT_EN=GPIO19
```

要点（引脚映射出处：[Seeed wiki](https://wiki.seeedstudio.com/getting-started-xiao-rp2350/) 与 pico-sdk 官方板级头 `seeed_xiao_rp2350.h`）：

- **左列**自 USB 端向下：`5V · GND · 3V3 · D3=GPIO5(SPI0 CSn) · D5=GPIO7(I2C1 SCL) · D10=GPIO3(SPI0 MOSI)`（D3/D5/D10 在左列，D4/D8/D9 在右列——与多数 XIAO 板的丝印位置一致，以官方图为准）；
- **右列**自 USB 端向下：`D0/A0=GPIO26 · D1/A1=GPIO27 · D2/A2=GPIO28 · D4=GPIO6(SDA1) · D8=GPIO2(SPI0 SCK) · D9=GPIO4(SPI0 MISO) · D6=GPIO0(TX0) · D7=GPIO1(RX0)`；
- RP2040 外设走**全矩阵重映射**（PIO 可合成任意时序），无 ESP 式固定复用表；
- GPIO22=WS2812、GPIO23=WS2812 电源使能、GPIO25=USER LED——被板载器件占用，勿挪用。

## 注意事项

- **下载模式与 ESP 完全不同**：按住 BOOT 插 USB（或运行中按住 BOOT 点按 RESET）进 BOOTSEL，枚举为 `RP2350` U 盘，拖入 UF2 即完成刷写；
- **免按键刷机**：pico-sdk 默认支持**以 1200bps 打开 CDC 串口触发软重启进 BOOTSEL**（本仓 blink 已验证可用）；
- **USER LED 低电平点亮**（active-low），驱动逻辑注意反相；
- GPIO23 控制 WS2812 供电——用 RGB 前先拉高使能；
- 无网络无 PSRAM：联网/大缓冲需求需外接方案或换板。

## 项目索引

| 项目 | 说明 |
|------|------|
| [blink](blink/README.zh.md) | 基线工程/测试固件：USER LED（GPIO25）心跳闪烁 + 硬件看门狗 + USB CDC 心跳日志（**真机已验证**，2026-10-01） |
