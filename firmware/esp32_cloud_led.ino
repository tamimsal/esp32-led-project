#include <WiFi.h>
#include <WebSocketsClient.h>

#define LED_PIN 12

const char* WIFI_SSID = "ASSD";
const char* WIFI_PASS = "Aeliasoft@2024";

const char* WS_HOST = "esp32-led-project.onrender.com";  
const int   WS_PORT = 443;                               
const char* DEVICE_ID = "esp32-1";
const char* KEY = "supersecret";

String wsPath = String("/ws?device=") + DEVICE_ID + "&key=" + KEY;

WebSocketsClient webSocket;

void onWsEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("[WS] connected");
      webSocket.sendTXT("READY");
      break;

    case WStype_DISCONNECTED:
      Serial.println("[WS] disconnected");
      break;

    case WStype_TEXT: {
      String cmd = String((char*)payload).substring(0, length);
      cmd.trim();
      Serial.printf("[WS] cmd: %s\n", cmd.c_str());
      if (cmd == "ON")  digitalWrite(LED_PIN, HIGH);
      if (cmd == "OFF") digitalWrite(LED_PIN, LOW);
      break;
    }

    default:
      break;
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  webSocket.onEvent(onWsEvent);

  webSocket.beginSSL(WS_HOST, WS_PORT, wsPath.c_str());

  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2);
}

void loop() {
  webSocket.loop(); 
}
