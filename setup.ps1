<#
.SYNOPSIS
    Script de instalación de herramientas de desarrollo para Windows 11
.DESCRIPTION
    Instala herramientas comunes de desarrollo (PHP, Composer, Git, Node.js, Arduino CLI, VS Code)
    usando Chocolatey como gestor de paquetes.
.NOTES
    Requiere ejecución como administrador
#>

# Configuración inicial
$ErrorActionPreference = "Stop"

# Guardar la política de ejecución actual
$originalExecutionPolicy = Get-ExecutionPolicy -Scope LocalMachine

# Habilitar ejecución de scripts temporalmente
try {
  Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process -Force -ErrorAction Stop
  Write-Host "Política de ejecución temporalmente configurada como Bypass para este proceso." -ForegroundColor Green
}
catch {
  Write-Host ("Error al configurar la política de ejecución: {0}" -f $_.Exception.Message) -ForegroundColor Red
  exit 1
}

function Show-Menu {
  Clear-Host
  Write-Host "======================================" -ForegroundColor Cyan
  Write-Host " INSTALADOR DE HERRAMIENTAS DE DESARROLLO " -ForegroundColor Cyan
  Write-Host "======================================" -ForegroundColor Cyan
  Write-Host "1. Instalar TODAS las herramientas" -ForegroundColor Green
  Write-Host "2. Seleccionar herramientas a instalar" -ForegroundColor Yellow
  Write-Host "3. Salir" -ForegroundColor Red
  Write-Host "======================================" -ForegroundColor Cyan
}

function Show-ToolsMenu {
  Clear-Host
  Write-Host "======================================" -ForegroundColor Cyan
  Write-Host " SELECCIONA HERRAMIENTAS A INSTALAR " -ForegroundColor Cyan
  Write-Host "======================================" -ForegroundColor Cyan
  Write-Host "1. PHP [ ]" -ForegroundColor Yellow
  Write-Host "2. Composer [ ]" -ForegroundColor Yellow
  Write-Host "3. Git [ ]" -ForegroundColor Yellow
  Write-Host "4. Node.js [ ]" -ForegroundColor Yellow
  Write-Host "5. Arduino CLI [ ]" -ForegroundColor Yellow
  Write-Host "6. Visual Studio Code [ ]" -ForegroundColor Yellow
  Write-Host "7. Continuar con la instalación" -ForegroundColor Green
  Write-Host "8. Cancelar" -ForegroundColor Red
  Write-Host "======================================" -ForegroundColor Cyan
}

# Verificar si es administrador
if (-NOT ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
  Write-Host "Por favor ejecuta este script como administrador" -ForegroundColor Red
  exit 1
}

# Variables para selección de herramientas
$installAll = $false
$toolsToInstall = @{
  PHP        = $false
  Composer   = $false
  Git        = $false
  NodeJS     = $false
  ArduinoCLI = $false
  VSCode     = $false
}

# Mostrar menú principal
do {
  Show-Menu
  $mainChoice = Read-Host "Selecciona una opcion (1-3)"

  switch ($mainChoice) {
    "1" {
      $installAll = $true
      foreach ($key in $toolsToInstall.Keys) {
        $toolsToInstall[$key] = $true
      }
      break
    }
    "2" {
      do {
        Show-ToolsMenu
        $selectionStatus = @()
        foreach ($tool in $toolsToInstall.Keys) {
          $status = if ($toolsToInstall[$tool]) { "X" } else { " " }
          $selectionStatus += "$tool [$status]"
        }
        Write-Host "Selección actual: " -NoNewline
        Write-Host ($selectionStatus -join ", ") -ForegroundColor Magenta

        $toolChoice = Read-Host "Selecciona una herramienta (1-8)"
        switch ($toolChoice) {
          "1" { $toolsToInstall.PHP = -not $toolsToInstall.PHP }
          "2" { $toolsToInstall.Composer = -not $toolsToInstall.Composer }
          "3" { $toolsToInstall.Git = -not $toolsToInstall.Git }
          "4" { $toolsToInstall.NodeJS = -not $toolsToInstall.NodeJS }
          "5" { $toolsToInstall.ArduinoCLI = -not $toolsToInstall.ArduinoCLI }
          "6" { $toolsToInstall.VSCode = -not $toolsToInstall.VSCode }
          "7" { break }
          "8" { exit 0 }
          default { Write-Host "Opción no válida. Intenta nuevamente." -ForegroundColor Red }
        }
      } while ($toolChoice -ne "7")
      break
    }
    "3" { exit 0 }
    default { Write-Host "Opción no válida. Intenta nuevamente." -ForegroundColor Red }
  }
} while (-not ($installAll -or ($toolsToInstall.Values -contains $true)))

# Verificar si hay herramientas seleccionadas
if (-not ($installAll -or ($toolsToInstall.Values -contains $true))) {
  Write-Host "No se seleccionaron herramientas para instalar. Saliendo..." -ForegroundColor Yellow
  exit 0
}

# Instalar Chocolatey (si no está instalado)
try {
  if (!(Test-Path -Path "$env:ProgramData\chocolatey\choco.exe")) {
    Write-Host "Instalando Chocolatey..." -ForegroundColor Cyan
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
    Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
  }
  else {
    Write-Host "Chocolatey ya está instalado. Actualizando..." -ForegroundColor Yellow
    choco upgrade chocolatey -y
  }
}
catch {
  Write-Host ("Error al instalar Chocolatey: {0}" -f $_.Exception.Message) -ForegroundColor Red
  exit 1
}

# Función para instalar paquetes con manejo de errores
function Install-Package {
  param (
    [string]$packageName,
    [string]$displayName,
    [switch]$isNeeded
  )

  if (-not $isNeeded) {
    Write-Host "Saltando instalación de $displayName..." -ForegroundColor Gray
    return
  }

  try {
    Write-Host "Instalando $displayName..." -ForegroundColor Cyan
    choco install $packageName -y --no-progress
    Write-Host "$displayName instalado correctamente." -ForegroundColor Green

    # Actualizar PATH para paquetes recién instalados
    if ($packageName -eq "composer") {
      $composerPath = Join-Path $env:ProgramData "ComposerSetup\bin"
      $env:Path += ";$composerPath"
    }
  }
  catch {
    Write-Host ("Error al instalar {0}: {1}" -f $displayName, $_.Exception.Message) -ForegroundColor Red
  }
}

# Instalar herramientas seleccionadas
$installedVersions = @{}

if ($toolsToInstall.PHP) {
  Install-Package -packageName "php" -displayName "PHP" -isNeeded:$true
  $phpVersion = & php -v 2>$null | Select-Object -First 1
  if ($phpVersion) { $installedVersions.PHP = $phpVersion }
}

if ($toolsToInstall.Composer) {
  Install-Package -packageName "composer" -displayName "Composer" -isNeeded:$true

  # Verificar Composer con la ruta completa
  $composerPath = Join-Path $env:ProgramData "ComposerSetup\bin\composer.phar"
  if (Test-Path $composerPath) {
    $composerVersion = & php $composerPath --version 2>$null
    if ($composerVersion) { $installedVersions.Composer = $composerVersion }
  }
  else {
    Write-Host "No se pudo verificar la versión de Composer. Es posible que necesites reiniciar." -ForegroundColor Yellow
  }
}

if ($toolsToInstall.Git) {
  Install-Package -packageName "git" -displayName "Git" -isNeeded:$true
  $gitVersion = & git --version 2>$null
  if ($gitVersion) { $installedVersions.Git = $gitVersion }
}

if ($toolsToInstall.NodeJS) {
  Install-Package -packageName "nodejs" -displayName "Node.js" -isNeeded:$true
  $nodeVersion = & node --version 2>$null
  $npmVersion = & npm --version 2>$null
  if ($nodeVersion) { $installedVersions.NodeJS = "Node $nodeVersion, npm $npmVersion" }
}

if ($toolsToInstall.ArduinoCLI) {
  Install-Package -packageName "arduino-cli" -displayName "Arduino CLI" -isNeeded:$true
  $arduinoVersion = & arduino-cli version 2>$null | Out-String
  if ($arduinoVersion) { $installedVersions.ArduinoCLI = $arduinoVersion.Trim() }
}

if ($toolsToInstall.VSCode) {
  Install-Package -packageName "vscode" -displayName "Visual Studio Code" -isNeeded:$true
  $vscodeVersion = & code --version 2>$null | Select-Object -First 1
  if ($vscodeVersion) { $installedVersions.VSCode = $vscodeVersion }
}

# Mostrar resumen
Write-Host '`n======================================' -ForegroundColor Cyan
Write-Host ' RESUMEN DE INSTALACIÓN ' -ForegroundColor Cyan
Write-Host '======================================' -ForegroundColor Cyan

if ($installedVersions.Count -gt 0) {
  Write-Host 'Versiones instaladas:' -ForegroundColor Green
  foreach ($key in $installedVersions.Keys) {
    Write-Host '- $key : $($installedVersions[$key])' -ForegroundColor Yellow
  }
}
else {
  Write-Host 'No se instalaron nuevas herramientas.' -ForegroundColor Yellow
}

# Restaurar política de ejecución
try {
  Set-ExecutionPolicy -ExecutionPolicy $originalExecutionPolicy -Scope LocalMachine -Force -ErrorAction Stop
  Write-Host '`nPolítica de ejecución restaurada a: $originalExecutionPolicy' -ForegroundColor Green
}
catch {
  Write-Host ('Error al restaurar la política de ejecución: {0}' -f $_.Exception.Message) -ForegroundColor Red
}

# Mensaje final actualizado
Write-Host '`n¡Proceso completado!' -ForegroundColor Green
Write-Host 'Recomendaciones:' -ForegroundColor Yellow
Write-Host '1. Reinicia tu terminal para aplicar los cambios de PATH' -ForegroundColor Yellow
Write-Host '2. Si algunos comandos no funcionan, reinicia tu computadora' -ForegroundColor Yellow
Write-Host '3. Verifica las instalaciones con:' -ForegroundColor Yellow
Write-Host '   php -v' -ForegroundColor Cyan
Write-Host '   composer --version' -ForegroundColor Cyan
Write-Host '   git --version' -ForegroundColor Cyan
