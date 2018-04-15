#ifndef __MQTT_H
#define __MQTT_H

#include <Arduino.h>
#include "common/common.h"

#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "GPIO/io.h"
#include "Operate/deviceManipulation.h"
#include "wifi/wifi.h"

#define SSDP_PORT 1883

bool MQTTSetup();

bool jsonPaser(byte* payload);

void callback(char* topic, byte* payload, unsigned int length);

void reconnect();

void constructSubTopic(char *topic);


#endif
