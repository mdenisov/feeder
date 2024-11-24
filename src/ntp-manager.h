#ifndef _NTP_MANAGER
#define _NTP_MANAGER

#include <Arduino.h>
#include <GyverNTP.h>
#include "config.h"
#include "utils.h"

GyverNTP ntp(config.timeZone);

void initNTP()
{
  DEBUGLN(F("Init NTP"));

  ntp.begin();
}

void loopNTP()
{
  ntp.tick();
}

#endif