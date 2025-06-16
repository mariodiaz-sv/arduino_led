<?php
require_once __DIR__ . '/../models/User.php';

class AuthController
{
  public function login($data)
  {
    $userModel = new User();
    $user = $userModel->findByUsername($data['username']);

    if ($user && password_verify($data['password'], $user['password'])) {
      return json_encode([
        "status" => "success",
        "user" => [
          "id" => $user['id'],
          "username" => $user['username'],
          "role" => $user['role']
        ]
      ]);
    } else {
      http_response_code(401);
      return json_encode(["status" => "error", "message" => "Credenciales incorrectas"]);
    }
  }

  public function register($data)
  {
    $userModel = new User();
    $success = $userModel->create($data['username'], $data['password'], $data['role'] ?? 'user');
    return json_encode(["status" => $success ? "success" : "error"]);
  }
}
