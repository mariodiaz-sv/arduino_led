<?php
require dirname(__DIR__) . '/vendor/autoload.php';

use Ratchet\MessageComponentInterface;
use Ratchet\ConnectionInterface;

class LedWebSocketServer implements MessageComponentInterface
{
  protected $clients;
  protected $ledState; // true = encendido, false = apagado

  public function __construct()
  {
    $this->clients = new \SplObjectStorage;
    $this->ledState = false;
    echo "Servidor WebSocket iniciado\n";
  }

  public function onOpen(ConnectionInterface $conn)
  {
    $this->clients->attach($conn);
    echo "Nueva conexión: ({$conn->resourceId})\n";

    // Enviar estado actual del LED al nuevo cliente
    $conn->send(json_encode([
      'type' => 'status',
      'led' => $this->ledState ? 'ON' : 'OFF'
    ]));
  }

  public function onMessage(ConnectionInterface $from, $msg)
  {
    echo "Mensaje recibido de {$from->resourceId}: $msg\n";

    $data = json_decode($msg, true);
    if (!$data) return;

    if (isset($data['command'])) {
      if ($data['command'] === 'toggle') {
        $this->ledState = !$this->ledState;

        // Enviar a todos clientes el nuevo estado
        foreach ($this->clients as $client) {
          $client->send(json_encode([
            'type' => 'status',
            'led' => $this->ledState ? 'ON' : 'OFF'
          ]));
        }
      }
    }
  }

  public function onClose(ConnectionInterface $conn)
  {
    $this->clients->detach($conn);
    echo "Conexión cerrada: ({$conn->resourceId})\n";
  }

  public function onError(ConnectionInterface $conn, \Exception $e)
  {
    echo "Error: {$e->getMessage()}\n";
    $conn->close();
  }
}

// Configuración del servidor WebSocket
use Ratchet\Server\IoServer;
use Ratchet\Http\HttpServer;
use Ratchet\WebSocket\WsServer;


$port = getenv('PORT') ?: 8080; // puerto WebSocket

$server = IoServer::factory(
  new HttpServer(
    new WsServer(
      new LedWebSocketServer()
    )
  ),
  $port
);

$host = getenv('RENDER') ? '0.0.0.0' : 'localhost';
echo "Servidor WebSocket corriendo en ws://$host:$port\n";
$server->run();
