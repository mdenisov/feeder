#include "env.cpp"

/* ================ Распиновка =============== */
#define BTN_PIN 5
#define LED_PIN 13
#define EN_PIN 12
#define STEP_PIN 14
#define DIR_PIN 16
/* =========================================== */

/* ============ Список библиотек ============= */
#include "Led.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <EncButton.h>
#include <GyverNTP.h>
#include <GyverPortal.h>
#include <GyverStepper2.h>
#include <LittleFS.h>
#include <PubSubClient.h>
/* =========================================== */

/* ============ Список объектов ============== */
GyverPortal ui(&LittleFS);
Button btn(BTN_PIN);
GyverNTP ntp(3);
GStepper2<STEPPER2WIRE> stepper(STEPPER_STEPS *STEPPER_MICRO_STEPS, STEP_PIN, DIR_PIN, EN_PIN);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Led led(LED_PIN);
/* =========================================== */

/* ================ Макросы ================== */
#ifdef DEBUG_MODE
#define DEBUG(...) Serial.print(__VA_ARGS__)
#define DEBUGLN(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG(...)
#define DEBUGLN(...)
#endif
/* =========================================== */

/* ========= Глобальные переменные =========== */
// Структура со всеми настройками
struct
{
  char apSsid[21] = AP_DEFAULT_SSID;   // Имя сети для AP режима по умолчанию
  char apPass[21] = AP_DEFAULT_PASS;   // Пароль сети для AP режима по умолчанию
  char staSsid[21] = STA_DEFAULT_SSID; // Имя сети для STA режима по умолчанию
  char staPass[21] =
      STA_DEFAULT_PASS;              // Пароль сети для STA режима по умолчанию
  bool staModeEn = false;            // Подключаться к роутеру по умолчанию?
  char mqttServer[21] = MQTT_SERVER; // Сервер MQTT
  int mqttPort = MQTT_PORT;          // Порт MQTT
  char mqttLogin[21] = MQTT_LOGIN;   // Логин MQTT
  char mqttPass[21] = MQTT_PASS;     // Пароль MQTT
  bool mqttEn = true;                // Использовать брокер?
  int dosage = 1;                    // Дозировка
} cfg;
// Расписание
const byte feedTime[][2] = {
    {9, 0},
    {14, 0},
    {20, 0},
};
const int feedAmount = (STEPPER_STEPS * STEPPER_MICRO_STEPS * STEPPER_GEAR_RATIO) / 4;
bool connectInProgress = 0;

/* =============== Колбэки WiFi ============== */
static WiFiEventHandler staConnectedHandler;
static WiFiEventHandler staDisconnectedHandler;
static WiFiEventHandler staGotIPHandler;
static WiFiEventHandler staDHCPTimeoutHandler;
/* =========================================== */

/* ================= Логика ================== */
String getChipID()
{
  return String(system_get_chip_id());
}

String getTopicName(const char *topic)
{
  const String clientId = getChipID();

  return String(PREFIX) + "/" + clientId + "/" + topic;
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
  // Пишем настройки
  EEPROM.put(1, cfg);
  // Запись
  EEPROM.commit();
  delay(50);
}

void resetEEPROM()
{

  DEBUGLN("Reset EEPROM");

  // Пишем ключ
  EEPROM.write(0, EE_KEY);
  updateEEPROM();
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
    cfg.dosage = message.toInt();
    updateEEPROM();
  }
}

/* ================= Билд веб-страницы + подсос значений ================= */
void build()
{                             // Билд страницы
  GP.BUILD_BEGIN(400);        // Ширина колонок
  GP.THEME(GP_DARK);          // Темная тема
  GP.PAGE_TITLE("Feeder");    // Обзываем титл
  GP.FORM_BEGIN("/cfg");      // Начало формы
  GP.GRID_RESPONSIVE(600);    // Отключение респонза при узком экране
  M_BLOCK(                    // Общий блок-колонка для WiFi
      GP.SUBMIT("Сохранить"); // Кнопка отправки формы
      M_BLOCK_TAB(            // Конфиг для AP режима -> текстбоксы (логин + пароль)
          "AP-Mode",          // Имя + тип DIV
          GP.TEXT("apSsid", "Логин", cfg.apSsid, "", 20);
          GP.BREAK(); GP.PASS_EYE("apPass", "Пароль", cfg.apPass, "", 20);
          GP.BREAK(););
      M_BLOCK_TAB( // Конфиг для STA режима -> текстбоксы (логин + пароль)
          "WiFi",  // Имя + тип DIV
          GP.TEXT("staSsid", "Логин", cfg.staSsid, "", 20);
          GP.BREAK(); GP.TEXT("staPass", "Пароль", cfg.staPass, "", 20);
          GP.BREAK(); M_BOX(GP_CENTER, GP.LABEL("WiFi Enable");
                            GP.SWITCH("staEn", cfg.staModeEn);););
      M_BLOCK_TAB( // Конфиг для AP режима -> текстбоксы (логин + пароль)
          "MQTT",  // Имя + тип DIV
          // GP.TEXT("mqttServer", "Сервер", cfg.mqttServer, "", 20);
          // GP.BREAK(); GP.NUMBER("mqttPort", "Порт", cfg.mqttPort, "", 20);
          // GP.BREAK(); GP.TEXT("mqttLogin", "Логин", cfg.mqttLogin, "", 20);
          // GP.BREAK(); GP.TEXT("mqttPass", "Пароль", cfg.mqttPass, "", 20);
          GP.TEXT("mqttUUID", "ID", getChipID(), "", 20, "", true);
          GP.BREAK(); M_BOX(GP_CENTER, GP.LABEL("MQTT Enable");
                            GP.SWITCH("mqttEn", cfg.mqttEn);););
      M_BLOCK_TAB("Дозировка", M_BOX(GP.LABEL("Значение");
                                     GP.SPINNER("dosage", cfg.dosage, 1, 5);););
      GP.FORM_END();         // <- Конец формы (костыль)
      M_BLOCK_TAB(           // Блок с OTA-апдейтом
          "ESP UPDATE",      // Имя + тип DIV
          GP.OTA_FIRMWARE(); // Кнопка с OTA начинкой
      ););
  GP.BUILD_END(); // Конец билда страницы
}

// Подсос значений со страницы
void action(GyverPortal &p)
{
  // Если есть сабмит формы - копируем все в переменные
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

    // Сохраняем все настройки в EEPROM
    updateEEPROM();

    // Перегружаем ESP
    ESP.restart();
    // Отключаем AP
    // WiFi.softAPdisconnect();
  }
}

/* ============================ Инициализация ============================= */
void initPins()
{

  DEBUGLN("Init pins");

  pinMode(BTN_PIN, INPUT_PULLUP);
}

void initEEPROM()
{

  DEBUGLN("Init EEPROM");

  // Инициализация EEPROM
  EEPROM.begin(250);
  delay(50);

  // Если ключ еепром не совпадает
  if (EEPROM.read(0) == EE_KEY)
  {

    // Читаем настройки
    EEPROM.get(1, cfg);
    delay(50);
  }
  else
  {
    // Пишем дефолтные настройки
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
  // Подключаем билд веб морды
  ui.attachBuild(build);
  // Подключаем обработчик действий
  ui.attach(action);
  // Стартуем!
  ui.start();
  // Включаем ОТА для прошивки по воздуху
  ui.enableOTA();
}

void initStepper()
{

  DEBUGLN("Init stepper");

  // stepper.setRunMode(FOLLOW_POS);
  // установка макс. скорости в шагах/сек
  stepper.setMaxSpeed(2300);
  // установка ускорения в шагах/сек/сек
  stepper.setAcceleration(3500);
  // отключать мотор при достижении цели
  stepper.autoPower(true);
  // stepper.reverse(true);
  stepper.reset();
  stepper.disable();
}

void initFS()
{
  // Инициализация файловой системы
  LittleFS.begin();
}

/* =================== WiFi ================== */
void onStaConnected(const WiFiEventStationModeConnected& evt)
{
  connectInProgress = 0;

  DEBUG("WiFi connected: ");
  DEBUG(evt.ssid);
  DEBUG(" ");
  DEBUGLN(evt.channel);
}

void onStaDisconnected(const WiFiEventStationModeDisconnected& evt)
{
  connectInProgress = 1;

  DEBUG("WiFi disconnected: ");
  DEBUG(evt.ssid);
  DEBUG(" ");
  DEBUGLN(evt.reason);
}

void onStaGotIP(const WiFiEventStationModeGotIP& evt)
{
  connectInProgress = 0;

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

  connectInProgress = 0;
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

    connectInProgress = 1;

    // Включаем wifi
    WiFi.mode(WIFI_STA);
    // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    // Make sure the wifi does not autoconnect but always reconnects
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(true);

    // WiFi Events
    staConnectedHandler = WiFi.onStationModeConnected(&onStaConnected);
    staDisconnectedHandler = WiFi.onStationModeDisconnected(&onStaDisconnected);
    staGotIPHandler = WiFi.onStationModeGotIP(&onStaGotIP);
    staDHCPTimeoutHandler = WiFi.onStationModeDHCPTimeout(&onStaDHCPTimeout);

    // Задаем сетевое имя
    WiFi.hostname(HOSTNAME);
    // Подключаемся к сети
    WiFi.begin(cfg.staSsid, cfg.staPass, 0, NULL, true);
    delay(2000);
  }
}

void startWiFi()
{
  if (cfg.staModeEn)
  {
    // Подключаемся к роутеру
    setupLocal();
  }
  else
  {
    // Режим точки доступа
    setupAP();
  }
}

/* ================================ MQTT ================================= */
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

/* =========================== Главные функции ============================ */
void setup()
{
#ifdef DEBUG_MODE
  Serial.begin(115200);
#endif

  initPins();
  initEEPROM();

  led.begin();

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
  stepper.tick();
  ui.tick();
  led.tick();
  mqttClient.loop();

  static bool isBlocking = false;
  if (btn.click() || btn.hold())
  {
    // Если кнопка нажата 2 раз
    if (btn.getClicks() == 1)
    {
      DEBUGLN("Button click 2 times");

      // Кормим
      feed();
    }

    // Если кнопка нажата 3 раза
    if (btn.getClicks() == 2)
    {
      DEBUGLN("Button click 3 times");

      // Ставим флаг засорения
      isBlocking = true;
      stepper.setTarget(-1, RELATIVE);
    }

    // Если кнопка удержана
    if (btn.hold(5))
    {
      DEBUGLN("Reset and restart");

      // Сбрасываем настройки
      resetEEPROM();
      ESP.restart();
    }
  }

  // Меняем состояние светодиода
  if (stepper.tick())
  {
    led.blink(5);
  }
  else if (connectInProgress)
  {
    led.pulse();
  }
  else if (cfg.staModeEn && !WiFi.isConnected())
  {
    led.smooth();
  }
  else
  {
    led.on();
  }

  // Защита от застревания
  static int attempts = 0;
  if (stepper.ready())
  {
    publishMessage(MQTT_TOPIC_FEED_STATUS, "0", false);

    if (isBlocking)
    {
      if (attempts < 10)
      {
        bool dir = attempts % 2;
        stepper.setTarget(dir ? 150 : -150, RELATIVE);
        attempts++;
      }
      else
      {
        attempts = 1;
        isBlocking = false;
        feed();
      }
    }
    else
    {
      // DEBUGLN(stepper.getTarget());
      // if (stepper.getCurrent() > 0 && stepper.getCurrent() < feedAmount * cfg.dosage) {
      // Если поворот был меньше 70% от нужного
      if (stepper.pos > 0 && (stepper.pos < feedAmount * cfg.dosage) && ((stepper.pos * 100) / (feedAmount * cfg.dosage)) < 70)
      {
        DEBUG("Stepper is blocking!!!");
        DEBUGLN(stepper.pos);
        isBlocking = true;
      }
    }
  }

  // Подключаемся к MQTT
  if (WiFi.isConnected() && !mqttClient.connected())
  {
    startMQTT();
  }

  // Если долго нет подключения, запускаем AP
  static uint32_t connectingTmr = millis();
  if (cfg.staModeEn && !WiFi.isConnected())
  {
    if (millis() - connectingTmr > 2 * 60 * 1000)
    {
      connectingTmr = millis();
      setupAP();
    }
  }

  // Публикация сообщения с заданной периодичностью
  static uint32_t heartbeatTmr = 0;
  if (millis() - heartbeatTmr > 30000)
  {
    heartbeatTmr = millis();
    publishMessage(MQTT_TOPIC_STATUS, "online", false);
  }

  // Следим за расписанием
  static uint32_t feedTmr = 0;
  if (millis() - feedTmr > 500)
  {
    static byte prevMin = 0;
    feedTmr = millis();
    uint8_t minute = ntp.minute();
    uint8_t hour = ntp.hour();

    if (prevMin != minute)
    {
      prevMin = minute;

      for (byte i = 0; i < sizeof(feedTime) / 2; i++)
      {
        if (feedTime[i][0] == hour && feedTime[i][1] == minute)
        {
          DEBUGLN("Schedule time");
          feed();
        }
      }
    }
  }
}