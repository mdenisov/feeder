#ifndef _SERVER
#define _SERVER

#include <Arduino.h>
#include <GyverPortal.h>
#include <LittleFS.h>
#include "config.h"
#include "utils.h"
#include "eeprom-manager.h"

GyverPortal ui(&LittleFS);

void build()
{
  GP.BUILD_BEGIN(400);
  GP.THEME(GP_DARK);
  GP.PAGE_TITLE("Feeder");
  GP.FORM_BEGIN("/config");
  GP.GRID_RESPONSIVE(600);
  M_BLOCK(
      M_BLOCK_TAB(
          "AP-Mode",
          GP.TEXT("apSsid", "Логин", config.apSsid, "", 20);
          GP.BREAK();
          GP.PASS_EYE("apPass", "Пароль", config.apPass, "", 20);
          GP.BREAK(););
      M_BLOCK_TAB(
          "WiFi",
          GP.TEXT("staSsid", "Логин", config.staSsid, "", 20);
          GP.BREAK();
          GP.PASS_EYE("staPass", "Пароль", config.staPass, "", 20);
          GP.BREAK();
          M_BOX(GP_CENTER, GP.LABEL("WiFi Enable");
                GP.SWITCH("staEn", config.staModeEn);););
      M_BLOCK_TAB(
          "MQTT",
          GP.TEXT("mqttServer", "Сервер", config.mqttServer, "", 20);
          GP.BREAK();
          GP.NUMBER("mqttPort", "Порт", config.mqttPort, "", 20);
          GP.BREAK();
          GP.TEXT("mqttLogin", "Логин", config.mqttLogin, "", 20);
          GP.BREAK();
          GP.PASS_EYE("mqttPass", "Пароль", config.mqttPass, "", 20);
          GP.BREAK();
          GP.TEXT("mqttUUID", "ID", getChipID(), "", 20, "", true);
          GP.BREAK();
          M_BOX(GP_CENTER, GP.LABEL("MQTT Enable");
                GP.SWITCH("mqttEn", config.mqttEn);););
      M_BLOCK_TAB(
        "Feeder",
        M_BOX(
          GP.LABEL("Time zone");
          GP.SPINNER("timeZone", config.timeZone, -10, 10);
          );
        M_BOX(
          GP.LABEL("Dosage");
          GP.SPINNER("dosage", config.dosage, 1, 5);
          );
        );
      GP.SUBMIT("Сохранить");
      GP.FORM_END();

      if (OTA_ENABLED) {
        M_BLOCK_TAB(
            "ESP UPDATE",
            GP.OTA_FIRMWARE(););
      });
  GP.BUILD_END();
}

void action(GyverPortal &p)
{
  if (p.form("/config"))
  {
    p.copyStr("apSsid", config.apSsid);
    p.copyStr("apPass", config.apPass);
    p.copyStr("staSsid", config.staSsid);
    p.copyStr("staPass", config.staPass);
    p.copyBool("staEn", config.staModeEn);

    p.copyStr("mqttServer", config.mqttServer);
    p.copyInt("mqttPort", config.mqttPort);
    p.copyStr("mqttLogin", config.mqttLogin);
    p.copyStr("mqttPass", config.mqttPass);
    p.copyBool("mqttEn", config.mqttEn);

    p.copyInt("timeZone", config.timeZone);
    p.copyInt("dosage", config.dosage);

    // Save settings
    updateEEPROM();

    // Restsrt ESP
    ESP.restart();
    // Отключаем AP
    // WiFi.softAPdisconnect();
  }
}

void initUI() {
  ui.attachBuild(build);
  ui.attach(action);
  ui.start(HOSTNAME);

  // Enable OTA
  if (OTA_ENABLED) {
    ui.enableOTA();
  }

  LittleFS.begin();

  DEBUGLN(F("Started UI"));
}

void loopUI() { ui.tick(); }

#endif