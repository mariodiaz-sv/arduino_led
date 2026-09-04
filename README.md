# ⚡ ESP32 LED Control System

[![Platform](https://img.shields.io/badge/Platform-ESP32-E7352C?logo=espressif)](https://www.espressif.com/)
[![Language](https://img.shields.io/badge/Arduino-C%2B%2B-00979D?logo=arduino)](https://www.arduino.cc/)
[![Backend](https://img.shields.io/badge/Backend-PHP%208.3-777BB4?logo=php)](https://www.php.net/)
[![WebSocket](https://img.shields.io/badge/Realtime-WebSocket-111827)](https://developer.mozilla.org/en-US/docs/Web/API/WebSocket)
[![Docker](https://img.shields.io/badge/Docker-Ready-2496ED?logo=docker)](https://www.docker.com/)
[![Deployment](https://img.shields.io/badge/Deployment-Render-46E3B7)](https://render.com/)

Sistema IoT educativo y demostrativo para controlar **dos LEDs conectados a un ESP32 mediante una interfaz web en tiempo real**.

El proyecto integra electrónica, programación embebida, comunicación WebSocket, backend PHP, autenticación básica, Docker y despliegue en la nube. La arquitectura separa el dispositivo físico, el servidor de comunicación y la interfaz web.

> **Estado:** funcional / proyecto educativo y de demostración.

---

## 📸 Vista general

> Coloca aquí las imágenes generadas con Nano Banana:
>
> - `docs/images/hero.png` — imagen principal del proyecto.
> - `docs/images/architecture.png` — arquitectura completa.
> - `docs/images/circuit.png` — conexión ESP32 + LEDs.
> - `docs/images/interface.png` — interfaz web.

![ESP32 LED Control System](docs/images/hero.png)

---

## 🎯 Objetivo

Demostrar cómo un dispositivo físico basado en ESP32 puede ser controlado desde una aplicación web utilizando una comunicación persistente mediante **WebSocket sobre TLS (`wss://`)**.

El flujo principal es:

```text
┌─────────────────────┐
│    Navegador Web    │
│ HTML + CSS + JS     │
└──────────┬──────────┘
           │ WSS
           ▼
┌─────────────────────┐
│  Servidor WebSocket │
│ PHP + Ratchet       │
└──────────┬──────────┘
           │ WSS
           ▼
┌─────────────────────┐
│        ESP32        │
│ WiFi + WebSocket    │
└──────────┬──────────┘
           │ GPIO
      ┌────┴────┐
      ▼         ▼
   LED 1      LED 2
 GPIO 18     GPIO 19
```

---

## ✨ Características

- Control remoto de dos LEDs.
- Comunicación bidireccional en tiempo real.
- WebSocket seguro mediante `wss://`.
- ESP32 conectado por Wi-Fi.
- Indicadores visuales del estado de cada LED.
- Reconexión automática del navegador si se pierde WebSocket.
- Servidor WebSocket desarrollado en PHP con Ratchet.
- Backend PHP con estructura separada de controladores, modelos, rutas y configuración.
- Autenticación básica con usuario y contraseña.
- Contraseñas almacenadas mediante `password_hash()`.
- Base de datos SQLite.
- Contenedores Docker independientes para HTTP y WebSocket.
- Configuración de despliegue para Render.
- Código pensado para aprendizaje de IoT, redes, backend y DevOps.

---

## 🧩 Tecnologías

| Capa | Tecnología |
|---|---|
| Microcontrolador | ESP32 |
| Firmware | Arduino / C++ |
| Conectividad | Wi-Fi |
| Comunicación | WebSocket / WSS |
| Mensajería | JSON |
| Backend | PHP 8.3 |
| WebSocket Server | Ratchet |
| Persistencia | SQLite |
| Frontend | HTML5, CSS3, JavaScript |
| UI | Bootstrap 5 |
| Contenedores | Docker |
| CI/CD | GitHub Actions |
| Cloud | Render |

---

## 🔌 Hardware

### Componentes

- 1 × ESP32
- 2 × LEDs
- 2 × resistencias apropiadas para los LEDs
- Protoboard
- Cables Dupont
- Cable USB para programación y alimentación
- Computadora para configurar el firmware

### Asignación de GPIO

| Dispositivo | GPIO |
|---|---:|
| LED 1 | 18 |
| LED 2 | 19 |

> **Importante:** utiliza una resistencia limitadora de corriente adecuada para cada LED. No conectes un LED directamente a un GPIO sin resistencia.

---

## 🔄 Funcionamiento

### 1. Interfaz web

La interfaz crea una conexión:

```javascript
const wsUrl = 'wss://sistema-websocket.onrender.com';
const ws = new WebSocket(wsUrl);
```

Cuando la conexión está activa, los botones de control quedan habilitados.

Los comandos enviados son JSON:

```json
{
  "command": "led1_on"
}
```

Ejemplos:

```json
{"command":"led1_on"}
{"command":"led1_off"}
{"command":"led2_on"}
{"command":"led2_off"}
```

También existen comandos de alternancia en el servidor:

```json
{"command":"led1_toggle"}
{"command":"led2_toggle"}
```

### 2. Servidor WebSocket

El servidor PHP mantiene una lista de clientes conectados y conserva el estado lógico de ambos LEDs.

Cuando recibe un comando:

```text
Browser
   │
   │ {"command":"led1_on"}
   ▼
WebSocket Server
   │
   │ actualiza estado
   ▼
ESP32
   │
   ▼
GPIO 18 = HIGH
```

Después, el servidor difunde el nuevo estado a los clientes conectados:

```json
{
  "type": "status",
  "led1": "ON",
  "led2": "OFF"
}
```

### 3. ESP32

El firmware se conecta a Wi-Fi y posteriormente establece el WebSocket seguro con el servidor.

El ESP32 procesa mensajes JSON y controla:

```cpp
digitalWrite(led1Pin, HIGH);
digitalWrite(led2Pin, HIGH);
```

Los pines utilizados son:

```cpp
const int led1Pin = 18;
const int led2Pin = 19;
```

---

## 🏗️ Arquitectura

![Arquitectura del sistema](docs/images/architecture.png)

### Componentes principales

#### ESP32

Responsable de:

- conexión Wi-Fi;
- conexión WSS;
- handshake WebSocket;
- recepción de frames;
- desenmascarado de mensajes;
- interpretación JSON;
- control de GPIO;
- respuesta PONG;
- detección de cierre y pérdida de conexión.

El firmware implementa manualmente parte del protocolo WebSocket en lugar de depender de una biblioteca WebSocket de alto nivel.

#### WebSocket Server

Implementado en PHP mediante:

```text
cboden/ratchet
```

El servidor:

- acepta conexiones;
- mantiene clientes conectados;
- recibe comandos;
- modifica el estado lógico;
- envía el estado actualizado a todos los clientes;
- responde a eventos de cierre y error.

#### Frontend

Aplicación web estática basada en:

- HTML;
- CSS;
- JavaScript;
- Bootstrap 5.

La interfaz muestra:

- estado de conexión;
- estado del LED 1;
- estado del LED 2;
- botones para encender/apagar.

---

## 📁 Estructura del proyecto

```text
arduino_led/
│
├── .github/
│   └── workflows/
│
├── arduino/
│   └── WebSocket_ESP32.ino
│
├── backend/
│   ├── database/
│   │   ├── db.sqlite
│   │   └── seed.sql
│   │
│   ├── public/
│   │   └── ws-server.php
│   │
│   ├── src/
│   │   ├── config/
│   │   │   └── database.php
│   │   ├── controllers/
│   │   │   └── AuthController.php
│   │   ├── models/
│   │   │   └── User.php
│   │   └── routes/
│   │       └── api.php
│   │
│   ├── composer.json
│   └── composer.lock
│
├── frontend/
│   ├── css/
│   ├── js/
│   │   └── app.js
│   └── index.html
│
├── Dockerfile
├── Dockerfile.websocket
├── render.yaml
├── render-worker.yaml
├── setup.ps1
└── README.md
```

---

## 🔐 Autenticación

El backend incluye una implementación básica de autenticación.

### Login

```http
POST /login
```

Ejemplo:

```json
{
  "username": "admin",
  "password": "password"
}
```

Respuesta exitosa:

```json
{
  "status": "success",
  "user": {
    "id": 1,
    "username": "admin",
    "role": "user"
  }
}
```

### Registro

```http
POST /register
```

Ejemplo:

```json
{
  "username": "usuario",
  "password": "password",
  "role": "user"
}
```

Las contraseñas se procesan mediante `password_hash()` antes de almacenarse.

> **Advertencia de seguridad:** la autenticación existente es una base educativa. No debe considerarse un sistema de autenticación de producción sin incorporar sesiones o tokens, autorización real, validación de entrada, rate limiting, CORS/CSRF según el caso, gestión de secretos y otras medidas de seguridad.

---

## 🛠️ Instalación local

### Requisitos

Instala:

- Arduino IDE
- ESP32 Board Package
- PHP 8.3+
- Composer
- Git
- Docker opcionalmente
- Una red Wi-Fi disponible para el ESP32

### Clonar

```bash
git clone https://github.com/mariodiaz-sv/arduino_led.git
cd arduino_led
```

---

## ⚙️ Configurar ESP32

Abre:

```text
arduino/WebSocket_ESP32.ino
```

Configura:

```cpp
const char *ssid = "TU SSID";
const char *password = "TU PASSWORD";
```

Y establece el host del servidor:

```cpp
const char* websocketHost = "TU-SERVIDOR-WEBSOCKET";
```

Puerto:

```cpp
const uint16_t websocketPort = 443;
```

Después:

1. Selecciona tu placa ESP32.
2. Selecciona el puerto COM correspondiente.
3. Compila el proyecto.
4. Carga el firmware.
5. Abre el monitor serial a `115200`.
6. Verifica la conexión Wi-Fi.
7. Verifica el handshake WebSocket.

---

## 🐘 Ejecutar backend

Desde:

```bash
cd backend
```

Instala dependencias:

```bash
composer install
```

Servidor HTTP:

```bash
php -S localhost:8000 -t public
```

---

## 🔌 Ejecutar WebSocket

Desde `backend`:

```bash
php public/ws-server.php
```

El servidor utiliza el puerto proporcionado mediante:

```text
PORT
```

y, si no existe, utiliza:

```text
8080
```

---

## 🐳 Docker

El proyecto incorpora dos imágenes:

### HTTP

```text
Dockerfile
```

Ejecuta el backend PHP utilizando el servidor embebido de PHP.

### WebSocket

```text
Dockerfile.websocket
```

Ejecuta:

```text
php public/ws-server.php
```

Construcción:

```bash
docker build -t arduino-led-http .
docker build -f Dockerfile.websocket -t arduino-led-websocket .
```

---

## ☁️ Despliegue en Render

El repositorio incluye:

```text
render.yaml
render-worker.yaml
```

El servicio HTTP utiliza:

```text
Dockerfile
```

Mientras que el servicio WebSocket utiliza:

```text
Dockerfile.websocket
```

El worker WebSocket se inicia mediante:

```bash
cd backend && php public/ws-server.php
```

### Arquitectura cloud

```text
                    INTERNET
                       │
             ┌─────────┴─────────┐
             │                   │
             ▼                   ▼
      Frontend estático     Render Web Service
             │                   │
             │ WSS               │
             └─────────┬─────────┘
                       ▼
                WebSocket Worker
                       │
                       │ Internet
                       ▼
                     ESP32
```

> En producción se recomienda separar claramente el servicio HTTP/API del servicio persistente WebSocket y gestionar secretos mediante variables de entorno.

---

## 🧪 Flujo de prueba

### Prueba 1 — ESP32

Verifica en el monitor serial:

```text
[WiFi] CONECTADO
[WiFi] IP: ...
[WS] 101 SWITCHING PROTOCOLS
```

### Prueba 2 — Navegador

Abre la interfaz web y espera:

```text
🟢 Conectado al servidor
```

### Prueba 3 — LED 1

Presiona:

```text
Encender
```

Resultado:

```text
GPIO 18 → HIGH
LED 1 → ON
```

### Prueba 4 — LED 2

Presiona:

```text
Encender
```

Resultado:

```text
GPIO 19 → HIGH
LED 2 → ON
```

---

## 📡 Protocolo de mensajes

### Comandos del cliente

| Comando | Acción |
|---|---|
| `led1_on` | Enciende LED 1 |
| `led1_off` | Apaga LED 1 |
| `led2_on` | Enciende LED 2 |
| `led2_off` | Apaga LED 2 |
| `led1_toggle` | Alterna LED 1 |
| `led2_toggle` | Alterna LED 2 |

### Estado

```json
{
  "type": "status",
  "led1": "ON",
  "led2": "OFF"
}
```

---

## 🔒 Consideraciones de seguridad

Este proyecto está orientado a aprendizaje y demostración.

Antes de utilizarlo en un entorno real se recomienda:

- eliminar credenciales del código fuente;
- utilizar variables de entorno;
- no almacenar secretos en Git;
- agregar autenticación al canal WebSocket;
- validar y autorizar comandos;
- limitar el tamaño de mensajes;
- aplicar rate limiting;
- configurar CORS correctamente;
- utilizar TLS;
- proteger endpoints de autenticación;
- registrar eventos de seguridad;
- utilizar una base de datos de producción adecuada;
- implementar manejo formal de sesiones o tokens;
- separar configuración de desarrollo y producción.

---

## 📈 Evolución recomendada

Este proyecto puede evolucionar hacia una plataforma IoT más completa.

### Fase 1 — Actual

```text
ESP32
  ↓
WebSocket
  ↓
PHP Ratchet
  ↓
Frontend
```

### Fase 2 — API moderna

```text
Vue 3
  ↓
Laravel API
  ↓
WebSocket / Realtime
  ↓
ESP32
```

### Fase 3 — Producción

```text
Vue 3
   │
   ▼
Laravel API ───── PostgreSQL
   │
   ├──── Auth / Sanctum
   │
   └──── WebSocket / Realtime
               │
               ▼
             ESP32
```

### Posibles ampliaciones

- múltiples ESP32;
- identificación de dispositivos;
- dashboard;
- usuarios y roles;
- grupos de dispositivos;
- historial de eventos;
- sensores;
- relés;
- servomotores;
- temperatura y humedad;
- MQTT;
- notificaciones;
- actualización OTA;
- monitoreo;
- métricas;
- logs;
- Docker Compose;
- CI/CD;
- Kubernetes.

---

## 🧠 Valor educativo

El proyecto permite estudiar de forma integrada:

- programación C++;
- microcontroladores;
- GPIO;
- redes Wi-Fi;
- JSON;
- WebSocket;
- HTTP;
- TLS;
- PHP;
- arquitectura cliente-servidor;
- bases de datos;
- autenticación;
- Docker;
- despliegue cloud;
- CI/CD;
- fundamentos DevOps;
- Internet of Things.

---

## 📝 Notas técnicas

El firmware implementa directamente el handshake y procesamiento de frames WebSocket, incluyendo:

- generación de clave WebSocket;
- handshake HTTP;
- comprobación `101 Switching Protocols`;
- lectura de payloads;
- desenmascarado;
- frames de texto;
- PING/PONG;
- CLOSE;
- límite de payload;
- procesamiento JSON.

Esto convierte al proyecto en un ejemplo interesante para estudiar qué ocurre debajo de una biblioteca WebSocket de alto nivel.

---

## 🤝 Contribuciones

Las contribuciones son bienvenidas.

Puedes:

1. Crear un fork.
2. Crear una rama:

```bash
git checkout -b feature/nueva-funcionalidad
```

3. Realizar cambios.
4. Hacer commit:

```bash
git commit -m "feat: nueva funcionalidad"
```

5. Crear un Pull Request.

---

## 👨‍💻 Autor

**Mario Diaz**

Ingeniero en Computación  
Desarrollo de Software · DevOps · IoT · Electrónica

GitHub:

https://github.com/mariodiaz-sv

Repositorio:

https://github.com/mariodiaz-sv/arduino_led

---

## 📄 Licencia

Consulta la licencia incluida en el repositorio antes de reutilizar o redistribuir el proyecto.

---

## ⭐ Si este proyecto te resulta útil

Considera darle una estrella al repositorio y compartirlo con otros estudiantes y desarrolladores interesados en Arduino, ESP32, IoT, WebSocket y DevOps.

---

### Tecnologías destacadas

`ESP32` `Arduino` `C++` `WiFi` `WebSocket` `WSS` `JSON` `PHP` `Ratchet` `SQLite` `Bootstrap` `Docker` `Render` `GitHub Actions` `IoT` `DevOps`
