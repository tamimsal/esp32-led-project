#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

const char* WIFI_SSID = "ASSD";
const char* WIFI_PASS = "Aeliasoft@2024";

const char* SERVER_URL = "https://esp32-led-project.onrender.com";
const char* DEVICE_ID = "esp32-1";
const char* KEY = "supersecret";

#define LED_PIN 12
unsigned long lastPoll = 0;
const unsigned long POLL_MS = 1500;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
}

void loop() {
  if (millis() - lastPoll > POLL_MS) {
    lastPoll = millis();
    fetchCommand();
  }
}

void fetchCommand() {
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClientSecure client;
  client.setInsecure(); // HTTPS without cert validation
  HTTPClient http;
  String url = String(SERVER_URL) + "/api/cmd?device=" + DEVICE_ID + "&key=" + KEY;
  if (!http.begin(client, url)) { Serial.println("HTTP begin failed"); return; }

  int code = http.GET();
  if (code > 0) {
    String cmd = http.getString();
    cmd.trim();
    Serial.printf("Server replied: %s\n", cmd.c_str());
    if (cmd == "ON") digitalWrite(LED_PIN, HIGH);
    if (cmd == "OFF") digitalWrite(LED_PIN, LOW);
  } else {
    Serial.printf("HTTP error: %d\n", code);
  }
  http.end();
}
