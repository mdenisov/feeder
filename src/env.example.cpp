/* ================ Settings ================= */
#define VERSION 1.0     // Версия
#define NAME "Feeder"   // Название устройства
#define DEBUG_MODE      // Режим отладки
#define EEPROM_KEY 0x10 // Ключ EEPROM (1 байт) - измени, чтобы сбросить настройки

/* ================== WiFi =================== */
#define AP_DEFAULT_SSID "Wi-Fi Feeder AP" // Стандартное имя точки доступа
#define AP_DEFAULT_PASS "00000000"        // Стандартный пароль точки доступа
#define STA_DEFAULT_SSID ""               // Стандартное имя точки доступа роутера
#define STA_DEFAULT_PASS ""               // Стандартный пароль точки доступа роутера
#define WIFI_TIMEOUT 60000                // Таймаут на подключение к Wi-Fi
#define NTP_TIMEZONE 3                    // Часовой пояс в часах (например Москва 3)

/* ============ MQTT credentials ============= */
#define MQTT_SERVER "m5.wqtt.ru" // Сервер MQTT
#define MQTT_PORT 7678           // Порт MQTT
#define MQTT_LOGIN "login"       // Логин MQTT
#define MQTT_PASS "pass"         // Пароль MQTT
#define MQTT_MSG_BUFFER_SIZE 100

/* ============== MQTT topics ================ */
#define MQTT_TOPIC_STATUS "status"
#define MQTT_TOPIC_FEED "feed"
#define MQTT_TOPIC_FEED_STATUS "feed_status"
#define MQTT_TOPIC_DOSAGE "dosage"

/* ============== App Settings =============== */
#define STEPPER_STEPS 200       // Количество шагов на 1 оборот
#define STEPPER_MICRO_STEPS 16  // Микрошаги
#define STEPPER_GEAR_RATIO 5.16 // Передаточное число редуктора
#define DRIVER_STEP_TIME 1      // Задержка между переключением