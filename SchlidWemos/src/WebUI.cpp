#include <WebUI.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include <Konfig.h>
#include <Version.h>

static const float *lux = nullptr;
static const bool *ein = nullptr;

void WebRoutenAnmelden(ESP8266WebServer &server, const float &luxQuelle, const bool &einQuelle)
{
    lux = &luxQuelle;
    ein = &einQuelle;

    // Direkt aus dem Flash gestreamt, statt die Datei in den RAM zu laden.
    server.serveStatic("/", LittleFS, "/index.html");
    server.serveStatic("/style.css", LittleFS, "/style.css", "max-age=86400");
    server.serveStatic("/script.js", LittleFS, "/script.js", "max-age=86400");
    server.serveStatic("/favicon.ico", LittleFS, "/favicon.ico", "max-age=604800");

    server.on("/api/config", HTTP_GET, [&server]()
              { server.send(200, "application/json", KonfigAlsJson(konfig)); });

    server.on("/api/config", HTTP_POST, [&server]()
              {
                  if (!server.hasArg("plain"))
                  {
                      server.send(400, "application/json", F("{\"fehler\":\"kein Body\"}"));
                      return;
                  }

                  Konfig neu = konfig;
                  if (!KonfigAusJson(server.arg("plain"), neu))
                  {
                      server.send(400, "application/json", F("{\"fehler\":\"JSON unlesbar\"}"));
                      return;
                  }

                  konfig = neu; // wirkt sofort, der naechste Durchlauf malt damit
                  bool ok = KonfigSpeichern(konfig);
                  server.send(ok ? 200 : 500, "application/json",
                              ok ? F("{\"ok\":true}") : F("{\"fehler\":\"nicht gespeichert\"}")); });

    server.on("/api/status", HTTP_GET, [&server]()
              {
                  JsonDocument doc;
                  doc["lux"] = lux ? *lux : 0;
                  doc["ein"] = ein ? *ein : false;
                  doc["version"] = FW_VERSION;
                  doc["ip"] = WiFi.localIP().toString();
                  doc["laufzeit"] = millis() / 1000;

                  String text;
                  serializeJson(doc, text);
                  server.send(200, "application/json", text); });

    server.onNotFound([&server]()
                      { server.send(404, "application/json", F("{\"fehler\":\"unbekannt\"}")); });

    ElegantOTA.begin(&server); // haengt nur Routen an, braucht noch kein Netz
}
