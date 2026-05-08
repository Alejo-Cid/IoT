#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <DHT.h>

// --- Configuracion ---
const char* SSID = "Telecentro-b40f";
const char* PASSWORD = "MQFYAEGQHMYQ";
const int LED_PIN = 4; // Flash del esp32 cam en GPIO4
const int DHT_PIN = 15; // GPIO 15 mas confiable en ESP32-CAM para DATA del DHT22
const int DHT_TYPE = DHT22;

DHT dht(DHT_PIN, DHT_TYPE);

AsyncWebServer server(80);

float lastTemperature = NAN;
float lastHumidity = NAN;

bool readDht22(float &temperature, float &humidity) {
  // Retry a few times because the DHT22 can sporadically fail a read.
  for (int attempt = 0; attempt < 3; attempt++) {
    humidity = dht.readHumidity(true);
    temperature = dht.readTemperature(false, true);
    Serial.printf(
      "[DHT22] intento %d/3: T=%.2f H=%.2f pin=%d\n",
      attempt + 1,
      temperature,
      humidity,
      digitalRead(DHT_PIN)
    );
    if (!isnan(temperature) && !isnan(humidity)) {
      return true;
    }
    delay(500);  // Aumentado de 250ms a 500ms para dar mas tiempo al sensor
  }
  Serial.println("[DHT22] lectura invalida despues de 3 intentos");
  return false;
}

void handleMetrics(AsyncWebServerRequest *request) {
  float t = NAN;
  float h = NAN;

  if (readDht22(t, h)) {
    lastTemperature = t;
    lastHumidity = h;
    Serial.printf("[DHT22] lectura OK: T=%.2fC H=%.2f%%\n", t, h);
  } else {
    Serial.println("Error al leer temperatura/humedad del DHT22");
  }

  if (isnan(lastTemperature) || isnan(lastHumidity)) {
    request->send(503, "application/json", "{\"error\":\"sensor_unavailable\"}");
    return;
  }

  String json = "{";
  json += "\"temperature\":" + String(lastTemperature, 2) + ",";
  json += "\"humidity\":" + String(lastHumidity, 2);
  json += "}";
  request->send(200, "application/json", json);
}

void handleLedOn(AsyncWebServerRequest *request) {
  digitalWrite(LED_PIN, HIGH);
  request->send(200, "text/plain", "LED Encendido");
}

void handleLedOff(AsyncWebServerRequest *request) {
  digitalWrite(LED_PIN, LOW);
  request->send(200, "text/plain", "LED Apagado");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Inicializar DHT22
  pinMode(DHT_PIN, INPUT_PULLUP);
  Serial.printf("[DIAG] GPIO %d ANTES de dht.begin(): pin=%d\n", DHT_PIN, digitalRead(DHT_PIN));
  delay(500);
  Serial.printf("[DIAG] GPIO %d (con pull-up, sin sensor): pin=%d (deberia ser 1)\n", DHT_PIN, digitalRead(DHT_PIN));
  if (digitalRead(DHT_PIN) == 0) {
    Serial.println("[ERROR] GPIO stuck a 0 - revisa cortocircuito o sensor dañado, desconecta el DHT22 y prueba de nuevo");
  }
  dht.begin();
  delay(5000);  // Aumentado a 5 segundos para que DHT22 se estabilice bien
  Serial.printf("[DIAG] GPIO %d DESPUES de dht.begin(): pin=%d (deberia ser 1)\n", DHT_PIN, digitalRead(DHT_PIN));
  Serial.printf("DHT22 inicializado en GPIO %d\n", DHT_PIN);

  float bootT = NAN;
  float bootH = NAN;
  if (readDht22(bootT, bootH)) {
    lastTemperature = bootT;
    lastHumidity = bootH;
    Serial.printf("Primera lectura DHT22 OK: T=%.2fC H=%.2f%%\n", bootT, bootH);

  } else {
    Serial.println("Primera lectura DHT22 fallida: revisar cableado/pull-up");
  }

  randomSeed(micros());

  // Inicializar SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
  } else {
    Serial.println("SPIFFS montado correctamente");
  }

  WiFi.begin(SSID, PASSWORD);
  Serial.print("Conectando a WiFi");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 40) {
    delay(500);
    Serial.print('.');
    retries++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conectado. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("No se pudo conectar a la red WiFi (timeout)");
  }

  // Servir archivos estáticos desde SPIFFS
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  
  // Endpoint para métricas (JSON)
  server.on("/metrics", HTTP_GET, handleMetrics);
  
  // Endpoints para control de LED
  server.on("/led/on", HTTP_GET, handleLedOn);
  server.on("/led/off", HTTP_GET, handleLedOff);

  server.begin();
  Serial.println("Servidor HTTP asincrónico iniciado");
}

void loop() {
}