#ifndef _MQTT_MANAGER
#define _MQTT_MANAGER

#include <Arduino.h>
#include <PubSubClient.h>
#include <TimerMs.h>
#include "AutoGrowBufferStream.h"
#include "config.h"
#include "utils.h"
#include "eeprom-manager.h"
#include "stepper-manager.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
AutoGrowBufferStream stream;
TimerMs heartbeatTimer(30000, 1, 0);

unsigned long mqttattempt = (millis() - 3000);

String getTopicName(const char *topic)
{
  const String clientId = getChipID();

  return String(HOSTNAME) + "/" + clientId + "/" + topic;
}

bool publishMessage(const char *topic, String payload, boolean retained)
{
  const String uniqTopic = getTopicName(topic);

  DEBUG(F("Publish message: ["));
  DEBUG(uniqTopic);
  DEBUG(F("] "));
  DEBUGLN(payload);

  return mqttClient.publish(uniqTopic.c_str(), payload.c_str(), retained);
}

bool subscribeToTopic(const char *topic)
{
  const String uniqTopic = getTopicName(topic);

  DEBUG(F("Subscribe to topic: ["));
  DEBUG(uniqTopic);
  DEBUGLN(F("] "));

  return mqttClient.subscribe(uniqTopic.c_str());
}

void parseCallback(char *topic, byte *payload, unsigned int length)
{
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    // Convert *byte to string
    message += (char)payload[i];
  }

  DEBUG(F("Message arrived ["));
  DEBUG(topic);
  DEBUG(F("] "));
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
    config.dosage = constrain(1, 5, message.toInt());
    updateEEPROM();
  }
}

void mqttMessageHandler(char *topic, byte *payload, unsigned int length)
{
  parseCallback(topic, (byte *)stream.get_buffer(), stream.current_length());
  stream.flush();
}

void heartbeatCallback()
{
  publishMessage(MQTT_TOPIC_STATUS, "online", false);
}

void connectMqtt()
{
  const String clientId = getChipID();
  const String statusTopic = getTopicName(MQTT_TOPIC_STATUS);

  if (!mqttClient.connected() && (millis() - mqttattempt) >= 3000)
  {
    if (mqttClient.connect(clientId.c_str(), config.mqttLogin, config.mqttPass, statusTopic.c_str(), 1, false, "offline"))
    {
      DEBUGLN(F("MQTT connected"));

      publishMessage(MQTT_TOPIC_STATUS, "online", false);
      subscribeToTopic(MQTT_TOPIC_FEED);
      subscribeToTopic(MQTT_TOPIC_DOSAGE);

      heartbeatTimer.attach(heartbeatCallback);
    }
    else
    {
      switch (mqttClient.state())
      {
      case -4:
        // MQTT_CONNECTION_TIMEOUT
        DEBUGLN(F("MQTT TIMEOUT"));
        break;
      case -2:
        // MQTT_CONNECT_FAILED
        DEBUGLN(F("MQTT CONNECT_FAILED"));
        break;
      case -3:
        // MQTT_CONNECTION_LOST
        DEBUGLN(F("MQTT CONNECTION_LOST"));
        break;
      case -1:
        // MQTT_DISCONNECTED
        DEBUGLN(F("MQTT DISCONNECTEDT"));
        break;
      case 1:
        break;
      case 2:
        break;
      case 3:
        break;
      case 4:
        break;
      case 5:
        // MQTT UNAUTHORIZED
        DEBUGLN(F("MQTT UNAUTHORIZED"));
        ESP.restart();
        break;
      }
    }
  }
}

void initMQTT()
{
  if (strlen(config.mqttServer) == 0 || strlen(config.mqttLogin) == 0 || strlen(config.mqttPass) == 0)
  {
    DEBUGLN(F("MQTT not configured"));

    return;
  }

  mqttClient.setServer(config.mqttServer, config.mqttPort);
  mqttClient.setCallback(mqttMessageHandler);
  mqttClient.setKeepAlive(60 * 60);
  mqttClient.setBufferSize(MQTT_MSG_BUFFER_SIZE);
  mqttClient.setStream(stream);

  DEBUGLN(F("Finished setting up MQTT, Attempting to connect"));

  connectMqtt();
}

void loopMQTT()
{
  mqttClient.loop();
  heartbeatTimer.tick();

  // publishMessage(MQTT_TOPIC_FEED_STATUS, String(appState.isFeeding), false);
}

#endif