#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

const char *ssid = "TU_WIFI";
const char *password = "TU_PASSWORD";

const char *websocket_server = "sistema-websocket.onrender.com";
const uint16_t websocket_port = 443;
const char *websocket_path = "/";

WebSocketsClient webSocket;

const int ledPin = 18;

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("[WebSocket] Desconectado");
    break;

  case WStype_CONNECTED:
    Serial.println("[WebSocket] Conectado a Render");
    break;

  case WStype_TEXT:
  {
    Serial.printf("[WebSocket] Mensaje recibido: %s\n", payload);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print("[JSON] Error: ");
      Serial.println(error.c_str());
      return;
    }

    const char *type = doc["type"];
    const char *led = doc["led"];

    if (type && strcmp(type, "status") == 0)
    {
      if (led && strcmp(led, "ON") == 0)
      {
        digitalWrite(ledPin, HIGH);
        Serial.println("[LED] ON");
      }
      else if (led && strcmp(led, "OFF") == 0)
      {
        digitalWrite(ledPin, LOW);
        Serial.println("[LED] OFF");
      }
    }

    break;
  }

  case WStype_ERROR:
    Serial.println("[WebSocket] Error");
    break;

  default:
    break;
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.println();
  Serial.println("=================================");
  Serial.println(" ESP32 Arduino LED");
  Serial.println("=================================");

  WiFi.begin(ssid, password);

  Serial.print("[WiFi] Conectando");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("[WiFi] Conectado");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("[WebSocket] Conectando a Render...");

  webSocket.beginSSL(
      websocket_server,
      websocket_port,
      websocket_path
  );

  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop()
{
  webSocket.loop();
}
