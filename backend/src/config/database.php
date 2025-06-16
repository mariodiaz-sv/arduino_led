<?php
function connectDB()
{
  $dbPath = __DIR__ . '/../../database/db.sqlite';
  try {
    $pdo = new PDO("sqlite:" . $dbPath);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    return $pdo;
  } catch (PDOException $e) {
    die("Error de conexión: " . $e->getMessage());
  }
}
