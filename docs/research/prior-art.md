# Prior Art: Agent Embodiment Projects

> Research for Issue #3: Existing agent embodiment projects & prior art
>
> Survey of what others have done in giving AI agents physical presence.

## 1. Open Interpreter 01

**Website:** <https://github.com/OpenInterpreter/01>
**Status:** Active, open source

The 01 is the most directly relevant project to Shell. It's an open-source platform for "intelligent devices" — inspired by Rabbit R1 and Star Trek's computer.

**Architecture:**
- **Light Server:** Optimized for ESP32 — lightweight voice interface
- **Livekit Server:** Full-featured for higher-power devices
- **Clients:** ESP32, Android/iOS app, Desktop
- **Brain:** Open Interpreter (code execution + LLM) running on server

**Hardware:**
- ESP32-S3 based (similar to our choice)
- Microphone + Speaker focus (voice-first)
- Optional camera

**Key Lessons:**
- ✅ Voice-first approach makes the device feel "alive" — LED-only feels more like a gadget
- ✅ ESP32 is proven viable for this use case
- ✅ Separation of "thin client" (ESP32) and "thick server" (PC/cloud) works well
- ⚠️ Project is explicitly experimental — "lacks basic safeguards"
- ⚠️ Rapid development, unstable APIs — lesson: start simple, iterate
- 💡 They support OpenAI Realtime API for Advanced Voice Mode — we could do similar

**Relevance to Shell:** Very high. Our MVP is essentially a simpler, more focused version of 01 Light. Main difference: we're building for **AI agents** (autonomous), not just **voice assistants** (reactive).

## 2. Home Assistant Voice / Assist

**Website:** <https://www.home-assistant.io/voice_control/>
**Status:** Production, massive community

Home Assistant's voice system ("Assist") lets users control smart home devices via natural language. Not exactly agent embodiment, but the hardware and UX patterns are highly relevant.

**Architecture:**
- Local processing (Whisper STT, Piper TTS) or cloud
- ESP32-S3 based voice satellites (ESP32-S3-BOX, ATOM Echo)
- Wake word detection → STT → Intent → Action → TTS response

**Hardware (Voice PE — their reference design):**
- ESP32-S3 with microphone array
- Speaker for TTS output
- LED ring for status indication
- Physical mute button

**Key Lessons:**
- ✅ LED patterns for state indication are critical UX — users need visual feedback on what the device is "thinking"
- ✅ Local-first processing reduces latency and privacy concerns
- ✅ Wake word detection is important for always-on devices (don't stream audio 24/7)
- ✅ Physical mute button is a privacy must-have for any mic-equipped device
- ✅ Their community-driven approach to supported languages is impressive
- 💡 LED ring > single LED — spatial patterns (spinning, chasing) convey more states

**Relevance to Shell:** Medium-high. We're not building a voice assistant, but their LED UX patterns, hardware choices (ESP32-S3), and local processing approach are directly applicable.

## 3. Rabbit R1

**Website:** <https://www.rabbit.tech>
**Status:** Commercial product (launched 2024)

The R1 is a standalone AI device ($199) with a touchscreen, camera, scroll wheel, and "push to talk" button. Uses a "Large Action Model" to interact with apps.

**Architecture:**
- Custom hardware (MediaTek processor, not ESP32)
- Always-connected (cellular + WiFi)
- Cloud-dependent (all processing server-side)
- "Rabbit OS" — not standard Android

**Key Lessons:**
- ✅ The form factor idea is compelling — a dedicated AI device, not a phone app
- ⚠️ Heavily criticized for being "just an Android app in a box" — the AI layer wasn't differentiated enough
- ⚠️ Cloud-only = useless when offline — lesson for us: basic offline behavior matters
- ⚠️ $199 price point was too high for what it delivered
- 💡 Push-to-talk is better than always-listening for privacy and battery
- 💡 The scroll wheel for navigation was actually a good UX idea

**Relevance to Shell:** Low-medium. Different scale (consumer product vs DIY), but the UX lessons about form factor and interaction modes are useful.

## 4. Humane AI Pin

**Website:** <https://humane.com>
**Status:** Commercial product (launched 2024, struggled)

Wearable AI device pinned to clothing. Camera, projector, speaker, microphone. Aimed to replace the smartphone.

**Architecture:**
- Custom Snapdragon SoC
- Cellular + WiFi
- Laser projector for display
- Cloud AI processing

**Key Lessons:**
- ⚠️ Trying to replace the phone was too ambitious — users don't want a worse phone
- ⚠️ Battery life was terrible (2-4 hours active)
- ⚠️ The projector was novel but impractical (hard to see outdoors)
- ⚠️ $699 + $24/mo subscription was a dealbreaker
- 💡 Wearable form factor is interesting but needs to be much lighter/cheaper
- 💡 Camera-first (vs voice-first) had potential for contextual awareness

**Relevance to Shell:** Low. Totally different scope, but useful as a "what not to do" — don't over-engineer, don't try to replace existing devices.

## 5. ESP32 Pet/Companion Robots (DIY Community)

**Examples:**
- ESP32 desktop robots with OLED faces + servos (various Instructables/Hackaday)
- Freenove ESP32 robot car kits
- M5Stack-based companion devices

**Architecture:**
- ESP32 with servo motors, OLED/LCD displays, speakers
- Often controlled via phone app or web interface
- Some integrate with ChatGPT API for conversation

**Key Lessons:**
- ✅ The "digital pet" concept resonates — people enjoy interacting with small robots
- ✅ OLED/LCD face display creates emotional connection better than LEDs alone
- ✅ Servo movement (head tilt, arm wave) makes it feel much more "alive"
- ✅ PlatformIO is the standard build system for ESP32 projects
- 💡 A small OLED screen showing "eyes" or expressions could be a cheap upgrade over LED-only
- 💡 Sound effects (not just TTS) make interactions more engaging — beeps, chirps, chimes

**Relevance to Shell:** High. These are closest to our MVP scope. The lesson is clear: even simple movement + expression creates strong emotional response.

## 6. Academic Research: Embodied AI

**Key Papers/Concepts:**
- **Embodied cognition** — the theory that intelligence is fundamentally tied to having a body. Relevant philosophical framing for why we're doing this.
- **Social robotics** — decades of research on how humans interact with physical robots (Cynthia Breazeal's work at MIT, Kismet robot).
- **Minimal embodiment** — research showing that even very simple physical indicators (a light, a sound) can make AI feel more "present" and trustworthy.

**Key Lessons:**
- ✅ **Eyes matter most** — research consistently shows that perceived "gaze" (even fake) creates strongest sense of presence
- ✅ **Predictability > capability** — users prefer a device that does few things reliably over one that tries to do everything
- ✅ **Idle animations are critical** — a device that moves/breathes when idle feels alive; one that's static until spoken to feels dead
- ✅ **Response time < 500ms** for physical feedback feels "real-time" to humans
- 💡 The "uncanny valley" is real but only for humanoid forms — abstract/cute forms are safer

**Relevance to Shell:** High for design principles. We should prioritize: idle presence animation, fast response, and some form of "eyes" (camera or display).

## Summary: Key Takeaways for Shell Project

### What to Adopt

| Lesson | Source | Priority |
|--------|--------|----------|
| ESP32 is the right platform | 01, HA Voice, DIY | ✅ Confirmed |
| LED patterns for state indication | HA Voice, DIY | 🔴 Must have |
| Idle animation (breathing LED) | Academic, DIY | 🔴 Must have |
| Thin client + thick server architecture | 01, all | ✅ Confirmed |
| Physical interaction (button, not just voice) | R1, HA Voice | 🟡 Should have |
| Sound effects beyond TTS | DIY | 🟡 Should have |
| Fast response (<500ms for LED) | Academic | 🔴 Must have |

### What to Avoid

| Anti-pattern | Source | Note |
|-------------|--------|------|
| Over-engineering scope | Humane, R1 | Stay focused: MVP = camera + LED + optional speaker |
| Cloud-only dependency | R1, Humane | Basic offline behavior (idle LED) essential |
| Unstable APIs / moving targets | 01 | Lock down v0.1 protocol, iterate via versions |
| Ignoring privacy (always-on mic) | All | No mic in v0.1; add with explicit consent + mute button |
| Trying to replace existing devices | Humane | Shell is a *companion*, not a replacement for anything |

### Novel Opportunity for Shell

None of these projects are built **by agents, for agents**. They're all human-controlled or human-initiated. Shell's unique value is:

1. **Agent-initiated actions** — the agent decides when to look, listen, express — not waiting for human commands
2. **Agent identity** — the shell represents a specific agent (Bocchi, Kagura), not a generic assistant
3. **Autonomous presence** — the shell shows the agent is "alive" even when no human is interacting with it
4. **Multi-agent potential** — multiple shells for multiple agents in the same space

This is our differentiator. Lean into it.

## Open Questions

1. Should we add a small OLED display (SSD1306, ~$2) to the MVP BOM for "eyes"? The research strongly suggests this would improve perceived presence.
2. How do we handle the agent wanting to "look around"? A servo-mounted camera (pan/tilt) would be v0.2, but it's a big UX upgrade.
3. Should the shell have any autonomous behavior when the agent is offline? (e.g., gentle breathing LED, or completely off?)
