
#include "deviceManipulation.h"


deviceManipulation::deviceManipulation(PubSubClient *mqttClient)
{
    this->mqttClient = mqttClient;
}

int deviceManipulation::deviceRegister()
{
    uint8 mac[6];
    char mac_str[12] = {0};
    char msg[256] = {0};

    wifi_get_macaddr(STATION_IF, mac);
    printf("get mac %0x%0x%0x%0x%0x%0x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    sprintf(msg, "{\"type\":\"lamp\",\"vendor\":\"ht\",\"MAC\":\"");
    sprintf(mac_str, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    strcat(msg, mac_str);
    strcat(msg, "\"}");

    printf("\nbegin to pub %s\n", msg);
    mqttClient->publish(topicRegister, msg);

    return 0;
}

int deviceManipulation::deviceOperate(byte* payload)
{
    char msg[256] = {0};
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parse(payload);
    if(!data.success()){
        return false;
    }
    const char* uuid = data["UUID"];
    const char* action = data["action"];
    const char* value = data["value"];

    printf("uuid is %s\n", uuid);
    printf("action is %s\n", action);
    printf("value is %s\n", value);

    operatePeer(action, value);

    strcat(msg, "{\"UUID\":\"");
    strcat(msg, uuid);
    strcat(msg, "\",\"attribute\":\"");
    strcat(msg, action);
    strcat(msg, "\",\"value\":\"");
    strcat(msg, value);
    strcat(msg, "\"}");

    printf("\nbegin to pub %s\n", msg);
    mqttClient->publish(topicStatus, msg);

    return 0;
}

int deviceManipulation::operatePeer(const char* action, const char* value)
{
    uint8 act_id = 0;

    if(strcmp(action, "onoff") == 0)
        act_id = 1;
    else if(strcmp(action, "lightness") == 0)
        act_id = 2;
    else if(strcmp(action, "color") == 0)
        act_id = 3;
    else if(strcmp(action, "mode") == 0)
        act_id = 4;

    Serial.print(act_id);
}
