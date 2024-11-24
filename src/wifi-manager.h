#ifndef _WIFI_MANAGER
#define _WIFI_MANAGER

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "env.h"
#include "config.h"

int connectionAttempts = 0;
const int maxConnectionAttempts = 10;

bool startAPMode()
{
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(config.apSsid, config.apPass);

    DEBUGLN(F("Started in AP mode"));
    DEBUGLN(WiFi.softAPIP());

    return true;
}

bool startSTAMode()
{
    while (connectionAttempts < maxConnectionAttempts)
    {
        if (WiFi.status() == WL_CONNECTED)
            break;

        WiFi.hostname(HOSTNAME);
        WiFi.mode(WIFI_STA);
        WiFi.begin(config.staSsid, config.staPass);
        delay(1000);

        DEBUGLN(F("Connecting to WIFI.. "));
        DEBUG(config.staSsid);
        DEBUG(F(" / "));
        DEBUGLN(config.staPass);
        delay(8000);
        connectionAttempts++;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUGLN(F("Failed to connect to wifi."));

        startAPMode();
        
        return false;
    }

    DEBUGLN(F("Connected To Wifi"));
    DEBUG(F("IP address: "));
    DEBUGLN(WiFi.localIP());
    DEBUG(F("RSSI:"));
    DEBUGLN(WiFi.RSSI());

    return true;
}

bool initWifi()
{
    DEBUGLN(F("-------------------------------------"));

    if (strlen(config.staSsid) == 0 || strlen(config.staPass) == 0)
    {
        return startAPMode();
    }

    return startSTAMode();
}

#endif