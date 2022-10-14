#include <Arduino.h>
#include <myWebServer.h>

myWebserver::myWebserver(){}

String myWebserver::CreateTXString()
{
        String message = "";

        message += String(configData.ON) + "/";
         message += String(configData.OFF) + "/";
          message += String(configData.r) + "/";
           message += String(configData.g) + "/";
            message += String(configData.b) + "/";
             message += String(configData.Helligkeit) + "/";
        message += String(configData.OFF) ;
      

        return message;
}

String myWebserver::GetFile(FileSystem &fs, char *FileName)
{

        String TXT = fs.ReadFile(FileName);
        return TXT;
}

_configData myWebserver::SplitPayload(String Payload)
{
        String val1[20];
        int z = 0;
        int s = 0;
        int n = -1;
        while (true)
        {
                n = Payload.indexOf('/', s);
                if (n == -1)
                {
                        break;
                }
                if (s - 1 == Payload.lastIndexOf('/'))
                {
                        break;
                }

                String va = Payload.substring(s, n);
              
                val1[z] = va;
                z++;
                s = n + 1;
        }
        _configData data;
        s = 0;
        
                data.ON = val1[s].toFloat();
                s++;
                data.OFF = val1[s].toFloat();
                s++;
                data.r = val1[s].toInt();
                 s++;
                data.g = val1[s].toInt();
                 s++;
                data.b = val1[s].toInt();
                 s++;
                data.Helligkeit = val1[s].toInt();
                
        return data;
}
