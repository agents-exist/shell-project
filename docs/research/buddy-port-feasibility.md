# Feasibility: Porting claude-desktop-buddy to M5StickS3

> Research for Issue #17

## TL;DR

**可行，工作量约 2-3 天。** 主要改动是 HAL 层适配（AXP192 → 内置电源管理、LCD 驱动差异、按钮 API），核心逻辑（BLE 协议、动画系统、宠物状态机）基本不需要动。

## Buddy 固件架构分析

### 代码结构
```
src/
├── main.cpp          — 主循环、状态机、UI 渲染、菜单
├── ble_bridge.cpp/h  — BLE Nordic UART 通信
├── buddy.cpp/h       — 宠物动画引擎（ASCII 模式）
├── buddy_common.h    — 公共宠物类型定义
├── character.cpp/h   — GIF 角色渲染引擎
├── data.h            — NVS 持久化（设置、物种选择）
├── stats.h           — 统计显示页面
├── xfer.h            — BLE 文件传输（角色包推送）
└── buddies/          — 18 种宠物的 ASCII 动画定义
    ├── axolotl.cpp, blob.cpp, cactus.cpp ...
    └── turtle.cpp
```

### 依赖项（platformio.ini）
```ini
lib_deps =
    m5stack/M5StickCPlus     ← 这个要换
    bitbank2/AnimatedGIF     ← 通用，不用动
    bblanchon/ArduinoJson    ← 通用，不用动
```

### M5 API 使用统计

| API | 调用次数 | 用途 | StickS3 对应 |
|-----|---------|------|-------------|
| `M5.Lcd.*` | 20 | 屏幕绘制 | `M5.Display.*`（M5Unified） |
| `M5.Axp.*` | 15 | 电源管理、亮度、电压 | 内置 PMU API 或直接 GPIO |
| `M5.Imu.*` | 4 | 加速度计（摇晃检测） | `M5.Imu.*`（M5Unified 兼容） |
| `M5.Rtc.*` | 4 | 时钟 | `M5.Rtc.*`（M5Unified 兼容） |
| `M5.BtnA/B` | 6 | 按钮输入 | `M5.BtnA/B/C`（S3 有 3 个按钮） |
| `M5.Beep` | 3 | 蜂鸣器 | `M5.Speaker`（S3 有 1W 喇叭，更强） |

## 需要修改的部分

### 1. 电源管理 (M5.Axp) — 工作量：中

StickC Plus 用 AXP192 电源管理芯片，StickS3 **没有 AXP192**。

| StickC Plus API | 用途 | StickS3 替代方案 |
|----------------|------|----------------|
| `M5.Axp.ScreenBreath(val)` | 屏幕亮度 | `M5.Display.setBrightness(val)` |
| `M5.Axp.SetLDO2(bool)` | 屏幕开关 | `M5.Display.sleep()` / `wakeup()` |
| `M5.Axp.GetBatVoltage()` | 电池电压 | `M5.Power.getBatteryVoltage()` (M5Unified) |
| `M5.Axp.GetBatCurrent()` | 电池电流 | 可能不可用，改为估算或删除 |
| `M5.Axp.GetBtnPress()` | 电源键检测 | GPIO 检测 |
| `M5.Axp.PowerOff()` | 关机 | `M5.Power.powerOff()` |
| `M5.Axp.GetTempInAXP192()` | 芯片温度 | ESP32-S3 内部温度传感器 |
| `M5.Axp.GetVBusVoltage()` | USB 电压 | 可能不可用，非关键功能 |

### 2. 显示驱动 (M5.Lcd) — 工作量：低

两者屏幕分辨率相同（135×240），都是 TFT LCD。M5Unified 统一了 API。

| StickC Plus | StickS3 (M5Unified) | 备注 |
|------------|-------------------|------|
| `M5.Lcd.fillScreen()` | `M5.Display.fillScreen()` | 改名 |
| `M5.Lcd.drawString()` | `M5.Display.drawString()` | 改名 |
| `M5.Lcd.fillRect()` | `M5.Display.fillRect()` | 改名 |
| `M5.Lcd.setRotation()` | `M5.Display.setRotation()` | 改名 |
| `TFT_eSprite(&M5.Lcd)` | `TFT_eSprite(&M5.Display)` | Sprite 初始化改名 |

**基本就是全局替换 `M5.Lcd` → `M5.Display`。**

### 3. IMU — 工作量：极低

StickC Plus 用 MPU6886，StickS3 用 BMI270。但 M5Unified 统一了 API：
- `M5.Imu.getAccelData(&ax, &ay, &az)` → 两边一样 ✅
- 只需确认坐标轴方向一致（可能需要翻转某个轴）

### 4. 按钮 — 工作量：极低

StickC Plus 有 2 按钮（A + B），StickS3 有 **3 按钮**（A + B + C）。

M5Unified API 兼容：`M5.BtnA.wasPressed()` 等调用不变。多出的 BtnC 可以作为额外功能键。

### 5. 蜂鸣器/喇叭 — 工作量：低

StickC Plus 用被动蜂鸣器（`M5.Beep.tone()`），StickS3 有 **1W I2S 喇叭**（更强）。

M5Unified: `M5.Speaker.tone(freq, duration)` 替代 `M5.Beep.tone(freq, duration)`。

### 6. BLE — 工作量：无

BLE 代码用的是标准 ESP32 BLE 库（NimBLE），不依赖 M5 库。StickS3 的 ESP32-S3 支持 BLE 5.0，**完全兼容**，无需修改。

### 7. LED — 工作量：极低

`LED_PIN = 10`（红色 LED）。StickS3 的 LED 引脚不同，需要查硬件手册改一下引脚号。

## 不需要修改的部分

- ✅ **BLE 协议**（ble_bridge.cpp）— 纯 ESP32 BLE 代码
- ✅ **18 种宠物动画**（buddies/*.cpp）— 纯绘图逻辑，不碰硬件
- ✅ **GIF 渲染引擎**（character.cpp）— AnimatedGIF 库 + Sprite
- ✅ **状态机**（main.cpp 核心逻辑）— PersonaState/TamaState
- ✅ **NVS 持久化**（data.h）— ESP32 通用 API
- ✅ **BLE 文件传输**（xfer.h）— 纯 BLE 逻辑
- ✅ **JSON 解析**（ArduinoJson）— 通用库

## 移植步骤

1. **platformio.ini**: `m5stack/M5StickCPlus` → `m5stack/M5Unified` + `board = m5stick-s3`
2. **全局替换**: `#include <M5StickCPlus.h>` → `#include <M5Unified.h>`
3. **全局替换**: `M5.Lcd` → `M5.Display`
4. **AXP192 适配**: 8 个调用改为 M5Unified Power/Display API
5. **蜂鸣器**: `M5.Beep` → `M5.Speaker`
6. **LED 引脚**: 查 StickS3 原理图，改 LED_PIN
7. **编译测试**: 修复剩余编译错误
8. **IMU 验证**: 确认摇晃检测方向正确

## 预估工作量

| 任务 | 时间 |
|-----|------|
| platformio.ini + include 替换 | 0.5h |
| Lcd → Display 全局替换 | 0.5h |
| AXP192 → M5Unified Power 适配 | 2-3h |
| 蜂鸣器 + LED + 按钮 | 1h |
| 编译调试 | 2-3h |
| IMU 方向验证 | 0.5h |
| **总计** | **~7-8h（约 2 天）** |

## 风险

| 风险 | 影响 | 概率 | 缓解 |
|------|------|------|------|
| AXP192 某些功能在 S3 上无法替代 | 功能缺失（电流读取等） | 中 | 非关键功能，可删除或用占位值 |
| 屏幕控制器不同导致显示异常 | UI 错乱 | 低 | M5Unified 已抽象，同分辨率 |
| Flash 分区表不兼容 | 刷机失败 | 低 | 重新配置 partitions.csv |
| BLE 5.0 vs 4.2 行为差异 | 连接问题 | 极低 | BLE 5.0 向下兼容 |

## 结论

**移植完全可行。** 代码架构良好——硬件依赖集中在少数 M5 API 调用上，核心逻辑（BLE 通信、宠物动画、状态机）完全硬件无关。M5Unified 库的设计初衷就是跨 M5 设备兼容，大部分改动是 API 名称替换。

**建议：** Luna 买 StickS3 后，我们直接 fork buddy 仓库开始移植，作为 Shell Project 的第一个可运行 demo。之后在这个基础上加入 OpenClaw 连接、自定义表情等 Shell 特有功能。
