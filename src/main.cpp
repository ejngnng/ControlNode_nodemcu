#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "common/common.h"

WiFiClient espClient;
PubSubClient MQTTClient(espClient);

StaticJsonBuffer<200> jsonBuffer;

void setup() {
    // put your setup code here, to run once:
    serial_init();
}

void loop() {
    // put your main code here, to run repeatedly:
}
