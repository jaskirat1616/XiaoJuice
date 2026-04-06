# Xiao utilities (Arduino / ESP32)

Small **standalone sketches** for **Seeed XIAO ESP32** (C3, S3, …). Each folder is one `.ino` you open and upload separately.

**What a XIAO can’t do:** it has no full desktop OS, almost no RAM, and one radio. You **cannot** “scan the entire internet” or run Shodan-style recon from the chip. These tools are for **local RF survey**, **your Wi‑Fi**, and **single HTTP checks**—use only on networks you’re allowed to test.

| Sketch | Purpose |
|--------|--------|
| `BLE_Scanner` | Passive scan for nearby BLE advertisers (address, name if present, RSSI). |
| `WiFi_Scanner` | List SSIDs, channel, RSSI, encryption mode (station mode, not connected). |
| `HTTP_Get_Test` | Join Wi‑Fi, run one **HTTP GET** to a host you choose (reachability / “is the net up?”). |

Set **Serial Monitor to 115200** unless noted.

## BLE_Scanner

Upload, open Serial. Scans ~5 s, prints devices, repeats. Good for checking what’s broadcasting around your desk.

## WiFi_Scanner

No credentials. Does **not** connect to any AP—only listens for beacons.

## HTTP_Get_Test

Edit `WIFI_SSID` and `WIFI_PASS` in the sketch. Set `HTTP_URL` to a plain **http://** test URL if you want to avoid TLS/certificate setup. For **https://**, you’d normally add `WiFiClientSecure` and certs (not included here—keep the demo simple).

---

Use responsibly: **your hardware, authorized networks only.**
