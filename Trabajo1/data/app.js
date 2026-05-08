// Actualizar métricas cada 2 segundos
setInterval(updateMetrics, 2000);

// Actualizar al cargar la página
window.addEventListener('load', updateMetrics);

function updateMetrics() {
    fetch('/metrics')
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            if (typeof data.temperature !== 'number' || typeof data.humidity !== 'number') {
                throw new Error('Payload de metricas invalido');
            }
            document.getElementById('temperature').textContent = data.temperature.toFixed(2);
            document.getElementById('humidity').textContent = data.humidity.toFixed(2);
        })
        .catch(error => {
            console.error('Error fetching metrics:', error);
            document.getElementById('temperature').textContent = 'Error';
            document.getElementById('humidity').textContent = 'Error';
        });
}

function toggleLED(action) {
    const url = action === 'on' ? '/led/on' : '/led/off';
    fetch(url)
        .then(response => {
            if (response.ok) {
                const status = action === 'on' ? 'Encendido' : 'Apagado';
                document.getElementById('status').textContent = `Estado: LED ${status}`;
                console.log(`LED ${action}:`);
            }
        })
        .catch(error => console.error('Error:', error));
}
