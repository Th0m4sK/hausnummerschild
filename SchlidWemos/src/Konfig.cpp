#include <Konfig.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#define KONFIG_DATEI "/config.json"
#define KONFIG_TMP "/config.tmp"

Konfig konfig;

static uint8_t begrenze(long wert, long min, long max)
{
    if (wert < min)
    {
        return min;
    }
    if (wert > max)
    {
        return max;
    }
    return wert;
}

void KonfigStandard(Konfig &k)
{
    k.luxEin = 30;
    k.luxAus = 80;
    k.helligkeit = 20;

    // Reihenfolge ist die des Datenkabels; von vorn gesehen ergibt das
    // rot oben, weiss in der Mitte, blau unten.
    k.balken[0] = {0, 0, 255, 100};
    k.balken[1] = {255, 255, 255, 100};
    k.balken[2] = {255, 0, 0, 100};
}

String KonfigAlsJson(const Konfig &k)
{
    JsonDocument doc;
    doc["schema"] = KONFIG_SCHEMA;
    doc["luxEin"] = k.luxEin;
    doc["luxAus"] = k.luxAus;
    doc["helligkeit"] = k.helligkeit;

    JsonArray balken = doc["balken"].to<JsonArray>();
    for (uint8_t i = 0; i < BALKEN_ANZAHL; i++)
    {
        JsonObject o = balken.add<JsonObject>();
        o["r"] = k.balken[i].r;
        o["g"] = k.balken[i].g;
        o["b"] = k.balken[i].b;
        o["prozent"] = k.balken[i].prozent;
    }

    String text;
    serializeJson(doc, text);
    return text;
}

// Nimmt auch unvollstaendiges JSON an: was fehlt, behaelt den Wert aus k.
// So kann die Oberflaeche einzelne Werte schicken, ohne den Rest zu kennen.
bool KonfigAusJson(const String &text, Konfig &k)
{
    JsonDocument doc;
    if (deserializeJson(doc, text) != DeserializationError::Ok)
    {
        return false;
    }

    k.luxEin = doc["luxEin"] | k.luxEin;
    k.luxAus = doc["luxAus"] | k.luxAus;
    k.helligkeit = begrenze(doc["helligkeit"] | (int)k.helligkeit, 0, 100);

    JsonArrayConst balken = doc["balken"];
    if (!balken.isNull())
    {
        for (uint8_t i = 0; i < BALKEN_ANZAHL && i < balken.size(); i++)
        {
            JsonObjectConst o = balken[i];
            k.balken[i].r = begrenze(o["r"] | (int)k.balken[i].r, 0, 255);
            k.balken[i].g = begrenze(o["g"] | (int)k.balken[i].g, 0, 255);
            k.balken[i].b = begrenze(o["b"] | (int)k.balken[i].b, 0, 255);
            k.balken[i].prozent = begrenze(o["prozent"] | (int)k.balken[i].prozent, 0, 100);
        }
    }

    return true;
}

// Erst in eine zweite Datei schreiben, dann umbenennen: geht dabei der Strom
// aus, bleibt die alte Konfiguration heil. Direktes Ueberschreiben wuerde im
// schlechtesten Fall eine halbe Datei hinterlassen.
bool KonfigSpeichern(const Konfig &k)
{
    File f = LittleFS.open(KONFIG_TMP, "w");
    if (!f)
    {
        Serial.println(F("Konfig: konnte nicht schreiben"));
        return false;
    }

    String text = KonfigAlsJson(k);
    size_t geschrieben = f.print(text);
    f.close();

    if (geschrieben != text.length())
    {
        LittleFS.remove(KONFIG_TMP);
        Serial.println(F("Konfig: unvollstaendig geschrieben"));
        return false;
    }

    LittleFS.remove(KONFIG_DATEI);
    return LittleFS.rename(KONFIG_TMP, KONFIG_DATEI);
}

bool KonfigLaden(Konfig &k)
{
    KonfigStandard(k);

    if (!LittleFS.exists(KONFIG_DATEI))
    {
        Serial.println(F("Konfig: keine Datei, Standardwerte"));
        KonfigSpeichern(k);
        return false;
    }

    File f = LittleFS.open(KONFIG_DATEI, "r");
    if (!f)
    {
        Serial.println(F("Konfig: Datei nicht lesbar, Standardwerte"));
        return false;
    }

    String text = f.readString();
    f.close();

    if (!KonfigAusJson(text, k))
    {
        Serial.println(F("Konfig: JSON kaputt, Standardwerte"));
        KonfigStandard(k);
        return false;
    }

    return true;
}
