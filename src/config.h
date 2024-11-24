#ifndef _CONFIG
#define _CONFIG

#include <Arduino.h>
#include "env.h"

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct AppStateStruct{
        bool isStucked = false;
        bool isFeeding = false;
    } AppStateStruct;

    AppStateStruct appState;

    typedef struct ConfigStruct{
        char apSsid[32] = AP_DEFAULT_SSID;
        char apPass[32] = AP_DEFAULT_PASS;
        char staSsid[21] = STA_DEFAULT_SSID;
        char staPass[21] = STA_DEFAULT_PASS;
        bool staModeEn = false;
        char mqttServer[32] = MQTT_SERVER;
        int mqttPort = MQTT_PORT;
        char mqttLogin[21] = MQTT_LOGIN;
        char mqttPass[21] = MQTT_PASS;
        bool mqttEn = false;
        int timeZone = NTP_TIMEZONE;
        int dosage = 2;
    } ConfigStruct;

    ConfigStruct config;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif