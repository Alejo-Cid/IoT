#include <Arduino.h>
#include <DHT.h>
#include "DHT22.h"

namespace {
const int DHT_PIN = 15;
const int DHT_TYPE = DHT22;

DHT dht(DHT_PIN, DHT_TYPE);
float lastTemperature = NAN;
float lastHumidity = NAN;

float simulatedTemperature() {
  return 20.0 + (random(0, 1000) / 1000.0);
}

float simulatedHumidity() {
  return 40.0 + (random(0, 3000) / 1000.0);
}

bool readDht22(float &temperature, float &humidity) {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  return !isnan(temperature) && !isnan(humidity);
}
}

void sensorSetup() {
  pinMode(DHT_PIN, INPUT_PULLUP);
  dht.begin();
  Serial.printf("DHT22 inicializado en GPIO %d\n", DHT_PIN);

  randomSeed(micros());

  float bootT = NAN;
  float bootH = NAN;
  if (readDht22(bootT, bootH)) {
    lastTemperature = bootT;
    lastHumidity = bootH;
    Serial.printf("Primera lectura DHT22 OK: T=%.2fC H=%.2f%%\n", bootT, bootH);
  } else {
    lastTemperature = simulatedTemperature();
    lastHumidity = simulatedHumidity();
    Serial.println("DHT22 no responde, usando valores simulados");
  }
}

bool sensorRead(float &temperature, float &humidity) {
  if (readDht22(temperature, humidity)) {
    lastTemperature = temperature;
    lastHumidity = humidity;
    return true;
  }

  lastTemperature = simulatedTemperature();
  lastHumidity = simulatedHumidity();
  temperature = lastTemperature;
  humidity = lastHumidity;
  return false;
}

float sensorGetTemperature() {
  return lastTemperature;
}

float sensorGetHumidity() {
  return lastHumidity;
}