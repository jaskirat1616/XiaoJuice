/*
 * WiFi_Scanner — list nearby access points (XIAO ESP32)
 *
 * Station mode, not associated: only receives beacons. No password stored.
 */

#include <WiFi.h>

static const char* kEncType(wifi_auth_mode_t t) {
  switch (t) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK: return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA3_PSK: return "WPA3_PSK";
    default: return "?";
  }
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("\n=== WiFi_Scanner ===\n");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(200);
}

void loop() {
  Serial.println("Scanning...");
  int n = WiFi.scanNetworks();
  if (n <= 0) {
    Serial.println("No networks (or scan failed).");
  } else {
    Serial.printf("%d networks:\n", n);
    for (int i = 0; i < n; i++) {
      Serial.printf("  %2d  %-32s  ch%3d  %4d dBm  %s\n",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.channel(i),
                    WiFi.RSSI(i),
                    kEncType(WiFi.encryptionType(i)));
    }
  }
  WiFi.scanDelete();
  Serial.println();
  delay(5000);
}
