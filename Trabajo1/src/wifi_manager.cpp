#include "wifi_manager.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <SPIFFS.h>

static DNSServer dnsServer;
static Preferences prefs;
static bool portalActive = false;
static const byte DNS_PORT = 53;

static const bool CLEAR_WIFI_CREDENTIALS_ON_BOOT = false;

void clearSavedCredentials() {
  prefs.begin("wifi", false);
  prefs.remove("ssid");
  prefs.remove("pass");
  prefs.end();
  Serial.println("Credenciales WiFi borradas");
}

bool connectSavedCredentials(int timeoutSeconds = 10) {
  
  prefs.begin("wifi", false);
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("pass", "");
  prefs.end();
  if (ssid.length() == 0) return false;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  int waited = 0;
  while (WiFi.status() != WL_CONNECTED && waited < timeoutSeconds * 2) {
    delay(500);
    waited++;
  }
  return WiFi.status() == WL_CONNECTED;
}

void startConfigAP(AsyncWebServer &server) {
  portalActive = true;
  WiFi.mode(WIFI_AP);
  String apName = "ESP32-Setup";
  WiFi.softAP(apName.c_str());
  IPAddress apIP = WiFi.softAPIP();

  Serial.print("AP levantado. SSID: ");
  Serial.print(apName);
  Serial.print(" IP: ");
  Serial.println(apIP);

  // start DNS server to redirect ALL domains to AP IP
  dnsServer.start(DNS_PORT, "*", apIP);

  // Serve static files from SPIFFS with config.html as default
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("config.html");

  // handler to save credentials via GET params
  server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("ssid") && request->hasParam("pass")) {
      String ssid = request->getParam("ssid")->value();
      String pass = request->getParam("pass")->value();
      prefs.begin("wifi", false);
      prefs.putString("ssid", ssid);
      prefs.putString("pass", pass);
      prefs.end();
      request->send(200, "text/plain", "saved");
      delay(200);
      ESP.restart();
      return;
    }
    request->send(400, "text/plain", "missing_fields");
  });
}

void wifiManagerSetup(AsyncWebServer &server) {

  if (CLEAR_WIFI_CREDENTIALS_ON_BOOT) {
    clearSavedCredentials();
  }
  // try to connect with saved creds
  if (connectSavedCredentials(8)) {
    Serial.print("Conectado con credenciales guardadas. IP: ");
    Serial.println(WiFi.localIP());
    portalActive = false;
    // serve main UI
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  } else {
    Serial.println("No hay credenciales validas. Iniciando portal de configuracion (AP)");
    startConfigAP(server);
  }
}

void wifiManagerLoop() {
  if (portalActive) dnsServer.processNextRequest();
}
