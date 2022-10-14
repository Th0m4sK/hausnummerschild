#pragma once
#include <Arduino.h>
#include <FileSystem.h>
#include <ESP8266WebServer.h>
#include <myWebServer.h>

struct _configData
{
    float ON = 1;
    float OFF = 1;
    int r=0;
    int g=0;
    int b=0;
    int Helligkeit=20;
   
};

class myWebserver
{
private:
public:
    _configData configData;

    myWebserver();

    String CreateTXString();
    String GetFile(FileSystem &fs, char *FileName);
    _configData SplitPayload(String Payload);
};
