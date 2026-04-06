# Xiao Pocket Chaos Juice

## Disclaimer

This software is provided **for education and security research on hardware you own**, in environments where you have **explicit permission** to test. Broadcasting spoofed Bluetooth Low Energy advertisements can annoy people, interfere with devices, and may violate **computer misuse, telecommunications, or harassment laws** where you live. **Do not** use it against strangers, in public spaces to target unknown phones, or for any purpose that is malicious, deceptive, or non-consensual. The authors assume **no liability** for misuse. You are solely responsible for compliance with applicable laws and policies.

---

## Project description

**Xiao Pocket Chaos Juice** is a firmware sketch for tiny **Seeed XIAO ESP32** boards (ESP32-C3 / ESP32-S3 and similar). Powered from USB with no extra components, it continuously transmits **Bluetooth Low Energy (BLE) legacy advertisements** that reuse a well-known pattern of **Apple manufacturer-specific data**—the same binary templates popularized in open-source “proximity” demos. Each cycle picks a **random “device profile”** (headphones, TV setup, transfer flow, etc.) and a **random display string** from a local list; optional **scan-response** data exposes a short **BLE device name** to scanners, while the primary ad carries the spoofed payload.

The sketch is meant as a **compact learning artifact**: how BLE AD structures work, how manufacturer data is packed, and how commodity hardware can broadcast arbitrary (within limits) legacy frames. A **Serial** log at **115200 baud** prints which profile and label were used, useful when correlating behavior with a phone or a BLE sniffer.

**What this is not:** it does not pair with phones like a real accessory, push files, or implement Apple’s proprietary pairing protocols. System UI on iOS is driven by **Apple’s interpretation** of the advertisement bytes and profile id, not by the custom strings in `NAMES[]`.

## Requirements

- **Board:** Seeed XIAO ESP32C3 or XIAO ESP32S3 (or another ESP32 with BLE—adjust the FQBN).
- **Arduino IDE** or **Arduino CLI** with the **Espressif ESP32** board package.
- No wiring beyond USB; optional Serial monitor.

## Quick start

1. Open `XiaoPocketChaosJuice/XiaoPocketChaosJuice.ino` (a duplicate may exist at the repo root—prefer one canonical copy).
2. Select the board that matches your hardware (e.g. **XIAO_ESP32C3**, **XIAO_ESP32S3**).
3. Choose the correct serial port and **Upload**.
4. Optionally open the **Serial Monitor** at **115200** baud.

## Utilities

Separate **Arduino sketches** (BLE scan, Wi‑Fi scan, simple HTTP GET) live under [`utilities/`](utilities/). See [`utilities/README.md`](utilities/README.md). They are **not** Python; they target the same XIAO ESP32 boards.

## Customization

- **`NAMES[]`** — Lines used for the BLE **Complete Local Name** in scan response. Keep each entry **≤ 29 ASCII characters** so it fits a typical 31-byte scan-response payload with overhead.
- **`DEVS[]`** — Rotating list of spoof profiles (see the **EvilAppleJuice-ESP32** section at the end of this file for where those templates come from).

## Credits

- Lineage: community projects around **AppleJuice** / **EvilAppleJuice** and public BLE continuity research.
- Further reading on message families: [furiousMAC / continuity](https://github.com/furiousMAC/continuity).

## License

This repo may not ship a SPDX license file. Treat the sketch as **use at your own risk** unless you add a license. If you fork, respect upstream licenses from referenced projects.

---

## Technical note: templates from [EvilAppleJuice-ESP32](https://github.com/ckcr4lyf/EvilAppleJuice-ESP32)

This project’s **packet construction** is aligned with **[EvilAppleJuice-ESP32](https://github.com/ckcr4lyf/EvilAppleJuice-ESP32)**—specifically the logic in its `devices.cpp` / `devices.hpp` (Arduino INO tree: `EvilAppleJuice-ESP32-INO/`).

| Upstream concept | What it is here |
|------------------|-----------------|
| `AppleDevice` / `ALL_DEVICES` | Our `Dev` struct and `DEVS[]` array—**the same set of model ids and names** as upstream’s full device list. |
| `generatePacket()` | Our `buildPacket()` — **identical layout**: fixed header bytes, single **model id** byte inserted per profile. |
| `APPLE_AUDIO` | **31-byte** advertisement payload; **model id at byte index 7** (after the `0xFF` manufacturer block and Apple prefix bytes). |
| `APPLE_SETUP` | **23-byte** advertisement payload; **model id at byte index 13** (shorter “setup” style template). |

Upstream documents those frames as spoofed **Continuity-style** manufacturer data (Apple company id `0x004C` in little-endian form inside the AD). **Xiao Pocket Chaos Juice** adds extras on top (random scan-response names, optional advertisement-type rotation, optional random static address when building with Bluedroid), but the **core templates and id table** match **EvilAppleJuice-ESP32** so behavior stays comparable to that reference firmware.

If you want the authoritative list of profiles or to diff against future upstream changes, use **`devices.hpp`** / **`devices.cpp`** in the [EvilAppleJuice-ESP32 repository](https://github.com/ckcr4lyf/EvilAppleJuice-ESP32) as the source of truth for template bytes and device labels.
# XiaoJuice
# XiaoJuice
