#include <WiFi.h>
#include <WebSocketsClient.h>

#define LED_PIN 12

// --- Your WiFi ---
const char* WIFI_SSID = "ASSD";
const char* WIFI_PASS = "Aeliasoft@2024";

// --- Your backend host (no protocol, just host) ---
const char* WS_HOST = "esp32-led-project.onrender.com";  // Render host
const int   WS_PORT = 443;                               // 443 for wss
const char* DEVICE_ID = "esp32-1";
const char* KEY = "supersecret";

// WebSocket path with auth in query string
String wsPath = String("/ws?device=") + DEVICE_ID + "&key=" + KEY;

WebSocketsClient webSocket;

// OPTIONAL: For production, paste Let's Encrypt ISRG Root X1 certificate here and uncomment:
// const char LE_ROOT_CA[] PROGMEM = R"EOF(
// -----BEGIN CERTIFICATE-----
// MIIFazCCA1OgAwIBAgISA2... (full ISRG Root X1 PEM here)
// -----END CERTIFICATE-----
// )EOF";

void onWsEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("[WS] connected");
      // Optionally tell server we're ready
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

  // --- Secure WebSocket (wss) ---
  webSocket.beginSSL(WS_HOST, WS_PORT, wsPath.c_str());

  // For production, use the CA:
  // webSocket.setCACert(LE_ROOT_CA);

  // For quick testing only (disables TLS verification):
  webSocket.setInsecure();

  webSocket.setReconnectInterval(5000);          // auto-reconnect every 5s if needed
  webSocket.enableHeartbeat(15000, 3000, 2);     // ping/pong keepalive (optional)
}

void loop() {
  webSocket.loop();   // must be called often
}
