# MCU/SBC Comparison for Agent Embodiment

> Research for Issue #1: Minimum viable hardware for agent embodiment
>
> MVP requirements: **Camera + LED + optional speaker**, WiFi connectivity to cloud agent

## Platform Comparison

| Platform | CPU | RAM | Flash | WiFi | BLE | Camera Support | GPIO | Approx. Price | Power |
|---|---|---|---|---|---|---|---|---|---|
| **ESP32 (WROOM-32)** | Xtensa LX6 dual-core 240MHz | 520KB SRAM | 4MB | ✅ 802.11 b/g/n | ✅ 4.2 | ❌ No native interface | 34 | ~$3–5 | ~160mA active WiFi |
| **ESP32-CAM (AI-Thinker)** | ESP32-S (single-core 240MHz) | 520KB + 4MB PSRAM | 4MB | ✅ | ✅ 4.2 | ✅ OV2640 built-in | ~10 usable (shared w/ cam) | ~$5–8 | ~310mA w/ camera |
| **ESP32-S3** | Xtensa LX7 dual-core 240MHz | 512KB + up to 8MB PSRAM | 8–16MB | ✅ | ✅ 5.0 | ✅ DVP/USB camera | 45 | ~$4–7 (module) | ~170mA active WiFi |
| **ESP32-C3** | RISC-V single-core 160MHz | 400KB SRAM | 4MB | ✅ | ✅ 5.0 | ❌ No camera interface | 22 | ~$2–4 | ~130mA active WiFi |
| **Raspberry Pi Zero 2W** | BCM2710A1 quad-core ARM Cortex-A53 1GHz | 512MB | microSD | ✅ | ✅ 4.2 | ✅ CSI (Pi Camera) | 40 | ~$15 (often $20+ due to supply) | ~400mA idle, ~1.2A peak |
| **Raspberry Pi Pico W** | RP2040 dual-core ARM Cortex-M0+ 133MHz | 264KB | 2MB | ✅ | ✅ (Pico 2W) | ❌ No native camera | 26 | ~$6 | ~50mA active WiFi |
| **Arduino Nano 33 IoT** | SAMD21 ARM Cortex-M0+ 48MHz | 256KB | 1MB | ✅ (NINA-W10) | ✅ | ❌ No camera interface | 14 analog + digital | ~$18–25 | ~50mA active WiFi |
| **Seeed XIAO ESP32S3 Sense** | ESP32-S3 dual-core 240MHz | 8MB PSRAM | 8MB | ✅ | ✅ 5.0 | ✅ OV2640 built-in + mic | 11 | **$13.90** (Seeed official) | ~180mA w/ camera |

## Detailed Analysis

### ESP32-CAM (AI-Thinker) — ~$5–8

**Pros:** Cheapest option with built-in camera (OV2640). Massive community — hundreds of tutorials for camera streaming, face detection, etc. Can drive LEDs and a small I2S speaker with careful GPIO management. MicroSD slot for local storage.

**Cons:** Very limited usable GPIOs after camera occupies most pins (~10 free, some shared). No USB connector — needs external FTDI programmer for uploads. Based on older ESP32-S chip. No built-in mic. Debugging is painful.

**Verdict:** Best bang-for-buck for camera MVP. GPIO limitation is real but manageable for LED + speaker.

### ESP32-S3 (DevKit) — ~$4–7

**Pros:** Modern chip with native USB, more GPIOs (45), BLE 5.0, up to 8MB PSRAM. Can attach external DVP or USB camera. Better AI inference capability (vector instructions). Active Espressif support.

**Cons:** Camera not included — need to buy separately ($2–5 for OV2640 module). More wiring/PCB work. Module alone isn't a complete solution.

**Verdict:** Best for custom builds where you want to choose your own camera and have plenty of GPIO headroom.

### Seeed XIAO ESP32S3 Sense — $13.90

**Pros:** Tiny form factor (21×17.5mm). Built-in OV2640 camera AND digital microphone. 8MB PSRAM + 8MB Flash. USB-C. SD card slot. Same ESP32-S3 chip. Excellent for compact enclosures.

**Cons:** Only 11 GPIOs — tight for LED matrix + speaker + servo. More expensive than bare ESP32-CAM. Smaller community than generic ESP32.

**Verdict:** Best for tiny builds. Camera + mic built-in is unique at this size. Premium justified if form factor matters.

### Raspberry Pi Zero 2W — ~$15–20

**Pros:** Full Linux (Python, Node.js, etc.). CSI camera with excellent Pi Camera ecosystem. Can run complex software, TTS, local voice processing. 512MB RAM is generous. Mature ecosystem.

**Cons:** Higher power draw (~1W idle). Needs SD card, power supply. Boot time ~20s. Overkill for LED + camera if agent logic runs in cloud. Supply issues persist (often out of stock or marked up). Not real-time GPIO.

**Verdict:** Best if you need on-device intelligence (local TTS, image processing). Overkill if the shell just streams to a cloud agent.

### ESP32-C3 — ~$2–4

**Pros:** Cheapest option. RISC-V. Low power. BLE 5.0. Good for LED-only or sensor-only shells.

**Cons:** No camera interface at all. Single core 160MHz. Not suitable for our MVP.

**Verdict:** ❌ Eliminated — no camera support.

### Raspberry Pi Pico W — ~$6

**Pros:** Very cheap. Low power. Good GPIO. MicroPython support.

**Cons:** No camera interface. 264KB RAM too small for image work. No BLE on base model (Pico 2W adds it).

**Verdict:** ❌ Eliminated — no camera support.

### Arduino Nano 33 IoT — ~$18–25

**Pros:** Arduino ecosystem. Built-in IMU. Crypto chip. Easy WiFi.

**Cons:** No camera interface. Weak CPU (48MHz). Expensive for what it offers. Only 1MB flash.

**Verdict:** ❌ Eliminated — no camera, overpriced for our use case.

## Availability (2026-04)

| Platform | AliExpress | Amazon | Official Store | Taobao |
|---|---|---|---|---|
| ESP32-CAM | ✅ $5–8 | ✅ $8–12 | N/A (3rd party) | ✅ ¥15–30 |
| ESP32-S3 DevKit | ✅ $4–7 | ✅ $8–15 | Espressif partners | ✅ ¥20–40 |
| XIAO ESP32S3 Sense | ✅ $13.90 | ✅ ~$15 | Seeed $13.90 | ✅ ¥80–100 |
| Pi Zero 2W | ⚠️ Markup | ⚠️ $20–30 | $15 (often OOS) | ⚠️ ¥120–180 |

## Recommendation

### 🏆 Primary: ESP32-CAM (AI-Thinker) — MVP Build

**Why:** Cheapest complete camera solution (~$6). Proven, massive community, tons of examples for exactly our use case (camera streaming over WiFi). Good enough GPIO for LED strip + I2S speaker. The limited GPIO is a constraint we can design around.

**MVP hardware list:**
- ESP32-CAM module: ~$6
- FTDI programmer (one-time): ~$2
- WS2812B LED ring/strip: ~$2
- Small I2S speaker (MAX98357): ~$3
- Total: **~$13**

### 🥈 Upgrade Path: Seeed XIAO ESP32S3 Sense

**When to upgrade:** If we need a smaller form factor, built-in mic for voice input, or run out of GPIOs on the ESP32-CAM. The $14 price is justified by integrated camera + mic + tiny size.

### 🥉 Future: Raspberry Pi Zero 2W

**When to consider:** If we want on-device AI (local wake word, edge inference, TTS) rather than pure cloud relay. Significant jump in complexity and power requirements.

## Decision Matrix

| Criterion (weight) | ESP32-CAM | XIAO ESP32S3 | ESP32-S3 DevKit | Pi Zero 2W |
|---|---|---|---|---|
| Cost (25%) | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐ |
| Camera ready (25%) | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ |
| GPIO for peripherals (15%) | ⭐⭐ | ⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ |
| Dev ecosystem (15%) | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| Power efficiency (10%) | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ | ⭐ |
| Form factor (10%) | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ |
| **Weighted Total** | **4.0** | **3.6** | **3.6** | **3.1** |

**→ Start with ESP32-CAM. Graduate to XIAO ESP32S3 Sense when form factor or mic matters.**
