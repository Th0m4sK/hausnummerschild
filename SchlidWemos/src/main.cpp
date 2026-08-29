#include <Arduino.h>

#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#include <ESP8266WiFi.h>
#include <secret.h>
#include <BH1750.h>
#include <Wire.h>

#include <ElegantOTA.h>
#include <myWebServer.h>

#define FW_VERSION "0.0.1"

bool LichtEin = false;
float lux;
BH1750 lightMeter;
#define LED_PIN D4
#define LED_COUNT 31 * 9
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(80);
myWebserver myWeb;

String webDat = "";
FileSystem tape;

// Das Schild soll auch ohne WLAN leuchten: Verbindungsaufbau laeuft nebenher,
// Sensor und LEDs haengen nicht daran.
#define WIFI_RETRY_MS 30000
#define MESS_INTERVALL_MS 1000
bool serverLaeuft = false;
unsigned long letzterWifiVersuch = 0;
unsigned long letzteMessung = 0;

void readParameter()
{
  if (tape.mount())
  {
    // Serial.println("Filesystem gemountet");
  }
  else
  {
    // Serial.println("Filesystem nicht gemountet");
  }

  String TXT = tape.ReadFile((char *)"/ParaM");
  String mes = "";

  if (TXT == "notExists")
  {
    mes = myWeb.CreateTXString();
    // Serial.println(mes);
    mes += "/";
    tape.WriteNewFile((char *)"/ParaM", (char *)mes.c_str());
    TXT = tape.ReadFile((char *)"/ParaM");
  }

  myWeb.configData = myWeb.SplitPayload(TXT);

  mes = myWeb.CreateTXString();
}

void RoutenAnmelden()
{
  server.on("/", []()
            { 
              Serial.println(myWeb.GetFile(tape, (char *)"/index.html"));
              server.send(200, "text/html", myWeb.GetFile(tape, (char *)"/index.html")); });

  server.on("/style.css", HTTP_GET, []()
            { server.send(200, "text/css", myWeb.GetFile(tape, (char *)"/style.css")); });
  server.on("/script.js", HTTP_GET, []()
            { server.send(200, "text/javascript", myWeb.GetFile(tape, (char *)"/script.js")); });
  server.on("/ParM", HTTP_GET, []()
            {
               String message = myWeb.CreateTXString();
               //Serial.println(message);
              server.send_P(200, "text/plain", message.c_str()); });
  server.on("/light", HTTP_GET, []()
            {
               String message = String(lux);
               //Serial.println(message);
              server.send_P(200, "text/plain", message.c_str()); });

  server.on("/data", HTTP_POST, []()
            {
              if (server.hasArg("plain") == false)
              { // Check if body received

                server.send(200, "text/plain", "Body not received");
                return;
              }

              String message = "";
              message += server.arg("plain");
              
              server.send(200, "text/plain", message);
              myWeb.configData = myWeb.SplitPayload(message); 
              tape.WriteNewFile((char *)"/ParaM", (char *)message.c_str()); });
}

void WifiStarten()
{
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(STAHOST);
  WiFi.setAutoReconnect(true);
  WiFi.begin(STASSID, STAPSK);
  letzterWifiVersuch = millis();
}

// Wird bei jedem Schleifendurchlauf aufgerufen und blockiert nie: solange kein
// WLAN da ist, wird nur alle WIFI_RETRY_MS ein neuer Versuch angestossen.
void WifiPflegen()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!serverLaeuft)
    {
      ElegantOTA.begin(&server); // Start ElegantOTA
      server.begin();
      serverLaeuft = true;
      Serial.print("WLAN verbunden, IP: ");
      Serial.println(WiFi.localIP());
    }
    return;
  }

  if (millis() - letzterWifiVersuch >= WIFI_RETRY_MS)
  {
    Serial.println("Kein WLAN, neuer Verbindungsversuch");
    WiFi.begin(STASSID, STAPSK);
    letzterWifiVersuch = millis();
  }
}

void SchildAktualisieren()
{
  lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  Serial.println(LichtEin);
  Serial.println(myWeb.configData.Helligkeit);
  Serial.println(myWeb.configData.OFF);

  strip.setBrightness(myWeb.configData.Helligkeit);
  // webDat= "Light: "+String(lux)+ " lx "+" EIN: "+String(LichtEin);
  if (lux < myWeb.configData.ON)
  {
    LichtEin = true;
  }
  if (lux > myWeb.configData.OFF)
  {
    LichtEin = false;
  }

  for (int i = 0; i < 31 * 3; i++)
  {
    if (LichtEin)
    {
      // strip.setPixelColor(i, strip.Color(myWeb.configData.r, myWeb.configData.g, myWeb.configData.b));

      strip.setPixelColor(i, strip.Color(0, 0, 255));
      strip.setPixelColor(i + 31 * 3, strip.Color(255, 255, 255));
      strip.setPixelColor(i + 31 * 6, strip.Color(255, 0, 0));
    }
    else
    {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
      strip.setPixelColor(i + 31 * 3, strip.Color(0, 0, 0));
      strip.setPixelColor(i + 31 * 6, strip.Color(0, 0, 0));
    }
  }
  strip.show();
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  Serial.print(F("Hausnummerschild FW "));
  Serial.println(F(FW_VERSION));

  // Erst die Hardware, dann das Netz: faellt das WLAN aus, schaltet die
  // Daemmerungsautomatik trotzdem.
  Wire.begin();
  // On esp8266 you can select SCL and SDA pins using Wire.begin(D4, D3);
  // For Wemos / Lolin D1 Mini Pro and the Ambient Light shield use
  // Wire.begin(D2, D1);

  lightMeter.begin();
  strip.begin();
  strip.show(); // definierter Startzustand: alles aus

  readParameter();

  RoutenAnmelden();
  WifiStarten();

  Serial.println(F("BH1750 Te"));
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
