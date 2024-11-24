#ifndef _EEPROM_MANAGER
#define _EEPROM_MANAGER

#include <Arduino.h>
#include <EEPROM.h>

#include "env.h"
#include "config.h"
#include "utils.h"

void updateEEPROM()
{
  EEPROM.write(0, EEPROM_KEY);
  EEPROM.put(1, config);
  EEPROM.commit();
  delay(50);
}

void resetEEPROM()
{
  DEBUGLN(F("Reset EEPROM"));

  EEPROM.write(0, EEPROM_RESET_KEY);
  EEPROM.commit();
  delay(50);
}

void initEEPROM()
{
  DEBUGLN(F("Init EEPROM"));

  EEPROM.begin(512);
  delay(50);

  // Read stored key and settings
  if (EEPROM.read(0) == EEPROM_KEY)
  {
    // Read stored settings
    EEPROM.get(1, config);
    delay(50);
  }
  else
  {
    // Save default settings
    resetEEPROM();
  }
}

#endif