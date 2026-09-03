<?php

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

    protected $led1State;

    protected $led2State;


    public function __construct()
    {
        $this->clients = new \SplObjectStorage;


        // GPIO 18
        $this->led1State = false;


        // GPIO 19
        $this->led2State = false;


        echo "[WS] ======================================\n";
        echo "[WS] Servidor WebSocket iniciado\n";
        echo "[WS] LED 1 GPIO 18: OFF\n";
        echo "[WS] LED 2 GPIO 19: OFF\n";
        echo "[WS] ======================================\n";
    }


    // =================================================
    // ESTADO ACTUAL
    // =================================================

    protected function getStatus()
    {
        return json_encode([
            'type' => 'status',

            'led1' =>
                $this->led1State ? 'ON' : 'OFF',

            'led2' =>
                $this->led2State ? 'ON' : 'OFF'
        ]);
    }


    // =================================================
    // NUEVA CONEXION
    // =================================================

    public function onOpen(ConnectionInterface $conn)
    {
        $this->clients->attach($conn);


        echo "[WS] NUEVA CONEXION\n";

        echo "[WS] Resource ID: ";
        echo $conn->resourceId;
        echo "\n";


        $message =
            $this->getStatus();


        echo "[WS] Enviando estado: ";
        echo $message;
        echo "\n";


        $conn->send($message);
    }


    // =================================================
    // MENSAJE
    // =================================================

    public function onMessage(
        ConnectionInterface $from,
        $msg
    )
    {
        echo "[WS] MENSAJE RECIBIDO\n";

        echo "[WS] Cliente: ";
        echo $from->resourceId;
        echo "\n";

        echo "[WS] Mensaje: ";
        echo $msg;
        echo "\n";


        $data =
            json_decode($msg, true);


        if (!is_array($data))
        {
            echo "[WS] JSON invalido\n";

            return;
        }


        if (!isset($data['command']))
        {
            echo "[WS] Comando no especificado\n";

            return;
        }


        $command =
            $data['command'];


        echo "[WS] Comando: ";
        echo $command;
        echo "\n";


        // =================================================
        // LED 1 GPIO 18
        // =================================================

        if ($command === 'led1_on')
        {
            $this->led1State = true;
        }

        else if ($command === 'led1_off')
        {
            $this->led1State = false;
        }


        // =================================================
        // LED 2 GPIO 19
        // =================================================

        else if ($command === 'led2_on')
        {
            $this->led2State = true;
        }

        else if ($command === 'led2_off')
        {
            $this->led2State = false;
        }


        // =================================================
        // TOGGLE LED 1
        // =================================================

        else if ($command === 'led1_toggle')
        {
            $this->led1State =
                !$this->led1State;
        }


        // =================================================
        // TOGGLE LED 2
        // =================================================

        else if ($command === 'led2_toggle')
        {
            $this->led2State =
                !$this->led2State;
        }


        else
        {
            echo "[WS] Comando desconocido\n";

            return;
        }


        // =================================================
        // NUEVO ESTADO
        // =================================================

        $message =
            $this->getStatus();


        echo "[WS] Nuevo estado:\n";

        echo "[WS] GPIO 18: ";

        echo $this->led1State
            ? "ON\n"
            : "OFF\n";


        echo "[WS] GPIO 19: ";

        echo $this->led2State
            ? "ON\n"
            : "OFF\n";


        // =================================================
        // ENVIAR A TODOS
        // =================================================

        foreach ($this->clients as $client)
        {
            $client->send($message);
        }
    }


    // =================================================
    // CERRAR
    // =================================================

    public function onClose(
        ConnectionInterface $conn
    )
    {
        echo "[WS] CONEXION CERRADA\n";

        echo "[WS] Resource ID: ";
        echo $conn->resourceId;
        echo "\n";


        $this->clients->detach($conn);
    }


    // =================================================
    // ERROR
    // =================================================

    public function onError(
        ConnectionInterface $conn,
        \Exception $e
    )
    {
        echo "[WS] ERROR\n";

        echo "[WS] ";
        echo $e->getMessage();
        echo "\n";


        $conn->close();
    }
}


// =====================================================
// SERVIDOR
// =====================================================

$port =
    getenv('PORT') ?: 8080;


echo "[WS] Puerto recibido: ";
echo $port;
echo "\n";


echo "[WS] Escuchando en 0.0.0.0:";
echo $port;
echo "\n";


$server =
    IoServer::factory(
        new HttpServer(
            new WsServer(
                new LedWebSocketServer()
            )
        ),
        $port
    );


echo "[WS] Servidor listo\n";


$server->run();
