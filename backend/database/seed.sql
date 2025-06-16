CREATE TABLE IF NOT EXISTS users (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  username TEXT NOT NULL UNIQUE,
  password TEXT NOT NULL,
  role TEXT NOT NULL CHECK(role IN ('admin', 'user'))
);

INSERT INTO users (username, password, role) VALUES
('admin', '$2y$10$e0MYzXyjpJS7Pd0RVvHwHeFQ0W8Hkjg0cfGqXwGJw5Duz8Y8Y8FZC', 'admin');
-- password: admin123
