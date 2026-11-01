#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Adafruit_NeoPixel.h>
#include <BH1750.h>
#include <Wire.h>
#include <LittleFS.h>

#include <Konfig.h>
#include <WebUI.h>
#include <Version.h>

#define LED_PIN D4

// Zugangsdaten kommen vom WiFiManager aus dem Flash, nicht mehr aus secret.h.
#define HOSTNAME "hausnummer"
#define AP_NAME "Hausnummer-Setup" // offen, erreichbar nur wenn kein WLAN da ist
#define PORTAL_TIMEOUT_S 300       // danach wieder normale STA-Versuche
#define CONNECT_TIMEOUT_S 15       // so lange darf der Start auf das WLAN warten

// Das Schild soll auch ohne WLAN leuchten: Verbindungsaufbau laeuft nebenher,
// Sensor und LEDs haengen nicht daran.
#define WIFI_RETRY_MS 30000
#define MESS_INTERVALL_MS 1000

bool LichtEin = false;
float lux = 0;

BH1750 lightMeter;
Adafruit_NeoPixel strip(LED_GESAMT, LED_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(80);
WiFiManager wm;

bool serverLaeuft = false;
unsigned long letzterWifiVersuch = 0;
unsigned long letzteMessung = 0;

void WifiStarten()
{
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  wm.setHostname(HOSTNAME);
  wm.setConfigPortalBlocking(false); // Portal darf loop() nicht anhalten
  wm.setConnectTimeout(CONNECT_TIMEOUT_S);
  wm.setConfigPortalTimeout(PORTAL_TIMEOUT_S);

  // Gespeicherte Daten probieren; erst wenn die nicht tragen, geht das Portal auf.
  wm.autoConnect(AP_NAME);
  letzterWifiVersuch = millis();
}

// Wird bei jedem Schleifendurchlauf aufgerufen und blockiert nie: solange kein
// WLAN da ist, laeuft entweder das Konfigportal oder alle WIFI_RETRY_MS ein
// neuer Verbindungsversuch.
void WifiPflegen()
{
  wm.process(); // bedient das Konfigportal und kehrt sofort zurueck

  if (wm.getConfigPortalActive())
  {
    // Das Portal belegt Port 80, unser Webserver muss so lange schweigen.
    if (serverLaeuft)
    {
      server.stop();
      serverLaeuft = false;
    }
    letzterWifiVersuch = millis();
    return;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    if (!serverLaeuft)
    {
      server.begin(); // Routen und ElegantOTA haengen schon dran
      serverLaeuft = true;
      Serial.print(F("WLAN verbunden, IP: "));
      Serial.println(WiFi.localIP());
    }
    return;
  }

  if (serverLaeuft)
  {
    server.stop();
    serverLaeuft = false;
  }

  if (millis() - letzterWifiVersuch >= WIFI_RETRY_MS)
  {
    Serial.println(F("Kein WLAN, neuer Verbindungsversuch"));
    WiFi.reconnect(); // gespeicherte Daten, vom WiFiManager abgelegt
    letzterWifiVersuch = millis();
  }
}

// Bewusst ohne Deckel: 279 LEDs auf Weiss ziehen bei 100 Prozent rund 16 A.
// Was das Netzteil nicht liefert, endet in Spannungseinbruechen und Neustarts.
void SchildAktualisieren()
{
  lux = lightMeter.readLightLevel();

  if (lux < konfig.luxEin)
  {
    LichtEin = true;
  }
  if (lux > konfig.luxAus)
  {
    LichtEin = false;
  }

  strip.setBrightness((uint16_t)konfig.helligkeit * 255 / 100);

  if (LichtEin)
  {
    for (uint8_t nr = 0; nr < BALKEN_ANZAHL; nr++)
    {
      const Balken &bal = konfig.balken[nr];
      uint8_t rot = (uint16_t)bal.r * bal.prozent / 100;
      uint8_t gruen = (uint16_t)bal.g * bal.prozent / 100;
      uint8_t blau = (uint16_t)bal.b * bal.prozent / 100;

      uint16_t start = nr * LED_PRO_BALKEN;
      for (uint16_t i = 0; i < LED_PRO_BALKEN; i++)
      {
        strip.setPixelColor(start + i, strip.Color(rot, gruen, blau));
      }
    }
  }
  else
  {
    strip.clear();
  }

  strip.show();
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.print(F("Hausnummerschild FW "));
  Serial.println(F(FW_VERSION));

  // Erst die Hardware, dann das Netz: faellt das WLAN aus, schaltet die
  // Daemmerungsautomatik trotzdem.
  Wire.begin();
  lightMeter.begin();
  strip.begin();
  strip.show(); // definierter Startzustand: alles aus

  if (!LittleFS.begin())
  {
    Serial.println(F("LittleFS nicht gemountet"));
  }
  KonfigLaden(konfig);

  SchildAktualisieren(); // leuchtet schon, waehrend das WLAN noch sucht
  letzteMessung = millis();

  WebRoutenAnmelden(server, lux, LichtEin);
  WifiStarten();
}

void loop()
{
  WifiPflegen();

  if (serverLaeuft)
  {
    server.handleClient();
  }

  if (millis() - letzteMessung >= MESS_INTERVALL_MS)
  {
    letzteMessung = millis();
    SchildAktualisieren();
  }
}
