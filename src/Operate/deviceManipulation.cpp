
#include "deviceManipulation.h"


deviceManipulation::deviceManipulation(PubSubClient *mqttClient)
{
    this->mqttClient = mqttClient;
}

int deviceManipulation::deviceRegister()
{
  #if 0
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
#endif
    deviceObj_t device;
    memset(&device, 0, sizeof(deviceObj_t));
    device.deviceName = "light";
    device.deviceType = "lamp";
    device.deviceVendor = "ht";

    registerMsg reg(&device);
    String msg = reg.registerMsgGenerator();
    char buffer[msg.length()+1];
    memset(buffer, 0, msg.length()+1);
    msg.toCharArray(buffer, msg.length()+1);
    #ifdef DEBUG_MQTT
    Serial.print("register msg: ");
    Serial.println(buffer);
    #endif
    mqttClient->publish(topicRegister, buffer);

    return 0;
}

int deviceManipulation::deviceOperate(byte* payload)
{
//    char msg[256] = {0};
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parse(payload);
    if(!data.success()){
        return false;
    }
    const char* uuid = data["UUID"];
    const char* action = data["action"];
    const char* value = data["value"];
#if 0
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
#endif
    deviceStatus_t deviceStatus;
    memset(&deviceStatus, 0, sizeof(deviceStatus_t));
    int act_id = operatePeer(action, value);
    deviceStatus.attribute = (attribute_type)act_id;
    deviceStatus.value = value;
    statusMsg status(&deviceStatus);
    String msg = status.statusMsgGenerator();

    char buffer[msg.length()+1];
    memset(buffer, 0, msg.length()+1);
    msg.toCharArray(buffer, msg.length()+1);

    #ifdef DEBUG_MQTT
    Serial.print("status msg: ");
    Serial.println(buffer);
    #endif
    mqttClient->publish(topicStatus, buffer);

    return 0;
}

int deviceManipulation::operatePeer(const char* action, const char* value)
{
    uint8 act_id = 0;

    if(strcmp(action, "onoff") == 0)
        act_id = (int)ONOFF;
    else if(strcmp(action, "lightness") == 0)
        act_id = (int)LIGHTNESS;
    else if(strcmp(action, "color") == 0)
        act_id = (int)COLOR;
    else if(strcmp(action, "mode") == 0)
        act_id = (int)MODE;

    Serial.print(act_id);
    return act_id;
}
