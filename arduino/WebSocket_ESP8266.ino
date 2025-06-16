#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>

const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";

// Dirección IP o dominio de tu backend (Render, etc.)
const char* websocket_server = "example.onrender.com";
const uint16_t websocket_port = 80;
const char* websocket_path = "/ws-server.php"; // ruta en backend PHP

WebSocketsClient webSocket;

const int ledPin = D5;

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_TEXT:
      Serial.printf("[WebSocket] Mensaje recibido: %s\n", payload);
      if (strcmp((char*)payload, "ON") == 0) {
        digitalWrite(ledPin, HIGH);
      } else if (strcmp((char*)payload, "OFF") == 0) {
        digitalWrite(ledPin, LOW);
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");

  webSocket.begin(websocket_server, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000); // reconexión automática
}

void loop() {
  webSocket.loop();
}
