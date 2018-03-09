#include "mqtt.h"

const char* mqtt_server = "www.futureSmart.top";

const char* subTopic = "device/device_operate";
const char* pubTopic = "device/device_register";

const char* device_name = "switch2";


WiFiClient espClient;
PubSubClient MQTTClient(espClient);
deviceManipulation devicMp(&MQTTClient);

// global variables
state_t sw_state = OFF;

bool jsonPaser(byte* payload){

// detect duplicate msg
  unsigned long currentMsg = millis();

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.parse(payload);
  if(!data.success()){
    return false;
  }
  const char* name = data["name"];
  const char* target_id = data["target_id"];
  const char* action = data["action"];
  const char* value = data["value"];

#ifdef DEBUG_JSON
    DEBUG_JSON.println(name);
    DEBUG_JSON.println(target_id);
    DEBUG_JSON.println(action);
    DEBUG_JSON.println(value);
#endif
  if(!strcmp(target_id, device_name)){
    if(!strcmp(value,"0")){
      TurnOFF(&sw_state);
    }else if(!strcmp(value,"1")){
      TurnON(&sw_state);
    }else{
      return false;
    }
  }else{
    return false;
  }
  return true;

}

void callback(char* topic, byte* payload, unsigned int length){
#ifdef DEBUG_MQTT
  DEBUG_MQTT.print(TimeStamp + "Message arrived [");
  DEBUG_MQTT.print(topic);
  DEBUG_MQTT.print("] ");

  for (int i = 0; i < length; i++) {
    DEBUG_MQTT.print((char)payload[i]);
  }
  DEBUG_MQTT.println();
#endif

  //jsonPaser(payload);
  if(strcmp(topic, "device/device_operate") == 0)
    devicMp.deviceOperate(payload);

}

bool MQTTSetup(){
  Serial.println(TimeStamp + "start MQTTSetup...");
#if 0
  char mqtt_server[255] = "";
  String location = SSDPClient.getLocation();
  if(location == ""){
    return false;
  }
  location.toCharArray(mqtt_server,location.length()+1);
#endif
#ifdef DEBUG_MQTT
  DEBUG_MQTT.print(TimeStamp + "mqtt server: ");
  DEBUG_MQTT.println(mqtt_server);
#endif
  MQTTClient.setServer(mqtt_server, SSDP_PORT);
  MQTTClient.setCallback(callback);
  return true;
}

void reconnect() {
  // Loop until we're reconnected
  if(!MQTTClient.connected()) {
 #ifdef DEBUG_MQTT
    DEBUG_MQTT.println(TimeStamp + "Attempting MQTT connection...");
 #endif
    MQTTSetup();
    // Create client name
    String clientId = "ESP8266-";
    clientId += device_name;
    // Attempt to connect
    if (MQTTClient.connect(clientId.c_str())) {
#ifdef DEBUG_MQTT
      DEBUG_MQTT.println(TimeStamp + "mqtt broker connected");
#endif
      // Once connected, publish an announcement...
      MQTTClient.publish(pubTopic, "hello world");
      //device register
      //MQTTClient.publish(pubTopic, "{\"type\":\"lamp\",\"vendor\":\"ht\",\"MAC\":\"2c3ae82205b1\"}");
      devicMp.deviceRegister();

      // ... and resubscribe
      MQTTClient.subscribe(subTopic);
    } else {

#ifdef DEBUG_MQTT
      DEBUG_MQTT.print(TimeStamp + "failed, rc=");
      DEBUG_MQTT.print(MQTTClient.state());
      DEBUG_MQTT.println(" try again in 5 seconds");
#endif
      //delay(1000);
    }
  }
}
