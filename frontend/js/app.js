const wsUrl = 'ws://localhost:8080'; // Cambia por tu dominio en producción

let ws;

const ledStatusEl = document.getElementById('led-status');
const btnEncender = document.getElementById('btn-encender');
const btnApagar = document.getElementById('btn-apagar');

function conectarWebSocket() {
  ws = new WebSocket(wsUrl);

  ws.onopen = () => {
    console.log('✅ Conectado al WebSocket');
  };

  ws.onmessage = (event) => {
    try {
      const data = JSON.parse(event.data);
      if (data.type === 'status') {
        actualizarEstadoLed(data.led);
      }
    } catch (error) {
      console.error('❌ Error al parsear mensaje WebSocket', error);
    }
  };

  ws.onclose = () => {
    console.warn('⚠️ Conexión cerrada, reintentando en 5s...');
    deshabilitarBotones();
    ledStatusEl.textContent = 'Desconectado';
    setTimeout(conectarWebSocket, 5000);
  };

  ws.onerror = (err) => {
    console.error('❌ WebSocket Error', err);
  };
}

function actualizarEstadoLed(estado) {
  ledStatusEl.textContent = estado;
  if (estado === 'ON') {
    btnEncender.disabled = true;
    btnApagar.disabled = false;
  } else {
    btnEncender.disabled = false;
    btnApagar.disabled = true;
  }
}

function deshabilitarBotones() {
  btnEncender.disabled = true;
  btnApagar.disabled = true;
}

btnEncender.addEventListener('click', () => {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({ command: 'toggle' }));
  }
});

btnApagar.addEventListener('click', () => {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({ command: 'toggle' }));
  }
});

// Conectar al WebSocket al cargar la página
conectarWebSocket();
