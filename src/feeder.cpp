#include "env.cpp"
#include "utils.cpp"

/* =================== Pins ================== */
#define BTN_PIN 5
#define LED_PIN 13
#define EN_PIN 12
#define STEP_PIN 14
#define DIR_PIN 16

/* ================== Libs =================== */
#include <Arduino.h>
#include <EEPROM.h>
#include <EncButton.h>
#include <GyverNTP.h>
#include <GyverPortal.h>
#include <GyverStepper2.h>
#include <TimerMs.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include "Led.h"
#include "Ticker.h"

/* ================ Objects ================== */
GyverPortal ui(&LittleFS);
Button btn(BTN_PIN, INPUT_PULLUP, HIGH);
GyverNTP ntp(NTP_TIMEZONE);
GStepper2<STEPPER2WIRE> stepper(STEPPER_STEPS *STEPPER_MICRO_STEPS, STEP_PIN, DIR_PIN, EN_PIN);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Led led(LED_PIN);
TimerMs scheduleTimer(500, 1, 0);
TimerMs heartbeatTimer(30000, 1, 0);
TimerMs connectingTimer(60000, 0, 1);

/* ============ Global variables ============= */
struct
{
  char apSsid[21] = AP_DEFAULT_SSID;   // AP ssid name
  char apPass[21] = AP_DEFAULT_PASS;   // AP password
  char staSsid[21] = STA_DEFAULT_SSID; // WiFi ssid
  char staPass[21] = STA_DEFAULT_PASS; // WiFi password
  bool staModeEn = false;              // WiFi enabled
  char mqttServer[21] = MQTT_SERVER;   // MQTT server
  int mqttPort = MQTT_PORT;            // MQTT port
  char mqttLogin[21] = MQTT_LOGIN;     // MQTT login
  char mqttPass[21] = MQTT_PASS;       // MQTT password
  bool mqttEn = true;                  // MQTT enabled
  int dosage = 1;                      // dosage
} cfg;

// Schedule
const byte schedule[][2] = {
    {9, 0},
    {14, 0},
    {20, 0},
};
const int feedAmount = (STEPPER_STEPS * STEPPER_MICRO_STEPS * STEPPER_GEAR_RATIO) / 4;

/* ============== WiFi callbacks ============= */
static WiFiEventHandler staConnectedHandler;
static WiFiEventHandler staDisconnectedHandler;
static WiFiEventHandler staGotIPHandler;
static WiFiEventHandler staDHCPTimeoutHandler;

/* ================== Logic ================== */
String getChipID()
{
  return String(system_get_chip_id());
}

String getTopicName(const char *topic)
{
  const String clientId = getChipID();

  return String(HOSTNAME) + "/" + clientId + "/" + topic;
}

bool publishMessage(const char *topic, String payload, boolean retained)
{
  if (cfg.staModeEn && cfg.mqttEn && mqttClient.connected())
  {
    const String uniqTopic = getTopicName(topic);

    DEBUG("Publish message: [");
    DEBUG(uniqTopic);
    DEBUG("] ");
    DEBUGLN(payload);

    return mqttClient.publish(uniqTopic.c_str(), payload.c_str(), retained);
  }

  return true;
}

bool subscribeToTopic(const char *topic)
{
  if (cfg.staModeEn && cfg.mqttEn && mqttClient.connected())
  {
    const String uniqTopic = getTopicName(topic);

    DEBUG("Subscribe to topic: [");
    DEBUG(uniqTopic);
    DEBUGLN("] ");

    return mqttClient.subscribe(uniqTopic.c_str());
  }

  return true;
}

void updateEEPROM()
{
  EEPROM.put(1, cfg);
  EEPROM.commit();
  delay(50);
}

void resetEEPROM()
{

  DEBUGLN("Reset EEPROM");

  EEPROM.write(0, EEPROM_RESET_KEY);
  EEPROM.commit();
  delay(50);
}

void feed()
{
  if (!stepper.tick())
  {

    DEBUGLN("feed");

    publishMessage(MQTT_TOPIC_FEED_STATUS, "1", false);
    stepper.reset();
    stepper.setTarget(feedAmount * cfg.dosage, RELATIVE);
  }
}

void mqttMessageHandler(char *topic, byte *payload, unsigned int length)
{
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    // Convert *byte to string
    message += (char)payload[i];
  }

  DEBUG("Message arrived [");
  DEBUG(topic);
  DEBUG("] ");
  DEBUGLN(message);

  if (String(topic) == getTopicName(MQTT_TOPIC_FEED))
  {
    if (message == "1")
    {
      feed();
    }
  }
  else if (String(topic) == getTopicName(MQTT_TOPIC_DOSAGE))
  {
    cfg.dosage = constrain(1, 5, message.toInt());
    updateEEPROM();
  }
}

/* ================ Web page ================= */
void build()
{
  GP.BUILD_BEGIN(400);
  GP.THEME(GP_DARK);
  GP.PAGE_TITLE("Feeder");
  GP.FORM_BEGIN("/cfg");
  GP.GRID_RESPONSIVE(600);
  M_BLOCK(
      M_BLOCK_TAB(
          "AP-Mode",
          GP.TEXT("apSsid", "Логин", cfg.apSsid, "", 20);
          GP.BREAK();
          GP.PASS_EYE("apPass", "Пароль", cfg.apPass, "", 20);
          GP.BREAK(););
      M_BLOCK_TAB(
          "WiFi",
          GP.TEXT("staSsid", "Логин", cfg.staSsid, "", 20);
          GP.BREAK();
          GP.PASS_EYE("staPass", "Пароль", cfg.staPass, "", 20);
          GP.BREAK();
          M_BOX(GP_CENTER, GP.LABEL("WiFi Enable");
                GP.SWITCH("staEn", cfg.staModeEn);););
      M_BLOCK_TAB(
          "MQTT",
          GP.TEXT("mqttServer", "Сервер", cfg.mqttServer, "", 20);
          GP.BREAK();
          GP.NUMBER("mqttPort", "Порт", cfg.mqttPort, "", 20);
          GP.BREAK();
          GP.TEXT("mqttLogin", "Логин", cfg.mqttLogin, "", 20);
          GP.BREAK();
          GP.PASS_EYE("mqttPass", "Пароль", cfg.mqttPass, "", 20);
          GP.BREAK();
          GP.TEXT("mqttUUID", "ID", getChipID(), "", 20, "", true);
          GP.BREAK();
          M_BOX(GP_CENTER, GP.LABEL("MQTT Enable");
                GP.SWITCH("mqttEn", cfg.mqttEn);););
      M_BLOCK_TAB("Дозировка", M_BOX(GP.LABEL("Значение");
                                     GP.SPINNER("dosage", cfg.dosage, 1, 5);););
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
  if (p.form("/cfg"))
  {
    p.copyStr("apSsid", cfg.apSsid);
    p.copyStr("apPass", cfg.apPass);
    p.copyStr("staSsid", cfg.staSsid);
    p.copyStr("staPass", cfg.staPass);
    p.copyBool("staEn", cfg.staModeEn);

    p.copyStr("mqttServer", cfg.mqttServer);
    p.copyInt("mqttPort", cfg.mqttPort);
    p.copyStr("mqttLogin", cfg.mqttLogin);
    p.copyStr("mqttPass", cfg.mqttPass);
    p.copyBool("mqttEn", cfg.mqttEn);

    p.copyInt("dosage", cfg.dosage);

    // Save settings
    updateEEPROM();

    // Restsrt ESP
    ESP.restart();
    // Отключаем AP
    // WiFi.softAPdisconnect();
  }
}

/* =================== Init ================== */
void initPins()
{
  DEBUGLN("Init pins");

  pinMode(BTN_PIN, INPUT_PULLUP);
}

void initEEPROM()
{
  DEBUGLN("Init EEPROM");

  EEPROM.begin(250);
  delay(50);

  // Read stored key and settings
  if (EEPROM.read(0) == EEPROM_KEY)
  {
    // Read stored settings
    EEPROM.get(1, cfg);
    delay(50);
  }
  else
  {
    // Save default settings
    resetEEPROM();
  }
}

void initNTP()
{
  DEBUGLN("Init NTP");

  ntp.begin();
}

void initUI()
{
  ui.attachBuild(build);
  ui.attach(action);
  ui.start(HOSTNAME);
  // Enable OTA
  if (OTA_ENABLED)
  {
    ui.enableOTA();
  }
}

void initStepper()
{

  DEBUGLN("Init stepper");

  stepper.setMaxSpeed(2300);
  stepper.setAcceleration(3500);
  stepper.autoPower(true);
  stepper.reset();
  stepper.disable();
}

void initFS()
{
  LittleFS.begin();
}

/* =================== WiFi ================== */
void onStaConnected(const WiFiEventStationModeConnected &evt)
{
  // Stop connecting timer
  connectingTimer.stop();

  DEBUG("WiFi connected: ");
  DEBUG(evt.ssid);
  DEBUG(" ");
  DEBUGLN(evt.channel);
}

void onStaDisconnected(const WiFiEventStationModeDisconnected &evt)
{
  // Start connecting timer
  if (!connectingTimer.active())
  {
    connectingTimer.start();
  }

  DEBUG("WiFi disconnected: ");
  DEBUG(evt.ssid);
  DEBUG(" ");
  DEBUGLN(evt.reason);
}

void onStaGotIP(const WiFiEventStationModeGotIP &evt)
{
  DEBUG("Got IP: ");
  DEBUG(evt.ip);
  DEBUG(" ");
  DEBUG(evt.mask);
  DEBUG(" ");
  DEBUGLN(evt.gw);
}

void onStaDHCPTimeout()
{
  DEBUGLN("DHCP Timeout");
}

void setupAP()
{
  if (WiFi.getMode() == WIFI_AP)
  {
    return;
  }

  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.persistent(true);
  WiFi.softAP(cfg.apSsid, cfg.apPass);

  DEBUGLN("Started in AP mode");
  DEBUGLN(WiFi.softAPIP());
}

void setupLocal()
{
  if (cfg.staSsid == NULL || cfg.staPass == NULL)
  {
    DEBUGLN("WiFi not configured");
    setupAP();
  }
  else
  {
    DEBUGLN("Connecting WiFi");

    // Start connecting timer
    connectingTimer.start();

    // Close old connections
    WiFi.disconnect(true);
#ifdef ESP8266
    WiFi.setPhyMode(WIFI_PHY_MODE_11N);
#endif

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    // Make sure the wifi does not autoconnect but always reconnects
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(true);

    // WiFi Events
    staConnectedHandler = WiFi.onStationModeConnected(&onStaConnected);
    staDisconnectedHandler = WiFi.onStationModeDisconnected(&onStaDisconnected);
    staGotIPHandler = WiFi.onStationModeGotIP(&onStaGotIP);
    staDHCPTimeoutHandler = WiFi.onStationModeDHCPTimeout(&onStaDHCPTimeout);

#ifdef ESP8266
    WiFi.hostname(HOSTNAME);
#endif
    WiFi.begin(cfg.staSsid, cfg.staPass, 0, NULL, true);
    // delay(2000);
  }
}

void startWiFi()
{
  if (cfg.staModeEn)
  {
    // Connect to WiFi
    setupLocal();
  }
  else
  {
    // AP mode
    setupAP();
  }
}

/* =================== MQTT ================== */
void startMQTT()
{
  if (!cfg.mqttEn || !WiFi.isConnected())
  {
    return;
  }

  if (cfg.mqttServer == NULL || cfg.mqttLogin == NULL || cfg.mqttPass == NULL)
  {
    DEBUGLN("MQTT not configured");

    return;
  }
  else
  {
    DEBUGLN("Connecting MQTT");

    mqttClient.setServer(cfg.mqttServer, cfg.mqttPort);
    mqttClient.setCallback(mqttMessageHandler);
    mqttClient.setKeepAlive(60 * 60);

    const String clientId = getChipID();
    const String statusTopic = getTopicName(MQTT_TOPIC_STATUS);

    if (mqttClient.connect(clientId.c_str(), cfg.mqttLogin, cfg.mqttPass, statusTopic.c_str(), 1, false, "offline"))
    {
      DEBUGLN("MQTT connected");

      publishMessage(MQTT_TOPIC_STATUS, "online", false);
      subscribeToTopic(MQTT_TOPIC_FEED);
      subscribeToTopic(MQTT_TOPIC_DOSAGE);
    }
  }
}

/* ============= Timer callbacks ============= */
void heartbeatCallback()
{
  publishMessage(MQTT_TOPIC_STATUS, "online", false);
}

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
        DEBUGLN("Schedule time");
        feed();
      }
    }
  }
}

/* ================== Main =================== */
void setup()
{
#ifdef DEBUG_MODE
  Serial.begin(115200);
#endif

  initPins();
  initEEPROM();

  led.begin();
  scheduleTimer.attach(scheduleCallback);
  heartbeatTimer.attach(heartbeatCallback);

  startWiFi();
  startMQTT();

  initNTP();
  initStepper();
  initUI();
  initFS();
}

void loop()
{
  btn.tick();
  ntp.tick();
  mqttClient.loop();
  stepper.tick();
  ui.tick();
  led.tick();
  scheduleTimer.tick();
  heartbeatTimer.tick();
  connectingTimer.tick();

  if (btn.hasClicks(2))
  {
    // Кормим
    feed();
  }

  // If button hold 3 seconds or more
  if (btn.release())
  {
    if (btn.pressFor() >= 3000)
    {
      DEBUGLN("Reset and restart");

      // Reset settings
      resetEEPROM();
      delay(100);
      ESP.restart();
    }
  }

  // Led
  if (stepper.tick())
  {
    led.blink(5);
  }
  else if (connectingTimer.active())
  {
    led.pulse();
  }
  else if (cfg.staModeEn && WiFi.status() != WL_CONNECTED)
  {
    led.smooth();
  }
  else
  {
    led.on();
  }

  if (stepper.ready())
  {
    publishMessage(MQTT_TOPIC_FEED_STATUS, "0", false);
  }

  if (cfg.staModeEn)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      // Connect to MQTT
      if (!mqttClient.connected())
      {
        startMQTT();
      }
    }
    else
    {
      // If there is no connection for a long time, launch AP
      if (connectingTimer.ready())
      {
        setupAP();
      }
    }
  }
}