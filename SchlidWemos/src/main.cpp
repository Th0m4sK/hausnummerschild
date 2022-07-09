#include <Arduino.h>

         


#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#include <ESP8266WiFi.h>
#include <secret.h>
#include <BH1750.h>
#include <Wire.h>
#include <ArduinoOTA.h>

bool LichtEin=false;
BH1750 lightMeter;
#define LED_PIN D4
#define LED_COUNT 28 * 5
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


void Wifi_OAT()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  WiFi.setHostname(STAHOST);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.onStart([]()
                     {
                       String type;
                       if (ArduinoOTA.getCommand() == U_FLASH)
                       {
                         type = "sketch";
                       }
                       else
                       { // U_FS
                         type = "filesystem";
                       }

                       // NOTE: if updating FS this would be the place to unmount FS using FS.end()
                       Serial.println("Start updating " + type);
                     });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
                       Serial.printf("Error[%u]: ", error);
                       if (error == OTA_AUTH_ERROR)
                       {
                         Serial.println("Auth Failed");
                       }
                       else if (error == OTA_BEGIN_ERROR)
                       {
                         Serial.println("Begin Failed");
                       }
                       else if (error == OTA_CONNECT_ERROR)
                       {
                         Serial.println("Connect Failed");
                       }
                       else if (error == OTA_RECEIVE_ERROR)
                       {
                         Serial.println("Receive Failed");
                       }
                       else if (error == OTA_END_ERROR)
                       {
                         Serial.println("End Failed");
                       }
                     });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}



void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(100);
    Wifi_OAT();
 Wire.begin();
  // On esp8266 you can select SCL and SDA pins using Wire.begin(D4, D3);
  // For Wemos / Lolin D1 Mini Pro and the Ambient Light shield use
  // Wire.begin(D2, D1);

  lightMeter.begin();

  Serial.println(F("BH1750 Test begin"));



    

    
   
   
}

void loop() {

  ArduinoOTA.handle();
    // put your main code here, to run repeatedly:


   float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  Serial.println(LichtEin);
  if (lux<15){
LichtEin= true;
  }
   if (lux>20){
LichtEin= false;
  }
for (int i=0;i<LED_COUNT;i++){
  if (LichtEin){
    
      strip.setPixelColor(i,strip.Color(150,0,255));
    
  }
  else{
    strip.setPixelColor(i,strip.Color(0,0,0));
  }
  }
  delay(5000);
    
}