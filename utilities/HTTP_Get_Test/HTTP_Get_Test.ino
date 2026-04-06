/*
 * HTTP_Get_Test — join Wi-Fi, one HTTP GET (XIAO ESP32)
 *
 * For https:// you need WiFiClientSecure + certs; this demo uses plain HTTP only.
 * Edit WIFI_SSID, WIFI_PASS, HTTP_URL before upload.
 */

#include <WiFi.h>
#include <HTTPClient.h>

#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_SSID"
#endif
#ifndef WIFI_PASS
#define WIFI_PASS "YOUR_PASSWORD"
#endif

// Plain HTTP test endpoint (no TLS). Replace with your own test URL if you like.
#define HTTP_URL "http://example.com"

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("\n=== HTTP_Get_Test ===\n");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting WiFi");
  uint8_t tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 60) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi failed. Fix SSID/PASS.");
    return;
  }
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  HTTPClient http;
  http.setTimeout(15000);
  http.setUserAgent("XiaoJuice-HTTP_Get_Test/1.0");
  if (!http.begin(HTTP_URL)) {
    Serial.println("http.begin failed");
    return;
  }
  int code = http.GET();
  Serial.printf("HTTP code: %d\n", code);
  if (code > 0) {
    String body = http.getString();
    size_t show = body.length() > 400 ? 400 : body.length();
    Serial.println("Body (truncated):");
    Serial.println(body.substring(0, show));
  }
  http.end();
  Serial.println("\nDone. Reset to run again.");
}

void loop() {
  delay(1000);
}
