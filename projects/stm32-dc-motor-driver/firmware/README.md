# B 题直流电机驱动基础版

目标 MCU 为 STM32F103C8T6。工程使用 8 MHz 内部 HSI、64 KB Flash 和 20 KB RAM，不依赖外部晶振或 CubeMX。用四个独立按键完成启停、分步换向和三档 PWM。

## 接线

| 用途 | MCU | 接法 |
|---|---|---|
| 共地 | GND | 驱动板 U4-1 |
| 两路共同使能 | PB12 | 驱动板 U4-2，低电平关断 |
| A 半桥 PWM | PA6 | 驱动板 U4-3 |
| B 半桥 PWM | PA7 | 驱动板 U4-4 |
| 启动 START | PA0 | 常开按键，另一端接 GND |
| 停止 STOP | PA1 | 常开按键，另一端接 GND |
| 方向 DIR | PA2 | 常开按键，另一端接 GND |
| 档位 SPEED | PA3 | 常开按键，另一端接 GND |

按键输入使用内部上拉。接线按开发板丝印确认 PA0–PA3。U7 的 5 V 接开发板经确认的 5 V 电源输入，U7 的 GND 接开发板 GND。实际板型和 5 V 入口仍需核对。U4 上的 GND 与 U7 的 GND 均为同一公共地。

## 按键和控制顺序

- 上电或复位后默认停止、A 方向和低档。按住 START 再复位时，要先松开再按才会启动。
- START 启动。先让两边下管导通约 10 ms 为自举电容充电，再让其中一侧进行约 20 kHz 的 PWM。另一侧的 IN 保持低电平。
- STOP 立即请求关闭两片驱动，优先于其他按键。此时电机滑行。程序设约 0.8 秒重启等待期，**操作者仍须目视等轴完全停稳**。停稳后短按 DIR，再短按 START。运行中按 DIR 不会改变方向。
- SPEED 短按循环切换低、中、高三档，初始占空比为 30%、50%、75%。长按只触发一次。占空比在 `control.c` 的 `duty_table` 调整。
- 电机实际正反转取决于接线和观察方向。按题目要求，以实际转动方向在轴端标注正转。

## 构建

安装带有 `arm-none-eabi-gcc`、`arm-none-eabi-objcopy` 和 `arm-none-eabi-size` 的 Arm GNU Toolchain。打开工程目录中的 PowerShell，运行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
```

也可以给脚本传 `-ToolchainBin '工具链的 bin 绝对路径'`。构建产物为 `build/motor.hex`、`build/motor.bin` 和 `build/motor.elf`。HEX 含 Flash 地址，BIN 对应起始地址 `0x08000000`。这三个文件分别是下载镜像、原始镜像和带调试信息的程序。调试时先断开电机。工程构建脚本只生成文件。

若有本机 C 编译器，可运行控制逻辑测试：

```text
cc -std=c11 -Wall -Wextra -Werror control.c control_test.c -o control_test
```

执行生成的 `control_test`（Windows 对应 `control_test.exe`）。测试覆盖复位时按住启动键、按键消抖、三档循环、停止优先、等待期间拒绝换向以及预充期间停止。

交付版本已用 Arm GNU Toolchain 14.3.Rel1 编译，生成 967 字节的 Flash 镜像。已在 Cortex-M3 指令模拟器里运行链接后的镜像，用模拟按键和寄存器验证复位、预充、启停、换向、三档切换及异常入口拉低使能。电机、驱动波形和测速成绩需要上板实测。

## 上板验收

用 ST-LINK 下载时，先断开电机。核对目标板电源和 ST-LINK 接口定义后，在 STM32CubeProgrammer 选择 SWD，打开 `motor.hex`，下载并校验，再复位测试。开发板正常从 Flash 启动时应把 BOOT0 设为低。

| 题目要求 | 验证动作 |
|---|---|
| 上电和复位保持静止 | 先断开电机、限流上电，确认 PB12 在上电和复位后为低。之后接电机重复测试。 |
| 启停 | 连续两轮 START、STOP。停止后保持静止，直到再次按 START。 |
| 双向运行 | 中档各运行正反转 10 秒。STOP 后确认轴停稳，按 DIR，再按 START。 |
| 三档调速 | 同方向各档连续空载运行 10 秒。换档后等 5 秒，再每隔 2 秒取一次外部测速读数，共取三次并算均值。相邻两档的后者均值至少是前者的 1.15 倍。 |

30% / 50% / 75% 是联调初值。若低档无法从静止起转、档间差达不到 15%，用实测数据重新标定。高档测试同时检查自举 `VB−VS` 与高侧 `VGS`，测量高侧使用差分探头。当前工程覆盖 B 题基础要求，编码器测速和闭环控速留待编码器型号、供电和每转计数确认后接入。

参考资料为 [STM32F103C8 数据手册](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf) 和 [IR2104 数据手册](https://www.infineon.com/assets/row/public/documents/60/49/infineon-ir2104-ds-en.pdf?fileId=5546d462533600a4015355c7c1c31671)。
