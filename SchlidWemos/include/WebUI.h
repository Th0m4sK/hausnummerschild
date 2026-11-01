#pragma once
#include <ESP8266WebServer.h>

// Haengt alle Routen an den Server. Braucht noch kein Netz - der Server wird
// erst gestartet, wenn die WLAN-Verbindung steht.
// luxQuelle und einQuelle werden fuer /api/status gelesen, nicht kopiert.
void WebRoutenAnmelden(ESP8266WebServer &server, const float &luxQuelle, const bool &einQuelle);
