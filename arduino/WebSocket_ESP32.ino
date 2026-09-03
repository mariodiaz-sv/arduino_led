#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// =====================================================
// WIFI
// =====================================================

const char *ssid = "TU SSID";
const char *password = "TU PASSWORD";


// =====================================================
// SERVIDOR WEBSOCKET
// =====================================================

const char* websocketHost = "sistema-websocket.onrender.com";//TU URL DE RENDER 
const uint16_t websocketPort = 443;
const char* websocketPath = "/";


// =====================================================
// LEDS
// =====================================================

const int led1Pin = 18;
const int led2Pin = 19;


// =====================================================
// CLIENTE TLS
// =====================================================

WiFiClientSecure client;


// =====================================================
// ESTADO
// =====================================================

bool websocketConnected = false;

unsigned long lastReconnect = 0;


// =====================================================
// PROTOTIPOS
// =====================================================

bool connectWebSocket();
void processWebSocket();
void sendPong();
String generateWebSocketKey();


// =====================================================
// GENERAR CLAVE WEBSOCKET
// =====================================================

String generateWebSocketKey()
{
  uint8_t randomBytes[16];

  for (int i = 0; i < 16; i++)
  {
    randomBytes[i] = random(0, 256);
  }

  const char* base64 =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

  String result = "";

  for (int i = 0; i < 16; i += 3)
  {
    uint32_t value = 0;

    value |= ((uint32_t)randomBytes[i]) << 16;

    if (i + 1 < 16)
      value |= ((uint32_t)randomBytes[i + 1]) << 8;

    if (i + 2 < 16)
      value |= randomBytes[i + 2];

    result += base64[(value >> 18) & 0x3F];
    result += base64[(value >> 12) & 0x3F];

    if (i + 1 < 16)
      result += base64[(value >> 6) & 0x3F];
    else
      result += "=";

    if (i + 2 < 16)
      result += base64[value & 0x3F];
    else
      result += "=";
  }

  return result;
}


// =====================================================
// CONECTAR WEBSOCKET
// =====================================================

bool connectWebSocket()
{
  Serial.println();
  Serial.println("========================================");
  Serial.println("[WS] CONECTANDO...");
  Serial.println("========================================");

  Serial.print("[WS] Servidor: ");
  Serial.println(websocketHost);

  Serial.print("[WS] Puerto: ");
  Serial.println(websocketPort);

  Serial.print("[WS] Ruta: ");
  Serial.println(websocketPath);


  websocketConnected = false;

  client.stop();

  // ===================================================
  // TLS
  // ===================================================

  client.setInsecure();

  Serial.println("[TLS] Conectando...");

  if (!client.connect(websocketHost, websocketPort))
  {
    Serial.println("[TLS] ❌ ERROR");

    return false;
  }

  Serial.println("[TLS] ✅ CONEXION TLS EXITOSA");


  // ===================================================
  // HANDSHAKE
  // ===================================================

  String wsKey = generateWebSocketKey();


  String request =
    "GET " + String(websocketPath) + " HTTP/1.1\r\n"
    "Host: " + String(websocketHost) + "\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Key: " + wsKey + "\r\n"
    "Sec-WebSocket-Version: 13\r\n"
    "Origin: https://" + String(websocketHost) + "\r\n"
    "\r\n";


  Serial.println("[WS] Enviando handshake...");

  client.print(request);


  // ===================================================
  // RESPUESTA HTTP
  // ===================================================

  unsigned long timeout = millis() + 10000;

  String statusLine = "";

  while (client.connected() && millis() < timeout)
  {
    if (client.available())
    {
      String line = client.readStringUntil('\n');

      line.trim();

      if (statusLine.length() == 0)
      {
        statusLine = line;

        Serial.print("[WS] Respuesta: ");
        Serial.println(statusLine);
      }

      if (line.length() == 0)
      {
        break;
      }
    }

    delay(5);
  }


  // ===================================================
  // COMPROBAR 101
  // ===================================================

  if (statusLine.indexOf("101") < 0)
  {
    Serial.println("[WS] ❌ HANDSHAKE RECHAZADO");

    client.stop();

    return false;
  }


  Serial.println("[WS] ✅ 101 SWITCHING PROTOCOLS");

  websocketConnected = true;

  return true;
}


// =====================================================
// PONG
// =====================================================

void sendPong()
{
  uint8_t mask[4];

  for (int i = 0; i < 4; i++)
  {
    mask[i] = random(0, 256);
  }


  // FIN + PONG
  client.write((uint8_t)0x8A);

  // MASK + payload length 0
  client.write((uint8_t)0x80);

  // Máscara
  client.write(mask, 4);


  Serial.println("[WS] PONG enviado");
}


// =====================================================
// PROCESAR WEBSOCKET
// =====================================================

void processWebSocket()
{
  if (!client.connected())
  {
    websocketConnected = false;

    Serial.println("[WS] ❌ CONEXION PERDIDA");

    return;
  }


  if (client.available() < 2)
    return;


  // ===================================================
  // HEADER
  // ===================================================

  uint8_t b1 = client.read();
  uint8_t b2 = client.read();


  uint8_t opcode = b1 & 0x0F;

  bool masked = b2 & 0x80;

  uint64_t payloadLength = b2 & 0x7F;


  // ===================================================
  // LONGITUD
  // ===================================================

  if (payloadLength == 126)
  {
    uint8_t ext[2];

    if (client.readBytes(ext, 2) != 2)
      return;

    payloadLength =
      ((uint16_t)ext[0] << 8) |
      ext[1];
  }

  else if (payloadLength == 127)
  {
    uint8_t ext[8];

    if (client.readBytes(ext, 8) != 8)
      return;

    payloadLength = 0;

    for (int i = 0; i < 8; i++)
    {
      payloadLength =
        (payloadLength << 8) |
        ext[i];
    }
  }


  // ===================================================
  // MASCARA
  // ===================================================

  uint8_t mask[4];

  if (masked)
  {
    if (client.readBytes(mask, 4) != 4)
      return;
  }


  // ===================================================
  // CLOSE
  // ===================================================

  if (opcode == 0x8)
  {
    Serial.println("[WS] CLOSE recibido");

    // Consumir payload
    for (uint64_t i = 0; i < payloadLength; i++)
    {
      while (!client.available())
        delay(1);

      client.read();
    }

    websocketConnected = false;

    client.stop();

    return;
  }


  // ===================================================
  // PING
  // ===================================================

  if (opcode == 0x9)
  {
    Serial.println("[WS] PING recibido");

    for (uint64_t i = 0; i < payloadLength; i++)
    {
      while (!client.available())
        delay(1);

      client.read();
    }

    sendPong();

    return;
  }


  // ===================================================
  // PONG
  // ===================================================

  if (opcode == 0xA)
  {
    Serial.println("[WS] PONG recibido");

    for (uint64_t i = 0; i < payloadLength; i++)
    {
      while (!client.available())
        delay(1);

      client.read();
    }

    return;
  }


  // ===================================================
  // LIMITE
  // ===================================================

  if (payloadLength > 4096)
  {
    Serial.println("[WS] ❌ FRAME DEMASIADO GRANDE");

    for (uint64_t i = 0; i < payloadLength; i++)
    {
      while (!client.available())
        delay(1);

      client.read();
    }

    return;
  }


  // ===================================================
  // LEER PAYLOAD
  // ===================================================

  uint8_t buffer[4097];

  size_t received = 0;


  while (received < payloadLength)
  {
    if (client.available())
    {
      uint8_t c = client.read();

      if (masked)
      {
        c ^= mask[received % 4];
      }

      buffer[received] = c;

      received++;
    }
    else
    {
      delay(1);
    }
  }


  buffer[received] = '\0';


  // ===================================================
  // TEXTO
  // ===================================================

  if (opcode == 0x1)
  {
    Serial.print("[WS] RX: ");

    Serial.write(buffer, received);

    Serial.println();


    // =================================================
    // JSON
    // =================================================

    JsonDocument doc;

    DeserializationError error =
      deserializeJson(doc, buffer, received);


    if (error)
    {
      Serial.print("[JSON] ❌ Error: ");

      Serial.println(error.c_str());

      return;
    }


    const char* type =
      doc["type"];


    // =================================================
    // ESTADO DE LOS LEDS
    // =================================================

    if (type &&
        strcmp(type, "status") == 0)
    {

      const char* led1 =
        doc["led1"];

      const char* led2 =
        doc["led2"];


      // -----------------------------------------------
      // LED 1 - GPIO 18
      // -----------------------------------------------

      if (led1 &&
          strcmp(led1, "ON") == 0)
      {
        digitalWrite(led1Pin, HIGH);

        Serial.println("[LED 18] 🟢 ON");
      }

      else if (led1 &&
               strcmp(led1, "OFF") == 0)
      {
        digitalWrite(led1Pin, LOW);

        Serial.println("[LED 18] 🔴 OFF");
      }


      // -----------------------------------------------
      // LED 2 - GPIO 19
      // -----------------------------------------------

      if (led2 &&
          strcmp(led2, "ON") == 0)
      {
        digitalWrite(led2Pin, HIGH);

        Serial.println("[LED 19] 🟢 ON");
      }

      else if (led2 &&
               strcmp(led2, "OFF") == 0)
      {
        digitalWrite(led2Pin, LOW);

        Serial.println("[LED 19] 🔴 OFF");
      }
    }
  }
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);


  // ===================================================
  // LEDS
  // ===================================================

  pinMode(led1Pin, OUTPUT);

  pinMode(led2Pin, OUTPUT);


  digitalWrite(led1Pin, LOW);

  digitalWrite(led2Pin, LOW);


  // ===================================================
  // RANDOM
  // ===================================================

  randomSeed(esp_random());


  // ===================================================
  // TITULO
  // ===================================================

  Serial.println();

  Serial.println("========================================");

  Serial.println("     ESP32 WSS - 2 LEDS");

  Serial.println("========================================");

  Serial.println();

  Serial.println("LED 1 -> GPIO 18");

  Serial.println("LED 2 -> GPIO 19");

  Serial.println();


  // ===================================================
  // WIFI
  // ===================================================

  Serial.println("[WiFi] Conectando...");

  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);

    Serial.print(".");
  }


  Serial.println();

  Serial.println("[WiFi] ✅ CONECTADO");


  Serial.print("[WiFi] IP: ");

  Serial.println(WiFi.localIP());


  // ===================================================
  // WEBSOCKET
  // ===================================================

  connectWebSocket();
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
  // ===================================================
  // WIFI
  // ===================================================

  if (WiFi.status() != WL_CONNECTED)
  {
    websocketConnected = false;

    delay(100);

    return;
  }


  // ===================================================
  // WEBSOCKET
  // ===================================================

  if (websocketConnected)
  {
    processWebSocket();
  }

  else
  {
    if (millis() - lastReconnect >= 5000)
    {
      lastReconnect = millis();

      connectWebSocket();
    }
  }


  delay(2);
}


