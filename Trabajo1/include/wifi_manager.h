#pragma once
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void wifiManagerSetup(AsyncWebServer &server);
void wifiManagerLoop();
