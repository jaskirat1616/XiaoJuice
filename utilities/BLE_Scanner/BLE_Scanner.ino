/*
 * BLE_Scanner — nearby BLE advertiser survey (XIAO ESP32)
 *
 * Lists devices seen in one scan window: address, RSSI, name (if advertised).
 * Board: XIAO_ESP32C3 / XIAO_ESP32S3 (package esp32 by Espressif).
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

static const uint32_t kScanSeconds = 5;

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("\n=== BLE_Scanner ===\n");
  BLEDevice::init("");
}

void loop() {
  Serial.println("--- scan start ---");
  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(99);

  BLEScanResults results = scan->start(kScanSeconds, false);
  int n = results.getCount();
  Serial.printf("Found %d device(s)\n", n);
  for (int i = 0; i < n; i++) {
    BLEAdvertisedDevice d = results.getDevice((uint32_t)i);
    const char* name = d.haveName() ? d.getName().c_str() : "(no name)";
    Serial.printf("  %s  RSSI %4d  %s\n",
                  d.getAddress().toString().c_str(),
                  d.getRSSI(),
                  name);
  }
  scan->clearResults();
  delay(3000);
}
