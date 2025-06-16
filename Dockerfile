# Usamos PHP CLI base, versión 8.1 (puedes cambiar versión si quieres)
FROM php:8.1-cli

# Instalar librerías necesarias (puedes agregar más según dependencias)
RUN apt-get update && apt-get install -y \
  libzip-dev \
  unzip \
  && docker-php-ext-install zip \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/*

# Instalar Composer
COPY --from=composer:latest /usr/bin/composer /usr/bin/composer

# Establecer directorio de trabajo
WORKDIR /app

# Copiar todo el código del proyecto al contenedor
COPY . /app

# Cambiar directorio a backend para instalar dependencias con Composer
WORKDIR /app/backend

RUN composer install --no-dev --optimize-autoloader

# Exponer puerto 8080 (o el que uses)
EXPOSE 8080

# Establecer variable de entorno PORT para la app
ENV PORT 8080

# Comando para iniciar el servidor WebSocket PHP
CMD ["php", "server.php"]
