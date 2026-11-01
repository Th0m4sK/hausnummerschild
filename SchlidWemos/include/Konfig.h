#pragma once
#include <Arduino.h>

// Aufbau des Schilds: 9 Reihen a 31 LEDs, zu drei gleich hohen Balken gruppiert.
// Balken 0 liegt am Dateneingang des Streifens.
#define LED_PRO_REIHE 31
#define REIHEN_PRO_BALKEN 3
#define BALKEN_ANZAHL 3
#define LED_PRO_BALKEN (LED_PRO_REIHE * REIHEN_PRO_BALKEN)
#define LED_GESAMT (LED_PRO_BALKEN * BALKEN_ANZAHL)

// Wird mitgespeichert, damit ein spaeterer Umbau der Struktur alte Dateien
// erkennen und umrechnen kann, statt sie stillschweigend falsch zu lesen.
#define KONFIG_SCHEMA 1

struct Balken
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t prozent; // 0..100, daempft nur diesen Balken
};

struct Konfig
{
    float luxEin;       // darunter schaltet das Schild ein
    float luxAus;       // darueber wieder aus (Hysterese, muss ueber luxEin liegen)
    uint8_t helligkeit; // 0..100, Gesamthelligkeit ueber alle Balken
    Balken balken[BALKEN_ANZAHL];
};

extern Konfig konfig;

void KonfigStandard(Konfig &k);
bool KonfigLaden(Konfig &k);          // false, wenn die Standardwerte einspringen mussten
bool KonfigSpeichern(const Konfig &k);
String KonfigAlsJson(const Konfig &k);
bool KonfigAusJson(const String &text, Konfig &k);
