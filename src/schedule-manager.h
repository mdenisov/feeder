#ifndef _SCHEDULE_MANAGER
#define _SCHEDULE_MANAGER

#include <Arduino.h>
#include <TimerMs.h>
#include "config.h"
#include "utils.h"

TimerMs scheduleTimer(500, 1, 0);

const byte schedule[][2] = {
    {9, 0},
    {14, 0},
    {20, 0},
};


void scheduleCallback()
{
  static byte prevMin = 0;
  uint8_t minute = ntp.minute();
  uint8_t hour = ntp.hour();

  if (prevMin != minute)
  {
    prevMin = minute;

    for (byte i = 0; i < sizeof(schedule) / 2; i++)
    {
      if (schedule[i][0] == hour && schedule[i][1] == minute)
      {
        DEBUGLN(F("Schedule time"));
        feed();
      }
    }
  }
}

void initSchedule()
{
  DEBUGLN(F("Init Schedule"));

  scheduleTimer.attach(scheduleCallback);
}

void loopSchedule()
{
  scheduleTimer.tick();
}

#endif