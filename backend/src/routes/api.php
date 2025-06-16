<?php
require_once __DIR__ . '/../controllers/AuthController.php';

$uri = $_SERVER['REQUEST_URI'];
$method = $_SERVER['REQUEST_METHOD'];
$input = json_decode(file_get_contents("php://input"), true);
$auth = new AuthController();

if ($uri === '/login' && $method === 'POST') {
  echo $auth->login($input);
} elseif ($uri === '/register' && $method === 'POST') {
  echo $auth->register($input);
} else {
  http_response_code(404);
  echo json_encode(["status" => "error", "message" => "Ruta no encontrada"]);
}
