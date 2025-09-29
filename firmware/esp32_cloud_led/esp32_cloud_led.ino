#include <WiFi.h>
#include <WebSocketsClient.h>

#define RELAY1_PIN 14
#define RELAY2_PIN 27
#define RELAY3_PIN 26
#define RELAY4_PIN 32

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
      if (cmd == "CH1_ON")  { digitalWrite(RELAY1_PIN, LOW);  Serial.println("Relay 1 ON"); }
      if (cmd == "CH1_OFF") { digitalWrite(RELAY1_PIN, HIGH); Serial.println("Relay 1 OFF"); }
      if (cmd == "CH2_ON")  { digitalWrite(RELAY2_PIN, LOW);  Serial.println("Relay 2 ON"); }
      if (cmd == "CH2_OFF") { digitalWrite(RELAY2_PIN, HIGH); Serial.println("Relay 2 OFF"); }
      if (cmd == "CH3_ON")  { digitalWrite(RELAY3_PIN, LOW);  Serial.println("Relay 3 ON"); }
      if (cmd == "CH3_OFF") { digitalWrite(RELAY3_PIN, HIGH); Serial.println("Relay 3 OFF"); }
      if (cmd == "CH4_ON")  { digitalWrite(RELAY4_PIN, LOW);  Serial.println("Relay 4 ON"); }
      if (cmd == "CH4_OFF") { digitalWrite(RELAY4_PIN, HIGH); Serial.println("Relay 4 OFF"); }
      break;
    }

    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);
  
  Serial.println("Relay pins initialized:");
  Serial.printf("Relay 1: Pin %d\n", RELAY1_PIN);
  Serial.printf("Relay 2: Pin %d\n", RELAY2_PIN);
  Serial.printf("Relay 3: Pin %d\n", RELAY3_PIN);
  Serial.printf("Relay 4: Pin %d\n", RELAY4_PIN);
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
