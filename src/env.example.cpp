/* ================ Settings ================= */
#define VERSION 1.0           // Version
#define DEBUG_MODE            // Debug mode
#define EEPROM_KEY 0x10       // EEPROM key
#define EEPROM_RESET_KEY 0x11 // EEPROM reset key
/* ================== WiFi =================== */
#define HOSTNAME "Feeder"                 // Device hostname
#define AP_DEFAULT_SSID "Wi-Fi Feeder AP" // AP ssid name
#define AP_DEFAULT_PASS "00000000"        // AP password
#define STA_DEFAULT_SSID ""               // WiFi ssid
#define STA_DEFAULT_PASS ""               // WiFi password
#define WIFI_TIMEOUT 60000                // WiFi connection timeout
#define NTP_TIMEZONE 3                    // Timezone in hours
#define OTA_ENABLED 1                     // OTA enabled

/* ============ MQTT credentials ============= */
#define MQTT_SERVER "m5.wqtt.ru" // MQTT server
#define MQTT_PORT 7678           // MQTT port
#define MQTT_LOGIN "login"       // MQTT login
#define MQTT_PASS "pass"         // MQTT password
#define MQTT_MSG_BUFFER_SIZE 100

/* ============== MQTT topics ================ */
#define MQTT_TOPIC_STATUS "status"           // Online/Offline status
#define MQTT_TOPIC_FEED "feed"               // Feed action
#define MQTT_TOPIC_FEED_STATUS "feed_status" // Feed status
#define MQTT_TOPIC_DOSAGE "dosage"           // Dosage change

/* ============== App Settings =============== */
#define STEPPER_STEPS 200       // Steps per revolution
#define STEPPER_MICRO_STEPS 16  // Microsteps
#define STEPPER_GEAR_RATIO 5.16 // Gear ratio
#define DRIVER_STEP_TIME 1      // Delay between switching steps
#define RESET_TIMEOUT 5000      // Reset timeout on button hold