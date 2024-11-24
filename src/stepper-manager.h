#ifndef _STEPPER_MANAGER
#define _STEPPER_MANAGER

#include <Arduino.h>
#include <GyverStepper2.h>
#include "env.h"
#include "config.h"
#include "utils.h"

GStepper2<STEPPER2WIRE> stepper(STEPPER_STEPS * STEPPER_MICRO_STEPS, STEP_PIN, DIR_PIN, EN_PIN);

const int feedAmount = (STEPPER_STEPS * STEPPER_MICRO_STEPS * STEPPER_GEAR_RATIO) / 4;

void feed()
{
  if (!stepper.tick())
  {

    DEBUGLN(F("Feeding"));

    // publishMessage(MQTT_TOPIC_FEED_STATUS, "1", false);
    stepper.reset();
    stepper.setTarget(feedAmount * config.dosage, RELATIVE);
  }
}

void initStepper()
{

  DEBUGLN(F("Init Stepper"));

  stepper.setMaxSpeed(2300);
  stepper.setAcceleration(3500);
  // stepper.reset();
  stepper.autoPower(true);
  stepper.disable();
}

void loopStepper()
{
  if (stepper.tick())
  {
    appState.isFeeding = true;
  } else {
    appState.isFeeding = false;
  }

  static int shakeAttempts = 0;
  if (stepper.ready())
  {
    if (appState.isStucked)
    {
      if (shakeAttempts == 0) {
        stepper.setTarget(-1, RELATIVE);
      }

      if (shakeAttempts < 10)
      {
        bool dir = shakeAttempts % 2;
        stepper.setTarget(dir ? 500 : -500, RELATIVE);
        shakeAttempts++;
      }
      else
      {
        shakeAttempts = 0;
        appState.isStucked = false;
        feed();
      }
    }
    else
    {
      // publishMessage(MQTT_TOPIC_FEED_STATUS, "0", false);
    }
  }
}

#endif