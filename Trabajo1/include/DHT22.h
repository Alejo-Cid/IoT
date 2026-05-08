#pragma once

void sensorSetup();
bool sensorRead(float &temperature, float &humidity);
float sensorGetTemperature();
float sensorGetHumidity();