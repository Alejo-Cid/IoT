function save() {
  const ssid = encodeURIComponent(document.getElementById('ssid').value);
  const pass = encodeURIComponent(document.getElementById('pass').value);
  if (!ssid) {
    document.getElementById('status').innerText = 'Ingresá SSID';
    return;
  }
  document.getElementById('status').innerText = 'Guardando...';
  // Use GET for simplicity since server handles it
  fetch(`/save?ssid=${ssid}&pass=${pass}`)
    .then(r => r.text())
    .then(t => {
      if (t === 'saved') {
        document.getElementById('status').innerText = 'Guardado. Reiniciando...';
      } else {
        document.getElementById('status').innerText = 'Error: ' + t;
      }
    })
    .catch(e => document.getElementById('status').innerText = 'Error: ' + e);
}
