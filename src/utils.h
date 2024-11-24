#ifndef _UTILS
#define _UTILS

#include <Arduino.h>
#include <ESP8266WiFi.h>

/* ================ Макросы ================== */
#ifdef DEBUG_MODE
#define DEBUG(...) Serial.print(__VA_ARGS__)
#define DEBUGLN(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG(...)
#define DEBUGLN(...)
#endif

String getChipID()
{
  return String(system_get_chip_id());
}

#endif