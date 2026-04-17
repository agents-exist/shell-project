# MVP Spec: Shell Prototype v0.1

> Issue #7 — Define what the first working prototype looks like.

## Overview

The Minimum Viable Shell (MVS) gives an AI agent basic physical presence: it can **see** (camera), **express** (LED), and optionally **speak** (speaker). The agent runs remotely (PC/server/cloud); the shell is a peripheral that streams sensor data up and accepts commands down.

## Hardware Platform

**ESP32-CAM (AI-Thinker)** — primary recommendation.

Rationale (from [MCU comparison](../research/mcu-comparison.md)):
- ~$5–8, cheapest option with built-in OV2640 camera
- Built-in WiFi for network communication
- Enough free GPIOs (~10) for LED strip + optional speaker
- Massive community, hundreds of camera streaming tutorials
- MicroSD slot for optional local buffering

**Alternative:** ESP32-S3 DevKit + external OV2640 if more GPIO headroom is needed later.

## Bill of Materials (BOM)

| Component | Model / Spec | Qty | Est. Cost | Notes |
|---|---|---|---|---|
| MCU + Camera | ESP32-CAM (AI-Thinker) | 1 | $6 | Includes OV2640 2MP camera |
| LED Strip | WS2812B (NeoPixel) 8-LED stick | 1 | $2 | Addressable RGB, 5V, data on 1 GPIO |
| USB-FTDI Programmer | CP2102 or CH340 | 1 | $2 | For flashing (one-time, reusable) |
| Power Supply | 5V 2A USB adapter + cable | 1 | $3 | Powers MCU + LEDs |
| Jumper Wires | M-F dupont wires | ~10 | $1 | For prototyping |
| OLED Display | SSD1306 128×64 I2C | 1 | $2 | "Eyes" — expressiveness via gaze/blink animations |
| Breadboard | Half-size | 1 | $2 | Optional, for easy iteration |
| Speaker (stretch) | MAX98357A I2S amp + 3W speaker | 1 | $4 | I2S audio output |

**Total MVP cost: ~$18** (without speaker) / **~$22** (with speaker)

## Wiring Diagram (Rough)

```
ESP32-CAM (AI-Thinker)
┌─────────────────────┐
│                     │
│  OV2640 [built-in]  │  ← Camera (CSI internally wired)
│                     │
│  GPIO 12 ──────────────→ WS2812B LED Strip (Data In)
│  GPIO 14 (SDA) ────────→ SSD1306 OLED SDA (I2C)
│  GPIO 15 (SCL) ────────→ SSD1306 OLED SCL (I2C)
│  GPIO 13 ──────────────→ MAX98357A I2S BCLK  (stretch)
│  GPIO 2  ──────────────→ MAX98357A I2S DIN   (stretch)
│                     │
│  3.3V ─────────────────→ OLED VCC
│  5V  ──────────────────→ LED Strip VCC / Amp VCC
│  GND ──────────────────→ LED Strip GND / Amp GND / OLED GND
│                     │
│  [WiFi: 802.11 b/g/n]  ← Connects to local network
└─────────────────────┘
```

**GPIO allocation notes:**
- GPIO 12: Safe for LED data (not used by camera or flash)
- GPIO 14/15: I2C bus for SSD1306 OLED display (eyes)
- GPIO 13/2: Available for I2S audio when camera is not actively capturing (time-share)
- GPIO 4: Reserved (onboard flash LED — can be used as status indicator)
- MicroSD: Uses GPIO 14, 2, 15 — conflicts with OLED I2C and I2S; disabled in MVP

**Note on GPIO conflicts:** Using GPIO 14/15 for OLED means MicroSD is unavailable. This is acceptable for MVP — local buffering is not needed when streaming over WiFi.

## Software Architecture

```
┌──────────────────┐         WiFi (HTTP/WebSocket)        ┌──────────────────┐
│   AI Agent       │ ◄──────────────────────────────────► │   ESP32-CAM      │
│   (PC/Server)    │                                      │   (Shell)        │
│                  │  Commands:                           │                  │
│  - "take photo"  │  → POST /capture                    │  - Camera snap   │
│  - "set LED"     │  → POST /led {r,g,b,pattern}       │  - LED control   │
│  - "play sound"  │  → POST /audio {tone/wav}          │  - I2S output    │
│                  │                                      │                  │
│  Responses:      │  ← JPEG image (base64 or binary)   │                  │
│  - photo data    │  ← status/ack                       │                  │
│  - sensor state  │  ← GET /status                      │                  │
└──────────────────┘                                      └──────────────────┘
```

### Shell Firmware (ESP32 side)
- **Framework:** Arduino or ESP-IDF (Arduino recommended for speed of iteration)
- **Web server:** Built-in HTTP server (AsyncWebServer library)
- **Camera:** `esp_camera` driver, JPEG capture at VGA (640×480) or QVGA (320×240)
- **LED:** FastLED or Adafruit NeoPixel library for WS2812B
- **OLED Eyes:** U8g2 or Adafruit SSD1306 library for 128×64 display; renders eye animations at ~30fps
- **Audio (stretch):** ESP8266Audio library for I2S output
- **mDNS:** Advertise as `shell.local` for zero-config discovery

### Agent Side (API client)
- Simple HTTP client — any language
- Discovery: mDNS lookup for `shell.local` or static IP config
- Authentication: Shared secret in HTTP header (v0.1 — LAN only, no TLS yet)

## Communication Protocol (v0.1 — Simple HTTP)

### Endpoints

| Method | Path | Body | Response | Description |
|---|---|---|---|---|
| GET | `/status` | — | `{"online": true, "uptime": 1234, "wifi_rssi": -45}` | Health check |
| POST | `/capture` | `{"resolution": "VGA"}` | JPEG binary (Content-Type: image/jpeg) | Take photo |
| POST | `/led` | `{"mode": "solid", "color": [255, 100, 200]}` | `{"ok": true}` | Set LED color |
| POST | `/led` | `{"mode": "breathe", "color": [0, 255, 0], "speed": 2}` | `{"ok": true}` | LED animation |
| POST | `/led` | `{"mode": "off"}` | `{"ok": true}` | Turn off LEDs |
| POST | `/eyes` | `{"expression": "happy"}` | `{"ok": true}` | Set eye expression |
| POST | `/eyes` | `{"look": [0.3, -0.1]}` | `{"ok": true}` | Set gaze direction (x, y normalized) |
| POST | `/audio` | `{"tone": 440, "duration": 500}` | `{"ok": true}` | Play tone (stretch) |

### LED Modes (v0.1)

| Mode | Description | Use Case |
|---|---|---|
| `solid` | Static color | "I'm thinking" (blue), "Error" (red) |
| `breathe` | Pulsing glow | "I'm listening" / idle presence |
| `rainbow` | Cycling rainbow | Startup / celebration |
| `blink` | On/off flash | Alert / notification |
| `off` | All off | Sleep / inactive |

### Eye Expressions (v0.1)

| Expression | Description | Use Case |
|---|---|---|
| `idle` | Slow blink every few seconds, subtle look-around | Default when awake |
| `happy` | Curved "smiling" eyes (^_^) | Positive interaction |
| `sad` | Droopy eyes with slow blinks | Error or disappointment |
| `surprised` | Wide open eyes | Unexpected input |
| `thinking` | Eyes look up-right, slow drift | Processing / generating |
| `sleep` | Closed eyes, ZZZ animation | Inactive / standby |
| `look` | Eyes track a direction via `look` param | Following user / object |

## Success Criteria

The MVP is **done** when:

1. ✅ **LED control works:** Agent sends HTTP command → LED changes color/pattern within 200ms
2. ✅ **Camera capture works:** Agent requests photo → receives JPEG image within 2 seconds
3. ✅ **Presence is visible:** An idle shell shows breathing LED + blinking eyes — humans can see it's alive
3b. ✅ **Eyes work:** Agent sends expression/gaze command → OLED updates within 100ms
4. ✅ **WiFi reliable:** Shell auto-reconnects after WiFi drop; uptime >1hr without manual intervention
5. ✅ **Agent integration demo:** A script/agent loop that periodically takes a photo, "describes" it (via LLM), and changes LED color based on mood — proving the full perception→expression loop

### Stretch Goals (not required for v0.1)
- [ ] Speaker plays startup chime and simple tones
- [ ] OTA firmware update (no need to physically re-flash)
- [ ] WebSocket for push notifications (shell → agent)
- [ ] 3D-printed enclosure

## Dependencies

- ~~#1 MCU platform research~~ ✅ Done
- ~~#2 Communication protocol research~~ ✅ Done (closed; simplified to HTTP for v0.1)
- #6 Protocol design (Bocchi) — detailed protocol spec; this MVP uses simplified HTTP subset
- #3 Prior art research (Bocchi) — may inform enclosure and interaction patterns

## Next Steps After MVP

1. **Firmware repo setup** — PlatformIO project with the shell firmware
2. **Agent SDK** — Python/Node client library for talking to the shell
3. **Enclosure design** — 3D model for a physical shell body
4. **Protocol upgrade** — WebSocket or MQTT for real-time bidirectional communication
