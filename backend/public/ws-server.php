<?php

// Evitar que los warnings/deprecated de PHP interfieran
// con la salida del servidor WebSocket.
error_reporting(E_ALL & ~E_DEPRECATED & ~E_NOTICE);
ini_set('display_errors', '0');

require dirname(__DIR__) . '/vendor/autoload.php';

use Ratchet\MessageComponentInterface;
use Ratchet\ConnectionInterface;
use Ratchet\Server\IoServer;
use Ratchet\Http\HttpServer;
use Ratchet\WebSocket\WsServer;

class LedWebSocketServer implements MessageComponentInterface
{
    protected $clients;
    protected $ledState;

    public function __construct()
    {
        $this->clients = new \SplObjectStorage;
        $this->ledState = false;

        echo "[WS] ======================================\n";
        echo "[WS] Servidor WebSocket iniciado\n";
        echo "[WS] Estado inicial LED: OFF\n";
        echo "[WS] ======================================\n";
    }

    public function onOpen(ConnectionInterface $conn)
    {
        $this->clients->attach($conn);

        echo "[WS] NUEVA CONEXION\n";
        echo "[WS] Resource ID: {$conn->resourceId}\n";

        $message = json_encode([
            'type' => 'status',
            'led' => $this->ledState ? 'ON' : 'OFF'
        ]);

        echo "[WS] Enviando estado: {$message}\n";

        $conn->send($message);
    }

    public function onMessage(ConnectionInterface $from, $msg)
    {
        echo "[WS] MENSAJE RECIBIDO\n";
        echo "[WS] Cliente: {$from->resourceId}\n";
        echo "[WS] Mensaje: {$msg}\n";

        $data = json_decode($msg, true);

        if (!is_array($data)) {
            echo "[WS] JSON invalido\n";
            return;
        }

        if (isset($data['command'])) {

            echo "[WS] Comando: {$data['command']}\n";

            if ($data['command'] === 'toggle') {

                $this->ledState = !$this->ledState;

                $message = json_encode([
                    'type' => 'status',
                    'led' => $this->ledState ? 'ON' : 'OFF'
                ]);

                echo "[WS] Nuevo estado LED: ";
                echo $this->ledState ? "ON\n" : "OFF\n";

                echo "[WS] Enviando a todos: {$message}\n";

                foreach ($this->clients as $client) {
                    $client->send($message);
                }
            }
        }
    }

    public function onClose(ConnectionInterface $conn)
    {
        echo "[WS] CONEXION CERRADA\n";
        echo "[WS] Resource ID: {$conn->resourceId}\n";

        $this->clients->detach($conn);
    }

    public function onError(ConnectionInterface $conn, \Exception $e)
    {
        echo "[WS] ERROR\n";
        echo "[WS] {$e->getMessage()}\n";

        $conn->close();
    }
}


// ========================================
// SERVIDOR WEBSOCKET
// ========================================

$port = getenv('PORT') ?: 8080;

echo "[WS] Puerto recibido: {$port}\n";
echo "[WS] Escuchando en 0.0.0.0:{$port}\n";

$server = IoServer::factory(
    new HttpServer(
        new WsServer(
            new LedWebSocketServer()
        )
    ),
    $port
);

echo "[WS] Servidor listo\n";

$server->run();