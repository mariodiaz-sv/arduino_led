<?php
require_once __DIR__ . '/../config/database.php';

class User
{
  private $db;

  public function __construct()
  {
    $this->db = connectDB();
  }

  public function findByUsername($username)
  {
    $stmt = $this->db->prepare("SELECT * FROM users WHERE username = ?");
    $stmt->execute([$username]);
    return $stmt->fetch(PDO::FETCH_ASSOC);
  }

  public function create($username, $password, $role = 'user')
  {
    $hashed = password_hash($password, PASSWORD_DEFAULT);
    $stmt = $this->db->prepare("INSERT INTO users (username, password, role) VALUES (?, ?, ?)");
    return $stmt->execute([$username, $hashed, $role]);
  }
}
