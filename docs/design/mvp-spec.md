# MVP Spec: Shell Prototype v0.1

> Issue #7 — Define what the first working prototype looks like.
> **Updated for Pi 3B pivot** — see [#18](https://github.com/agents-exist/shell-project/issues/18).

## Overview

The Minimum Viable Shell (MVS) gives an AI agent basic physical presence: it can **express** (animated eyes on screen + LED), and later **see** (camera) and **speak** (speaker). The agent runs remotely (PC/server/cloud); the shell is a local device that displays expressions and accepts commands over HTTP.

## Hardware Platform

**Raspberry Pi 3B** — Sakana's existing hardware. Zero additional cost to start.

Why pivot from ESP32:
- Hardware already available (Pi 3B + 480×320 SPI screen + camera ribbon)
- Full Linux = Python/Node development, much faster iteration than ESP32 firmware
- Bigger screen (480×320) = richer expressions and eye animations
- Camera can be added later (~¥15 for Pi camera module)
- ESP32/M5Stick becomes **Phase 2** (miniaturization)

## Bill of Materials (BOM)

| Component | Model / Spec | Qty | Est. Cost | Notes |
|---|---|---|---|---|
| SBC | Raspberry Pi 3B | 1 | ¥0 | Sakana already has this |
| Display | 480×320 SPI TFT | 1 | ¥0 | Sakana already has this |
| Camera ribbon | CSI ribbon cable | 1 | ¥0 | Sakana already has this |
| Camera module | Pi Camera v1/v2 | 1 | ~¥15 | **Stretch — not in v0.1** |
| LED Strip | WS2812B (NeoPixel) 8-LED | 1 | ~¥5 | Optional, GPIO18 (PWM) |
| Power Supply | 5V 2.5A USB micro | 1 | ¥0 | Likely already available |
| SD Card | 16GB+ microSD | 1 | ¥0 | For Raspberry Pi OS |

**Total MVP cost: ¥0** (all existing hardware) / **~¥5** with LED strip / **~¥20** with camera

## Wiring Diagram (Rough)

```
Raspberry Pi 3B
┌─────────────────────────┐
│                         │
│  SPI0 ─────────────────────→ 480×320 TFT Display
│    GPIO 8  (CE0)            (SPI chip select)
│    GPIO 11 (SCLK)           (SPI clock)
│    GPIO 10 (MOSI)           (SPI data)
│    GPIO 25 (DC)             (data/command)
│    GPIO 24 (RST)            (reset)
│                         │
│  GPIO 18 (PWM) ────────────→ WS2812B LED Strip (Data In)
│                         │
│  CSI port ─────────────────→ Pi Camera (stretch)
│                         │
│  3.5mm / I2S ──────────────→ Speaker (stretch)
│                         │
│  [WiFi: 802.11 b/g/n]     ← Connects to local network
│  [Ethernet: 10/100]       ← Alternative wired connection
└─────────────────────────┘
```

## Software Architecture

```
┌──────────────────┐         WiFi/LAN (HTTP)              ┌──────────────────┐
│   AI Agent       │ ◄──────────────────────────────────► │   Raspberry Pi   │
│   (PC/Server)    │                                      │   (Shell)        │
│                  │  Commands:                           │                  │
│  - "set eyes"    │  → POST /expression {mood, anim}    │  - Pygame render │
│  - "set LED"     │  → POST /led {color, mode}          │  - LED control   │
│  - "take photo"  │  → POST /capture (stretch)          │  - picamera      │
│                  │                                      │                  │
│  Responses:      │  ← expression ack                   │                  │
│  - status        │  ← GET /status                      │                  │
│  - photo (later) │  ← JPEG binary                      │                  │
└──────────────────┘                                      └──────────────────┘
```

### Shell Software (Pi side)
- **Language:** Python 3
- **Display:** Pygame for fullscreen eye animations on SPI TFT
  - Animated eyes with blink, look-around, mood expressions
  - Smooth transitions between states
- **HTTP Server:** Flask or FastAPI for agent ↔ shell communication
- **LED:** rpi_ws281x library for WS2812B NeoPixel control via GPIO18
- **Camera (stretch):** picamera2 for CSI camera capture
- **Audio (stretch):** pygame.mixer or aplay for sound output
- **Service:** systemd unit for auto-start on boot

### Agent Side (API client)
- Simple HTTP client — any language
- Discovery: mDNS (`shell.local`) or static IP
- Authentication: Shared secret in HTTP header (v0.1 — LAN only)

## Communication Protocol (v0.1 — HTTP REST)

### Endpoints

| Method | Path | Body | Response | Description |
|---|---|---|---|---|
| GET | `/status` | — | `{"online": true, "uptime": 1234, "expression": "neutral"}` | Health check |
| POST | `/expression` | `{"mood": "happy", "intensity": 0.8}` | `{"ok": true}` | Set eye expression |
| POST | `/expression` | `{"animation": "blink"}` | `{"ok": true}` | Trigger animation |
| POST | `/led` | `{"mode": "solid", "color": [255, 100, 200]}` | `{"ok": true}` | Set LED color |
| POST | `/led` | `{"mode": "breathe", "color": [0, 255, 0], "speed": 2}` | `{"ok": true}` | LED animation |
| POST | `/led` | `{"mode": "off"}` | `{"ok": true}` | Turn off LEDs |
| POST | `/capture` | `{"resolution": "480x320"}` | JPEG binary | Take photo (stretch) |
| POST | `/audio` | `{"tone": 440, "duration": 500}` | `{"ok": true}` | Play tone (stretch) |

### Expression Moods (v0.1)

| Mood | Eye Animation | Use Case |
|---|---|---|
| `neutral` | Relaxed eyes, slow blink | Default / idle |
| `happy` | Upward curved eyes, sparkle | Good news, task complete |
| `thinking` | Eyes looking up-right, slow drift | Processing, waiting |
| `surprised` | Wide open eyes | Unexpected event |
| `sleepy` | Half-closed, slow droop | Low activity, night mode |
| `error` | X-eyes or swirl | Something went wrong |

### LED Modes (v0.1)

| Mode | Description | Use Case |
|---|---|---|
| `solid` | Static color | Status indicator |
| `breathe` | Pulsing glow | Idle presence |
| `rainbow` | Cycling rainbow | Startup / celebration |
| `blink` | On/off flash | Alert / notification |
| `off` | All off | Sleep / inactive |

## Success Criteria

The MVP is **done** when:

1. ✅ **Eye animation works:** Pygame renders animated eyes on the 480×320 TFT — idle blink loop runs smoothly
2. ✅ **Expression control works:** Agent sends HTTP command → eyes change expression within 500ms
3. ✅ **LED control works:** Agent sends HTTP command → LED changes color/pattern within 200ms
4. ✅ **Presence is visible:** An idle shell shows blinking eyes + gentle breathing LED — humans can see it's alive
5. ✅ **WiFi reliable:** Pi auto-starts shell service on boot; uptime >1hr without manual intervention
6. ✅ **Agent integration demo:** A script/agent loop that changes expressions based on agent state — proving the expression pipeline works end-to-end

### Stretch Goals (not required for v0.1)
- [ ] Camera capture via Pi Camera module
- [ ] Speaker plays startup chime and simple tones
- [ ] Touch input from TFT touchscreen (if supported)
- [ ] WebSocket for push notifications (shell → agent)
- [ ] 3D-printed or cardboard enclosure

## Dependencies

- ~~#1 MCU platform research~~ ✅ Done
- ~~#2 Communication protocol research~~ ✅ Done
- #6 Protocol design — detailed protocol spec; this MVP uses simplified HTTP subset
- ~~#16 Hardware comparison~~ ✅ Done (Pi 3B chosen, #18)
- ~~#12 OLED eyes~~ Superseded by 480×320 TFT

## Phase 2: Miniaturization (ESP32/M5Stick)

After MVP is validated on Pi 3B:
1. Port eye animations to M5StickS3 (see #17 feasibility study)
2. Adapt protocol for constrained MQTT transport
3. Smaller form factor, battery-powered, portable

## Next Steps After MVP

1. **Pi setup guide** — OS install, SPI screen driver, Python env, systemd service
2. **Eye animation engine** — Pygame-based renderer with expression state machine
3. **HTTP API server** — Flask/FastAPI with the endpoints above
4. **Agent SDK** — Python/Node client library for talking to the shell
5. **Enclosure design** — Simple cardboard or 3D-printed shell body
