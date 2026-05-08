#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "DHT22.h"
#include "wifi_manager.h"

// --- Configuracion ---
const char* SSID = "Telecentro-b40f";
const char* PASSWORD = "MQFYAEGQHMYQ";
const int LED_PIN = 4; // Flash del esp32 cam en GPIO4

AsyncWebServer server(80);

void handleMetrics(AsyncWebServerRequest *request) {
  float t = NAN;
  float h = NAN;

  sensorRead(t, h);

  String json = "{";
  json += "\"temperature\":" + String(t, 2) + ",";
  json += "\"humidity\":" + String(h, 2);
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

  sensorSetup();

  // Inicializar SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
  } else {
    Serial.println("SPIFFS montado correctamente");
  }

  // Setup WiFi manager (intenta conectar o levanta portal de configuracion)
  wifiManagerSetup(server);

  // Endpoint para métricas (JSON)
  server.on("/metrics", HTTP_GET, handleMetrics);
  
  // Endpoints para control de LED
  server.on("/led/on", HTTP_GET, handleLedOn);
  server.on("/led/off", HTTP_GET, handleLedOff);

  server.begin();
  Serial.println("Servidor HTTP asincrónico iniciado");
}

void loop() {
  // Needed for captive portal DNS redirect
  wifiManagerLoop();
  delay(1);  // Keep watchdog happy
}