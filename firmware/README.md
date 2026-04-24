# Shell Firmware

ESP32-S3 firmware for the M5StickS3, giving AI agents a physical embodiment with a pet companion, BLE communication, and approval UI.

**Ported from [CodeBuddy](https://github.com/anthropics/code-buddy)** (MIT License, Copyright 2026 Anthropic, PBC). The original firmware targets the same M5StickS3 hardware; this port adapts it for the shell-project with rebranded BLE advertising and UI text. The HAL abstraction (`board_compat.h`), animation system, and BLE protocol are preserved as-is.

> For the BLE protocol and JSON payloads, see [REFERENCE.md](REFERENCE.md).

## Build Requirements

- [PlatformIO](https://platformio.org/) (CLI or IDE plugin)
- ESP32-S3 toolchain (installed automatically by PlatformIO)
- M5StickS3 hardware

Dependencies (fetched automatically by PlatformIO):
- `m5stack/M5Unified`
- `m5stack/M5PM1`
- `bitbank2/AnimatedGIF@^2.1.1`
- `bblanchon/ArduinoJson@^7.0.0`

## Flashing to M5StickS3

Build and flash with PlatformIO:

```bash
cd firmware
pio run -t upload
```

For a clean reflash:

```bash
pio run -t erase && pio run -t upload
```

## BLE Pairing

The device advertises as **`Shell-XXXX`** where `XXXX` are the last two bytes of the Bluetooth MAC address. This allows multiple devices to be distinguished when pairing.

- Uses Nordic UART Service (NUS) over BLE
- LE Secure Connections with passkey display for bonding
- JSON-based protocol for heartbeats, permission requests, and file transfers

## Features

- **18 ASCII pet species** with frame-based animations (axolotl, blob, cat, dragon, etc.)
- **GIF character support** via LittleFS character packs
- **Mood/energy system** with personality states (idle, busy, celebrate, dizzy, sleep, etc.)
- **Approval UI** for agent permission requests
- **Clock display** with orientation detection
- **Stats tracking** (approvals, denials, tokens, sessions)
- **Face-down nap** mode for energy refill

## Controls

| Input | Normal | Pet | Info | Approval |
|---|---|---|---|---|
| **A** (front) | next screen | next screen | next screen | **approve** |
| **B** (right) | scroll transcript | next page | next page | **deny** |
| **Hold A** | menu | menu | menu | menu |
| **Power** (left, short) | toggle screen off | | | |
| **Power** (left, ~6s) | hard power off | | | |
| **Shake** | dizzy | | | |
| **Face-down** | nap (energy refills) | | | |

## Project Layout

```text
src/
  main.cpp       main loop, state machine, UI screens
  buddy.cpp      ASCII species render helpers
  buddies/       one file per species (18 total)
  ble_bridge.cpp Nordic UART service
  character.cpp  GIF decode + render
  board_compat.h M5Unified HAL abstraction
  data.h         wire protocol and JSON parse
  xfer.h         folder push receiver
  stats.h        persisted settings and counters
characters/      example GIF character packs
tests/           unit tests (about_info, clock, persona, utf8)
tools/           asset prep and flashing helpers
```

## Running Tests

```bash
pio test
```

## Attribution

This firmware is a port of [CodeBuddy](https://github.com/anthropics/code-buddy) by Anthropic, PBC, licensed under the MIT License. The GIF assets in `characters/bufo/` are from the [bufo](https://bufo.zone) community emoji set and retain their original rights.
