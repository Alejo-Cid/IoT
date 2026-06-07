#include <Arduino.h>
#include <WiFi.h> 
#include <PubSubClient.h> 

// --- CONFIGURACIÓN DE RED ---
const char* ssid = "iPhone de Alejo";
const char* password = "Cone1234";

// --- CONFIGURACIÓN MQTT ---
const char* mqtt_server = "172.20.10.4"; 

WiFiClient espClient;
PubSubClient client(espClient); 

// Pin analógico libre en la ESP32-CAM
const int sensorPin = 13; 

void connectWiFiAndMQTT() {
  Serial.print("Conectando al WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");
  
  Serial.print("Conectando al Broker MQTT...");
  while (!client.connected()) {
    /
    String clientId = "ESP32CAM-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("\n¡Conectado a Mosquitto!");
    } else {
      Serial.print(" Falló. Reintentando en 2 seg...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200); 
  client.setServer(mqtt_server, 1883); 
}

void loop() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100); 
  
  int nivel_agua = analogRead(sensorPin);
  Serial.print("\n--- NUEVA LECTURA ---\nNivel de agua medido: ");
  Serial.println(nivel_agua);

  WiFi.mode(WIFI_STA);
  connectWiFiAndMQTT();

  char msg[50]; // [cite: 45]
  sprintf(msg, "{\"nivel_agua\": %d}", nivel_agua); 
  
  client.publish("ambiente/agua", msg);
  Serial.print("Dato publicado en MQTT: ");
  Serial.println(msg);


  delay(500); 
  client.disconnect(); 
  espClient.stop(); 

  delay(5000); 