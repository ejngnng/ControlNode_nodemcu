
#ifndef deviceManipulation_H
#define deviceManipulation_H

#include <Arduino.h>

#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <WiFiManager.h>
#include <DNSServer.h>


class deviceManipulation
{

private:
    PubSubClient* mqttClient;
    const char* topicRegister = "device/device_register";
    const char* topicStatus = "device/status_update";

    int operatePeer(const char* action, const char* value);


public:
    deviceManipulation(PubSubClient *mqttClient);
    int deviceRegister();
    int deviceOperate(byte* payload);


};

#endif
