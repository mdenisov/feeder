#include "env.h"

/* ================== Libs =================== */
#include <Arduino.h>
#include <EncButton.h>
#include "Led.h"

#include "config.h"
#include "utils.h"
#include "wifi-manager.h"
#include "mqtt-manager.h"
#include "eeprom-manager.h"
#include "ntp-manager.h"
#include "stepper-manager.h"
#include "schedule-manager.h"
#include "web-server.h"

/* ================ Objects ================== */
Button btn(BTN_PIN, INPUT_PULLUP, HIGH);
Led led(LED_PIN);

/* ================== Main =================== */
void setup()
{
#ifdef DEBUG_MODE
  Serial.begin(115200);
#endif

  pinMode(BTN_PIN, INPUT_PULLUP);

  led.begin();
  led.on();
  initEEPROM();
  initWifi();
  initMQTT();
  initNTP();
  initStepper();
  initSchedule();
  initUI();
}

void loop()
{
  btn.tick();
  led.tick();
  loopNTP();
  loopMQTT();
  loopStepper();
  loopSchedule();
  loopUI();

  if (btn.hasClicks(2))
  {
    feed();
  }

  if (btn.hasClicks(3))
  {
    DEBUGLN(F("Button click 3 times"));

    appState.isStucked = true;
  }

  // Led
  if (appState.isFeeding)
  {
    led.blink(5);
  }
  else if (WiFi.status() != WL_CONNECTED)
  {
    led.smooth();
  }
  else
  {
    led.on();
  }
}