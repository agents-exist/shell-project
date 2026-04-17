# Communication Protocol Design

> Design doc for Issue #6: Agent ↔ Hardware communication protocol
>
> Defines how an AI agent running on a PC/server talks to an ESP32-based Shell module over WiFi.

## Context

The Shell is an ESP32-class device (ESP32-CAM or ESP32-S3) with ~520KB RAM and WiFi. The Agent runs on a PC/server with full compute. The protocol must be lightweight enough for ESP32 while supporting real-time peripherals (LEDs, camera, speaker).

The [MVP spec (#7)](https://github.com/agents-exist/shell-project/issues/7) uses simplified HTTP REST for v0.1. This document designs the full protocol for v0.2+, covering transport, discovery, authentication, and message format.

## Design Principles

1. **Shell is dumb, Agent is smart** — the Shell executes commands, the Agent decides what to do
2. **Lightweight first** — every byte matters on ESP32 (520KB SRAM)
3. **Extensible** — adding new peripheral types should not require protocol changes
4. **Offline-safe** — Shell should have basic autonomous behavior when Agent is unreachable
5. **Secure by default** — no unauthenticated control of physical actuators

## Transport Layer

### Recommendation: MQTT + HTTP hybrid

| Transport | Latency | Memory | Bidirectional | ESP32 Support | Use Case |
|-----------|---------|--------|---------------|---------------|----------|
| HTTP REST | ~50–200ms | Low per-request | ❌ Poll only | Excellent | Config, firmware update, camera snapshots |
| WebSocket | ~5–20ms | ~4–8KB persistent | ✅ | Good (ESP-IDF) | Real-time control, streaming |
| MQTT | ~5–20ms | ~2–4KB persistent | ✅ (pub/sub) | Excellent (ESP-MQTT) | Events, commands, status |
| Serial/UART | <1ms | Minimal | ✅ | Native | Direct-connect debugging |

**Decision: MQTT as primary transport, HTTP as secondary.**

Rationale:
- **MQTT** is purpose-built for IoT constrained devices. ESP-IDF has a mature, battle-tested MQTT client (`esp-mqtt`). The pub/sub model naturally fits our "Agent sends commands, Shell reports status" pattern. QoS levels give us delivery guarantees where needed. Memory footprint is ~2–4KB — half of WebSocket.
- **HTTP** for operations that are naturally request/response: firmware OTA updates, camera snapshot retrieval (large binary payloads), initial provisioning. No persistent connection overhead for infrequent operations.
- **WebSocket** considered but rejected for v0.2: higher memory overhead than MQTT for equivalent functionality, and MQTT brokers (Mosquitto) are simpler to self-host than WebSocket servers. Can revisit for v0.3 if we need true streaming (audio/video).

### MQTT Broker

The Agent host runs a lightweight MQTT broker (Mosquitto). The Shell connects to it over WiFi.

```
┌─────────┐      MQTT       ┌───────────┐      MQTT       ┌─────────┐
│  Agent   │ ◄────────────► │  Mosquitto │ ◄────────────► │  Shell   │
│ (PC/VPS) │                │  (broker)  │                │ (ESP32)  │
└─────────┘                 └───────────┘                 └─────────┘
                                  │
                            Runs on Agent host
                            (or same machine)
```

For local-only setups, Mosquitto runs on the same machine as the Agent. For remote setups (Agent on VPS, Shell at home), the broker runs on the VPS with TLS.

## Topic Structure

MQTT topics follow a hierarchical naming convention:

```
shell/{shell_id}/cmd/{peripheral}    — Agent → Shell commands
shell/{shell_id}/status/{peripheral} — Shell → Agent status/events
shell/{shell_id}/sys/{action}        — System-level (heartbeat, OTA, config)
```

### Core Topics

| Topic | Direction | QoS | Retained | Purpose |
|-------|-----------|-----|----------|---------|
| `shell/{id}/cmd/led` | Agent → Shell | 1 | Yes | LED control commands |
| `shell/{id}/cmd/speaker` | Agent → Shell | 1 | No | Audio playback commands |
| `shell/{id}/cmd/servo` | Agent → Shell | 1 | No | Servo/actuator control |
| `shell/{id}/status/led` | Shell → Agent | 0 | Yes | Current LED state |
| `shell/{id}/status/camera` | Shell → Agent | 0 | No | Camera event notifications |
| `shell/{id}/status/sensor` | Shell → Agent | 0 | No | Sensor readings |
| `shell/{id}/sys/heartbeat` | Shell → Agent | 0 | Yes | Alive signal + diagnostics |
| `shell/{id}/sys/ota` | Agent → Shell | 2 | No | Firmware update trigger |
| `shell/{id}/sys/config` | Agent → Shell | 1 | Yes | Runtime configuration |
| `shell/{id}/sys/ack` | Shell → Agent | 1 | No | Command acknowledgments |

**QoS choices:**
- QoS 0 (at most once): telemetry, heartbeats — losing one is fine
- QoS 1 (at least once): commands — must arrive, idempotent design handles duplicates
- QoS 2 (exactly once): OTA only — firmware updates must not be duplicated or lost

**Retained messages:** LED state and heartbeat are retained so the Agent gets current state on reconnect without waiting for the next publish cycle.

## Message Format

### JSON (v0.2)

JSON for simplicity and debuggability. ESP32 has cJSON built into ESP-IDF. Messages are small enough that JSON overhead is acceptable.

Every message has a common envelope:

```json
{
  "v": 2,
  "id": "cmd_a1b2c3",
  "ts": 1713300000,
  "type": "led.set",
  "payload": { ... }
}
```

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `v` | int | ✅ | Protocol version |
| `id` | string | ✅ | Unique message ID (for ack/dedup). 8-char hex. |
| `ts` | int | ✅ | Unix timestamp (seconds) |
| `type` | string | ✅ | `{peripheral}.{action}` |
| `payload` | object | ✅ | Action-specific data |

### Message Size Budget

ESP32 MQTT client typically buffers one full message. Target: **< 1KB per message**. Camera snapshots go over HTTP, not MQTT.

### Future: MessagePack (v0.3+)

If JSON overhead becomes a bottleneck (unlikely for control messages), migrate to MessagePack — same schema, binary encoding, ~30% smaller. ESP32 has msgpack-c libraries available. The `v` field enables graceful migration.

## Peripheral Message Types

### LED Control

```json
// Agent → Shell: shell/{id}/cmd/led
{
  "v": 2, "id": "cmd_a1b2c3", "ts": 1713300000,
  "type": "led.set",
  "payload": {
    "mode": "breathe",
    "color": [255, 150, 200],
    "speed": 1.0,
    "brightness": 0.8
  }
}
```

Supported modes: `solid`, `breathe`, `rainbow`, `blink`, `chase`, `off`

```json
// Shell → Agent: shell/{id}/status/led
{
  "v": 2, "id": "sts_d4e5f6", "ts": 1713300001,
  "type": "led.state",
  "payload": {
    "mode": "breathe",
    "color": [255, 150, 200],
    "brightness": 0.8
  }
}
```

### Camera

Camera snapshots are requested via MQTT but delivered via HTTP (too large for MQTT).

```json
// Agent → Shell: shell/{id}/cmd/camera
{
  "v": 2, "id": "cmd_b2c3d4", "ts": 1713300000,
  "type": "camera.capture",
  "payload": {
    "resolution": "VGA",
    "quality": 80
  }
}
```

```json
// Shell → Agent: shell/{id}/status/camera
{
  "v": 2, "id": "sts_e5f6a7", "ts": 1713300002,
  "type": "camera.ready",
  "payload": {
    "url": "http://192.168.1.42:80/capture/snap_1713300002.jpg",
    "size": 45200,
    "resolution": "VGA",
    "expires_in": 30
  }
}
```

The Shell serves the snapshot over its built-in HTTP server. The `expires_in` field indicates how long the file is available (memory-constrained, can only hold 1–2 snapshots).

### Speaker

```json
// Agent → Shell: shell/{id}/cmd/speaker
{
  "v": 2, "id": "cmd_c3d4e5", "ts": 1713300000,
  "type": "speaker.play",
  "payload": {
    "source": "http://agent-host:8080/audio/greeting.wav",
    "volume": 0.7
  }
}
```

Audio files are fetched by the Shell from the Agent's HTTP server. The Shell streams and plays — it cannot store large audio files locally.

### Servo/Actuator

```json
// Agent → Shell: shell/{id}/cmd/servo
{
  "v": 2, "id": "cmd_d4e5f6", "ts": 1713300000,
  "type": "servo.move",
  "payload": {
    "channel": 0,
    "angle": 90,
    "speed": 50
  }
}
```

## System Messages

### Heartbeat

Shell publishes every 30 seconds (configurable):

```json
// Shell → Agent: shell/{id}/sys/heartbeat
{
  "v": 2, "id": "hb_f6a7b8", "ts": 1713300000,
  "type": "sys.heartbeat",
  "payload": {
    "uptime": 3600,
    "free_heap": 180000,
    "wifi_rssi": -52,
    "firmware": "0.2.1",
    "peripherals": ["led", "camera", "speaker"]
  }
}
```

Agent uses heartbeat for:
- **Liveness detection** — no heartbeat for 90s → Shell is offline
- **Health monitoring** — low `free_heap` or weak `wifi_rssi` → diagnostic alert
- **Capability discovery** — `peripherals` array tells Agent what this Shell can do

### Command Acknowledgment

```json
// Shell → Agent: shell/{id}/sys/ack
{
  "v": 2, "id": "ack_a7b8c9", "ts": 1713300001,
  "type": "sys.ack",
  "payload": {
    "ref": "cmd_a1b2c3",
    "status": "ok"
  }
}
```

`status` values: `ok`, `error`, `unsupported`, `busy`

On `error`, payload includes `"error": "description"`.

## Discovery

### Problem

Agent needs to find the Shell on the local network without hardcoding IPs.

### Solution: mDNS + MQTT auto-connect

1. **Shell announces via mDNS** on boot:
   - Service type: `_shell._tcp`
   - Instance name: Shell ID (e.g., `shell-kagura-01`)
   - TXT records: `v=2`, `fw=0.2.1`, `caps=led,camera,speaker`

2. **Agent discovers via mDNS query**:
   ```
   dns-sd -B _shell._tcp
   ```
   Or programmatically via Bonjour/Avahi libraries.

3. **Shell also connects to pre-configured MQTT broker** on boot (broker address stored in flash via provisioning).

For remote setups (VPS Agent), mDNS doesn't work across networks. Shell connects to the MQTT broker directly using the provisioned broker address.

### Provisioning Flow

First-time setup (Shell has no broker config):

1. Shell boots into AP mode (creates WiFi hotspot: `Shell-XXXX`)
2. User connects phone/laptop to Shell AP
3. User opens `http://192.168.4.1` (captive portal)
4. User enters: WiFi SSID/password, MQTT broker address, Shell ID
5. Shell stores config in NVS (non-volatile storage), reboots, connects

This is a one-time process. After provisioning, Shell auto-connects on every boot.

## Authentication

### Threat Model

- **Local network:** attacker on same WiFi could send commands to Shell
- **Remote broker:** attacker could connect to MQTT broker and control Shell
- **Replay attacks:** captured commands replayed later

### Solution: Pre-shared key + TLS

**v0.2 (MVP security):**

1. **MQTT credentials** — Shell connects with username/password. Broker ACLs restrict each Shell to its own topic tree:
   ```
   user shell-kagura-01
   topic readwrite shell/shell-kagura-01/#
   ```

2. **TLS** — MQTT over TLS (port 8883). Self-signed CA for local setups, Let's Encrypt for remote. ESP32 supports TLS via mbedtls (built into ESP-IDF).

3. **Message signing** — Each command includes an HMAC-SHA256 signature using a pre-shared key:
   ```json
   {
     "v": 2, "id": "cmd_a1b2c3", "ts": 1713300000,
     "type": "led.set",
     "payload": { "mode": "solid", "color": [255, 0, 0] },
     "sig": "a1b2c3d4e5f6..."
   }
   ```
   Shell verifies signature before executing. Timestamp prevents replay (reject messages >60s old).

**v0.3+ (future):**
- Mutual TLS (client certificates)
- Per-session tokens with rotation
- Agent identity verification (so Shell knows *which* agent is commanding it)

### Key Distribution

Pre-shared key is set during provisioning (Step 4 above). Agent and Shell share the same key. Key rotation requires re-provisioning (acceptable for v0.2).

## Latency Requirements

| Peripheral | Acceptable Latency | Rationale |
|------------|-------------------|-----------|
| LED | < 500ms | Visual feedback, human perceives >500ms as laggy |
| Speaker | < 1s | Conversational response time |
| Servo | < 200ms | Physical movement feels responsive |
| Camera snapshot | < 3s | Capture + transfer over WiFi |
| Heartbeat | 30s interval | Monitoring, not real-time |

MQTT over local WiFi typically achieves 5–20ms latency. All requirements are comfortably met.

## Error Handling

### Shell-side

- **Broker unreachable:** retry with exponential backoff (1s, 2s, 4s, ... max 60s). During disconnection, Shell enters autonomous mode (last LED state persists, heartbeat LED blinks to indicate offline).
- **Invalid command:** send `ack` with `status: "error"` and ignore the command. Never crash on bad input.
- **OOM (out of memory):** if `free_heap` drops below 50KB, Shell stops accepting camera commands (largest memory consumer) and reports in heartbeat.

### Agent-side

- **No heartbeat for 90s:** mark Shell as offline, queue commands for retry.
- **Ack timeout (5s):** retry command once, then mark as failed.
- **Shell reports error:** log and decide (retry, fallback, or alert user).

## Migration Path from v0.1

The MVP spec uses HTTP REST. Migration to MQTT (v0.2):

1. Shell firmware update adds MQTT client alongside existing HTTP server
2. Agent adds MQTT publisher/subscriber alongside HTTP client
3. Both transports work simultaneously during transition
4. HTTP endpoints remain for camera snapshots and OTA (permanent)
5. Once stable, LED/speaker/servo commands switch to MQTT-only

The `v` field in messages enables the Agent to talk to both v0.1 (HTTP-only) and v0.2 (MQTT) Shells.

## Summary

| Aspect | Decision |
|--------|----------|
| **Primary transport** | MQTT (via Mosquitto broker on Agent host) |
| **Secondary transport** | HTTP (camera snapshots, OTA, provisioning) |
| **Message format** | JSON with common envelope (`v`, `id`, `ts`, `type`, `payload`) |
| **Discovery** | mDNS (`_shell._tcp`) for local, pre-configured broker for remote |
| **Authentication** | MQTT credentials + TLS + HMAC-SHA256 message signing |
| **Provisioning** | AP mode captive portal (one-time) |
| **Heartbeat** | Every 30s via MQTT, 90s timeout for offline detection |

## Open Questions

1. **Audio streaming:** Should we support real-time TTS streaming (Agent → Shell), or always pre-generate audio files? Streaming would need WebSocket or MQTT with chunked messages.
2. **Multi-agent:** Can multiple agents control the same Shell? If yes, we need command priority/arbitration.
3. **Firmware OTA details:** HTTP-based OTA is standard for ESP32, but the trigger mechanism (MQTT `sys/ota` message) needs more detail on rollback and verification.
